
NAME = map_obj_viw

CC = g++
SRC = ./srcs/main.cpp ./srcs/img_matrix.cpp 
OBJS = $(SRC:.cpp=.o)

# Flags de compilação
CXXFLAGS =  -fPIC -I/usr/include/opencv4 `pkg-config --cflags Qt5Widgets`

# Flags de link
LDFLAGS = `pkg-config --libs opencv4 Qt5Widgets`

# Compilar objetos
%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

# Target principal
all: $(NAME)

# Linkar objetos
$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $(NAME) $(LDFLAGS)

# Limpar objetos
clean:
	rm -f $(OBJS)

# Limpar objetos + executável
fclean: clean
	rm -f $(NAME)

# Recompilar tudo
re: fclean
	make all

# Rodar executável
s: re
	./$(NAME)

# Docker (mantido)
docker:
	xhost +local:docker
	docker-compose up --build

