#include <iostream>
#include "JackAnalyzer.hpp"
#include <stdexcept>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        throw std::runtime_error("Compiler: you must specify a single directory or file.");
    }
    std::string path = argv[1];
    JackAnalyzer analyzer(path);
    analyzer.generateVM();
    return 0;
}