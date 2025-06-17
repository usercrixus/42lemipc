#pragma once

#include "main.h"
#include <stdbool.h>

void drawMap();
void move(t_move move);
int isAlive(t_player *p);
bool isGameEnd();
t_move getBestMove();