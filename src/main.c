#include "gh.h"

int main(int argc, char *argv[]) {
    struct Prompt prompt = parse_prompt(argc, argv);
    add_instruction(&prompt);
    system(prompt.instruction);
    return 0;
}
