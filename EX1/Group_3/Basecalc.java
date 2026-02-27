//TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.

import java.util.InputMismatchException;
import java.util.Scanner;

public class Basecalc {
    static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        boolean running = true;

        while (running) {
            System.out.println("=== Conversion Calculator ===");
            System.out.println("1 - Binary to Decimal");
            System.out.println("2 - Decimal to Binary");
            System.out.println("3 - Exit");
            System.out.print("Your choice: ");

            int choice;

            // בדיקת קלט תפריט
            try {
                choice = scanner.nextInt();   // קלט ישיר כ-int
                scanner.nextLine();            // לצרוך את ה-newline שנשאר אחרי nextInt()
            } catch (InputMismatchException e) {
                System.out.println("Invalid input. Please enter 1, 2 or 3.");
                System.out.println();
                scanner.nextLine();
                continue;
            }
            if (choice != 1 && choice != 2 && choice != 3) {
                System.out.println("Invalid choice. Please enter 1, 2, or 3.");
                System.out.println();
                continue;
            }

            if (choice == 1) {
                //בינארי לעשרוני
                boolean valid = false;
                while (!valid) {
                    System.out.print("Enter a binary number (for example: 101101): ");
                    String binary = scanner.nextLine();

                    if (!binary.matches("[01]+")) {
                        System.out.println("Invalid input. Please use only 0s and 1s.");
                        System.out.println();
                        continue;
                    }

                    int bits = binary.length();
                    int decimal;

                    // אם הביט השמאלי הוא 1 מפרשים כשלילי
                    if (binary.charAt(0) == '1') {
                        int value = binaryToDecimalManual(binary); //ממיר את המחרוזת לעשרוני רגיל
                        int maxValue = 1;
                        for (int i = 0; i < bits; i++) {
                            maxValue *= 2; // 2^bits חישוב
                        }
                        decimal = value - maxValue; // לפי שיטת משלים ל2
                    } else {
                        decimal = binaryToDecimalManual(binary); // חיובי רגיל
                    }

                    System.out.println("The result in decimal base: " + decimal);
                    System.out.println();
                    valid = true;
                }

            } else if (choice == 2) {
                // עשרוני לבינארי
                boolean valid = false;
                while (!valid) {
                    System.out.print("Enter a decimal number (for example: 45 or -7): ");
                    System.out.println("(Value range - [-2,147,483,648,...,2,147,483,648])");
                    int decimal;
                    try {
                        decimal = scanner.nextInt();   // קלט ישיר כ-int
                        scanner.nextLine();            // לצרוך את ה-newline שנשאר אחרי nextInt()
                    } catch (InputMismatchException e) {
                        System.out.println("Invalid input. Please enter a valid integer number.");
                        System.out.println();
                        scanner.nextLine();
                        continue;
                    }

                    String binary;
                    if (decimal < 0) {
                        int bits = 32; // כמות ביטים מקסימלי באינטג'ר
                        binary = decimalToTwosComplementManual(decimal, bits);
                    } else {
                        binary = decimalToBinaryManual(decimal); // מספר חיובי מעשרוני לבינארי
                    }

                    System.out.println("Binary: " + binary);
                    System.out.println();
                    valid = true;
                }

            } else if (choice == 3) {  //יציאה מהערכת והפסקת הלולאה הראשית
                System.out.println("Exiting program... Goodbye!");
                running = false;
            }
        }

        scanner.close();
    }

    // המרה ממחרוזת בינארית למספר עשרוני
    public static int binaryToDecimalManual(String binary) {
        int result = 0;
        int power = 1;

        for (int i = binary.length() - 1; i >= 0; i--) {
            int bit = binary.charAt(i) - '0';
            result += bit * power; // מוסיפים לתוצאה את הערך של הביט הנוכחי כפול 2^מיקום
            power *= 2; // מכפילים את הערך של החזקה פי 2 ואז מתקדמים ספרה שמאלה
        }
        return result;
    }

    // המרה ממספר עשרוני לבינארי (חיובי)
    public static String decimalToBinaryManual(int num) {
        if (num == 0) return "0";

        String result = "";
        while (num > 0) {
            int left = num % 2;
            result = left + result;
            num /= 2;  // כל פעם מוסיפים את השארית ומחלקים ב-2
        }
        return result;
    }

    // המרה לפי שיטת המשלים ל-2 (לשליליים בלבד)
    public static String decimalToTwosComplementManual(int num, int bits) {
        int positive = -num; // הופכים את המספר לשלילי לחיובי
        String binary = decimalToBinaryManual(positive); // ממירים את הערך החיובי לבינארי רגיל

        // השלמה באפסים לשמאל עד לגודל הביטים
        while (binary.length() < bits) {
            binary = "0" + binary; // מוסיפים אפסים משמאל עד שהאורך שווה למספר הביטים המבוקש
        }

        // היפוך ביטים
        char[] inverted = new char[binary.length()]; // מערך חדש לשמירת הביטים ההפוכים
        for (int i = 0; i < binary.length(); i++) {
            inverted[i] = (binary.charAt(i) == '0') ? '1' : '0'; // כל 0 הופך ל-1 וכל 1 הופך ל-0
        }

        // הוספת 1 מהביט הימני ביותר
        for (int i = bits - 1; i >= 0; i--) {
            if (inverted[i] == '0') { // אם הביט הוא 0
                inverted[i] = '1'; // נהפוך אותו ל-1
                break; // נעצור כי הוספת 1 הסתיימה
            } else {
                inverted[i] = '0'; // אם הביט הוא 1 – נהפוך ל-0 ונמשיך שמאלה
            }
        }

        // בניית מחרוזת ידנית שתייצג את התוצאה הסופית כמחרוזת
        String result = "";
        for (int i = 0; i < inverted.length; i++) {
            result += inverted[i]; // נוסיף כל ביט למחרוזת
        }

        return result;
   }
}
