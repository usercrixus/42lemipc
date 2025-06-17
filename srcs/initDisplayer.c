#include "initDisplayer.h"
#include "sharedMemory.h"
#include "main.h"

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
	shared->displayPid = getpid();
	shared->isGameStarted = true;
	drawMap();
}