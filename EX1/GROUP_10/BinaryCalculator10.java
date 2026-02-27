import java.util.Scanner;

public class BinaryCalculator10
{
    public static void main(String[] args) {
        Scanner input = new Scanner(System.in);
        boolean run = true;

        System.out.println("Welcome to the Base Converter!");
        //תפריט
        while (run) {
            System.out.println("\nChoose an option:");
            System.out.println("1. Convert from binary to decimal");
            System.out.println("2. Convert from decimal to binary");
            System.out.println("3. Exit");

            System.out.print("Enter your choice: ");
            String choice = input.nextLine();

            switch (choice) {

                //המרת מספר מבסיס בינארי לעשרוני
                case "1":
                    System.out.print("Enter a binary number: ");
                    String binary = input.nextLine();
                    if (isBinary(binary)) {
                        int decimal = binaryToDecimal(binary);
                        System.out.println("Decimal value: " + decimal);
                    } else {
                        System.out.println("Invalid input! A binary number must contain only 0 and 1.");
                    }
                    break;

                //המרת מספר מבסיס עשרוני לבינארי
                case "2":
                    System.out.print("Enter a decimal number: ");
                    String decStr = input.nextLine();
                    if (isInteger(decStr)) {
                        int dec = Integer.parseInt(decStr);
                        System.out.println("Binary value: " + decimalToBinary(dec));
                    } else {
                        System.out.println("Invalid input! Please enter a valid integer.");
                    }
                    break;

                //יציאה מהתוכנית
                case "3":
                    System.out.println("Goodbye!");
                    run = false;
                    break;

                //ברירת מחדל
                default:
                    System.out.println("Invalid choice. Try again.");
            }
        }

        input.close();
    }

    //בודקת את תקינות הקלט בcase1
    public static boolean isBinary(String s) {
        if (s.isEmpty()) return false;
        for (char c : s.toCharArray()) {
            if (c != '0' && c != '1') return false;
        }
        return true;
    }

    //  בודקת אם קלט הוא מספר שלם (חיובי או שלילי)
    public static boolean isInteger(String s) {
        if (s.isEmpty()) return false;
        if (s.charAt(0) == '-') s = s.substring(1); //אם המספר שלילי, אז נתייחס אליו כחיובי למען בדיקת הקלט
        for (char c : s.toCharArray()) {
            if (!Character.isDigit(c)) return false;
        }
        return true;
    }

    //ממירה את המספר הבינארי לעשרוני (בשיטת בגודל וסימן)
    public static int binaryToDecimal(String binary) {
        int result = 0;
        boolean isNegative;
        //בדיקה האם המספר הבינארי חיובי או שלילי
        if (binary.charAt(0) == '1') {
            isNegative = true;
        } else {
            isNegative = false;
        }
        // קביעת הערך המוחלט (החלק שאחרי ביט הסימן)
        String size;
        if (isNegative) {
            size = binary.substring(1);
        } else {
            size = binary;
        }
        //המרה
        for (int i = 0; i < size.length(); i++) {
            char bit = size.charAt(size.length() - 1 - i);
            if (bit == '1') {
                result += Math.pow(2, i);
            }
        }
        //הוספת הסימן למספר העשרוני לאחר ההמרה
        return isNegative ? -result : result;
    }

    //ממירה את המספר העשרוני לבינארי (לפי שיטת גודל וסימן)
    public static String decimalToBinary(int num) {
        if (num == 0) return "0";
        // בדיקה האם המספר שלילי
        boolean isNegative;
        if (num < 0) {
            isNegative = true;
        } else {
            isNegative = false;
        }
        int absNum = Math.abs(num);//המספר בערך מוחלט

        String result = ""; // אתחול מחרוזת שתכיל את התוצאה הבינארית

        while (absNum > 0) {
            int bit = absNum % 2; // הוספת הספרה הבינארית (שארית החלוקה מ-2)
            result = bit + result;
            absNum = absNum / 2; // המספר החדש לאיטרציה הבאה
        }
        // הוספת ביט סימן בהתאם
        if (isNegative) {
            result = "1" + result; // סימן שלילי
        } else {
            result = "0" + result; // סימן חיובי
        }
        return result;
    }
}
