#pragma once

#include <Command.h>

#include <SFML/Graphics.hpp>
#include <vector>

class Board;
class Tile;
class TilePool;

namespace commands {

class WriteTiles : public Command {
public:
    WriteTiles(Board& board, TilePool& pool);
    virtual ~WriteTiles();

    size_t getTileCount() const;
    Tile accessTile(size_t index);
    Tile pushBackTile(const sf::Vector2i& pos);

    virtual void execute() override;
    virtual void undo() override;

private:
    Board& board_;
    TilePool& pool_;
    std::vector<size_t> poolSectors_;
    std::vector<sf::Vector2i> tilePositions_;
    size_t lastExecuteSize_;
};

} // namespace commands
