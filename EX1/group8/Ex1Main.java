package EX1.group8; 
import java.util.Scanner;

public class Ex1Main {
	public static void main(String[] args) {
        
        BaseConvertor convertor = new BaseConvertor(); 
        
        Scanner scanner = new Scanner(System.in);
        while (true) {
            System.out.println("מחשבון המרות: בינארי <-> עשרוני");
            System.out.println("אנא בחר את הפעולה הרצויה:");
            System.out.println("1. המרה מבינארי לעשרוני");
            System.out.println("2. המרה מעשרוני לבינארי");
            System.out.println("3. יציאה"); 
            System.out.println("הכנס 1, 2 או 3");

            String choice = scanner.nextLine();
            try {
                if (choice.equals("1")) {
                    System.out.println(" בדיקת בינארי -> עשרוני");
                    System.out.println("הכנס מספר בינארי ");
                    String binInput = scanner.nextLine();
                    String decOutput = convertor.binaryToDecimal(binInput);
                    System.out.println("התוצאה העשרונית: " + decOutput);

                } else if (choice.equals("2")) {
                    System.out.println("בדיקת עשרוני -> בינארי");
                    System.out.println("הכנס מספר עשרוני ");
                    String decInput = scanner.nextLine();
                    String binOutput = convertor.decimalToBinary(decInput);
                    System.out.println("התוצאה הבינארית: " + binOutput);
                    
                } else if (choice.equals("3")) {
                    System.out.println("יוצא מהמחשבון");
                    break; 
                    
                } else {
                    System.err.println("בחירה לא חוקית. אנא הזן 1, 2 או 3");
                }

            } catch (IllegalArgumentException e) {
                // תפיסת שגיאות קלט (כמו "102" בבינארי או "abc" בעשרוני)
                System.err.println("שגיאת קלט: " + e.getMessage());
            } 
        } 
        scanner.close();
        System.out.println("מחשבון נסגר");
    }
}