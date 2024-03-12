#pragma once

#include <Command.h>
#include <Tile.h>

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class Board;

namespace commands {

class ToggleTiles : public Command {
public:
    ToggleTiles(Board& board, bool highlightTiles);
    virtual ~ToggleTiles() = default;

    void pushBackTile(const sf::Vector2i& pos);

    virtual std::string getMessage() const override;
    virtual bool isGroupingAllowed() const override;
    virtual void execute() override;
    virtual void undo() override;

private:
    struct ToggleData {
        sf::Vector2i pos;
        State::t state1;
        State::t state2;
        State::t newState;
    };

    Board& board_;
    std::vector<ToggleData> toggleData_;
    bool highlightTiles_;
};

} // namespace commands
