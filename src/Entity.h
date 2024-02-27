#pragma once

class Chunk;

class Entity {
public:
    Entity(Chunk& chunk, unsigned int tileIndex);
    virtual ~Entity() = default;
    Entity(const Entity& rhs) = delete;
    Entity(Entity&& rhs) noexcept = default;
    Entity& operator=(const Entity& rhs) = delete;
    Entity& operator=(Entity&& rhs) noexcept = delete;    // FIXME: determine why this needs to be deleted now.

    Chunk& getChunk();
    const Chunk& getChunk() const;
    unsigned int getIndex() const;

private:
    Chunk& chunk_;
    unsigned int tileIndex_;
};
