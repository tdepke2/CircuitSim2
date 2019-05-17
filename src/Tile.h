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
    static unsigned int currentUpdateTime;
    
    Tile();
    Tile(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false, bool suppressUpdate = false);
    virtual ~Tile();
    virtual int getTextureID() const;
    const Vector2u& getPosition() const;
    Direction getDirection() const;
    bool getHighlight() const;
    void setPosition(const Vector2u& position, bool keepOverwrittenTile = false);
    virtual void setDirection(Direction direction);
    void setHighlight(bool highlight);
    virtual void setState(State state);
    virtual void flip(bool acrossHorizontal);
    virtual State checkOutput(Direction direction) const;
    pair<State, Tile*> checkState(Direction direction) const;
    virtual void addUpdate(bool isCosmetic = false, bool noAdjacentUpdates = false);
    virtual void followWire(Direction direction, State state);
    virtual Tile* clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false);
    
    protected:
    Board* _boardPtr;
    Vector2u _position;
    Direction _direction;
    bool _highlight;
    
    void _updateAdjacentTiles();
};

#endif