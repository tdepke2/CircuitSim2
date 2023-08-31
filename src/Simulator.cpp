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
#include <portable-file-dialogs.h>
#include <stdexcept>
#include <thread>
#include <typeinfo>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

const unsigned int Simulator::FRAMERATE_LIMIT = 60;
const Vector2u Simulator::INITIAL_WINDOW_SIZE(1300, 900);
Simulator::Configuration Simulator::config;
atomic<Simulator::State> Simulator::state(State::Uninitialized);
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
string Simulator::directoryPath;
Direction Simulator::currentTileDirection = NORTH;
bool Simulator::editMode = true, Simulator::copyBufferVisible = false, Simulator::wireToolVerticalFirst = true;
Vector2i Simulator::mouseStart(0, 0), Simulator::tileCursor(-1, -1), Simulator::selectionStart(-1, -1), Simulator::wireToolStart(-1, -1);
Vector2u Simulator::wireVerticalPosition(0, 0), Simulator::wireHorizontalPosition(0, 0);
IntRect Simulator::selectionArea(0, 0, 0, 0);
Tile* Simulator::relabelTargetTile = nullptr;

const Simulator::Configuration& Simulator::getConfig() {
    return config;
}

const Vector2u Simulator::getWindowSize() {
    return windowPtr->getSize();
}

const UserInterface* Simulator::getUserInterface() {
    return userInterfacePtr;
}

// Gets the cwd of current process. There is a better option in c++17 to do
// this, but this simple implementation should work in most cases.
string getWorkingDirectory() {
    char path[260];
    #ifdef _WIN32
        GetCurrentDirectory(sizeof(path), path);
    #else
        getcwd(path, sizeof(path));
    #endif
    return string(path);
}

