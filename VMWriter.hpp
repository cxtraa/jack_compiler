#pragma once

#include <filesystem>
#include <fstream>
#include "Enums.hpp"

class VMWriter {
public:
    VMWriter(std::filesystem::path outputPath);

    void writePush(std::string segment, int index);
    void writePop(std::string segment, int index);
    void writeArithmetic(std::string command);
    void writeLabel(std::string label);
    void writeGoTo(std::string label);
    void writeIf(std::string label);
    void writeCall(std::string name, int nArgs);
    void writeFunction(std::string name, int nVars);
    void writeReturn();
    void close();
private:
    std::ofstream outputStream;
};