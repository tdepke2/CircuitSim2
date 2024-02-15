#include <commands/WriteTile.h>

namespace commands {

WriteTile::WriteTile(Board& board, const sf::Vector2i& tilePos) :
    board_(board),
    tilePos_(tilePos) {
}

void WriteTile::execute() {

}

void WriteTile::undo() {

}

}
