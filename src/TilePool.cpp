#include <Tile.h>
#include <TilePool.h>
#include <tiles/Blank.h>

#include <algorithm>
#include <cassert>
#include <limits>
#include <spdlog/spdlog.h>

namespace {

constexpr int CHUNK_AREA = Chunk::WIDTH * Chunk::WIDTH;

}

TilePool::TilePool() :
    items_(),
    baseId_(0),
    upperId_(std::numeric_limits<decltype(upperId_)>::max()) {
}

std::pair<Tile, uint64_t> TilePool::allocateTile() {
    ++upperId_;
    const auto itemIndex = static_cast<size_t>((upperId_ - baseId_) / CHUNK_AREA);
    if (itemIndex == items_.size()) {
        items_.emplace_back(new PoolItem());
    }
    //spdlog::debug("TilePool allocating id {} at index {} (items size is {}).", upperId_, itemIndex, items_.size());

    PoolItem* item = items_[itemIndex].get();
    item->allocated.set(upperId_ % CHUNK_AREA);
    ++item->allocatedCount;
    return {
        item->chunk.accessTile(upperId_ % CHUNK_AREA),
        upperId_
    };
}

void TilePool::freeTile(uint64_t id) {
    const auto itemIndex = static_cast<size_t>((id - baseId_) / CHUNK_AREA);
    //spdlog::debug("TilePool freeing id {} at index {} (items size is {}).", id, itemIndex, items_.size());

    PoolItem* item = items_[itemIndex].get();
    assert(item->allocated.test(id % CHUNK_AREA));
    item->chunk.accessTile(id % CHUNK_AREA).setType(tiles::Blank::instance());
    item->allocated.reset(id % CHUNK_AREA);
    --item->allocatedCount;

    if (item->allocatedCount == 0 && (upperId_ - baseId_) / CHUNK_AREA > itemIndex) {
        spdlog::debug("TilePool item at index {} is now empty.", itemIndex);

        // Move the item to the end of the vector, and leave a nullptr in its
        // place. The nullptrs get cleaned up if we're at the first item.
        items_.emplace_back(items_[itemIndex].release());
        if (itemIndex == 0) {
            auto firstNonNull = std::find_if(items_.begin(), items_.end(),
                [](const decltype(items_)::value_type& v) {
                    return v != nullptr;
                }
            );
            baseId_ += (firstNonNull - items_.begin()) * CHUNK_AREA;
            spdlog::debug("TilePool removing {} null items, baseId = {}, upperId = {}.", firstNonNull - items_.begin(), baseId_, upperId_);
            items_.erase(items_.begin(), firstNonNull);
        }
    }
}

Tile TilePool::accessTile(uint64_t id) {
    spdlog::debug("TilePool access id {}.", id);
    PoolItem* item = items_[static_cast<size_t>((id - baseId_) / CHUNK_AREA)].get();
    assert(item->allocated.test(id % CHUNK_AREA));
    return item->chunk.accessTile(id % CHUNK_AREA);
}

void TilePool::debugPrintInfo() {
    spdlog::debug("TilePool info: pool has {} items, baseId = {}, upperId = {}", items_.size(), baseId_, upperId_);
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i] == nullptr) {
            spdlog::debug("  item {}: null", i);
        } else {
            spdlog::debug("  item {}: {} allocated", i, items_[i]->allocatedCount);
        }
    }
}
