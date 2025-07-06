#pragma once

#include <memory>
#include <ostream>
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

protected:
    virtual bool equals(const Entity& rhs) const = 0;
    virtual void print(std::ostream& out) const = 0;

private:
    Chunk* chunk_;
    unsigned int tileIndex_;

    friend bool operator==(const Entity& lhs, const Entity& rhs);
    friend bool operator!=(const Entity& lhs, const Entity& rhs);
    friend std::ostream& operator<<(std::ostream& out, const Entity& entity);
};
