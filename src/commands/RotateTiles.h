#pragma once

#include <Command.h>

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class Board;

namespace commands {

class RotateTiles : public Command {
public:
    RotateTiles(Board& board, bool clockwise, bool highlightTiles);
    virtual ~RotateTiles() = default;

    void pushBackTile(const sf::Vector2i& pos);

    virtual std::string getMessage() const override;
    virtual bool isGroupingAllowed() const override;
    virtual void execute() override;
    virtual void undo() override;

private:
    Board& board_;
    std::vector<sf::Vector2i> tilePositions_;
    bool clockwise_;
    bool highlightTiles_;
};

} // namespace commands
