def binary_to_decimal(binary_str):
    # --- בדיקת טיפוס ---
    if not isinstance(binary_str, str):
        raise TypeError("הקלט חייב להיות מסוג מחרוזת (string).")

    # --- קלט ריק ---
    if binary_str == "":
        raise ValueError("קלט בינארי לא תקין: המחרוזת ריקה.")

    # --- רק '0' או '1' ---
    if not all(c in '01' for c in binary_str):
        raise ValueError(f"קלט בינארי לא תקין: '{binary_str}'. חייב להכיל רק 0 או 1.")

    decimal_value = 0
    power = 0

    # נרוץ על המחרוזת מהסוף להתחלה
    for i in range(len(binary_str) - 1, -1, -1):
        digit = int(binary_str[i])
        decimal_value += digit * (2 ** power)
        power += 1

    return decimal_value


def decimal_to_binary(decimal_num):
    # --- בדיקת טיפוס ---
    if not isinstance(decimal_num, int):
        raise TypeError("הקלט חייב להיות מספר שלם (int).")

    # --- רק מספרים חיוביים ---
    if decimal_num < 0:
        raise ValueError("הקלט חייב להיות מספר שלם חיובי (>= 0).")

    # מקרה 0
    if decimal_num == 0:
        return "0"

    binary_str = ""

    while decimal_num > 0:
        remainder = decimal_num % 2
        binary_str = str(remainder) + binary_str
        decimal_num //= 2

    return binary_str


def main():
    print("=== Base Conversion Calculator ===")

    while True:
        print("\nChoose an option:")
        print("1 - Binary to Decimal")
        print("2 - Decimal to Binary")
        print("0 - Exit")

        choice = input("Enter your choice: ").strip()

        # ---- אופציה 1: המרת בינארי לעשרוני ----
        if choice == "1":
            binary_str = input("Enter a binary number: ").strip()

            try:
                result = binary_to_decimal(binary_str)
                print("Decimal value:", result)
            except Exception as e:
                print("Error:", e)

        # ---- אופציה 2: המרת עשרוני לבינארי ----
        elif choice == "2":
            decimal_input = input("Enter a decimal number: ").strip()

            try:
                decimal_num = int(decimal_input)  # יזרוק ValueError אם לא מספר
                result = decimal_to_binary(decimal_num)
                print("Binary value:", result)
            except ValueError:
                print("Error: please enter a valid positive integer.")
            except Exception as e:
                print("Error:", e)

        # ---- יציאה ----
        elif choice == "0":
            print("Calculator closed. Goodbye!")
            break

        # ---- קלט תפריט לא חוקי ----
        else:
            print("Invalid choice, please try again.")


# הפעלה רק בקובץ הראשי – חובה לטסטים
if __name__ == "__main__":
    main()
