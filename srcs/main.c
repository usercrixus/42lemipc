#include "init.h"
#include "42libft/ft_base/libft.h"
#include "42libft/ft_printf/ft_printf.h"

t_shared *shared;
int playerId;

int main(int argc, char const *argv[])
{
	if (argc != 2 || !ft_isalpha(argv[1][0]))
		return (ft_printf("Usage error: ./program [a-zA-Z]"), 1);
	if (!launchGame(argv[1][0]))
		return (1);
	return 0;
}
