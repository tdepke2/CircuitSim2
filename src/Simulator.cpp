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
#include <csignal>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <typeinfo>
#include <windows.h>

const unsigned int Simulator::FRAMERATE_LIMIT = 60;
atomic<Simulator::State> Simulator::state = State::Uninitialized;
Simulator::SimSpeed Simulator::simSpeed = SimSpeed::Medium;
mt19937 Simulator::mainRNG;
mutex Simulator::renderMutex, Simulator::renderReadyMutex;
int Simulator::fpsCounter = 0, Simulator::tpsCounter = 0;
View Simulator::boardView, Simulator::windowView;
float Simulator::zoomLevel;
RenderWindow* Simulator::windowPtr = nullptr;
Board* Simulator::boardPtr = nullptr;
Board* Simulator::currentTileBoardPtr = nullptr;
Board* Simulator::copyBufferBoardPtr = nullptr;
Board* Simulator::wireVerticalBoardPtr = nullptr;
Board* Simulator::wireHorizontalBoardPtr = nullptr;
UserInterface* Simulator::userInterfacePtr = nullptr;
Text* Simulator::wireToolLabelPtr = nullptr;
Direction Simulator::currentTileDirection = NORTH;
bool Simulator::editMode = true, Simulator::copyBufferVisible = false, Simulator::wireToolVerticalFirst = true;
Vector2i Simulator::tileCursor(-1, -1), Simulator::selectionStart(-1, -1), Simulator::wireToolStart(-1, -1);
Vector2u Simulator::wireVerticalPosition(0, 0), Simulator::wireHorizontalPosition(0, 0);
IntRect Simulator::selectionArea(0, 0, 0, 0);
Tile* Simulator::relabelTargetTile = nullptr;

