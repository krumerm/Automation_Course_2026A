package a;
import java.util.Scanner;

public class binarydecimalycalc {
	
	//calculate decimal number from any base
	public static int toDecimal(String num, int fromBase) {
	    int sum = 0;
	    int power = 0; 

	    // start from right to left
	    for (int i = num.length() - 1; i >= 0; i--) {
	        int digit = charToNum(num.charAt(i)); // transfer from char to int

	        // check if valid- to its not a number or not in the wanted base
	        if (digit < 0 || digit >= fromBase) {
	            throw new IllegalArgumentException(
	                "Illegal digit for base " + fromBase + ": " + num.charAt(i)
	            );
	        }

	        // changing to binary num
	        sum += digit * (int) Math.pow(fromBase, power);
	        power++; // make the powre bigger every round
	    }

	    return sum;
	}
	
	// calculate decimal with fraction
	public static double toDecimalWithFraction(String num, int fromBase) {
	    int point = num.indexOf('.');
	    
	    //if there isn't point send to toDecimal
	    if (point == -1) {
	        return toDecimal(num, fromBase);
	    }

	    //take the whole part
	    String wholePart = num.substring(0, point);
	    
	  //if there isn't whole part than 0 else sent to toDecimal
	    int whole = wholePart.isEmpty() ? 0 : toDecimal(wholePart, fromBase); 
	    
	    
	    // calculate negative pow
	    double frac = 0.0;
	    double base = fromBase; 
	    for (int i = point + 1; i < num.length(); i++) {
	        int d = charToNum(num.charAt(i)); //changing char to num
	        if (d < 0 || d >= fromBase) {
	            throw new IllegalArgumentException(
	                "Illegal digit for base " + fromBase + ": " + num.charAt(i)
	            );
	        }
	     // create the num by divide the base in the negative pow by the index
	        frac += d / base; 
	        base *= fromBase; //like negative pow
	    }

	    return whole + frac;
	}
	//maxFracDigits= how many digits after dot
	// transfer to binary  digit
	public static String toBinary(String num, int fromBase, int maxFracDigits) {
	    // changing from any base to decimal
	    double decimal = toDecimalWithFraction(num, fromBase);

	    // separate to whole and fraction
	    int whole = (int) decimal;
	    double frac = decimal - whole;

	    // change the whole part to binary
	    String binaryWhole = "";
	    if (whole == 0) {
	        binaryWhole = "0";
	    } else {
	        while (whole > 0) {
	            binaryWhole = (whole % 2) + binaryWhole; //make sure the num is in the right order
	            whole /= 2;  // divide the number by 2 to move to the next binary digit
	        }
	    }

	    // change the fraction part to binary
	    if (frac == 0.0) {
	        return binaryWhole; // no fraction
	    }

	    StringBuilder sb = new StringBuilder(); //Used to create a string dynamically without creating new String objects each time
	    sb.append(binaryWhole).append('.'); // Adds the whole part and a decimal point
	    int count = 0;
	    while (frac > 0 && count < maxFracDigits) { //make sure the maximum number of digits is reached
	        frac *= 2; //need to multiply because its fraction
	        if (frac >= 1) { //checks if there is an integer part(whole part)
	            sb.append('1'); //add 1 to the string
	            frac -= 1; //minus 1 in the fraction
	        } else {
	            sb.append('0'); 
	        }
	        count++;
	    }
	    return sb.toString(); //can't return just the sb so toString
	}

	// if the user don't want to enter max digit so default 10 digits
	public static String toBinary(String num, int fromBase) {
	    return toBinary(num, fromBase, 10);
	}


	//transfer char to num
	private static int charToNum(char c) {
	    if (c >= '0' && c <= '9') return c - '0';
	    if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
	    if (c >= 'a' && c <= 'z') return 10 + (c - 'a');
	    return -1;
	}

    
	public static void main(String[] args) {
		Scanner scanner = new Scanner(System.in);
	    boolean flag = true; // flag=Controls program repetition

	    while (flag) {

	        //Menu 
	        System.out.println("\n--- Base Conversion Calculator ---");
	        System.out.println("1 - Binary → Decimal");
	        System.out.println("2 - Decimal → Binary");
	        System.out.print("Enter your choice (1/2): ");

	        String choice = scanner.nextLine().trim();

	        // Validate menu choice (must be 1 or 2). If invalid than ask again.
	        //checks negative and invalid chars
	        if (!choice.equals("1") && !choice.equals("2")) {
	            System.out.println("Invalid input. Please enter 1 or 2.");
	            continue; // Restart loop without asking to continue
	        }

	        try {
	            if (choice.equals("1")) 
	            	choiceOne();

	             else {
	                choiceTwo();
	            }

	        } catch (Exception e) {
	            // Handles invalid number format or invalid conversion inputs
	            System.out.println("Error: " + e.getMessage());
	        }

	        // Ask user if they want to perform another conversion
	        String answer;
	        while (true) {
	            System.out.print("\nDo you want to continue? (yes/no): ");
	            answer = scanner.nextLine().trim().toLowerCase();

	            if (answer.equals("yes")) {
	                break; // loop again
	            } else if (answer.equals("no")) {
	                flag = false;
	                System.out.println("Goodbye!");
	                break; // exit
	            } else {
	                System.out.println("Invalid input. Please answer yes or no.");
	            }
	        }
	    }

	    scanner.close(); // Close scanner to free resources
	}
	//choice1
	public static void choiceOne(){
		Scanner scanner = new Scanner(System.in);
		System.out.print("Enter a binary number (you may include a dot like 101.11): ");
		String binaryInput = scanner.nextLine().trim();

		// If input contains a dot than use the fractional converter 
		if (binaryInput.contains(".")) {
			double result = toDecimalWithFraction(binaryInput, 2); // base=2
			System.out.println("Decimal result: " + result);
		} else {
			// No dot so use the integer converter 
			int result = toDecimal(binaryInput, 2); // base=2
			System.out.println("Decimal result: " + result);
		}	
	}
	//choice2
	public static void choiceTwo() {
		 Scanner scanner = new Scanner(System.in);
		// Decimal to Binary 
        // Read as STRING so we can support fraction like 13.625 (not only int)
		 System.out.print("Enter a decimal number (you may include a dot, like 13.625): ");
		 String decimalInput = scanner.nextLine().trim();

		 String result = toBinary(decimalInput, 10); // fromBase=10
		 System.out.println("Binary result: " + result);
	}
	}