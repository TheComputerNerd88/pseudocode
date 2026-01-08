CC = g++
EXE = scsa

all:
	$(CC) src/*.cpp -o $(EXE) -std=c++11 -Wall -Wextra -Werror

