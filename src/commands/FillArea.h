#pragma once

#include <commands/PlaceTiles.h>

class Board;
class TilePool;

namespace commands {

class FillArea : public PlaceTiles {
public:
    FillArea(Board& board, TilePool& pool);
    virtual ~FillArea() = default;

    virtual std::string getMessage() const override;
    virtual void execute() override;
    virtual void undo() override;
};

} // namespace commands
