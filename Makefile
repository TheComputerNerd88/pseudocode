CC = g++
EXE = scsa

all:
	$(CC) src/*.cpp -o $(EXE) -std=c++14 -Wall -Wextra -Werror

