import re

# Skrypt generujący wywołania funkcji dla programu TimeAnalysis.java

INPUT_FILE = "./coeffs/FilterCoeffs.java"
OUTPUT_FILE = "generated_calls_time_java.txt"

with open(INPUT_FILE, "r") as f:
    content = f.read()

pattern_full = re.compile(
    r'public\s+static\s+final\s+(float|double)\s*\[\]\[\]\s+((BUTTER|CHEBY1|CHEBY2|ELLIP|BESSEL)_[A-Z0-9_]+)\s*=\s*\{.*?\};',
    re.DOTALL | re.MULTILINE
)

type_map = {
    "float": ("float", "F32"),
    "double": ("double", "F64")
}

def generate_ba_call(full_name, dtype):
    base_name = full_name.lower()
    java_type, suffix = type_map[dtype]

    if "_DF1_" in full_name:
        struct = "DF1"
        func_d, func_f = "DF1::filterDouble", "DF1::filterFloat"
    elif "_DF2_" in full_name:
        struct = "DF2"
        func_d, func_f = "DF2::filterDouble", "DF2::filterFloat"
    elif "_TDF2_" in full_name:
        struct = "TDF2"
        func_d, func_f = "TDF2::filterDouble", "TDF2::filterFloat"
    else:
        return None

    order_match = re.search(r"_ORDER(\d+)_", full_name)
    order = int(order_match.group(1)) if order_match else 2
    n = order + 1  # teraz order+1, nie (order//2)+1

    if dtype == "double":
        return f'benchmarkAndLog(fp, "{base_name}", "{java_type}", "{struct}", null, {func_d}, null, null, FilterCoeffs.{full_name}[0], FilterCoeffs.{full_name}[1], {n});'
    else:
        return f'benchmarkAndLog(fp, "{base_name}", "{java_type}", "{struct}", {func_f}, null, FilterCoeffs.{full_name}[0], FilterCoeffs.{full_name}[1], null, null, {n});'

def generate_cascade_call(full_name, dtype):
    base_name = full_name.lower()
    java_type, suffix = type_map[dtype]

    order_match = re.search(r"_ORDER(\d+)_", full_name)
    order = int(order_match.group(1)) if order_match else 2
    sections = order // 2

    if dtype == "double":
        return f'benchmarkCascadeAndLog(fp, "{base_name}", "{java_type}", null, CASCADE::filterDouble, null, FilterCoeffs.{full_name}, {sections});'
    else:
        return f'benchmarkCascadeAndLog(fp, "{base_name}", "{java_type}", CASCADE::filterFloat, null, FilterCoeffs.{full_name}, null, {sections});'

calls = ["// === AUTO-GENERATED TIME ANALYSIS CALLS ===\n"]
calls.append("// --- DF1 / DF2 / TDF2 ---")

for match in pattern_full.finditer(content):
    dtype, full_name, prefix = match.groups()
    if "_BA" in full_name:
        call = generate_ba_call(full_name, dtype)
        if call:
            calls.append(call)

calls.append("\n// --- CASCADE ---")

for match in pattern_full.finditer(content):
    dtype, full_name, prefix = match.groups()
    if "_SOS" in full_name:
        call = generate_cascade_call(full_name, dtype)
        if call:
            calls.append(call)

with open(OUTPUT_FILE, "w") as f:
    f.write("\n".join(calls))

print(f"[INFO] Generated {len(calls)-2} benchmark calls to {OUTPUT_FILE}")
