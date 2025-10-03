import re

# Skrypt generujący wywołania funkcji dla programu PrecisionAnalysis_fixed.c

INPUT_FILE = "../_filtercoeffs/filtercoeffs.c"
OUTPUT_FILE = "generated_calls_precision_fixed.txt"

with open(INPUT_FILE, "r") as f:
    content = f.read()

# Wzorce dopasowania dla współczynników
ba_pattern = re.compile(r"double\s+(\w+)_f64_ba\s*\[\d+\]\[\d+\]\s*=")
sos_pattern = re.compile(r"double\s+(\w+)_f64_sos\s*\[\d+\]\[\d+\]\s*=")

# Generowanie wywołań precision_analysis_q (jednoliniowe)
def generate_precision_ba(name, order):
    struct_match = re.search(r"_(df1|df2|tdf2)_", name)
    structure = struct_match.group(1).upper() if struct_match else "UNKNOWN"
    call_q24 = f'precision_analysis_q(fp_precision, "{name}", "{structure}", 24, (void (*)(void*, void*, void*, void*, int, int)){structure}_q24, {structure}_d, {name}_f64_ba[0], {name}_f64_ba[1], {order + 1});'
    call_q511 = f'precision_analysis_q(fp_precision, "{name}", "{structure}", 12, (void (*)(void*, void*, void*, void*, int, int)){structure}_q12, {structure}_d, {name}_f64_ba[0], {name}_f64_ba[1], {order + 1});'
    return call_q24 + "\n" + call_q511

# Generowanie wywołań precision_analysis_sos (jednoliniowe)
def generate_precision_sos(name, sections):
    call_q24 = f'precision_analysis_sos(fp_precision, "{name}", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, {name}_f64_sos, {sections});'
    call_q511 = f'precision_analysis_sos(fp_precision, "{name}", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, {name}_f64_sos, {sections});'
    return call_q24 + "\n" + call_q511

# Zbieranie wszystkich BA
ba_calls = []
for match in ba_pattern.finditer(content):
    name = match.group(1)
    order_match = re.search(r"_order(\d+)_", name)
    order = int(order_match.group(1)) if order_match else 0
    ba_calls.append(generate_precision_ba(name, order))

# Zbieranie wszystkich SOS
sos_calls = []
for match in sos_pattern.finditer(content):
    name = match.group(1)
    order_match = re.search(r"_order(\d+)_", name)
    order = int(order_match.group(1)) if order_match else 0
    sections = order // 2
    sos_calls.append(generate_precision_sos(name, sections))

# Tworzenie pliku wyjściowego
output = "// === AUTO-GENERATED PRECISION ANALYSIS CALLS ===\n\n"
output += "// --- DF1 / DF2 / TDF2 ---\n\n"
output += "\n".join(ba_calls)
output += "\n\n// --- CASCADE (SOS) ---\n\n"
output += "\n".join(sos_calls)

# Zapis do pliku
with open(OUTPUT_FILE, "w") as f:
    f.write(output)

print(f"[INFO] Precision calls written to {OUTPUT_FILE}")
