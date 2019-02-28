#include "Simulator.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <stdexcept>
#include <string>

using namespace std;
using namespace sf;

const float Simulator::FPS_CAP = 60.0f;
Simulator::State Simulator::state = State::uninitialized;
mt19937 Simulator::mainRNG;

int Simulator::start() {
    RenderWindow window;
    try {
        assert(state == State::uninitialized);
        state = State::running;
        mainRNG.seed(static_cast<unsigned long>(chrono::high_resolution_clock::now().time_since_epoch().count()));
        window.create(VideoMode(500, 500), "CircuitSim2");
        
        View view(FloatRect(Vector2f(0.0f, 0.0f), Vector2f(window.getSize())));
        Clock mainClock, fpsClock;    // The mainClock keeps track of elapsed frame time, fpsClock is used to count frames per second.
        int fpsCounter = 0;
        
        RectangleShape testRect(Vector2f(50, 30));
        
        while (state != State::exiting) {
            window.clear ();
            window.setView(view);
            window.draw(testRect);
            window.display();
            
            while (mainClock.getElapsedTime().asSeconds() < 1.0f / FPS_CAP) {}    // Slow down simulation if the current FPS is greater than the FPS cap.
            float deltaTime = mainClock.restart().asSeconds();    // Change in time since the last frame.
            if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {    // Calculate FPS.
                window.setTitle("CircuitSim2 (FPS: " + to_string(fpsCounter));
                fpsClock.restart();
                fpsCounter = 0;
            } else {
                ++fpsCounter;
            }
            
            Event event;
            while (window.pollEvent(event)) {    // Process events.
                if (event.type == Event::Resized) {
                    view.reset(FloatRect(Vector2f(0.0f, 0.0f), Vector2f(window.getSize())));
                } else if (event.type == Event::Closed) {
                    window.close();
                    state = State::exiting;
                }
            }
        }
    } catch (exception& ex) {
        cout << "\nException thrown: " << ex.what() << endl;
        cout << "(Press enter)" << endl;
        cin.get();
        return -1;
    }
    
    return 0;
}

int Simulator::randomInteger(int min, int max) {
    uniform_int_distribution<int> minMaxRange(min, max);
    return minMaxRange(mainRNG);
}

int Simulator::randomInteger(int n) {
    return randomInteger(0, n - 1);
}