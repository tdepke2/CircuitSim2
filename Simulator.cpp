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
View Simulator::boardView, Simulator::windowView;
float Simulator::zoomLevel;
RenderWindow* Simulator::windowPtr = nullptr;
Board* Simulator::boardPtr = nullptr;
Board* Simulator::bufferBoardPtr = nullptr;

int Simulator::start() {
    cout << "Initializing setup." << endl;
    RenderWindow window;
    windowPtr = &window;
    try {
        assert(state == State::Uninitialized);
        state = State::Running;
        mainRNG.seed(static_cast<unsigned long>(chrono::high_resolution_clock::now().time_since_epoch().count()));
        window.create(VideoMode(800, 800), "[CircuitSim2] Loading...", Style::Default, ContextSettings(0, 0, 4));
        windowHandle = window.getSystemHandle();
        
        zoomReset();
        Board::loadTextures("resources/texturePackGrid.png", "resources/texturePackNoGrid.png", Vector2u(32, 32));
        Board board, bufferBoard;
        boardPtr = &board;
        bufferBoardPtr = &bufferBoard;
        board.newBoard();
        bufferBoard.newBoard();
        bufferBoard.setPosition(100, 30);
        UserInterface userInterface;
        Vector2i mouseStart(0, 0), tileCursor(-1, -1);
        Clock mainClock, fpsClock;    // The mainClock keeps track of elapsed frame time, fpsClock is used to count frames per second.
        int fpsCounter = 0;
        
        cout << "Loading completed." << endl;
        while (state != State::Exiting) {
            window.clear ();
            window.setView(boardView);
            window.draw(board);
            window.draw(bufferBoard);
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
            
            Vector2i newTileCursor(window.mapPixelToCoords(mouseStart, boardView));
            newTileCursor.x /= board.getTileSize().x;
            newTileCursor.y /= board.getTileSize().y;
            if (newTileCursor.x >= 0 && newTileCursor.x < static_cast<int>(board.getSize().x) && newTileCursor.y >= 0 && newTileCursor.y < static_cast<int>(board.getSize().y)) {
                board.redrawTile(Vector2u(newTileCursor), true);
                if (tileCursor != newTileCursor) {
                    if (tileCursor != Vector2i(-1, -1)) {
                        board.redrawTile(Vector2u(tileCursor), false);
                    }
                    tileCursor = newTileCursor;
                }
            } else if (tileCursor != Vector2i(-1, -1)) {
                board.redrawTile(Vector2u(tileCursor), false);
                tileCursor = Vector2i(-1, -1);
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

void Simulator::newBoard() {
    boardPtr->newBoard();
    zoomReset();
}

void Simulator::loadBoard() {
    OPENFILENAME fileDialog;    // https://docs.microsoft.com/en-us/windows/desktop/dlgbox/using-common-dialog-boxes
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
    fileDialog.lpstrInitialDir = "boards";
    fileDialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&fileDialog) == TRUE) {
        boardPtr->loadFile(string(fileDialog.lpstrFile));
        zoomReset();
    } else {
        cout << "No file selected." << endl;
    }
}

void Simulator::saveBoard() {
    
}

void Simulator::saveAsBoard() {
    
}

void Simulator::renameBoard() {
    
}

void Simulator::exitProgram() {
    
}

void Simulator::zoomReset() {
    zoomLevel = 1.0f;
    boardView.setSize(Vector2f(windowPtr->getSize().x * zoomLevel, windowPtr->getSize().y * zoomLevel));
    windowView.reset(FloatRect(Vector2f(0.0f, 0.0f), Vector2f(windowPtr->getSize())));
    boardView.setCenter(0.5f * Vector2f(windowPtr->getSize()) - Vector2f(32.0f, 60.0f));
}