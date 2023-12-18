#include <ChunkGroup.h>
#include <Tile.h>
#include <tiles/Wire.h>

#include <utility>

ChunkGroup::ChunkGroup() :
    chunks_(),
    chunkDrawables_() {

    auto coords = ChunkCoords::pack(0, 0);
    auto chunk = chunks_.emplace(std::piecewise_construct, std::forward_as_tuple(coords), std::forward_as_tuple(nullptr, coords)).first;
    chunkDrawables_[coords].setChunk(&chunk->second);
    chunk->second.accessTile(0).setType(tiles::Wire::instance(), TileId::wireTee, Direction::north, State::high);
}

void ChunkGroup::setRenderArea(const OffsetView& offsetView, float zoom) {
    
}

void ChunkGroup::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(chunkDrawables_.at(ChunkCoords::pack(0, 0)), states);
}
