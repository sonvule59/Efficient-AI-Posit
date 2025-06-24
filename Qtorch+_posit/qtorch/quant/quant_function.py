import torch
from qtorch import Number, FixedPoint, BlockFloatingPoint, FloatingPoint, Posit
import torch.nn as nn
import torch.nn.functional as F
import numpy as np
from torch.utils.cpp_extension import load
import os

current_path = os.path.dirname(os.path.realpath(__file__))

# Try to load CPU extension
try:
    quant_cpu = load(
        name="quant_cpu",
        sources=[
            os.path.join(current_path, "quant_cpu/quant_cpu.cpp"),
            os.path.join(current_path, "quant_cpu/bit_helper.cpp"),
            os.path.join(current_path, "quant_cpu/sim_helper.cpp"),
        ],
        extra_cflags=['-O3', '-std=c++17', '-fPIC'],
        extra_ldflags=['-shared'],
        verbose=True,
    )
except Exception as e:
    print(f"Warning: Failed to load CPU extension: {e}")
    quant_cpu = None

# Try to load CUDA extension if CUDA is available
if torch.cuda.is_available():
    try:
        quant_cuda = load(
            name="quant_cuda",
            sources=[
                os.path.join(current_path, "quant_cuda/quant_cuda.cpp"),
                os.path.join(current_path, "quant_cuda/bit_helper.cu"),
                os.path.join(current_path, "quant_cuda/sim_helper.cu"),
                os.path.join(current_path, "quant_cuda/block_kernel.cu"),
                os.path.join(current_path, "quant_cuda/float_kernel.cu"),
                os.path.join(current_path, "quant_cuda/fixed_point_kernel.cu"),
                os.path.join(current_path, "quant_cuda/quant.cu"),
                os.path.join(current_path, "quant_cuda/posit_kernel.cu"),
            ],
            extra_cflags=['-O3', '-std=c++17', '-fPIC'],
            extra_cuda_cflags=['-O3', '--use_fast_math', '-std=c++17'],
            extra_ldflags=['-shared'],
            verbose=True,
        )
    except Exception as e:
        print(f"Warning: Failed to load CUDA extension: {e}")
        quant_cuda = quant_cpu
else:
    quant_cuda = quant_cpu

__all__ = ["fixed_point_quantize", "block_quantize", "float_quantize", "quantizer", "posit_quantize", "posit_sigmoid", "posit_tanh", "posit_tanh_enhanced", "new_format_quantize", "act_format_quantize", "configurable_table_quantize", "configurable_table_quantize_rounding_hint", "configurable_table_quantize_geomean"]


def assert_wl_fl(wl, fl, stage=""):
    if wl == -1 and fl != -1:
        raise ValueError("fixed point {} wl {}, fl {}".format(stage, wl, fl))


def get_module(x):
    if x.is_cuda and quant_cuda is not None:
        quant_module = quant_cuda
    elif quant_cpu is not None:
        quant_module = quant_cpu
    else:
        raise ValueError("No valid quantization module found")
    return quant_module


