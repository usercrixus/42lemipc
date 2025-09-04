OBJ = \
	srcs/ai.o \
	srcs/game.o \
	srcs/initDisplayer.o \
	srcs/initPlayer.o \
	srcs/main.o \
	srcs/move.o \
	srcs/sharedMemory.o

all: submodule main.out

submodule:
	git submodule update --init --recursive

main.out: $(OBJ)
	make -C srcs/42libft/
	gcc -Wall -Wextra -Werror -Isrcs/include $^ -pthread -Lsrcs/42libft/ft_printf/ -lftprintf -o $@

%.o: %.c
	gcc -Wall -Wextra -Werror -Isrcs/include -pthread -c $^ -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f main.out

re: fclean main.out 

.PHONY: clean fclean re