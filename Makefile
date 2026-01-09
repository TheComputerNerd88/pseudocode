CC = g++
EXE = scsa

all:
	$(CC) src/*.cpp -o $(EXE) -std=c++17 -Wall -Wextra -Werror

