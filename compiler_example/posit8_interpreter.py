class Posit8:
    def __init__(self, bits, es=1):
        self.n = 8
        self.es = es
        self.bits = bits & 0xFF  # Only keep 8 bits

    def decode(self):
        bits = self.bits
        n, es = self.n, self.es
        if bits == 0:
            return 0.0
        if bits == 0x80:  # special "NaR"
            return float('nan')
        # 1. Sign bit
        sign = (bits >> (n - 1)) & 1
        if sign:  # Two's complement for negative posits
            bits = (~bits + 1) & 0xFF
        # 2. Regime
        tmp = bits << 1  # Skip sign
        regime = 0
        regime_sign = (tmp >> (n-1)) & 1
        i = 1
        while ((tmp >> (n-i)) & 1) == regime_sign and i < n:
            regime += 1
            i += 1
        k = regime if regime_sign else -regime
        # 3. Exponent
        exp_offset = 1 + regime  # bits used so far: 1 for regime termination + regime
        if exp_offset + es > n:
            exponent = 0
        else:
            exponent = (bits << (exp_offset+1)) & 0xFF
            exponent = exponent >> (n-es)
        # 4. Fraction
        frac_offset = 1 + regime + es
        frac_bits = n - frac_offset
        if frac_bits > 0:
            frac = (bits & ((1 << frac_bits) - 1))
            fraction = frac / (1 << frac_bits)
        else:
            fraction = 0.0
        useed = 2 ** (2 ** es)
        value = ((-1) ** sign) * (useed ** k) * (2 ** exponent) * (1 + fraction)
        return value

    @staticmethod
    def encode(val, es=1):
        # This is a simplified version for demonstration; real posit encoding is more complex!
        # Only handles values >= 0
        if val == 0:
            return Posit8(0, es)
        elif val != val:
            return Posit8(0x80, es)
        sign = 0
        if val < 0:
            sign = 1
            val = -val
        useed = 2 ** (2 ** es)
        k = 0
        tmp = val
        while tmp >= useed:
            tmp /= useed
            k += 1
        while tmp < 1:
            tmp *= useed
            k -= 1
        exponent = 0
        while tmp >= 2:
            tmp /= 2
            exponent += 1
        if exponent >= (1 << es):
            exponent = (1 << es) - 1
        fraction = tmp - 1
        frac_bits = 8 - 1 - (abs(k)+1) - es
        frac_val = int(round(fraction * (1 << frac_bits))) if frac_bits > 0 else 0
        # Build the bit pattern
        bits = sign << 7
        if k >= 0:
            bits |= ((1 << (k+1)) - 1) << (7 - k)
            bit_pos = 7 - k
        else:
            bits |= 1 << (7 - (-k))
            bit_pos = 7 - (-k)
        bit_pos -= 1
        bits |= (exponent << bit_pos)
        bit_pos -= es
        if bit_pos >= 0 and frac_bits > 0:
            bits |= (frac_val & ((1 << frac_bits) - 1))
        return Posit8(bits, es)

    def __repr__(self):
        return f"Posit8({bin(self.bits)}, value={self.decode()})"

# Simple interpreter loop
def repl():
    print("Posit8 interpreter (add <bin1> <bin2>), q to quit")
    while True:
        cmd = input("> ")
        if cmd.strip() == "q":
            break
        parts = cmd.strip().split()
        if len(parts) != 3 or parts[0] != 'add':
            print("Usage: add 0bXXXXXXXX 0bXXXXXXXX")
            continue
        a = int(parts[1], 2)
        b = int(parts[2], 2)
        p1 = Posit8(a)
        p2 = Posit8(b)
        v1 = p1.decode()
        v2 = p2.decode()
        v_sum = v1 + v2
        res = Posit8.encode(v_sum)
        print(f"Result: {bin(res.bits)} (approx value: {res.decode()})")
        print(f"Operand 1 (binary): {bin(a)} (approx value: {p1.decode()})")
        print(f"Operand 2 (binary): {bin(b)} (approx value: {p2.decode()})")
        print(f"Result (binary): {bin(res.bits)} (approx value: {res.decode()})")

if __name__ == "__main__":
    repl()
