
#include "JackAnalyzer.hpp"
#include "CompilationEngine.hpp"
#include <string>
#include <filesystem>
#include <stdexcept>
#include <iostream>

JackAnalyzer::JackAnalyzer(std::string inputFilePath) : path(inputFilePath) {}

void JackAnalyzer::generateVM() {
    std::cout << "Began compiling files in " << path.string() << std::endl;
    if (std::filesystem::is_regular_file(path)) {
        generateVMForSingleFile(path);
    }
    else if (std::filesystem::is_directory(path)) {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file() && entry.path().extension().string() == ".jack") {
                generateVMForSingleFile(entry.path());
            }
        }
    }
    std::cout << "Finished compiling files in " << path.string() << std::endl;
}

void JackAnalyzer::generateVMForSingleFile(std::filesystem::path inputPath) {
    std::filesystem::path outputPath = inputPath.parent_path() / (inputPath.stem().string() + ".vm");
    CompilationEngine compilationEngine(inputPath, outputPath);
    std::cout << "Began compiling " << inputPath.filename().string() << std::endl;
    compilationEngine.compileClass();
    std::cout << "Finished compiling " << inputPath.filename().string() << std::endl;
}