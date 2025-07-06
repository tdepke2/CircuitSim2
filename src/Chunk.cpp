#include <Chunk.h>
#include <FileStorage.h>
#include <LodRenderer.h>
#include <MakeUnique.h>
#include <tiles/Blank.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Label.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>

#include <algorithm>
#include <cstring>
#include <iomanip>

// Disable a false-positive warning issue with gcc:
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdangling-reference"
#endif
    #include <spdlog/fmt/ostr.h>
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

void swap(TileData& lhs, TileData& rhs) {
    TileData temp = lhs;
    temp.highlight = rhs.highlight;
    rhs.highlight = lhs.highlight;
    lhs = std::move(rhs);
    rhs = std::move(temp);
}

bool operator==(const TileData& lhs, const TileData& rhs) {
    return (
        lhs.id == rhs.id &&
        lhs.state1 == rhs.state1 &&
        lhs.state2 == rhs.state2 &&
        lhs.dir == rhs.dir &&
        lhs.highlight == rhs.highlight &&
        lhs.meta == rhs.meta
    );
}

bool operator!=(const TileData& lhs, const TileData& rhs) {
    return !(lhs == rhs);
}

constexpr int Chunk::WIDTH;
Chunk::StaticInit* Chunk::staticInit_ = nullptr;

Chunk::StaticInit::StaticInit() {
    spdlog::debug("Chunk::StaticInit initializing.");
    tileIdToType = {};

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
    for (; id <= TileId::label; id = static_cast<TileId::t>(id + 1)) {
        tileIdToType[id] = tiles::Label::instance();
    }
    assert(id == TileId::count);
}

Chunk::Chunk(LodRenderer* lodRenderer, ChunkCoords::repr coords) :
    tiles_{},
    entities_(),
    entitiesCapacity_(0),
    lodRenderer_(lodRenderer),
    coords_(coords),
    dirtyFlags_(),
    empty_(true), 
    highlighted_(false) {

    static StaticInit staticInit;
    staticInit_ = &staticInit;
}

ChunkCoords::repr Chunk::getCoords() const {
    return coords_;
}

void Chunk::setLodRenderer(LodRenderer* lodRenderer) {
    lodRenderer_ = lodRenderer;
}

const LodRenderer* Chunk::getLodRenderer() const {
    return lodRenderer_;
}

LodRenderer* Chunk::getLodRenderer() {
    return lodRenderer_;
}

bool Chunk::isUnsaved() const {
    return dirtyFlags_.test(ChunkDirtyFlag::unsaved);
}

bool Chunk::isEmpty() const {
    if (dirtyFlags_.test(ChunkDirtyFlag::emptyIsStale)) {
        empty_ = std::all_of(tiles_, tiles_ + WIDTH * WIDTH, [](TileData tile) { return tile.id == 0; });
        dirtyFlags_.reset(ChunkDirtyFlag::emptyIsStale);
    }
    return empty_;
}

bool Chunk::isHighlighted() const {
    if (dirtyFlags_.test(ChunkDirtyFlag::highlightedIsStale)) {
        highlighted_ = std::any_of(tiles_, tiles_ + WIDTH * WIDTH, [](TileData tile) { return tile.highlight; });
        dirtyFlags_.reset(ChunkDirtyFlag::highlightedIsStale);
    }
    return highlighted_;
}

Tile Chunk::accessTile(unsigned int tileIndex) {
    return {staticInit_->tileIdToType[tiles_[tileIndex].id], *this, tileIndex};
}

uint32_t Chunk::serializeLength() const {
    if (isEmpty()) {
        return 0;
    }
    uint32_t length = sizeof(length) + WIDTH * WIDTH * sizeof(TileData);
    if (entitiesCapacity_ == 0) {
        return length;
    }
    for (unsigned int i = 0; i < WIDTH * WIDTH; ++i) {
        if (staticInit_->tileIdToType[tiles_[i].id]->isTileEntity()) {
            length += 0;//entities_[tiles_[i].meta]->serializeLength();    // FIXME: need to finish up entity serialization.
        }
    }
    return length;
}

uint32_t Chunk::serialize(std::ostream& out) const {
    if (isEmpty()) {
        return 0;
    }
    uint32_t length = serializeLength();
    auto lengthBE = FileStorage::swapHostBigEndian(length);
    out.write(reinterpret_cast<char*>(&lengthBE), sizeof(lengthBE));

    for (unsigned int i = 0; i < WIDTH * WIDTH; ++i) {
        // FIXME: For entities, we may need to update the tile's meta to point to a new index.
        TileData tile = tiles_[i];
        auto tileBE = FileStorage::swapHostBigEndian(*reinterpret_cast<uint32_t*>(&tile));
        out.write(reinterpret_cast<char*>(&tileBE), sizeof(tileBE));
    }

    uint32_t bytesWritten = sizeof(length) + WIDTH * WIDTH * sizeof(TileData);

    // serialize entities and increment bytesWritten.

    assert(length == bytesWritten);
    return length;
    /*if (entitiesCapacity_ == 0) {
        return data;
    }
    for (unsigned int i = 0; i < WIDTH * WIDTH; ++i) {
        Tile tile = accessTile(i);
        if (tile.isTileEntity()) {
            std::vector<char> entityData = entities_[tiles_[i].meta]->serialize();

        }
    }
    return data;*/
}

