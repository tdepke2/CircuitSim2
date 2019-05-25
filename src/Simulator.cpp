#include "Board.h"
#include "Simulator.h"
#include "TileButton.h"
#include "TileGate.h"
#include "TileLED.h"
#include "TileSwitch.h"
#include "TileWire.h"
#include "UserInterface.h"
#include <cassert>
#include <cctype>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <typeinfo>
#include <windows.h>

const float Simulator::FPS_CAP = 60.0f;
Simulator::State Simulator::state = State::Uninitialized;
Simulator::SimSpeed Simulator::simSpeed = SimSpeed::Paused;
mt19937 Simulator::mainRNG;
View Simulator::boardView, Simulator::windowView;
float Simulator::zoomLevel;
RenderWindow* Simulator::windowPtr = nullptr;
Board* Simulator::boardPtr = nullptr;
Board* Simulator::currentTileBoardPtr = nullptr;
Board* Simulator::copyBufferBoardPtr = nullptr;
UserInterface* Simulator::userInterfacePtr = nullptr;
Direction Simulator::currentTileDirection = NORTH;
bool Simulator::editMode = true, Simulator::copyBufferVisible = false;
Vector2i Simulator::tileCursor(-1, -1), Simulator::selectionStart(-1, -1);
IntRect Simulator::selectionArea(0, 0, 0, 0);

