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
	for (size_t y = 0; y < (size_t)MAX_MAP_HEIGHT; y++)
	{
		for (size_t x = 0; x < (size_t)MAX_MAP_WIDTH; x++)
			shared->map[y][x] = EMPTY_TILE;
	}
}

static int is_pid_alive(pid_t pid)
{
	if (pid <= 0)
		return 0;
	if (kill(pid, 0) == 0)
		return 1;
	return (errno == EPERM);
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

static bool shmAlreadyExist(int key)
{
	int shm_id = shmget(key, 0, 0666);
	if (shm_id == -1)
		return (perror("shmget existing"), false);
	struct shmid_ds ds;
	if (shmctl(shm_id, IPC_STAT, &ds) == -1)
		return (perror("shmctl IPC_STAT"), false);
	if (ds.shm_segsz != sizeof(t_shared))
	{
		// Old segment has different size: recreate with new layout
		if (shmctl(shm_id, IPC_RMID, NULL) == -1)
			return (perror("shmctl IPC_RMID"), false);
		int new_id = shmget(key, sizeof(t_shared), IPC_CREAT | IPC_EXCL | 0666);
		if (new_id == -1)
			return (perror("shmget recreate"), false);
		return shmCreation(new_id);
	}
	shared = shmat(shm_id, NULL, 0);
	if (shared == (void *)-1)
		return (perror("shmat"), false);
	while (!(shared->isSegmentInitialized))
	{
		// spinlock
	}
    // If the displayer is not alive anymore, reset the segment so players can join
    if (shared->isGameStarted && !is_pid_alive(shared->displayerPid))
    {
        // Clean message queue and reset shared segment state
        destroyMSGQueue();
        shared->nextPlayerId = 0;
        initMap();
        sem_init(&shared->semGame, 1, 0);
        sem_init(&shared->semInit, 1, 1);
        shared->isGameStarted = false;
    }
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
}
