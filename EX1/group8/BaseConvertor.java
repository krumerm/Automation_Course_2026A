package EX1.group8; 

import java.math.BigInteger;



public class BaseConvertor {



    public String binaryToDecimal(String binaryInput) {

        if (binaryInput == null || binaryInput.trim().isEmpty()) {

            throw new IllegalArgumentException("קלט בינארי לא יכול להיות ריק");

        }

        String s = binaryInput.trim();



        // בודק אם יש סימן מינוס בהתחלה

        boolean isNegative = s.startsWith("-");

        if (isNegative) {

            s = s.substring(1); // מסיר את הסימן לבדיקה

        }



        if (!s.matches("^[01]+$")) {

            throw new IllegalArgumentException("קלט בינארי לא חוקי: מותר רק 0/1 (ואפשר סימן מינוס בהתחלה)");

        }



        BigInteger val = new BigInteger(s, 2);

        if (isNegative) val = val.negate(); // משחזר את הסימן

        return val.toString(10);

    }



    public String decimalToBinary(String decimalInput) {

        if (decimalInput == null || decimalInput.trim().isEmpty()) {

            throw new IllegalArgumentException("קלט עשרוני לא יכול להיות ריק");

        }

        String s = decimalInput.trim();



        // בדיקת סימן

        boolean isNegative = s.startsWith("-");

        if (isNegative) {

            s = s.substring(1);

        }



        if (!s.matches("^[0-9]+$")) {

            throw new IllegalArgumentException("קלט עשרוני לא חוקי: מותר רק ספרות 0-9 (ואפשר סימן מינוס בהתחלה)");

        }



        BigInteger val = new BigInteger(s, 10);

        if (val.signum() == 0) return "0";



        String binary = val.toString(2);

        if (isNegative) binary = "-" + binary;

        return binary;

    } 

}