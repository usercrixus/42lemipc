# 42lemipc

Usage
- Build: `make`
- Quick start with launcher:
  - Make the script executable once: `chmod +x ./launch`
  - Start T teams with N players each, then the displayer: `./launch T N`
    - Example: `./launch 3 4` starts 3 teams with 4 players per team.
    - Team symbols are `1-9` then `A-Z` (no `0`). The launcher caps counts so `T*N <= MAX_PLAYER`.

Manual
- Displayer: `./main.out`
- Player (AI): `./main.out 1` (team symbol). Any non-`0` character can be used; launcher uses `1-9A-Z`.
