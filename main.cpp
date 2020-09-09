#include <iostream>
#include <fstream>
#include <vector>

#include "assert.h"
#include "tokenizer.h"

void run(const std::string& code) {
    Unimplemented();
}

void runFile(const std::string& fileName) {
    std::ifstream ifs(fileName);
    if (!ifs) {
        std::cerr << "Failed to open " << fileName << std::endl;
        return;
    }
    std::string code;
    ifs >> code;
    run(code);
}

void startRepl() {
    Unimplemented();
}

int main(int argc, char *argv[]){
    if (argc > 2) {
        std::cout << "Usage: startear [script]" << std::endl;
        exit(0);
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        startRepl();
    }
    return 0;
}
