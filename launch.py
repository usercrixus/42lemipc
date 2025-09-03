import os
import sys
import subprocess
from pathlib import Path


def usage(exit_code: int = 1) -> None:
    print(f"Usage: {Path(sys.argv[0]).name} <team_count> <players_per_team>", file=sys.stderr)
    sys.exit(exit_code)


if __name__ == "__main__":
    DIR = Path(__file__).resolve().parent
    os.chdir(DIR)
    SYMS = [
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "I",
        "J",
        "K",
        "L",
        "M",
        "N",
        "O",
        "P",
        "Q",
        "R",
        "S",
        "T",
        "U",
        "V",
        "W",
        "X",
        "Y",
        "Z",
        "a",
        "b",
        "c",
        "d",
        "e",
        "f",
        "g",
        "h",
        "i",
        "j",
        "k",
        "l",
        "m",
        "n",
        "o",
        "p",
        "q",
        "r",
        "s",
        "t",
        "u",
        "v",
        "w",
        "x",
        "y",
        "z",
    ]
    if len(sys.argv) == 2 and sys.argv[1] in ("-h", "--help"):
        usage(0)
    if len(sys.argv) != 3:
        usage(1)
    try:
        team_count = int(sys.argv[1])
        players_per_team = int(sys.argv[2])
        try:
            for _ in range(players_per_team):
                for t in range(team_count):
                    sym = SYMS[t]
                    subprocess.Popen(["./main.out", sym])

            display = subprocess.Popen(["./main.out"])
            try:
                display.wait()
            except KeyboardInterrupt:
                pass
        except:
            print("An error occured during process launch")
    except Exception:
        usage(1)
