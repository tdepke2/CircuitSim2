#include <Chunk.h>

Chunk::Chunk(unsigned int textureWidth, unsigned int tileWidth) :
    textureWidth_(textureWidth),
    tileWidth_(tileWidth) {

    for (unsigned int i = 0; i < WIDTH * WIDTH; ++i) {
        tileStates_[i] = i;
    }

    vertices_.setPrimitiveType(sf::Triangles);
    vertices_.resize(WIDTH * WIDTH * 6);
    for (unsigned int y = 0; y < WIDTH; ++y) {
        for (unsigned int x = 0; x < WIDTH; ++x) {
            unsigned int tileState = tileStates_[y * WIDTH + x];
            sf::Vertex* tileVertices = &vertices_[(y * WIDTH + x) * 6];

            float px = x * tileWidth_;
            float py = y * tileWidth_;
            tileVertices[0].position = {px, py};
            tileVertices[1].position = {px + tileWidth_, py};
            tileVertices[2].position = {px + tileWidth_, py + tileWidth_};
            tileVertices[3].position = {px + tileWidth_, py + tileWidth_};
            tileVertices[4].position = {px, py + tileWidth_};
            tileVertices[5].position = {px, py};

            float tx = static_cast<unsigned int>(tileState % (textureWidth_ / tileWidth_)) * tileWidth_;
            float ty = static_cast<unsigned int>(tileState / (textureWidth_ / tileWidth_)) * tileWidth_;
            tileVertices[0].texCoords = {tx, ty};
            tileVertices[1].texCoords = {tx + tileWidth_, ty};
            tileVertices[2].texCoords = {tx + tileWidth_, ty + tileWidth_};
            tileVertices[3].texCoords = {tx + tileWidth_, ty + tileWidth_};
            tileVertices[4].texCoords = {tx, ty + tileWidth_};
            tileVertices[5].texCoords = {tx, ty};
        }
    }
}

void Chunk::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(vertices_, states);
}
