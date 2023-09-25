#include <Chunk.h>
#include <tiles/Blank.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>




#include <iomanip>



TileData::TileData(TileId::t id, State::t state1, State::t state2, Direction::t dir, bool highlight, uint16_t meta) :
    id(id),
    state1(state1),
    state2(state2),
    dir(dir),
    highlight(highlight),
    meta(meta) {
}

uint16_t TileData::getTextureHash() const {
    return (static_cast<uint16_t>(state2) << 7) | (static_cast<uint16_t>(state1) << 5) | id;
}

Chunk::Chunk() :
    tiles_{} {
}

Tile Chunk::accessTile(unsigned int x, unsigned int y) {
    TileData tileData = tiles_[y * WIDTH + x];
    if (tileData.id == TileId::blank) {
        return {tiles::Blank::instance(), *this, y * WIDTH + x};
    } else if (tileData.id <= TileId::wireCrossover) {
        return {tiles::Wire::instance(), *this, y * WIDTH + x};
    } else if (tileData.id <= TileId::inButton) {
        return {tiles::Input::instance(), *this, y * WIDTH + x};
    } else if (tileData.id == TileId::outLed) {
        return {tiles::Led::instance(), *this, y * WIDTH + x};
    } else if (tileData.id <= TileId::gateXnor) {
        return {tiles::Gate::instance(), *this, y * WIDTH + x};
    } else {
        std::cout << "unknown tile id " << tileData.id << "\n";
        assert(false);
        return {tiles::Blank::instance(), *this, y * WIDTH + x};
    }
}

void Chunk::debugPrintChunk() {
    for (unsigned int y = 0; y < WIDTH; ++y) {
        for (unsigned int x = 0; x < WIDTH; ++x) {
            std::cout << std::setw(8) << std::hex << *reinterpret_cast<uint32_t*>(&tiles_[y * WIDTH + x]) << std::dec << " ";
        }
        std::cout << "\n";
    }
}
