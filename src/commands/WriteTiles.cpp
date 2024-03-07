#include <Board.h>
#include <commands/WriteTiles.h>
#include <TilePool.h>

namespace commands {

WriteTiles::WriteTiles(Board& board, TilePool& pool) :
    board_(board),
    pool_(pool),
    poolSectors_(),
    tilePositions_(),
    lastExecuteSize_(0) {
}

WriteTiles::~WriteTiles() {
    for (const auto sector : poolSectors_) {
        pool_.freeSector(sector);
    }
}

const Board& WriteTiles::getBoard() const {
    return board_;
}

Board& WriteTiles::getBoard() {
    return board_;
}

size_t WriteTiles::getTileCount() const {
    return tilePositions_.size();
}

Tile WriteTiles::accessTile(size_t index) {
    return pool_.accessTile(poolSectors_[index / TilePool::SECTOR_SIZE], index % TilePool::SECTOR_SIZE);
}

Tile WriteTiles::pushBackTile(const sf::Vector2i& pos) {
    unsigned int offset = tilePositions_.size() % TilePool::SECTOR_SIZE;
    if (offset == 0) {
        poolSectors_.push_back(pool_.allocateSector());
    }
    tilePositions_.push_back(pos);
    return pool_.accessTile(poolSectors_.back(), offset);
}

std::string WriteTiles::getMessage() const {
    return "place " + std::to_string(tilePositions_.size()) + " tiles";
}

bool WriteTiles::isGroupingAllowed() const {
    // Arbitrary limit to prevent group from growing too large.
    return tilePositions_.size() < 4096;
}

void WriteTiles::execute() {
    Command::execute();
    for (size_t i = lastExecuteSize_; i < tilePositions_.size(); ++i) {
        accessTile(i).swapWith(board_.accessTile(tilePositions_[i]));
    }
    lastExecuteSize_ = tilePositions_.size();
}

void WriteTiles::undo() {
    for (size_t i = lastExecuteSize_; i > 0; --i) {
        accessTile(i - 1).swapWith(board_.accessTile(tilePositions_[i - 1]));
    }
    lastExecuteSize_ = 0;
}

void WriteTiles::setLastExecuteSize(size_t lastExecuteSize) {
    lastExecuteSize_ = lastExecuteSize;
}

const std::vector<sf::Vector2i>& WriteTiles::getTilePositions() const {
    return tilePositions_;
}

size_t WriteTiles::getLastExecuteSize() const {
    return lastExecuteSize_;
}

} // namespace commands
