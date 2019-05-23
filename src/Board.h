#ifndef _BOARD_H
#define _BOARD_H

class TileButton;
class TileGate;
class TileLED;
class TileSwitch;
class TileWire;

#include "Tile.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;
using namespace sf;

const vector<string> WIRE_SYMBOL_TABLE = {
    "| ",  "[ ",  "--",  "==",
    "\'-", "\"=", ",-",  ";=",  ", ",  "; ",  "\' ", "\" ",
    ">-",  ">=",  "v-",  "v=",  "< ",  "<.",  "^-",  "^=",
    "+-",  "#=",
    "|-",  "[-",  "|=",  "[="
};
const vector<string> INPUT_SYMBOL_TABLE = {
    "s",   "S",
    "t",   "T"
};
const vector<string> OUTPUT_SYMBOL_TABLE = {
    "..",  "##"
};
const vector<string> GATE_SYMBOL_TABLE = {
    "^d",  "^D",  ">d",  ">D",  "vd",  "vD",  "<d",  "<D",
    "^m",  "^M",  ">m",  ">M",  "vm",  "vM",  "<m",  "<M",
    "^n",  "^N",  ">n",  ">N",  "vn",  "vN",  "<n",  "<N",
    "^a",  "^A",  ">a",  ">A",  "va",  "vA",  "<a",  "<A",
    "^b",  "^B",  ">b",  ">B",  "vb",  "vB",  "<b",  "<B",
    "^o",  "^O",  ">o",  ">O",  "vo",  "vO",  "<o",  "<O",
    "^p",  "^P",  ">p",  ">P",  "vp",  "vP",  "<p",  "<P",
    "^x",  "^X",  ">x",  ">X",  "vx",  "vX",  "<x",  "<X",
    "^y",  "^Y",  ">y",  ">Y",  "vy",  "vY",  "<y",  "<Y"
};

class Board : public Drawable, public Transformable {    // Class for a circuit board with logic components that can be drawn to the window.
    public:
    static string newBoardDefaultPath;
    static bool gridActive;
    static vector<TileLED*> endpointLEDs;
    static vector<TileGate*> endpointGates;
    string name;
    unordered_set<Tile*> cosmeticUpdates;
    unordered_set<TileWire*> wireUpdates;
    unordered_set<TileSwitch*> switchUpdates;
    unordered_set<TileButton*> buttonUpdates;
    unordered_set<TileLED*> LEDUpdates;
    unordered_set<TileGate*> gateUpdates;
    unordered_map<char, vector<TileSwitch*>> switchKeybinds;
    unordered_map<char, vector<TileButton*>> buttonKeybinds;
    unordered_map<Tile*, Text> tileLabels;
    
    static const Font& getFont();
    static const Vector2u& getTileSize();
    static void loadTextures(const string& filenameGrid, const string& filenameNoGrid, const Vector2u& tileSize);
    static void loadFont(const string& filename);
    Board();
    virtual ~Board();
    const Vector2u& getSize() const;
    Tile* getTile(const Vector2i& position) const;
    Tile* getTile(const Vector2u& position) const;
    void setTile(const Vector2i& position, Tile* tile);
    void setTile(const Vector2u& position, Tile* tile);
    void redrawTileVertices(int textureID, const Vector2u& position, Direction direction, bool highlight);
    void updateCosmetics();
    void updateTiles();
    void replaceTile(Tile* tile);
    void clear();
    void resize(const Vector2u& size);
    void cloneArea(const Board& source, const IntRect& region, const Vector2i& destination, bool noAdjacentUpdates = false, bool keepOverwrittenTiles = false);
    void highlightArea(const IntRect& region, bool highlight);
    void rotate(bool counterClockwise = false);
    void flip(bool acrossHorizontal = false);
    void newBoard(const Vector2u& size = Vector2u(20, 20), const string& filename = newBoardDefaultPath, bool startEmpty = false);
    void loadFile(const string& filename);
    void saveFile(const string& filename);
    
    private:
    static Texture* _tilesetGridPtr;
    static Texture* _tilesetNoGridPtr;
    static Font* _fontPtr;
    static int _textureIDMax;
    static Vector2u _tileSize;
    VertexArray _vertices;
    Vector2u _size;
    Tile*** _tileArray;
    
    static void _clampToSize(Image& image, const Vector2u& topLeft, const Vector2u& bottomRight);
    static void _buildTexture(const Image& source, Texture* target, const Vector2u& tileSize);
    void _setVertexCoords();
    int _findSymbol(char c1, char c2, const vector<string>& symbolTable) const;
    virtual void draw (RenderTarget& target, RenderStates states) const;
};

#endif