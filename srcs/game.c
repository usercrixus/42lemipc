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

bool isGameEnd()
{
	char first = 0;
	int seen = 0;

	if (shared->playersAlive == 2)
		return (true);
	for (size_t y = 0; y < (size_t)MAP_HEIGHT; y++)
	{
		for (size_t x = 0; x < (size_t)MAP_WIDTH; x++)
		{
			char c = shared->map[y][x];
			if (c == EMPTY_TILE)
				continue;
			seen = 1;
			if (first == 0)
				first = c;
			else if (c != first)
				return (false);
		}
	}
	return (seen == 0) || (first != 0);
}

