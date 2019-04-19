#ifndef _TILE_H
#define _TILE_H

class Board;

#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

enum Direction : int {
    NORTH = 0, EAST, SOUTH, WEST
};

enum State : int {
    DISCONNECTED = 0, LOW, HIGH, HIGH_Z, INVALID
};

class Tile {    // Generic tile that is stored in a Board.
    public:
    Tile();
    Tile(Board* boardPtr, const Vector2u& position, bool suppressUpdate = false);
    virtual ~Tile();
    virtual int getTextureID() const;
    const Vector2u& getPosition() const;
    Direction getDirection() const;
    bool getHighlight() const;
    void setPosition(const Vector2u& position, Board& board, bool keepOverwrittenTile = false);
    void setHighlight(bool highlight);
    virtual void setDirection(Direction direction, Board& board);
    virtual void flip(bool acrossHorizontal, Board& board);
    virtual State checkOutput(Direction direction) const;
    pair<State, Tile*> checkState(Direction direction) const;
    virtual void addUpdate(bool isCosmetic = false);
    virtual Tile* clone(Board* boardPtr, const Vector2u& position);
    
    protected:
    Board* _boardPtr;
    Vector2u _position;
    Direction _direction;
    bool _highlight;
};

#endif