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
	bool isDead = false;
	while (!isDead && !isGameEnd())
	{
		pthread_mutex_lock(&shared->mutexGame);
		if (!isAlive(&p))
		{
			shared->map[p.y][p.x] = EMPTY_TILE;
			isDead = true;
		}
		t_move bestMove = getBestMove();
		move(bestMove);
		if (!isAlive(&p))
		{
			shared->map[p.y][p.x] = EMPTY_TILE;
			isDead = true;
		}
		pthread_mutex_unlock(&shared->mutexGame);
	}
}

static int initPlayerPosition(t_player *p)
{
	for (int i = 0; i < MAP_HEIGHT; i++)
	{
		for (int j = 0; j < MAP_WIDTH; j++)
		{
			if (shared->map[i][j] == EMPTY_TILE)
			{
				p->x = j;
				p->y = i;
				if (isAlive(p))
					return (true);
			}
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
