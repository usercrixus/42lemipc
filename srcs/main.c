#include "initPlayer.h"
#include "initDisplayer.h"
#include "main.h"
#include "42libft/ft_base/libft.h"
#include "42libft/ft_printf/ft_printf.h"
#include <time.h>
#include <unistd.h>

t_shared *shared;
int playerId;

static int isValidTeam(char c)
{
    // Allowed: '1'-'9', 'A'-'Z', 'a'-'z' (total MAX_TEAM)
    if (c >= '1' && c <= '9')
        return 1;
    if (c >= 'A' && c <= 'Z')
        return 1;
    if (c >= 'a' && c <= 'z')
        return 1;
    return 0;
}

int main(int argc, char const *argv[])
{
    srand((unsigned)(getpid() ^ time(NULL)));
    if (argc == 1)
    {
        if (!launchDisplayer())
            return (1);
        return (0);
    }
    else if (argc == 2)
    {
        if (!isValidTeam(argv[1][0]))
            return (ft_printf("Invalid team symbol. Use 1-9, A-Z, or a-i (max %d teams)\n", MAX_TEAM), 1);
        if (!launchPlayer(argv[1][0]))
            return (1);
        return (0);
    }
    else
    {
        return (ft_printf("Usage error: ./program [a-zA-Z]"), 1);
    }
}
