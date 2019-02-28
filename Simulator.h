#ifndef _SIMULATOR_H
#define _SIMULATOR_H

#include <random>

using namespace std;

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