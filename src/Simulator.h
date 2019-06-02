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

class Simulator {    // Singleton class that controls the simulation logic. Includes the main thread and render thread.
    public:
    static const unsigned int FRAMERATE_LIMIT;
    
    static int start();    // Main thread, handles window event processing and simulation updates.
    static int randomInteger(int min, int max);    // Generates a random integer between min and max inclusive.
    static int randomInteger(int n);    // Generates a random integer between 0 and n - 1.
    static void fileOption(int option);    // Calls an option in the file menu.
    static void viewOption(int option);    // Calls an option in the view menu.
    static void runOption(int option);    // Calls an option in the run menu.
    static void toolsOption(int option);    // Calls an option in the tools menu.
    static void placeTile(int option);    // Copies a tile into the currentTileBoard. The option corresponds to the tile type.
    static void relabelTarget(int option = 0);    // Sets the label on the switch/button pointed to by relabelTargetTile, used by the GUI to set a label.
    
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
    static Text* wireToolLabelPtr;
    static Direction currentTileDirection;
    static bool editMode, copyBufferVisible, wireToolVerticalFirst;
    static Vector2i tileCursor, selectionStart, wireToolStart;
    static Vector2u wireVerticalPosition, wireHorizontalPosition;
    static IntRect selectionArea;
    static Tile* relabelTargetTile;
    
    static void terminationHandler(int sigNum);    // Sets simulation to exit when a termination signal is received from main thread.
    static void renderLoop();    // The render thread.
    static void handleKeyPress(Event::KeyEvent keyEvent);    // Controls keybinds and their actions.
    static void pasteToBoard(const Vector2i& tileCursor, bool forcePaste);    // Pastes the current tile or copy buffer at the selected position.
    static void updateWireTool(const Vector2i& tileCursor, const Vector2i& newTileCursor);    // Updates the wire tool by recalculating wire positions.
};

#endif