#ifndef _SIMULATOR_H
#define _SIMULATOR_H

class Board;

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
    static void newBoard();
    static void loadBoard();
    static void saveBoard();
    static void saveAsBoard();
    static void renameBoard();
    static void exitProgram();
    static void zoomReset();
    
    private:
    enum class State {
        Uninitialized, Running, Exiting
    };
    
    static State state;
    static mt19937 mainRNG;
    static WindowHandle windowHandle;
    static View boardView, windowView;
    static float zoomLevel;
    static RenderWindow* windowPtr;
    static Board* boardPtr;
    static Board* bufferBoardPtr;
};

#endif