#include "gh.h"

static int parse_cmd(char *arg[]);
static int generate_git_remote_url(char *url);
static void find_git_remote_url(FILE *git_config, char *buff);
static void adapt_git_remote_url(char *url);
static char *generate_firefox_instruction(char *url);
static void close_firefox_argument(struct Context *c);
static void handle_pulls_options(struct Context *c);
static void filter_open_prs(struct Context *c);
static void filter_closed_prs(struct Context *c);
static void filter_prs_by_author(struct Context *c, char *author);
static void filter_prs_to_review(struct Context *c);
static void handle_newpr_options(struct Context *c);
static void set_destination_and_source(struct Context *c, char *dest_src);
static void set_template(struct Context *c, char *template);
static void set_title(struct Context *c, char *title);
static void set_assignees(struct Context *c, char *assignees);
static void set_labels(struct Context *c, char *labels);
static void assure_query_param_support(struct Context *c);
static void warn_missing_branches(struct Context *c);

int cmd_table(char *cmd_str) {
    int result;

    if (compare_command(cmd_str, "help") ||
        compare_command(cmd_str, "-h") ||
        compare_command(cmd_str, "--help"))
        result = HELP_CMD;
    else if (compare_command(cmd_str, "repo"))
        result = REPO_CMD;
    else if (compare_command(cmd_str, "pulls"))
        result = PULLS_CMD;
    else if (compare_command(cmd_str, "newpr"))
        result = NEWPR_CMD;
    else
        result = INVALID_CMD;

    return result;
}

void add_instruction(struct Context *context) {
    if (context->prompt.cmd == HELP_CMD) {
        strcpy(context->instruction, "man gh");
        return;
    }

    char remote_url[MAX_STR_SIZE];
    int success = generate_git_remote_url(remote_url);
    if (!success) {
        strcpy(context->error, "Repository configuration not found.");
        return;
    }
    strcpy(context->instruction, generate_firefox_instruction(remote_url));

    if (context->prompt.cmd == PULLS_CMD) strcat(context->instruction, "/pulls?q=is:pr");
    else if (context->prompt.cmd == NEWPR_CMD) strcat(context->instruction, "/compare");

    if (context->prompt.opts[0].key[0] != '\0') {
        handle_pulls_options(context);
        handle_newpr_options(context);
    }

    close_firefox_argument(context);
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
    char firefox_bin_cmd[MAX_STR_SIZE] = "firefox --new-tab '";
    return strcat(firefox_bin_cmd, url);
}

static void close_firefox_argument(struct Context *c) {
    size_t sz = strlen(c->instruction);
    if (c->instruction[sz - 1] == '\'') return;

    strcat(c->instruction, "'");
}

static void handle_pulls_options(struct Context *c) {
    if (c->prompt.cmd != PULLS_CMD) return;

    for (int i = 0; i < MAX_CMD_OPTS; i++) {
        if (strcmp(c->prompt.opts[i].key, "--open") == 0) {
            filter_open_prs(c);
        } else if (strcmp(c->prompt.opts[i].key, "--closed") == 0) {
            filter_closed_prs(c);
        } else if (strcmp(c->prompt.opts[i].key, "--author") == 0) {
            filter_prs_by_author(c, c->prompt.opts[i].value);
        } else if (strcmp(c->prompt.opts[i].key, "--to-review") == 0) {
            filter_open_prs(c);
            filter_prs_to_review(c);
        }
    }
}

static void filter_open_prs(struct Context *c) {
    strcat(c->instruction, "+is:open");
}

static void filter_closed_prs(struct Context *c) {
    strcat(c->instruction, "+is:closed");
}

static void filter_prs_by_author(struct Context *c, char *author) {
    char author_param[MAX_STR_SIZE];
    strcpy(author_param, "+author:");
    strcat(author_param, author);
    strcat(c->instruction, author_param);
}

static void filter_prs_to_review(struct Context *c) {
    strcat(c->instruction, "+user-review-requested:@me");
}

static void handle_newpr_options(struct Context *c) {
    if (c->prompt.cmd != NEWPR_CMD) return;

    for (int i = 0; i < MAX_CMD_OPTS; i++) {
        if (strcmp(c->prompt.opts[i].key, "--dest-src") == 0) {
            set_destination_and_source(c, c->prompt.opts[i].value);
        } else if (strcmp(c->prompt.opts[i].key, "--template") == 0) {
            set_template(c, c->prompt.opts[i].value);
        } else if (strcmp(c->prompt.opts[i].key, "--title") == 0) {
            set_title(c, c->prompt.opts[i].value);
        } else if (strcmp(c->prompt.opts[i].key, "--assignees") == 0) {
            set_assignees(c, c->prompt.opts[i].value);
        } else if (strcmp(c->prompt.opts[i].key, "--labels") == 0) {
            set_labels(c, c->prompt.opts[i].value);
        }
    }

    warn_missing_branches(c);
}

static void set_destination_and_source(struct Context *c, char *dest_src) {
    char *query_params = memchr(c->instruction, '?', strlen(c->instruction));
    if (query_params == NULL) {
        strcat(c->instruction, "/");
        strcat(c->instruction, dest_src);
    } else {
        char dest_src_param[MAX_STR_SIZE];
        dest_src_param[0] = '/';
        strcpy(dest_src_param+1, dest_src);
        size_t query_sz = strlen(query_params);
        size_t dest_src_sz = strlen(dest_src_param);
        memmove(query_params+dest_src_sz, query_params, query_sz);
        strncpy(query_params, dest_src_param, dest_src_sz);
    }
}

static void set_template(struct Context *c, char *template) {
    assure_query_param_support(c);

    strcat(c->instruction, "&template=");
    strcat(c->instruction, template);
}

static void set_title(struct Context *c, char *title) {
    assure_query_param_support(c);

    strcat(c->instruction, "&title=");
    strcat(c->instruction, title);
}

static void set_assignees(struct Context *c, char *assignees) {
    assure_query_param_support(c);

    strcat(c->instruction, "&assignees=");
    strcat(c->instruction, assignees);
}

static void set_labels(struct Context *c, char *labels) {
    assure_query_param_support(c);

    strcat(c->instruction, "&labels=");
    strcat(c->instruction, labels);
}

static void assure_query_param_support(struct Context *c) {
    if (strstr(c->instruction, "?expand=1") != NULL) return;

    strcat(c->instruction, "?expand=1");
}

static void warn_missing_branches(struct Context *c) {
    if (strstr(c->instruction, "...") != NULL) return;

    strcpy(c->warn, "WARNING: No branches have been selected. Execute `gh help` for more details.");
}
