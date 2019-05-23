#ifndef _TILE_H
#define _TILE_H

class Board;

#include <SFML/Graphics.hpp>
#include <string>

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
    Tile(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false, bool suppressUpdate = false);    // Construct tile with parent board and position, noAdjacentUpdates stops updates to adjacent tiles, suppressUpdate stops the initial update to this tile.
    virtual ~Tile();
    const Vector2u& getPosition() const;
    Direction getDirection() const;
    bool getHighlight() const;
    virtual State getState() const;
    void setPosition(const Vector2u& position, bool noAdjacentUpdates = false, bool keepOverwrittenTile = false);    // Moves the tile to a new location, if keepOverwrittenTile is true then the tile that occupies the specified position will not be deleted.
    virtual void setDirection(Direction direction, bool noAdjacentUpdates = false);
    void setHighlight(bool highlight);
    virtual void setState(State state);
    virtual void flip(bool acrossHorizontal, bool noAdjacentUpdates = false);    // Flips the tile across the vertical/horizontal axis.
    virtual State checkOutput(Direction direction) const;    // Check for output from this tile on the side that is closest when travelling the given direction towards the tile.
    virtual void addUpdate(bool isCosmetic = false, bool noAdjacentUpdates = false);    // Add an update to this tile into the corresponding hash set, isCosmetic disables the state update part, noAdjacentUpdates stops updates to adjacent tiles.
    virtual void followWire(Direction direction, State state);    // Used in wire path following algorithm, traverses a wire using DFS and marks locations of endpoints to be updated later.
    virtual void redrawTile() const;
    virtual string toString() const;
    virtual Tile* clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false);    // Make a copy of this tile, the new tile needs its own board and position.
    
    protected:
    Board* _boardPtr;
    Vector2u _position;
    Direction _direction;
    bool _highlight;
    
    void _updateAdjacentTiles();    // Send a single state update to each of the tiles adjacent to this one, this does not cascade to other tiles.
};

#endif