import re

INPUT_FILE = "../_filtercoeffs/filtercoeffs.c"
OUTPUT_FILE = "generated_calls_overflow.txt"

with open(INPUT_FILE, "r") as f:
    content = f.read()

# Regex do BA i SOS
ba_pattern = re.compile(r"double\s+(\w+)_f64_ba\s*\[\d+\]\[\d+\]\s*=")
sos_pattern = re.compile(r"double\s+(\w+)_f64_sos\s*\[\d+\]\[\d+\]\s*=")

def generate_overflow_ba(name, order):
    struct_match = re.search(r"_(df1|df2|tdf2)_", name)
    structure = struct_match.group(1).upper() if struct_match else "UNKNOWN"
    call_q24 = f'overflow_analysis_q(fp, "{name}", "{structure}", 24, (void (*)(void*, void*, void*, void*, int, int)){structure}_q24, {name}_f64_ba[0], {name}_f64_ba[1], {order + 1});'
    call_q12 = f'overflow_analysis_q(fp, "{name}", "{structure}", 12, (void (*)(void*, void*, void*, void*, int, int)){structure}_q12, {name}_f64_ba[0], {name}_f64_ba[1], {order + 1});'
    return call_q24 + "\n" + call_q12

def generate_overflow_sos(name, sections):
    call_q24 = f'overflow_analysis_sos_q(fp, "{name}", "CASCADE", 24, (void (*)(void*, void*, void*, int,  int))CASCADE_q24, {name}_f64_sos, {sections});'
    call_q12 = f'overflow_analysis_sos_q(fp, "{name}", "CASCADE", 12, (void (*)(void*, void*, void*, int,  int))CASCADE_q12, {name}_f64_sos, {sections});'
    return call_q24 + "\n" + call_q12

# Generuj wywołania BA
ba_calls = []
for match in ba_pattern.finditer(content):
    name = match.group(1)
    order_match = re.search(r"_order(\d+)_", name)
    order = int(order_match.group(1)) if order_match else 0
    ba_calls.append(generate_overflow_ba(name, order))

# Generuj wywołania SOS
sos_calls = []
for match in sos_pattern.finditer(content):
    name = match.group(1)
    order_match = re.search(r"_order(\d+)_", name)
    order = int(order_match.group(1)) if order_match else 0
    sections = order // 2
    sos_calls.append(generate_overflow_sos(name, sections))

# Zbuduj wynik
output = "// === AUTO-GENERATED OVERFLOW ANALYSIS CALLS ===\n\n"
output += "// --- DF1 / DF2 / TDF2 ---\n\n"
output += "\n".join(ba_calls)
output += "\n\n// --- CASCADE (SOS) ---\n\n"
output += "\n".join(sos_calls)

# Zapisz do pliku
with open(OUTPUT_FILE, "w") as f:
    f.write(output)

print(f"[INFO] Overflow analysis calls written to {OUTPUT_FILE}")