int Simulator::start() {
    cout << "Initializing setup..." << endl;
    thread renderThread;
    try {
        assert(state == State::Uninitialized);
        state = State::Running;
        mainRNG.seed(static_cast<unsigned long>(chrono::high_resolution_clock::now().time_since_epoch().count()));
        windowPtr = new RenderWindow(VideoMode(900, 900), "[CircuitSim2] Loading...", Style::Default, ContextSettings(0, 0, 4));
        windowPtr->setActive(false);
        
        Board::loadTextures("resources/texturePackGrid.png", "resources/texturePackNoGrid.png", Vector2u(32, 32));
        Board::loadFont("resources/consolas.ttf");
        boardPtr = new Board();
        currentTileBoardPtr = new Board();
        copyBufferBoardPtr = new Board();
        char directoryPath[260];
        GetCurrentDirectory(sizeof(directoryPath), directoryPath);
        Board::newBoardDefaultPath = string(directoryPath) + "\\boards\\NewBoard.txt";
        boardPtr->newBoard();
        copyBufferBoardPtr->newBoard(Vector2u(1, 1), "");
        copyBufferBoardPtr->highlightArea(IntRect(0, 0, copyBufferBoardPtr->getSize().x, copyBufferBoardPtr->getSize().y), true);
        userInterfacePtr = new UserInterface();
        viewOption(3);
        Vector2i mouseStart(0, 0);
        renderThread = thread(renderLoop);
        
        cout << "Loading completed." << endl;
        while (state != State::Exiting) {
            boardPtr->updateCosmetics();    // ######################################################################################################## May want to move this to end of loop (but make sure updates called before first draw).
            copyBufferBoardPtr->updateCosmetics();
            currentTileBoardPtr->updateCosmetics();
            
            
            
            Event event;
            while (windowPtr->pollEvent(event)) {    // Process events.
                if (event.type == Event::MouseMoved) {
                    if (Mouse::isButtonPressed(Mouse::Left)) {
                        Vector2f newCenter(boardView.getCenter().x + (mouseStart.x - event.mouseMove.x) * zoomLevel, boardView.getCenter().y + (mouseStart.y - event.mouseMove.y) * zoomLevel);
                        if (newCenter.x < 0.0f) {
                            newCenter.x = 0.0f;
                        } else if (newCenter.x > static_cast<float>(boardPtr->getSize().x * boardPtr->getTileSize().x)) {
                            newCenter.x = static_cast<float>(boardPtr->getSize().x * boardPtr->getTileSize().x);
                        }
                        if (newCenter.y < 0.0f) {
                            newCenter.y = 0.0f;
                        } else if (newCenter.y > static_cast<float>(boardPtr->getSize().y * boardPtr->getTileSize().y)) {
                            newCenter.y = static_cast<float>(boardPtr->getSize().y * boardPtr->getTileSize().y);
                        }
                        boardView.setCenter(newCenter);
                    } else {
                        userInterfacePtr->update(event.mouseMove.x, event.mouseMove.y, false);
                    }
                    mouseStart.x = event.mouseMove.x;
                    mouseStart.y = event.mouseMove.y;
                } else if (event.type == Event::MouseButtonPressed) {
                    if (event.mouseButton.button == Mouse::Left) {    // Check if view is moved.
                        userInterfacePtr->update(event.mouseButton.x, event.mouseButton.y, true);
                    } else if (event.mouseButton.button == Mouse::Right && (currentTileBoardPtr->getSize() != Vector2u(0, 0) || copyBufferVisible)) {    // Check if tile/buffer will be placed.
                        pasteToBoard(tileCursor, Keyboard::isKeyPressed(Keyboard::LShift) || Keyboard::isKeyPressed(Keyboard::RShift));
                    }
                } else if (event.type == Event::MouseButtonReleased) {
                    if (event.mouseButton.button == Mouse::Right && currentTileBoardPtr->getSize() == Vector2u(0, 0) && !copyBufferVisible) {
                        if (selectionStart == Vector2i(-1, -1)) {    // Check if selection was cancelled (right click made without dragging).
                            if (selectionArea != IntRect(0, 0, 0, 0)) {
                                boardPtr->highlightArea(selectionArea, false);
                                selectionArea = IntRect(0, 0, 0, 0);
                                if (tileCursor != Vector2i(-1, -1)) {
                                    boardPtr->getTile(tileCursor)->setHighlight(true);
                                }
                            }
                        } else {    // Else, finish the selection.
                            selectionStart = Vector2i(-1, -1);
                        }
                    }
                } else if (event.type == Event::MouseWheelScrolled) {
                    float zoomDelta = event.mouseWheelScroll.delta * zoomLevel * -0.04f;
                    if (zoomLevel + zoomDelta > 0.2f && zoomLevel + zoomDelta < 20.0f) {
                        zoomLevel += zoomDelta;
                        boardView.setSize(Vector2f(windowPtr->getSize().x * zoomLevel, windowPtr->getSize().y * zoomLevel));
                    }
                } else if (event.type == Event::KeyPressed) {
                    handleKeyPress(event.key);
                } else if (event.type == Event::TextEntered) {
                    if (!editMode && event.text.unicode >= 33 && event.text.unicode <= 126) {    // Check if key is a printable character (excluding a space).
                        auto mapIter = boardPtr->switchKeybinds.find(static_cast<char>(event.text.unicode));
                        if (mapIter != boardPtr->switchKeybinds.end() && !mapIter->second.empty()) {
                            for (TileSwitch* switchPtr : mapIter->second) {
                                if (switchPtr->getState() == LOW) {
                                    switchPtr->setState(HIGH);
                                } else {
                                    switchPtr->setState(LOW);
                                }
                            }
                        }
                        auto mapIter2 = boardPtr->buttonKeybinds.find(static_cast<char>(event.text.unicode));
                        if (mapIter2 != boardPtr->buttonKeybinds.end() && !mapIter2->second.empty()) {
                            for (TileButton* buttonPtr : mapIter2->second) {
                                if (buttonPtr->getState() == LOW) {
                                    buttonPtr->setState(HIGH);
                                } else {
                                    buttonPtr->setState(LOW);
                                }
                            }
                        }
                    }
                } else if (event.type == Event::Resized) {
                    boardView.setSize(Vector2f(windowPtr->getSize().x * zoomLevel, windowPtr->getSize().y * zoomLevel));
                    windowView.reset(FloatRect(Vector2f(0.0f, 0.0f), Vector2f(windowPtr->getSize())));
                } else if (event.type == Event::Closed) {
                    state = State::Exiting;
                }
            }
            
            Vector2i newTileCursor(windowPtr->mapPixelToCoords(mouseStart, boardView));    // Check if cursor moved.
            newTileCursor.x /= boardPtr->getTileSize().x;
            newTileCursor.y /= boardPtr->getTileSize().y;
            if (newTileCursor.x >= 0 && newTileCursor.x < static_cast<int>(boardPtr->getSize().x) && newTileCursor.y >= 0 && newTileCursor.y < static_cast<int>(boardPtr->getSize().y)) {
                if (tileCursor != newTileCursor) {
                    if (tileCursor != Vector2i(-1, -1) && !selectionArea.contains(tileCursor)) {
                        boardPtr->getTile(tileCursor)->setHighlight(false);
                    }
                    boardPtr->getTile(newTileCursor)->setHighlight(true);
                    if (selectionStart == Vector2i(-1, -1) && Mouse::isButtonPressed(Mouse::Right) && currentTileBoardPtr->getSize() == Vector2u(0, 0) && !copyBufferVisible) {
                        boardPtr->highlightArea(selectionArea, false);
                        if (tileCursor != Vector2i(-1, -1)) {
                            selectionStart = tileCursor;
                        }
                        selectionArea = IntRect(0, 0, 0, 0);
                    }
                    if (selectionStart != Vector2i(-1, -1)) {
                        boardPtr->highlightArea(selectionArea, false);
                        selectionArea.left = min(selectionStart.x, newTileCursor.x);
                        selectionArea.top = min(selectionStart.y, newTileCursor.y);
                        selectionArea.width = max(selectionStart.x, newTileCursor.x) - selectionArea.left + 1;
                        selectionArea.height = max(selectionStart.y, newTileCursor.y) - selectionArea.top + 1;
                        boardPtr->highlightArea(selectionArea, true);
                    } else if (Mouse::isButtonPressed(Mouse::Right)) {
                        pasteToBoard(newTileCursor, Keyboard::isKeyPressed(Keyboard::LShift) || Keyboard::isKeyPressed(Keyboard::RShift));
                    }
                    tileCursor = newTileCursor;
                    currentTileBoardPtr->setPosition(static_cast<float>(tileCursor.x * Board::getTileSize().x), static_cast<float>(tileCursor.y * Board::getTileSize().y));
                    copyBufferBoardPtr->setPosition(currentTileBoardPtr->getPosition());
                }
            } else if (tileCursor != Vector2i(-1, -1)) {
                if (!selectionArea.contains(tileCursor)) {
                    boardPtr->getTile(tileCursor)->setHighlight(false);
                }
                tileCursor = Vector2i(-1, -1);
            }
        }
    } catch (exception& ex) {
        state = State::Exiting;
        cout << "\n****************************************************" << endl;
        cout << "* A fatal error has occurred, terminating program. *" << endl;
        cout << "****************************************************" << endl;
        cout << "Exception details: " << ex.what() << endl;
        cout << "(Press enter)" << endl;
        cin.get();
        renderThread.join();
        return -1;
    }
    
    renderThread.join();
    return 0;
}

