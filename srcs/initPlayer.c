#include "initPlayer.h"
#include "main.h"
#include "game.h"
#include "sharedMemory.h"
#include "move.h"
#include "ai.h"
#include "42libft/ft_printf/ft_printf.h"
#include <signal.h>
#include <unistd.h>

static void handleMove()
{
	t_player *p = &shared->players[playerId];
	while (true)
	{
		sem_wait(&shared->semGame);
		if (isGameEnd())
		{
			sem_post(&shared->semGame);
			break;
		}
		usleep(1000*1000*0.01);
		if (!isAlive(p))
		{
			shared->map[p->y][p->x] = EMPTY_TILE;
			shared->playersAlive--;
			kill(shared->displayerPid, SIGUSR1);
			sem_post(&shared->semGame);
			break;
		}
		t_move bestMove = getBestMove();
		move(bestMove);
		if (!isAlive(p))
		{
			shared->map[p->y][p->x] = EMPTY_TILE;
			shared->playersAlive--;
			kill(shared->displayerPid, SIGUSR1);
			sem_post(&shared->semGame);
			break;
		}
		kill(shared->displayerPid, SIGUSR1);
		sem_post(&shared->semGame);
		usleep(1000*1000*0.01*shared->playersAlive);
	}
}

static int initPlayerPosition(t_player *p)
{
    // Try random empty tiles first for better dispersion
    int tries = MAX_MAP_HEIGHT * MAX_MAP_WIDTH * 2;
    for (int t = 0; t < tries; t++)
    {
        int y = rand() % MAX_MAP_HEIGHT;
        int x = rand() % MAX_MAP_WIDTH;
        if (shared->map[y][x] == EMPTY_TILE)
        {
            p->x = x;
            p->y = y;
            if (isAlive(p))
                return (true);
        }
    }
    // Fallback to deterministic scan if random failed (very unlikely)
    int startY = rand() % MAX_MAP_HEIGHT;
    int startX = rand() % MAX_MAP_WIDTH;
    for (int offset = 0; offset < MAX_MAP_HEIGHT * MAX_MAP_WIDTH; offset++)
    {
        int index = (startY * MAX_MAP_WIDTH + startX + offset) % (MAX_MAP_HEIGHT * MAX_MAP_WIDTH);
        int y = index / MAX_MAP_WIDTH;
        int x = index % MAX_MAP_WIDTH;
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
	sem_post(&shared->semInit);
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
