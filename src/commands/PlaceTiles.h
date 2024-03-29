#pragma once

#include <Command.h>

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class Board;
class Tile;
class TilePool;

namespace commands {

class PlaceTiles : public Command {
public:
    PlaceTiles(Board& board, TilePool& pool);
    virtual ~PlaceTiles();

    const Board& getBoard() const;
    Board& getBoard();
    size_t getTileCount() const;
    Tile accessTile(size_t index);
    Tile pushBackTile(const sf::Vector2i& pos);

    virtual std::string getMessage() const override;
    virtual bool isGroupingAllowed() const override;
    virtual void execute() override;
    virtual void undo() override;

protected:
    void setLastExecuteSize(size_t lastExecuteSize);
    const std::vector<sf::Vector2i>& getTilePositions() const;
    size_t getLastExecuteSize() const;

private:
    Board& board_;
    TilePool& pool_;
    std::vector<size_t> poolSectors_;
    std::vector<sf::Vector2i> tilePositions_;
    size_t lastExecuteSize_;
};

} // namespace commands
