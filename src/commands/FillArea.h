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

private:
    // FIXME: track highlights with a bool? we may have tiles that are not placed but still part of the highlight tho, like when filling while pasting area
};

} // namespace commands
