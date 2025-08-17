import re

# Skrypt generujący wywołania funkcji dla programu TimeAnalysis_fixed.c

INPUT_FILE = "../_filtercoeffs/filtercoeffs.c"
OUTPUT_FILE = "generated_calls_time_fixed.txt"

with open(INPUT_FILE, "r") as f:
    content = f.read()

# Szukanie tylko zmiennych z `double ..._f64_ba` i `double ..._f64_sos`
ba_pattern = re.compile(r"double\s+(\w+)_f64_ba\s*\[\d+\]\[\d+\]\s*=")
sos_pattern = re.compile(r"double\s+(\w+)_f64_sos\s*\[\d+\]\[\d+\]\s*=")

# Generowanie DF1/DF2/TDF2
def generate_fixed(name, order):
    struct_match = re.search(r"_(df1|df2|tdf2)_", name)
    structure = struct_match.group(1).upper() if struct_match else "UNKNOWN"
    return f"""benchmark_fixed_q24(fp, "{name}_f64_ba", "{structure}", {structure}_q24, {name}_f64_ba[0], {name}_f64_ba[1], {order + 1});
benchmark_fixed_q12(fp, "{name}_f64_ba", "{structure}", {structure}_q12, {name}_f64_ba[0], {name}_f64_ba[1], {order + 1});"""

# Generowanie CASCADE
def generate_cascade(name, sections):
    return f"""benchmark_cascade_q24(fp, "{name}_f64_sos", CASCADE_q24, {name}_f64_sos, {sections});
benchmark_cascade_q12(fp, "{name}_f64_sos", CASCADE_q12, {name}_f64_sos, {sections});"""

# FIXED
fixed_calls = []
for match in ba_pattern.finditer(content):
    name = match.group(1)
    order_match = re.search(r"_order(\d+)_", name)
    order = int(order_match.group(1)) if order_match else 0
    fixed_calls.append(generate_fixed(name, order))

# CASCADE
cascade_calls = []
for match in sos_pattern.finditer(content):
    name = match.group(1)
    order_match = re.search(r"_order(\d+)_", name)
    order = int(order_match.group(1)) if order_match else 0
    sections = order // 2
    cascade_calls.append(generate_cascade(name, sections))

# Łączenie całości
output = "// === AUTO-GENERATED BENCHMARK CALLS ===\n\n"
output += "// --- FIXED BA ---\n"
output += "\n\n".join(fixed_calls)
output += "\n\n// --- CASCADE SOS ---\n"
output += "\n\n".join(cascade_calls)

# Zapis do pliku
with open(OUTPUT_FILE, "w") as f:
    f.write(output)

print(f"[INFO] Benchmark calls written to {OUTPUT_FILE}")