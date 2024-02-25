CC = gcc
CC_FLAGS = -g -o
GH_PATH=~/.local/bin/gh
MAN_PATH=~/.local/man/man1/gh.1

build_tests:
	$(CC) $(CC_FLAGS) tests.out ./tests/*.c ./src/gh.c -lcmocka -Wl,--wrap=find_git_config -DTESTING
install:
	$(CC) $(CC_FLAGS) $(GH_PATH) ./src/*.c
	cp ./docs/gh.1 $(MAN_PATH)
	gzip $(MAN_PATH)
uninstall:
	rm -f $(GH_PATH)
	rm -f $(MAN_PATH).gz
clean:
	rm -f ./*.out
