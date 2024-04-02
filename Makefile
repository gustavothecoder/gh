CC = gcc
CC_FLAGS = -g -o
GH_DIR=~/.local/bin
MAN_DIR=~/.local/man/man1

build_tests:
	$(CC) $(CC_FLAGS) tests.out ./tests/*.c ./src/gh.c -lprompt -lcmocka -Wl,--wrap=find_git_config -DTESTING
install:
	mkdir -p $(GH_DIR)
	mkdir -p $(MAN_DIR)
	$(CC) $(CC_FLAGS) $(GH_DIR)/gh ./src/*.c -lprompt
	cp ./docs/gh.1 $(MAN_DIR)/gh.1
	gzip $(MAN_DIR)/gh.1
uninstall:
	rm -f $(GH_DIR)/gh
	rm -f $(MAN_DIR)/gh.1.gz
clean:
	rm -f ./*.out
