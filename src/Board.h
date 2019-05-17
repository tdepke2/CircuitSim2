#ifndef _BOARD_H
#define _BOARD_H

class Tile;
class TileButton;
class TileGate;
class TileLED;
class TileSwitch;
class TileWire;

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;
using namespace sf;

class Board : public Drawable, public Transformable {    // Class for a circuit board with logic components that can be drawn to the window.
    public:
    static bool gridActive;
    string name;
    unordered_set<Tile*> cosmeticUpdates;
    unordered_set<TileWire*> wireUpdates;
    unordered_set<TileSwitch*> switchUpdates;
    unordered_set<TileButton*> buttonUpdates;
    unordered_set<TileLED*> LEDUpdates;
    unordered_set<TileGate*> gateUpdates;
    unordered_map<char, vector<TileSwitch*>> switchKeybinds;
    unordered_map<char, vector<TileButton*>> buttonKeybinds;
    
    static void loadTextures(const string& filenameGrid, const string& filenameNoGrid, const Vector2u& tileSize);
    static const Vector2u& getTileSize();
    Board();
    virtual ~Board();
    const Vector2u& getSize() const;
    Tile* getTile(const Vector2i& position) const;
    Tile* getTile(const Vector2u& position) const;
    void setTile(const Vector2i& position, Tile* tile);
    void setTile(const Vector2u& position, Tile* tile);
    void updateCosmetics();
    void updateTiles();
    void replaceTile(Tile* tile);
    void clear();
    void resize(const Vector2u& size);
    void cloneArea(const Board& source, const IntRect& region, const Vector2i& destination, bool keepOverwrittenTiles = false);
    void highlightArea(const IntRect& region, bool highlight);
    void rotate(bool counterClockwise = false);
    void flip(bool acrossHorizontal = false);
    void newBoard(const Vector2u& size = Vector2u(20, 20), const string& filename = "boards/NewBoard.txt", bool startEmpty = false);
    void loadFile(const string& filename);
    
    private:
    static const vector<string> WIRE_SYMBOL_TABLE;
    static const vector<string> INPUT_SYMBOL_TABLE;
    static const vector<string> OUTPUT_SYMBOL_TABLE;
    static const vector<string> GATE_SYMBOL_TABLE;
    static Texture* _tilesetGridPtr;
    static Texture* _tilesetNoGridPtr;
    static int _textureIDMax;
    static Vector2u _tileSize;
    VertexArray _vertices;
    Vector2u _size;
    Tile*** _tileArray;
    
    static void _clampToSize(Image& image, const Vector2u& topLeft, const Vector2u& bottomRight);
    static void _buildTexture(const Image& source, Texture* target, const Vector2u& tileSize);
    void _redrawTile(Tile* tile);
    void _setVertexCoords();
    int _findSymbol(char c1, char c2, const vector<string>& symbolTable) const;
    virtual void draw (RenderTarget& target, RenderStates states) const;
};

#endif