void Chunk::deserialize(std::istream& in) {
    uint32_t length;
    in.read(reinterpret_cast<char*>(&length), sizeof(length));
    length = FileStorage::swapHostBigEndian(length);

    //assert(length >= sizeof(length) + WIDTH * WIDTH * sizeof(TileData));    // FIXME: assert or throw exception?
    for (unsigned int i = 0; i < WIDTH * WIDTH; ++i) {
        uint32_t tile;
        in.read(reinterpret_cast<char*>(&tile), sizeof(tile));
        tile = FileStorage::swapHostBigEndian(tile);
        tiles_[i] = *reinterpret_cast<TileData*>(&tile);
    }
    dirtyFlags_.set(ChunkDirtyFlag::emptyIsStale);
    dirtyFlags_.set(ChunkDirtyFlag::highlightedIsStale);
    // FIXME: this should reset all state in the Chunk, no? should clear any entities and set capacity to zero beforehand.
}

void Chunk::markAsSaved() const {
    dirtyFlags_.reset(ChunkDirtyFlag::unsaved);
}

void Chunk::markAsDrawn() const {
    dirtyFlags_.reset(ChunkDirtyFlag::drawPending);
}

void Chunk::debugPrintChunk() const {
    spdlog::debug("{}", *this);
}

void Chunk::markTileDirty(unsigned int /*tileIndex*/) {
    if (!dirtyFlags_.test(ChunkDirtyFlag::drawPending) && lodRenderer_ != nullptr) {
        //spdlog::debug("Calling Board::markChunkDrawDirty() for chunk {}.", ChunkCoords::toPair(coords_));
        lodRenderer_->markChunkDrawDirty(coords_);
    }
    //tiles_[tileIndex].redraw = true;    // Tracking redraw per tile did not show a noticeable boost in rendering.
    dirtyFlags_.set();
}

void Chunk::markHighlightDirty(unsigned int /*tileIndex*/) {
    if (!dirtyFlags_.test(ChunkDirtyFlag::drawPending) && lodRenderer_ != nullptr) {
        lodRenderer_->markChunkDrawDirty(coords_);
    }
    dirtyFlags_.set(ChunkDirtyFlag::highlightedIsStale);
    dirtyFlags_.set(ChunkDirtyFlag::drawPending);
}

void Chunk::allocateEntity(unsigned int tileIndex, std::unique_ptr<Entity>&& entity) {
    for (size_t i = 0; i < entitiesCapacity_; ++i) {
        if (entities_[i] == nullptr) {
            tiles_[tileIndex].meta = i;
            entities_[i] = std::move(entity);
            return;
        }
    }

    size_t newCapacity = std::max(entitiesCapacity_ * 2, entitiesCapacity_ + 1);
    spdlog::debug("Chunk::allocateEntity() increasing capacity from {} to {}.", entitiesCapacity_, newCapacity);
    EntityArray newEntities = details::make_unique<std::unique_ptr<Entity>[]>(newCapacity);
    for (size_t i = 0; i < entitiesCapacity_; ++i) {
        newEntities[i] = std::move(entities_[i]);
    }

    tiles_[tileIndex].meta = entitiesCapacity_;
    newEntities[entitiesCapacity_] = std::move(entity);
    entities_ = std::move(newEntities);
    entitiesCapacity_ = newCapacity;
}

void Chunk::freeEntity(unsigned int tileIndex) {
    entities_[tiles_[tileIndex].meta].reset();

    // FIXME: as an improvement, we could track the number of allocated entities and choose to reduce the capacity if needed.
}

bool operator==(const Chunk& lhs, const Chunk& rhs) {
    for (size_t i = 0; i < static_cast<size_t>(Chunk::WIDTH * Chunk::WIDTH); ++i) {
        if (lhs.tiles_[i] != rhs.tiles_[i]) {
            return false;
        }
    }
    size_t i = 0;
    while (i < lhs.entitiesCapacity_) {
        if (lhs.entities_[i] != nullptr &&
            (i >= rhs.entitiesCapacity_ || rhs.entities_[i] == nullptr || *lhs.entities_[i] != *rhs.entities_[i])) {
            return false;
        }
        ++i;
    }
    while (i < rhs.entitiesCapacity_) {
        if (rhs.entities_[i] != nullptr &&
            (i >= lhs.entitiesCapacity_ || lhs.entities_[i] == nullptr || *lhs.entities_[i] != *rhs.entities_[i])) {
            return false;
        }
        ++i;
    }
    return true;
}

bool operator!=(const Chunk& lhs, const Chunk& rhs) {
    return !(lhs == rhs);
}

template<> struct fmt::formatter<Chunk> : fmt::ostream_formatter {};
std::ostream& operator<<(std::ostream& out, const Chunk& chunk) {
    out << "-- tiles --\n";
    for (unsigned int y = 0; y < Chunk::WIDTH; ++y) {
        for (unsigned int x = 0; x < Chunk::WIDTH; ++x) {
            out << std::setw(8) << std::hex << *reinterpret_cast<const uint32_t*>(&chunk.tiles_[y * Chunk::WIDTH + x]) << std::dec << " ";
        }
        out << "\n";
    }
    out << "-- entities --\n";
    for (size_t i = 0; i < chunk.entitiesCapacity_; ++i) {
        if (chunk.entities_[i] == nullptr) {
            out << i << ": null\n";
        } else {
            out << i << ": " << *chunk.entities_[i] << "\n";
        }
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
