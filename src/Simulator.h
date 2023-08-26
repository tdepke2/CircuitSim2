#ifndef _SIMULATOR_H
#define _SIMULATOR_H

class Board;
class UserInterface;

#include "Tile.h"
#include <atomic>
#include <mutex>
#include <random>
#include <SFML/Graphics.hpp>
#include <string>

using namespace std;
using namespace sf;

class Simulator {    // Singleton class that controls the simulation logic. Includes the main thread and render thread.
    public:
    struct Configuration {
        float slowTPSLimit = 2.0f;
        float mediumTPSLimit = 30.0f;
        float fastTPSLimit = 60.0f;
        bool triStateLogicDefault = true;
        bool pauseOnConflict = true;
    };
    
    static const unsigned int FRAMERATE_LIMIT;
    static const Vector2u INITIAL_WINDOW_SIZE;
    
    static const Configuration& getConfig();
    static const Vector2u getWindowSize();
    static const UserInterface* getUserInterface();
    static int start();    // Main thread, handles window event processing and simulation updates.
    static int randomInteger(int min, int max);    // Generates a random integer between min and max inclusive.
    static int randomInteger(int n);    // Generates a random integer between 0 and n - 1.
    static string decimalToString(float f);    // Formats a float to a string with minimal trailing zeros.
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
    
    static Configuration config;
    static atomic<State> state;
    static SimSpeed simSpeed;
    static mt19937 mainRNG;
    static mutex renderMutex, renderReadyMutex;    // Ensures render call and simulation tick are mutually exclusive.
    static int fpsCounter, tpsCounter;
    static View boardView, windowView;    // The boardView keeps the current view transform of the board, windowView is the default transform of the application.
    static float zoomLevel;
    static RenderWindow* windowPtr;    // Main render window provided by SFML.
    static Board* boardPtr;    // The primary circuit board.
    static Board* currentTileBoardPtr;    // Stores currently selected tile to place on the board.
    static Board* copyBufferBoardPtr;    // Stores last copied region.
    static Board* wireVerticalBoardPtr;    // Both wireVerticalBoardPtr and wireHorizontalBoardPtr are used to display the wire tool.
    static Board* wireHorizontalBoardPtr;
    static UserInterface* userInterfacePtr;    // Displays the dropdown menu, dialog prompts, and event messages.
    static Text* wireToolLabelPtr;    // Displays the distance measurement for the wire tool.
    static string directoryPath;    // Base path to the application directory.
    static Direction currentTileDirection;
    static bool editMode, copyBufferVisible, wireToolVerticalFirst;
    static Vector2i mouseStart, tileCursor, selectionStart, wireToolStart;
    static Vector2u wireVerticalPosition, wireHorizontalPosition;
    static IntRect selectionArea;
    static Tile* relabelTargetTile;
    
    static void processEvents();    // Poll all events from the window and update simulation.
    static void openConfig(const string& filename, bool saveData);    // Opens a configuration file for loading/saving data.
    static void updateSimSpeed(SimSpeed newSpeed);    // Changes the simulation speed and updates this in the user interface.
    static void terminationHandler(int sigNum);    // Sets simulation to exit when a termination signal is received from main thread.
    static void renderLoop();    // The render thread.
    static void nextTick();    // Runs the main board simulation for one tick.
    static void handleKeyPress(Event::KeyEvent keyEvent);    // Controls keybinds and their actions.
    static void pasteToBoard(const Vector2i& tileCursor, bool forcePaste);    // Pastes the current tile or copy buffer at the selected position.
    static void updateWireTool(const Vector2i& tileCursor, const Vector2i& newTileCursor);    // Updates the wire tool by recalculating wire positions.
};

#endif