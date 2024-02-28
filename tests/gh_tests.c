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
    char *fake_argv1[argc];
    fake_argv1[0] = "gh";
    fake_argv1[1] = "-h";
    char *fake_argv2[argc];
    fake_argv2[0] = "gh";
    fake_argv2[1] = "--help";

    struct Prompt result1 = parse_prompt(argc, fake_argv1);
    struct Prompt result2 = parse_prompt(argc, fake_argv2);

    assert_int_equal(result1.cmd, HELP_CMD);
    assert_int_equal(result2.cmd, HELP_CMD);
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
    int argc = 2;
    char *fake_argv[argc];
    fake_argv[0] = "gh";
    fake_argv[1] = "pulls";

    struct Prompt result = parse_prompt(argc, fake_argv);

    assert_int_equal(result.cmd, PR_CMD);
}

static void test_parsing_prompt_with_pulls_options(void **state) {
    int argc = 2;
    char *fake_argv[argc];
    fake_argv[0] = "gh";
    fake_argv[1] = "pulls";

    struct Prompt result = parse_prompt(argc, fake_argv);

    assert_int_equal(result.cmd, PR_CMD);
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

    assert_string_equal(repo.instruction, "firefox --new-tab github.com/fakeuser/fakerepo");
}

static void test_repo_instruction_generation_https_remote(void **state) {
    struct Prompt repo = { REPO_CMD };
    char fake_config_path[MAX_STR_SIZE];
    getcwd((char *)&fake_config_path, MAX_STR_SIZE);
    strcat(fake_config_path, "/tests/fake_https_config");
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));

    add_instruction(&repo);

    assert_string_equal(repo.instruction, "firefox --new-tab github.com/gustavothecoder/gh");
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

static void test_pr_instruction_generation(void **state) {
    struct Prompt pr = { PR_CMD };
    char fake_config_path[MAX_STR_SIZE];
    getcwd((char *)&fake_config_path, MAX_STR_SIZE);
    strcat(fake_config_path, "/tests/fake_git_config");
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));

    add_instruction(&pr);

    assert_string_equal(pr.instruction, "firefox --new-tab github.com/fakeuser/fakerepo/pulls");
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_parsing_invalid_prompt),
        cmocka_unit_test(test_parsing_prompt_with_no_flags),
        cmocka_unit_test(test_parsing_prompt_with_help_options),
        cmocka_unit_test(test_parsing_prompt_with_repo_cmd),
        cmocka_unit_test(test_parsing_prompt_with_pulls_cmd),
        cmocka_unit_test(test_parsing_prompt_with_pulls_options),
        cmocka_unit_test(test_repo_instruction_generation_git_remote),
        cmocka_unit_test(test_repo_instruction_generation_https_remote),
        cmocka_unit_test(test_repo_instruction_generation_errors),
        cmocka_unit_test(test_help_instruction_generation),
        cmocka_unit_test(test_pr_instruction_generation)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
