#pragma once

#include "42libft/ft_printf/ft_printf.h"
#include "main.h"
#include <signal.h>
#include <stdbool.h>

void drawMap();
void manageDeath(t_player *p);
void move(t_move move);
int isAlive(t_player *p);
t_move getBestMove();