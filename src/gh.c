#include "gh.h"

static int parse_cmd(char *arg[]);
static struct Option parse_opt(char *opt);
static int generate_git_remote_url(char *url);
static void find_git_remote_url(FILE *git_config, char *buff);
static void adapt_git_remote_url(char *url);
static char *generate_firefox_instruction(char *url);
static void handle_pulls_options(struct Prompt *p);
static void filter_open_prs(struct Prompt *p);
static void filter_closed_prs(struct Prompt *p);
static void filter_prs_by_author(struct Prompt *p, char *author);
static void filter_prs_to_review(struct Prompt *p);

struct Prompt parse_prompt(int argc, char *argv[]) {
    struct Prompt result = { DEFAULT_CMD };

    // The first arg will be `gh`, so we can skip it.
    int opt_i = 0;
    for (int i = 1; i < argc; i++) {
        if (i == 1) {
            result.cmd = parse_cmd(&argv[i]);
        } else {
            result.opts[opt_i] = parse_opt(argv[i]);
            opt_i++;
        }
    }

    return result;
}

static int parse_cmd(char *arg[]) {
    int result;

    if (strcmp(*arg, "help") == 0)
        result = HELP_CMD;
    else if (strcmp(*arg, "repo") == 0)
        result = REPO_CMD;
    else if (strcmp(*arg, "pulls") == 0)
        result = PULLS_CMD;
    else if (strcmp(*arg, "newpr") == 0)
        result = NEWPR_CMD;
    else
        result = INVALID_CMD;

    return result;
}

static struct Option parse_opt(char *opt) {
    struct Option o;

    char received_opt[MAX_STR_SIZE];
    strcpy(received_opt, opt);
    size_t opt_sz = strlen(received_opt);

    char *value_with_sep = memchr(received_opt, '=', opt_sz);
    if (value_with_sep != NULL) {
        char *only_value = value_with_sep+1;
        strcpy(o.value, only_value);
        char *break_line = memchr(o.value, '\n', strlen(o.value));
        if (break_line != NULL)
            memset(break_line, '\0', 1);

        memset(value_with_sep, '\0', 1);
    }

    strcpy(o.key, received_opt);

    return o;
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

    if (prompt->cmd == PULLS_CMD) strcat(prompt->instruction, "/pulls?q=is:pr");
    else if (prompt->cmd == NEWPR_CMD) strcat(prompt->instruction, "/compare");

    if (prompt->opts[0].key[0] != '\0') {
        handle_pulls_options(prompt);
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

static void handle_pulls_options(struct Prompt *p) {
    if (p->cmd != PULLS_CMD) return;

    for (int i = 0; i < MAX_CMD_OPTS; i++) {
        if (strcmp(p->opts[i].key, "--open") == 0) {
            filter_open_prs(p);
        } else if (strcmp(p->opts[i].key, "--closed") == 0) {
            filter_closed_prs(p);
        } else if (strcmp(p->opts[i].key, "--author") == 0) {
            filter_prs_by_author(p, p->opts[i].value);
        } else if (strcmp(p->opts[i].key, "--to-review") == 0) {
            filter_open_prs(p);
            filter_prs_to_review(p);
        }
    }
}

static void filter_open_prs(struct Prompt *p) {
    strcat(p->instruction, "+is:open");
}

static void filter_closed_prs(struct Prompt *p) {
    strcat(p->instruction, "+is:closed");
}

static void filter_prs_by_author(struct Prompt *p, char *author) {
    char author_param[MAX_STR_SIZE];
    strcpy(author_param, "+author:");
    strcat(author_param, author);
    strcat(p->instruction, author_param);
}

static void filter_prs_to_review(struct Prompt *p) {
    strcat(p->instruction, "+user-review-requested:@me");
}
