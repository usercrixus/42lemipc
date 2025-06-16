#pragma once

#include <sys/types.h>
#include <pthread.h>

#define MAP_HEIGHT 10
#define MAP_WIDTH 30
#define EMPTY_TILE '0'
#define TEAM_1_TILE '1'
#define TEAM_2_TILE '2'
#define MAX_PLAYER 10

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
	pid_t pid;
	int isAlive;
} t_player;

typedef struct s_shared
{
	int nextPlayerId;
	int map[MAP_HEIGHT][MAP_WIDTH];
	t_player players[MAX_PLAYER];
	int numberOfPlayer;
	int shmid;
	int isKilled;
	pthread_mutex_t mutexGame;
} t_shared;

extern t_shared *shared;
extern int playerId;