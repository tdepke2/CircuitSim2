#include "Board.h"
#include "Simulator.h"
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
View Simulator::boardView, Simulator::windowView;
float Simulator::zoomLevel;
RenderWindow* Simulator::windowPtr = nullptr;
Board* Simulator::boardPtr = nullptr;
Board* Simulator::currentTileBoardPtr = nullptr;
Board* Simulator::copyBufferBoardPtr = nullptr;
Direction Simulator::currentTileDirection = NORTH;
bool Simulator::copyBufferVisible = false;
Vector2i Simulator::tileCursor(-1, -1), Simulator::selectionStart(-1, -1);
IntRect Simulator::selectionArea(0, 0, 0, 0);

int Simulator::start() {
    cout << "Initializing setup." << endl;
    RenderWindow window;
    windowPtr = &window;
    try {
        assert(state == State::Uninitialized);
        state = State::Running;
        mainRNG.seed(static_cast<unsigned long>(chrono::high_resolution_clock::now().time_since_epoch().count()));
        window.create(VideoMode(800, 800), "[CircuitSim2] Loading...", Style::Default, ContextSettings(0, 0, 4));
        
        viewOption(2);
        Board::loadTextures("resources/texturePackGrid.png", "resources/texturePackNoGrid.png", Vector2u(32, 32));
        Board board, currentTileBoard, copyBufferBoard;
        boardPtr = &board;
        currentTileBoardPtr = &currentTileBoard;
        copyBufferBoardPtr = &copyBufferBoard;
        board.newBoard();
        copyBufferBoard.newBoard(Vector2u(1, 1), "");
        UserInterface userInterface;
        Vector2i mouseStart(0, 0);
        Clock mainClock, fpsClock;    // The mainClock keeps track of elapsed frame time, fpsClock is used to count frames per second.
        int fpsCounter = 0;
        
        cout << "Loading completed." << endl;
        while (state != State::Exiting) {
            board.updateCosmetics();    // ######################################################################################################## May want to move this to end of loop (but make sure updates called before first draw).
            copyBufferBoard.updateCosmetics();
            currentTileBoard.updateCosmetics();
            window.clear();
            window.setView(boardView);
            window.draw(board);
            if (tileCursor != Vector2i(-1, -1)) {
                if (copyBufferVisible) {
                    window.draw(copyBufferBoard);
                } else {
                    window.draw(currentTileBoard);
                }
            }
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
                    if (event.mouseButton.button == Mouse::Left) {    // Check if view is moved.
                        userInterface.update(event.mouseButton.x, event.mouseButton.y, true);
                    } else if (event.mouseButton.button == Mouse::Right && (currentTileBoard.getSize() != Vector2u(0, 0) || copyBufferVisible)) {    // Check if tile/buffer will be placed.
                        pasteToBoard(tileCursor, Keyboard::isKeyPressed(Keyboard::LShift) || Keyboard::isKeyPressed(Keyboard::RShift));
                    }
                } else if (event.type == Event::MouseButtonReleased) {
                    if (event.mouseButton.button == Mouse::Right && currentTileBoard.getSize() == Vector2u(0, 0) && !copyBufferVisible) {
                        if (selectionStart == Vector2i(-1, -1)) {    // Check if selection was cancelled (right click made without dragging).
                            if (selectionArea != IntRect(0, 0, 0, 0)) {
                                board.highlightArea(selectionArea, false);
                                selectionArea = IntRect(0, 0, 0, 0);
                                if (tileCursor != Vector2i(-1, -1)) {
                                    board.getTile(tileCursor)->setHighlight(true);
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
                        boardView.setSize(Vector2f(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel));
                    }
                } else if (event.type == Event::KeyPressed) {
                    if (Keyboard::isKeyPressed(Keyboard::LControl) || Keyboard::isKeyPressed(Keyboard::RControl)) {
                        if (event.key.code == Keyboard::N) {
                            fileOption(0);
                        } else if (event.key.code == Keyboard::O) {
                            fileOption(1);
                        } else if (event.key.code == Keyboard::S) {
                            fileOption(2);
                        } else if (event.key.code == Keyboard::A) {
                            toolsOption(0);
                        } else if (event.key.code == Keyboard::X) {
                            toolsOption(6);
                        } else if (event.key.code == Keyboard::C) {
                            toolsOption(7);
                        } else if (event.key.code == Keyboard::V) {
                            toolsOption(8);
                        }
                    } else if (!Keyboard::isKeyPressed(Keyboard::LAlt) && !Keyboard::isKeyPressed(Keyboard::RAlt)) {
                        if (event.key.code == Keyboard::G) {
                            Board::gridActive = !Board::gridActive;
                        } else if (event.key.code == Keyboard::Tab) {
                            runOption(0);
                        } else if (event.key.code == Keyboard::Escape) {
                            toolsOption(1);
                        } else if (event.key.code == Keyboard::R) {
                            if (!Keyboard::isKeyPressed(Keyboard::LShift) && !Keyboard::isKeyPressed(Keyboard::RShift)) {
                                toolsOption(2);
                            } else {
                                toolsOption(3);
                            }
                        } else if (event.key.code == Keyboard::F) {
                            if (!Keyboard::isKeyPressed(Keyboard::LShift) && !Keyboard::isKeyPressed(Keyboard::RShift)) {
                                toolsOption(4);
                            } else {
                                toolsOption(5);
                            }
                        } else if (event.key.code == Keyboard::Delete) {
                            toolsOption(9);
                        } else if (event.key.code == Keyboard::Space) {
                            placeTile(0);
                        } else if (event.key.code == Keyboard::T) {
                            if (!Keyboard::isKeyPressed(Keyboard::LShift) && !Keyboard::isKeyPressed(Keyboard::RShift)) {
                                placeTile(1);
                            } else {
                                placeTile(3);
                            }
                        } else if (event.key.code == Keyboard::C) {
                            if (!Keyboard::isKeyPressed(Keyboard::LShift) && !Keyboard::isKeyPressed(Keyboard::RShift)) {
                                placeTile(2);
                            } else {
                                placeTile(5);
                            }
                        } else if (event.key.code == Keyboard::J) {
                            placeTile(4);
                        } else if (event.key.code == Keyboard::S) {
                            if (!Keyboard::isKeyPressed(Keyboard::LShift) && !Keyboard::isKeyPressed(Keyboard::RShift)) {
                                placeTile(6);
                            } else {
                                placeTile(7);
                            }
                        } else if (event.key.code == Keyboard::L) {
                            placeTile(8);
                        } else if (event.key.code == Keyboard::D) {
                            placeTile(9);
                        } else if (event.key.code == Keyboard::B) {
                            if (!Keyboard::isKeyPressed(Keyboard::LShift) && !Keyboard::isKeyPressed(Keyboard::RShift)) {
                                placeTile(10);
                            } else {
                                placeTile(11);
                            }
                        } else if (event.key.code == Keyboard::A) {
                            if (!Keyboard::isKeyPressed(Keyboard::LShift) && !Keyboard::isKeyPressed(Keyboard::RShift)) {
                                placeTile(12);
                            } else {
                                placeTile(13);
                            }
                        } else if (event.key.code == Keyboard::O) {
                            if (!Keyboard::isKeyPressed(Keyboard::LShift) && !Keyboard::isKeyPressed(Keyboard::RShift)) {
                                placeTile(14);
                            } else {
                                placeTile(15);
                            }
                        } else if (event.key.code == Keyboard::X) {
                            if (!Keyboard::isKeyPressed(Keyboard::LShift) && !Keyboard::isKeyPressed(Keyboard::RShift)) {
                                placeTile(16);
                            } else {
                                placeTile(17);
                            }
                        }
                    }
                } else if (event.type == Event::Resized) {
                    boardView.setSize(Vector2f(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel));
                    windowView.reset(FloatRect(Vector2f(0.0f, 0.0f), Vector2f(window.getSize())));
                } else if (event.type == Event::Closed) {
                    window.close();
                    state = State::Exiting;
                }
            }
            
            Vector2i newTileCursor(window.mapPixelToCoords(mouseStart, boardView));    // Check if cursor moved.
            newTileCursor.x /= board.getTileSize().x;
            newTileCursor.y /= board.getTileSize().y;
            if (newTileCursor.x >= 0 && newTileCursor.x < static_cast<int>(board.getSize().x) && newTileCursor.y >= 0 && newTileCursor.y < static_cast<int>(board.getSize().y)) {
                if (tileCursor != newTileCursor) {
                    if (tileCursor != Vector2i(-1, -1) && !selectionArea.contains(tileCursor)) {
                        board.getTile(tileCursor)->setHighlight(false);
                    }
                    board.getTile(newTileCursor)->setHighlight(true);
                    if (selectionStart == Vector2i(-1, -1) && Mouse::isButtonPressed(Mouse::Right) && currentTileBoard.getSize() == Vector2u(0, 0) && !copyBufferVisible) {
                        board.highlightArea(selectionArea, false);
                        if (tileCursor != Vector2i(-1, -1)) {
                            selectionStart = tileCursor;
                        }
                        selectionArea = IntRect(0, 0, 0, 0);
                    }
                    if (selectionStart != Vector2i(-1, -1)) {
                        board.highlightArea(selectionArea, false);
                        selectionArea.left = min(selectionStart.x, newTileCursor.x);
                        selectionArea.top = min(selectionStart.y, newTileCursor.y);
                        selectionArea.width = max(selectionStart.x, newTileCursor.x) - selectionArea.left + 1;
                        selectionArea.height = max(selectionStart.y, newTileCursor.y) - selectionArea.top + 1;
                        board.highlightArea(selectionArea, true);
                    } else if (Mouse::isButtonPressed(Mouse::Right)) {
                        pasteToBoard(newTileCursor, Keyboard::isKeyPressed(Keyboard::LShift) || Keyboard::isKeyPressed(Keyboard::RShift));
                    }
                    tileCursor = newTileCursor;
                    currentTileBoard.setPosition(static_cast<float>(tileCursor.x * Board::getTileSize().x), static_cast<float>(tileCursor.y * Board::getTileSize().y));
                    copyBufferBoard.setPosition(currentTileBoard.getPosition());
                }
            } else if (tileCursor != Vector2i(-1, -1)) {
                if (!selectionArea.contains(tileCursor)) {
                    board.getTile(tileCursor)->setHighlight(false);
                }
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

void Simulator::fileOption(int option) {
    if (option == 0) {    // New board.
        boardPtr->newBoard();
        cout << "Created new board with size " << boardPtr->getSize().x << " x " << boardPtr->getSize().y << "." << endl;
        viewOption(2);
    } else if (option == 1) {    // Load board.
        OPENFILENAME fileDialog;    // https://docs.microsoft.com/en-us/windows/desktop/dlgbox/using-common-dialog-boxes
        char filename[260];
        
        ZeroMemory(&fileDialog, sizeof(fileDialog));    // Initialize fileDialog.
        fileDialog.lStructSize = sizeof(fileDialog);
        fileDialog.hwndOwner = windowPtr->getSystemHandle();
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
            viewOption(2);
        } else {
            cout << "No file selected." << endl;
        }
    } else if (option == 2) {    // Save board.
        
    } else if (option == 3) {    // Save as board.
        
    } else if (option == 4) {    // Rename board.
        
    } else if (option == 5) {    // Resize board.
        
    } else if (option == 6) {    // Exit program.
        state = State::Exiting;
    } else {
        assert(false);
    }
}

void Simulator::viewOption(int option) {
    if (option == 0) {    // Zoom in.
        
    } else if (option == 1) {    // Zoom out.
        
    } else if (option == 2) {    // Default zoom.
        zoomLevel = 1.0f;
        boardView.setSize(Vector2f(windowPtr->getSize().x * zoomLevel, windowPtr->getSize().y * zoomLevel));
        windowView.reset(FloatRect(Vector2f(0.0f, 0.0f), Vector2f(windowPtr->getSize())));
        boardView.setCenter(0.5f * Vector2f(windowPtr->getSize()) - Vector2f(32.0f, 60.0f));
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
            copyBufferBoardPtr->cloneArea(*boardPtr, selectionArea, Vector2i(0, 0), true);
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
                        if (boardPtr->getTile(Vector2u(x, y))->getTextureID() != 0) {
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