#include "initPlayer.h"
#include "main.h"
#include "game.h"
#include "sharedMemory.h"
#include "move.h"
#include "ai.h"
#include "42libft/ft_printf/ft_printf.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

static void handleMove()
{
	t_player *p = &shared->players[playerId];
	while (!shared->isEndGame)
	{
		if (sem_wait(&shared->semGame) == -1)
		{
			perror("sem_wait");
			break;
		}
		if (isGameEnd())
		{
			sem_post(&shared->semGame);
			break;
		}
		usleep(1000 * 1000 * 0.01);
		if (!isAlive(p))
		{
			shared->map[p->y][p->x] = EMPTY_TILE;
			sem_post(&shared->semGame);
			break;
		}
		t_move bestMove = getBestMove();
		move(bestMove);
		if (!isAlive(p))
		{
			shared->map[p->y][p->x] = EMPTY_TILE;
			sem_post(&shared->semGame);
			break;
		}
		sem_post(&shared->semGame);
		usleep(1000 * 1000 * 0.01 * shared->playersAlive);
	}
}

static int initPlayerPosition(t_player *p)
{
	// Try random empty tiles first for better dispersion
	int tries = MAP_HEIGHT * MAP_WIDTH * 2;
	for (int t = 0; t < tries; t++)
	{
		int y = rand() % MAP_HEIGHT;
		int x = rand() % MAP_WIDTH;
		if (shared->map[y][x] == EMPTY_TILE)
		{
			p->x = x;
			p->y = y;
			if (isAlive(p))
				return (true);
		}
	}
	// Fallback to deterministic scan if random failed (very unlikely)
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
	sem_wait(&shared->semInit);
	if (shared->isGameStarted)
		return (sem_post(&shared->semInit), ft_printf("The game already started, sorry"), 0);
	if (shared->nextPlayerId == MAX_PLAYER)
		return (sem_post(&shared->semInit), ft_printf("Max number of player reached"), 0);
	t_player *p = &shared->players[shared->nextPlayerId];
	p->symbole = team;
	if (!initPlayerPosition(p))
		return (sem_post(&shared->semInit), ft_printf("Can't find a valid position for the player"), 0);
	p->playerId = shared->nextPlayerId;
	playerId = p->playerId;
	shared->map[p->y][p->x] = p->symbole;
	shared->nextPlayerId++;
	shared->playersAlive++;
	sem_post(&shared->semInit);
	return (1);
}

void quitPlayer(int sig)
{
	(void)sig;
	shared->isEndGame = true;
	sem_post(&shared->semGame);
	// we can safely sem_post here, even if it look weird
	// i cant find a beautyfull way to manage it cause of signal unsafety with sem_wait...
}

bool launchPlayer(char team)
{
	if (!initSharedMemory(quitPlayer))
		return (false);
	if (!initPlayer(team))
		return (false);
	handleMove();
	if (--shared->playersAlive == 0)
	{
		destroyMSGQueue();
		destroySharedMemory();
	}
	return (true);
}
