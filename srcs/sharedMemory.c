#include "sharedMemory.h"
#include "main.h"
#include <sys/ipc.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include <signal.h>
#include "42libft/ft_printf/ft_printf.h"

static void initMap()
{
	for (size_t y = 0; y < (size_t)MAP_HEIGHT; y++)
	{
		for (size_t x = 0; x < (size_t)MAP_WIDTH; x++)
			shared->map[y][x] = EMPTY_TILE;
	}
}

static bool shmCreation(int shm_id)
{
	shared = shmat(shm_id, NULL, 0);
	if (shared == (void *)-1)
		return (perror("shmat"), false);
	shared->sharedMemoryId = shm_id;
	shared->nextPlayerId = 0;
	shared->playersAlive = 0;
	shared->isGameStarted = false;
	shared->isEndGame = false;
	initMap();
	sem_init(&shared->semGame, 1, 0);
	sem_init(&shared->semInit, 1, 1);
	shared->isSegmentInitialized = true;
	return (true);
}

static bool shmAlreadyExist(int key)
{
	int shm_id = shmget(key, 0, 0666);
	if (shm_id == -1)
		return (perror("shmget existing"), false);
	shared = shmat(shm_id, NULL, 0);
	if (shared == (void *)-1)
		return (perror("shmat"), false);
	while (!(shared->isSegmentInitialized))
	{
		// spinlock
	}
	return (true);
}

static bool setSignal(void (*signalHandler)(int))
{
	struct sigaction sa = {0};
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		perror("sigaction(SIGINT)");
		return (false);
	}
	return (true);
}

static bool activeSignals(bool status, sigset_t *all, sigset_t *old)
{
	if (!status)
	{
		sigfillset(all);
		if (sigprocmask(SIG_BLOCK, all, old) == -1)
			return (perror("sigprocmask(SIG_BLOCK)"), false);
	}
	else
	{
		if (sigprocmask(SIG_SETMASK, old, NULL) == -1)
			return (perror("sigprocmask(SIG_SETMASK)"), false);
	}
	return (true);
}

bool initSharedMemory(void (*signalHandler)(int))
{
	bool status = false;
	sigset_t all;
	sigset_t old;

	if (!activeSignals(false, &all, &old))
		return (activeSignals(true, &all, &old), false);
	if (!setSignal(signalHandler))
		return (activeSignals(true, &all, &old), false);
	key_t key = ftok(".gitmodules", 130);
	if (key == -1)
	{
		perror("ftok");
		return (activeSignals(true, &all, &old), false);
	}
	int shm_id = shmget(key, sizeof(t_shared), IPC_CREAT | IPC_EXCL | 0666);
	if (shm_id == -1)
	{
		if (errno == EEXIST)
			status = shmAlreadyExist(key);
		else
		{
			perror("shmget");
			status = false;
		}
	}
	else
		status = shmCreation(shm_id);
	return (activeSignals(true, &all, &old), status);
}

void destroySharedMemory()
{
	sem_destroy(&shared->semInit);
	sem_destroy(&shared->semGame);
	shmctl(shared->sharedMemoryId, IPC_RMID, NULL);
}

void destroyMSGQueue()
{
	key_t key = ftok(".gitmodules", 130);
	if (key == -1)
		perror("ftok");
	else
	{
		int msgid = msgget(key, 0666);
		if (msgid >= 0)
			msgctl(msgid, IPC_RMID, NULL);
	}
}
