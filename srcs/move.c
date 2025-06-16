#include "move.h"

static void effectiveMove(int dx, int dy)
{
	t_player *player = &shared->players[playerId];
	int new_x = player->x + dx;
	int new_y = player->y + dy;
	if (!(new_x < 0 || new_x >= MAP_WIDTH || new_y < 0 || new_y >= MAP_HEIGHT))
	{
		if (shared->map[new_y][new_x] == EMPTY_TILE)
		{
			shared->map[player->y][player->x] = EMPTY_TILE;
			shared->map[new_y][new_x] = player->symbole;
			player->x = new_x;
			player->y = new_y;
		}
	}
}

void move(t_move move)
{
	if (shared->players[playerId].isAlive)
	{
		if (move == TOP)
			effectiveMove(0, -1);
		else if (move == BOT)
			effectiveMove(0, 1);
		else if (move == LEFT)
			effectiveMove(-1, 0);
		else if (move == RIGHT)
			effectiveMove(1, 0);
		for (int i = 0; i < MAX_PLAYER; i++)
		{
			if (shared->players[i].pid != -1)
				kill(shared->players[i].pid, SIGUSR1);
		}
	}
}