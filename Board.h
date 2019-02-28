#ifndef _BOARD_H
#define _BOARD_H

#include <SFML/Graphics.hpp>
#include <string>

using namespace std;
using namespace sf;

class Board : public Drawable, public Transformable {    // Class for a circuit board with logic components that can be drawn to the window.
    public:
    bool gridActive;
    
    Board();
    void loadTextures(const string& filenameNoGrid, const string& filenameGrid, const Vector2u& tileSize);
    void resizeBoard(const Vector2u& boardSize);
    
    private:
    VertexArray _vertices;
    Texture _tilesetNoGrid, _tilesetGrid;
    Vector2u _tileSize, _boardSize;
    
    virtual void draw (RenderTarget& target, RenderStates states) const;
};

#endif