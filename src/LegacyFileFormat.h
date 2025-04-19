#pragma once

#include <FileStorage.h>
#include <Filesystem.h>

#include <fstream>
#include <string>

class Board;

/**
 * Legacy format used for circuit files.
 * 
 * This file format puts everything into a single text file, with a big text
 * block containing all the circuit tiles at the end. In the text block, every
 * two characters represents a tile. This is convenient since the file is
 * visually similar to the circuit in a text editor and easy for an external
 * tool to read/edit tiles in the circuit. Unfortunately this format does not
 * support chunk loading, is inefficient for larger circuits, and may not
 * support some special tiles that can't be packed into just two characters.
 */
class LegacyFileFormat : public FileStorage {
public:
    struct HeaderState {
        std::string lastField = "headerBegin";
        fs::path filename = "";
        float fileVersion = 1.0;
        unsigned int width = 0;
        unsigned int height = 0;
        bool extraLogicStates = false;
        std::string notesString = "";
    };

    static void parseHeader(Board& board, const std::string& line, int lineNumber, HeaderState& state);
    static void writeHeader(Board& board, const fs::path& filename, std::ostream& boardFile, float version);

    LegacyFileFormat(const fs::path& filename);

    virtual fs::path getDefaultFileExtension() const override;
    virtual bool validateFileVersion(float version) override;
    virtual void loadFromFile(Board& board, const fs::path& filename, fs::ifstream& boardFile) override;
    virtual void saveToFile(Board& board) override;

private:
    struct ParseState : public HeaderState {
        int x = 0;
        int y = 0;
    };

    static void parseTiles(Board& board, const std::string& line, int lineNumber, ParseState& state);
};
