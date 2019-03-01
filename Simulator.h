#ifndef _SIMULATOR_H
#define _SIMULATOR_H

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
    
    private:
    enum class State {
        uninitialized, running, exiting
    };
    
    static State state;
    static mt19937 mainRNG;
};

#endif