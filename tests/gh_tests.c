#include "_cmocka.h"
#include "../src/gh.h"

static void test_parsing_invalid_prompt(void **state) {
    int argc = 2;
    char *fake_argv[argc];
    fake_argv[0] = "gh";
    fake_argv[1] = "--invalid";

    struct Prompt result = parse_prompt(argc, fake_argv);

    assert_int_equal(result.cmd, INVALID_CMD);
}

static void test_parsing_prompt_with_no_flags(void **state) {
    char *fake_argv[1];
    fake_argv[0] = "gh";

    struct Prompt result = parse_prompt(1, fake_argv);

    assert_int_equal(result.cmd, REPO_CMD);
}

static void test_parsing_prompt_with_help_options(void **state) {
    int argc = 2;
    char *fake_argv[argc];
    fake_argv[0] = "gh";
    fake_argv[1] = "help";

    struct Prompt result = parse_prompt(argc, fake_argv);

    assert_int_equal(result.cmd, HELP_CMD);
}

static void test_parsing_prompt_with_repo_cmd(void **state) {
    int argc = 2;
    char *fake_argv[argc];
    fake_argv[0] = "gh";
    fake_argv[1] = "repo";

    struct Prompt result = parse_prompt(argc, fake_argv);

    assert_int_equal(result.cmd, REPO_CMD);
}

static void test_parsing_prompt_with_pulls_cmd(void **state) {
    int argc = 4;
    char *fake_argv[argc];
    fake_argv[0] = "gh";
    fake_argv[1] = "pulls";
    fake_argv[2] = "--author=me\n";
    fake_argv[3] = "--closed";

    struct Prompt result = parse_prompt(argc, fake_argv);

    assert_int_equal(result.cmd, PULLS_CMD);
    assert_string_equal(result.opts[0].key, "--author");
    assert_string_equal(result.opts[0].value, "me");
    assert_string_equal(result.opts[1].key, "--closed");
}

static void test_parsing_prompt_with_newpr_cmd(void **state) {
    int argc = 2;
    char *fake_argv[argc];
    fake_argv[0] = "gh";
    fake_argv[1] = "newpr";

    struct Prompt result = parse_prompt(argc, fake_argv);

    assert_int_equal(result.cmd, NEWPR_CMD);
}

static void test_parsing_prompt_with_spaces(void **state) {
    int argc = 6;
    char *fake_argv[argc];
    fake_argv[0] = "gh";
    fake_argv[1] = "newpr";
    fake_argv[2] = "--title=this";
    fake_argv[3] = "title";
    fake_argv[4] = "has";
    fake_argv[5] = "spaces";

    struct Prompt result = parse_prompt(argc, fake_argv);

    assert_int_equal(result.cmd, NEWPR_CMD);
    assert_string_equal(result.opts[0].key, "--title");
    assert_string_equal(result.opts[0].value, "this+title+has+spaces");
}

FILE *__wrap_find_git_config() {
    return (FILE *)mock();
}

static void test_repo_instruction_generation_git_remote(void **state) {
    struct Prompt repo = { REPO_CMD };
    char fake_config_path[MAX_STR_SIZE];
    getcwd((char *)&fake_config_path, MAX_STR_SIZE);
    strcat(fake_config_path, "/tests/fake_git_config");
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));

    add_instruction(&repo);

    assert_string_equal(repo.instruction, "firefox --new-tab 'github.com/fakeuser/fakerepo'");
}

static void test_repo_instruction_generation_https_remote(void **state) {
    struct Prompt repo = { REPO_CMD };
    char fake_config_path[MAX_STR_SIZE];
    getcwd((char *)&fake_config_path, MAX_STR_SIZE);
    strcat(fake_config_path, "/tests/fake_https_config");
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));

    add_instruction(&repo);

    assert_string_equal(repo.instruction, "firefox --new-tab 'github.com/gustavothecoder/gh'");
}

static void test_repo_instruction_generation_errors(void **state) {
    struct Prompt repo = { REPO_CMD };
    will_return(__wrap_find_git_config, NULL);

    add_instruction(&repo);

    assert_string_equal(repo.instruction, "\0");
    assert_string_equal(repo.error, "Repository configuration not found.");
}

static void test_help_instruction_generation(void **state) {
    struct Prompt help = { HELP_CMD };

    add_instruction(&help);

    assert_string_equal(help.instruction, "man gh");
}

