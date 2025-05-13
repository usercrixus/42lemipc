#include "init.h"

t_shared *shared;
int playerId;

int main(int argc, char const *argv[])
{
	if (argc != 2)
		return (1);
	if (!launchGame(argv[1][0]))
		return (1);
	return 0;
}
