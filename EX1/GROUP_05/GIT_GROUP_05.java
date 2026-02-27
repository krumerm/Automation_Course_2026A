public class BaseConverter {
	public static int binaryToDecimal(String binary) {
		if (binary == null) {
			throw new IllegalArgumentException("Input cannot be null");
		}
		String trimmed = binary.trim();
		if (trimmed.isEmpty()) {
			throw new IllegalArgumentException("Input cannot be empty");
		}

		boolean isNegative = false;
		int index = 0;
		if (trimmed.charAt(0) == '-') {
			isNegative = true;
			index = 1;
			if (trimmed.length() == 1) {
				throw new IllegalArgumentException("No digits after sign");
			}
		}

		// accumulate the magnitude in a long so that we can correctly handle values
		// up to 2^31 when interpreting a negative binary string.
		long value = 0L;
		for (; index < trimmed.length(); index++) {
			char c = trimmed.charAt(index);
			if (c != '0' && c != '1') {
				throw new IllegalArgumentException(
						"Invalid binary character: '" + c + "'");
			}
			int bit = c - '0';
			value = value * 2 + bit;
			// If the number is positive (no sign) then any magnitude greater than
			// Integer.MAX_VALUE is invalid.  If the number is negative, we allow
			// one extra bit so that −2^31 (which has magnitude 2^31) can be represented.
			if (!isNegative && value > (long) Integer.MAX_VALUE) {
				throw new IllegalArgumentException(
						"Binary value is too large to fit in an int");
			}
			if (isNegative && value > -(long) Integer.MIN_VALUE) {
				throw new IllegalArgumentException(
						"Binary value is too large to fit in an int");
			}
		}
		long signedValue = isNegative ? -value : value;
		return (int) signedValue;
	}

	public static String decimalToBinary(int decimal) {
		if (decimal == 0) {
			return "0";
		}
		boolean isNegative = decimal < 0;
		long value = decimal;
		if (isNegative) {
			value = -(long) decimal;
		}
		StringBuilder sb = new StringBuilder();
		while (value > 0) {
			sb.append(value % 2);
			value /= 2;
		}
		if (isNegative) {
			sb.append('-');
		}
		return sb.reverse().toString();
	}

	public static void main(String[] args) {
		System.out.println("Binary → Decimal tests:");
		String[] binaryInputs = {
				"0", "1", "10", "1111", "101010", "-1010", "1000000000",
				"-10000000000000000000000000000000"
		};
		for (String bin : binaryInputs) {
			try {
				int dec = binaryToDecimal(bin);
				System.out.printf("%s → %d%n", bin, dec);
			} catch (IllegalArgumentException e) {
				System.out.printf("%s → ERROR: %s%n", bin, e.getMessage());
			}
		}
		System.out.println();

		System.out.println("Decimal → Binary tests:");
		int[] decimalInputs = {
				0, 1, 2, 15, 42, -10, 256, Integer.MIN_VALUE, Integer.MAX_VALUE
		};
		for (int dec : decimalInputs) {
			String bin = decimalToBinary(dec);
			System.out.printf("%d → %s%n", dec, bin);
		}
		System.out.println();

		System.out.println("Roundtrip tests:");
		boolean allPassed = true;
		for (int dec : decimalInputs) {
			try {
				String bin = decimalToBinary(dec);
				int back = binaryToDecimal(bin);
				boolean passed = (dec == back);
				System.out.printf("%d → %s → %d : %s%n", dec, bin, back,
						passed ? "PASS" : "FAIL");
				if (!passed) {
					allPassed = false;
				}
			} catch (IllegalArgumentException e) {
				System.out.printf("Error converting %d: %s%n", dec, e.getMessage());
				allPassed = false;
			}
		}
		System.out.println(allPassed ? "All roundtrip tests passed." : "Some tests failed.");
	}
}

