#pragma once

#include <Chunk.h>
#include <Tile.h>

#include <memory>

class Entity;
struct TileData;

/**
 * Abstract interface for tile behavior.
 * 
 * Defines common operations for all tiles. These should usually be called
 * through a `Tile` instance instead of directly. Derived classes should be
 * implemented as stateless singletons as the state is passed into the function
 * and global access is needed.
 */
class TileType {
public:
    TileType() = default;
    TileType(const TileType& rhs) = delete;
    TileType& operator=(const TileType& rhs) = delete;

    virtual void destroy(Chunk& chunk, unsigned int tileIndex);

    virtual void setDirection(Chunk& chunk, unsigned int tileIndex, Direction::t direction);
    virtual void setHighlight(Chunk& chunk, unsigned int tileIndex, bool highlight);
    virtual void setState(Chunk& chunk, unsigned int tileIndex, State::t state);
    virtual TileId::t getId(const Chunk& chunk, unsigned int tileIndex) const;
    virtual Direction::t getDirection(const Chunk& chunk, unsigned int tileIndex) const;
    virtual bool getHighlight(const Chunk& chunk, unsigned int tileIndex) const;
    virtual State::t getState(const Chunk& chunk, unsigned int tileIndex) const;
    TileData getRawData(const Chunk& chunk, unsigned int tileIndex) const;
    virtual void flip(Chunk& chunk, unsigned int tileIndex, bool acrossHorizontal) = 0;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) = 0;
    // Note: this method is not marked const to allow implementations to call
    // their non-const member functions on the target, such as init().
    virtual void cloneTo(const Chunk& chunk, unsigned int tileIndex, Tile target) = 0;

protected:
    ~TileType() = default;

    inline TileData& modifyTileData(Chunk& chunk, unsigned int tileIndex) {
        chunk.markTileDirty(tileIndex);
        return chunk.tiles_[tileIndex];
    }
    inline const TileData& getTileData(const Chunk& chunk, unsigned int tileIndex) const {
        return chunk.tiles_[tileIndex];
    }
    // Alternative method to modifyTileData() that would allow more flexibility
    // in marking the tile dirty (such as updating the state of the tile without
    // forcing redraw).
    //inline void markTileDirty(Chunk& chunk, unsigned int tileIndex) {
    //    chunk.markTileDirty(tileIndex);
    //}

    inline void allocateEntity(Chunk& chunk, unsigned int tileIndex, std::unique_ptr<Entity>&& entity) {
        chunk.allocateEntity(tileIndex, std::move(entity));
    }
    inline void freeEntity(Chunk& chunk, unsigned int tileIndex) {
        chunk.freeEntity(tileIndex);
    }
    inline Entity* modifyEntity(Chunk& chunk, unsigned int tileIndex) {
        return chunk.entities_[chunk.tiles_[tileIndex].meta].get();
    }
    inline const Entity* getEntity(const Chunk& chunk, unsigned int tileIndex) const {
        return chunk.entities_[chunk.tiles_[tileIndex].meta].get();
    }
};
