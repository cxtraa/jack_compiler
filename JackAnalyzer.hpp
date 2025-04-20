#pragma once

#include <filesystem>

class JackAnalyzer {
public:
    JackAnalyzer(std::string inputFilePath);
    void generateXML();
private:
    bool isDir;
    void generateXMLForSingleFile(std::filesystem::path inputPath);
    std::filesystem::path path;
};