NAME = philo

CC = cc
CFLAGS = -Wall -Wextra -Werror -I./src
# Add -fsanitize=thread for debugging data races during development
# CFLAGS += -fsanitize=thread

SRC_DIR = src
OBJ_DIR = obj

SRC = $(SRC_DIR)/main.c \
      $(SRC_DIR)/init.c \
      $(SRC_DIR)/routine.c \
      $(SRC_DIR)/monitor.c \
      $(SRC_DIR)/time.c \
      $(SRC_DIR)/parsing/parsing.c

OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
