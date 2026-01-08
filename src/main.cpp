#include "pseudocode.hpp"

int main(int argc, char* argv[]) {
    if (argc == 1) {
        return Pseudocode::runRepl();
    }

    return Pseudocode::runFile(argv[1]);
}
