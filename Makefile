CC = g++
EXE = scsa

all:
	$(CC) src/*.cpp -o $(EXE) -std=c++17 -Wall -Wextra -Werror

format:
	clang-format -i src/*.cpp src/*.hpp

format-check:
	clang-format --dry-run --Werror src/*.cpp src/*.hpp

clean:
	rm -f $(EXE)

.PHONY: all format format-check lint lint-fix clean
