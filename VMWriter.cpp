
#include "VMWriter.hpp"
#include "Enums.hpp"
#include <filesystem>
#include <fstream>

VMWriter::VMWriter(std::filesystem::path outputPath) {
    outputStream = std::ofstream(outputPath);
    if (!outputStream) {
        throw std::runtime_error("Unable to open the specified output path for the VMWriter.");
    }
}

void VMWriter::writePush(std::string segment, int index) {
    outputStream << "push " << segment << " " << index << std::endl;
}

void VMWriter::writePop(std::string segment, int index) {
    outputStream << "pop " << segment << " " << index << std::endl;
}

void VMWriter::writeArithmetic(std::string command) {
    outputStream << command << std::endl;
}

void VMWriter::writeLabel(std::string label) {
    outputStream << "label " << label << std::endl;
}

void VMWriter::writeGoTo(std::string label) {
    outputStream << "goto " << label << std::endl;
}

void VMWriter::writeIf(std::string label) {
    outputStream << "if-goto " << label << std::endl;
}

void VMWriter::writeCall(std::string name, int nArgs) {
    outputStream << "call " << name << " " << nArgs << std::endl;
}

void VMWriter::writeFunction(std::string name, int nVars) {
    outputStream << "function " << name << " " << nVars << std::endl;
}

void VMWriter::writeReturn() {
    outputStream << "return" << std::endl;
}

void VMWriter::close() {
    outputStream.close();
}