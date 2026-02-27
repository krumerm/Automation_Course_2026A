import java.util.*;
public class Targil1 {
	static Scanner sc = new Scanner(System.in);
	public static void main(String[] args) {
		String choice="";
		while(true) {
			System.out.println("1. Convert Binary to Decimal \n2. Convert Decimal to Binary \n0. For exit ");
			choice=sc.next();
			switch(choice) {
			case "1":// binary to decimal
				System.out.println("Enter a binary number");
				sc.nextLine(); 
				String number1 = sc.nextLine().trim();	
				while (!number1.matches("[01]+")) {//valid check
					if (number1.contains(" ")) { 
						System.out.println("Invalid binary number (no spaces allowed). Try again:");
					} else {
						System.out.println("Invalid binary number. Try again:");
					}
					number1 = sc.nextLine().trim(); 
				}
				System.out.println( binaryToDecimal(number1));
				continue;
			case "2"://decimal to binary
				System.out.println("Enter a decimal number");
				sc.nextLine();
				while(true) {
					String input = sc.nextLine().trim();
					if (input.contains(" ")) {
						System.out.println("Invalid decimal number (no spaces allowed). Try again:");
						continue;
					}
					try {
						int number2 = Integer.parseInt(input);
						if (number2 < 0) {
							System.out.println("Invalid decimal number. Try again:");
							continue; 
						}
						System.out.println(decimalToBinary(number2));
						break;
					} catch (NumberFormatException  e) {//valid check
						System.out.println("Invalid decimal number. Try again:");
					}
				}
				continue;
			case "0":
				System.out.println("Goodbye");
				break;
			default:
				System.out.println("Wrong input please try again:");
				continue;
			}
			break;
		}
	}	
	public static int binaryToDecimal(String binary) {// binary to decimal
		int decimal = 0;
		int power = 0;
		for (int i = binary.length() - 1; i >= 0; i--) {
			char bit = binary.charAt(i);
			if (bit == '1') {
				decimal += Math.pow(2, power);
			}
			power++;
		}

		return decimal;
	}
	public static String decimalToBinary(int decimal) {//decimal to binary
		if (decimal == 0) {
			return "0";
		}
		String binary = "";
		while (decimal > 0) {
			int remainder = decimal % 2;   
			binary = remainder + binary;   
			decimal = decimal / 2;         
		}

		return binary;
	}
}