import re

# Skrypt generujący wywołania funkcji dla programu TimeAnalysis.c

with open("../_filtercoeffs/filtercoeffs.h", "r") as f:
    lines = f.readlines()

output_lines = []
output_filename = "generated_calls_time.txt"

for line in lines:
    match = re.match(r'extern\s+(float|double)\s+(\w+)\[(\d+)\]\[(\d+)\];', line)
    if not match:
        continue

    dtype, name, dim1, dim2 = match.groups()
    dim1, dim2 = int(dim1), int(dim2)
    order = dim2 if "sos" in name else dim2  # dla BA i SOS wygląda na to, że liczba rzędów jest kluczowa

    # Domyślne wartości
    func_d = "NULL"
    func_f = "NULL"
    b_f = a_f = b_d = a_d = "NULL"
    sos_f = sos_d = "NULL"

    # Ustal typ struktury
    if "tdf2" in name:
        struct = "TDF2"
        func_name = "TDF2_f" if dtype == "float" else "TDF2_d"
    elif "df2" in name:
        struct = "DF2"
        func_name = "DF2_f" if dtype == "float" else "DF2_d"
    elif "df1" in name:
        struct = "DF1"
        func_name = "DF1_f" if dtype == "float" else "DF1_d"
    elif "cascade" in name:
        struct = "CASCADE"
        func_name = "CASCADE_f" if dtype == "float" else "CASCADE_d"
    else:
        continue

    # Generuj linijkę w zależności od typu filtra
    if "cascade" in name:
        # Wyciągnij order z nazwy, np. "butter_cascade_order4_f64_sos" → 4
        order_match = re.search(r'order(\d+)', name)
        if not order_match:
            continue
        order = int(order_match.group(1))
        sections = order // 2  # każda sekcja = biquad = 2 rzędy

        if dtype == "float":
            line = f'benchmark_cascade_and_log(fp, "{name}", "{dtype}", {func_name}, NULL, *{name}, NULL, {sections});'
        else:  # double
            line = f'benchmark_cascade_and_log(fp, "{name}", "{dtype}", NULL, {func_name}, NULL, *{name}, {sections});'
    else:
        if dtype == "float":
            b_f = f"{name}[0]"
            a_f = f"{name}[1]"
            func_f = func_name
        else:
            b_d = f"{name}[0]"
            a_d = f"{name}[1]"
            func_d = func_name

        line = f'benchmark_and_log(fp, "{name}", "{dtype}", "{struct}", {func_f}, {func_d}, {b_f}, {a_f}, {b_d}, {a_d}, {order});'

    output_lines.append(line)

with open(output_filename, "w") as f:
    f.write("\n".join(output_lines))

print(" Gotowe: poprawne wywołania wygenerowane w " + output_filename + ".")
