#include <Chunk.h>
#include <tiles/Blank.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>




#include <iomanip>

unsigned int Chunk::textureWidth_, Chunk::tileWidth_;
uint8_t Chunk::textureLookup_[512];
unsigned int Chunk::textureHighlightStart_;

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

void Chunk::setupTextureData(const sf::Vector2u& textureSize, unsigned int tileWidth) {
    textureWidth_ = textureSize.x;
    tileWidth_ = tileWidth;
    textureHighlightStart_ = (textureSize.x / tileWidth / 2) * (textureSize.y / tileWidth / 4);

    uint8_t textureId = 0;
    TileId::t id = TileId::blank;

    auto addThreeTextures = [&textureId](TileId::t id, State::t state2) {
        textureLookup_[TileData(id, State::low, state2).getTextureHash()] = textureId++;
        textureLookup_[TileData(id, State::high, state2).getTextureHash()] = textureId++;
        textureLookup_[TileData(id, State::middle, state2).getTextureHash()] = textureId++;
    };

    // Blank tile.
    textureLookup_[id] = textureId++;
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
        textureLookup_[TileData(id, State::low, State::disconnected).getTextureHash()] = textureId++;
        textureLookup_[TileData(id, State::high, State::disconnected).getTextureHash()] = textureId++;
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
    std::cout << "building textureLookup_ finished, final textureId is " << static_cast<int>(textureId) << "\n";
}

Chunk::Chunk() :
    tiles_{} {

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

void Chunk::forceRedraw() {
    for (unsigned int y = 0; y < WIDTH; ++y) {
        for (unsigned int x = 0; x < WIDTH; ++x) {
            redrawTile(x, y);
        }
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

void Chunk::redrawTile(unsigned int x, unsigned int y) {
    sf::Vertex* tileVertices = &vertices_[(y * WIDTH + x) * 6];
    TileData tileData = tiles_[y * WIDTH + x];

    unsigned int textureId = textureLookup_[tileData.getTextureHash()] + (tileData.highlight ? textureHighlightStart_ : 0);

    float tx = static_cast<float>(static_cast<unsigned int>(textureId % (textureWidth_ / tileWidth_ / 2)) * tileWidth_ * 2 + static_cast<unsigned int>(tileWidth_ / 2));
    float ty = static_cast<float>(static_cast<unsigned int>(textureId / (textureWidth_ / tileWidth_ / 2)) * tileWidth_ * 2 + static_cast<unsigned int>(tileWidth_ / 2));

    sf::Vector2f texCoordsQuad[4] = {
        {tx, ty},
        {tx + tileWidth_, ty},
        {tx + tileWidth_, ty + tileWidth_},
        {tx, ty + tileWidth_}
    };
    tileVertices[0].texCoords = texCoordsQuad[(4 - tileData.dir) % 4];
    tileVertices[1].texCoords = texCoordsQuad[(5 - tileData.dir) % 4];
    tileVertices[2].texCoords = texCoordsQuad[(6 - tileData.dir) % 4];
    tileVertices[3].texCoords = texCoordsQuad[(6 - tileData.dir) % 4];
    tileVertices[4].texCoords = texCoordsQuad[(7 - tileData.dir) % 4];
    tileVertices[5].texCoords = texCoordsQuad[(4 - tileData.dir) % 4];
}

void Chunk::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(vertices_, states);
}
