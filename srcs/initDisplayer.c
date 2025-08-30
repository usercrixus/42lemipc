#include "initDisplayer.h"
#include "sharedMemory.h"
#include "main.h"
#include "game.h"
#include "42libft/ft_printf/ft_printf.h"
#include <unistd.h>
#include <signal.h>

static volatile sig_atomic_t g_stalemate = 0;

static const char *color_for(char c)
{
	static const char *palette[] = {
		"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m", "\033[91m", "\033[92m", "\033[93m"};
	size_t n = sizeof(palette) / sizeof(palette[0]);
	if (c == EMPTY_TILE)
		return "\033[0m";
	int idx = 0;
	if (c >= '1' && c <= '9')
		idx = c - '1';
	else if (c >= 'A' && c <= 'Z')
		idx = c - 'A';
	else if (c >= 'a' && c <= 'z')
		idx = c - 'a';
	return palette[idx % (int)n];
}

static void drawMap(int sig)
{
    (void)sig;
    static char last[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];
    static int init = 0;
    int same = 1;
    for (size_t y = 0; y < (size_t)MAX_MAP_HEIGHT; y++)
    {
        for (size_t x = 0; x < (size_t)MAX_MAP_WIDTH; x++)
        {
            char c = shared->map[y][x];
            const char *col = color_for(c);
            ft_printf(" %s%c\033[0m ", col, c);
            if (!init || last[y][x] != c)
                same = 0;
            last[y][x] = c;
        }
        ft_printf("\n");
    }
	ft_printf("\n");
	init = 1;
	static int stagnant = 0;
	if (same)
		stagnant++;
	else
		stagnant = 0;
	if (stagnant > 50)
		g_stalemate = 1;
}

bool launchDisplayer()
{
    if (!initSharedMemory())
        return (false);
    sem_wait(&shared->semInit);
    shared->displayerPid = getpid();
    shared->playersAlive = shared->nextPlayerId - 1;
    shared->isGameStarted = true;
    signal(SIGUSR1, drawMap);
    sem_post(&shared->semInit);
    drawMap(0);
    sem_post(&shared->semGame);
    while (true)
    {
        pause();
        if (isGameEnd() || g_stalemate)
            break;
    }
    destroyMSGQueue();
    destroySharedMemory();
    return (true);
}