def quantizer(
    forward_number=None,
    backward_number=None,
    forward_rounding="stochastic",
    backward_rounding="stochastic",
    clamping_grad_zero=False,
    backward_hooks=[],
):
    """
    Creates a quantization function to support quantizing forward and backward process differently.

    Args:
        - :param: forward_number (qtorch.Number, optional) : the number format used for forward quantization.
                  if is None, the quantization would be a identity mapping.
        - :param: backward_number (qtorch.Number, optional) : the number format used for backward quantization.
                  if is None, the quantization would be a identity mapping.
        - :param: forward_rounding (string) : rounding mode, \"stochastic\" or \"nearest\" (default: \"stochastic\")
        - :param: backward_rounding (string) : rounding mode, \"stochastic\" or \"nearest\" (default: \"stochastic\")
        - :param: clamping_grad_zero (bool) : zero out the gradient of numbers that are being clamped during forward propagation.
                  currently requires forward_number to be a fixed point number.
        - :param: backward_hooks (iterable) : iterable of functions that will be applied to gradients before backward quantization.
                  For example, this can be used to support custom scaling.

    Returns:
        A quantization function as specified (torch.Tensor -> torch.Tensor)
    """

    for rounding in [forward_rounding, backward_rounding]:
        assert rounding in ["stochastic", "nearest",], "invalid rounding type {:s}".format(rounding)
    for num in [forward_number, backward_number]:
        if num != None:
            assert isinstance(num, Number)

    if clamping_grad_zero == False:
        if forward_rounding == "nearest":
            if type(forward_number) == BlockFloatingPoint:
                forward_quant = lambda x, quant_module: quant_module.block_quantize_nearest(
                    x, forward_number.wl, forward_number.dim
                )
            elif type(forward_number) == FixedPoint:
                forward_quant = lambda x, quant_module: quant_module.fixed_point_quantize_nearest(
                    x, forward_number.wl, forward_number.fl, forward_number.clamp, forward_number.symmetric,
                )
            elif type(forward_number) == FloatingPoint:
                forward_quant = lambda x, quant_module: quant_module.float_quantize_nearest(
                    x, forward_number.man, forward_number.exp
                )
            elif type(forward_number) == Posit:
                forward_quant = lambda x, quant_module: quant_module.posit_quantize_nearest(
                    x, forward_number.nsize, forward_number.es, forward_number.scale
                )
        elif forward_rounding == "stochastic":
            if type(forward_number) == BlockFloatingPoint:
                forward_quant = lambda x, quant_module: quant_module.block_quantize_stochastic(
                    x, forward_number.wl, forward_number.dim
                )
            elif type(forward_number) == FixedPoint:
                forward_quant = lambda x, quant_module: quant_module.fixed_point_quantize_stochastic(
                    x, forward_number.wl, forward_number.fl, forward_number.clamp, forward_number.symmetric,
                )
            elif type(forward_number) == FloatingPoint:
                forward_quant = lambda x, quant_module: quant_module.float_quantize_stochastic(
                    x, forward_number.man, forward_number.exp
                )
            elif type(forward_number) == Posit:
                forward_quant = lambda x, quant_module: quant_module.posit_quantize_nearest(
                    x, forward_number.nsize, forward_number.es, forward_number.scale
                )
        else:
            forward_quant = lambda x, quant_module: x
    else:
        if type(forward_number) == FixedPoint or forward_number == None:
            assert (
                forward_number == None or forward_number.clamp == True
            ), "must use clamping if zeroing out clamped gradient"
            if forward_rounding == "nearest":
                forward_quant = lambda x, quant_module: quant_module.fixed_point_quantize_nearest_mask(
                    x, forward_number.wl, forward_number.fl, forward_number.symmetric
                )
            elif forward_rounding == "stochastic":
                forward_quant = lambda x, quant_module: quant_module.fixed_point_quantize_stochastic_mask(
                    x, forward_number.wl, forward_number.fl, forward_number.symmetric
                )
            else:
                forward_quant = lambda x, quant_module: x
        else:
            raise ValueError("zeroing clamping gradient only support fixed point.")

    if backward_rounding == "nearest":
        if type(backward_number) == BlockFloatingPoint:
            backward_quant = lambda a, quant_module: quant_module.block_quantize_nearest(
                a, backward_number.wl, backward_number.dim
            )
        elif type(backward_number) == FixedPoint:
            backward_quant = lambda a, quant_module: quant_module.fixed_point_quantize_nearest(
                a, backward_number.wl, backward_number.fl, backward_number.clamp, backward_number.symmetric,
            )
        elif type(backward_number) == FloatingPoint:
            backward_quant = lambda a, quant_module: quant_module.float_quantize_nearest(
                a, backward_number.man, backward_number.exp
            )
        elif type(backward_number) == Posit:
            backward_quant = lambda a, quant_module: quant_module.posit_quantize_nearest(
                a, backward_number.nsize, backward_number.es, backward_number.scale
            )
        else:
            backward_quant = lambda a, quant_module: a

    elif backward_rounding == "stochastic":
        if type(backward_number) == BlockFloatingPoint:
            backward_quant = lambda a, quant_module: quant_module.block_quantize_stochastic(
                a, backward_number.wl, backward_number.dim
            )
        elif type(backward_number) == FixedPoint:
            backward_quant = lambda a, quant_module: quant_module.fixed_point_quantize_stochastic(
                a, backward_number.wl, backward_number.fl, backward_number.clamp, backward_number.symmetric,
            )
        elif type(backward_number) == FloatingPoint:
            backward_quant = lambda a, quant_module: quant_module.float_quantize_stochastic(
                a, backward_number.man, backward_number.exp
            )
        elif type(backward_number) == Posit:
            backward_quant = lambda a, quant_module: quant_module.posit_quantize_nearest(
                a, backward_number.nsize, backward_number.es, backward_number.scale
            )
        else:
            backward_quant = lambda a, quant_module: a
    else:
        backward_quant = lambda a, quant_module: a

    if clamping_grad_zero == False:

        class Rounding(torch.autograd.Function):
            @staticmethod
            def forward(self, x):
                if forward_number == None:
                    return x

                quant_module = get_module(x)
                out = forward_quant(x.contiguous(), quant_module)

                return out

            @staticmethod
            def backward(self, grad_output):
                if self.needs_input_grad[0]:
                    if backward_number == None:
                        grad_input = grad_output
                    else:
                        quant_module = get_module(grad_output)
                        grad_input = backward_quant(grad_output.contiguous(), quant_module)
                else:
                    grad_input = None

                return grad_input

    else:

        class Rounding(torch.autograd.Function):
            @staticmethod
            def forward(self, x):
                if forward_number == None:
                    self.mask = torch.zeros_like(x).bool()
                    return x
                else:
                    quant_module = get_module(x)
                    out, mask = forward_quant(x.contiguous(), quant_module)
                    self.mask = mask

                return out

            @staticmethod
            def backward(self, grad_output):
                if self.needs_input_grad[0]:
                    if backward_number == None:
                        grad_input = grad_output
                    else:
                        quant_module = get_module(grad_output)
                        # grad_output = grad_output.contiguous().masked_fill_(self.mask, 0)
                        for f in backward_hooks:
                            grad_output = f(grad_output)
                        grad_input = backward_quant(grad_output.contiguous(), quant_module).masked_fill(
                            self.mask.bool(), 0
                        )
                else:
                    grad_input = None

                return grad_input

    return Rounding.apply


