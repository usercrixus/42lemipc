#include "initDisplayer.h"
#include "sharedMemory.h"
#include "main.h"
#include "42libft/ft_printf/ft_printf.h"
#include <unistd.h>
#include <signal.h>

static void drawMap()
{
	for (size_t y = 0; y < MAP_HEIGHT; y++)
	{
		for (size_t x = 0; x < MAP_WIDTH; x++)
		{
			ft_printf(" %c ", shared->map[y][x]);
		}
		ft_printf("\n");
	}
	ft_printf("\n");
}

bool launchDisplayer()
{
	if (!initSharedMemory())
		return (false);
	sem_wait(&shared->semInit);
	shared->displayerPid = getpid();
	shared->isGameStarted = true;
	signal(SIGUSR1, drawMap);
	sem_post(&shared->semInit);
	sem_post(&shared->semGame); // unblock the game
	destroySharedMemory();
	destroyMSGQueue();
	return (true);
}