#include "sharedMemory.h"
#include "main.h"

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

bool initSharedMemory()
{
	key_t key = ftok(".gitmodules", 130);
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