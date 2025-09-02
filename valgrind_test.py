#!/usr/bin/env python3
import os
import sys
import time
import glob
import signal
import subprocess
from pathlib import Path


def main() -> int:
    # Work in repo root (where main.out is)
    base_dir = Path(__file__).resolve().parent
    os.chdir(base_dir)

    # Fixed config: 2 teams, 2 players per team
    teams = ["1", "2"]
    players_per_team = 2

    # Prepare logs directory
    logs_dir = base_dir / "valgrind-logs"
    logs_dir.mkdir(exist_ok=True)
    # Clear previous logs for this run pattern
    for old in glob.glob(str(logs_dir / "valgrind-player-*.log")):
        try:
            os.remove(old)
        except Exception:
            pass

    procs: list[subprocess.Popen] = []

    def start_player(sym: str, idx: int) -> subprocess.Popen:
        log_file = logs_dir / f"valgrind-player-{sym}-{idx}.log"
        cmd = [
            "valgrind",
            "--leak-check=full",
            "--show-leak-kinds=all",
            "--track-origins=yes",
            f"--log-file={str(log_file)}",
            "./main.out",
            sym,
        ]
        return subprocess.Popen(cmd)

    # Launch 4 players total (2 teams x 2 players each)
    for sym in teams:
        for idx in range(players_per_team):
            procs.append(start_player(sym, idx))

    print("Started 4 players under Valgrind (2 teams x 2).")

    # Fixed duration: run for a short time, then stop automatically
    DURATION_SECONDS = 5
    try:
        time.sleep(DURATION_SECONDS)
    except KeyboardInterrupt:
        pass
    finally:
        # Gracefully terminate, then kill if needed
        # Try graceful stop to let Valgrind flush logs
        for p in procs:
            if p.poll() is None:
                try:
                    p.send_signal(signal.SIGINT)
                except Exception:
                    pass
        for p in procs:
            try:
                p.wait(timeout=1.0)
            except Exception:
                pass
        # Escalate to SIGTERM, then SIGKILL if needed
        for p in procs:
            if p.poll() is None:
                try:
                    p.terminate()
                except Exception:
                    pass
        for p in procs:
            try:
                p.wait(timeout=1.0)
            except Exception:
                try:
                    p.kill()
                except Exception:
                    pass

    # Print quick Valgrind summaries
    print("\nValgrind summaries:")
    for sym in teams:
        for idx in range(players_per_team):
            path = logs_dir / f"valgrind-player-{sym}-{idx}.log"
            label = f"player {sym} #{idx}"
            try:
                with open(path, "r", encoding="utf-8", errors="ignore") as f:
                    lines = f.readlines()
                summary = next((ln.strip() for ln in lines if ln.startswith("ERROR SUMMARY")), None)
                print(f"- {label}: {summary or 'no summary found'}")
            except FileNotFoundError:
                print(f"- {label}: log not found ({path.name})")
            except Exception as e:
                print(f"- {label}: failed to read log ({e})")

    return 0


if __name__ == "__main__":
    sys.exit(main())
