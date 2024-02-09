#pragma once

#include <Command.h>

#include <SFML/Graphics.hpp>

class Board;

namespace commands {

class WriteTile : public Command {
public:
    WriteTile(Board& board, const sf::Vector2i& tilePos);

    void execute() override;
    void undo() override;

private:
    Board& board_;
    sf::Vector2i tilePos_;
};

} // namespace commands
