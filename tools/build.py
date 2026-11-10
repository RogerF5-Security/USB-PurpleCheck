"""Build using an installed SDK, without flashing."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess

p = argparse.ArgumentParser(description=__doc__)
p.add_argument("--sdk-home", type=Path, default=Path.home() / ".ufbt")
args = p.parse_args()
root = Path(__file__).resolve().parents[1]
sdk = args.sdk_home.resolve()
state = json.loads((sdk / "current/ufbt_state.json").read_text())
ufbt = shutil.which("ufbt")
if not ufbt:
    p.error("Install uFBT first")
env = os.environ.copy()
env["UFBT_HOME"] = str(sdk)
if (sdk / "toolchain").is_dir():
    env["FBT_TOOLCHAIN_PATH"] = str(sdk)
subprocess.run([ufbt], cwd=root, env=env, check=True)
fap = root / "dist/usb_purplecheck.fap"
meta = {"version": "0.3", "sdk": state, "size": fap.stat().st_size,
        "sha256": hashlib.sha256(fap.read_bytes()).hexdigest()}
(root / "dist/BUILD.json").write_text(json.dumps(meta, indent=2) + "\n")
