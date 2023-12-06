#pragma once

#include <ChunkCoords.h>

#include <algorithm>

/**
 * Two `ChunkCoords` points forming a rectangular area.
 * 
 * Very similar to `sf::IntRect` but specialized for working with `ChunkCoords`
 * instead of the top-left and size of an axis aligned rectangle.
 */
class ChunkCoordsRange {
public:
    ChunkCoordsRange() :
        left(0), top(0), width(0), height(0) {
    }
    ChunkCoordsRange(int left, int top, int width, int height) :
        left(left), top(top), width(width), height(height) {
    }
    ChunkCoordsRange(ChunkCoords::repr first, ChunkCoords::repr second) :
        left(ChunkCoords::x(first)),
        top(ChunkCoords::y(first)),
        width(ChunkCoords::x(second) - ChunkCoords::x(first) + 1),
        height(ChunkCoords::y(second) - ChunkCoords::y(first) + 1) {
    }
    int getFirstX() const {
        return left;
    }
    int getFirstY() const {
        return top;
    }
    ChunkCoords::repr getFirst() const {
        return ChunkCoords::pack(left, top);
    }
    int getSecondX() const {
        return left + width - 1;
    }
    int getSecondY() const {
        return top + height - 1;
    }
    ChunkCoords::repr getSecond() const {
        return ChunkCoords::pack(left + width - 1, top + height - 1);
    }
    bool contains(int x, int y) const {
        return (
            x >= std::min(left, left + width) && x < std::max(left, left + width) &&
            y >= std::min(top, top + height) && y < std::max(top, top + height)
        );
    }
    bool contains(ChunkCoords::repr coords) const {
        return contains(ChunkCoords::x(coords), ChunkCoords::y(coords));
    }

    int left, top, width, height;

    friend bool operator==(const ChunkCoordsRange& lhs, const ChunkCoordsRange& rhs) {
        return (
            lhs.left == rhs.left && lhs.top == rhs.top &&
            lhs.width == rhs.width && lhs.height == rhs.height
        );
    }
    friend bool operator!=(const ChunkCoordsRange& lhs, const ChunkCoordsRange& rhs) {
        return !(lhs == rhs);
    }
};
