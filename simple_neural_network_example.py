# ============================================================
# 0.  Imports
# ============================================================
import torch, torch.nn as nn, torch.nn.functional as F
import numpy as np, struct
torch.manual_seed(0)

# ============================================================
# 1.  Bit-pattern helpers (unchanged from earlier explanation)
# ============================================================
BIT_STRINGS = [
    "0111011100001110",  # four random-ish patterns …
    "1010101000010111",
    "0011101001000100",
    "1010100101110101",
    "0000000000000000",  #   + explicit  0x0000
    "1000000000000000",  #   + explicit  0x8000
]

# -- IEEE binary16 ------------------------------------------------------
def f16_from_bits(b):
    return np.frombuffer(struct.pack(">H", int(b, 2)), dtype=np.float16)[0].astype(np.float32)

# -- bfloat16 -----------------------------------------------------------
def bf16_from_bits(b):
    bits32 = int(b, 2) << 16
    return struct.unpack(">f", struct.pack(">I", bits32))[0]

def posit16_es2_from_bits(b):
    """
    Robust Posit-16 (es = 2) decoder.
    Handles cases where the regime eats all remaining bits
    so (remaining-2) could be negative.
    """
    bits = int(b, 2)
    if bits == 0:            # exact zero
        return 0.0

    sign = -1 if bits & 0x8000 else 1
    bits &= 0x7FFF           # strip sign

    # ── decode regime ─────────────────────────────────────────
    regime_sign = 1 if bits & 0x4000 else 0
    run_len, mask = 0, 0x4000
    while mask and ((bits & mask) >> 14) == regime_sign:
        run_len += 1
        mask >>= 1
    k = run_len if regime_sign else -run_len - 1

    remaining = 14 - run_len      # bits left for (exponent + fraction)

    # ── exponent field (es = 2) ──────────────────────────────
    if remaining >= 2:
        exp_bits = (bits >> (remaining - 2)) & 0x3
        frac_bits = bits & ((1 << (remaining - 2)) - 1)
        frac_len  = remaining - 2
    else:                          # no room for exponent
        exp_bits  = 0
        frac_bits = 0
        frac_len  = 0

    # ── construct float value ────────────────────────────────
    useed    = 16                 # 2^(2^es) with es = 2
    exponent = exp_bits
    fraction = 1 + frac_bits / (1 << frac_len) if frac_len else 1.0

    return sign * (useed ** k) * (2 ** exponent) * fraction

# -- Posit-16  (es = 2) -------------------------------------------------
# def posit16_es2_from_bits(b):
#     bits = int(b, 2)
#     if bits == 0:
#         return 0.0
#     sign = -1 if bits & 0x8000 else 1
#     bits &= 0x7FFF

#     regime_sign = 1 if bits & 0x4000 else 0
#     run_len, mask = 0, 0x4000
#     while mask and ((bits & mask) >> 14) == regime_sign:
#         run_len += 1;  mask >>= 1
#     k = run_len if regime_sign else -run_len - 1

#     remaining = 14 - run_len
#     exp_bits   = (bits >> (remaining - 2)) & 0x3
#     frac_bits  =  bits & ((1 << (remaining - 2)) - 1)
#     frac_len   = remaining - 2

#     useed   = 16          # 2^(2^es) with es = 2
#     fraction = 1 + frac_bits / (1 << frac_len) if frac_len else 1
#     return sign * (useed ** k) * (2 ** exp_bits) * fraction


# ============================================================
# 2.  Build "decoder" lambdas for each numeric format
#     (we will use them to initialise weights later)
# ============================================================
decoders = {
    "Posit16": posit16_es2_from_bits,
    "FP16"   : f16_from_bits,
    "BF16"   : bf16_from_bits,
}

# ============================================================
# 3.  Straight-Through Estimator wrapper so quantisers are
#     differentiable (if you later decide to *train*).
# ============================================================
class STEQuant(torch.autograd.Function):
    @staticmethod
    def forward(ctx, tensor, quant_fn):
        q = quant_fn(tensor.detach().cpu().numpy())
        return torch.from_numpy(np.asarray(q, dtype=np.float32)).to(tensor.device)

    @staticmethod
    def backward(ctx, grad_outputs):
        return grad_outputs, None

def make_quant_fn(np_quant):
    return lambda t: STEQuant.apply(t, np_quant)

# -------------- INT8 uniform quantiser (demo of run-time quant) -------
def int8_uniform_np(x, rng=6.0):
    step = (2 * rng) / 255
    return np.clip(np.round((x + rng) / step) * step - rng, -rng, rng)

q_int8 = make_quant_fn(int8_uniform_np)         # will quantise *activations*

# ============================================================
# 4.  Tiny fully-connected network that keeps FP32 weights
#     but quantises all activations with q_int8
# ============================================================
class TinyNet(nn.Module):
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(6, 12, bias=True)
        self.fc2 = nn.Linear(12, 3, bias=True)

    def forward(self, x):
        x = q_int8(x)
        x = q_int8(F.relu(self.fc1(x)))
        x = q_int8(self.fc2(x))
        return x

net = TinyNet()

# ============================================================
# 5.  Initialise the first‐layer weights from your bit-patterns
#     in *three* different numeric interpretations.
# ============================================================
print("Initialising first-layer weight[0] with each format:")
for name, decode in decoders.items():
    decoded_vals = np.array([decode(bits) for bits in BIT_STRINGS], dtype=np.float32)
    with torch.no_grad():
        # put them into the first row of fc1 weight (shape = [12, 6])
        net.fc1.weight[0, :6] = torch.from_numpy(decoded_vals)
    print(f"  {name:7s} →", decoded_vals)

# ============================================================
# 6.  Forward pass on a dummy batch
# ============================================================
batch = torch.randn(4, 6)          # batch of 4 samples, 6 features
logits = net(batch)
print("\nNetwork output (INT8-quantised activations, FP32 weights):\n", logits)
