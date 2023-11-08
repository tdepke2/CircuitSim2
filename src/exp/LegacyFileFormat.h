#pragma once

#include <FileStorage.h>

#include <map>
#include <string>

class Board;
struct TileSymbol;

class LegacyFileFormat : public FileStorage {
public:
    LegacyFileFormat();

    virtual void loadFromFile(Board& board, const std::string& filename) override;
    virtual void saveToFile(Board& board) override;

private:
    struct ParseState {
        std::string lastField = "";
        std::string filename = "";
        float fileVersion = 1.0;
        unsigned int width = 0;
        unsigned int height = 0;
        bool enableExtraLogicStates = false;
        std::string notesString = "";
        int x = 0;
        int y = 0;
    };

    void parseFile(Board& board, const std::string& line, int lineNumber, ParseState& parseState, const std::map<TileSymbol, unsigned int>& symbolLookup);

    std::string path_, name_;
};
