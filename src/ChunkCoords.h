#pragma once

#include <cstdint>
#include <limits>
#include <utility>

/**
 * The x and y coordinates for a `Chunk`.
 * 
 * This is a middle ground between `std::pair` and `sf::Vector2i` in that the
 * coordinates have an explicit x/y component and a defined ordering (sorting y
 * first, then x in increasing order). This ordering is important because it
 * allows some assumptions when using a binary search over a sorted range of
 * coordinates. This may lead to some surprising values for the uint64_t though,
 * for example the zero coordinates have a value of `0x8000000080000000`.
 * 
 * I'm not sure why I decided to make it a type alias instead of a simple class
 * (although it does make it clear that it doesn't need pass-by-reference), but
 * it works.
 */
class ChunkCoords {
public:
    using repr = uint64_t;

    static inline repr pack(int x, int y) {
        return static_cast<uint64_t>(y + std::numeric_limits<int>::min()) << 32 | static_cast<uint32_t>(x + std::numeric_limits<int>::min());
    }

    static inline int x(repr coords) {
        return static_cast<int32_t>(coords) - std::numeric_limits<int>::min();
    }

    static inline int y(repr coords) {
        return static_cast<int32_t>(coords >> 32) - std::numeric_limits<int>::min();
    }

    static inline std::pair<int, int> toPair(repr coords) {
        return {x(coords), y(coords)};
    }
};
