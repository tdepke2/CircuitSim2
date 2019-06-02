#ifndef _SIMULATOR_H
#define _SIMULATOR_H

class Board;
class UserInterface;

#include "Tile.h"
#include <atomic>
#include <mutex>
#include <random>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class Simulator {
    public:
    static const unsigned int FRAMERATE_LIMIT;
    
    static int start();
    static int randomInteger(int min, int max);
    static int randomInteger(int n);
    static void fileOption(int option);
    static void viewOption(int option);
    static void runOption(int option);
    static void toolsOption(int option);
    static void placeTile(int option);
    static void relabelTarget(int option = 0);
    
    private:
    enum class State {
        Uninitialized, Running, Exiting
    };
    enum class SimSpeed {
        Paused, Slow, Medium, Fast, Extreme
    };
    
    static atomic<State> state;
    static SimSpeed simSpeed;
    static mt19937 mainRNG;
    static mutex renderMutex, renderReadyMutex;
    static int fpsCounter, upsCounter;
    static View boardView, windowView;
    static float zoomLevel;
    static RenderWindow* windowPtr;
    static Board* boardPtr;
    static Board* currentTileBoardPtr;
    static Board* copyBufferBoardPtr;
    static Board* wireVerticalBoardPtr;
    static Board* wireHorizontalBoardPtr;
    static UserInterface* userInterfacePtr;
    static Direction currentTileDirection;
    static bool editMode, copyBufferVisible, wireToolVerticalFirst;
    static Vector2i tileCursor, selectionStart, wireToolStart;
    static Vector2u wireVerticalPosition, wireHorizontalPosition;
    static IntRect selectionArea;
    static Tile* relabelTargetTile;
    static Text* wireToolLabelPtr;
    
    static void terminationHandler(int sigNum);
    static void renderLoop();
    static void handleKeyPress(Event::KeyEvent keyEvent);
    static void pasteToBoard(const Vector2i& tileCursor, bool forcePaste);
    static void updateWireTool(const Vector2i& tileCursor, const Vector2i& newTileCursor);
};

#endif