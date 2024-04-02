#include "_cmocka.h"
#include "../src/gh.h"

FILE *__wrap_find_git_config() {
    return (FILE *)mock();
}

static void test_cmd_table(void **state) {
    assert_int_equal(cmd_table("help"), HELP_CMD);
    assert_int_equal(cmd_table("-h"), HELP_CMD);
    assert_int_equal(cmd_table("--help"), HELP_CMD);
    assert_int_equal(cmd_table("repo"), REPO_CMD);
    assert_int_equal(cmd_table("pulls"), PULLS_CMD);
    assert_int_equal(cmd_table("newpr"), NEWPR_CMD);
    assert_int_equal(cmd_table("invalid"), INVALID_CMD);
}

static void test_repo_instruction_generation_errors(void **state) {
    struct Prompt repo = { REPO_CMD };
    struct Context con;
    con.prompt = repo;
    will_return(__wrap_find_git_config, NULL);

    add_instruction(&con);

    assert_string_equal(con.instruction, "\0");
    assert_string_equal(con.error, "Repository configuration not found.");
}

static void test_repo_instruction_generation_git_remote(void **state) {
    struct Prompt repo = { REPO_CMD };
    struct Context con;
    con.prompt = repo;
    char fake_config_path[MAX_STR_SIZE];
    getcwd((char *)&fake_config_path, MAX_STR_SIZE);
    strcat(fake_config_path, "/tests/fake_git_config");
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));

    add_instruction(&con);

    assert_string_equal(con.instruction, "firefox --new-tab 'github.com/fakeuser/fakerepo'");
}

static void test_repo_instruction_generation_https_remote(void **state) {
    struct Prompt repo = { REPO_CMD };
    struct Context con;
    con.prompt = repo;
    char fake_config_path[MAX_STR_SIZE];
    getcwd((char *)&fake_config_path, MAX_STR_SIZE);
    strcat(fake_config_path, "/tests/fake_https_config");
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));

    add_instruction(&con);

    assert_string_equal(con.instruction, "firefox --new-tab 'github.com/gustavothecoder/gh'");
}

static void test_help_instruction_generation(void **state) {
    struct Prompt help = { HELP_CMD };
    struct Context con;
    con.prompt = help;

    add_instruction(&con);

    assert_string_equal(con.instruction, "man gh");
}

static void test_pulls_instruction_generation(void **state) {
    // Arrange
    char fake_config_path[MAX_STR_SIZE];
    getcwd((char *)&fake_config_path, MAX_STR_SIZE);
    strcat(fake_config_path, "/tests/fake_git_config");

    struct Prompt pulls_without_options = { PULLS_CMD };
    struct Context con_without_options;
    con_without_options.prompt = pulls_without_options;

    struct Prompt pulls_with_open = { PULLS_CMD };
    strcpy(pulls_with_open.opts[0].key, "--open");
    struct Context con_with_open;
    con_with_open.prompt = pulls_with_open;

    struct Prompt pulls_with_author_and_closed = { PULLS_CMD };
    strcpy(pulls_with_author_and_closed.opts[0].key, "--closed");
    strcpy(pulls_with_author_and_closed.opts[1].key, "--author");
    strcpy(pulls_with_author_and_closed.opts[1].value, "@me");
    struct Context con_with_author_and_closed;
    con_with_author_and_closed.prompt = pulls_with_author_and_closed;

    struct Prompt pulls_with_to_review = { PULLS_CMD };
    strcpy(pulls_with_to_review.opts[0].key, "--to-review");
    struct Context con_with_to_review;
    con_with_to_review.prompt = pulls_with_to_review;

    // Act
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&con_without_options);
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&con_with_open);
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&con_with_author_and_closed);
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&con_with_to_review);

    // Assert
    assert_string_equal(con_without_options.instruction, "firefox --new-tab 'github.com/fakeuser/fakerepo/pulls?q=is:pr'");
    assert_string_equal(con_with_open.instruction, "firefox --new-tab 'github.com/fakeuser/fakerepo/pulls?q=is:pr+is:open'");
    assert_string_equal(
                        con_with_author_and_closed.instruction,
                        "firefox --new-tab 'github.com/fakeuser/fakerepo/pulls?q=is:pr+is:closed+author:@me'"
                        );
    assert_string_equal(
                        con_with_to_review.instruction,
                        "firefox --new-tab 'github.com/fakeuser/fakerepo/pulls?q=is:pr+is:open+user-review-requested:@me'"
                        );
}

