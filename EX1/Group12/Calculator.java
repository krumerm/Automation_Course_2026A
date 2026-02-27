import java.util.Scanner;

public class Calculator {

	public static void main(String[] args) {
		Scanner scanner = new Scanner(System.in);

		System.out.println("Type 'exit' at any time to quit.");

		while (true) {

			System.out.println("Choose start base:");
			System.out.println("2. I want to convert from Binary to Decimal");
			System.out.println("10. I want to convert from Decimal to Binary");

			String choice = getInput(scanner);
			//--------------------------------------------------------------------------------------

			if (choice.equals("2")) {

				String binary;

				// לולאה שקולטת בינארי עד שהוא תקין
				while (true) {
					System.out.print("Enter a binary number: ");
					binary = getInput(scanner);

					if (isBinary(binary)) {
						break;  // יציאה מהלולאה - הקלט תקין
					} else {
						System.out.println("Error: binary number can contain only 0 and 1");
					}
				}

				// פה הקלט כבר תקין מבצעים המרה
				int decimal = binaryToDecimal(binary);
				System.out.println("Result in decimal: " + decimal);
			}

			//------------------------------------------------------------------------		

			else if (choice.equals("10")) {

				String decimal;

				// לולאה שקולטת עשרוני עד שהוא תקין (כולל אפשרות למינוס בהתחלה)
				while (true) {
					System.out.print("Enter a decimal number: ");
					decimal= getInput(scanner);  
					
					if (isDecimal(decimal)) {
						break; 
					} else {
						System.out.println("Error: decimal number can contain only digits 0-9");
					}
				}

				// ההמרה עצמה
				String binary = decimalToBinary(decimal);
				System.out.println("Result in binary: " + binary);
			}
			
			//אם נכנס קלט שהוא לא 2 10 או יציאה
		      else {
	                System.out.println("Invalid choice. Please enter 2 or 10.");
	                continue; 
	            }
			
			//האם המשתמש רוצה להמשיך לאחר קבלת התוצאה הראשונה
			 while (true) {
	                System.out.println("Do you want to convert another number?");
	                System.out.print("please write yes/no");
	                String answer = getInput(scanner);

	                if (answer.equals("yes") || answer.equals("Yes") || answer.equals("YES")) {
	                    break;
	                } else if (answer.equals("no") || answer.equals("NO") || answer.equals("No")) {	                    System.out.println("program stopped");
	                    scanner.close();
	                    return; 
	                    
	                } else {
	                    System.out.println("Please type 'yes' or 'no'.");
	                }
	            }
		}
			
			}

			//------------------------------------------------------------------------------

			/// פונקציית קלט-בודקת אם משתמש הכניס יציאה ואם כן מוציאה אותו
			static String getInput(Scanner scanner) {
				String input = scanner.nextLine().trim();
				if (input.equals("exit") || input.equals("Exit") || input.equals("EXIT")) {
					System.out.println("program stopped");
					System.exit(0);
				}
				return input;
			}

			//פונקציות בינארי לעשרוני--------------------------------------------------------

			// בודקת אם המחרוזת היא מספר בינארי תקין: רק 0 ו-1, לפחות ספרה אחת
			static boolean isBinary(String s) {
				if (s.length() == 0) {// אם המחרוזת ריקה קלט לא תקין
					return false;
				}

				int startIndex = 0;

				// אם יש מינוס בהתחלה
				if (s.charAt(0) == '-') {
					if (s.length() == 1) {
						return false; //רק מינוס לא תקין
					}
					startIndex=1; // נתחיל לבדוק ספרות מהאינדקס 1
				}

				for (int i=startIndex; i < s.length(); i++) {
					char c = s.charAt(i);
					if (c!= '0' && c!= '1') {
						return false; // אם יש תו שהוא לא 0 או 1 לא תקין
					}
				}

				return true;
			}

			///המרת בינארי לעשרוני
			static int binaryToDecimal(String binary) {
				boolean isNegative = false;
				int result=0;
				int power=0; 

				// בדיקת סימן מינוס
				if (binary.charAt(0) == '-') {
					isNegative = true;
					binary = binary.substring(1); // עובדות רק על החלק החיובי
				}

				for (int i=binary.length()-1; i>=0; i--) {
					char c= binary.charAt(i);
					int digit=c-'0'; 

					result+= digit*Math.pow(2, power);

					power++; 
				}

				if (isNegative) {
					result = -result;
				}//אם הקלט היה שלילי מחזיר למינוס

				return result;
			}

			//פונקציות עשרוני לבינארי-------------------------------------------------------------
			// בודקת אם המחרוזת היא מספר עשרוני תקין רק ספרות 0-9 או מינוס בהתחלה, לפחות ספרה אחת
			static boolean isDecimal(String s) {
				if (s.length() == 0) {
					return false;
				}

				int startIndex = 0;

				if (s.charAt(0) == '-') {
					if (s.length() == 1) {
						return false; // רק "-" זה לא מספר
					}
					startIndex = 1;
				}//בדיקת שליליות

				for (int i=startIndex; i< s.length(); i++) {
					char c= s.charAt(i);
					if (c< '0' || c > '9') { // כל תו שלא בין '0' ל-'9' לא תקין
						return false;
					}
				}
				return true;
			}

			//המרת עשרוני לבינארי
			static String decimalToBinary(String decimal) {

				int number = Integer.parseInt(decimal); 
				if (number == 0) {
					return "0";
				}

				boolean isNegative = false;
				if (number < 0) {
					isNegative = true;
					number = -number; // עובדות עם הערך המוחלט
				}

				String result = "";

				while (number > 0) {
					int remainder = number % 2;        // שארית 0 או 1
					result = remainder + result;       // מוסיפים לתשובה את השארית (נוסיף בשמאל כל פעם כי אמורים לקרוא מלמטה למעלה
					number = number / 2;               // מחלקים ב2 לאיטרציה הבאה
				}

				if (isNegative) {
					result= "-"+ result;
				}

				return result;
			}
		}
