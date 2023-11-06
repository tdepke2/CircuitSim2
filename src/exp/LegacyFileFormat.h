#pragma once

#include <FileStorage.h>

#include <string>

class Board;

class LegacyFileFormat : public FileStorage {
public:
    virtual void loadFromFile(Board& board, const std::string& filename) override;
    virtual void saveToFile(Board& board) override;
};
