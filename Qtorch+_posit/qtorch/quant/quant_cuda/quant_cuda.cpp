#include <torch/torch.h>
#include "quant_cuda.h"
#include <tuple>

using namespace at;

#define CHECK_CUDA(x) TORCH_CHECK(x.is_cuda(), #x " must be a CUDA tensor")
#define CHECK_CONTIGUOUS(x) TORCH_CHECK(x.is_contiguous(), #x " must be contiguous")
#define CHECK_INPUT(x) \
  CHECK_CUDA(x);       \
  CHECK_CONTIGUOUS(x)

Tensor fixed_point_quantize_nearest(Tensor a, int wl, int fl, bool use_clamp, bool symmetric)
{
  CHECK_INPUT(a);
  return fixed_point_quantize_nearest_cuda(a, wl, fl, use_clamp, symmetric);
}

std::tuple<Tensor, Tensor> fixed_point_quantize_nearest_mask(Tensor a, int wl, int fl,
                                                             bool symmetric)
{
  CHECK_INPUT(a);
  return fixed_point_quantize_nearest_mask_cuda(a, wl, fl, symmetric);
}

Tensor block_quantize_nearest(Tensor a, int wl, int dim)
{
  CHECK_INPUT(a);
  return block_quantize_nearest_cuda(a, wl, dim);
}

Tensor block_quantize_sim_nearest(Tensor a, int wl)
{
  CHECK_INPUT(a);
  return block_quantize_sim_nearest_cuda(a, wl);
}

Tensor float_quantize_nearest(Tensor a, int man_bits, int exp_bits)
{
  CHECK_INPUT(a);
  return float_quantize_nearest_cuda(a, man_bits, exp_bits);
}

Tensor fixed_point_quantize_stochastic(Tensor a, int wl, int fl, bool use_clamp, bool symmetric)
{
  CHECK_INPUT(a);
  return fixed_point_quantize_stochastic_cuda(a, wl, fl, use_clamp, symmetric);
}

std::tuple<Tensor, Tensor> fixed_point_quantize_stochastic_mask(Tensor a, int wl, int fl,
                                                                bool symmetric)
{
  CHECK_INPUT(a);
  return fixed_point_quantize_stochastic_mask_cuda(a, wl, fl, symmetric);
}

Tensor block_quantize_stochastic(Tensor a, int wl, int dim)
{
  CHECK_INPUT(a);
  return block_quantize_stochastic_cuda(a, wl, dim);
}

Tensor block_quantize_sim_stochastic(Tensor a, int wl)
{
  CHECK_INPUT(a);
  return block_quantize_sim_stochastic_cuda(a, wl);
}

Tensor float_quantize_stochastic(Tensor a, int man_bits, int exp_bits)
{
  CHECK_INPUT(a);
  return float_quantize_stochastic_cuda(a, man_bits, exp_bits);
}

Tensor posit_quantize_nearest(Tensor a, int nsize, int es, float scale)
{
  CHECK_INPUT(a);
  return posit_quantize_nearest_cuda(a, nsize, es, scale);
}

Tensor new_format_quantize(Tensor a, float scale)
{
  CHECK_INPUT(a);
  return newformat_quantize_nearest_cuda(a, scale);
}

Tensor act_format_quantize(Tensor a, float scale)
{
  CHECK_INPUT(a);
  return actformat_quantize_nearest_cuda(a, scale);
}

Tensor configurable_table_quantize(Tensor a, Tensor lookup_table, float scale)
{
  CHECK_INPUT(a);
  return configurable_table_quantize_cuda(a, lookup_table,  scale);
}

Tensor configurable_table_quantize_rounding_hint(Tensor a, Tensor lookup_table, Tensor rounding_hint, float scale)
{
  CHECK_INPUT(a);
  return configurable_table_quantize_rounding_hint_cuda(a, lookup_table, rounding_hint, scale);
}

Tensor posit_sigmoid(Tensor a, int nsize, int es, float scale)
{
  CHECK_INPUT(a);
  return posit_sigmoid_cuda(a, nsize, es, scale);
}
Tensor posit_tanh(Tensor a, int nsize, int es, float scale)
{
  CHECK_INPUT(a);
  return posit_tanh_cuda(a, nsize, es, scale);
}
Tensor posit_tanh_enhanced(Tensor a, int nsize, int es, float scale)
{
  CHECK_INPUT(a);
  return posit_tanh_enhanced_cuda(a, nsize, es, scale);
}


Tensor posit_quantize_stochastic(Tensor a, int nsize, int es, float scale)
{
  //todo: implement stochastic rounding
  CHECK_INPUT(a);
  return posit_quantize_nearest_cuda(a, nsize, es, scale);
}
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m)
{
  m.def("fixed_point_quantize_stochastic", &fixed_point_quantize_stochastic, "Fixed Point Number Stochastic Quantization (CUDA)");
  m.def("fixed_point_quantize_stochastic_mask", &fixed_point_quantize_stochastic_mask, "Fixed Point Number Stochastic Quantization (CUDA)");
  m.def("block_quantize_stochastic", &block_quantize_stochastic, "Block Floating Point Number Stochastic Quantization (CUDA)");
  m.def("block_quantize_sim_stochastic", &block_quantize_sim_stochastic, "Block Floating Point Number Stochastic Quantization (CUDA)");
  m.def("float_quantize_stochastic", &float_quantize_stochastic, "Low-Bitwidth Floating Point Number Stochastic Quantization (CUDA)");
  m.def("fixed_point_quantize_nearest", &fixed_point_quantize_nearest, "Fixed Point Number Nearest Neighbor Quantization (CUDA)");
  m.def("fixed_point_quantize_nearest_mask", &fixed_point_quantize_nearest_mask, "Fixed Point Number Nearest Neighbor Quantization (CUDA)");
  m.def("block_quantize_nearest", &block_quantize_nearest, "Block Floating Point Number Nearest Neighbor Quantization (CUDA)");
  m.def("block_quantize_sim_nearest", &block_quantize_sim_nearest, "Block Floating Point Number Stochastic Quantization (CUDA)");
  m.def("float_quantize_nearest", &float_quantize_nearest, "Low-Bitwidth Floating Point Number Nearest Neighbor Quantization (CUDA)");
  m.def("posit_quantize_nearest", &posit_quantize_nearest, "Low-Bitwidth Posit nearest rounding (CUDA)");
  m.def("new_format_quantize", &new_format_quantize, "Table lookup format 5-6 bit rounding (CUDA)");
  m.def("act_format_quantize", &act_format_quantize, "Table lookup format 5-6 bit rounding (Activation CUDA)");

  m.def("configurable_table_quantize", &configurable_table_quantize, "Configurable table-lookup Format (CUDA)");
  m.def("configurable_table_quantize_rounding_hint", &configurable_table_quantize_rounding_hint, "Configurable table-lookup Format with hints for rounding for every interval (CUDA)");

  m.def("posit_quantize_stochastic", &posit_quantize_stochastic, "Low-Bitwidth Posit stochastic rounding - temporarily use nearest (CUDA)");
  m.def("posit_sigmoid", &posit_sigmoid, "Low-Bitwidth Posit sigmoid");
  m.def("posit_tanh", &posit_tanh, "Low-Bitwidth Posit tanh");
  m.def("posit_tanh_enhanced", &posit_tanh_enhanced, "Low-Bitwidth Posit tanh with correction");
}
