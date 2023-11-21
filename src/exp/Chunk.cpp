#include <Chunk.h>
#include <tiles/Blank.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>

#include <array>
#include <iomanip>

// Disable a false-positive warning issue with gcc:
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdangling-reference"
#endif
    #include <spdlog/fmt/bundled/ostream.h>
    #include <spdlog/spdlog.h>
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic pop
#endif

TileData::TileData(TileId::t id, State::t state1, State::t state2, Direction::t dir, bool highlight, uint16_t meta) :
    id(id),
    state1(state1),
    state2(state2),
    dir(dir),
    highlight(highlight),
    meta(meta) {
}

uint16_t TileData::getTextureHash() const {
    return (static_cast<uint16_t>(state2) << 7) | (static_cast<uint16_t>(state1) << 5) | id;
}

constexpr int Chunk::WIDTH;

std::array<TileType*, TileId::count> tileIdToType = {};

void Chunk::setupChunks() {
    TileId::t id = TileId::blank;
    for (; id <= TileId::blank; id = static_cast<TileId::t>(id + 1)) {
        tileIdToType[id] = tiles::Blank::instance();
    }
    for (; id <= TileId::wireCrossover; id = static_cast<TileId::t>(id + 1)) {
        tileIdToType[id] = tiles::Wire::instance();
    }
    for (; id <= TileId::inButton; id = static_cast<TileId::t>(id + 1)) {
        tileIdToType[id] = tiles::Input::instance();
    }
    for (; id <= TileId::outLed; id = static_cast<TileId::t>(id + 1)) {
        tileIdToType[id] = tiles::Led::instance();
    }
    for (; id <= TileId::gateXnor; id = static_cast<TileId::t>(id + 1)) {
        tileIdToType[id] = tiles::Gate::instance();
    }
}

Chunk::Chunk() :
    tiles_{} {
}

Tile Chunk::accessTile(unsigned int x, unsigned int y) {
    TileData tileData = tiles_[y * WIDTH + x];
    return {tileIdToType[tileData.id], *this, y * WIDTH + x};
}

void Chunk::debugPrintChunk() {
    spdlog::debug("chunk:\n{}", *this);
}

template<> struct fmt::formatter<Chunk> : fmt::ostream_formatter {};
std::ostream& operator<<(std::ostream& out, const Chunk& chunk) {
    for (unsigned int y = 0; y < Chunk::WIDTH; ++y) {
        for (unsigned int x = 0; x < Chunk::WIDTH; ++x) {
            out << std::setw(8) << std::hex << *reinterpret_cast<const uint32_t*>(&chunk.tiles_[y * Chunk::WIDTH + x]) << std::dec << " ";
        }
        out << "\n";
    }
    return out;
}

/*
// Just for reference, an alternative method to fmt formatting:
// https://fmt.dev/latest/api.html#udt
template<>
struct fmt::formatter<my_type> {
    constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator {
        return ctx.end();
    }
    auto format(const my_type& t, format_context& ctx) const -> format_context::iterator {
        return format_to(ctx.out(), "[my_type x={}]", t.x);
    }
};
*/
