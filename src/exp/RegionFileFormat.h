#pragma once

#include <FileStorage.h>

#include <fstream>
#include <string>

class Board;

class RegionFileFormat : public FileStorage {
public:
    RegionFileFormat();

    virtual bool validateFileVersion(float version) override;
    virtual void loadFromFile(Board& board, const std::string& filename, std::ifstream& inputFile) override;
    virtual void saveToFile(Board& board) override;

private:
    std::string path_, name_;
};
