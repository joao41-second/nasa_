NAME = map_obj_viw

CC = g++
FLAG = `pkg-config --cflags --libs opencv4`
SRC = ./srcs/main.cpp

SRCS = $(SRC:.cpp=.o)

%.o:%.cpp
	$(CC) $(FLAG) -c $< -o $@


all: $(NAME)

	g++ -o $(NAME) $(SRCS) `pkg-config --cflags --libs opencv4`

$(NAME): $(SRCS)

docker:
	 xhost +local:docker
	 docker-compose up --build

re:fclean
	make all

clean:
	rm -fr $(SRCS)

fclean:clean
	rm -fr $(SRCS) $(NAME)

s:re
	./$(NAME)
	