int Simulator::randomInteger(int min, int max) {
    uniform_int_distribution<int> minMaxRange(min, max);
    return minMaxRange(mainRNG);
}

int Simulator::randomInteger(int n) {
    return randomInteger(0, n - 1);
}

void Simulator::fileOption(int option) {
    if (option == 0) {    // New board.
        tileCursor = Vector2i(-1, -1);
        selectionStart = Vector2i(-1, -1);
        selectionArea = IntRect(0, 0, 0, 0);
        boardPtr->newBoard();
        viewOption(3);
        cout << "Created new board with size " << boardPtr->getSize().x << " x " << boardPtr->getSize().y << "." << endl;
    } else if (option == 1 || option == 3) {    // Load board. Save as board.
        OPENFILENAME fileDialog;    // https://docs.microsoft.com/en-us/windows/desktop/dlgbox/using-common-dialog-boxes
        char filename[260];
        ZeroMemory(&fileDialog, sizeof(fileDialog));    // Initialize fileDialog.
        fileDialog.lStructSize = sizeof(fileDialog);
        fileDialog.hwndOwner = windowPtr->getSystemHandle();
        fileDialog.lpstrFilter = "All types (*.*)\0*.*\0Text file (*.txt)\0*.TXT\0";
        fileDialog.nFilterIndex = 2;
        fileDialog.lpstrFile = filename;
        fileDialog.lpstrFile[0] = '\0';    // Set to null string so that GetOpenFileName/GetSaveFileName does not initialize itself with the filename.
        fileDialog.nMaxFile = sizeof(filename);
        fileDialog.lpstrFileTitle = NULL;
        fileDialog.nMaxFileTitle = 0;
        fileDialog.lpstrInitialDir = "boards";
        fileDialog.lpstrDefExt = "txt";
        
        try {
            if (option == 1) {
                fileDialog.lpstrTitle = "Open Board File";
                fileDialog.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                
                if (GetOpenFileName(&fileDialog) == TRUE) {
                    tileCursor = Vector2i(-1, -1);
                    selectionStart = Vector2i(-1, -1);
                    selectionArea = IntRect(0, 0, 0, 0);
                    boardPtr->loadFile(string(filename));
                    viewOption(3);
                } else {
                    cout << "No file selected." << endl;
                }
            } else {
                fileDialog.lpstrTitle = "Save As Board File";
                fileDialog.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
                
                if (GetSaveFileName(&fileDialog) == TRUE) {
                    string filenameStr(filename);    // Decapitalize the file extension before saving.
                    size_t dotPosition = filenameStr.rfind('.');
                    if (dotPosition != string::npos) {
                        while (dotPosition < filenameStr.length()) {
                            filenameStr[dotPosition] = tolower(filenameStr[dotPosition]);
                            ++dotPosition;
                        }
                    }
                    boardPtr->saveFile(filenameStr);
                } else {
                    cout << "No file selected." << endl;
                }
            }
        } catch (exception& ex) {
            viewOption(3);
            cout << "Error occurred during file access! There may be a problem with file permissions and/or file formats." << endl;
            cout << "Exception details: " << ex.what() << endl;
        }
    } else if (option == 2) {    // Save board.
        boardPtr->saveFile(boardPtr->name + ".txt");
    } else if (option == 4) {    // Rename board.
        
    } else if (option == 5) {    // Resize board.
        
    } else if (option == 6) {    // Exit program.
        state = State::Exiting;
    } else {
        assert(false);
    }
}

