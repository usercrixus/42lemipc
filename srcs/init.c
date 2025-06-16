#include "init.h"
#include <errno.h>

static void manageSignal(int signo)
{
	if (signo == SIGUSR1)
	{
		drawMap();
		if (!(shared->players[playerId].isAlive))
		{
			shared->players[playerId].pid = -1;
			--(shared->numberOfPlayer);
			pthread_mutex_unlock(&shared->mutexGame);
			exit(0);
		}
	}
	else if (signo == SIGINT)
	{
		manageDeath(&shared->players[playerId]);
		shared->players[playerId].pid = -1;
		--(shared->numberOfPlayer);
		for (int i = 0; i < MAX_PLAYER; i++)
		{
			if (shared->players[i].pid != -1)
				kill(shared->players[i].pid, SIGUSR1);
		}
		if (shared->numberOfPlayer == 0 && !shared->isKilled)
		{
			shared->isKilled = 1;
			shmctl(shared->shmid, IPC_RMID, NULL);
		}
		exit(0);
	}
}

static void handleMove()
{
	while (shared->players[playerId].isAlive)
	{
		pthread_mutex_lock(&shared->mutexGame);

		// 1) Re-evaluate survival based on current map
		setIsAlive();
		if (!shared->players[playerId].isAlive)
		{
			// clean up and exit
			manageDeath(&shared->players[playerId]);
			pthread_mutex_unlock(&shared->mutexGame);
			exit(0);
		}

		// 2) Compute and execute aggressive move
		t_move bestMove = getBestMove();
		move(bestMove);

		// 3) Check again after moving
		setIsAlive();

		pthread_mutex_unlock(&shared->mutexGame);
		sleep(1);
	}
}

static void initMap()
{
	for (size_t y = 0; y < MAP_HEIGHT; y++)
	{
		for (size_t x = 0; x < MAP_WIDTH; x++)
			shared->map[y][x] = EMPTY_TILE;
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
	p->playerId = shared->nextPlayerId;
	playerId = p->playerId;
	p->symbole = team;
	p->pid = getpid();
	p->isAlive = 1;
	shared->map[p->y][p->x] = p->symbole;
	shared->nextPlayerId++;
	shared->numberOfPlayer++;
	pthread_mutex_unlock(&shared->mutexGame);
	return (1);
}



bool launchGame(char team)
{
	signal(SIGUSR1, manageSignal);
	signal(SIGINT, manageSignal);
	if (!initSharedMemory())
		return (false);
	if (shared->nextPlayerId == 0)
		initMap();
	if (!initPlayer(team))
		return (false);
	for (int i = 0; i < MAX_PLAYER; i++)
	{
		if (shared->players[i].pid != -1)
			kill(shared->players[i].pid, SIGUSR1);
	}
	handleMove();
	pthread_mutex_lock(&shared->mutexGame);
	if (shared->numberOfPlayer == 0 && !shared->isKilled)
	{
		int msgid = msgget(MSGQ_KEY, 0666);
		if (msgid >= 0)
			msgctl(msgid, IPC_RMID, NULL);
		shared->isKilled = 1;
		shmctl(shared->shmid, IPC_RMID, NULL);
	}
	pthread_mutex_unlock(&shared->mutexGame);
	return (true);
}
