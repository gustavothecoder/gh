#include "gh.h"

int main(int argc, char *argv[]) {
    struct Prompt prompt = parse_prompt(argc, argv);
    if (prompt.cmd == INVALID_CMD) {
        printf("Option not supported. Execute `gh -h` to see all available options.\n");
        return EXIT_FAILURE;
    }

    add_instruction(&prompt);
    system(prompt.instruction);
    return EXIT_SUCCESS;
}
