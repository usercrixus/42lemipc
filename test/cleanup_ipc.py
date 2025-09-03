#!/usr/bin/env python3
"""
Clean up SysV IPC artifacts (shared memory segments and message queues)
for lemipc when a run crashes or Ctrl-C interrupts.

By default, computes the same key as the program using ftok(path, proj_id)
with path ".gitmodules" and proj_id 130, then removes any SHM/MSGQ that
match that key.

Usage examples:
  - Dry run (show what would be removed):
      python3 scripts/cleanup_ipc.py --dry-run
  - Force removal without prompt:
      python3 scripts/cleanup_ipc.py -y
  - Specify a different ftok path or proj id:
      python3 scripts/cleanup_ipc.py --ftok-path somefile --proj-id 130
  - Specify an explicit key (decimal or 0x... hex):
      python3 scripts/cleanup_ipc.py --key 0x1e000082
"""

import argparse
import os
import re
import shlex
import subprocess
import sys
from typing import List, Tuple


def compute_ftok_key(path: str, proj_id: int) -> int:
    """Approximate Linux ftok(path, proj_id) implementation.

    key = (proj_id & 0xff) << 24 | (minor(st_dev) & 0xff) << 16 | (st_ino & 0xffff)
    """
    st = os.stat(path)
    ino = st.st_ino & 0xFFFF
    try:
        # Linux-specific device number helpers
        minor = os.minor(st.st_dev) & 0xFF  # type: ignore[attr-defined]
    except Exception:
        # Fallback: low 8 bits of st_dev
        minor = st.st_dev & 0xFF
    key = ((proj_id & 0xFF) << 24) | ((minor & 0xFF) << 16) | ino
    # On some systems key_t is signed; normalize to 32-bit unsigned range
    return key & 0xFFFFFFFF


def run(cmd: List[str]) -> Tuple[int, str, str]:
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    out, err = proc.communicate()
    return proc.returncode, out, err


def parse_ipcs(kind_flag: str) -> List[Tuple[str, int, int]]:
    """Parse `ipcs -m` or `ipcs -q` output.

    Returns list of tuples: (kind, key_int, id_int)
    kind is 'shm' for -m and 'msg' for -q.
    """
    kind = 'shm' if kind_flag == '-m' else 'msg'
    code, out, err = run(['ipcs', kind_flag])
    if code != 0:
        raise RuntimeError(f"ipcs {kind_flag} failed: {err.strip() or out.strip()}")
    items: List[Tuple[str, int, int]] = []
    # Skip until header with KEY appears; then parse lines starting with 0x...
    key_line = re.compile(r"^0x[0-9a-fA-F]+\s+")
    for line in out.splitlines():
        line = line.strip()
        if not key_line.match(line):
            continue
        parts = line.split()
        # Expected: KEY ID OWNER PERMS ...
        if len(parts) < 2:
            continue
        key_str, id_str = parts[0], parts[1]
        try:
            key_int = int(key_str, 16)
            id_int = int(id_str)
        except ValueError:
            continue
        items.append((kind, key_int, id_int))
    return items


def cleanup_for_key(key: int, *, dry_run: bool = False) -> int:
    """Remove SHM segments and MSG queues matching the given key.

    Returns number of resources removed (or that would be removed in dry-run).
    """
    items = []  # type: List[Tuple[str, int, int]]
    items += parse_ipcs('-m')
    items += parse_ipcs('-q')
    to_remove = [(kind, key_i, id_i) for (kind, key_i, id_i) in items if key_i == key]
    if not to_remove:
        print(f"No IPC objects found for key 0x{key:08x} ({key}).")
        return 0
    count = 0
    for kind, key_i, id_i in to_remove:
        if kind == 'shm':
            cmd = ['ipcrm', '-m', str(id_i)]
        else:
            cmd = ['ipcrm', '-q', str(id_i)]
        if dry_run:
            print(f"[dry-run] Would run: {' '.join(shlex.quote(c) for c in cmd)}  # key=0x{key_i:08x}")
            count += 1
            continue
        code, out, err = run(cmd)
        if code == 0:
            print(f"Removed {kind} id={id_i} for key=0x{key_i:08x}.")
            count += 1
        else:
            # ipcrm prints errors on stderr/stdout; show something helpful
            msg = (err.strip() or out.strip())
            print(f"Failed to remove {kind} id={id_i} (key=0x{key_i:08x}): {msg}", file=sys.stderr)
    return count


def parse_args(argv: List[str]) -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Cleanup SysV shared memory and message queue for a specific ftok key.")
    p.add_argument('--ftok-path', default='.gitmodules', help='Path used for ftok() (default: .gitmodules)')
    p.add_argument('--proj-id', type=int, default=130, help='Project id (0..255) used for ftok() (default: 130)')
    p.add_argument('--key', help='Explicit key to cleanup (decimal or 0x..). Overrides --ftok-path/--proj-id')
    p.add_argument('-y', '--yes', action='store_true', help='Do not prompt for confirmation')
    p.add_argument('--dry-run', action='store_true', help='Only show what would be removed')
    return p.parse_args(argv)


def main(argv: List[str]) -> int:
    args = parse_args(argv)
    if args.key is not None:
        key_str = str(args.key)
        key = int(key_str, 16) if key_str.lower().startswith('0x') else int(key_str)
    else:
        try:
            key = compute_ftok_key(args.ftok_path, args.proj_id)
        except FileNotFoundError:
            print(f"ftok path not found: {args.ftok_path}", file=sys.stderr)
            return 2
        except PermissionError as e:
            print(f"Cannot stat ftok path: {args.ftok_path}: {e}", file=sys.stderr)
            return 2
    key_hex = f"0x{key:08x}"

    # Show planned actions
    print(f"Target key: {key_hex} ({key})")
    try:
        items = []  # type: List[Tuple[str, int, int]]
        items += parse_ipcs('-m')
        items += parse_ipcs('-q')
    except Exception as e:
        print(f"Failed to list IPC objects: {e}", file=sys.stderr)
        return 2
    matches = [(k, kid, iid) for (k, kid, iid) in items if kid == key]
    if not matches:
        print("No matching IPC objects found. Nothing to do.")
        return 0
    print("Will remove:")
    for kind, kid, iid in matches:
        print(f"  - {kind}: id={iid} key=0x{kid:08x}")

    if not args.yes and not args.dry_run:
        resp = input("Proceed? [y/N] ").strip().lower()
        if resp not in ("y", "yes"):
            print("Aborted.")
            return 1

    removed = cleanup_for_key(key, dry_run=args.dry_run)
    if removed:
        print(f"Done. {'(dry-run) ' if args.dry_run else ''}{removed} object(s) processed.")
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

