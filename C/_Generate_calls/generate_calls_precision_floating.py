import re
from collections import defaultdict

# Skrypt generujący wywołania funkcji dla programu PrecisionAnalysis.c

with open("../_filtercoeffs/filtercoeffs.h", "r") as f:
    lines = f.readlines()

output_filename = "generated_calls_precision.txt"

# Grupuj po wspólnej nazwie (bez "_f32"/"_f64")
groups = defaultdict(dict)

pattern = re.compile(r'extern\s+(float|double)\s+(\w+)\[(\d+)\]\[(\d+)\];')

for line in lines:
    match = pattern.match(line)
    if not match:
        continue

    dtype, name, dim1, dim2 = match.groups()
    dim1, dim2 = int(dim1), int(dim2)

    if "cascade" in name and "_sos" not in name:
        continue  # filtrujemy błędne

    base_name = name.replace("_f32", "").replace("_f64", "")
    groups[base_name][dtype] = name

output_lines = []

for base_name, variants in groups.items():
    dtype_f = variants.get("float")
    dtype_d = variants.get("double")

    # Rozpoznaj typ struktury i funkcji
    if "tdf2" in base_name:
        struct = "TDF2"
        func_f = "TDF2_f"
        func_d = "TDF2_d"
    elif "df2" in base_name:
        struct = "DF2"
        func_f = "DF2_f"
        func_d = "DF2_d"
    elif "df1" in base_name:
        struct = "DF1"
        func_f = "DF1_f"
        func_d = "DF1_d"
    elif "cascade" in base_name:
        struct = "CASCADE"
        func_f = "CASCADE_f"
        func_d = "CASCADE_d"
    else:
        continue

    # Wyciągnij order
    order_match = re.search(r'order(\d+)', base_name)
    if not order_match:
        continue
    order = int(order_match.group(1))

    if struct == "CASCADE":
        sections = order // 2
        line = (
            f'precision_analysis_cascade(fp_precision, "{base_name}", {func_f}, {func_d}, '
            f'(float*){dtype_f}, (double*){dtype_d}, {sections});'
        )
    else:
        line = (
            f'precision_analysis(fp_precision, "{base_name}", "{struct}", {func_f}, {func_d}, '
            f'{dtype_f}[0], {dtype_f}[1], {dtype_d}[0], {dtype_d}[1], {order + 1});'
        )

    output_lines.append(line)

with open(output_filename, "w") as f:
    f.write("\n".join(output_lines))

print("✅ Gotowe: wygenerowano wspólne wywołania precision_analysis i precision_analysis_cascade do" + output_filename + ".")