int Simulator::start() {
    cout << "Initializing setup..." << endl;
    int exitCode = 0;
    thread renderThread;
    try {
        renderMutex.lock();
        assert(state == State::Uninitialized);
        state = State::Running;
        SetConsoleCtrlHandler(NULL, TRUE);    // Disable control signals (like Ctrl + C) in log window in case the user tries to close application this way.
        signal(SIGBREAK, terminationHandler);    // If log window closed, need to clean up in the few milliseconds available before program goes down.
        mainRNG.seed(static_cast<unsigned long>(chrono::high_resolution_clock::now().time_since_epoch().count()));
        windowPtr = new RenderWindow(VideoMode(1300, 900), "[CircuitSim2] Loading...", Style::Default, ContextSettings(0, 0, 4));
        windowPtr->setFramerateLimit(FRAMERATE_LIMIT);
        windowPtr->setActive(false);
        renderThread = thread(renderLoop);
        
        Board::loadTextures("resources/texturePackGrid.png", "resources/texturePackNoGrid.png", Vector2u(32, 32));
        Board::loadFont("resources/consolas.ttf");
        boardPtr = new Board();
        currentTileBoardPtr = new Board();
        copyBufferBoardPtr = new Board();
        wireVerticalBoardPtr = new Board();
        wireHorizontalBoardPtr = new Board();
        char directoryPath[260];
        GetCurrentDirectory(sizeof(directoryPath), directoryPath);
        Board::newBoardDefaultPath = string(directoryPath) + "\\boards\\NewBoard.txt";
        boardPtr->newBoard();
        copyBufferBoardPtr->newBoard(Vector2u(1, 1), "");
        copyBufferBoardPtr->highlightArea(IntRect(0, 0, copyBufferBoardPtr->getSize().x, copyBufferBoardPtr->getSize().y), true);
        userInterfacePtr = new UserInterface();
        wireToolLabelPtr = new Text("", Board::getFont(), 30);
        wireToolLabelPtr->setFillColor(Color::White);
        wireToolLabelPtr->setOutlineColor(Color::Black);
        wireToolLabelPtr->setOutlineThickness(1.0f);
        viewOption(3);
        Vector2i mouseStart(0, 0);
        Clock perSecondClock, loopClock, tickClock;    // The perSecondClock counts FPS and TPS, loopClock limits loops per second, tickClock manages tick speed.
        renderMutex.unlock();
        
        cout << "Loading completed." << endl;
        while (state != State::Exiting) {
            renderReadyMutex.lock();    // Two mutex system helps prevent starvation of render thread and main thread (a binary semaphore could work even better though).
            renderMutex.lock();
            renderReadyMutex.unlock();
            Event event;
            while (windowPtr->pollEvent(event)) {    // Process events.
                if (event.type == Event::MouseMoved) {
                    if (!UserInterface::isDialogPromptOpen() && Mouse::isButtonPressed(Mouse::Left)) {
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
                    } else if (!UserInterface::isDialogPromptOpen() && event.mouseButton.button == Mouse::Right) {
                        if (wireToolStart == Vector2i(-1, -1)) {    // Check if tile/buffer will be placed.
                            pasteToBoard(tileCursor, Keyboard::isKeyPressed(Keyboard::LShift) || Keyboard::isKeyPressed(Keyboard::RShift));
                        } else {
                            for (unsigned int x = 0; x < wireHorizontalBoardPtr->getSize().x; ++x) {
                                boardPtr->replaceTile(wireHorizontalBoardPtr->getTile(Vector2u(x, 0))->clone(boardPtr, wireHorizontalPosition + Vector2u(x, 0)));
                            }
                            for (unsigned int y = 0; y < wireVerticalBoardPtr->getSize().y; ++y) {
                                boardPtr->replaceTile(wireVerticalBoardPtr->getTile(Vector2u(0, y))->clone(boardPtr, wireVerticalPosition + Vector2u(0, y)));
                            }
                            
                            wireToolStart = Vector2i(-1, -1);
                            wireVerticalBoardPtr->clear();
                            wireHorizontalBoardPtr->clear();
                        }
                    }
                } else if (event.type == Event::MouseButtonReleased) {
                    if (!UserInterface::isDialogPromptOpen() && event.mouseButton.button == Mouse::Right && currentTileBoardPtr->getSize() == Vector2u(0, 0) && !copyBufferVisible) {
                        if (selectionStart == Vector2i(-1, -1)) {    // Check if selection was cancelled (right click made without dragging).
                            boardPtr->highlightArea(selectionArea, false);
                            selectionArea = IntRect(0, 0, 0, 0);
                            if (tileCursor != Vector2i(-1, -1)) {
                                boardPtr->getTile(tileCursor)->setHighlight(true);
                            }
                        } else {    // Else, finish the selection.
                            selectionStart = Vector2i(-1, -1);
                        }
                    }
                } else if (event.type == Event::MouseWheelScrolled) {
                    float zoomDelta = event.mouseWheelScroll.delta * (1 + (Keyboard::isKeyPressed(Keyboard::LShift) || Keyboard::isKeyPressed(Keyboard::RShift)) * 5) * zoomLevel * -0.04f;
                    if (!UserInterface::isDialogPromptOpen() && zoomLevel + zoomDelta > 0.2f && zoomLevel + zoomDelta < 20.0f) {
                        zoomLevel += zoomDelta;
                        boardView.setSize(Vector2f(windowPtr->getSize().x * zoomLevel, windowPtr->getSize().y * zoomLevel));
                    }
                } else if (event.type == Event::KeyPressed) {
                    if (!UserInterface::isDialogPromptOpen()) {
                        handleKeyPress(event.key);
                    }
                } else if (event.type == Event::TextEntered) {
                    userInterfacePtr->update(event.text);
                    if (!UserInterface::isDialogPromptOpen()) {
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
                    }
                } else if (event.type == Event::Resized) {
                    boardView.setSize(Vector2f(windowPtr->getSize().x * zoomLevel, windowPtr->getSize().y * zoomLevel));
                    windowView.reset(FloatRect(Vector2f(0.0f, 0.0f), Vector2f(windowPtr->getSize())));
                } else if (event.type == Event::Closed) {
                    fileOption(6);
                }
            }
            if (UserInterface::fieldToSelectPtr != nullptr) {    // If dialog box with text field is opening up, the field gets selected after events have been processed.
                UserInterface::fieldToSelectPtr->selected = true;
                UserInterface::fieldToSelectPtr = nullptr;
            }
            
            if (!UserInterface::isDialogPromptOpen()) {
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
                        updateWireTool(tileCursor, newTileCursor);
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
                
                if (simSpeed == SimSpeed::Slow && tickClock.getElapsedTime().asSeconds() >= 1.0f / 2.0f) {
                    nextTick();
                    tickClock.restart();
                } else if (simSpeed == SimSpeed::Medium && tickClock.getElapsedTime().asSeconds() >= 1.0f / 30.0f) {
                    nextTick();
                    tickClock.restart();
                } else if (simSpeed == SimSpeed::Fast && tickClock.getElapsedTime().asSeconds() >= 1.0f / 60.0f) {
                    nextTick();
                    tickClock.restart();
                } else if (simSpeed == SimSpeed::Extreme) {
                    nextTick();
                }
            }
            
            if (perSecondClock.getElapsedTime().asSeconds() >= 1.0f) {    // Calculate FPS and TPS.
                windowPtr->setTitle("[CircuitSim2] [" + boardPtr->name + "] [Size: " + to_string(boardPtr->getSize().x) + " x " + to_string(boardPtr->getSize().y) + "] [FPS: " + to_string(fpsCounter) + ", TPS: " + to_string(tpsCounter) + "]");
                perSecondClock.restart();
                fpsCounter = 0;
                tpsCounter = 0;
            }
            renderMutex.unlock();
            
            int64_t sleepTime = static_cast<int64_t>(1.0 / 500.0 * 1.0e6) - loopClock.restart().asMicroseconds();    // Pause a little bit (limit to 500 loops per second) if not running in extreme mode.
            if (simSpeed != SimSpeed::Extreme && sleepTime > 0) {
                this_thread::sleep_for(chrono::microseconds(sleepTime));
            }
        }
    } catch (exception& ex) {
        state = State::Exiting;
        renderMutex.unlock();
        cout << "\n****************************************************" << endl;
        cout << "* A fatal error has occurred, terminating program. *" << endl;
        cout << "****************************************************" << endl;
        cout << "Exception details: " << ex.what() << endl;
        cout << "(Press enter)" << endl;
        cin.get();
        exitCode = -1;
    }
    
    renderThread.join();    // Clean up dynamically allocated memory and exit.
    delete windowPtr;
    delete boardPtr;
    delete currentTileBoardPtr;
    delete copyBufferBoardPtr;
    delete wireVerticalBoardPtr;
    delete wireHorizontalBoardPtr;
    delete userInterfacePtr;
    delete wireToolLabelPtr;
    return exitCode;
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
        if (!userInterfacePtr->savePrompt.visible && boardPtr->changesMade) {
            userInterfacePtr->savePrompt.optionButtons[1].actionOption = 0;
            userInterfacePtr->savePrompt.show();
        } else {
            toolsOption(1);
            if (tileCursor != Vector2i(-1, -1)) {
                boardPtr->getTile(tileCursor)->setHighlight(false);
                tileCursor = Vector2i(-1, -1);
            }
            boardPtr->newBoard();
            viewOption(3);
            cout << "Created new board with size " << boardPtr->getSize().x << " x " << boardPtr->getSize().y << "." << endl;
            UserInterface::closeAllDialogPrompts();
        }
    } else if (option == 1 || option == 3) {    // Load board. Save as board.
        if (option == 1 && !userInterfacePtr->savePrompt.visible && boardPtr->changesMade) {
            userInterfacePtr->savePrompt.optionButtons[1].actionOption = 1;
            userInterfacePtr->savePrompt.show();
            return;
        }
        UserInterface::closeAllDialogPrompts();
        
        OPENFILENAME fileDialog;    // https://docs.microsoft.com/en-us/windows/desktop/dlgbox/using-common-dialog-boxes
        char filename[260];
        ZeroMemory(&fileDialog, sizeof(fileDialog));    // Initialize fileDialog.
        fileDialog.lStructSize = sizeof(fileDialog);
        fileDialog.hwndOwner = windowPtr->getSystemHandle();
        fileDialog.lpstrFilter = "All types (*.*)\0*.*\0Text file (*.txt)\0*.txt\0";
        fileDialog.nFilterIndex = 2;
        fileDialog.lpstrFile = filename;
        fileDialog.lpstrFile[0] = '\0';    // Set to null string so that GetOpenFileName does not initialize itself with the filename.
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
                    toolsOption(1);
                    if (tileCursor != Vector2i(-1, -1)) {
                        boardPtr->getTile(tileCursor)->setHighlight(false);
                        tileCursor = Vector2i(-1, -1);
                    }
                    boardPtr->loadFile(string(filename));
                    viewOption(3);
                } else {
                    cout << "No file selected." << endl;
                }
            } else {
                strcpy_s(filename, 260, boardPtr->name.substr(boardPtr->name.rfind('\\') + 1).c_str());
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
            cout << "Error: Exception occurred during file access. " << ex.what() << endl;
        }
    } else if (option == 2) {    // Save board.
        try {
            boardPtr->saveFile(boardPtr->name + ".txt");
        } catch (exception& ex) {
            cout << "Error: Failed to save board. " << ex.what() << endl;
        }
        UserInterface::closeAllDialogPrompts();
    } else if (option == 4) {    // Rename board.
        if (!userInterfacePtr->renamePrompt.visible) {
            userInterfacePtr->renamePrompt.optionFields[0].setString(boardPtr->name.substr(boardPtr->name.rfind('\\') + 1));
            userInterfacePtr->renamePrompt.show();
        } else {
            try {
                string newPath, newName = userInterfacePtr->renamePrompt.optionFields[0].field.getString().toAnsiString();
                if (newName.find('/') != string::npos || newName.find('\\') != string::npos) {
                    throw runtime_error("Name cannot use slash or backslash, use \"Save As...\" to change board path.");
                }
                size_t lastSlashPosition = boardPtr->name.rfind('\\');
                if (lastSlashPosition != string::npos) {
                    newPath = boardPtr->name.substr(0, lastSlashPosition + 1) + newName;
                } else {
                    throw runtime_error("Cannot parse the board path.");
                }
                ifstream inputFile(boardPtr->name + ".txt");
                if (inputFile.is_open()) {    // If save file exists, try to rename it.
                    inputFile.close();
                    if (rename((boardPtr->name + ".txt").c_str(), (newPath + ".txt").c_str()) != 0) {
                        throw runtime_error("Board with this name already exists.");
                    }
                } else {    // Else, make sure no save exists for a board with the new name.
                    inputFile.open(newPath + ".txt");
                    if (inputFile.is_open()) {
                        inputFile.close();
                        throw runtime_error("Board with this name already exists.");
                    }
                }
                boardPtr->name = newPath;
                cout << "Board renamed to: \"" << newName << "\"." << endl;
                UserInterface::closeAllDialogPrompts();
            } catch (exception& ex) {
                cout << "Error: Unable to rename the board. " << ex.what() << endl;
            }
        }
    } else if (option == 5) {    // Resize board.
        if (!userInterfacePtr->resizePrompt.visible) {
            userInterfacePtr->resizePrompt.optionFields[0].setString(to_string(boardPtr->getSize().x));
            userInterfacePtr->resizePrompt.optionFields[1].setString(to_string(boardPtr->getSize().y));
            userInterfacePtr->resizePrompt.show();
        } else {
            try {
                int width = stol(userInterfacePtr->resizePrompt.optionFields[0].field.getString().toAnsiString());
                int height = stol(userInterfacePtr->resizePrompt.optionFields[1].field.getString().toAnsiString());
                if (width <= 0 || height <= 0) {
                    throw runtime_error("Board dimensions cannot be zero or negative.");
                }
                toolsOption(1);
                if (tileCursor != Vector2i(-1, -1)) {
                    boardPtr->getTile(tileCursor)->setHighlight(false);
                    tileCursor = Vector2i(-1, -1);
                }
                boardPtr->resize(Vector2u(width, height));
                viewOption(3);
                UserInterface::closeAllDialogPrompts();
            } catch (exception& ex) {
                cout << "Error: Failed to resize the board. " << ex.what() << endl;
            }
        }
    } else if (option == 6) {    // Exit program.
        if (!userInterfacePtr->savePrompt.visible && boardPtr->changesMade) {
            userInterfacePtr->savePrompt.optionButtons[1].actionOption = 6;
            userInterfacePtr->savePrompt.show();
        } else {
            UserInterface::closeAllDialogPrompts();
            state = State::Exiting;
        }
    } else {
        assert(false);
    }
}

