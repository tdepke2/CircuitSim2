#include <Chunk.h>
#include <ChunkDrawable.h>
#include <Tile.h>



#include <iostream>



unsigned int ChunkDrawable::textureWidth_, ChunkDrawable::tileWidth_;
uint8_t ChunkDrawable::textureLookup_[512];
unsigned int ChunkDrawable::textureHighlightStart_;

void ChunkDrawable::setupTextureData(const sf::Vector2u& textureSize, unsigned int tileWidth) {
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

ChunkDrawable::ChunkDrawable() :
    chunk_(nullptr),
    vertices_(sf::Triangles),
    renderIndices_(),
    renderIndicesSum_(-ChunkRender::LEVELS_OF_DETAIL),
    renderDirty_() {

    renderIndices_.fill(-1);
    renderDirty_.set();
}

void ChunkDrawable::setChunk(const Chunk* chunk) {
    chunk_ = chunk;
    if (chunk != nullptr) {
        renderDirty_.set();
    }
}

const Chunk* ChunkDrawable::getChunk() const {
    return chunk_;
}

void ChunkDrawable::setRenderIndex(int levelOfDetail, int renderIndex) {
    renderIndicesSum_ -= renderIndices_[levelOfDetail];
    renderIndices_[levelOfDetail] = renderIndex;
    renderIndicesSum_ += renderIndex;
    renderDirty_.set(levelOfDetail);
}

int ChunkDrawable::getRenderIndex(int levelOfDetail) const {
    return renderIndices_[levelOfDetail];
}

bool ChunkDrawable::hasAnyRenderIndex() const {
    return renderIndicesSum_ != -ChunkRender::LEVELS_OF_DETAIL;
}

void ChunkDrawable::markDirty() {
    renderDirty_.set();
}

bool ChunkDrawable::isRenderDirty(int levelOfDetail) const {
    return renderDirty_.test(levelOfDetail);
}

void ChunkDrawable::redrawTile(unsigned int x, unsigned int y) const {
    sf::Vertex* tileVertices = &vertices_[(y * Chunk::WIDTH + x) * 6];
    TileData tileData = chunk_->tiles_[y * Chunk::WIDTH + x];

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

void ChunkDrawable::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (vertices_.getVertexCount() == 0) {
        vertices_.resize(Chunk::WIDTH * Chunk::WIDTH * 6);
        for (unsigned int y = 0; y < Chunk::WIDTH; ++y) {
            for (unsigned int x = 0; x < Chunk::WIDTH; ++x) {
                sf::Vertex* tileVertices = &vertices_[(y * Chunk::WIDTH + x) * 6];

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
    } else {
        for (unsigned int y = 0; y < Chunk::WIDTH; ++y) {
            for (unsigned int x = 0; x < Chunk::WIDTH; ++x) {
                redrawTile(x, y);
            }
        }
    }

    target.draw(vertices_, states);
}
