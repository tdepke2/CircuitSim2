#pragma once

#include <ChunkCoords.h>

class ChunkCoordsRange {
public:
    ChunkCoordsRange() :
        left(0), top(0), width(0), height(0) {
    }
    ChunkCoordsRange(int left, int top, int width, int height) :
        left(left), top(top), width(width), height(height) {
    }
    int getLowX() const {
        return left;
    }
    int getLowY() const {
        return top;
    }
    ChunkCoords::repr getLow() const {
        return ChunkCoords::pack(left, top);
    }
    int getHighX() const {
        return left + width - 1;
    }
    int getHighY() const {
        return top + height - 1;
    }
    ChunkCoords::repr getHigh() const {
        return ChunkCoords::pack(left + width - 1, top + height - 1);
    }
    bool contains(int x, int y) const {
        // FIXME may need to adjust, see sf::Rect
        return (x >= left && x < left + width && y >= top && y < top + height);
    }
    bool contains(ChunkCoords::repr coords) const {
        return contains(ChunkCoords::x(coords), ChunkCoords::y(coords));
    }

    // FIXME Which is better? We could match sf::Rect and use left/top, but it may be useful to assume the two range points could be anything (negative width/height is weird).
    //int x1, y1, x2, y2;
    int left, top, width, height;
};
