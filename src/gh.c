#include "gh.h"

static int parse_cmd(char *arg[]);
static int parse_opt(char *arg[]);
static int generate_git_remote_url(char *url);
static void find_git_remote_url(FILE *git_config, char *buff);
static void adapt_git_remote_url(char *url);
static char *generate_firefox_instruction(char *url);

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
    else if (strcmp(*arg, "repo") == 0)
        result = REPO_CMD;
    else if (strcmp(*arg, "pulls") == 0)
        result = PR_CMD;
    else
        result = INVALID_CMD;

    return result;
}

static int parse_opt(char *arg[]) {
    int result;
    return result;
}

void add_instruction(struct Prompt *prompt) {
    if (prompt->cmd == HELP_CMD) {
        strcpy(prompt->instruction, "man gh");
        return;
    }

    char remote_url[MAX_STR_SIZE];
    int success = generate_git_remote_url(remote_url);
    if (!success) {
        strcpy(prompt->error, "Repository configuration not found.");
        return;
    }
    strcpy(prompt->instruction, generate_firefox_instruction(remote_url));

    if (prompt->cmd == PR_CMD) {
        strcat(prompt->instruction, "/pulls");
    }
}

static int generate_git_remote_url(char *url) {
    FILE *git_config = find_git_config();
    if (git_config != NULL) {
        find_git_remote_url(git_config, url);
        adapt_git_remote_url(url);
        fclose(git_config);
        return 1;
    }
    return 0;
}

#ifndef TESTING
FILE *find_git_config() {
    char git_config_path[MAX_STR_SIZE];
    getcwd((char *)&git_config_path, MAX_STR_SIZE);
    strcat(git_config_path, "/.git/config");
    return fopen(git_config_path, "r");
}
#endif

static void find_git_remote_url(FILE *git_config, char *buff) {
    char current_line[MAX_STR_SIZE], remote[MAX_STR_SIZE];
    char *remote_section;
    while (fgets(current_line, MAX_STR_SIZE, git_config)) {
        remote_section = strstr(current_line, "[remote \"origin\"]");

        if (remote_section != NULL) {
            fgets(remote, MAX_STR_SIZE, git_config);
            break;
        }
    }

    strcpy(buff, strstr(remote, "github.com"));
}

static void adapt_git_remote_url(char *url) {
    size_t remote_sz = strlen(url);
    url[remote_sz - 5] = '\0';
    char *colon = memchr(url, ':', remote_sz);
    if (colon != NULL)
        memset(colon, '/', 1);
}

static char *generate_firefox_instruction(char *url) {
    char firefox_bin_cmd[MAX_STR_SIZE] = "firefox --new-tab ";
    return strcat(firefox_bin_cmd, url);
}
