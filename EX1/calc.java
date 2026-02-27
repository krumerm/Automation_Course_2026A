import java.util.Scanner;
public class calc {
	public static void main(String[] args) {
		Scanner scanner = new Scanner(System.in);
        System.out.println("Base Conversion Calculator (Decimal - Binary)");
        
        while (true) { // לולאה חיצונית לרפטטיביות
            int base = 0;
            while (true) { // שלב 1: קבלת הבסיס (2 או 10 בלבד)
                System.out.println("Enter the base you want to convert FROM (2 or 10), or 'exit' to quit:");
                String baseInput = scanner.next();
                
                if (baseInput.equalsIgnoreCase("exit")) {
                    System.out.println("Goodbye!");
                    scanner.close();
                    return;
                }
                
                if (baseInput.compareTo("2")== 0) {
                    base = 2;
                    break;
                }
                if (baseInput.compareTo("10")== 0) {
                    base = 10;
                    break;
                } 
                
                System.out.println("Invalid input! Base must be 2 or 10 only.");
            }
            
            // שלב 2: קבלת המספר להמרה
            String numberInput = "";
            while (true) {
                System.out.println("Enter the number you want to convert, or 'exit' to quit:");
                numberInput = scanner.next();
                
                if (numberInput.equalsIgnoreCase("exit")) {
                    System.out.println("Goodbye!");
                    scanner.close();
                    return;
                }
                
                if (isValidNumberForBase(numberInput, base)) {
                    break;
                }
                
                System.out.println("Invalid number! Please enter a valid number for base " + base + ".");
            }
            
            // שלב 3: ביצוע ההמרה
            if (base == 10) {
                int decimal = Integer.parseInt(numberInput);
                String binaryResult = decimalToBinary(decimal);
                System.out.println("The number " + numberInput + " in binary is: " + binaryResult);
            } 
            else if (base == 2) {
                int decimalResult = binaryToDecimal(numberInput);
                System.out.println("The number " + numberInput + " in decimal is: " + decimalResult);
            }
            
            // שלב 4: שאלה אם רוצים להמשיך
            while (true) {
                System.out.println("\nDo you want to perform another conversion? (yes/no):");
                String answer = scanner.next();
                
                if (answer.equalsIgnoreCase("yes") || answer.equalsIgnoreCase("y")) {
                    System.out.println(); // שורה ריקה להפרדה
                    break; // יציאה מהלולאה הפנימית וחזרה להתחלה
                } 
                else if (answer.equalsIgnoreCase("no") || answer.equalsIgnoreCase("n") || answer.equalsIgnoreCase("exit")) {
                    System.out.println("Goodbye!");
                    scanner.close();
                    return; // יציאה מהתוכנית
                }
                else {
                    System.out.println("Invalid input! Please enter 'yes' or 'no'.");
                }
            }
        }
    }
    
    // פונקציה לבדוק קלט מספר
    public static boolean isValidNumberForBase(String input, int base) {
        if (input == null || input.isEmpty()) {
        	return false;
        }
        // בדיקת סימן מינוס בתחילת המספר בלבד
        if (input.charAt(0) == '-') {
            if (input.length() == 1) {//*רק סימן מינוס
            	return false;
            }
            input = input.substring(1);
        }
        // רק ספרות
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            if (base == 10) {
            	if (c < '0' || c > '9') {
                    return false; // מצאנו תו שאינו מספר
            	}
            }
            if (base == 2) {
            	if (c != '0' && c != '1') {
                    return false; // מצאנו תו שאינו מתאים לבסיס בינארי 
            	}
            }
        }
        return true;
    }
    
    // פונקציה להמרה מעשרוני לבינארי
    public static String decimalToBinary(int n) {
    	if (n == 0) {
        	return "0";
        }
    	boolean isNegative = false;		
    	
        if (n < 0) {//אם המספר שלילי
        	isNegative = true;
        	n = -n;
        }
        
        String binary = "";
        while (n > 0) {
            int remainder = n % 2;
            binary = remainder + binary;
            n = n / 2;
        }
        if (isNegative) {
        	binary = "-" + binary;
        }
        return binary;
    }
    
    // פונקציה להמרה מבינארי לעשרוני
    public static int binaryToDecimal(String binary) {
        boolean isNegative = false;
        if (binary.charAt(0) == '-') {
            isNegative = true;
            binary = binary.substring(1);
        }
        int decimal = 0;
        int power = 0;
        for (int i = binary.length() - 1; i >= 0; i--) {
            int digit = binary.charAt(i) - '0';
            decimal += digit * Math.pow(2, power);
            power++;
        }
        if (isNegative) {
        	decimal = -decimal;
        }
        return decimal;
    }
}