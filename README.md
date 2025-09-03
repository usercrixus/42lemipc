# 42lemipc

Simple multi‑process arena using System V IPC (shared memory, semaphores, messages).
Each player is its own process, belongs to a team, and moves on a grid; a
displayer renders the board in the terminal. Goal: teams coordinate via IPC to
outnumber/eliminate opponents until one team remains.

Build & Run
- Build: `make`
- Quick start (spawns players then the displayer): `./launch.py <teams> <players_per_team>`
  - Example: `./launch.py 3 4`
- Manual run:
  - Displayer: `./main.out`
  - Player (AI): `./main.out X` where `X` is the team symbol (e.g. `1`, `A`).

Makefile (basics)
- `all`: default target; ensures submodules are ready, builds `main.out`.
- `clean`: removes object files.
- `fclean`: also removes `main.out`.
- `re`: full rebuild (equivalent to `fclean` then build).

Test (basics)
- `python3 test/test_ipc_cleanup.py`: Verify shared memory segment and msg queue is well destroyed.
- `python3 test/test_valgrind.py`: launch a 2 team 2 player game with valgrind (terminal popup).

Notes
- Team symbols used by the launcher start at `1-9`, then `A-Z`, then `a-z`.
- Limit: total players must be `teams * players_per_team <= MAX_PLAYER` defined in `srcs/include/main.h`.
