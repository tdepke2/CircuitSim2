#include <Chunk.h>
#include <tiles/Blank.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>




#include <iomanip>

uint8_t textureLookup[512];
void buildTextureLookup() {
    uint8_t textureId = 0;
    for (TileId::t id = TileId::blank; id < TileId::count; id = static_cast<TileId::t>(id + 1)) {
        if (id == TileId::blank) {
            textureLookup[id] = textureId++;
        } else if (id < TileId::wireCrossover) {
            textureLookup[(static_cast<uint16_t>(State::disconnected) << 7) | (static_cast<uint16_t>(State::low) << 5) | id] = textureId++;
            textureLookup[(static_cast<uint16_t>(State::disconnected) << 7) | (static_cast<uint16_t>(State::high) << 5) | id] = textureId++;
            textureLookup[(static_cast<uint16_t>(State::disconnected) << 7) | (static_cast<uint16_t>(State::middle) << 5) | id] = textureId++;
        } else if (id == TileId::wireCrossover) {
            // FIXME need to continue here.
        }
    }
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
