#ifndef _SIMULATOR_H
#define _SIMULATOR_H

class Board;

#include "Tile.h"
#include <random>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class Simulator {
    public:
    static const float FPS_CAP;
    
    static int start();
    static int randomInteger(int min, int max);
    static int randomInteger(int n);
    static void fileOption(int option);
    static void viewOption(int option);
    static void runOption(int option);
    static void toolsOption(int option);
    static void placeTile(int option);
    
    private:
    enum class State {
        Uninitialized, Running, Exiting
    };
    
    static State state;
    static mt19937 mainRNG;
    static View boardView, windowView;
    static float zoomLevel;
    static RenderWindow* windowPtr;
    static Board* boardPtr;
    static Board* currentTileBoardPtr;
    static Board* copyBufferBoardPtr;
    static Direction currentTileDirection;
    static bool copyBufferVisible;
    
    static void pasteToBoard(const Vector2i& tileCursor);
};

#endif