void Simulator::viewOption(int option) {
    if (option == 0) {    // Toggle view/edit mode.
        if (editMode) {
            currentTileBoardPtr->clear();
            copyBufferVisible = false;
            wireToolStart = Vector2i(-1, -1);
            wireVerticalBoardPtr->clear();
            wireHorizontalBoardPtr->clear();
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
    if (option == 0) {    // Step one tick.
        nextTick();
        simSpeed = SimSpeed::Paused;
        userInterfacePtr->tpsDisplay.text.setString(" Current TPS limit: Paused    ");
        userInterfacePtr->tpsDisplay.button.setFillColor(Color(10, 10, 230));
    } else if (option == 1) {    // Change max TPS.
        if (simSpeed == SimSpeed::Paused) {
            simSpeed = SimSpeed::Slow;
            userInterfacePtr->tpsDisplay.text.setString(" Current TPS limit: 2         ");
            userInterfacePtr->tpsDisplay.button.setFillColor(Color(10, 230, 230));
        } else if (simSpeed == SimSpeed::Slow) {
            simSpeed = SimSpeed::Medium;
            userInterfacePtr->tpsDisplay.text.setString(" Current TPS limit: 30        ");
            userInterfacePtr->tpsDisplay.button.setFillColor(Color(10, 230, 10));
        } else if (simSpeed == SimSpeed::Medium) {
            simSpeed = SimSpeed::Fast;
            userInterfacePtr->tpsDisplay.text.setString(" Current TPS limit: 60        ");
            userInterfacePtr->tpsDisplay.button.setFillColor(Color(230, 230, 10));
        } else if (simSpeed == SimSpeed::Fast) {
            simSpeed = SimSpeed::Extreme;
            userInterfacePtr->tpsDisplay.text.setString(" Current TPS limit: Unlimited ");
            userInterfacePtr->tpsDisplay.button.setFillColor(Color(230, 10, 10));
        } else {
            simSpeed = SimSpeed::Paused;
            userInterfacePtr->tpsDisplay.text.setString(" Current TPS limit: Paused    ");
            userInterfacePtr->tpsDisplay.button.setFillColor(Color(10, 10, 230));
        }
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
        boardPtr->highlightArea(selectionArea, false);
        selectionStart = Vector2i(-1, -1);
        selectionArea = IntRect(0, 0, 0, 0);
        if (tileCursor != Vector2i(-1, -1)) {
            boardPtr->getTile(tileCursor)->setHighlight(true);
        }
        wireToolStart = Vector2i(-1, -1);
        wireVerticalBoardPtr->clear();
        wireHorizontalBoardPtr->clear();
    } else if (!editMode) {
        return;
    } else if (option == 2 || option == 3) {    // Rotate selection CW. Rotate selection CCW.
        if (currentTileBoardPtr->getSize() != Vector2u(0, 0)) {
            currentTileDirection = static_cast<Direction>((currentTileDirection + 1 + 2 * (option == 3)) % 4);
            currentTileBoardPtr->getTile(Vector2u(0, 0))->setDirection(currentTileDirection);
        } else if (copyBufferVisible) {
            copyBufferBoardPtr->rotate(option == 3);
        } else if (selectionArea != IntRect(0, 0, 0, 0)) {
            for (int y = selectionArea.top + selectionArea.height - 1; y >= selectionArea.top; --y) {
                for (int x = selectionArea.left + selectionArea.width - 1; x >= selectionArea.left; --x) {
                    Tile* targetTile = boardPtr->getTile(Vector2u(x, y));
                    targetTile->setDirection(static_cast<Direction>((targetTile->getDirection() + 1 + 2 * (option == 3)) % 4));
                }
            }
        } else if (wireToolStart != Vector2i(-1, -1)) {
            wireToolVerticalFirst = !wireToolVerticalFirst;
            updateWireTool(Vector2i(-1, -1), tileCursor);
        } else if (tileCursor != Vector2i(-1, -1)) {
            boardPtr->getTile(tileCursor)->setDirection(static_cast<Direction>((boardPtr->getTile(tileCursor)->getDirection() + 1 + 2 * (option == 3)) % 4));
        }
    } else if (option == 4 || option == 5) {    // Flip across vertical. Flip across horizontal.
        if (currentTileBoardPtr->getSize() != Vector2u(0, 0)) {
            currentTileBoardPtr->getTile(Vector2u(0, 0))->flip(option == 5);
        } else if (copyBufferVisible) {
            copyBufferBoardPtr->flip(option == 5);
        } else if (tileCursor != Vector2i(-1, -1)) {
            boardPtr->getTile(tileCursor)->flip(option == 5);
        }
    } else if (option == 6) {    // Toggle state.
        if (currentTileBoardPtr->getSize() != Vector2u(0, 0)) {
            if (currentTileBoardPtr->getTile(Vector2u(0, 0))->getState() == LOW) {
                currentTileBoardPtr->getTile(Vector2u(0, 0))->setState(HIGH);
            } else {
                currentTileBoardPtr->getTile(Vector2u(0, 0))->setState(LOW);
            }
        } else if (copyBufferVisible) {
            for (unsigned int y = 0; y < copyBufferBoardPtr->getSize().y; ++y) {
                for (unsigned int x = 0; x < copyBufferBoardPtr->getSize().x; ++x) {
                    Tile* targetTile = copyBufferBoardPtr->getTile(Vector2u(x, y));
                    if (targetTile->getState() == LOW) {
                        targetTile->setState(HIGH);
                    } else {
                        targetTile->setState(LOW);
                    }
                }
            }
        } else if (selectionArea != IntRect(0, 0, 0, 0)) {
            for (int y = selectionArea.top + selectionArea.height - 1; y >= selectionArea.top; --y) {
                for (int x = selectionArea.left + selectionArea.width - 1; x >= selectionArea.left; --x) {
                    Tile* targetTile = boardPtr->getTile(Vector2u(x, y));
                    if (targetTile->getState() == LOW) {
                        targetTile->setState(HIGH);
                    } else {
                        targetTile->setState(LOW);
                    }
                }
            }
        } else if (tileCursor != Vector2i(-1, -1)) {
            if (boardPtr->getTile(tileCursor)->getState() == LOW) {
                boardPtr->getTile(tileCursor)->setState(HIGH);
            } else {
                boardPtr->getTile(tileCursor)->setState(LOW);
            }
        }
    } else if (option == 7) {    // Edit/alternative tile.
        if (currentTileBoardPtr->getSize() != Vector2u(0, 0)) {
            if (currentTileBoardPtr->getTile(Vector2u(0, 0))->alternativeTile()) {
                relabelTargetTile = currentTileBoardPtr->getTile(Vector2u(0, 0));
                userInterfacePtr->relabelPrompt.clearFields();
                userInterfacePtr->relabelPrompt.show();
            }
        } else if (tileCursor != Vector2i(-1, -1)) {
            if (boardPtr->getTile(tileCursor)->alternativeTile()) {
                relabelTargetTile = boardPtr->getTile(tileCursor);
                userInterfacePtr->relabelPrompt.clearFields();
                userInterfacePtr->relabelPrompt.show();
            }
        }
    } else if (option == 8) {    // Cut selection.
        toolsOption(9);
        toolsOption(11);
    } else if (option == 9) {    // Copy selection.
        if (selectionArea != IntRect(0, 0, 0, 0)) {
            copyBufferBoardPtr->newBoard(Vector2u(selectionArea.width, selectionArea.height), "", true);
            copyBufferBoardPtr->cloneArea(*boardPtr, selectionArea, Vector2i(0, 0), true, true);
            copyBufferBoardPtr->highlightArea(IntRect(0, 0, copyBufferBoardPtr->getSize().x, copyBufferBoardPtr->getSize().y), true);
            toolsOption(10);
        }
    } else if (option == 10) {    // Paste selection.
        currentTileBoardPtr->clear();
        copyBufferVisible = true;
        wireToolStart = Vector2i(-1, -1);
        wireVerticalBoardPtr->clear();
        wireHorizontalBoardPtr->clear();
    } else if (option == 11) {    // Delete selection.
        for (int y = selectionArea.top + selectionArea.height - 1; y >= selectionArea.top; --y) {
            for (int x = selectionArea.left + selectionArea.width - 1; x >= selectionArea.left; --x) {
                if (typeid(*(boardPtr->getTile(Vector2u(x, y)))) != typeid(Tile)) {
                    boardPtr->replaceTile(new Tile(boardPtr, Vector2u(x, y)));
                }
            }
        }
        boardPtr->highlightArea(selectionArea, false);
        selectionStart = Vector2i(-1, -1);
        selectionArea = IntRect(0, 0, 0, 0);
        if (tileCursor != Vector2i(-1, -1)) {
            boardPtr->getTile(tileCursor)->setHighlight(true);
        }
    } else if (option == 12) {    // Wire tool.
        if (wireToolStart == Vector2i(-1, -1)) {
            if (tileCursor != Vector2i(-1, -1)) {
                currentTileBoardPtr->clear();
                copyBufferVisible = false;
                wireToolStart = tileCursor;
                wireToolLabelPtr->setString("");
            }
        } else {
            wireToolStart = Vector2i(-1, -1);
            wireVerticalBoardPtr->clear();
            wireHorizontalBoardPtr->clear();
        }
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
    wireToolStart = Vector2i(-1, -1);
    wireVerticalBoardPtr->clear();
    wireHorizontalBoardPtr->clear();
}

void Simulator::relabelTarget(int option) {
    assert(relabelTargetTile != nullptr);
    if (userInterfacePtr->relabelPrompt.optionFields[0].field.getString().getSize() == 0) {
        cout << "Error: No label entered." << endl;
        return;
    }
    if (typeid(*relabelTargetTile) == typeid(TileSwitch)) {
        static_cast<TileSwitch*>(relabelTargetTile)->setCharID(userInterfacePtr->relabelPrompt.optionFields[0].field.getString()[0]);
    } else if (typeid(*relabelTargetTile) == typeid(TileButton)) {
        static_cast<TileButton*>(relabelTargetTile)->setCharID(userInterfacePtr->relabelPrompt.optionFields[0].field.getString()[0]);
    } else {
        assert(false);
    }
    UserInterface::closeAllDialogPrompts();
}

void Simulator::terminationHandler(int sigNum) {
    state = State::Exiting;
    cout << "Closing program." << endl;
    this_thread::sleep_for(chrono::milliseconds(1000));
}

void Simulator::renderLoop() {
    windowPtr->setActive(true);
    renderMutex.lock();    // First time lock to wait for setup.
    renderMutex.unlock();
    
    while (state != State::Exiting) {
        renderReadyMutex.lock();
        renderMutex.lock();
        renderReadyMutex.unlock();
        boardPtr->updateCosmetics();
        copyBufferBoardPtr->updateCosmetics();
        currentTileBoardPtr->updateCosmetics();
        wireVerticalBoardPtr->updateCosmetics();
        wireHorizontalBoardPtr->updateCosmetics();
        
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
        if (wireToolStart != Vector2i(-1, -1)) {
            windowPtr->draw(*wireVerticalBoardPtr);
            windowPtr->draw(*wireHorizontalBoardPtr);
            windowPtr->draw(*wireToolLabelPtr);
        }
        windowPtr->setView(windowView);
        windowPtr->draw(*userInterfacePtr);
        ++fpsCounter;
        renderMutex.unlock();
        windowPtr->display();    // Display invokes sf::sleep to pause for next frame and keep framerate constant.
    }
    windowPtr->close();
}

void Simulator::nextTick() {
    boardPtr->updateTiles();
    ++tpsCounter;
    if (Board::numStateErrors > 0) {
        if (Board::numStateErrors > 10) {
            cout << "(and " << Board::numStateErrors - 10 << " more...)" << endl;
        }
        cout << "Detected " << Board::numStateErrors << " total conflict(s). Sources of error have been highlighted." << endl;
        Board::numStateErrors = 0;
        simSpeed = SimSpeed::Paused;
        userInterfacePtr->tpsDisplay.text.setString(" Current TPS limit: Paused    ");
        userInterfacePtr->tpsDisplay.button.setFillColor(Color(10, 10, 230));
    }
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
            toolsOption(8);
        } else if (keyEvent.code == Keyboard::C) {
            toolsOption(9);
        } else if (keyEvent.code == Keyboard::V) {
            toolsOption(10);
        }
    } else {
        if (keyEvent.code == Keyboard::Enter) {
            viewOption(0);
        } else if (keyEvent.code == Keyboard::Tab) {
            if (!keyEvent.shift) {
                runOption(0);
            } else {
                runOption(1);
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
            } else if (keyEvent.code == Keyboard::E) {
                if (!keyEvent.shift) {
                    toolsOption(6);
                } else {
                    toolsOption(7);
                }
            } else if (keyEvent.code == Keyboard::Delete) {
                toolsOption(11);
            } else if (keyEvent.code == Keyboard::W) {
                toolsOption(12);
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
        boardPtr->highlightArea(selectionArea, false);
        selectionStart = Vector2i(-1, -1);
        selectionArea = IntRect(0, 0, 0, 0);
        if (tileCursor != Vector2i(-1, -1)) {
            boardPtr->getTile(tileCursor)->setHighlight(true);
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

void Simulator::updateWireTool(const Vector2i& tileCursor, const Vector2i& newTileCursor) {
    if (wireToolStart != Vector2i(-1, -1)) {
        Vector2u verticalCornerPosition(numeric_limits<unsigned int>::max(), numeric_limits<unsigned int>::max()), horizontalCornerPosition(numeric_limits<unsigned int>::max(), numeric_limits<unsigned int>::max());
        if (wireToolStart == tileCursor) {
            wireToolVerticalFirst = abs(newTileCursor.x - tileCursor.x) <= abs(newTileCursor.y - tileCursor.y);
        }
        if (wireToolVerticalFirst) {
            if (abs(newTileCursor.x - wireToolStart.x) > 0) {
                wireHorizontalBoardPtr->newBoard(Vector2u(abs(newTileCursor.x - wireToolStart.x), 1), "", true);
                wireHorizontalPosition = Vector2u(min(newTileCursor.x, wireToolStart.x + 1), newTileCursor.y);
                wireHorizontalBoardPtr->setPosition(static_cast<float>(wireHorizontalPosition.x * Board::getTileSize().x), static_cast<float>(wireHorizontalPosition.y * Board::getTileSize().y));
            } else {
                wireHorizontalBoardPtr->clear();
            }
            if (abs(newTileCursor.y - wireToolStart.y) > 0) {
                wireVerticalBoardPtr->newBoard(Vector2u(1, abs(newTileCursor.y - wireToolStart.y)), "", true);
                wireVerticalPosition = Vector2u(wireToolStart.x, min(newTileCursor.y, wireToolStart.y + 1));
                wireVerticalBoardPtr->setPosition(static_cast<float>(wireVerticalPosition.x * Board::getTileSize().x), static_cast<float>(wireVerticalPosition.y * Board::getTileSize().y));
                
                if (newTileCursor.x != wireToolStart.x) {
                    verticalCornerPosition = Vector2u(0, (newTileCursor.y < wireToolStart.y) ? 0 : (wireVerticalBoardPtr->getSize().y - 1));
                    if (newTileCursor.x < wireToolStart.x) {
                        wireVerticalBoardPtr->setTile(verticalCornerPosition, new TileWire(wireVerticalBoardPtr, verticalCornerPosition, true, (newTileCursor.y < wireToolStart.y) ? SOUTH : WEST, TileWire::CORNER));
                    } else {
                        wireVerticalBoardPtr->setTile(verticalCornerPosition, new TileWire(wireVerticalBoardPtr, verticalCornerPosition, true, (newTileCursor.y < wireToolStart.y) ? EAST : NORTH, TileWire::CORNER));
                    }
                }
            } else {
                wireVerticalBoardPtr->clear();
            }
        } else {
            if (abs(newTileCursor.y - wireToolStart.y) > 0) {
                wireVerticalBoardPtr->newBoard(Vector2u(1, abs(newTileCursor.y - wireToolStart.y)), "", true);
                wireVerticalPosition = Vector2u(newTileCursor.x, min(newTileCursor.y, wireToolStart.y + 1));
                wireVerticalBoardPtr->setPosition(static_cast<float>(wireVerticalPosition.x * Board::getTileSize().x), static_cast<float>(wireVerticalPosition.y * Board::getTileSize().y));
            } else {
                wireVerticalBoardPtr->clear();
            }
            if (abs(newTileCursor.x - wireToolStart.x) > 0) {
                wireHorizontalBoardPtr->newBoard(Vector2u(abs(newTileCursor.x - wireToolStart.x), 1), "", true);
                wireHorizontalPosition = Vector2u(min(newTileCursor.x, wireToolStart.x + 1), wireToolStart.y);
                wireHorizontalBoardPtr->setPosition(static_cast<float>(wireHorizontalPosition.x * Board::getTileSize().x), static_cast<float>(wireHorizontalPosition.y * Board::getTileSize().y));
                
                if (newTileCursor.y != wireToolStart.y) {
                    horizontalCornerPosition = Vector2u((newTileCursor.x < wireToolStart.x) ? 0 : (wireHorizontalBoardPtr->getSize().x - 1), 0);
                    if (newTileCursor.x < wireToolStart.x) {
                        wireHorizontalBoardPtr->setTile(horizontalCornerPosition, new TileWire(wireHorizontalBoardPtr, horizontalCornerPosition, true, (newTileCursor.y < wireToolStart.y) ? NORTH : EAST, TileWire::CORNER));
                    } else {
                        wireHorizontalBoardPtr->setTile(horizontalCornerPosition, new TileWire(wireHorizontalBoardPtr, horizontalCornerPosition, true, (newTileCursor.y < wireToolStart.y) ? WEST : SOUTH, TileWire::CORNER));
                    }
                }
            } else {
                wireHorizontalBoardPtr->clear();
            }
        }
        for (unsigned int x = 0; x < wireHorizontalBoardPtr->getSize().x; ++x) {
            if (x != horizontalCornerPosition.x) {
                const Tile* belowTile = boardPtr->getTile(wireHorizontalPosition + Vector2u(x, 0));
                if (typeid(*belowTile) == typeid(TileWire) && ((static_cast<const TileWire*>(belowTile)->getType() == TileWire::STRAIGHT) && (belowTile->getDirection() % 2 == 0) || static_cast<const TileWire*>(belowTile)->getType() >= TileWire::JUNCTION)) {
                    wireHorizontalBoardPtr->setTile(Vector2u(x, 0), new TileWire(wireHorizontalBoardPtr, Vector2u(x, 0), true, EAST, TileWire::CROSSOVER));
                } else {
                    wireHorizontalBoardPtr->setTile(Vector2u(x, 0), new TileWire(wireHorizontalBoardPtr, Vector2u(x, 0), true, EAST, TileWire::STRAIGHT));
                }
            }
        }
        for (unsigned int y = 0; y < wireVerticalBoardPtr->getSize().y; ++y) {
            if (y != verticalCornerPosition.y) {
                const Tile* belowTile = boardPtr->getTile(wireVerticalPosition + Vector2u(0, y));
                if (typeid(*belowTile) == typeid(TileWire) && ((static_cast<const TileWire*>(belowTile)->getType() == TileWire::STRAIGHT) && (belowTile->getDirection() % 2 == 1) || static_cast<const TileWire*>(belowTile)->getType() >= TileWire::JUNCTION)) {
                    wireVerticalBoardPtr->setTile(Vector2u(0, y), new TileWire(wireVerticalBoardPtr, Vector2u(0, y), true, NORTH, TileWire::CROSSOVER));
                } else {
                    wireVerticalBoardPtr->setTile(Vector2u(0, y), new TileWire(wireVerticalBoardPtr, Vector2u(0, y), true, NORTH, TileWire::STRAIGHT));
                }
            }
        }
        wireHorizontalBoardPtr->highlightArea(IntRect(0, 0, wireHorizontalBoardPtr->getSize().x, wireHorizontalBoardPtr->getSize().y), true);
        wireVerticalBoardPtr->highlightArea(IntRect(0, 0, wireVerticalBoardPtr->getSize().x, wireVerticalBoardPtr->getSize().y), true);
        wireToolLabelPtr->setString("(" + to_string(wireHorizontalBoardPtr->getSize().x) + ", " + to_string(wireVerticalBoardPtr->getSize().y) + ")");
        wireToolLabelPtr->setPosition(static_cast<float>((newTileCursor.x + 1) * Board::getTileSize().x), static_cast<float>((newTileCursor.y + 1) * Board::getTileSize().y));
    }
}