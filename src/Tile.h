#pragma once

#include <cassert>
#include <cstdint>
#include <utility>

class Chunk;
struct TileData;
class TileType;

namespace tiles {
    class Blank;
    class Gate;
    class Input;
    class Label;
    class Led;
    class Wire;
}

namespace TileId {
    enum t : uint8_t {
        blank = 0,
        wireStraight, wireCorner, wireTee, wireJunction, wireCrossover,
        inSwitch, inButton, outLed,
        gateDiode, gateBuffer, gateNot, gateAnd, gateNand, gateOr, gateNor, gateXor, gateXnor,
        label,
        count
    };
}

namespace State {
    enum t : uint8_t {
        disconnected = 0, low, high, middle
    };
}

namespace Direction {
    enum t : uint8_t {
        north = 0, east, south, west
    };
}

/**
 * Provides an interface for a `TileData` object in a `Chunk`. The actual type
 * of the tile and state are stored separately as per the Type Object pattern.
 * 
 * The state of a tile belongs only in the `TileData` and the owning chunk, so
 * tile types are stateless classes. This is backwards from typical
 * object-oriented design but allows us to avoid some dynamic allocation and
 * storing duplicate state of a tile within a wrapper class.
 */
class Tile {
public:
    Tile(TileType* type, Chunk& chunk, unsigned int tileIndex);
    void setType(tiles::Blank* type);
    void setType(tiles::Gate* type, TileId::t gateId = TileId::gateDiode, Direction::t direction = Direction::north, State::t state = State::low);
    void setType(tiles::Input* type, TileId::t inputId = TileId::inSwitch, State::t state = State::low, char keycode = ' ');
    void setType(tiles::Label* type);
    void setType(tiles::Led* type, State::t state = State::low);
    void setType(tiles::Wire* type, TileId::t wireId = TileId::wireStraight, Direction::t direction = Direction::north, State::t state1 = State::low, State::t state2 = State::low);
    TileType* getType();
    const TileType* getType() const;
    Chunk& getChunk();
    const Chunk& getChunk() const;
    unsigned int getIndex() const;

    void setDirection(Direction::t direction);
    void setHighlight(bool highlight);
    void setState(State::t state);
    TileId::t getId() const;
    Direction::t getDirection() const;
    bool getHighlight() const;
    State::t getState() const;
    TileData getRawData() const;
    bool isTileEntity() const;

    void flip(bool acrossVertical);
    void alternativeTile();
    // Overloads on r-value references are provided for cases where the target
    // does not need to be left in a valid state (its tileType_ is not updated).
    void cloneTo(Tile& target) const;
    void cloneTo(Tile&& target) const;
    void swapWith(Tile& target);
    void swapWith(Tile&& target);

private:
    TileType* tileType_;
    Chunk& chunk_;
    unsigned int tileIndex_;

public:
    // Calls a member function from a derived `TileType` class. This allows
    // calling member functions that are specific to the derived class and not
    // exposed in the `Tile` interface.
    template<typename T, typename Func, typename... Args>
    inline auto call(Func func, Args&&... args) -> decltype((static_cast<T*>(tileType_)->*func)(chunk_, tileIndex_, std::forward<Args>(args)...)) {
        assert(tileType_ == T::instance());
        return (static_cast<T*>(tileType_)->*func)(chunk_, tileIndex_, std::forward<Args>(args)...);
    }
    template<typename T, typename Func, typename... Args>
    inline auto call(Func func, Args&&... args) const -> decltype((static_cast<const T*>(tileType_)->*func)(chunk_, tileIndex_, std::forward<Args>(args)...)) {
        assert(tileType_ == T::instance());
        return (static_cast<const T*>(tileType_)->*func)(chunk_, tileIndex_, std::forward<Args>(args)...);
    }
};
