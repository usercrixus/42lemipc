#!/usr/bin/env python3
import os
import sys
import signal
import subprocess
from pathlib import Path

# Minimal launcher: prints usage, launches processes, handles Ctrl-C.

DIR = Path(__file__).resolve().parent
os.chdir(DIR)

def usage(exit_code: int = 1) -> None:
    print(f"Usage: {Path(sys.argv[0]).name} <team_count> <players_per_team>", file=sys.stderr)
    sys.exit(exit_code)

# Usage handling (no validation beyond parsing ints)
if len(sys.argv) == 2 and sys.argv[1] in ("-h", "--help"):
    usage(0)
if len(sys.argv) != 3:
    usage(1)
try:
    team_count = int(sys.argv[1])
    players_per_team = int(sys.argv[2])
except Exception:
    usage(1)

# Team symbols (same ordering as before)
SYMS = [
    "1","2","3","4","5","6","7","8","9",
    "A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
    "a","b","c","d","e","f","g","h","i",
]

players = []
display = None
try:
    # Start players in background
    for _ in range(players_per_team):
        for t in range(team_count):
            sym = SYMS[t]
            players.append(subprocess.Popen(["./main.out", sym]))

    # Run displayer in foreground (but as a child we can terminate)
    display = subprocess.Popen(["./main.out"]) 
    try:
        display.wait()
    except KeyboardInterrupt:
        pass
finally:
    # Try to gracefully stop the display first
    if display and display.poll() is None:
        try:
            display.terminate()
        except Exception:
            pass
    # Then stop all players
    for p in players:
        if p.poll() is None:
            try:
                p.terminate()
            except Exception:
                pass
    # Ensure processes exit; if not, kill
    for proc in ([display] if display else []) + players:
        if not proc:
            continue
        try:
            proc.wait(timeout=0.5)
        except Exception:
            try:
                proc.kill()
            except Exception:
                pass
