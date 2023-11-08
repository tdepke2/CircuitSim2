#pragma once

#include <cstdint>
#include <limits>

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
};
