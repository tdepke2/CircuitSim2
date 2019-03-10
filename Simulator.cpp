#include "Board.h"
#include "Simulator.h"
#include "Tile.h"
#include "TileButton.h"
#include "TileGate.h"
#include "TileLED.h"
#include "TileSwitch.h"
#include "TileWire.h"
#include "UserInterface.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <windows.h>

const float Simulator::FPS_CAP = 60.0f;
Simulator::State Simulator::state = State::Uninitialized;
mt19937 Simulator::mainRNG;
WindowHandle Simulator::windowHandle;

int Simulator::start() {
    cout << "Initializing setup." << endl;
    RenderWindow window;
    try {
        assert(state == State::Uninitialized);
        state = State::Running;
        mainRNG.seed(static_cast<unsigned long>(chrono::high_resolution_clock::now().time_since_epoch().count()));
        window.create(VideoMode(800, 800), "[CircuitSim2] Loading...", Style::Default, ContextSettings(0, 0, 4));
        windowHandle = window.getSystemHandle();
        
        View boardView(FloatRect(Vector2f(0.0f, 0.0f), Vector2f(window.getSize()))), windowView(FloatRect(Vector2f(0.0f, 0.0f), Vector2f(window.getSize())));
        float zoomLevel = 1.0f;
        Vector2i mouseStart(0, 0);
        Clock mainClock, fpsClock;    // The mainClock keeps track of elapsed frame time, fpsClock is used to count frames per second.
        int fpsCounter = 0;
        
        Board board;
        board.loadTextures("resources/texturePackGrid.png", "resources/texturePackNoGrid.png", Vector2u(32, 32));
        board.newBoard();
        
        //board.loadFile("boards/Computer.txt");
        UserInterface userInterface;
        
        cout << "Loading completed." << endl;
        while (state != State::Exiting) {
            window.clear ();
            window.setView(boardView);
            window.draw(board);
            window.setView(windowView);
            window.draw(userInterface);
            window.display();
            
            while (mainClock.getElapsedTime().asSeconds() < 1.0f / FPS_CAP) {}    // Slow down simulation if the current FPS is greater than the FPS cap.
            float deltaTime = mainClock.restart().asSeconds();    // Change in time since the last frame.
            if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {    // Calculate FPS.
                window.setTitle("[CircuitSim2] [" + board.name + "] [Size: " + to_string(board.getSize().x) + " x " + to_string(board.getSize().y) + "] [FPS: " + to_string(fpsCounter) + "]");
                fpsClock.restart();
                fpsCounter = 0;
            } else {
                ++fpsCounter;
            }
            
            Event event;
            while (window.pollEvent(event)) {    // Process events.
                if (event.type == Event::MouseMoved) {
                    if (Mouse::isButtonPressed(Mouse::Left)) {
                        Vector2f newCenter(boardView.getCenter().x + (mouseStart.x - event.mouseMove.x) * zoomLevel, boardView.getCenter().y + (mouseStart.y - event.mouseMove.y) * zoomLevel);
                        if (newCenter.x < 0.0f) {
                            newCenter.x = 0.0f;
                        } else if (newCenter.x > static_cast<float>(board.getSize().x * board.getTileSize().x)) {
                            newCenter.x = static_cast<float>(board.getSize().x * board.getTileSize().x);
                        }
                        if (newCenter.y < 0.0f) {
                            newCenter.y = 0.0f;
                        } else if (newCenter.y > static_cast<float>(board.getSize().y * board.getTileSize().y)) {
                            newCenter.y = static_cast<float>(board.getSize().y * board.getTileSize().y);
                        }
                        boardView.setCenter(newCenter);
                    } else {
                        userInterface.update(event.mouseMove.x, event.mouseMove.y, false);
                    }
                    mouseStart.x = event.mouseMove.x;
                    mouseStart.y = event.mouseMove.y;
                } else if (event.type == Event::MouseButtonPressed) {
                    if (event.mouseButton.button == Mouse::Left) {
                        userInterface.update(event.mouseButton.x, event.mouseButton.y, true);
                    }
                } else if (event.type == Event::MouseWheelScrolled) {
                    float zoomDelta = event.mouseWheelScroll.delta * zoomLevel * -0.04f;
                    if (zoomLevel + zoomDelta > 0.2f && zoomLevel + zoomDelta < 20.0f) {
                        zoomLevel += zoomDelta;
                        boardView.setSize(Vector2f(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel));
                    }
                } else if (event.type == Event::KeyPressed) {
                    if (event.key.code == Keyboard::G) {
                        board.gridActive = !board.gridActive;
                    }
                } else if (event.type == Event::Resized) {
                    boardView.setSize(Vector2f(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel));
                    windowView.reset(FloatRect(Vector2f(0.0f, 0.0f), Vector2f(window.getSize())));
                } else if (event.type == Event::Closed) {
                    window.close();
                    state = State::Exiting;
                }
            }
        }
    } catch (exception& ex) {
        window.close();
        cout << "\nUnhandled exception thrown: " << ex.what() << endl;
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

void Simulator::doThing() {    // https://docs.microsoft.com/en-us/windows/desktop/dlgbox/using-common-dialog-boxes
    cout << "Doing the thing." << endl;
    OPENFILENAME fileDialog;
    char filename[260];
    
    ZeroMemory(&fileDialog, sizeof(fileDialog));    // Initialize fileDialog.
    fileDialog.lStructSize = sizeof(fileDialog);
    fileDialog.hwndOwner = windowHandle;
    fileDialog.lpstrFile = filename;
    fileDialog.lpstrFile[0] = '\0';    // Set to null string so that GetOpenFileName does not initialize itself with the filename.
    fileDialog.nMaxFile = sizeof(filename);
    fileDialog.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
    fileDialog.nFilterIndex = 1;
    fileDialog.lpstrFileTitle = NULL;
    fileDialog.nMaxFileTitle = 0;
    fileDialog.lpstrInitialDir = NULL;
    fileDialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&fileDialog) == TRUE) {
        cout << "The user chose file: \"" << fileDialog.lpstrFile << "\"." << endl;
    } else {
        cout << "Something bad happened?" << endl;
    }
}