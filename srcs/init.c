#include "init.h"
#include <errno.h>

static void set_input_mode(bool enable)
{
	static struct termios oldt;
	static struct termios newt;

	if (enable)
	{
		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	}
	else
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

static void sig_redraw(int signo)
{
	if (signo == SIGUSR1)
	{
		drawMap();
		if (!(shared->players[playerId].isAlive))
		{
			--(shared->numberOfPlayer);
			set_input_mode(false);
			exit(0);
		}
	}
}

static void handle_input()
{
	char seq[3];

	seq[0] = 0;
	while (seq[0] != 'q' && shared->players[playerId].isAlive)
	{
		read(STDIN_FILENO, &seq[0], 1);
		if (seq[0] == 27)
		{
			read(STDIN_FILENO, &seq[1], 1);
			if (seq[1] == '[')
			{
				read(STDIN_FILENO, &seq[2], 1);
				if (seq[2] == 'A')
					move(TOP);
				else if (seq[2] == 'B')
					move(BOT);
				else if (seq[2] == 'C')
					move(RIGHT);
				else if (seq[2] == 'D')
					move(LEFT);
			}
		}
		else if (seq[0] == 'q')
			manageDeath();
		if (!(shared->players[playerId].isAlive))
			--(shared->numberOfPlayer);
	}
}

static void initMap()
{
	for (size_t y = 0; y < MAP_HEIGHT; y++)
	{
		for (size_t x = 0; x < MAP_WIDTH; x++)
		{
			shared->map[y][x] = '0';
		}
	}
}

static void initPlayer(char team)
{
	t_player *p = &shared->players[shared->nextPlayerId];
	p->playerId = shared->nextPlayerId;
	playerId = p->playerId;
	p->symbole = team;
	p->x = 0;
	p->y = 0;
	p->pid = getpid();
	p->isAlive = 1;
	shared->map[p->y][p->x] = p->symbole;
	shared->nextPlayerId++;
	shared->numberOfPlayer++;
}

static bool shmAlreadyExist(int key)
{
	int shm_id = shmget(key, sizeof(t_shared), 0666);
	if (shm_id == -1)
		return (perror("shmget existing"), false);
	shared = shmat(shm_id, NULL, 0);
	if (shared == (void *)-1)
		return (perror("shmat"), false);
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
	pthread_mutex_init(&shared->mut, 0);
	return (true);
}

static bool init_shared_memory()
{
	key_t key = ftok("faq.en.txt", 42);
	if (key == -1) {
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
	signal(SIGUSR1, sig_redraw);
	if (!init_shared_memory())
		return (false);
	if (shared->nextPlayerId == 0)
		initMap();
	initPlayer(team);
	set_input_mode(true);
	for (int i = 0; i < shared->nextPlayerId; i++)
			kill(shared->players[i].pid, SIGUSR1);
	handle_input();
	if (shared->numberOfPlayer == 0)
		shmctl(shared->shmid, IPC_RMID, NULL);
	set_input_mode(false);
	return (true);
}