void Simulator::viewOption(int option) {
    if (option == 0) {    // Toggle view/edit mode.
        if (editMode) {
            currentTileBoardPtr->clear();
            copyBufferVisible = false;
        }
        editMode = !editMode;
        Board::gridActive = editMode;
    } else if (option == 1) {    // Zoom in.
        float zoomDelta = 10.0f * zoomLevel * -0.04f;
        if (zoomLevel + zoomDelta > 0.2f && zoomLevel + zoomDelta < 20.0f) {
            zoomLevel += zoomDelta;
            boardView.setSize(Vector2f(windowPtr->getSize().x * zoomLevel, windowPtr->getSize().y * zoomLevel));
        }
    } else if (option == 2) {    // Zoom out.
        float zoomDelta = -10.0f * zoomLevel * -0.04f;
        if (zoomLevel + zoomDelta > 0.2f && zoomLevel + zoomDelta < 20.0f) {
            zoomLevel += zoomDelta;
            boardView.setSize(Vector2f(windowPtr->getSize().x * zoomLevel, windowPtr->getSize().y * zoomLevel));
        }
    } else if (option == 3) {    // Default zoom.
        zoomLevel = 1.0f;
        boardView.setSize(Vector2f(windowPtr->getSize().x * zoomLevel, windowPtr->getSize().y * zoomLevel));
        float xCenter = 0.5f * min(static_cast<float>(boardPtr->getSize().x * boardPtr->getTileSize().x), windowPtr->getSize().x - boardPtr->getTileSize().x * 2.0f);
        float yCenter = 0.5f * min(static_cast<float>(boardPtr->getSize().y * boardPtr->getTileSize().y) - 28.0f, windowPtr->getSize().y - (boardPtr->getTileSize().y + 28.0f) * 2.0f);
        boardView.setCenter(xCenter, yCenter);
        windowView.reset(FloatRect(Vector2f(0.0f, 0.0f), Vector2f(windowPtr->getSize())));
    } else {
        assert(false);
    }
}

void Simulator::runOption(int option) {
    if (option == 0) {    // Step frame.
        boardPtr->updateTiles();
    } else if (option == 1) {    // Change run mode.
        
    } else {
        assert(false);
    }
}