static void test_pulls_instruction_generation(void **state) {
    // Arrange
    char fake_config_path[MAX_STR_SIZE];
    getcwd((char *)&fake_config_path, MAX_STR_SIZE);
    strcat(fake_config_path, "/tests/fake_git_config");

    struct Prompt pulls_without_options = { PULLS_CMD };

    struct Prompt pulls_with_open = { PULLS_CMD };
    strcpy(pulls_with_open.opts[0].key, "--open");

    struct Prompt pulls_with_author_and_closed = { PULLS_CMD };
    strcpy(pulls_with_author_and_closed.opts[0].key, "--closed");
    strcpy(pulls_with_author_and_closed.opts[1].key, "--author");
    strcpy(pulls_with_author_and_closed.opts[1].value, "@me");

    struct Prompt pulls_with_to_review = { PULLS_CMD };
    strcpy(pulls_with_to_review.opts[0].key, "--to-review");

    // Act
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&pulls_without_options);
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&pulls_with_open);
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&pulls_with_author_and_closed);
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&pulls_with_to_review);

    // Assert
    assert_string_equal(pulls_without_options.instruction, "firefox --new-tab 'github.com/fakeuser/fakerepo/pulls?q=is:pr'");
    assert_string_equal(pulls_with_open.instruction, "firefox --new-tab 'github.com/fakeuser/fakerepo/pulls?q=is:pr+is:open'");
    assert_string_equal(
                        pulls_with_author_and_closed.instruction,
                        "firefox --new-tab 'github.com/fakeuser/fakerepo/pulls?q=is:pr+is:closed+author:@me'"
                        );
    assert_string_equal(
                        pulls_with_to_review.instruction,
                        "firefox --new-tab 'github.com/fakeuser/fakerepo/pulls?q=is:pr+is:open+user-review-requested:@me'"
                        );
}

static void test_newpr_instruction_generation(void **state) {
    // Arrange
    char fake_config_path[MAX_STR_SIZE];
    getcwd((char *)&fake_config_path, MAX_STR_SIZE);
    strcat(fake_config_path, "/tests/fake_git_config");

    struct Prompt newpr_without_options = { NEWPR_CMD };

    struct Prompt newpr_with_options = { NEWPR_CMD };
    strcpy(newpr_with_options.opts[0].key, "--dest-src");
    strcpy(newpr_with_options.opts[0].value, "main...task/jc-123");
    strcpy(newpr_with_options.opts[1].key, "--template");
    strcpy(newpr_with_options.opts[1].value, "feature.md");
    strcpy(newpr_with_options.opts[2].key, "--assignees");
    strcpy(newpr_with_options.opts[2].value, "gustavothecoder");
    strcpy(newpr_with_options.opts[3].key, "--labels");
    strcpy(newpr_with_options.opts[3].value, "feature,minor");

    struct Prompt newpr_with_only_template_option = { NEWPR_CMD };
    strcpy(newpr_with_only_template_option.opts[0].key, "--template");
    strcpy(newpr_with_only_template_option.opts[0].value, "bug.md");

    struct Prompt newpr_with_title = { NEWPR_CMD };
    strcpy(newpr_with_title.opts[0].key, "--title");
    strcpy(newpr_with_title.opts[0].value, "Add a very nice feature");
    strcpy(newpr_with_title.opts[1].key, "--dest-src");
    strcpy(newpr_with_title.opts[1].value, "main...my-nice-feature");

    // Act
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&newpr_without_options);
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&newpr_with_options);
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&newpr_with_only_template_option);
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&newpr_with_title);

    // Assert
    assert_string_equal(newpr_without_options.instruction, "firefox --new-tab 'github.com/fakeuser/fakerepo/compare'");
    assert_string_equal(
                        newpr_with_options.instruction,
                        "firefox --new-tab 'github.com/fakeuser/fakerepo/compare/main...task/jc-123" \
                        "?expand=1&template=feature.md&assignees=gustavothecoder&labels=feature,minor'"
                        );
    assert_string_equal(
                        newpr_with_only_template_option.instruction,
                        "firefox --new-tab 'github.com/fakeuser/fakerepo/compare?expand=1&template=bug.md'"
                        );
    assert_string_equal(
                        newpr_with_only_template_option.warn,
                        "WARNING: No branches have been selected. Execute `gh help` for more details."
                        );
    assert_string_equal(
                        newpr_with_title.instruction,
                        "firefox --new-tab 'github.com/fakeuser/fakerepo/compare/main...my-nice-feature" \
                        "?expand=1&title=Add a very nice feature'"
                        );
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_parsing_invalid_prompt),
        cmocka_unit_test(test_parsing_prompt_with_no_flags),
        cmocka_unit_test(test_parsing_prompt_with_help_options),
        cmocka_unit_test(test_parsing_prompt_with_repo_cmd),
        cmocka_unit_test(test_parsing_prompt_with_pulls_cmd),
        cmocka_unit_test(test_parsing_prompt_with_newpr_cmd),
        cmocka_unit_test(test_parsing_prompt_with_spaces),
        cmocka_unit_test(test_repo_instruction_generation_git_remote),
        cmocka_unit_test(test_repo_instruction_generation_https_remote),
        cmocka_unit_test(test_repo_instruction_generation_errors),
        cmocka_unit_test(test_help_instruction_generation),
        cmocka_unit_test(test_pulls_instruction_generation),
        cmocka_unit_test(test_newpr_instruction_generation)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
