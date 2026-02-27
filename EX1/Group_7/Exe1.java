package aaa;

import java.util.Scanner;

public class Exe1 {
	// Function to convert a binary number to a decimal number
	public static int binaryToDecimal(String binary) {
		int decimalValue  = 0;
		int power = 1; 
		 // Go through the binary number from right to left
		for (int i = binary.length() - 1; i >= 0; i--) {
			char bit = binary.charAt(i);
			if (bit == '1') {
				decimalValue  += power;
			} else if (bit != '0') {
				System.out.println("Error: Invalid binary input (only 0 and 1 are allowed).");
				return -1;
			}
			power *= 2;
		}

		return decimalValue ;
	}

	// Function to convert a decimal number to a binary string
	public static String decimalToBinary(int number) {
		if (number == 0) 
			return "0";

		String result = "";

		while (number > 0) {
			int remainder = number % 2;
			result = remainder + result; 
			number = number / 2;
		}

		return result;
	}

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		int choice;

		System.out.println("Binary - Decimal Base Conversion Calculator");
		 // repeat until the user chooses to exit
		do {
			System.out.println("\nSelect an action:");
			System.out.println("1. Convert from Binary to Decimal");
			System.out.println("2. Convert from Decimal to Binary");
			System.out.println("0. Exit the calculator");
			System.out.print("Your choice: ");

			choice = input.nextInt();
			input.nextLine(); 
			
			if (choice == 1) {  // Binary to Decimal conversion
				System.out.print("Enter a binary number: ");
				String binary = input.nextLine();
				int decimal = binaryToDecimal(binary);
				if (decimal != -1)
					System.out.println("Result in decimal base: " + decimal);

			} else if (choice == 2) {  // Decimal to Binary conversion
				System.out.print("Enter a decimal number: ");
				int decimal = input.nextInt();
				String binary = decimalToBinary(decimal);
				System.out.println("Result in binary base: " + binary);

			} else if (choice == 0) {
				System.out.println("Exiting calculator... Goodbye!");

			} else {
				System.out.println("Invalid choice, try again.");
			}


		} while (choice != 0); // continue until user chooses 0

		input.close();
	}
}

