OBJ = \
	srcs/game.o \
	srcs/init.o \
	srcs/main.o \

main.out: $(OBJ)
	make -C srcs/42libft/
	gcc -Wall -Wextra -Werror $^ -Lsrcs/42libft/ft_printf/ -lftprintf -o $@

%.o: %c
	gcc -Wall -Wextra -Werror -c $^ -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f main.out

re: fclean main.out 

.PHONY: clean fclean re