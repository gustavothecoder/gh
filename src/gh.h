#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_STR_SIZE 512

#define INVALID_CMD -1
#define MAX_CMD_OPTS 5
#define DEFAULT_CMD 1
#define REPO_CMD 1
#define HELP_CMD 2
#define PULLS_CMD 3
#define NEWPR_CMD 4

#define FIREFOX_INSTRUCTOR 1
#define TERM_INSTRUCTOR 2

struct Option {
    char key[MAX_STR_SIZE];
    char value[MAX_STR_SIZE];
};

struct Prompt {
    int cmd;
    char instruction[MAX_STR_SIZE];
    char error[MAX_STR_SIZE];
    char warn[MAX_STR_SIZE];
    struct Option opts[MAX_CMD_OPTS];
};

struct Prompt parse_prompt(int argc, char *argv[]);
void add_instruction(struct Prompt *prompt);
FILE *find_git_config();
