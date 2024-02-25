#include "gh.h"

static int parse_cmd(char *arg[]);
static int parse_opt(char *arg[]);

struct Prompt parse_prompt(int argc, char *argv[]) {
    struct Prompt result = { DEFAULT_CMD };

    // The first arg will be `gh`, so we can skip it.
    int opt_i = 0;
    for (int i = 1; i < argc; i++) {
        if (i == 1) {
            result.cmd = parse_cmd(&argv[i]);
        } else {
            result.opts[opt_i] = parse_opt(&argv[i]);
            opt_i++;
        }
    }

    return result;
}

static int parse_cmd(char *arg[]) {
    int result;

    if (strcmp(*arg, "-h") == 0 || strcmp(*arg, "--help") == 0)
        result = HELP_CMD;
    else if (strcmp(*arg, "-r") == 0 || strcmp(*arg, "--repo") == 0)
        result = REPO_CMD;
    else
        result = INVALID_CMD;

    return result;
}

static int parse_opt(char *arg[]) {
    int result;
    return result;
}

void add_instruction(struct Prompt *prompt) {
    switch (prompt->cmd) {
    case HELP_CMD:
        strcpy(prompt->instruction, "man gh");
        break;
    case REPO_CMD:
        if (0) {}; // Just to avoid CCLS expected expression error
        char git_config_path[MAX_STR_SIZE];
        getcwd((char *)&git_config_path, MAX_STR_SIZE);
        strcat(git_config_path, "/.git/config");

        FILE *git_config = find_git_config(git_config_path);
        if (git_config == NULL) {
            strcpy(prompt->error, "Repository configuration not found.");
            break;
        }

        char current_line[MAX_STR_SIZE], remote[MAX_STR_SIZE];
        char *remote_section;
        while (fgets(current_line, MAX_STR_SIZE, git_config)) {
            remote_section = strstr(current_line, "[remote \"origin\"]");

            if (remote_section != NULL) {
                fgets(remote, MAX_STR_SIZE, git_config);
                break;
            }
        }

        fclose(git_config);

        char *remote_url = strstr(remote, "github.com");
        size_t remote_sz = strlen(remote_url);
        remote_url[remote_sz - 5] = '\0';
        char *colon = memchr(remote_url, ':', remote_sz);
        if (colon != NULL)
            memset(colon, '/', 1);
        char firefox_bin_cmd[MAX_STR_SIZE] = "firefox --new-tab ";
        strcat(firefox_bin_cmd, remote_url);
        strcpy(prompt->instruction, firefox_bin_cmd);

        break;
    }
}

#ifndef TESTING
FILE *find_git_config(const char *path) {
    return fopen(path, "r");
}
#endif
