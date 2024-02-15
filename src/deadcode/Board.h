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
    "| ",  "[ ",  "{ ",  "--",  "==",  "~~",
    "\'-", "\"=", "`~",  ",-",  ";=",  ".~",  ", ",  "; ",  ". ",  "\' ", "\" ", "` ",
    ">-",  ">=",  ">~",  "v-",  "v=",  "v~",  "< ",  "<.",  "<:",  "^-",  "^=",  "^~",
    "+-",  "#=",  "+~",
    "|-",  "[-",  "{-",  "|=",  "[=",  "{=",  "|~",  "[~",  "{~"
};
const vector<string> INPUT_SYMBOL_TABLE = {
    "s",   "S",
    "t",   "T"
};
const vector<string> OUTPUT_SYMBOL_TABLE = {
    "..",  "##"
};
const vector<string> GATE_SYMBOL_TABLE = {
    "^d",  "^D",  "`d",  ">d",  ">D",  "}d",  "vd",  "vD",  ",d",  "<d",  "<D",  "{d",
    "^m",  "^M",  "`m",  ">m",  ">M",  "}m",  "vm",  "vM",  ",m",  "<m",  "<M",  "{m",
    "^n",  "^N",  "`n",  ">n",  ">N",  "}n",  "vn",  "vN",  ",n",  "<n",  "<N",  "{n",
    "^a",  "^A",  "`a",  ">a",  ">A",  "}a",  "va",  "vA",  ",a",  "<a",  "<A",  "{a",
    "^b",  "^B",  "`b",  ">b",  ">B",  "}b",  "vb",  "vB",  ",b",  "<b",  "<B",  "{b",
    "^o",  "^O",  "`o",  ">o",  ">O",  "}o",  "vo",  "vO",  ",o",  "<o",  "<O",  "{o",
    "^p",  "^P",  "`p",  ">p",  ">P",  "}p",  "vp",  "vP",  ",p",  "<p",  "<P",  "{p",
    "^x",  "^X",  "`x",  ">x",  ">X",  "}x",  "vx",  "vX",  ",x",  "<x",  "<X",  "{x",
    "^y",  "^Y",  "`y",  ">y",  ">Y",  "}y",  "vy",  "vY",  ",y",  "<y",  "<Y",  "{y"
};

class Board : public Drawable, public Transformable {    // Class for a circuit board with logic components that can be drawn to the window.
    public:
    static string newBoardDefaultPath;
    static bool gridActive;
    static bool enableExtraLogicStates;
    static int numStateErrors;
    static vector<TileLED*> endpointLEDs;
    static vector<TileGate*> endpointGates;
    string name;
    bool changesMade;
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
    static void loadTextures(const string& filenameGrid, const string& filenameNoGrid, const Vector2u& tileSize);    // Loads the texture files, then clamps and stitches them to fix mipmap blending.
    static void loadFont(const string& filename);    // Loads the font file.
    Board();
    virtual ~Board();
    const Vector2u& getSize() const;
    Tile* getTile(const Vector2i& position) const;
    Tile* getTile(const Vector2u& position) const;
    void setTile(const Vector2i& position, Tile* tile);
    void setTile(const Vector2u& position, Tile* tile);
    void redrawTileVertices(int textureID, const Vector2u& position, Direction direction, bool highlight);    // Redraws the vertices of a single tile to the VertexArray.
    void updateCosmetics();    // Runs all cosmetic updates that are scheduled.
    void updateTiles();    // Updates all tiles and advances the simulation to the next tick if any updates are available.
    void replaceTile(Tile* tile);    // Replaces a tile on the board with a different one, deletes the replaced tile.
    void clear();    // Sets the board size to zero and deletes all tiles.
    void resize(const Vector2u& size);    // Changes the board size (must not be zero), objects are preserved in their original locations where applicable.
    void cloneArea(const Board& source, const IntRect& region, const Vector2i& destination, bool noAdjacentUpdates = false, bool keepOverwrittenTiles = false);    // Clones an area from another board into this one.
    void highlightArea(const IntRect& region, bool highlight);    // Sets the highlight on an area of the board.
    void rotate(bool counterClockwise = false);    // Rotates the board and all objects in CW/CCW direction.
    void flip(bool acrossHorizontal = false);    // Flips the board across the vertical/horizontal axis.
    void newBoard(const Vector2u& size = Vector2u(20, 20), const string& filename = newBoardDefaultPath, bool startEmpty = false);    // Resets the board to a new one, all tiles are deleted.
    void loadFile(const string& filename);    // Loads data for a board from a file.
    void saveFile(const string& filename);    // Saves current board data to a file.
    
    private:
    static Texture* _tilesetGridPtr;
    static Texture* _tilesetNoGridPtr;
    static Font* _fontPtr;
    static int _textureIDMax;
    static Vector2u _tileSize;
    VertexArray _vertices;
    Vector2u _size;
    Tile*** _tileArray;
    RectangleShape _notesBox;
    Text _notesText;
    
    static void _clampToSize(Image& image, const Vector2u& topLeft, const Vector2u& bottomRight);    // Clamps an image to a specified border, does not resize the image object.
    static void _buildTexture(const Image& source, Texture* target, const Vector2u& tileSize);    // Builds a large texture with clamped tiles from a condensed texture image.
    void _setVertexCoords();    // Recalculates the coordinates of all vertices.
    int _findSymbol(char c1, char c2, const vector<string>& symbolTable) const;    // Finds the index of the given pattern in a symbol table. If no pattern found, -1 is returned.
    virtual void draw (RenderTarget& target, RenderStates states) const;    // A function of Drawable objects to draw the object to a window.
};

#endif