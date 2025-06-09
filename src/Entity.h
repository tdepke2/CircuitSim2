#pragma once

#include <memory>
#include <vector>

class Chunk;

class Entity {
public:
    Entity(Chunk& chunk, unsigned int tileIndex);
    virtual ~Entity() = default;
    Entity(const Entity& rhs) = delete;
    Entity(Entity&& rhs) noexcept = default;
    Entity& operator=(const Entity& rhs) = delete;
    Entity& operator=(Entity&& rhs) noexcept = default;

    virtual void setChunkAndIndex(Chunk& chunk, unsigned int tileIndex);
    Chunk& getChunk();
    const Chunk& getChunk() const;
    unsigned int getIndex() const;

    virtual std::vector<char> serialize() const = 0;
    virtual void deserialize(const std::vector<char>& data) = 0;
    virtual std::unique_ptr<Entity> clone(Chunk& chunk, unsigned int tileIndex) const = 0;

private:
    Chunk* chunk_;
    unsigned int tileIndex_;
};
