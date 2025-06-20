#include "sharedMemory.h"
#include "main.h"
#include <sys/ipc.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include "42libft/ft_printf/ft_printf.h"

static void initMap()
{
	for (size_t y = 0; y < MAP_HEIGHT; y++)
	{
		for (size_t x = 0; x < MAP_WIDTH; x++)
			shared->map[y][x] = EMPTY_TILE;
	}
}

static bool shmAlreadyExist(int key)
{
	int shm_id = shmget(key, sizeof(t_shared), 0666);
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

static bool shmCreation(int shm_id)
{
	shared = shmat(shm_id, NULL, 0);
	if (shared == (void *)-1)
		return (perror("shmat"), false);
	shared->sharedMemoryId = shm_id;
	shared->nextPlayerId = 0;
	shared->isGameStarted = false;
	initMap();
	sem_init(&shared->semGame, 1, 0);
	sem_init(&shared->semInit, 1, 1);
	shared->isSegmentInitialized = true;
	return (true);
}

bool initSharedMemory()
{
	key_t key = ftok(".gitmodules", 130);
	if (key == -1)
		return (perror("ftok"), false);
	int shm_id = shmget(key, sizeof(t_shared), IPC_CREAT | IPC_EXCL | 0666);
	if (shm_id == -1)
	{
		if (errno == EEXIST)
			return (shmAlreadyExist(key));
		else
			return (perror("shmget"), false);
	}
	else
		return (shmCreation(shm_id));
}

void destroySharedMemory()
{
	shmctl(shared->sharedMemoryId, IPC_RMID, NULL);
}

void destroyMSGQueue()
{
	int msgid = msgget(MSGQ_KEY, 0666);
	if (msgid >= 0)
		msgctl(msgid, IPC_RMID, NULL);
	shared->isGameStarted = 1;
}