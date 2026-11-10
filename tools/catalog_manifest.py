"""Write an immutable catalog manifest for a verified source commit."""
import argparse
from pathlib import Path
import re

p = argparse.ArgumentParser(description=__doc__)
p.add_argument("--commit", required=True)
p.add_argument("--output", type=Path, required=True)
args = p.parse_args()
if not re.fullmatch(r"[0-9a-f]{40}", args.commit):
    p.error("Use a full lowercase 40-character source commit SHA")
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(
    "sourcecode:\n"
    "  type: git\n"
    "  location:\n"
    "    origin: https://github.com/RogerF5-Security/USB-PurpleCheck.git\n"
    f"    commit_sha: {args.commit}\n"
    'description: "@docs/catalog-description.md"\n'
    'changelog: "@docs/catalog-changelog.md"\n'
    "screenshots:\n"
    "  - screenshots/splash.png\n"
    "  - screenshots/menu.png\n"
    "  - screenshots/settings.png\n"
    "  - screenshots/spanish.png\n"
    "  - screenshots/ukrainian.png\n"
    "  - screenshots/about.png\n",
    encoding="utf-8",
)
print(args.output)
