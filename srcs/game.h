#pragma once

#include "42libft/ft_printf/ft_printf.h"
#include "main.h"
#include <signal.h>
#include <stdbool.h>
#include <sys/msg.h>

#define MSGQ_KEY 4242
#define MSG_TYPE_TARGET 1

typedef struct s_msg_target {
    long mtype;
    int targetX;
    int targetY;
    char team;
} t_msg_target;


void drawMap();
void manageDeath(t_player *p);
void move(t_move move);
int isAlive(t_player *p);
void setIsAlive();
bool isGameEnd();
t_move getBestMove();