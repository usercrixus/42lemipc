#pragma once

#include <sys/types.h>
#include <stdbool.h>
#include <semaphore.h>

// Fixed grid size (3:1 proportion)
#define MAP_HEIGHT 30
#define MAP_WIDTH 30
#define MAX_TEAM 9+26*2 // 9 digits (1-9) + 26 uppercase (A-Z) + 24 lowercase (a-z)
#define EMPTY_TILE '0'
#define MAX_PLAYER (MAP_HEIGHT * MAP_WIDTH / 2)

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
	_Atomic int playersAlive;
    int map[MAP_HEIGHT][MAP_WIDTH];
    t_player players[MAX_PLAYER];
    int sharedMemoryId;
    bool isGameStarted;
	bool isEndGame;
    sem_t semGame;
	sem_t semInit;
	bool isSegmentInitialized;
} t_shared;

typedef struct s_msg_target
{
	long mtype;
	int targetX;
	int targetY;
} t_msg_target;

extern t_shared *shared;
extern int playerId;
