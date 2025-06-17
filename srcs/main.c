#include "initPlayer.h"
#include "initDisplayer.h"
#include "main.h"
#include "42libft/ft_base/libft.h"
#include "42libft/ft_printf/ft_printf.h"

t_shared *shared;
int playerId;

int main(int argc, char const *argv[])
{
	if (argc == 1)
	{
		if (!launchDisplayer())
			return (1);
		return (0);
	}
	else if (argc == 2)
	{
		if (!launchPlayer(argv[1][0]))
			return (1);
		return (0);
	}
	else
	{
		return (ft_printf("Usage error: ./program [a-zA-Z]"), 1);
	}
}
