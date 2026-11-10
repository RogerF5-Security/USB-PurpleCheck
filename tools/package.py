"""Package the app, source, documentation and tests without local evidence."""
from pathlib import Path
import json
import zipfile
import hashlib

root = Path(__file__).resolve().parents[1]
binary = root / "dist/usb_purplecheck.fap"
meta = json.loads((root / "dist/BUILD.json").read_text())
assert meta["version"] == "0.3"
assert meta["sha256"] == hashlib.sha256(binary.read_bytes()).hexdigest()
files = [root / n for n in ("application.fam", "README.md", "LEEME-PRIMERO.md",
                            "LICENSE", "THIRD_PARTY_NOTICES.md", ".gitignore", ".clang-format")]
for folder in ("src", "vendor", "assets", "tests", "docs", "tools",
               "screenshots", "catalog", ".github"):
    files.extend(p for p in (root / folder).rglob("*") if p.is_file()
                 and "__pycache__" not in p.parts
                 and p.suffix not in (".pyc", ".exe", ".img", ".zip"))
files.extend([binary, root / "dist/BUILD.json"])
archive = root / "dist/USB-PurpleCheck-0.3.zip"
with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as z:
    for p in sorted(files):
        z.write(p, Path("USB-PurpleCheck") / p.relative_to(root))
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
print(json.dumps({"file": str(archive), "files": len(files),
                  "bytes": archive.stat().st_size}, indent=2))
