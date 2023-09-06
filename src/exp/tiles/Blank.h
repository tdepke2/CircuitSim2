#pragma once

#include <TileType.h>



#include <iostream>

class Blank : public TileType {
public:
    static Blank* instance();
    Blank(const Blank& blank) = delete;
    Blank(Blank&& blank) = delete;
    Blank& operator=(const Blank& blank) = delete;
    Blank& operator=(Blank&& blank) = delete;

    virtual void flip(Chunk& chunk, unsigned int tileIndex, bool acrossHorizontal) override;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) override;

private:
    Blank() {
        std::cout << "Blank class has been constructed.\n";
    }

    void init(Chunk& chunk, unsigned int tileIndex);

    friend class Tile;
};