static void test_newpr_instruction_generation(void **state) {
    // Arrange
    char fake_config_path[MAX_STR_SIZE];
    getcwd((char *)&fake_config_path, MAX_STR_SIZE);
    strcat(fake_config_path, "/tests/fake_git_config");

    struct Prompt newpr_without_options = { NEWPR_CMD };
    struct Context con_without_options;
    con_without_options.prompt = newpr_without_options;

    struct Prompt newpr_with_options = { NEWPR_CMD };
    strcpy(newpr_with_options.opts[0].key, "--dest-src");
    strcpy(newpr_with_options.opts[0].value, "main...task/jc-123");
    strcpy(newpr_with_options.opts[1].key, "--template");
    strcpy(newpr_with_options.opts[1].value, "feature.md");
    strcpy(newpr_with_options.opts[2].key, "--assignees");
    strcpy(newpr_with_options.opts[2].value, "gustavothecoder");
    strcpy(newpr_with_options.opts[3].key, "--labels");
    strcpy(newpr_with_options.opts[3].value, "feature,minor");
    struct Context con_with_options;
    con_with_options.prompt = newpr_with_options;

    struct Prompt newpr_with_only_template_option = { NEWPR_CMD };
    strcpy(newpr_with_only_template_option.opts[0].key, "--template");
    strcpy(newpr_with_only_template_option.opts[0].value, "bug.md");
    struct Context con_with_only_template_option;
    con_with_only_template_option.prompt = newpr_with_only_template_option;

    struct Prompt newpr_with_title = { NEWPR_CMD };
    strcpy(newpr_with_title.opts[0].key, "--title");
    strcpy(newpr_with_title.opts[0].value, "Add a very nice feature");
    strcpy(newpr_with_title.opts[1].key, "--dest-src");
    strcpy(newpr_with_title.opts[1].value, "main...my-nice-feature");
    struct Context con_with_title;
    con_with_title.prompt = newpr_with_title;

    // Act
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&con_without_options);
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&con_with_options);
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&con_with_only_template_option);
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));
    add_instruction(&con_with_title);

    // Assert
    assert_string_equal(con_without_options.instruction, "firefox --new-tab 'github.com/fakeuser/fakerepo/compare'");
    assert_string_equal(
                        con_with_options.instruction,
                        "firefox --new-tab 'github.com/fakeuser/fakerepo/compare/main...task/jc-123" \
                        "?expand=1&template=feature.md&assignees=gustavothecoder&labels=feature,minor'"
                        );
    assert_string_equal(
                        con_with_only_template_option.instruction,
                        "firefox --new-tab 'github.com/fakeuser/fakerepo/compare?expand=1&template=bug.md'"
                        );
    assert_string_equal(
                        con_with_only_template_option.warn,
                        "WARNING: No branches have been selected. Execute `gh help` for more details."
                        );
    assert_string_equal(
                        con_with_title.instruction,
                        "firefox --new-tab 'github.com/fakeuser/fakerepo/compare/main...my-nice-feature" \
                        "?expand=1&title=Add a very nice feature'"
                        );
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_cmd_table),
        cmocka_unit_test(test_repo_instruction_generation_errors),
        cmocka_unit_test(test_repo_instruction_generation_git_remote),
        cmocka_unit_test(test_repo_instruction_generation_https_remote),
        cmocka_unit_test(test_help_instruction_generation),
        cmocka_unit_test(test_pulls_instruction_generation),
        cmocka_unit_test(test_newpr_instruction_generation)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
