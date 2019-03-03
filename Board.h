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
    const Vector2u& getSize() const;
    const Vector2u& getTileSize() const;
    Tile*** getTileArray() const;
    void loadTextures(const string& filenameGrid, const string& filenameNoGrid, const Vector2u& tileSize);
    void resize(const Vector2u& size);
    void redrawTile(Tile* tile);
    void replaceTile(Tile* tile);
    void loadFile(const string& filename);
    
    private:
    static const string SYMBOL_INFO_TABLE[];
    VertexArray _vertices;
    Texture _tilesetGrid, _tilesetNoGrid;
    Vector2u _size, _tileSize;
    Tile*** _tileArray;
    
    virtual void draw (RenderTarget& target, RenderStates states) const;
};

#endif