#pragma once

#include <commands/WriteTiles.h>

class Board;
class TilePool;

namespace commands {

class FillArea : public WriteTiles {
public:
    FillArea(Board& board, TilePool& pool);
    virtual ~FillArea() = default;

    virtual std::string getMessage() const override;
    virtual void execute() override;
    virtual void undo() override;
};

} // namespace commands
