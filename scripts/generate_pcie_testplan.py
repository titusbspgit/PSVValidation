import os, re, json
from datetime import datetime
from zoneinfo import ZoneInfo
from pathlib import Path
from typing import List, Tuple

try:
    from openpyxl import Workbook
except ImportError as e:
    raise SystemExit("openpyxl is required. Install with: pip install openpyxl")

OWNER = os.environ.get("OWNER", "titusbspgit")
REPO = os.environ.get("REPO", "PSVValidation")
BRANCH = os.environ.get("BRANCH", "main")
BASE_DIR = os.environ.get("BASE_DIR", "TestRepo/pcie")
IP_NAME = os.environ.get("IP_NAME", "PCIE")
OUT_DIR = Path("Test_Output/PCIE/TestPlan")
FILENAME = os.environ.get("EXACT_FILENAME")

if not FILENAME:
    # Fallback: compute IST timestamp now (Asia/Kolkata)
    now_ist = datetime.now(ZoneInfo("Asia/Kolkata"))
    FILENAME = f"{IP_NAME}_TestPlan_{now_ist:%Y%m%d_%H%M%S}.xlsx"

OUT_PATH = OUT_DIR / FILENAME

SRC_EXTS = {".c", ".cpp", ".py", ".h"}
SCRIPT_EXTS = {".sh", ".bat", ".ps1", ".tcl", ".pl"}


def is_test_case_folder(folder: Path) -> bool:
    has_code = any(p.suffix in SRC_EXTS for p in folder.rglob('*') if p.is_file())
    has_readme = any(p.name.lower().startswith("readme") for p in folder.iterdir() if p.is_file())
    has_script = any(p.suffix in SCRIPT_EXTS for p in folder.rglob('*') if p.is_file())
    return has_code or has_readme or has_script


def find_test_folders(base_dir: Path) -> List[Path]:
    if not base_dir.exists():
        return []
    tests = []
    for root, dirs, files in os.walk(base_dir):
        root_path = Path(root)
        # Only consider leaf-level or any directory containing qualifying files
        if is_test_case_folder(root_path):
            tests.append(root_path)
    # unique and sort by folder name
    uniq = {p.as_posix(): p for p in tests}
    return sorted(uniq.values(), key=lambda p: p.name.lower())


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding='utf-8', errors='ignore')
    except Exception:
        return ""


def extract_metadata(folder: Path) -> Tuple[str, str, str, str, str, str, str, str]:
    test_name = folder.name
    test_folder_rel = folder.as_posix()
    repo_url = f"https://github.com/{OWNER}/{REPO}/tree/{BRANCH}/{test_folder_rel}"

    # Collect source files
    src_files = sorted([p.name for p in folder.glob('**/*') if p.is_file() and p.suffix in SRC_EXTS])
    src_list = "; ".join(src_files) if src_files else ""

    # Parse program.c if present for heuristics
    program_c = folder / "program.c"
    code = read_text(program_c) if program_c.exists() else ""

    entry_fn = "test_case" if re.search(r"\bint\s+test_case\s*\(", code) else ("test_case" if any("test_case(" in read_text(p) for p in folder.glob('**/*.c')) else "")

    key_ops_parts = []
    if "chk_rst_val" in code:
        key_ops_parts.append("chk_rst_val()")
    if "chk_rd_wr" in code:
        key_ops_parts.append("chk_rd_wr()")
    if "link_training" in code:
        key_ops_parts.append("link_training_*()")
    if "mem_base_program" in code:
        key_ops_parts.append("mem_base_program_*()")
    if re.search(r"read_\w+reg|write_\w+reg", code):
        key_ops_parts.append("read/write *_reg()")
    if "wait_on(" in code:
        key_ops_parts.append("wait_on() polling")
    if "finish(0)" in code:
        key_ops_parts.append("finish(0) on success")
    key_ops = "; ".join(dict.fromkeys(key_ops_parts))

    # Objective heuristics based on folder name and code
    obj = ""
    lname = test_name.lower()
    if "cfg_wr_rd" in lname:
        obj = ("Perform PCIe link training, program coherency control and memory base, "
               "configure BARs, and validate config space reads/writes including handshake.")
    elif "sii_rc_reg_wr_rd" in lname:
        obj = ("Validate SII RC register default reset values and masked read/write behavior; "
               "skip default check for PHY reset control if applicable.")
    elif "dbi_dsp_reg_wr_rd" in lname:
        obj = ("Validate DBI DSP register default reset values and masked read/write behavior "
               "using standard data patterns.")
    elif "dbi_usp_reg_wr_rd" in lname:
        obj = ("Validate DBI USP register default reset values and masked read/write behavior "
               "using standard data patterns.")
    else:
        obj = "Validate register defaults and masked read/write behavior for PCIe block."

    # Expected result
    expected = "Test passes if finish(0) is reached with no reported mismatches/errors."
    if "def_fail_cnt" in code or "wr_fail_cnt" in code:
        expected = ("Pass if def_fail_cnt==0 and wr_fail_cnt==0, leading to finish(0); "
                    "fail otherwise.")
    if "0xE6004100" in code and "0x12345678" in code:
        expected = ("Pass when handshake register 0xE6004100 reaches 0x12345678 and finish(0) is called.")

    return (IP_NAME, test_name, test_folder_rel, obj, entry_fn, key_ops, expected, src_list, repo_url)


def build_workbook(rows: List[Tuple[str, ...]]):
    headers = [
        "IP", "Test Name", "Test Folder", "Objective", "Entry Function",
        "Key Operations", "Expected Result", "Source Files", "Repo URL"
    ]
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"
    ws.append(headers)
    for row in rows:
        ws.append(row)
    # Basic column sizing
    for col in (1,2,3,4,5,6,7,8,9):
        ws.column_dimensions[chr(64+col)].width = 28
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    wb.save(OUT_PATH)


def main():
    base = Path(BASE_DIR)
    tests = find_test_folders(base)
    rows = [extract_metadata(p) for p in tests]
    # Only keep unique by folder name (already sorted in find function)
    build_workbook(rows)
    print(f"Wrote: {OUT_PATH}")


if __name__ == "__main__":
    main()