def fixed_point_quantize(x, wl, fl, clamp=True, symmetric=False, rounding="stochastic"):
    """
    Quantize a single precision Floating Point into low-precision Fixed Point

    Args:
        - :param: `x` (torch.Tensor) :  the single precision number to be quantized
        - :param: `wl` (int) : word length of the fixed point number being simulated
        - :param: `fl` (int) : fractional length of the fixed point number being simulated
        - :param: `clamp` (bool, optional) : clamp input numbers into representable range. if false,
                  the quantization will only simulate the effect on precision
        - :param: `symmetric` (bool, optional) : discard the minimum representable number to make the representable
                  range symmetric
        - :param: `rounding` (string) : rounding mode, \"stochastic\" or \"nearest\" (default: \"stochastic\")

    Returns:
        - a quantized low-precision block floating point number (torch.Tensor)
    """
    assert isinstance(x, torch.Tensor)
    assert rounding in ["stochastic", "nearest"]
    assert_wl_fl(wl, fl)
    quant_module = get_module(x)
    if rounding == "nearest":
        out = quant_module.fixed_point_quantize_nearest(x.contiguous(), wl, fl, clamp, symmetric)
    elif rounding == "stochastic":
        out = quant_module.fixed_point_quantize_stochastic(x.contiguous(), wl, fl, clamp, symmetric)
    else:
        out = x
    return out


