#pragma once

#include <filesystem>

class JackAnalyzer {
public:
    JackAnalyzer(std::string inputFilePath);
    void generateVM();
private:
    bool isDir;
    void generateVMForSingleFile(std::filesystem::path inputPath);
    std::filesystem::path path;
};