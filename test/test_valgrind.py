#!/usr/bin/env python3
import os
import sys
import shutil
import subprocess
from pathlib import Path
import time

TERMINALS = (
    (
        "gnome-terminal",
        lambda title, cmd: ["gnome-terminal", "--title", title, "--", "bash", "-lc", f"{cmd}; exec bash"],
    ),
    ("konsole", lambda title, cmd: ["konsole", "--hold", "-p", f"tabtitle={title}", "-e", "bash", "-lc", f"{cmd}"]),
    (
        "xfce4-terminal",
        lambda title, cmd: ["xfce4-terminal", "--title", title, "--hold", "-e", f"bash -lc '{cmd}; exec bash'"],
    ),
    ("xterm", lambda title, cmd: ["xterm", "-T", title, "-hold", "-e", "bash", "-lc", f"{cmd}; exec bash"]),
    ("kitty", lambda title, cmd: ["kitty", "--title", title, "bash", "-lc", f"{cmd}; exec bash"]),
    ("alacritty", lambda title, cmd: ["alacritty", "-t", title, "-e", "bash", "-lc", f"{cmd}; exec bash"]),
    ("tilix", lambda title, cmd: ["tilix", "-t", title, "-e", "bash", "-lc", f"{cmd}; exec bash"]),
    (
        "wezterm",
        lambda title, cmd: ["wezterm", "start", "--always-new-process", "--", "bash", "-lc", f"{cmd}; exec bash"],
    ),
)


def find_terminal():
    for name, builder in TERMINALS:
        if shutil.which(name):
            return name, builder
    return None, None


def open_in_terminal(title: str, command: str) -> None:
    name, builder = find_terminal()
    if not name:
        raise RuntimeError("No emulator find.")
    cmd = builder(title, command)
    subprocess.Popen(cmd)


def wrap(cmd: str) -> str:
    vg = "valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes"
    return f"{vg} {cmd}"


def main() -> int:

    if not Path("main.out").exists():
        print("no main.out. launch 'make' first.", file=sys.stderr)
        return 1

    if not shutil.which("valgrind"):
        print("No valgrind found.", file=sys.stderr)
        return 1

    try:
        teams = ["1", "2"]
        players_per_team = 2
        for sym in teams:
            for i in range(players_per_team):
                title = f"player team {sym} #{i+1}"
                open_in_terminal(title, wrap(f"./main.out {sym}"))
        time.sleep(0.5)
        open_in_terminal("displayer", wrap("./main.out"))
        print(f"Launch: displayer + {len(teams)} teams x {players_per_team} player (with valgrind).")
    except Exception as e:
        print(f"Erreur: {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
