#include "initDisplayer.h"
#include "sharedMemory.h"
#include "main.h"
#include "game.h"
#include "42libft/ft_printf/ft_printf.h"
#include <unistd.h>
#include <signal.h>

static volatile sig_atomic_t g_stalemate = 0;

static const char *colorFor(char c)
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

static void termGoto(size_t row, size_t col)
{
    ft_printf("\033[%d;%dH", (int)row, (int)col);
}

static void drawCell(size_t y, size_t x, char c)
{
    const char *col = colorFor(c);
    size_t row = y + 1;
    size_t start_col = x * 3 + 1;
    termGoto(row, start_col);
    ft_printf(" %s%c\033[0m ", col, c);
}

static void fullDraw(char last[MAP_HEIGHT][MAP_WIDTH], int *p_init)
{
    ft_printf("\033[2J\033[H");
    for (size_t y = 0; y < (size_t)MAP_HEIGHT; y++)
    {
        for (size_t x = 0; x < (size_t)MAP_WIDTH; x++)
        {
            char c = shared->map[y][x];
            last[y][x] = c;
            drawCell(y, x, c);
        }
    }
    termGoto(MAP_HEIGHT + 1, 1);
    ft_printf("\n");
    *p_init = 1;
}

static void drawMap()
{
    static char last[MAP_HEIGHT][MAP_WIDTH];
    static int init = 0;
    int same = 1;
    if (!init)
        fullDraw(last, &init);
    for (size_t y = 0; y < (size_t)MAP_HEIGHT; y++)
    {
        for (size_t x = 0; x < (size_t)MAP_WIDTH; x++)
        {
            char c = shared->map[y][x];
            if (last[y][x] != c)
            {
                same = 0;
                last[y][x] = c;
                drawCell(y, x, c);
            }
        }
    }
    termGoto(MAP_HEIGHT + 1, 1);
    static int stagnant = 0;
    if (same)
        stagnant++;
    else
        stagnant = 0;
    if (stagnant > 50)
        g_stalemate = 1;
}

void quitDisplayer(int sig)
{
	(void)sig;
	shared->isEndGame = true;
}

bool launchDisplayer()
{
    signal(SIGINT, quitDisplayer);
    if (!initSharedMemory())
        return (false);
    sem_wait(&shared->semInit);
    shared->playersAlive = shared->nextPlayerId;
    shared->isGameStarted = true;
    sem_post(&shared->semInit);
    drawMap(0);
    sem_post(&shared->semGame);
    while (!isGameEnd())
    {
        drawMap();
        usleep(1000*1000*0.1);
    }
    drawMap();
    return (true);
}