def block_quantize(x, wl, dim=-1, rounding="stochastic"):
    """
    Quantize a single precision Floating Point into low-precision Block Floating Point

    Args:
        - :param: `x` (torch.Tensor) :  the single precision number to be quantized
        - :param: `wl` (int) : word length of the block floating point number being simulated
        - :param: `rounding` (string) : rounding mode, \"stochastic\" or \"nearest\"

    Returns:
        - a quantized low-precision block floating point number (torch.Tensor)
    """
    assert isinstance(x, torch.Tensor), "x is not a single precision Floating Point Tensor"
    assert rounding in ["stochastic", "nearest"], "invalid rounding mode, {}".format(rounding)
    quant_module = get_module(x)
    if rounding == "nearest":
        out = quant_module.block_quantize_nearest(x.contiguous(), wl, dim)
    elif rounding == "stochastic":
        out = quant_module.block_quantize_stochastic(x.contiguous(), wl, dim)
    else:
        out = x
    return out


def float_quantize(x, exp, man, rounding="stochastic"):
    """
    Quantize a single precision Floating Point into low-precision Floating Point

    Args:
        - :attr: `x` (torch.Tensor) : the single precision number(torch.Tensor) to be quantized
        - :attr: `exp` (int) : number of bits allocated for exponent
        - :attr: `man` (int) : number of bits allocated for mantissa, not counting the virtual bit
        - :attr: `rounding` (string) : rounding mode, \"stochastic\" or \"nearest\"

    Returns:
        - a quantized low-precision floating point number (torch.Tensor)
    """
    assert isinstance(x, torch.Tensor), "x is not a single precision Floating Point Tensor"
    assert rounding in ["stochastic", "nearest"], "invalid rounding mode, {}".format(rounding)
    quant_module = get_module(x)
    if rounding == "nearest":
        out = quant_module.float_quantize_nearest(x.contiguous(), man, exp)
    elif rounding == "stochastic":
        out = quant_module.float_quantize_stochastic(x.contiguous(), man, exp)
    else:
        out = x
    return out

def posit_quantize(x, nsize, es, scale = 1.0, rounding="nearest"):
    """
    Quantize a single precision Floating Point into low-precision Floating Point

    Args:
        - :attr: `x` (torch.Tensor) : the single precision number(torch.Tensor) to be quantized
        - :attr: `nsize` (int) : number of bits allocated for the posit format
        - :attr: `es` (int) : number of bits allocated for es field (exponent)
        - :attr: `rounding` (string) : rounding mode, \"stochastic\" or \"nearest\"
        - default rounding: `nearest` because it is easier to implement on hardware
        - conventional: posit(8,2): 8 bits posit with 2 bits exponent es

    Returns:
        - a quantized low-precision posit tensor (torch.Tensor)
    """
    assert isinstance(x, torch.Tensor), "x is not a single precision Floating Point Tensor"
    assert rounding in ["stochastic", "nearest"], "invalid rounding mode, {}".format(rounding)
    quant_module = get_module(x)
    if rounding == "nearest":
        out = quant_module.posit_quantize_nearest(x.contiguous(), nsize, es, scale)
    elif rounding == "stochastic":
        out = quant_module.posit_quantize_nearest(x.contiguous(), nsize, es, scale) #todo; temporarily use nearest rounding at all time
    else:
        out = x
    return out

