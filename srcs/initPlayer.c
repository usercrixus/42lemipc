#include "initPlayer.h"
#include "main.h"
#include "game.h"
#include "sharedMemory.h"
#include "move.h"
#include "ai.h"
#include "42libft/ft_printf/ft_printf.h"

static void handleMove()
{
	t_player p = shared->players[playerId];
	while (true)
	{
		pthread_mutex_lock(&shared->mutexGame);
		if (isGameEnd())
			break;
		if (!isAlive(&p))
		{
			shared->map[p.y][p.x] = EMPTY_TILE;
			break;
		}
		t_move bestMove = getBestMove();
		move(bestMove);
		if (!isAlive(&p))
		{
			shared->map[p.y][p.x] = EMPTY_TILE;
			break;
		}
		pthread_mutex_unlock(&shared->mutexGame);
	}
}

static int initPlayerPosition(t_player *p)
{
	int startY = rand() % MAP_HEIGHT;
	int startX = rand() % MAP_WIDTH;

	for (int offset = 0; offset < MAP_HEIGHT * MAP_WIDTH; offset++)
	{
		int index = (startY * MAP_WIDTH + startX + offset) % (MAP_HEIGHT * MAP_WIDTH);
		int y = index / MAP_WIDTH;
		int x = index % MAP_WIDTH;

		if (shared->map[y][x] == EMPTY_TILE)
		{
			p->x = x;
			p->y = y;
			if (isAlive(p))
				return (true);
		}
	}
	return (false);
}

static int initPlayer(char team)
{
	pthread_mutex_lock(&shared->mutexGame);
	if (shared->nextPlayerId == MAX_PLAYER)
		return (pthread_mutex_unlock(&shared->mutexGame), ft_printf("Max number of player reached"), 0);
	t_player *p = &shared->players[shared->nextPlayerId];
	if (!initPlayerPosition(p))
		return (pthread_mutex_unlock(&shared->mutexGame), ft_printf("Can't find a valid position for the player"), 0);
	shared->map[p->y][p->x] = p->symbole;
	p->playerId = shared->nextPlayerId;
	playerId = p->playerId;
	p->symbole = team;
	shared->nextPlayerId++;
	pthread_mutex_unlock(&shared->mutexGame);
	return (1);
}

bool launchPlayer(char team)
{
	if (!initSharedMemory())
		return (false);
	if (!initPlayer(team))
		return (false);
	handleMove();
	return (true);
}
