#include <Board.h>
#include <LegacyFileFormat.h>
#include <RegionFileFormat.h>

#include <limits>
#include <spdlog/spdlog.h>
#include <stdexcept>

constexpr int RegionFileFormat::REGION_WIDTH;

RegionFileFormat::RegionFileFormat() :
    filename_("boards/NewBoard/board.txt"),
    savedRegions_() {
}

bool RegionFileFormat::validateFileVersion(float version) {
    return version == 2.0;
}

void RegionFileFormat::loadFromFile(Board& board, const fs::path& filename, fs::ifstream& inputFile) {
    if (!inputFile.is_open()) {
        throw std::runtime_error("\"" + filename.string() + "\": unable to open file for reading.");
    }
    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);

    std::string line;
    int lineNumber = 0;
    LegacyFileFormat::HeaderState state;
    state.filename = filename;
    try {
        while (std::getline(inputFile, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            LegacyFileFormat::parseHeader(board, line, ++lineNumber, state);
        }
        if (state.lastField != "headerEnd") {
            throw std::runtime_error("missing data, end of file reached.");
        }
    } catch (std::exception& ex) {
        throw std::runtime_error("\"" + filename.string() + "\" at line " + std::to_string(lineNumber) + ": " + ex.what());
    }

    // FIXME add a new field called "regions" tracking all of the region files.

}

constexpr int constLog2(int x) {
    return x == 1 ? 0 : 1 + constLog2(x / 2);
}

void RegionFileFormat::saveToFile(Board& board) {
    if (filename_.has_parent_path()) {
        fs::create_directories(filename_.parent_path());
    }
    fs::ofstream outputFile(filename_);
    LegacyFileFormat::writeHeader(board, filename_, outputFile);
    outputFile.close();
    fs::create_directory(filename_.parent_path() / "region");

    std::map<RegionCoords, Region> dirtyRegions;

    constexpr int widthLog2 = constLog2(REGION_WIDTH);
    for (const auto& chunk : board.getLoadedChunks()) {
        if (/*FIXME chunk is dirty*/ true) {
            auto& region = dirtyRegions[{ChunkCoords::x(chunk.first) >> widthLog2, ChunkCoords::y(chunk.first) >> widthLog2}];
            region.insert(chunk.first);
        }
    }

    spdlog::debug("save file: {}", filename_);
    spdlog::debug("dirtyRegions:");
    for (auto& region : dirtyRegions) {
        spdlog::debug("  ({}, {}) ->", region.first.first, region.first.second);
        for (auto& coords : region.second) {
            spdlog::debug("    {}, {}", ChunkCoords::x(coords), ChunkCoords::y(coords));
        }
    }

    //for (const auto& region : dirtyRegions) {
    //    saveRegion(board, region.first, region.second);
    //}
    const auto region = dirtyRegions.find({0, 0});
    saveRegion(board, region->first, region->second);
}

void RegionFileFormat::saveRegion(Board& board, const RegionCoords& coords, const Region& region) {
    fs::path regionFilename = std::to_string(coords.first) + "." + std::to_string(coords.second) + ".dat";
    regionFilename = filename_.parent_path() / "region" / regionFilename;
    fs::ofstream regionFile(regionFilename, std::ios::binary);
    if (!regionFile.is_open()) {
        throw std::runtime_error("\"" + regionFilename.string() + "\": unable to open file for reading/writing.");
    }

    /*
    // FIXME this may not work unless we open in read mode first...

    // Determine number of bytes we can read from the file, should match the file size in most cases.
    // https://stackoverflow.com/questions/22984956/tellg-function-give-wrong-size-of-file/22986486#22986486
    regionFile.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize readLength = regionFile.gcount();
    regionFile.clear();
    regionFile.seekg(0, std::ios::beg);

    spdlog::debug("readLength = {}", readLength);
    */

    constexpr int SECTOR_SIZE = 256;
    const char emptySector[SECTOR_SIZE] = {};
    struct ChunkHeader {
        uint32_t offset : 24;
        uint8_t sectors : 8;
    };

    ChunkHeader header[1024] = {};
    header[0].offset = 0x010203;
    header[0].sectors = 99;
    for (int i = 0; i < 1024; ++i) {
        uint32_t offset = byteswap(header[i].offset << 8);
        regionFile.write(reinterpret_cast<char*>(&offset), 3);
        uint8_t sectors = header[i].sectors;
        regionFile.write(reinterpret_cast<char*>(&sectors), 1);
    }

    const auto& chunk = board.getLoadedChunks().at(ChunkCoords::pack(0, 0));
    chunk.debugPrintChunk();
    auto data = chunk.serialize();

    const auto dataSize = data.size() + sizeof(data.size());
    auto dataSizeSwapped = byteswap(dataSize);
    regionFile.write(reinterpret_cast<char*>(&dataSizeSwapped), sizeof(dataSizeSwapped));
    regionFile.write(data.data(), data.size());
    const auto paddingBytes = SECTOR_SIZE - (dataSize - 1) % SECTOR_SIZE - 1;
    spdlog::debug("writing {} extra padding", paddingBytes);
    regionFile.write(emptySector, paddingBytes);

    regionFile.close();
}

/*

serialize chunks into memory

read header...
header exists and no existing chunks need more memory?
    write new/update existing chunks and replace header
else
    header exists?
        read all chunks into memory
    write header and chunks

clean up dirty chunks in board that are empty
*/
