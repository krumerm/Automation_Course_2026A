package automation;

import java.util.Scanner;

public class BaseConverterInteractive {

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int choice;
        do {
            printMenu();
            while (!sc.hasNextInt()) {
                System.out.println("שגיאה: יש להקליד מספר שלם בלבד.");
                sc.next();
            }
            choice = sc.nextInt();
            handleChoice(choice, sc);
        } while (choice != 0);
        System.out.println("סיום התוכנית. להתראות!");
        sc.close();
    }

    // תפריט ראשי
    public static void printMenu() {
        System.out.println("\nבחר פעולה:");
        System.out.println("1 - המרה מבינארי לעשרוני");
        System.out.println("2 - המרה מעשרוני לבינארי");
        System.out.println("0 - יציאה");
        System.out.print("בחירה: ");
    }

    public static void handleChoice(int choice, Scanner sc) {
        switch (choice) {
            case 1 -> handleBinaryToDecimal(sc);
            case 2 -> handleDecimalToBinary(sc);
            case 0 -> {}
            default -> System.out.println("בחירה לא חוקית. נסה שוב.");
        }
    }

    //המרה מבינארי לעשרוני
    private static void handleBinaryToDecimal(Scanner sc) {
        System.out.print("הקלד מספר בינארי (רק 0 ו-1): ");
        String binary = sc.next();

        if (!binary.matches("[01]+")) {
            System.out.println("שגיאה: הקלט חייב להכיל רק 0 ו-1, ללא רווחים או תווים אחרים.");
            return;
        }

        // בדיקה שהקלט לא ארוך מדי
        if (binary.length() > 31) {
            System.out.println("שגיאה: מספר באורך מעל 31 ביטים חורג מטווח int.");
            return;
        }

        int bits = binary.length();
        try {
            int result = binaryToDecimalTwosComplement(binary, bits);
            System.out.println("תוצאה עשרונית: " + result);
        } catch (ArithmeticException e) {
            System.out.println("שגיאה: המספר חורג מטווח int. (" + e.getMessage() + ")");
        }
    }

    //  המרה מעשרוני לבינארי
    private static void handleDecimalToBinary(Scanner sc) {
        System.out.print("הקלד מספר עשרוני שלם (יכול להיות שלילי): ");
        if (!sc.hasNextInt()) {
            System.out.println("שגיאה: יש להזין מספר שלם בלבד (ללא נקודה עשרונית).");
            sc.next();
            return;
        }

        int num = sc.nextInt();
        int bits = calcRequiredBits(num);
        String result = decimalToTwosComplement(num, bits);

        System.out.println("ייצוג בינארי: " + result);
    }

    // חישוב מספר ביטים נדרש אוטומטית
    private static int calcRequiredBits(int n) {
        if (n == 0) return 1;
        int abs = Math.abs(n);
        int bits = (int) (Math.floor(Math.log(abs) / Math.log(2))) + 2;
        return bits;
    }

    // המרות לוגיות

    // בינארי לעשרוני לפי שיטת המשלים ל2
    public static int binaryToDecimalTwosComplement(String binary, int bits) {
        char signBit = binary.charAt(0);
        if (signBit == '0') {
            return binaryToDecimalSafe(binary);
        } else {
            int unsignedValue = binaryToDecimalSafe(binary);
            int shifted = (int) Math.pow(2, bits);
            int result = unsignedValue - shifted;
            return result;
        }
    }

    // עשרוני לבינארי לפי שיטת המשלים ל2 
    public static String decimalToTwosComplement(int n, int bits) {
        if (n >= 0)
            return addLeft(decimalToBinary(n), bits);

        int abs = Math.abs(n);
        String binary = addLeft(decimalToBinary(abs), bits);

        // היפוך ביטים
        StringBuilder inverted = new StringBuilder();
        for (char c : binary.toCharArray())
            inverted.append(c == '0' ? '1' : '0');

        // הוספת 1
        int decimal = binaryToDecimalSafe(inverted.toString());
        if (decimal == Integer.MAX_VALUE) {
            System.out.println("אזהרה: המספר קרוב לטווח המרבי של int.");
        }
        decimal = Math.addExact(decimal, 1); // זורק חריגה אם חורג
        String result = decimalToBinary(decimal);
        return addLeft(result, bits);
    }

    // המרה בינארי רגיל לעשרוני עם בדיקה שלא חורגים מגודל int
    public static int binaryToDecimalSafe(String binary) {
        int decimal = 0, power = 1;
        for (int i = binary.length() - 1; i >= 0; i--) {
            if (binary.charAt(i) == '1') {
                decimal = Math.addExact(decimal, power); // תופס גלישה
            }
            power = Math.multiplyExact(power, 2); // תופס גלישה
        }
        return decimal;
    }

    // המרה עשרוני רגיל לבינארי
    public static String decimalToBinary(int n) {
        if (n == 0) return "0";
        StringBuilder sb = new StringBuilder();
        while (n > 0) {
            sb.insert(0, n % 2);
            n /= 2;
        }
        return sb.toString();
    }

    // השלמת אפסים משמאל
    public static String addLeft(String str, int length) {
        while (str.length() < length) str = "0" + str;
        if (str.length() > length) str = str.substring(str.length() - length);
        return str;
    }
}
