#ifndef _TILE_H
#define _TILE_H

class Board;

#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

enum Direction : int {
    NORTH = 0, EAST, SOUTH, WEST
};

class Tile {    // Generic tile that is stored in a Board.
    public:
    Tile();
    Tile(const Vector2u& position, Board& board);
    virtual ~Tile();
    virtual int getTextureID() const;
    const Vector2u& getPosition() const;
    void setPosition(const Vector2u& position, Board& board);
    
    protected:
    Vector2u _position;
};

#endif