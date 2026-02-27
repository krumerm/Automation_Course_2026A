def decimal_to_binary(num, precision=10, bits=8):

    # בדיקת קלט תקין
    if not isinstance(num, (int, float)):
        print("נא להכניס מספרים בלבד, ניתן להכניס מספרים עשרוניים ושליליים")
        return None

    # טיפול במקרים של אפס
    if num == 0:
        return "0" * bits

    # טיפול במספרים שליליים – משלים ל-1
    if num < 0:
        abs_num = -num
        binary_abs = decimal_to_binary(abs_num, precision, bits)
        inverted = ''.join('1' if b == '0' else '0' for b in binary_abs.split('.')[0])
        return inverted

    # חלק שלם וחלק שברי
    integer_part = int(num)
    fractional_part = num - integer_part

    # המרת החלק השלם
    integer_binary = ""
    while integer_part > 0:
        integer_binary = str(integer_part % 2) + integer_binary
        integer_part //= 2

    # השלמת אפסים משמאל לגודל הביטים
    integer_binary = integer_binary.zfill(bits)

    # המרת החלק השברי (אם יש)
    if fractional_part > 0:
        fractional_binary = "."
        count = 0
        while fractional_part > 0 and count < precision:
            fractional_part *= 2
            bit = int(fractional_part)
            fractional_binary += str(bit)
            fractional_part -= bit
            count += 1
        return integer_binary + fractional_binary
    else:
        return integer_binary

def binary_to_decimal(binary_str):

    if not isinstance(binary_str, str):
        return None

    allowed_chars = {'0', '1', '.'}
    if not set(binary_str).issubset(allowed_chars):
        print("קלט לא חוקי – מותר להזין רק את הספרות 0 ו-1, ונקודה")
        return None

    if binary_str.count('.') > 1:
        print("קלט לא חוקי – ניתן להשתמש בנקודה אחת בלבד")
        return None

    if binary_str == " " or binary_str == ".":
        print("קלט לא חוקי – הזן מספר בינארי תקין")
        return None

    if '.' in binary_str:
        int_part_str, frac_part_str = binary_str.split('.')
    else:
        int_part_str, frac_part_str = binary_str, ""

    decimal_value = 0
    for i, digit in enumerate(reversed(int_part_str)):
        decimal_value += int(digit) * (2 ** i)

    for i, digit in enumerate(frac_part_str, start=1):
        decimal_value += int(digit) * (2 ** -i)

    return decimal_value

while True:
    print("\nבחר פעולה:")
    print("1. המרת מספר בינארי לעשרוני")
    print("2. המרת מספר עשרוני לבינארי")
    print("3. יציאה מהמשחק")

    choice = input("הכנס בחירה (1/2/3): ")

    if choice == "1":
        binary_str = input("הכנס מספר בינארי חיובי (ניתן לכלול נקודה עשרונית): ")
        result = binary_to_decimal(binary_str)
        if result is not None:
            print("התוצאה העשרונית היא:", result)

    elif choice == "2":
        dec_input = input("הכנס מספר עשרוני (ניתן מספרים עשרוניים ושליליים): ")
        try:
            dec_num = float(dec_input)
        except ValueError:
            print("נא להכניס מספרים בלבד, ניתן להכניס מספרים עשרוניים ושליליים")
            continue

        result = decimal_to_binary(dec_num)
        if result is not None:
            print("הייצוג הבינארי הוא:", result)

    elif choice == "3":
        print("להתראות, נשמח לראותך שוב!")
        break

    else:
        print("בחירה לא חוקית. נא לבחור 1 2 או 3.")