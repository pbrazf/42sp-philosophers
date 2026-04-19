NAME = philo

CC = cc
CFLAGS = -Wall -Wextra -Werror

OBJ_DIR = obj

SRC = main.c \
      init.c \
      parsing.c \
      time.c \
      print.c \
      routine.c \
      monitor.c

OBJ = $(SRC:%.c=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.c philosophers.h
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
