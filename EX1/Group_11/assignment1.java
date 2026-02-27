package automation;

import java.math.BigInteger;
import java.util.Scanner;

public class assignment1 {
	
	// Fixed bit-width for conversions (using 8-bit values)
   private static final int BIT_WIDTH = 8; 
   
   // Pads the given string on the left with the given character until it reaches the desired length.
   private static String padLeft(String s, int len, char ch) {
        StringBuilder sb = new StringBuilder(len);
        for (int i = s.length(); i < len; i++) sb.append(ch);
        sb.append(s);
        return sb.toString();

    }
  // Checks if the given string is a valid binary number (only 0/1, optional +/- sign, no dots or commas).
    public static boolean isValidBinary(String s) {
        if (s == null) return false;
        s = s.trim();
        if (s.isEmpty()) return false;

        int start = 0;
        char first = s.charAt(0);
        if (first == '+' || first == '-') {
            if (s.length() == 1) return false;
            start = 1;
        }

        for (int i = start; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '.' || c == ',') return false;
            if (c != '0' && c != '1') return false;
        }
        return true;
    }
   // Checks if the given string is a valid hexadecimal number (0–9, A–F, optional +/- sign, no dots or commas).
    public static boolean isValidHex(String s) {
        if (s == null) return false;
        s = s.trim();
        if (s.isEmpty()) return false;

        int start = 0;
        char first = s.charAt(0);
        if (first == '+' || first == '-') {
            if (s.length() == 1) return false;
            start = 1;
        }

        for (int i = start; i < s.length(); i++) {
            char c = Character.toUpperCase(s.charAt(i));
            if (c == '.' || c == ',') return false;
            if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) return false;
        }
        return true;
    }
 // Converts a (signed) binary string into its fixed-width hexadecimal representation.
 // Supports two's complement for negative values and enforces the configured bit width.
    public static String binaryToHex(String binaryStr) {
        binaryStr = binaryStr.trim();// Remove spaces from the beginning and end of the string.
        boolean negative = false;// Flag to check if the number is negative.

        if (binaryStr.charAt(0) == '+' || binaryStr.charAt(0) == '-') { // If the first character is + or -, save the sign.
            negative = (binaryStr.charAt(0) == '-');// true if the number is negative.
            binaryStr = binaryStr.substring(1);// Remove the sign from the string for processing.
        }

        BigInteger mag = new BigInteger(binaryStr, 2);// Convert the binary string (base 2) into a decimal number.
        BigInteger twoPowN = BigInteger.ONE.shiftLeft(BIT_WIDTH);// Calculate 2^BIT_WIDTH (for example, 2^8 = 256).
        BigInteger pattern;
        if (negative) {
            pattern = twoPowN.subtract(mag);// If negative, calculate two's complement (256 - value).
        } else {
            pattern = mag;
        }


        BigInteger maxMag = BigInteger.ONE.shiftLeft(BIT_WIDTH - 1);// Calculate 2^(BIT_WIDTH - 1) = max magnitude for negatives.
        if (negative && mag.compareTo(maxMag) >= 0) // If the negative number is too large to fit in the selected bit width, 
        	//throw an error.
            throw new IllegalArgumentException("Value does not fit in " + BIT_WIDTH + " bits (negative).");
        if (!negative && pattern.compareTo(twoPowN) >= 0) // If the positive number is too large to fit in the selected bit width, 
        	//throw an error.
            throw new IllegalArgumentException("Value does not fit in " + BIT_WIDTH + " bits (positive).");

        String hex = pattern.toString(16).toUpperCase();// Convert the number to a hexadecimal string (base 16, uppercase).
        int hexLen = (BIT_WIDTH + 3) / 4; // Calculate how many hex digits are needed (4 bits per hex digit).
        return padLeft(hex, hexLen, '0'); // Add leading zeros if needed.
    }
    
 // Converts a hexadecimal string (with optional +/− sign) into its binary representation.
 // The logic is similar to binaryToHex but reversed: it interprets the input as base 16,
 // handles negative numbers using two’s complement, and returns a fixed-width binary string.
    public static String hexToBinary(String hexStr) {
        hexStr = hexStr.trim();
        boolean negative = false;

        if (hexStr.charAt(0) == '+' || hexStr.charAt(0) == '-') {
            negative = (hexStr.charAt(0) == '-');
            hexStr = hexStr.substring(1);
        }

        BigInteger mag = new BigInteger(hexStr, 16);// Parse the hexadecimal string into a decimal BigInteger (base 16).
        BigInteger twoPowN = BigInteger.ONE.shiftLeft(BIT_WIDTH);

        BigInteger pattern;
        if (negative) {
            pattern = twoPowN.subtract(mag);
        } else {
            pattern = mag;
        }

        BigInteger maxMag = BigInteger.ONE.shiftLeft(BIT_WIDTH - 1);
        if (negative && mag.compareTo(maxMag) >= 0)
            throw new IllegalArgumentException("Value does not fit in " + BIT_WIDTH + " bits (negative).");
        if (!negative && pattern.compareTo(twoPowN) >= 0)
            throw new IllegalArgumentException("Value does not fit in " + BIT_WIDTH + " bits (positive).");

        String bin = pattern.toString(2);// Convert the final value into a binary string (base 2).
        return padLeft(bin, BIT_WIDTH, '0');// Pad the binary output with leading zeros to match the BIT_WIDTH (e.g., 8 bits).
    }

    private static void printMenu() {
        System.out.println("1) Binary -> Hexadecimal");
        System.out.println("2) Hexadecimal -> Binary");
        System.out.println("3) Exit");
        System.out.print("Choose an option: ");
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);

        while (true) {
            printMenu();
            String choice = sc.nextLine().trim();

            if (choice.equals("1")) {
                System.out.print("Enter a binary number (0/1, optional +/−): ");
                String bin = sc.nextLine();
                if (!isValidBinary(bin)) {
                    System.out.println("Invalid input: Only 0/1 allowed (optional +/−), no dots/commas.");
                    continue;
                }
                System.out.println("Result (Hexadecimal): " + binaryToHex(bin));

            } else if (choice.equals("2")) {
                System.out.print("Enter a hexadecimal number (0–9, A–F, optional +/−): ");
                String hex = sc.nextLine();
                if (!isValidHex(hex)) {
                    System.out.println("Invalid input: Only 0–9 or A–F allowed (optional +/−), no dots/commas.");
                    continue;
                }
                System.out.println("Result (Binary): " + hexToBinary(hex));

            } else if (choice.equals("3")) {
                System.out.println("Goodbye!");
                break;

            } else {
                System.out.println("Invalid choice. Please try again.\n");
            }
        }

        sc.close();
    }
}