void Simulator::toolsOption(int option) {
    if (option == 0) {    // Select all.
        selectionArea = IntRect(0, 0, boardPtr->getSize().x, boardPtr->getSize().y);
        boardPtr->highlightArea(selectionArea, true);
    } else if (option == 1) {    // Deselect all.
        currentTileBoardPtr->clear();
        copyBufferVisible = false;
        if (selectionArea != IntRect(0, 0, 0, 0)) {
            boardPtr->highlightArea(selectionArea, false);
            selectionStart = Vector2i(-1, -1);
            selectionArea = IntRect(0, 0, 0, 0);
            if (tileCursor != Vector2i(-1, -1)) {
                boardPtr->getTile(tileCursor)->setHighlight(true);
            }
        }
    } else if (!editMode) {
        return;
    } else if (option == 2) {    // Rotate selection CW.
        if (currentTileBoardPtr->getSize() != Vector2u(0, 0)) {
            currentTileDirection = static_cast<Direction>((currentTileDirection + 1) % 4);
            currentTileBoardPtr->getTile(Vector2u(0, 0))->setDirection(currentTileDirection);
            currentTileBoardPtr->getTile(Vector2u(0, 0))->setHighlight(true);
        } else if (copyBufferVisible) {
            copyBufferBoardPtr->rotate(false);
            copyBufferBoardPtr->highlightArea(IntRect(0, 0, copyBufferBoardPtr->getSize().x, copyBufferBoardPtr->getSize().y), true);
        } else if (selectionArea != IntRect(0, 0, 0, 0)) {
            for (int y = selectionArea.top + selectionArea.height - 1; y >= selectionArea.top; --y) {
                for (int x = selectionArea.left + selectionArea.width - 1; x >= selectionArea.left; --x) {
                    Tile* targetTile = boardPtr->getTile(Vector2u(x, y));
                    targetTile->setDirection(static_cast<Direction>((targetTile->getDirection() + 1) % 4));
                    targetTile->setHighlight(true);
                }
            }
        }
    } else if (option == 3) {    // Rotate selection CCW.
        if (currentTileBoardPtr->getSize() != Vector2u(0, 0)) {
            currentTileDirection = static_cast<Direction>((currentTileDirection + 3) % 4);
            currentTileBoardPtr->getTile(Vector2u(0, 0))->setDirection(currentTileDirection);
            currentTileBoardPtr->getTile(Vector2u(0, 0))->setHighlight(true);
        } else if (copyBufferVisible) {
            copyBufferBoardPtr->rotate(true);
            copyBufferBoardPtr->highlightArea(IntRect(0, 0, copyBufferBoardPtr->getSize().x, copyBufferBoardPtr->getSize().y), true);
        } else if (selectionArea != IntRect(0, 0, 0, 0)) {
            for (int y = selectionArea.top + selectionArea.height - 1; y >= selectionArea.top; --y) {
                for (int x = selectionArea.left + selectionArea.width - 1; x >= selectionArea.left; --x) {
                    Tile* targetTile = boardPtr->getTile(Vector2u(x, y));
                    targetTile->setDirection(static_cast<Direction>((targetTile->getDirection() + 3) % 4));
                    targetTile->setHighlight(true);
                }
            }
        }
    } else if (option == 4) {    // Flip across vertical.
        if (copyBufferVisible) {
            copyBufferBoardPtr->flip(false);
            copyBufferBoardPtr->highlightArea(IntRect(0, 0, copyBufferBoardPtr->getSize().x, copyBufferBoardPtr->getSize().y), true);
        }
    } else if (option == 5) {    // Flip across horizontal.
        if (copyBufferVisible) {
            copyBufferBoardPtr->flip(true);
            copyBufferBoardPtr->highlightArea(IntRect(0, 0, copyBufferBoardPtr->getSize().x, copyBufferBoardPtr->getSize().y), true);
        }
    } else if (option == 6) {    // Cut selection.
        
    } else if (option == 7) {    // Copy selection.
        if (selectionArea != IntRect(0, 0, 0, 0)) {
            copyBufferBoardPtr->newBoard(Vector2u(selectionArea.width, selectionArea.height), "", true);
            copyBufferBoardPtr->cloneArea(*boardPtr, selectionArea, Vector2i(0, 0), true, true);
            copyBufferBoardPtr->highlightArea(IntRect(0, 0, copyBufferBoardPtr->getSize().x, copyBufferBoardPtr->getSize().y), true);
            toolsOption(8);
        }
    } else if (option == 8) {    // Paste selection.
        if (copyBufferBoardPtr->getSize() != Vector2u(0, 0)) {
            currentTileBoardPtr->clear();
            copyBufferVisible = true;
        }
    } else if (option == 9) {    // Delete selection.
        
    } else if (option == 10) {    // Wire tool.
        
    } else {
        assert(false);
    }
}

