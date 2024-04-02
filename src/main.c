#include "gh.h"

int main(int argc, char *argv[]) {
    struct Prompt prompt = parse_prompt(argc, argv, cmd_table, DEFAULT_CMD);
    if (prompt.cmd == INVALID_CMD) {
        printf("Command not supported. Execute `gh -h` to see all available commands.\n");
        return EXIT_FAILURE;
    }

    struct Context context;
    context.prompt = prompt;
    add_instruction(&context);
    if (context.error[0] != '\0') {
        printf("%s\n", context.error);
        return EXIT_FAILURE;
    }

    if (context.warn[0] != '\0') printf("%s\n", context.warn);

    system(context.instruction);
    return EXIT_SUCCESS;
}
