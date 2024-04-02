#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <prompt.h>

#define DEFAULT_CMD 1
#define REPO_CMD 1
#define HELP_CMD 2
#define PULLS_CMD 3
#define NEWPR_CMD 4

struct Context {
    char instruction[MAX_STR_SIZE];
    char error[MAX_STR_SIZE];
    char warn[MAX_STR_SIZE];
    struct Prompt prompt;
};

int cmd_table(char *cmd_str);
void add_instruction(struct Context *context);
FILE *find_git_config();