int Simulator::start() {
    cout << "Initializing setup..." << endl;
    int exitCode = 0;
    thread renderThread;
    try {
        renderMutex.lock();
        assert(state == State::Uninitialized);
        state = State::Running;
        #ifdef _WIN32
            SetConsoleCtrlHandler(NULL, TRUE);    // Disable control signals (like Ctrl + C) in log window in case the user tries to close application this way.
            signal(SIGBREAK, terminationHandler);    // If log window closed, need to clean up in the few milliseconds available before program goes down.
        #endif
        mainRNG.seed(static_cast<unsigned long>(chrono::high_resolution_clock::now().time_since_epoch().count()));
        windowPtr = new RenderWindow(VideoMode(INITIAL_WINDOW_SIZE.x, INITIAL_WINDOW_SIZE.y), "[CircuitSim2] Loading...", Style::Default, ContextSettings(0, 0, 4));
        windowPtr->setFramerateLimit(FRAMERATE_LIMIT);
        windowPtr->setActive(false);
        renderThread = thread(renderLoop);

        if (!pfd::settings::available()) {
            // FIXME: we should have a fallback implementation when pfd is not supported.
            throw runtime_error("Portable File Dialogs are not available on this platform.");
        }
        pfd::settings::verbose(true);
        
        directoryPath = getWorkingDirectory();    // Load file resources and create boards.
        Board::loadTextures("resources/texturePackGrid.png", "resources/texturePackNoGrid.png", Vector2u(32, 32));
        Image appIcon;
        if (!appIcon.loadFromFile("resources/icon.png")) {
            throw runtime_error("\"resources/icon.png\": Unable to load texture file.");
        }
        windowPtr->setIcon(appIcon.getSize().x, appIcon.getSize().y, appIcon.getPixelsPtr());
        Board::loadFont("resources/consolas.ttf");
        Board::newBoardDefaultPath = directoryPath + pfd::path::separator() + "boards" + pfd::path::separator() + "NewBoard.txt";
        boardPtr = new Board();
        currentTileBoardPtr = new Board();
        copyBufferBoardPtr = new Board();
        wireVerticalBoardPtr = new Board();
        wireHorizontalBoardPtr = new Board();
        userInterfacePtr = new UserInterface();
        
        openConfig(directoryPath + "/resources/config.ini", false);    // Load the configuration file.
        Tile::currentUpdateTime = 1;
        boardPtr->newBoard();
        Board::enableExtraLogicStates = config.triStateLogicDefault;
        copyBufferBoardPtr->newBoard(Vector2u(1, 1), "");
        copyBufferBoardPtr->highlightArea(IntRect(0, 0, copyBufferBoardPtr->getSize().x, copyBufferBoardPtr->getSize().y), true);
        wireToolLabelPtr = new Text("", Board::getFont(), 30);
        wireToolLabelPtr->setFillColor(Color::White);
        wireToolLabelPtr->setOutlineColor(Color::Black);
        wireToolLabelPtr->setOutlineThickness(1.0f);
        viewOption(3);
        Clock perSecondClock, loopClock, tickClock;    // The perSecondClock counts FPS and TPS, loopClock limits loops per second, tickClock manages tick speed.
        renderMutex.unlock();
        
        cout << "Setup complete." << endl;
        while (state != State::Exiting) {
            renderReadyMutex.lock();    // Two mutex system helps prevent starvation of render thread and main thread (a binary semaphore could work even better though).
            renderMutex.lock();
            renderReadyMutex.unlock();
            processEvents();
            if (UserInterface::fieldToSelectPtr != nullptr) {    // If dialog box with text field is opening up, the field gets selected after events have been processed.
                UserInterface::fieldToSelectPtr->selected = true;
                UserInterface::fieldToSelectPtr = nullptr;
            }
            userInterfacePtr->tickCounter.text.setString(" Update counter: " + to_string(Tile::currentUpdateTime - 1));
            UserInterface::updateMessages();
            
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
                
                if (simSpeed == SimSpeed::Slow && tickClock.getElapsedTime().asSeconds() >= 1.0f / config.slowTPSLimit) {
                    nextTick();
                    tickClock.restart();
                } else if (simSpeed == SimSpeed::Medium && tickClock.getElapsedTime().asSeconds() >= 1.0f / config.mediumTPSLimit) {
                    nextTick();
                    tickClock.restart();
                } else if (simSpeed == SimSpeed::Fast && tickClock.getElapsedTime().asSeconds() >= 1.0f / config.fastTPSLimit) {
                    nextTick();
                    tickClock.restart();
                } else if (simSpeed == SimSpeed::Extreme) {
                    nextTick();
                }
            }
            
            if (perSecondClock.getElapsedTime().asSeconds() >= 1.0f) {    // Calculate FPS and TPS.
                string changesMadeIndicator = boardPtr->changesMade ? "*" : "";
                windowPtr->setTitle("[CircuitSim2] [" + changesMadeIndicator + boardPtr->name + "] [Size: " + to_string(boardPtr->getSize().x) + " x " + to_string(boardPtr->getSize().y) + "] [FPS: " + to_string(fpsCounter) + ", TPS: " + to_string(tpsCounter) + "]");
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
        UserInterface::pushMessage("Fatal error: " + string(ex.what()), true);
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

string Simulator::decimalToString(float f) {
    string s = to_string(f);
    unsigned int minLength = s.rfind('.') + 2;
    while (s.length() > minLength && s.back() == '0') {
        s.pop_back();
    }
    return s;
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
            Tile::currentUpdateTime = 1;
            boardPtr->newBoard();
            Board::enableExtraLogicStates = config.triStateLogicDefault;
            viewOption(3);
            UserInterface::pushMessage("Created new board with size " + to_string(boardPtr->getSize().x) + " x " + to_string(boardPtr->getSize().y) + ".");
            UserInterface::closeAllDialogPrompts();
        }
    } else if (option == 1 || option == 3) {    // Load board. Save as board.
        if (option == 1 && !userInterfacePtr->savePrompt.visible && boardPtr->changesMade) {
            userInterfacePtr->savePrompt.optionButtons[1].actionOption = 1;
            userInterfacePtr->savePrompt.show();
            return;
        }
        UserInterface::closeAllDialogPrompts();

        // FIXME: the HWND doesn't seem to get passed to pfd even though the current thread has the window context, seems like a bug in pfd. Doesn't matter too much.
        //cout << "Got request to open/save file." << endl;
        //cout << "windowPtr->getSystemHandle() = " << windowPtr->getSystemHandle() << endl;
        //cout << "GetActiveWindow() =            " << GetActiveWindow() << endl;

        try {
            if (option == 1) {
                auto openDialog = pfd::open_file("Open Board File", directoryPath + "/boards", {
                    "Plain Text (*.txt)", "*.txt",
                    "All Files (*.*)", "*"
                }, pfd::opt::none).result();
                
                if (!openDialog.empty()) {
                    toolsOption(1);
                    if (tileCursor != Vector2i(-1, -1)) {
                        boardPtr->getTile(tileCursor)->setHighlight(false);
                        tileCursor = Vector2i(-1, -1);
                    }
                    Tile::currentUpdateTime = 1;
                    boardPtr->loadFile(openDialog[0]);
                    viewOption(3);
                } else {
                    cout << "No file selected." << endl;
                }
            } else {
                auto saveDialog = pfd::save_file("Save As Board File", boardPtr->name + ".txt", {    // FIXME: it seems like the board should have name and file path attributes instead of doing this.
                    "Plain Text (*.txt)", "*.txt",
                    "All Files (*.*)", "*"
                }, pfd::opt::none).result();
                
                if (!saveDialog.empty()) {
                    boardPtr->saveFile(saveDialog);
                } else {
                    cout << "No file selected." << endl;
                }
            }
        } catch (exception& ex) {
            viewOption(3);
            UserInterface::pushMessage("Error: Exception in file access. " + string(ex.what()), true);
        }
    } else if (option == 2) {    // Save board.
        try {
            boardPtr->saveFile(boardPtr->name + ".txt");
        } catch (exception& ex) {
            UserInterface::pushMessage("Error: Failed to save board. " + string(ex.what()), true);
        }
        UserInterface::closeAllDialogPrompts();
    } else if (option == 4) {    // Rename board.
        if (!userInterfacePtr->renamePrompt.visible) {
            userInterfacePtr->renamePrompt.optionFields[0].setString(boardPtr->name.substr(boardPtr->name.rfind(pfd::path::separator()[0]) + 1));
            userInterfacePtr->renamePrompt.show();
        } else {
            try {
                string newPath, newName = userInterfacePtr->renamePrompt.optionFields[0].field.getString().toAnsiString();
                if (newName.find('/') != string::npos || newName.find('\\') != string::npos) {
                    throw runtime_error("Name cannot use slash or backslash, use \"Save As...\" to change board path.");
                }
                size_t lastSlashPosition = boardPtr->name.rfind(pfd::path::separator()[0]);
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
                UserInterface::pushMessage("Board renamed to: \"" + newName + "\".");
                UserInterface::closeAllDialogPrompts();
            } catch (exception& ex) {
                UserInterface::pushMessage("Error: Unable to rename the board. " + string(ex.what()), true);
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
                UserInterface::pushMessage("Error: Failed to resize the board. " + string(ex.what()), true);
            }
        }
    } else if (option == 6) {    // Configuration.
        if (!userInterfacePtr->configPrompt.visible) {
            userInterfacePtr->configPrompt.optionFields[0].setString(decimalToString(config.slowTPSLimit));    // Update user interface to match configuration.
            userInterfacePtr->configPrompt.optionFields[1].setString(decimalToString(config.mediumTPSLimit));
            userInterfacePtr->configPrompt.optionFields[2].setString(decimalToString(config.fastTPSLimit));
            userInterfacePtr->configPrompt.optionChecks[0].setChecked(config.triStateLogicDefault);
            userInterfacePtr->configPrompt.optionChecks[1].setChecked(Board::enableExtraLogicStates);
            userInterfacePtr->configPrompt.optionChecks[2].setChecked(config.pauseOnConflict);
            userInterfacePtr->configPrompt.show();
        } else {
            try {
                float f = stof(userInterfacePtr->configPrompt.optionFields[0].field.getString().toAnsiString());
                if (f <= 0.0f) {
                    throw runtime_error("Value for slow TPS limit must be greater than zero.");
                }
                config.slowTPSLimit = f;
                f = stof(userInterfacePtr->configPrompt.optionFields[1].field.getString().toAnsiString());
                if (f <= 0.0f) {
                    throw runtime_error("Value for medium TPS limit must be greater than zero.");
                }
                config.mediumTPSLimit = f;
                f = stof(userInterfacePtr->configPrompt.optionFields[2].field.getString().toAnsiString());
                if (f <= 0.0f) {
                    throw runtime_error("Value for fast TPS limit must be greater than zero.");
                }
                config.fastTPSLimit = f;
                config.triStateLogicDefault = userInterfacePtr->configPrompt.optionChecks[0].isChecked();
                if (Board::enableExtraLogicStates != userInterfacePtr->configPrompt.optionChecks[1].isChecked()) {
                    boardPtr->changesMade = true;
                    Board::enableExtraLogicStates = userInterfacePtr->configPrompt.optionChecks[1].isChecked();
                }
                config.pauseOnConflict = userInterfacePtr->configPrompt.optionChecks[2].isChecked();
                openConfig(directoryPath + "/resources/config.ini", true);
                UserInterface::closeAllDialogPrompts();
            } catch (exception& ex) {
                UserInterface::pushMessage("Error: " + string(ex.what()), true);
            }
        }
    } else if (option == 7) {    // Exit program.
        if (!userInterfacePtr->savePrompt.visible && boardPtr->changesMade) {
            userInterfacePtr->savePrompt.optionButtons[1].actionOption = 7;
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
        updateSimSpeed(SimSpeed::Paused);
    } else if (option == 1) {    // Change max TPS.
        if (simSpeed == SimSpeed::Paused) {
            updateSimSpeed(SimSpeed::Slow);
        } else if (simSpeed == SimSpeed::Slow) {
            updateSimSpeed(SimSpeed::Medium);
        } else if (simSpeed == SimSpeed::Medium) {
            updateSimSpeed(SimSpeed::Fast);
        } else if (simSpeed == SimSpeed::Fast) {
            updateSimSpeed(SimSpeed::Extreme);
        } else {
            updateSimSpeed(SimSpeed::Paused);
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
    } else if (option == 13) {    // Selection query tool.
        userInterfacePtr->queryPrompt.optionFields[0].setString("");    // Clear previous fields.
        userInterfacePtr->queryPrompt.optionFields[1].setString("");
        userInterfacePtr->queryPrompt.optionFields[2].setString("");
        userInterfacePtr->queryPrompt.optionFields[3].setString("");
        userInterfacePtr->queryPrompt.optionFields[4].setString("");
        
        if (selectionArea.width != 1 && selectionArea.height != 1) {
            userInterfacePtr->queryPrompt.optionFields[0].setString("Size must be 1-wide only.");
        } else {
            vector<bool> binaryValue;
            binaryValue.reserve(32);
            bool foundError = false;
            
            int index, indexLast;
            Direction dirPerpendicular;
            if (selectionArea.width == 1) {    // Selection is along y-axis.
                index = selectionArea.top + selectionArea.height - 1;
                indexLast = selectionArea.top;
                dirPerpendicular = EAST;
            } else {    // Selection is along x-axis.
                index = selectionArea.left + selectionArea.width - 1;
                indexLast = selectionArea.left;
                dirPerpendicular = NORTH;
            }
            while (index >= indexLast) {    // Scan selection to get bits.
                const Tile* t;
                if (selectionArea.width == 1) {
                    t = boardPtr->getTile(Vector2u(selectionArea.left, index));
                } else {
                    t = boardPtr->getTile(Vector2u(index, selectionArea.top));
                }
                if (typeid(*t) != typeid(Tile)) {
                    ::State tileState = DISCONNECTED;
                    if (!userInterfacePtr->queryPrompt.optionChecks[1].isChecked()) {
                        if (typeid(*t) == typeid(TileWire)) {
                            if (t->checkOutput(dirPerpendicular) == t->checkOutput(static_cast<Direction>(dirPerpendicular + 2))) {
                                tileState = t->checkOutput(dirPerpendicular);
                            }
                        } else {
                            tileState = t->getState();
                        }
                    } else if (typeid(*t) == typeid(TileLED)) {
                        tileState = t->getState();
                    }
                    if (tileState == LOW) {
                        binaryValue.push_back(false);
                    } else if (tileState == HIGH) {
                        binaryValue.push_back(true);
                    } else if (tileState != DISCONNECTED) {
                        binaryValue.push_back(false);
                        foundError = true;
                    }
                }
                --index;
            }
            userInterfacePtr->queryPrompt.optionFields[0].setString(to_string(selectionArea.width == 1 ? selectionArea.height : selectionArea.width) + " tiles (" + to_string(binaryValue.size()) + " bits)");
            
            if (foundError || binaryValue.size() > 32) {    // Report that value could not be determined (usually because of a tri-state element).
                userInterfacePtr->queryPrompt.optionFields[1].setString("???");
                userInterfacePtr->queryPrompt.optionFields[2].setString("???");
                userInterfacePtr->queryPrompt.optionFields[3].setString("???");
                userInterfacePtr->queryPrompt.optionFields[4].setString("???");
            } else if (binaryValue.size() > 0) {    // Convert binary value to other possible formats.
                if (!userInterfacePtr->queryPrompt.optionChecks[0].isChecked()) {    // If using different direction convention, swap the bits around.
                    for (int i = 0; i < static_cast<int>(binaryValue.size() / 2); ++i) {
                        swap(binaryValue[i], binaryValue[binaryValue.size() - 1 - i]);
                    }
                }
                
                string strBinary = "0b", strHexadecimal = "0x";
                int hexDigit = 0;
                uint32_t unsignedValue = 0;
                int32_t signedValue = 0;
                for (int i = static_cast<int>(binaryValue.size() - 1); i >= 0; --i) {
                    hexDigit <<= 1;
                    unsignedValue <<= 1;
                    if (binaryValue[i]) {
                        strBinary.push_back('1');
                        hexDigit += 1;
                        unsignedValue += 1;
                    } else {
                        strBinary.push_back('0');
                    }
                    
                    if (i % 4 == 0) {
                        if (hexDigit < 10) {
                            strHexadecimal.push_back('0' + hexDigit);
                        } else {
                            strHexadecimal.push_back('A' + hexDigit - 10);
                        }
                        hexDigit = 0;
                    }
                }
                if (binaryValue[binaryValue.size() - 1]) {
                    signedValue = unsignedValue - (static_cast<int64_t>(1) << binaryValue.size());
                } else {
                    signedValue = unsignedValue;
                }
                userInterfacePtr->queryPrompt.optionFields[1].setString(strBinary);
                userInterfacePtr->queryPrompt.optionFields[2].setString(strHexadecimal);
                userInterfacePtr->queryPrompt.optionFields[3].setString(to_string(unsignedValue));
                userInterfacePtr->queryPrompt.optionFields[4].setString(to_string(signedValue));
            }
        }
        userInterfacePtr->queryPrompt.show();
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
        UserInterface::pushMessage("Error: No label entered.", true);
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

void Simulator::processEvents() {
    Event event;
    while (windowPtr->pollEvent(event)) {    // Process all queued events.
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
            userInterfacePtr->update(event.size);
        } else if (event.type == Event::Closed) {
            fileOption(7);
        }
    }
}

void Simulator::openConfig(const string& filename, bool saveData) {
    if (!saveData) {
        cout << "Loading configuration..." << endl;
    }
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        throw runtime_error("\"" + filename + "\": Unable to open configuration file.");
    }
    
    string line, fileData;    // Parse the configuration file, expects the file format to conform to the INI style. The fileData string stores the entire file.
    int lineNumber = 0, numEntries = 0;
    try {
        while (getline(inputFile, line)) {
            ++lineNumber;
            if (line.length() != 0 && line.find(';') == string::npos) {
                string option = "", value = "";    // Parse "option = value" from the current line.
                unsigned int i = 0;
                while (i < line.length() && line[i] == ' ') {
                    ++i;
                }
                while (i < line.length() && line[i] != ' ' && line[i] != '=') {
                    option.push_back(line[i]);
                    ++i;
                }
                while (i < line.length() && line[i] == ' ') {
                    ++i;
                }
                ++i;
                while (i < line.length()) {
                    value.push_back(line[i]);
                    ++i;
                }
                
                if (numEntries == 0 && line == "[settings]") {
                    ++numEntries;
                } else if (numEntries == 1) {
                    if (!saveData) {
                        if (option == "slow_tps_limit") {    // Set configuration values from the file.
                            float valueAsFloat = stof(value);
                            if (valueAsFloat <= 0.0f) {
                                throw runtime_error("Value for \"slow_tps_limit\" must be greater than zero.");
                            }
                            config.slowTPSLimit = valueAsFloat;
                        } else if (option == "medium_tps_limit") {
                            float valueAsFloat = stof(value);
                            if (valueAsFloat <= 0.0f) {
                                throw runtime_error("Value for \"medium_tps_limit\" must be greater than zero.");
                            }
                            config.mediumTPSLimit = valueAsFloat;
                        } else if (option == "fast_tps_limit") {
                            float valueAsFloat = stof(value);
                            if (valueAsFloat <= 0.0f) {
                                throw runtime_error("Value for \"fast_tps_limit\" must be greater than zero.");
                            }
                            config.fastTPSLimit = valueAsFloat;
                        } else if (option == "tri-state_logic_default") {
                            config.triStateLogicDefault = static_cast<bool>(stoi(value));
                        } else if (option == "pause_on_conflict") {
                            config.pauseOnConflict = static_cast<bool>(stoi(value));
                        } else {
                            throw runtime_error("Invalid configuration file data.");
                        }
                    } else {
                        line = option + " = ";
                        if (option == "slow_tps_limit") {    // Modify line to match current configuration values.
                            line += decimalToString(config.slowTPSLimit);
                        } else if (option == "medium_tps_limit") {
                            line += decimalToString(config.mediumTPSLimit);
                        } else if (option == "fast_tps_limit") {
                            line += decimalToString(config.fastTPSLimit);
                        } else if (option == "tri-state_logic_default") {
                            line += to_string(config.triStateLogicDefault);
                        } else if (option == "pause_on_conflict") {
                            line += to_string(config.pauseOnConflict);
                        } else {
                            throw runtime_error("Invalid configuration file data.");
                        }
                    }
                } else {
                    throw runtime_error("Invalid configuration file data.");
                }
            }
            fileData += line + "\n";
        }
        if (numEntries != 1) {
            throw runtime_error("Missing data, end of file reached.");
        }
    } catch (exception& ex) {
        inputFile.close();
        throw runtime_error("\"" + filename + "\" at line " + to_string(lineNumber) + ": " + ex.what());
    }
    inputFile.close();
    
    if (saveData) {    // Save the configuration file.
        ofstream outputFile(filename);
        if (!outputFile.is_open()) {
            throw runtime_error("\"" + filename + "\": Unable to open configuration file.");
        }
        outputFile << fileData;
        outputFile.close();
    }
    updateSimSpeed(simSpeed);    // Update the simSpeed indicator.
}

void Simulator::updateSimSpeed(SimSpeed newSpeed) {
    simSpeed = newSpeed;
    if (newSpeed == SimSpeed::Paused) {
        userInterfacePtr->tpsDisplay.text.setString(" Current TPS limit: Paused    ");
        userInterfacePtr->tpsDisplay.button.setFillColor(Color(10, 10, 230));
    } else if (newSpeed == SimSpeed::Slow) {
        userInterfacePtr->tpsDisplay.text.setString(" Current TPS limit: " + decimalToString(config.slowTPSLimit));
        userInterfacePtr->tpsDisplay.button.setFillColor(Color(10, 230, 230));
    } else if (newSpeed == SimSpeed::Medium) {
        userInterfacePtr->tpsDisplay.text.setString(" Current TPS limit: " + decimalToString(config.mediumTPSLimit));
        userInterfacePtr->tpsDisplay.button.setFillColor(Color(10, 230, 10));
    } else if (newSpeed == SimSpeed::Fast) {
        userInterfacePtr->tpsDisplay.text.setString(" Current TPS limit: " + decimalToString(config.fastTPSLimit));
        userInterfacePtr->tpsDisplay.button.setFillColor(Color(230, 230, 10));
    } else {
        userInterfacePtr->tpsDisplay.text.setString(" Current TPS limit: Unlimited ");
        userInterfacePtr->tpsDisplay.button.setFillColor(Color(230, 10, 10));
    }
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
        
        windowPtr->clear();    // Clear and redraw all of the visible elements.
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
    if (Board::numStateErrors > 0) {    // Check for state errors in the last tick.
        if (Board::numStateErrors > 10) {
            cout << "(and " << Board::numStateErrors - 10 << " more...)" << endl;
        }
        UserInterface::pushMessage("Detected " + to_string(Board::numStateErrors) + " total conflict(s). Simulation has been paused.", true);
        Board::numStateErrors = 0;
        updateSimSpeed(SimSpeed::Paused);
    }
}

void Simulator::handleKeyPress(Event::KeyEvent keyEvent) {
    if (keyEvent.control) {    // Most key bindings are listed in the user interface dropdown menus.
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
            } else if (keyEvent.code == Keyboard::Q) {
                toolsOption(13);
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
    if (!selectionArea.contains(tileCursor)) {    // If not pasting into a selection, just copy the tiles to be pasted onto the board at the position.
        if (currentTileBoardPtr->getSize() == Vector2u(1, 1)) {
            boardPtr->replaceTile(currentTileBoardPtr->getTile(Vector2u(0, 0))->clone(boardPtr, Vector2u(tileCursor)));
        } else if (copyBufferVisible) {
            IntRect pasteArea(0, 0, copyBufferBoardPtr->getSize().x, copyBufferBoardPtr->getSize().y);
            if (tileCursor.x + pasteArea.width > static_cast<int>(boardPtr->getSize().x)) {    // Check if paste area extends off the board. If it does, crop paste area if forcePaste is enabled.
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
            if (!forcePaste) {    // If not forcePaste and the paste area is not blank, fail the paste.
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
        boardPtr->highlightArea(selectionArea, false);    // Remove highlight for any active selection.
        selectionStart = Vector2i(-1, -1);
        selectionArea = IntRect(0, 0, 0, 0);
        if (tileCursor != Vector2i(-1, -1)) {
            boardPtr->getTile(tileCursor)->setHighlight(true);
        }
    } else {    // Pasting into selection, need to tile the paste to fill the selection area.
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