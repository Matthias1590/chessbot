CPP = g++
CPPFLAGS = -std=c++20 -Wall -Wextra -Werror -pedantic -g -Ofast

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:src/%.cpp=obj/%.o)

NAME = chess

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CPPFLAGS) -o $@ $^

obj/%.o: src/%.cpp
	$(CPP) $(CPPFLAGS) -c -o $@ $<
