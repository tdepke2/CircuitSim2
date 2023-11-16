#pragma once

#include <FileStorage.h>

#include <fstream>
#include <string>

class Board;

class LegacyFileFormat : public FileStorage {
public:
    struct HeaderState {
        std::string lastField = "headerBegin";
        std::string filename = "";
        float fileVersion = 1.0;
        unsigned int width = 0;
        unsigned int height = 0;
        bool extraLogicStates = false;
        std::string notesString = "";
    };

    static void parseHeader(Board& board, const std::string& line, int lineNumber, HeaderState& state);

    LegacyFileFormat();

    virtual bool validateFileVersion(float version) override;
    virtual void loadFromFile(Board& board, const std::string& filename, std::ifstream& inputFile) override;
    virtual void saveToFile(Board& board) override;

private:
    struct ParseState : public HeaderState {
        int x = 0;
        int y = 0;
    };

    static void parseTiles(Board& board, const std::string& line, int lineNumber, ParseState& state);

    std::string path_, name_;
};
