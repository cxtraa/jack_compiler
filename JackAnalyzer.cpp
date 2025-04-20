
#include "JackAnalyzer.hpp"
#include "CompilationEngine.hpp"
#include <string>
#include <filesystem>
#include <stdexcept>
#include <iostream>

JackAnalyzer::JackAnalyzer(std::string inputFilePath) : path(inputFilePath) {}

void JackAnalyzer::generateXML() {
    if (std::filesystem::is_regular_file(path)) {
        generateXMLForSingleFile(path);
    }
    else if (std::filesystem::is_directory(path)) {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file() && entry.path().extension().string() == ".jack") {
                generateXMLForSingleFile(entry.path());
            }
        }
    }
}

void JackAnalyzer::generateXMLForSingleFile(std::filesystem::path inputPath) {
    std::filesystem::path outputPath = inputPath.parent_path() / (inputPath.stem().string() + "N.xml");
    CompilationEngine compilationEngine(inputPath, outputPath);
    std::cout << "Beginning compilation of " << inputPath.filename().string() << std::endl;
    compilationEngine.compileClass();
    std::cout << "Completed compilation of " << inputPath.filename().string() << std::endl;
}