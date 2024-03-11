#pragma once

#include <Command.h>

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class Board;

namespace commands {

class FlipTiles : public Command {
public:
    FlipTiles(Board& board, bool acrossVertical, bool highlightTiles);
    virtual ~FlipTiles() = default;

    void pushBackTile(const sf::Vector2i& pos);

    virtual std::string getMessage() const override;
    virtual bool isGroupingAllowed() const override;
    virtual void execute() override;
    virtual void undo() override;

private:
    Board& board_;
    std::vector<sf::Vector2i> tilePositions_;
    bool acrossVertical_;
    bool highlightTiles_;
};

} // namespace commands