def posit_sigmoid(x, nsize, es=0, scale = 1.0, rounding="nearest"):
    """
    Quantize a single precision Floating Point into low-precision Floating Point

    Args:
        - :attr: `x` (torch.Tensor) : the single precision number(torch.Tensor) to be quantized
        - :attr: `nsize` (int) : number of bits allocated for the posit format
        - :attr: `es` (int) : number of bits allocated for es field (exponent)
        - :attr: `rounding` (string) : rounding mode, \"stochastic\" or \"nearest\"
        - default rounding: `nearest` because it is easier to implement on hardware
        - conventional: posit(8,2): 8 bits posit with 2 bits exponent es

    Returns:
        - a quantized low-precision posit tensor (torch.Tensor)
    """
    assert isinstance(x, torch.Tensor), "x is not a single precision Floating Point Tensor"
    assert nsize in [8,16], "only nsize = 8 or 16 is supported, es automatically set to 0"
    assert rounding in ["stochastic", "nearest"], "invalid rounding mode, {}".format(rounding)
    quant_module = get_module(x)
    out = quant_module.posit_sigmoid(x.contiguous(), nsize, 0, scale)
    return out

def posit_tanh(x, nsize, es=0, scale = 1.0, rounding="nearest"):
    """
    Quantize a single precision Floating Point into low-precision Floating Point

    Args:
        - :attr: `x` (torch.Tensor) : the single precision number(torch.Tensor) to be quantized
        - :attr: `nsize` (int) : number of bits allocated for the posit format
        - :attr: `es` (int) : number of bits allocated for es field (exponent)
        - :attr: `rounding` (string) : rounding mode, \"stochastic\" or \"nearest\"
        - default rounding: `nearest` because it is easier to implement on hardware
        - conventional: posit(8,2): 8 bits posit with 2 bits exponent es

    Returns:
        - a quantized low-precision posit tensor (torch.Tensor)
    """
    assert isinstance(x, torch.Tensor), "x is not a single precision Floating Point Tensor"
    assert nsize in [8,16], "only nsize = 8 or 16 is supported, es automatically set to 0"
    assert rounding in ["stochastic", "nearest"], "invalid rounding mode, {}".format(rounding)
    quant_module = get_module(x)
    out = quant_module.posit_tanh(x.contiguous(), nsize, 0, scale)
    return out

def posit_tanh_enhanced(x, nsize, es=0, scale = 1.0, rounding="nearest"):
    """
    Quantize a single precision Floating Point into low-precision Floating Point

    Args:
        - :attr: `x` (torch.Tensor) : the single precision number(torch.Tensor) to be quantized
        - :attr: `nsize` (int) : number of bits allocated for the posit format
        - :attr: `es` (int) : number of bits allocated for es field (exponent)
        - :attr: `rounding` (string) : rounding mode, \"stochastic\" or \"nearest\"
        - default rounding: `nearest` because it is easier to implement on hardware
        - conventional: posit(8,2): 8 bits posit with 2 bits exponent es

    Returns:
        - a quantized low-precision posit tensor (torch.Tensor)
    """
    assert isinstance(x, torch.Tensor), "x is not a single precision Floating Point Tensor"
    assert nsize in [8,16], "only nsize = 8 or 16 is supported, es automatically set to 0"
    assert rounding in ["stochastic", "nearest"], "invalid rounding mode, {}".format(rounding)
    quant_module = get_module(x)
    out = quant_module.posit_tanh_enhanced(x.contiguous(), nsize, 0, scale)
    return out

def new_format_quantize(x, scale = 1.0, rounding="nearest"):
    """
    Quantize a single precision Floating Point into low-precision Floating Point

    Args:
        - :attr: `x` (torch.Tensor) : the single precision number(torch.Tensor) to be quantized
        - :attr: `nsize` (int) : number of bits allocated for the posit format
        - :attr: `es` (int) : number of bits allocated for es field (exponent)
        - :attr: `rounding` (string) : rounding mode, \"stochastic\" or \"nearest\"
        - default rounding: `nearest` because it is easier to implement on hardware
        - conventional: posit(8,2): 8 bits posit with 2 bits exponent es

    Returns:
        - a quantized low-precision posit tensor (torch.Tensor)
    """
    assert isinstance(x, torch.Tensor), "x is not a single precision Floating Point Tensor"
    assert rounding in ["stochastic", "nearest"], "invalid rounding mode, {}".format(rounding)
    quant_module = get_module(x)
    out = quant_module.new_format_quantize(x.contiguous(), scale)
    return out

