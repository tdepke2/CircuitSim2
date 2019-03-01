#ifndef _BOARD_H
#define _BOARD_H

class Tile;

#include <SFML/Graphics.hpp>
#include <string>

using namespace std;
using namespace sf;

class Board : public Drawable, public Transformable {    // Class for a circuit board with logic components that can be drawn to the window.
    public:
    bool gridActive;
    
    Board();
    virtual ~Board();
    Tile*** getTileArray() const;
    void loadTextures(const string& filenameGrid, const string& filenameNoGrid, const Vector2u& tileSize);
    void resize(const Vector2u& boardSize);
    void redrawTile(Tile* tile);
    
    private:
    VertexArray _vertices;
    Texture _tilesetGrid, _tilesetNoGrid;
    Vector2u _tileSize, _boardSize;
    Tile*** _tileArray;
    
    virtual void draw (RenderTarget& target, RenderStates states) const;
};

#endif