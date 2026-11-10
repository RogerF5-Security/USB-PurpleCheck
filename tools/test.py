"""Run native logic tests, including sanitizer checks on Linux."""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
from PIL import Image

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--sanitize", action="store_true")
args = parser.parse_args()
os.chdir(root)
gcc = shutil.which("gcc")
if not gcc:
    raise SystemExit("A native GCC compiler is required")
out = root / "dist"
out.mkdir(exist_ok=True)
binary = out / ("test_core.exe" if os.name == "nt" else "test_core")
sources = ["tests/test_core.c", "src/probe_disk.c", "src/probe_status.c",
           "src/pc_settings.c", "src/pc_i18n.c", "vendor/mass_storage_scsi.c"]
flags = ["-std=c11", "-Wall", "-Wextra", "-Werror", "-Wno-unused-variable",
         "-I", "tests/include"]
if args.sanitize:
    flags += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-g"]
subprocess.run([gcc, *flags, *sources, "-o", str(binary)], check=True)
subprocess.run([str(binary), str(out / "probe-test.img")], check=True)
text = (root / "src/pc_i18n.c").read_text(encoding="utf-8")
font = (root / "src/pc_font.c").read_text(encoding="utf-8")
supported = {int(cp, 16) for cp in re.findall(r"\{0x([0-9A-Fa-f]+),", font)}
required = {ord(c) for c in text if "\u0400" <= c <= "\u04ff"}
assert required and required <= supported, sorted(required - supported)
icon = Image.open(root / "assets/purplecheck_10px.png")
assert icon.size == (10, 10) and icon.mode == "1"
for p in (root / "screenshots").glob("*.png"):
    screenshot = Image.open(p).convert("RGB")
    assert screenshot.size == (512, 256), p
    assert set(screenshot.getdata()) <= {(254, 138, 44), (0, 0, 0)}, p
print("PASS: Ukrainian glyph coverage, icon and available catalog screenshots")
