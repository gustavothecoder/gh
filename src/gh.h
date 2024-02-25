#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_STR_SIZE 255

#define MAX_CMD_OPTS 1
#define DEFAULT_CMD 1
#define HOME_CMD 1
#define HELP_CMD 2

#define FIREFOX_INSTRUCTOR 1
#define TERM_INSTRUCTOR 2

struct Prompt {
    int cmd;
    int opts[MAX_CMD_OPTS];
    char instruction[MAX_STR_SIZE];
};

struct Prompt parse_prompt(int argc, char *argv[]);
void add_instruction(struct Prompt *prompt);
FILE *find_git_config(const char *path);