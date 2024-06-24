#include <MakeUnique.h>
#include <Tile.h>
#include <TilePool.h>
#include <tiles/Blank.h>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <spdlog/spdlog.h>

namespace {

constexpr int CHUNK_AREA = Chunk::WIDTH * Chunk::WIDTH;
constexpr int SECTORS_PER_CHUNK = CHUNK_AREA / TilePool::SECTOR_SIZE;

}

TilePool::TilePool() :
    items_(),
    itemsAllocatedCount_() {
}

unsigned int TilePool::getTotalAllocated() const {
    return std::accumulate(itemsAllocatedCount_.begin(), itemsAllocatedCount_.end(), 0);
}

size_t TilePool::allocateSector() {
    /*++upperId_;
    const auto itemIndex = static_cast<size_t>((upperId_ - baseId_) / CHUNK_AREA);
    if (itemIndex == items_.size()) {
        items_.emplace_back(details::make_unique<PoolItem>());
    }
    //spdlog::debug("TilePool allocating id {} at index {} (items size is {}).", upperId_, itemIndex, items_.size());

    PoolItem* item = items_[itemIndex].get();
    item->allocated.set(upperId_ % CHUNK_AREA);
    ++item->allocatedCount;
    return {
        item->chunk.accessTile(upperId_ % CHUNK_AREA),
        upperId_
    };*/

    for (size_t i = 0; i < itemsAllocatedCount_.size(); ++i) {
        if (itemsAllocatedCount_[i] < SECTORS_PER_CHUNK) {
            for (int j = 0; j < SECTORS_PER_CHUNK; ++j) {
                if (!items_[i]->allocated.test(j)) {
                    items_[i]->allocated.set(j);
                    ++itemsAllocatedCount_[i];
                    spdlog::debug("TilePool allocating sector {} at index {} (items size is {}).", i * SECTORS_PER_CHUNK + j, i, items_.size());
                    return i * SECTORS_PER_CHUNK + j;
                }
            }
        }
    }
    items_.emplace_back(details::make_unique<PoolItem>());
    items_.back()->allocated.set(0);
    itemsAllocatedCount_.push_back(1);
    spdlog::debug("TilePool allocating sector {} at end (items size is {}).", (items_.size() - 1) * SECTORS_PER_CHUNK, items_.size());
    return (items_.size() - 1) * SECTORS_PER_CHUNK;
}

void TilePool::freeSector(size_t sectorId) {
    /*const auto itemIndex = static_cast<size_t>((id - baseId_) / CHUNK_AREA);
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
    }*/

    spdlog::debug("TilePool freeing sector {} at index {} (items size is {}).", sectorId, sectorId / SECTORS_PER_CHUNK, items_.size());
    items_[sectorId / SECTORS_PER_CHUNK]->allocated.reset(sectorId % SECTORS_PER_CHUNK);
    --itemsAllocatedCount_[sectorId / SECTORS_PER_CHUNK];
}

Tile TilePool::accessTile(size_t sectorId, unsigned int offset) {
    /*spdlog::debug("TilePool access id {}.", id);
    PoolItem* item = items_[static_cast<size_t>((id - baseId_) / CHUNK_AREA)].get();
    assert(item->allocated.test(id % CHUNK_AREA));
    return item->chunk.accessTile(id % CHUNK_AREA);*/

    spdlog::debug("TilePool access sector {} with offset {}.", sectorId, offset);
    PoolItem* item = items_[sectorId / SECTORS_PER_CHUNK].get();
    assert(item->allocated.test(sectorId % SECTORS_PER_CHUNK) && offset < SECTOR_SIZE);
    return item->chunk.accessTile(sectorId % SECTORS_PER_CHUNK * SECTOR_SIZE + offset);
}

void TilePool::debugPrintInfo() {
    /*spdlog::debug("TilePool info: pool has {} items, baseId = {}, upperId = {}", items_.size(), baseId_, upperId_);
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i] == nullptr) {
            spdlog::debug("  item {}: null", i);
        } else {
            spdlog::debug("  item {}: {} allocated", i, items_[i]->allocatedCount);
        }
    }*/

    spdlog::debug("TilePool info: pool has {} items", items_.size());
    for (size_t i = 0; i < items_.size(); ++i) {
        spdlog::debug("  item {}: {} allocated, {:#018x}", i, itemsAllocatedCount_[i], items_[i]->allocated.to_ullong());
    }
}
