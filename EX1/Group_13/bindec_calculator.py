
from __future__ import annotations

from decimal import Decimal, getcontext, InvalidOperation
from typing import Union

# Set high precision for decimal operations
getcontext().prec = 60

def _auto_bit_width(abs_value: int) -> int:
    """
    Compute the MINIMAL two's-complement bit width that can represent -abs_value.
    (sign bit + enough bits for the magnitude)
    """
    return abs_value.bit_length() + 1


def binary_to_decimal(bin_str: str) -> Decimal:
    """Convert a binary string (with optional fraction) to a Decimal.
    """
    if not isinstance(bin_str, str):
        raise ValueError("binary_to_decimal expects a string")

    s = bin_str.strip()
    if s == "":
        raise ValueError("empty string is not a valid binary number")

    if s[0] == "-":
        raise ValueError("Negative binary input is not allowed. Use positive binary only.")
    if s[0] == "+":
        s = s[1:]
        s = s.lstrip()

    if s.startswith(("0b", "0B")):
        s = s[2:]

    if s == "":
        raise ValueError(f"no binary digits found in input: {bin_str!r}")

    if s.count(".") > 1:
        raise ValueError(f"multiple decimal points in input: {bin_str!r}")

    if "." in s:
        int_part_s, frac_part_s = s.split(".")
    else:
        int_part_s, frac_part_s = s, ""

    if int_part_s == "" and frac_part_s == "":
        raise ValueError(f"no binary digits found in input: {bin_str!r}")

    # Validate characters
    for ch in int_part_s + frac_part_s:
        if ch not in ("0", "1"):
            raise ValueError(f"invalid binary digit {ch!r} in input: {bin_str!r}")

    # Integer part
    int_value = 0
    if int_part_s:
        int_value = int(int_part_s, 2) if int_part_s != "" else 0

    total = Decimal(int_value)

    # Fractional part: sum bit * 2^{-i}
    if frac_part_s:
        frac = Decimal(0)
        two = Decimal(2)
        for i, ch in enumerate(frac_part_s, start=1):
            if ch == "1":
                frac += Decimal(1) / (two ** i)
        total += frac

    return total


def decimal_to_binary(dec: Union[str, int, float, Decimal], precision: int = 12) -> str:
    """Convert a decimal number (possibly fractional) to binary string.
    """
    # Parse input into Decimal
    if isinstance(dec, Decimal):
        value = dec
    elif isinstance(dec, int):
        value = Decimal(dec)
    elif isinstance(dec, float):
        # Convert float via str to reduce binary float artifacts
        try:
            value = Decimal(str(dec))
        except InvalidOperation:
            raise ValueError(f"invalid decimal float: {dec!r}")
    elif isinstance(dec, str):
        s = dec.strip()
        if s == "":
            raise ValueError("empty string is not a valid decimal number")
        try:
            value = Decimal(s)
        except InvalidOperation:
            raise ValueError(f"invalid decimal number: {dec!r}")
    else:
        raise ValueError("decimal_to_binary expects int, float, Decimal, or string")

    if precision < 0:
        raise ValueError("precision must be non-negative")

    # Two's complement for NEGATIVE values (auto bit width for the integer part)
    if value < 0:
        int_part = int(value // 1)  # floor toward -inf (negative or zero)
        mag = abs(int_part)
        bit_width = _auto_bit_width(mag)

        # two's-complement encoding for the integer part only
        tc = (1 << bit_width) + int_part  # since int_part <= 0
        int_bits = bin(tc)[2:].zfill(bit_width)

        # Fractional part: sign-magnitude (use magnitude of the fractional part)
        frac = abs(value - Decimal(int_part))
        if precision == 0 or frac == 0:
            return int_bits

        bits = []
        two = Decimal(2)
        count = 0
        while frac != 0 and count < precision:
            frac *= two
            if frac >= 1:
                bits.append("1")
                frac -= 1
            else:
                bits.append("0")
            count += 1
        frac_bits = "".join(bits) if bits else "0"
        return int_bits + "." + frac_bits

    # Non-negative values: classic sign-magnitude style text form (no '0b' prefix)
    integer_part = int(value // 1)
    frac = value - Decimal(integer_part)

    int_bits = bin(integer_part)[2:]  # '0' for 0, otherwise binary digits
    if precision == 0 or frac == 0:
        return int_bits

    bits = []
    two = Decimal(2)
    count = 0
    while frac != 0 and count < precision:
        frac *= two
        if frac >= 1:
            bits.append("1")
            frac -= 1
        else:
            bits.append("0")
        count += 1
    frac_bits = "".join(bits) if bits else "0"
    return int_bits + "." + frac_bits


def _run_cli() -> None:
    """Interactive command line for conversions."""
    print("Binary <-> Decimal Converter ")
    print("Type '1' to convert binary -> decimal, '2' for decimal -> binary, or 'q' to quit.")

    while True:
        choice = input("Choose (1=binary->dec, 2=dec->binary, q=quit): ").strip().lower()
        if choice in ("q", "quit", "exit"):
            print("Goodbye")
            return

        if choice == "1":
            s = input("Enter binary (e.g. 101.101 or 0b10.01): ")
            try:
                d = binary_to_decimal(s)
            except ValueError as e:
                print(f"Invalid binary input: {e}")
            else:
                # Print cleanly: if whole number, show int, else show decimal fixed form
                if d == d.to_integral_value():
                    print(f"Decimal: {int(d)}")
                else:
                    print(f"Decimal: {format(d, 'f')}")

        elif choice == "2":
            s = input("Enter decimal number (e.g. 42, -7, 5.625): ")
            prec_in = input("Precision (fractional binary digits, default 12): ").strip()
            try:
                precision = int(prec_in) if prec_in != "" else 12
            except ValueError:
                print("Invalid precision; using default 12")
                precision = 12

            try:
                b = decimal_to_binary(s, precision=precision)
            except ValueError as e:
                print(f"Invalid decimal input: {e}")
            else:
                print(f"Binary: {b}")

        else:
            print("Unknown option. Please type '1', '2', or 'q'.")


if __name__ == "__main__":
    _run_cli()
