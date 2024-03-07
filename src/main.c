#include "gh.h"

int main(int argc, char *argv[]) {
    struct Prompt prompt = parse_prompt(argc, argv);
    if (prompt.cmd == INVALID_CMD) {
        printf("Option not supported. Execute `gh -h` to see all available options.\n");
        return EXIT_FAILURE;
    }

    add_instruction(&prompt);
    if (prompt.error[0] != '\0') {
        printf("%s\n", prompt.error);
        return EXIT_FAILURE;
    }

    if (prompt.warn[0] != '\0') printf("%s\n", prompt.warn);

    system(prompt.instruction);
    return EXIT_SUCCESS;
}
