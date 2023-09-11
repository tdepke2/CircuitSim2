#include <Chunk.h>
#include <tiles/Blank.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>




#include <iomanip>

uint8_t Chunk::textureLookup[512];

TileData::TileData() :
    id(TileId::blank),
    state1(State::disconnected),
    state2(State::disconnected),
    dir(Direction::north),
    highlight(false),
    meta(0) {
}

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

void Chunk::buildTextureLookup() {
    uint8_t textureId = 0;
    TileId::t id = TileId::blank;

    auto addThreeTextures = [&textureId](TileId::t id, State::t state2) {
        textureLookup[TileData(id, State::low, state2).getTextureHash()] = textureId++;
        textureLookup[TileData(id, State::high, state2).getTextureHash()] = textureId++;
        textureLookup[TileData(id, State::middle, state2).getTextureHash()] = textureId++;
    };

    // Blank tile.
    textureLookup[id] = textureId++;
    id = static_cast<TileId::t>(id + 1);

    // Wire tiles besides crossover have 3 textures.
    for (; id < TileId::wireCrossover; id = static_cast<TileId::t>(id + 1)) {
        addThreeTextures(id, State::low);
    }

    // Crossover wire has 9 textures.
    addThreeTextures(id, State::low);
    addThreeTextures(id, State::high);
    addThreeTextures(id, State::middle);
    id = static_cast<TileId::t>(id + 1);

    // Switch, button, and diode each have 2 textures.
    for (; id < TileId::gateDiode; id = static_cast<TileId::t>(id + 1)) {
        textureLookup[TileData(id, State::low, State::disconnected).getTextureHash()] = textureId++;
        textureLookup[TileData(id, State::high, State::disconnected).getTextureHash()] = textureId++;
    }

    // Gates each have 9 textures, and gates that support 3 inputs have an additional 3 textures.
    for (; id < TileId::count; id = static_cast<TileId::t>(id + 1)) {
        addThreeTextures(id, State::disconnected);
        addThreeTextures(id, State::low);
        addThreeTextures(id, State::high);
        if (id >= TileId::gateAnd) {
            addThreeTextures(id, State::middle);
        }
    }
    std::cout << "building textureLookup finished, final textureId is " << textureId << "\n";
}

Chunk::Chunk(unsigned int textureWidth, unsigned int tileWidth) :
    tiles_{},
    textureWidth_(textureWidth),
    tileWidth_(tileWidth) {

    vertices_.setPrimitiveType(sf::Triangles);
    vertices_.resize(WIDTH * WIDTH * 6);
    for (unsigned int y = 0; y < WIDTH; ++y) {
        for (unsigned int x = 0; x < WIDTH; ++x) {
            sf::Vertex* tileVertices = &vertices_[(y * WIDTH + x) * 6];

            float px = static_cast<float>(x * tileWidth_);
            float py = static_cast<float>(y * tileWidth_);
            tileVertices[0].position = {px, py};
            tileVertices[1].position = {px + tileWidth_, py};
            tileVertices[2].position = {px + tileWidth_, py + tileWidth_};
            tileVertices[3].position = {px + tileWidth_, py + tileWidth_};
            tileVertices[4].position = {px, py + tileWidth_};
            tileVertices[5].position = {px, py};

            redrawTile(x, y);
        }
    }
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
    }
}

void Chunk::debugPrintChunk() {
    for (unsigned int y = 0; y < WIDTH; ++y) {
        for (unsigned int x = 0; x < WIDTH; ++x) {
            std::cout << std::setw(8) << std::hex << *reinterpret_cast<uint32_t*>(&tiles_[y * WIDTH + x]) << " ";
        }
        std::cout << "\n";
    }
}

void Chunk::redrawTile(unsigned int x, unsigned int y) {
    sf::Vertex* tileVertices = &vertices_[(y * WIDTH + x) * 6];
    TileData tileData = tiles_[y * WIDTH + x];

    unsigned int textureId = tileData.id;

    float tx = static_cast<float>(static_cast<unsigned int>(textureId % (textureWidth_ / tileWidth_)) * tileWidth_);
    float ty = static_cast<float>(static_cast<unsigned int>(textureId / (textureWidth_ / tileWidth_)) * tileWidth_);
    tileVertices[0].texCoords = {tx, ty};
    tileVertices[1].texCoords = {tx + tileWidth_, ty};
    tileVertices[2].texCoords = {tx + tileWidth_, ty + tileWidth_};
    tileVertices[3].texCoords = {tx + tileWidth_, ty + tileWidth_};
    tileVertices[4].texCoords = {tx, ty + tileWidth_};
    tileVertices[5].texCoords = {tx, ty};
}

void Chunk::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(vertices_, states);
}
