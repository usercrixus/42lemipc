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
		t_move bestMove = getBestMove();
		move(bestMove);
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

static bool shmAlreadyExist(int key)
{
	int shm_id = shmget(key, sizeof(t_shared), 0666);
	if (shm_id == -1)
		return (perror("shmget existing"), false);
	shared = shmat(shm_id, NULL, 0);
	if (shared == (void *)-1)
		return (perror("shmat"), false);
	if (shared->isKilled)
		return (ft_printf("The game is over, you can try to create a new one"), false);
	return (true);
}

static bool shmCreation(int key, int shm_id)
{
	shared = shmat(shm_id, NULL, 0);
	if (shared == (void *)-1)
		return (perror("shmat"), false);
	shared->shmid = shm_id;
	shared->nextPlayerId = 0;
	shared->numberOfPlayer = 0;
	shared->isKilled = 0;
	for (int i = 0; i < MAX_PLAYER; i++)
		shared->players[i].pid = -1;
	pthread_mutex_init(&shared->mutexGame, 0);
	return (true);
}

static bool initSharedMemory()
{
	key_t key = ftok(".gitmodules", 110);
	if (key == -1)
	{
		perror("ftok");
		exit(1);
	}
	int shm_id = shmget(key, sizeof(t_shared), IPC_CREAT | IPC_EXCL | 0666);
	if (shm_id == -1)
	{
		if (errno == EEXIST)
			return (shmAlreadyExist(key));
		else
			return (perror("shmget"), false);
	}
	else
		return (shmCreation(key, shm_id));
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
		shared->isKilled = 1;
		shmctl(shared->shmid, IPC_RMID, NULL);
	}
	pthread_mutex_unlock(&shared->mutexGame);
	return (true);
}
