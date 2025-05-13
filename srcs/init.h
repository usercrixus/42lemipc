#include <unistd.h>
#include <termios.h>
#include "main.h"
#include "game.h"
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <pthread.h>

bool launchGame(char team);