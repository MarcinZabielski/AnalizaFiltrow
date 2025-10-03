import re

# Skrypt generujący wywołania funkcji dla programu PrecisionAnalysis.java

INPUT_FILE = "./coeffs/FilterCoeffs.java"
OUTPUT_FILE = "generated_calls_precision_java.txt"

with open(INPUT_FILE, "r") as f:
    content = f.read()

pattern = re.compile(
    r'public\s+static\s+final\s+(float|double)\s*\[\]\[\]\s+((BUTTER|CHEBY1|CHEBY2|ELLIP|BESSEL)_[A-Z0-9_]+)\s*=',
    re.DOTALL | re.MULTILINE
)

def extract_order(name):
    match = re.search(r"_ORDER(\d+)_", name)
    return int(match.group(1)) if match else 2

def get_structure(name):
    if "_DF1_" in name:
        return "DF1"
    if "_DF2_" in name:
        return "DF2"
    if "_TDF2_" in name:
        return "TDF2"
    return None

def generate_precision_call(base_name):
    lower_name = base_name.lower()
    structure = get_structure(base_name)
    order = extract_order(base_name)
    n = order + 1

    if not structure:
        return None

    return f'Precision(fp, "{lower_name}", "{structure}",{structure}::filterFloat, {structure}::filterDouble,' \
           f'FilterCoeffs.{base_name.replace("F64", "F32")}[0], FilterCoeffs.{base_name.replace("F64", "F32")}[1],' \
           f'FilterCoeffs.{base_name}[0], FilterCoeffs.{base_name}[1],{n});'

def generate_precision_cascade_call(base_name):
    lower_name = base_name.lower()
    order = extract_order(base_name)
    sections = order // 2
    return f'PrecisionCascade(fp, "{lower_name}",CASCADE::filterFloat, CASCADE::filterDouble,' \
           f'FilterCoeffs.{base_name.replace("F64", "F32")},FilterCoeffs.{base_name},{sections});'

calls = ["// === AUTO-GENERATED PRECISION ANALYSIS CALLS ===", "// --- DF1 / DF2 / TDF2 ---"]

seen_ba_pairs = set()

for match in pattern.finditer(content):
    dtype, full_name, _ = match.groups()

    if "_BA" in full_name and dtype == "double":
        pair_key = full_name.replace("F64", "")
        if pair_key in seen_ba_pairs:
            continue
        seen_ba_pairs.add(pair_key)
        call = generate_precision_call(full_name)
        if call:
            calls.append(call)

calls.append("\n// --- CASCADE ---")

seen_sos_pairs = set()

for match in pattern.finditer(content):
    dtype, full_name, _ = match.groups()

    if "_SOS" in full_name and dtype == "double":
        pair_key = full_name.replace("F64", "")
        if pair_key in seen_sos_pairs:
            continue
        seen_sos_pairs.add(pair_key)
        call = generate_precision_cascade_call(full_name)
        if call:
            calls.append(call)

with open(OUTPUT_FILE, "w") as f:
    f.write("\n".join(calls))

print(f"[INFO] Generated {len(calls)-2} Precision/PrecisionCascade calls in '{OUTPUT_FILE}'")
