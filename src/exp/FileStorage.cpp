#include <FileStorage.h>

#include <stdexcept>

float FileStorage::getFileVersion(const std::string& filename, std::ifstream& inputFile) {
    if (!inputFile.is_open()) {
        throw std::runtime_error("\"" + filename + "\": unable to open file for reading.");
    }
    inputFile.clear();
    inputFile.seekg(0);

    std::string line;
    while (std::getline(inputFile, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.length() == 0) {
            continue;
        } else if (line.find("version:") == 0) {
            return std::stof(line.substr(8));
        } else {
            break;
        }
    }
    return -1.0;
}
