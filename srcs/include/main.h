#pragma once

#include <sys/types.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>

#define MAP_HEIGHT 10
#define MAP_WIDTH 30
#define EMPTY_TILE '0'
#define TEAM_1_TILE '1'
#define TEAM_2_TILE '2'
#define MAX_PLAYER 10
#define MSGQ_KEY 4242
#define MSG_TYPE_TARGET 1

typedef enum e_move
{
	TOP,
	BOT,
	LEFT,
	RIGHT,
	STAY
} t_move;

typedef struct s_player
{
	int x;
	int y;
	char symbole;
	int playerId;
} t_player;

typedef struct s_shared
{
	int nextPlayerId;
	int map[MAP_HEIGHT][MAP_WIDTH];
	t_player players[MAX_PLAYER];
	int sharedMemoryId;
	bool isGameStarted;
	pid_t displayerPid;
	sem_t semGame;
	sem_t semInit;
} t_shared;

typedef struct s_msg_target
{
	long mtype;
	int targetX;
	int targetY;
	char team;
} t_msg_target;

extern t_shared *shared;
extern int playerId;