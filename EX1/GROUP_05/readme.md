# BaseConverter

A simple Java utility class for converting numbers between **binary** and **decimal** formats.

## Overview

The `BaseConverter` class provides two main static methods:

- **`binaryToDecimal(String binary)`**  
  Converts a binary string (e.g., `"1010"` or `"-1010"`) into its decimal (`int`) value.

- **`decimalToBinary(int decimal)`**  
  Converts a decimal integer (e.g., `42` or `-10`) into its binary string representation.

The program also includes a `main()` method that demonstrates and tests both conversions.

## Features

 1. Handles both positive and negative numbers  
 2. Validates input  
 3. Prevents integer overflow  
 4. Includes automated test cases in the `main()` method  
 5. Performs *round-trip verification* (decimal → binary → decimal)  

##  Error Handling

`binaryToDecimal` throws `IllegalArgumentException` if:

1. Input is `null` or empty
2. Input contains characters other than `'0'`, `'1'`, or a leading `'-'`
3. Binary value exceeds the 32-bit integer range


## How to Run

1. Save the file as `GIT_GROUP_05.java`
2. Compile:

   ```bash
   javac GIT_GROUP_05.java
   ```
3. Run:

   ```bash
   java BaseConverter
   ```