def act_format_quantize(x, scale = 1.0, rounding="nearest"):
    """
    Quantize a single precision Floating Point into low-precision Floating Point

    Args:
        - :attr: `x` (torch.Tensor) : the single precision number(torch.Tensor) to be quantized
    Returns:
        - a quantized low-precision posit tensor (torch.Tensor)
    """
    assert isinstance(x, torch.Tensor), "x is not a single precision Floating Point Tensor"
    assert rounding in ["stochastic", "nearest"], "invalid rounding mode, {}".format(rounding)
    quant_module = get_module(x)
    out = quant_module.act_format_quantize(x.contiguous(), scale)
    return out


def configurable_table_quantize(x, table_lookup, scale = 1.0, rounding="nearest"):
    """
    Quantize a single precision Floating Point into low-precision Floating Point

    Args:
        - :attr: `x` (torch.Tensor) : the single precision number(torch.Tensor) to be quantized
        - :attr: `table_lookup` (torch.Tensor) : the table for quantization (quantized values)

    Returns:
        - a quantized low-precision posit tensor (torch.Tensor)
    """
    assert isinstance(x, torch.Tensor), "x is not a single precision Floating Point Tensor"
    assert rounding in ["stochastic", "nearest"], "invalid rounding mode, {}".format(rounding)
    quant_module = get_module(x)
    out = quant_module.configurable_table_quantize(x.contiguous(), table_lookup.contiguous(), scale)
    return out

def configurable_table_quantize_geomean (x, table_lookup, scale = 1.0):
    """
        round based on geomean instead of nearest rounding
    """
    assert isinstance(x, torch.Tensor), "x is not a single precision Floating Point Tensor"
    quant_module = get_module(x)
    table_np = table_lookup.numpy()
    round_hints = [1e-4]
    for i in range(1,len(table_np)):
        round_hints.append((table_np[i]*table_np[i-1])**0.5)

    #print (round_hints)
    round_hints =  torch.tensor(round_hints, dtype = torch.float)
    out = quant_module.configurable_table_quantize_rounding_hint(x.contiguous(), table_lookup.contiguous(), round_hints.contiguous(), scale)
    return out

def configurable_table_quantize_rounding_hint(x, table_lookup, rounding_hint, scale = 1.0):
    """
    Quantize a single precision Floating Point into low-precision Floating Point

    Args:
        - :attr: `x` (torch.Tensor) : the single precision number(torch.Tensor) to be quantized
        - :attr: `table_lookup` (torch.Tensor) : the table for quantization (quantized values)
        - :attr: `rounding_hint` (torch.Tensor) : hints for rounding to table entries. not mid point or any standard rounding methods

    Returns:
        - a quantized low-precision posit tensor (torch.Tensor)
    """
    assert isinstance(x, torch.Tensor), "x is not a single precision Floating Point Tensor"
    quant_module = get_module(x)
    out = quant_module.configurable_table_quantize_rounding_hint(x.contiguous(), table_lookup.contiguous(), rounding_hint.contiguous(), scale)
    return out


'''
def posit_tanh_enhanced2(x, nsize, es=0, scale = 1.0, rounding="nearest"):

    assert isinstance(x, torch.Tensor), "x is not a single precision Floating Point Tensor"
    assert nsize in [8,16], "only nsize = 8 or 16 is supported, es automatically set to 0"
    assert rounding in ["stochastic", "nearest"], "invalid rounding mode, {}".format(rounding)
    quant_module = get_module(x)
    out = quant_module.posit_tanh_enhanced2(x.contiguous(), nsize, 0, scale)
    return out
'''
