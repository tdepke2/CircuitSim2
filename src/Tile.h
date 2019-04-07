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
    bool stateChange;
    
    Tile();
    Tile(Board* boardPtr, const Vector2u& position);
    virtual ~Tile();
    virtual int getTextureID() const;
    const Vector2u& getPosition() const;
    Direction getDirection() const;
    bool getHighlight() const;
    void setPosition(const Vector2u& position, Board& board, bool keepOverwrittenTile = false);
    void setHighlight(bool highlight);
    virtual void setDirection(Direction direction, Board& board);
    virtual void flip(bool acrossHorizontal, Board& board);
    virtual Tile* clone(Board* boardPtr, const Vector2u& position);
    
    protected:
    Board* _boardPtr;
    Vector2u _position;
    Direction _direction;
    bool _highlight;
};

#endif