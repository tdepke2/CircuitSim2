#ifndef _TILE_H
#define _TILE_H

class Board;

#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class Tile {    // Generic tile that is stored in a Board.
    public:
    Tile(const Vector2u& position, const Board& board);
    virtual ~Tile();
    virtual int getID() const;
    const Vector2u& getPosition() const;
    void setPosition(const Vector2u& position, const Board& board);
    
    private:
    Vector2u _position;
};

#endif