void Simulator::placeTile(int option) {
    if (!editMode) {
        return;
    }
    if (currentTileBoardPtr->getSize() != Vector2u(1, 1)) {
        currentTileBoardPtr->newBoard(Vector2u(1, 1), "");
    }
    if (option == 0) {
        currentTileBoardPtr->replaceTile(new Tile(currentTileBoardPtr, Vector2u(0, 0), true));
    } else if (option < 6) {
        currentTileBoardPtr->replaceTile(new TileWire(currentTileBoardPtr, Vector2u(0, 0), true, currentTileDirection, static_cast<TileWire::Type>(option - 1)));
    } else if (option == 6) {
        currentTileBoardPtr->replaceTile(new TileSwitch(currentTileBoardPtr, Vector2u(0, 0), true));
    } else if (option == 7) {
        currentTileBoardPtr->replaceTile(new TileButton(currentTileBoardPtr, Vector2u(0, 0), true));
    } else if (option == 8) {
        currentTileBoardPtr->replaceTile(new TileLED(currentTileBoardPtr, Vector2u(0, 0), true));
    } else if (option < 18) {
        currentTileBoardPtr->replaceTile(new TileGate(currentTileBoardPtr, Vector2u(0, 0), true, currentTileDirection, static_cast<TileGate::Type>(option - 9)));
    } else {
        assert(false);
    }
    currentTileBoardPtr->getTile(Vector2u(0, 0))->setHighlight(true);
    copyBufferVisible = false;
}

void Simulator::renderLoop() {
    windowPtr->setActive(true);
    Clock mainClock, fpsClock;    // The mainClock keeps track of elapsed frame time, fpsClock is used to count frames per second.
    int fpsCounter = 0;
    
    while (state != State::Exiting) {
        windowPtr->clear();
        windowPtr->setView(boardView);
        windowPtr->draw(*boardPtr);
        if (tileCursor != Vector2i(-1, -1)) {
            if (copyBufferVisible) {
                windowPtr->draw(*copyBufferBoardPtr);
            } else {
                windowPtr->draw(*currentTileBoardPtr);
            }
        }
        windowPtr->setView(windowView);
        windowPtr->draw(*userInterfacePtr);
        windowPtr->display();
        
        while (mainClock.getElapsedTime().asSeconds() < 1.0f / FPS_CAP) {}    // Slow down render if the current FPS is greater than the FPS cap.
        float deltaTime = mainClock.restart().asSeconds();    // Change in time since the last frame.
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {    // Calculate FPS.
            windowPtr->setTitle("[CircuitSim2] [" + boardPtr->name + "] [Size: " + to_string(boardPtr->getSize().x) + " x " + to_string(boardPtr->getSize().y) + "] [FPS: " + to_string(fpsCounter) + "]");
            fpsClock.restart();
            fpsCounter = 0;
        } else {
            ++fpsCounter;
        }
    }
    windowPtr->close();
}

void Simulator::handleKeyPress(Event::KeyEvent keyEvent) {
    if (keyEvent.control) {
        if (keyEvent.code == Keyboard::N) {
            fileOption(0);
        } else if (keyEvent.code == Keyboard::O) {
            fileOption(1);
        } else if (keyEvent.code == Keyboard::S) {
            fileOption(2);
        } else if (keyEvent.code == Keyboard::A) {
            toolsOption(0);
        } else if (keyEvent.code == Keyboard::X) {
            toolsOption(6);
        } else if (keyEvent.code == Keyboard::C) {
            toolsOption(7);
        } else if (keyEvent.code == Keyboard::V) {
            toolsOption(8);
        }
    } else {
        if (keyEvent.code == Keyboard::Enter) {
            viewOption(0);
        } else if (keyEvent.code == Keyboard::Tab) {
            if (!keyEvent.shift) {
                runOption(0);
            } else {
                //runOption(1);
            }
        } else if (keyEvent.code == Keyboard::Escape) {
            toolsOption(1);
        }
        if (editMode) {
            if (keyEvent.code == Keyboard::R) {
                if (!keyEvent.shift) {
                    toolsOption(2);
                } else {
                    toolsOption(3);
                }
            } else if (keyEvent.code == Keyboard::F) {
                if (!keyEvent.shift) {
                    toolsOption(4);
                } else {
                    toolsOption(5);
                }
            } else if (keyEvent.code == Keyboard::Delete) {
                toolsOption(9);
            } else if (keyEvent.code == Keyboard::Space) {
                placeTile(0);
            } else if (keyEvent.code == Keyboard::T) {
                if (!keyEvent.shift) {
                    placeTile(1);
                } else {
                    placeTile(3);
                }
            } else if (keyEvent.code == Keyboard::C) {
                if (!keyEvent.shift) {
                    placeTile(2);
                } else {
                    placeTile(5);
                }
            } else if (keyEvent.code == Keyboard::J) {
                placeTile(4);
            } else if (keyEvent.code == Keyboard::S) {
                if (!keyEvent.shift) {
                    placeTile(6);
                } else {
                    placeTile(7);
                }
            } else if (keyEvent.code == Keyboard::L) {
                placeTile(8);
            } else if (keyEvent.code == Keyboard::D) {
                placeTile(9);
            } else if (keyEvent.code == Keyboard::B) {
                if (!keyEvent.shift) {
                    placeTile(10);
                } else {
                    placeTile(11);
                }
            } else if (keyEvent.code == Keyboard::A) {
                if (!keyEvent.shift) {
                    placeTile(12);
                } else {
                    placeTile(13);
                }
            } else if (keyEvent.code == Keyboard::O) {
                if (!keyEvent.shift) {
                    placeTile(14);
                } else {
                    placeTile(15);
                }
            } else if (keyEvent.code == Keyboard::X) {
                if (!keyEvent.shift) {
                    placeTile(16);
                } else {
                    placeTile(17);
                }
            }
        }
    }
}

