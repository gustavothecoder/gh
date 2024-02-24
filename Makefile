CC = gcc
CC_FLAGS = -g -o
GH_DIR=~/.local/bin/gh

build_tests:
	$(CC) $(CC_FLAGS) tests.out ./tests/*.c ./src/gh.c -lcmocka -Wl,--wrap=find_git_config -DTESTING

install:
	$(CC) $(CC_FLAGS) $(GH_DIR) ./src/*.c

clean:
	rm -f ./*.out
	rm -f $(GH_DIR)
