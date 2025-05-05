#include <ChunkCoordsRange.h>
#include <FileStorage.h>

#include <stdexcept>
#include <string>

float FileStorage::getFileVersion(const fs::path& filename, fs::ifstream& boardFile) {
    if (!boardFile.is_open()) {
        throw std::runtime_error("\"" + filename.string() + "\": unable to open file for reading.");
    }
    boardFile.clear();
    boardFile.seekg(0, std::ios::beg);

    std::string line;
    while (std::getline(boardFile, line)) {
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

FileStorage::FileStorage(const fs::path& filename) :
    filename_(filename),
    newFile_(true) {
}

const fs::path& FileStorage::getFilename() const {
    return filename_;
}

bool FileStorage::isNewFile() const {
    return newFile_;
}

void FileStorage::updateVisibleChunks(Board& /*board*/, const ChunkCoordsRange& /*visibleChunks*/) {

}

bool FileStorage::loadChunk(Board& /*board*/, ChunkCoords::repr /*chunkCoords*/) {
    return false;
}

void FileStorage::setFilename(const fs::path& filename) {
    filename_ = filename;
}

void FileStorage::setNewFile(bool newFile) {
    newFile_ = newFile;
}
