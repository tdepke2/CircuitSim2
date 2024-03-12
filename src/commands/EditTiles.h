#pragma once

#include <commands/PlaceTiles.h>

#include <string>

class Board;
class TilePool;

namespace commands {

class EditTiles : public PlaceTiles {
public:
    EditTiles(Board& board, TilePool& pool);
    virtual ~EditTiles() = default;

    virtual std::string getMessage() const override;
};

} // namespace commands
