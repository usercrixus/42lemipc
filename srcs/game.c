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
	char team;

	team = '0';
	for (size_t y = 0; y < MAP_HEIGHT; y++)
	{
		for (size_t x = 0; x < MAP_WIDTH; x++)
		{
			if (shared->map[y][x] != team)
			{
				if (team == '0')
					team = shared->map[y][x];
				else
					return (true);
			}
		}
	}
	return (false);
}