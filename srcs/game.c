#include "game.h"

static int countCardinalThreats(t_player *p)
{
	int count = 0;

	if (p->y > 0 &&
		shared->map[p->y - 1][p->x] != EMPTY_TILE &&
		shared->map[p->y - 1][p->x] != p->symbole)
		count++;

	if (p->y < MAP_HEIGHT - 1 &&
		shared->map[p->y + 1][p->x] != EMPTY_TILE &&
		shared->map[p->y + 1][p->x] != p->symbole)
		count++;

	if (p->x > 0 &&
		shared->map[p->y][p->x - 1] != EMPTY_TILE &&
		shared->map[p->y][p->x - 1] != p->symbole)
		count++;

	if (p->x < MAP_WIDTH - 1 &&
		shared->map[p->y][p->x + 1] != EMPTY_TILE &&
		shared->map[p->y][p->x + 1] != p->symbole)
		count++;

	return count;
}

static int countDiagonalThreats(t_player *p)
{
	int count = 0;

	if (p->y > 0 && p->x > 0 &&
		shared->map[p->y - 1][p->x - 1] != EMPTY_TILE &&
		shared->map[p->y - 1][p->x - 1] != p->symbole)
		count++;

	if (p->y > 0 && p->x < MAP_WIDTH - 1 &&
		shared->map[p->y - 1][p->x + 1] != EMPTY_TILE &&
		shared->map[p->y - 1][p->x + 1] != p->symbole)
		count++;

	if (p->y < MAP_HEIGHT - 1 && p->x > 0 &&
		shared->map[p->y + 1][p->x - 1] != EMPTY_TILE &&
		shared->map[p->y + 1][p->x - 1] != p->symbole)
		count++;

	if (p->y < MAP_HEIGHT - 1 && p->x < MAP_WIDTH - 1 &&
		shared->map[p->y + 1][p->x + 1] != EMPTY_TILE &&
		shared->map[p->y + 1][p->x + 1] != p->symbole)
		count++;

	return count;
}

int isAlive(t_player *p)
{
	return (countCardinalThreats(p) + countDiagonalThreats(p) < 2);
}

void manageDeath(t_player *p)
{
	shared->map[p->y][p->x] = EMPTY_TILE;
	p->isAlive = 0;
}

void setIsAlive()
{
	for (size_t i = 0; i < MAX_PLAYER; i++)
	{
		t_player *p = &shared->players[i];
		if (p->pid != -1 && !isAlive(p))
			manageDeath(p);
	}
}

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
	pthread_mutex_lock(&shared->mutexGame);
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
		setIsAlive();
		for (int i = 0; i < MAX_PLAYER; i++)
		{
			if (shared->players[i].pid != -1)
				kill(shared->players[i].pid, SIGUSR1);
		}
	}
	pthread_mutex_unlock(&shared->mutexGame);
}

void drawMap()
{
	for (size_t y = 0; y < MAP_HEIGHT; y++)
	{
		for (size_t x = 0; x < MAP_WIDTH; x++)
		{
			ft_printf(" %c ", shared->map[y][x]);
		}
		ft_printf("\n");
	}
	ft_printf("\n");
}
