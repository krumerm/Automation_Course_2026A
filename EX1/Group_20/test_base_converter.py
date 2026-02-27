import unittest
from base_converter import binary_to_decimal, decimal_to_binary

class TestBaseConverter(unittest.TestCase):

    def test_binary_to_decimal_basic(self):
        self.assertEqual(binary_to_decimal("0"), 0)
        self.assertEqual(binary_to_decimal("1"), 1)
        self.assertEqual(binary_to_decimal("101"), 5)
        self.assertEqual(binary_to_decimal("1101"), 13)

    def test_binary_to_decimal_leading_zeroes(self):
        self.assertEqual(binary_to_decimal("0001"), 1)
        self.assertEqual(binary_to_decimal("00101"), 5)

    def test_decimal_to_binary_basic(self):
        self.assertEqual(decimal_to_binary(0), "0")
        self.assertEqual(decimal_to_binary(1), "1")
        self.assertEqual(decimal_to_binary(5), "101")
        self.assertEqual(decimal_to_binary(13), "1101")

    def test_decimal_to_binary_large(self):
        self.assertEqual(decimal_to_binary(1024), "10000000000")
        self.assertEqual(decimal_to_binary(4096), "1000000000000")

    def test_round_trip(self):
        for num in [0, 1, 2, 5, 13, 255, 1024, 4096, 999999]:
            binary = decimal_to_binary(num)
            result = binary_to_decimal(binary)
            self.assertEqual(result, num)

    def test_long_binary(self):
        long_binary = "1" * 64
        expected = int(long_binary, 2)
        result = binary_to_decimal(long_binary)
        self.assertEqual(result, expected)

    def test_binary_with_lots_of_leading_zeroes(self):
        test_binary = "0" * 30 + "101"
        self.assertEqual(binary_to_decimal(test_binary), 5)

    def test_invalid_binary_inputs(self):
        invalid_inputs = ["", "2", "10a01", "11102", "abc", "01b01"]

        for test in invalid_inputs:
            with self.assertRaises(Exception):
                binary_to_decimal(test)

    def test_negative_decimal(self):
        with self.assertRaises(Exception):
            decimal_to_binary(-5)


if __name__ == '__main__':
    unittest.main()
