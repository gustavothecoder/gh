#include "_cmocka.h"
#include "../src/gh.h"

static void test_parsing_prompt_with_no_flags(void **state) {
    char *fake_argv[1];
    fake_argv[0] = "gh";

    struct Prompt result = parse_prompt(1, fake_argv);

    assert_int_equal(result.cmd, HOME_CMD);
}

static void test_parsing_prompt_with_help_flags(void **state) {
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

FILE *__wrap_find_git_config(const char *path) {
    return (FILE *)mock();
}

static void test_home_instruction_generation(void **state) {
    struct Prompt home = { HOME_CMD };
    char fake_config_path[MAX_STR_SIZE];
    getcwd((char *)&fake_config_path, MAX_STR_SIZE);
    strcat(fake_config_path, "/tests/fake_git_config");
    will_return(__wrap_find_git_config, fopen(fake_config_path, "r"));

    add_instruction(&home);

    assert_string_equal(home.instruction, "firefox --new-tab github.com/fakeuser/fakerepo");
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_parsing_prompt_with_no_flags),
        cmocka_unit_test(test_parsing_prompt_with_help_flags),
        cmocka_unit_test(test_home_instruction_generation)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