void Simulator::pasteToBoard(const Vector2i& tileCursor, bool forcePaste) {
    if (tileCursor == Vector2i(-1, -1)) {
        return;
    }
    if (!selectionArea.contains(tileCursor)) {
        if (currentTileBoardPtr->getSize() == Vector2u(1, 1)) {
            boardPtr->replaceTile(currentTileBoardPtr->getTile(Vector2u(0, 0))->clone(boardPtr, Vector2u(tileCursor)));
        } else if (copyBufferVisible) {
            IntRect pasteArea(0, 0, copyBufferBoardPtr->getSize().x, copyBufferBoardPtr->getSize().y);
            if (tileCursor.x + pasteArea.width > static_cast<int>(boardPtr->getSize().x)) {
                if (forcePaste) {
                    pasteArea.width = boardPtr->getSize().x - tileCursor.x;
                } else {
                    return;
                }
            }
            if (tileCursor.y + pasteArea.height > static_cast<int>(boardPtr->getSize().y)) {
                if (forcePaste) {
                    pasteArea.height = boardPtr->getSize().y - tileCursor.y;
                } else {
                    return;
                }
            }
            if (!forcePaste) {
                for (int y = tileCursor.y + pasteArea.height - 1; y >= tileCursor.y; --y) {
                    for (int x = tileCursor.x + pasteArea.width - 1; x >= tileCursor.x; --x) {
                        if (typeid(*boardPtr->getTile(Vector2u(x, y))) != typeid(Tile)) {
                            return;
                        }
                    }
                }
            }
            boardPtr->cloneArea(*copyBufferBoardPtr, pasteArea, tileCursor);
        }
        if (selectionArea != IntRect(0, 0, 0, 0)) {
            boardPtr->highlightArea(selectionArea, false);
            selectionStart = Vector2i(-1, -1);
            selectionArea = IntRect(0, 0, 0, 0);
            if (tileCursor != Vector2i(-1, -1)) {
                boardPtr->getTile(tileCursor)->setHighlight(true);
            }
        }
    } else {
        if (currentTileBoardPtr->getSize() == Vector2u(1, 1)) {
            for (int y = selectionArea.top + selectionArea.height - 1; y >= selectionArea.top; --y) {
                for (int x = selectionArea.left + selectionArea.width - 1; x >= selectionArea.left; --x) {
                    boardPtr->replaceTile(currentTileBoardPtr->getTile(Vector2u(0, 0))->clone(boardPtr, Vector2u(x, y)));
                    boardPtr->getTile(Vector2u(x, y))->setHighlight(true);
                }
            }
        } else if (copyBufferVisible) {
            IntRect pasteArea(0, 0, copyBufferBoardPtr->getSize().x, copyBufferBoardPtr->getSize().y);
            for (int y = selectionArea.top; y <= selectionArea.top + selectionArea.height - pasteArea.height; y += pasteArea.height) {
                for (int x = selectionArea.left; x <= selectionArea.left + selectionArea.width - pasteArea.width; x += pasteArea.width) {
                    boardPtr->cloneArea(*copyBufferBoardPtr, pasteArea, Vector2i(x, y));
                    boardPtr->highlightArea(IntRect(x, y, pasteArea.width, pasteArea.height), true);
                }
            }
        }
    }
}