CC = gcc
DEFAULT_FLAGS = -o

run_tests:
	$(CC) $(DEFAULT_FLAGS) tests.out ./tests/*.c -l cmocka
	./tests.out
	rm ./tests.out

install:
	$(CC) $(DEFAULT_FLAGS) gh.out ./src/*.c

clean:
	rm ./*.out
