#pragma once

#include <Tile.h>
#include <TileType.h>

class Chunk;

namespace entities {
    class Label;
}

namespace tiles {

class Label : public TileType {
public:
    static Label* instance();
    virtual ~Label() = default;
    Label(const Label& label) = delete;
    Label(Label&& label) = delete;
    Label& operator=(const Label& label) = delete;
    Label& operator=(Label&& label) = delete;

    const entities::Label* getEntity(const Chunk& chunk, unsigned int tileIndex) const;
    entities::Label* modifyEntity(Chunk& chunk, unsigned int tileIndex);

    virtual bool isTileEntity() const override;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) override;
    virtual void cloneTo(const Chunk& chunk, unsigned int tileIndex, Tile target) override;

private:
    Label();

    void init(Chunk& chunk, unsigned int tileIndex, const entities::Label* labelToCopy = nullptr);
    virtual void destroy(Chunk& chunk, unsigned int tileIndex) override;

    friend class ::Tile;
};

} // namespace tiles
