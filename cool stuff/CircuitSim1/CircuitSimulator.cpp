/*
Circuit Simulator. Create digital logic circuits and test them.

Notes and stuff:

 Cross-compatible for Windows and Linux. Compile this program with C++ 11.

 Todo:
  * Bring back duplicate switches/buttons.
  * Test if its possible to print special chars in cmd.     https://stackoverflow.com/questions/31083573/c-printing-special-ascii-characters-to-the-windows-console
  * Figure out a way to categorize objects so that adding new objects in the future won't cause problems.
  * Button/switch/gate logic in followWire () might still be flawed.
  * Remove development commands for finished version.

 Commands (arguments in <angle brackets> mean "fill in this field", arguments in [brackets] are optional):
  * <row> <column> <ID number> [<switch or button ID>]       | Places an object on the board.
  * run/r [<number of iterations>] [no-print/np]             | Simulate a number of frames, no-print disables screen updates until finished.
  * switch/s <switch ID>                                     | Toggles a switch with switch ID.
  * button/b <button ID>                                     | Activates a button with button ID.
  * resize <new width> <new height>                          | Resizes the board, preserves data currently on the board.
  * new <width> <height>                                     | Creates a new board.
  * copy/c <row 1> <column 1> <row 2> <column 2>             | Copies the selected area to a buffer.
  * paste/p <row> <column>                                   | Pastes data in the buffer using the row and column as the top left corner.
  * fill/f <row 1> <column 1> <row 2> <column 2> <ID number> | Fills the specified area with objects.
  * rename <new board name>                                  | Renames the current board.
  * save [<new board name>]                                  | Saves the current board to a file.
  * load [<board name>]                                      | Loads a board from a file. Excluding the board name lists available boards.
  * reset                                                    | Deactivates all objects on the board to fix half-powered wires.
  * view/v [resize] <width/row> <height/column>              | Changes the view to show only a part of the board, useful for smaller screens.
  * trim <extra width> <extra height>                        | Crops the board and adds a border of extra space, does not delete objects.
  * table/t                                                  | Prints a table showing the ID numbers of objects.
  * help/h                                                   | Displays command help.
  * exit                                                     | Exits program.

 Data structures:
  * Array full of symbols (strings) for each game object. This will be used for graphics drawing and board loading.
  * Array full of directions (four booleans per object) used to show which way wires connect. This will be used for the simulation algorithm.
  * Dynamic array full of numbers (shorts) to represent each object on the board.
  * Dynamic array full of shorts to store object data for the copy buffer.
  * Vector of strings for storing the separate arguments in a user-entered command.
  * Vector for all input and logic objects. Stores the position, checked status, and symbol (for input) for all input and logic objects. This will be used for update checking.
  * Vector for storing switch/button data when copying a region.
  * Vector for storing positions of objects that need to be turned off when following the path of a wire that is turning off.

 Simulation algorithm:
  1. Update states of all attribute objects (input and logic). Gates will look at adjacent wire/input/other gates pointing towards them.
   A. Each object that changed will be marked for next step (the flag will be set to changed).
  2. For each changed object, wire algorithm will follow path and update wire states as it goes (good idea to use function recursion for this). Start with all the changed objects that are off, check the ones that are on later.
   A. If the changed object is off (it is turning on in most cases)...
    1. Activate wires that are off, stop subpath if an active wire is encountered (this would indicate a loop in wire). If the output of a changed gate is found, change the gate state and set the gate to unchanged.
   B. If the changed object is on (it is turning off in most cases)...
    1. Go through wire first to check for unchanged active control objects, add wire positions to a vector. Stop subpath if an inactive wire is found or a junction wire is found that is already in the wire positions vector. As before, update changed gates encountered that have an output connected to wire.
    2. If no unchanged active control objects were found, go through wire positions vector and change wires.

 Old idea (where diodes are counted as wires):
  2. For each marked gate, wire algorithm will follow path and mark nodes at intersections (good idea to use function recursion for this).
   A. If the wire is turning on...
    1. End path before reverse diodes, unmark any other marked gates/output that are found.
   B. If the wire is turning off...
    1. End path at forward diodes, unmark any other marked gates/output that are found.
   C. Lists for positions of wires and junctions, crossovers and diodes are wire.

 Object lookup table:
  [Name]                | [ID] | [Symbol] | [Name]               | [ID] | [Symbol]
  Empty space           |  0   | "  "     |                      |      |
  Wire vertical off     |  1   | "│ "     | Wire vertical on     |  2   | "║ "
  Wire horizontal off   |  3   | "──"     | Wire horizontal on   |  4   | "══"
  Wire up right off     |  5   | "└─"     | Wire up right on     |  6   | "╚═"
  Wire down right off   |  7   | "┌─"     | Wire down right on   |  8   | "╔═"
  Wire down left off    |  9   | "┐ "     | Wire down left on    | 10   | "╗ "
  Wire up left off      | 11   | "┘ "     | Wire up left on      | 12   | "╝ "
  Wire vert. right off  | 13   | "├─"     | Wire vert. right on  | 14   | "╠═"
  Wire horiz. down off  | 15   | "┬─"     | Wire horiz. down on  | 16   | "╦═"
  Wire vert. left off   | 17   | "┤ "     | Wire vert. left on   | 18   | "╣ "
  Wire horiz. up off    | 19   | "┴─"     | Wire horiz. up on    | 20   | "╩═"
  Wire junction off     | 21   | "┼─"     | Wire junction on     | 22   | "╬═"
  Wire crossover off    | 23   | "│─"     | Wire cross. vert. on | 24   | "║─"
  Wire cross. horiz. on | 25   | "│═"     | Wire cross. both on  | 26   | "║═"
  Switch off            | 27   | "s#"     | Switch on            | 28   | "S#"    # = ID
  Button off            | 29   | "t#"     | Button on            | 30   | "T#"
  Lamp off              | 31   | "░░"     | Lamp on              | 32   | "██"
  Diode up off          | 33   | "^d"     | Diode up on          | 34   | "^D"
  Diode right off       | 35   | ">d"     | Diode right on       | 36   | ">D"
  Diode down off        | 37   | "vd"     | Diode down on        | 38   | "vD"
  Diode left off        | 39   | "<d"     | Diode left on        | 40   | "<D"
  NOT gate up off       | 41   | "^n"     | NOT gate up on       | 42   | "^N"
  NOT gate right off    | 43   | ">n"     | NOT gate right on    | 44   | ">N"
  NOT gate down off     | 45   | "vn"     | NOT gate down on     | 46   | "vN"
  NOT gate left off     | 47   | "<n"     | NOT gate left on     | 48   | "<N"
  AND gate up off       | 49   | "^a"     | AND gate up on       | 50   | "^A"
  AND gate right off    | 51   | ">a"     | AND gate right on    | 52   | ">A"
  AND gate down off     | 53   | "va"     | AND gate down on     | 54   | "vA"
  AND gate left off     | 55   | "<a"     | AND gate left on     | 56   | "<A"
  OR gate up off        | 57   | "^o"     | OR gate up on        | 58   | "^O"
  OR gate right off     | 59   | ">o"     | OR gate right on     | 60   | ">O"
  OR gate down off      | 61   | "vo"     | OR gate down on      | 62   | "vO"
  OR gate left off      | 63   | "<o"     | OR gate left on      | 64   | "<O"
  XOR gate up off       | 65   | "^x"     | XOR gate up on       | 66   | "^X"
  XOR gate right off    | 67   | ">x"     | XOR gate right on    | 68   | ">X"
  XOR gate down off     | 69   | "vx"     | XOR gate down on     | 70   | "vX"
  XOR gate left off     | 71   | "<x"     | XOR gate left on     | 72   | "<X"
  NAND gate up off      | 73   | "^b"     | NAND gate up on      | 74   | "^B"
  NAND gate right off   | 75   | ">b"     | NAND gate right on   | 76   | ">B"
  NAND gate down off    | 77   | "vb"     | NAND gate down on    | 78   | "vB"
  NAND gate left off    | 79   | "<b"     | NAND gate left on    | 80   | "<B"
  NOR gate up off       | 81   | "^p"     | NOR gate up on       | 82   | "^P"
  NOR gate right off    | 83   | ">p"     | NOR gate right on    | 84   | ">P"
  NOR gate down off     | 85   | "vp"     | NOR gate down on     | 86   | "vP"
  NOR gate left off     | 87   | "<p"     | NOR gate left on     | 88   | "<P"
  XNOR gate up off      | 89   | "^y"     | XNOR gate up on      | 90   | "^Y"
  XNOR gate right off   | 91   | ">y"     | XNOR gate right on   | 92   | ">Y"
  XNOR gate down off    | 93   | "vy"     | XNOR gate down on    | 94   | "vY"
  XNOR gate left off    | 95   | "<y"     | XNOR gate left on    | 96   | "<Y"

 Conjectures made from object ID's:
  * An object is only "on" if ID != 0 and ID % 2 == 0    (exception for crossover wire).
  * An object connects no where if ID == 0.
  * An object connects depending on CONNECTION_INFO [(ID - 1) / 2] if ID >= 1 && ID <= 20.
  * An object connects four ways if ID >= 21    (slight exception for crossover wire).
  * An object's output is in the ((ID - 1) / 2) % 4 direction if ID >= 33.
*/

#include <cstdlib>     // For system ().
#include <iostream>    // For cout, cin.
#include <iomanip>     // For setw (), setfill ().
#include <cmath>       // For pow () and ceil ().
#include <sstream>     // For stringstream ().
#include <vector>      // For vector data type.
#include <fstream>     // For file input/output.
#include <thread>      // For this_thread::sleep_for ().
#include <chrono>      // For chrono::milliseconds ().
#include <string>      // For various string functions.
#include <dirent.h>    // For listing directory contents.

#define BOARD_DIR_SAVE    BOARD_DIR_WIN     // Saving type and location.
#define SYMBOL_INFO_SAVE  SYMBOL_INFO_WIN

#define BOARD_DIR_LOAD    BOARD_DIR_UNIX     // Loading type and location.
#define SYMBOL_INFO_LOAD  SYMBOL_INFO_UNIX

#define SYMBOL_INFO_DISP  SYMBOL_INFO_UNIX     // Graphics type.

#ifdef __unix__
#define clearScreen cout<<"\033[;H\033[J"

#else
#define clearScreen system("CLS")

#endif

using namespace std;

//// Class Definitions. ////

class ControlType    // Class for input and logic objects. Stores position and update/ID data for each of these objects.
{
    public:
    int row, column;   // Stores object position in objectData.
    char symbol;      // For switches and buttons, stores the ID character.
    bool changed;    // Indicates if the state of the object has changed recently.
    
    ControlType (int r, int c, char s, bool ch);
    void setID (short);
    short ID ();
    bool active ();
};

class ChangedObjectType     // Class for storing data for objects that need to be changed during frame simulation.
{
    public:
    int row, column;    // Stores object position in objectData.
    short newID;       // The objects new ID value.
    
    ChangedObjectType (int r, int c, short n);
    void updateState ();
};

//// Global Variables. ////

short ** objectData = nullptr, ** copyBuffer = nullptr;
int width, height, paddingX, paddingY, objectDataSize = 0, copyBufferHeight = 0, copyBufferWidth = 0, viewWidth = 100, viewHeight = 55, viewRow, viewColumn;
string boardName;

vector <string> subCommands;
vector <ControlType> controlObjects;
vector <ControlType> copyBufferData;
vector <ChangedObjectType> changedObjects;

//// Global Constants. ////

const string BOARD_DIR_UNIX = "Boards (Linux)/";
const string SYMBOL_INFO_UNIX [97] = {    // Textures for each object. Used to convert ID's to symbols and symbols to ID's.
"  ",
"│ ", "║ ", "──", "══",
"└─", "╚═", "┌─", "╔═", "┐ ", "╗ ", "┘ ", "╝ ",
"├─", "╠═", "┬─", "╦═", "┤ ", "╣ ", "┴─", "╩═",
"┼─", "╬═",
"│─", "║─", "│═", "║═",
 "s",  "S",
 "t",  "T",
"░░", "██",
"^d", "^D", ">d", ">D", "vd", "vD", "<d", "<D",
"^n", "^N", ">n", ">N", "vn", "vN", "<n", "<N",
"^a", "^A", ">a", ">A", "va", "vA", "<a", "<A",
"^o", "^O", ">o", ">O", "vo", "vO", "<o", "<O",
"^x", "^X", ">x", ">X", "vx", "vX", "<x", "<X",
"^b", "^B", ">b", ">B", "vb", "vB", "<b", "<B",
"^p", "^P", ">p", ">P", "vp", "vP", "<p", "<P",
"^y", "^Y", ">y", ">Y", "vy", "vY", "<y", "<Y"};

const string BOARD_DIR_WIN = "Boards (Windows)/";
const string SYMBOL_INFO_WIN [97] = {    // Windows CMD textures.
 "  ",
 "| ",  "[ ", "--", "==",
"\'-", "\"=", ",-", ";=", ", ", "; ", "\' ", "\" ",
 ">-",  ">=", "v-", "v=", "< ", "<.",  "^-",  "^=",
 "+-",  "#=",
 "|-",  "[-", "|=", "[=",
  "s",   "S",
  "t",   "T",
 "..",  "##",
 "^d",  "^D", ">d", ">D", "vd", "vD",  "<d",  "<D",
 "^n",  "^N", ">n", ">N", "vn", "vN",  "<n",  "<N",
 "^a",  "^A", ">a", ">A", "va", "vA",  "<a",  "<A",
 "^o",  "^O", ">o", ">O", "vo", "vO",  "<o",  "<O",
 "^x",  "^X", ">x", ">X", "vx", "vX",  "<x",  "<X",
 "^b",  "^B", ">b", ">B", "vb", "vB",  "<b",  "<B",
 "^p",  "^P", ">p", ">P", "vp", "vP",  "<p",  "<P",
 "^y",  "^Y", ">y", ">Y", "vy", "vY",  "<y",  "<Y"};

const bool CONNECTION_INFO [11][4] = {    // Used to convert wire ID to direction data for most wires.
{1, 0, 1, 0}, {0, 1, 0, 1},
{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 1, 1}, {1, 0, 0, 1},
{1, 1, 1, 0}, {0, 1, 1, 1}, {1, 0, 1, 1}, {1, 1, 0, 1},
{1, 1, 1, 1}};

//// Class Member Functions. ////

ControlType::ControlType (int r, int c, char s = '\0', bool ch = false)
{
    row = r;
    column = c;
    symbol = s;
    changed = ch;
}

void ControlType::setID (short newID)    // Sets the object ID.
{
    objectData [row][column] = newID;
}

short ControlType::ID ()    // Returns the object ID.
{
    return objectData [row][column];
}

bool ControlType::active ()    // Returns the object state (off = false, on = true).
{
    return objectData [row][column] % 2 == 0;
}

ChangedObjectType::ChangedObjectType (int r, int c, short n)
{
    row = r;
    column = c;
    newID = n;
}

void ChangedObjectType::updateState ()    // Updates the object state by changing the ID number.
{
    objectData [row][column] = newID;
}

//// Main Functions. ////

int findControlObject (int row, int column)    // Find and return the index of the data for objectData [row][column] in controlObjects. Returns -1 if no match found.
{
    for (unsigned int i = 0; i < controlObjects.size (); ++i)
    {
        if (controlObjects [i].row == row && controlObjects [i].column == column)
            return i;
    }
    
    return -1;
}

int findControlObject (char symbol, short type)    // Find and return the index of the data for the controlObject with the symbol and type. Returns -1 if no match found.
{
    for (unsigned int i = 0; i < controlObjects.size (); ++i)
    {
        if (controlObjects [i].symbol == symbol && (controlObjects [i].ID () - 1) / 2 == type)   // For type, 13 = switch, 14 = button.
            return i;
    }
    
    return -1;
}

int findCopyBuffer (int row, int column)    // Find and return the index of the data for copyBuffer [row][column] in copyBufferData. Returns -1 if no match found.
{
    for (unsigned int i = 0; i < copyBufferData.size (); ++i)
    {
        if (copyBufferData [i].row == row && copyBufferData [i].column == column)
            return i;
    }
    
    return -1;
}

int findChangedObject (int row, int column)    // Find and return the index of the data for changedObjects at the row and column. Returns -1 if no match found.
{
    for (unsigned int i = 0; i < changedObjects.size (); ++i)
    {
        if (changedObjects [i].row == row && changedObjects [i].column == column)
            return i;
    }
    
    return -1;
}

int findSymbolInfo (const string& symbol, int start = 0, int end = 96)    // Find and return the ID of an object in SYMBOL_INFO_LOAD using symbol, from start to end inclusive. Returns -1 if no match found.
{
    for (int i = start; i <= end; ++i)
    {
        if (SYMBOL_INFO_LOAD [i] == symbol)
            return i;
    }
    
    return -1;
}

void drawBoard ()   // Outputs the current circuit board data.
{
    clearScreen;
    cout << "[Circuit Simulator] [" << boardName << "] [Size: " << width << " x " << height << "] [View: " << viewWidth << " x " << viewHeight << "]" << endl;
    
    int visibleWidth = width - viewColumn;     // The total width of the board that is visible.
    if (viewWidth > 0 && viewWidth < visibleWidth)
        visibleWidth = viewWidth;
    
    int visibleHeight = height - viewRow;     // The total height of the board that is visible.
    if (viewHeight > 0 && viewHeight < visibleHeight)
        visibleHeight = viewHeight;
    
    for (int i = paddingY; i > 0; --i)
    {
        int spacing = 0;     // Spacing is half the distance from the edge of paddingX to the start of the character line.
        for (int j = 1; j < i; ++j)
            spacing = (spacing + 1) * 26;
        
        if (spacing < visibleWidth + viewColumn)
        {
            if (spacing <= viewColumn)
                cout << setw (paddingX) << " ";
            else
                cout << setw (paddingX + (spacing - viewColumn) * 2) << " ";
            
            int charNum = int ('a');
            int charLength = int (pow (26.0, i - 1.0));
            int charCounter = 0;
            
            for (int j = 0; j < visibleWidth + viewColumn - spacing; ++j)   // Loop for the length of the character line.
            {
                if (viewColumn - spacing <= j)
                    cout << " " << char (charNum);
                
                ++charCounter;
                if (charCounter >= charLength)
                {
                    ++charNum;
                    charCounter = 0;
                    if (charNum > int ('z'))
                        charNum = int ('a');
                }
            }
        }
        
        cout << endl;
    }
    
    bool topExtends = viewRow > 0;        // If a side of the board continues off-screen, change the graphics to show that.
    bool rightExtends = width > visibleWidth + viewColumn;
    bool bottomExtends = height > visibleHeight + viewRow;
    bool leftExtends = viewColumn > 0;
    
    cout << setw (paddingX) << " ";     // Print the top edge of the board.
    if (topExtends)
    {
        if (leftExtends)
            cout << "\'";
        else
            cout << "*";
        
        cout << setfill ('\'') << setw (visibleWidth * 2) << "\'" << setfill (' ');
        
        if (rightExtends)
            cout << "\'" << endl;
        else
            cout << "*" << endl;
    }
    else
        cout << setfill ('*') << setw (visibleWidth * 2 + 2) << "*" << setfill (' ') << endl;
    
    for (int i = viewRow; i < visibleHeight + viewRow; ++i)      // Print all of the board data and left/right edges.
    {
        cout << setw (paddingX) << i + 1;
        if (leftExtends)
            cout << "\'";
        else
            cout << "*";
        
        for (int j = viewColumn; j < visibleWidth + viewColumn; ++j)
        {
            cout << SYMBOL_INFO_DISP [objectData [i][j]];
            
            if (objectData [i][j] >= 27 && objectData [i][j] <= 30)
            {
                int a = findControlObject (i, j);
                if (a == -1)
                    cout << "\nError: No data found for switch/button at (" << i << ", " << j << ")." << endl;
                else
                    cout << controlObjects [a].symbol;
            }
        }
        
        if (rightExtends)
            cout << "\'" << endl;
        else
            cout << "*" << endl;
    }
    
    cout << setw (paddingX) << " ";     // Print the bottom edge of the board.
    if (bottomExtends)
    {
        if (leftExtends)
            cout << "\'";
        else
            cout << "*";
        
        cout << setfill ('\'') << setw (visibleWidth * 2) << "\'" << setfill (' ');
        
        if (rightExtends)
            cout << "\'" << endl;
        else
            cout << "*" << endl;
    }
    else
        cout << setfill ('*') << setw (visibleWidth * 2 + 2) << "*" << setfill (' ') << endl;
}

void resetData (const string& newName = "NewBoard")   // Resets board data and object data. The new board size depends on the current values of the width and height variables. Does not reset copyBuffer.
{
    boardName = newName;   // Reset name.
    
    if (objectData != nullptr)
    {
        for (int i = 0; i < objectDataSize; ++i)    // Deallocate objectData.
            delete [] objectData [i];
        
        delete [] objectData;
    }
    
    objectDataSize = height;    // Set objectDataSize to remember the original size if resetData () is called again.
    
    objectData = new short * [height];   // Reallocate objectData.
    for (int i = 0; i < height; ++i)
    {
        objectData [i] = new short [width];
        
        for (int j = 0; j < width; ++j)   // Fill with zeros.
            objectData [i][j] = 0;
    }
    
    controlObjects.clear ();   // Remove all data previously in controlObjects and changedObjects.
    changedObjects.clear ();
    viewRow = 0;     // Reset the view position.
    viewColumn = 0;
    
    paddingY = 1;                                 // Calculate the padding distance (padding is the distance from edge of board to edge of window).
    for (int i = 1; i * 26.0 / double (width) < 1.0; i = i * 26 + 1)
        ++paddingY;
    
    paddingX = 1;
    while ((pow (10.0, paddingX) - 1.0) / double (height) < 1.0)
        ++paddingX;
}

int columnToInt (const string& column)   // Converts a column code to its integer equivalent.
{
    int value = -1;   // a = 0, b = 1, c = 2, ...
    
    for (unsigned int i = 0; i < column.size (); ++i)
        value += (int (column [column.size () - i - 1]) - int ('a') + 1) * int (pow (26.0, double (i)));
    
    return value;
}

void parseCommand (const string& command)    // Takes a command and breaks it up into substrings without spaces, the substrings are saved into the subCommands vector.
{
    subCommands.clear ();    // subCommands is a vector that stores a list of the commands.
    unsigned int startPosition = 0;   // startPosition will store the starting position of the next subcommand.
    
    for (unsigned int i = 0; i < command.size (); ++i)
    {
        if (command [i] == ' ')
        {
            if (startPosition < i)    // If subcommand found, add it to the vector.
                subCommands.push_back (command.substr (startPosition, i - startPosition));
            
            startPosition = i + 1;    // Set startPosition to the index of the next character.
        }
    }
    
    if (startPosition < command.size ())    // If command does not end with a space, add the last subcommand to the vector.
        subCommands.push_back (command.substr (startPosition, command.size () - startPosition));
}

int checkOutput (int row, int column, int direction)     // Checks around an object to detect what outputs are nearby in the selected direction. Returns a code representing the selected object status.
{
    short ID;
    if (direction == 0 && row > 0)
        ID = objectData [row - 1][column];
    else if (direction == 1 && column < width - 1)
        ID = objectData [row][column + 1];
    else if (direction == 2 && row < height - 1)
        ID = objectData [row + 1][column];
    else if (direction == 3 && column > 0)
        ID = objectData [row][column - 1];
    else
        return 2;    // No compatible object in this direction.
    
    if (ID == 0)     // Empty space.
        return 2;
    else if (ID <= 22)     // Straight/bent/tri-junction/quad-junction wire.
    {
        if (!CONNECTION_INFO [(ID - 1) / 2][(direction + 2) % 4])    // If the wire does not have a connection pointing in the opposite direction, no connection made.
            return 2;
    }
    else if (ID <= 26)     // Crossover wire.
    {
        if (direction == 0 || direction == 2)
        {
            if (ID == 24 || ID == 26)
                return 1;     // Connected object is on.
        }
        else if (ID >= 25)
            return 1;
        
        return 0;     // Connected object is off.
    }
    else if (ID == 31 || ID == 32)     // Lamp.
        return 2;
    else if (ID >= 33)     // Logic gates.
    {
        if (((ID + 3) / 2) % 4 != direction)    // If the logic gate does not point in the opposite direction, no output received from the gate.
            return 2;
    }
    
    if (ID % 2 == 0)
        return 1;     // Connected object is on.
    else
        return 0;     // Connected object is off.
}

void checkGate (int i, bool& changeHappened, bool condition)      // Checks a logic condition for a logic gate and sets controlObjects [i].changed to true if the gate has changed state.
{
    if (condition)        // Gate should be on.
    {
        if (!controlObjects [i].active ())
        {
            controlObjects [i].changed = true;
            changeHappened = true;
        }
        else if (controlObjects [i].changed)
            controlObjects [i].changed = false;
    }
    else if (controlObjects [i].active ())    // Gate should be off.
    {
        controlObjects [i].changed = true;
        changeHappened = true;
    }
}

int followWire (int row, int column, short startID, int direction, bool state)     // Wire-following algorithm. Checks the object in the selected direction, changes its state if necessary, then calls this function for the new object. Returns 1 if an unchanged active object is found and state is false.
{
    if (direction == 0 && row > 0)     // Set row and column to the position of the object in the selected direction.
        --row;
    else if (direction == 1 && column < width - 1)
        ++column;
    else if (direction == 2 && row < height - 1)
        ++row;
    else if (direction == 3 && column > 0)
        --column;
    else
        return 0;    // No more wire to follow.
    
    short ID = objectData [row][column];       // Set ID to the new object ID.
    int directionBack = (direction + 2) % 4;   // directionBack points back toward the starting object.
    
    if (ID == 0)     // Empty space.
        return 0;
    else if (ID <= 22)     // Straight/bent/tri-junction/quad-junction wire.
    {
        if (CONNECTION_INFO [(ID - 1) / 2][directionBack] && ID % 2 == int (state))    // If wire has a connection pointing in opposite direction and wire is not the same state as the state variable, continue path.
        {
            if (state)
                ++objectData [row][column];
            else
            {
                if ((ID >= 13 || (startID >= 13 && startID <= 22)) && findChangedObject (row, column) != -1)    // If target wire or start wire is a junction and already exists in changedObjects, end path because a loop has been found.
                    return 0;
                
                changedObjects.push_back (ChangedObjectType (row, column, ID - 1));
            }
            
            for (int i = 0; i < 4; ++i)
            {
                if (i != directionBack && CONNECTION_INFO [(ID - 1) / 2][i])
                {
                    if (followWire (row, column, ID, i, state) == 1)
                        return 1;
                }
            }
        }
    }
    else if (ID <= 26)     // Crossover wire.
    {
        int duplicateIndex = -1;    // Index in changedObjects of a duplicate crossover wire.
        if (!state)
        {
            duplicateIndex = findChangedObject (row, column);
            if (duplicateIndex != -1)
                ID = changedObjects [duplicateIndex].newID;
        }
        
        if (direction == 0 || direction == 2)    // If wire is connecting top to bottom...
        {
            if (ID == 24 || ID == 26)    // If wire is on...
            {
                if (state)
                    return 0;
                
                if (duplicateIndex == -1)
                    changedObjects.push_back (ChangedObjectType (row, column, ID - 1));
                else
                    changedObjects [duplicateIndex].newID = ID - 1;
            }
            else if (!state)    // Wire is off.
                return 0;
            else
                ++objectData [row][column];
        }
        else if (ID >= 25)     // Wire is connecting left to right. If wire on...
        {
            if (state)
                return 0;
            
            if (duplicateIndex == -1)
                changedObjects.push_back (ChangedObjectType (row, column, ID - 2));
            else
                changedObjects [duplicateIndex].newID = ID - 2;
        }
        else if (!state)    // Wire is off.
            return 0;
        else
            objectData [row][column] += 2;
        
        return followWire (row, column, ID, direction, state);
    }
    else if (!state && ID <= 30)    // If wires are turning off and an active switch or changed button is found, return 1 because the switch/button will keep wires powered.
    {
        if (ID >= 29)    // Button.
        {
            int a = findControlObject (row, column);
            if (a == -1)
            {
                cout << "Wire Algorithm Error: No data found for object at (" << row + 1 << ", " << column + 1 << ")." << endl;
                return 1;
            }
            else if (!controlObjects [a].changed)
                return 0;
        }
        else if (ID == 27)    // Switch off.
            return 0;
        
        return 1;
    }
    else if (ID == 31)     // Lamp off.
    {
        if (state)
            ++objectData [row][column];
    }
    else if (ID == 32)     // Lamp on.
    {
        if (!state)
            changedObjects.push_back (ChangedObjectType (row, column, ID - 1));
    }
    else if (ID >= 33)      // Logic gates.
    {
        if (((ID - 1) / 2) % 4 == directionBack)    // If the logic gate points in the opposite direction, mark the gate as not changed.
        {
            int a = findControlObject (row, column);
            if (a == -1)
            {
                cout << "Wire Algorithm Error: No data found for object at (" << row + 1 << ", " << column + 1 << ")." << endl;
                return 1;
            }
            else if (controlObjects [a].changed)    // If the gate has changed, update it and mark it as unchanged.
            {
                controlObjects [a].changed = false;
                
                if (controlObjects [a].active ())
                    --objectData [row][column];    // Turn the gate off.
                else
                {
                    ++objectData [row][column];    // Turn the gate on.
                    
                    if (!state)    // If wires are turning off, return 1 because this gate will keep wires powered.
                        return 1;
                }
            }
            else if (!state && controlObjects [a].active ())    // If the wires are turning off and the gate is on, return 1 because this gate will keep wires powered.
                return 1;
        }
    }
    
    return 0;
}

int simulateFrame ()     // Simulates the interaction of objects on the board for one frame. Returns 1 if any objects changed state during the simulation. See Simulation Algorithm in header comments for details.
{
    bool changeHappened = false;
    
    for (unsigned int i = 0; i < controlObjects.size (); ++i)    // Check for updates for logic objects and switches/buttons. Does not modify any data other than the ControlType::changed flag.
    {
        if (controlObjects [i].ID () >= 33)
        {
            int totalInputs = 0, activeInputs = 0, outputSide = ((controlObjects [i].ID () - 1) / 2) % 4;
            
            for (int j = 0; j < 4; ++j)     // Check all sides, except output side, for input objects.
            {
                if (j != outputSide)
                {
                    int returnCode = checkOutput (controlObjects [i].row, controlObjects [i].column, j);
                    if (returnCode != 2)
                    {
                        ++totalInputs;
                        if (returnCode == 1)
                            ++activeInputs;
                    }
                }
            }
            
            if (controlObjects [i].ID () <= 40)
                checkGate (i, changeHappened, activeInputs == 1 && totalInputs <= 1);    // Diode.
            else if (controlObjects [i].ID () <= 48)
                checkGate (i, changeHappened, activeInputs == 0 && totalInputs <= 1);    // NOT gate.
            else if (controlObjects [i].ID () <= 56)
                checkGate (i, changeHappened, activeInputs > 1 && totalInputs == activeInputs);    // AND gate.
            else if (controlObjects [i].ID () <= 64)
                checkGate (i, changeHappened, activeInputs > 0);    // OR gate.
            else if (controlObjects [i].ID () <= 72)
                checkGate (i, changeHappened, activeInputs % 2 == 1);    // XOR gate.
            else if (controlObjects [i].ID () <= 80)
                checkGate (i, changeHappened, activeInputs <= 1 || totalInputs != activeInputs);    // NAND gate.
            else if (controlObjects [i].ID () <= 88)
                checkGate (i, changeHappened, activeInputs == 0);    // NOR gate.
            else
                checkGate (i, changeHappened, activeInputs % 2 == 0);    // XNOR gate.
        }
        else if (controlObjects [i].changed)
            changeHappened = true;
    }
    
    if (!changeHappened)
        return 0;     // No changes happened, no need to update objects.
    
    for (unsigned int i = 0; i < controlObjects.size (); ++i)    // Update each object that is off and has changed (for logic gates turning on and switches/buttons turning off).
    {
        if (controlObjects [i].changed && !controlObjects [i].active ())
        {
            controlObjects [i].changed = false;
            
            if (controlObjects [i].ID () >= 33)
            {
                ++objectData [controlObjects [i].row][controlObjects [i].column];
                followWire (controlObjects [i].row, controlObjects [i].column, controlObjects [i].ID (), ((controlObjects [i].ID () - 1) / 2) % 4, true);
            }
            else
            {
                for (int j = 0; j < 4; ++j)
                {
                    if (followWire (controlObjects [i].row, controlObjects [i].column, controlObjects [i].ID (), j, false) == 0)
                    {
                        for (unsigned int k = 0; k < changedObjects.size (); ++k)
                            changedObjects [k].updateState ();
                    }
                    
                    changedObjects.clear ();
                }
            }
        }
    }
    
    for (unsigned int i = 0; i < controlObjects.size (); ++i)    // Update each object that is on and has changed (for logic gates turning off and switches/buttons turning on).
    {
        if (controlObjects [i].changed)
        {
            if (controlObjects [i].ID () >= 33)
            {
                controlObjects [i].changed = false;
                --objectData [controlObjects [i].row][controlObjects [i].column];
                
                if (followWire (controlObjects [i].row, controlObjects [i].column, controlObjects [i].ID (), ((controlObjects [i].ID () - 1) / 2) % 4, false) == 0)
                {
                    for (unsigned int j = 0; j < changedObjects.size (); ++j)
                        changedObjects [j].updateState ();
                }
                
                changedObjects.clear ();
            }
            else
            {
                if (controlObjects [i].ID () >= 29)    // If the object is a button...
                    --objectData [controlObjects [i].row][controlObjects [i].column];
                else
                    controlObjects [i].changed = false;
                
                for (int j = 0; j < 4; ++j)
                    followWire (controlObjects [i].row, controlObjects [i].column, controlObjects [i].ID (), j, true);
            }
        }
    }
    
    return 1;    // Changes were made.
}

int copyRegion (int row1, int column1, int row2, int column2)    // Copies a region on the board to the copyBuffer. Returns 1 if an error occurred.
{
    if (row1 < 0 || row1 >= height || column1 < 0 || column1 >= width || row2 < 0 || row2 >= height || column2 < 0 || column2 >= width)
    {
        cout << "Error: Specified region extends off the board." << endl;
        return 1;    // Copy error.
    }
    
    if (row2 < row1)    // Switch row values so that row1 is the min value.
    {
        int tempRow = row2;
        row2 = row1;
        row1 = tempRow;
    }
    if (column2 < column1)    // Switch column values so that column1 is the min value.
    {
        int tempColumn = column2;
        column2 = column1;
        column1 = tempColumn;
    }
    
    if (copyBuffer != nullptr)
    {
        for (int i = 0; i < copyBufferHeight; ++i)    // Deallocate copyBuffer.
            delete [] copyBuffer [i];
        
        delete [] copyBuffer;
    }
    
    copyBufferWidth = column2 - column1 + 1;    // Reset data for the buffer.
    copyBufferHeight = row2 - row1 + 1;
    copyBufferData.clear ();
    
    copyBuffer = new short * [copyBufferHeight];   // Reallocate copyBuffer.
    for (int i = 0; i < copyBufferHeight; ++i)
        copyBuffer [i] = new short [copyBufferWidth];
    
    for (int i = 0; i < copyBufferHeight; ++i)
    {
        for (int j = 0; j < copyBufferWidth; ++j)
        {
            if (objectData [row1 + i][column1 + j] >= 27 && objectData [row1 + i][column1 + j] <= 30)    // Switch/button.
            {
                int a = findControlObject (row1 + i, column1 + j);
                if (a == -1)
                {
                    cout << "Error: No data found for switch/button at position (" << row1 + i + 1 << ", " << column1 + j + 1 << ")." << endl;
                    copyBuffer [i][j] = 0;
                    continue;
                }
                
                copyBufferData.push_back (ControlType (i, j, controlObjects [a].symbol, true));
            }
            
            copyBuffer [i][j] = objectData [row1 + i][column1 + j];
        }
    }
    
    return 0;    // Copy successful.
}

int pasteRegion (int row, int column)    // Pastes data in copyBuffer to the board. Returns 1 if an error occurred.
{
    if (row < 0 || row + copyBufferHeight > height || column < 0 || column + copyBufferWidth > width)
    {
        cout << "Error: The " << copyBufferWidth << " x " << copyBufferHeight << " region in the copy buffer extends off the board." << endl;
        return 1;    // Paste error.
    }
    
    bool duplicateObject = false;
    
    for (int i = 0; i < copyBufferHeight; ++i)
    {
        for (int j = 0; j < copyBufferWidth; ++j)
        {
            int a = findControlObject (row + i, column + j);    // Delete data for the old object at the selected position.
            if (a != -1)
                controlObjects.erase (controlObjects.begin () + a);
            
            if (copyBuffer [i][j] >= 27 && copyBuffer [i][j] <= 30)    // Switch/button.
            {
                a = findCopyBuffer (i, j);
                if (a == -1)
                {
                    cout << "Error: No data found in copyBuffer for switch/button at position (" << row + i + 1 << ", " << column + j + 1 << ")." << endl;
                    objectData [row + i][column + j] = 0;
                    continue;
                }
                else if (findControlObject (copyBufferData [a].symbol, short ((copyBuffer [i][j] - 1) / 2)) != -1)
                {
                    duplicateObject = true;
                    objectData [row + i][column + j] = 0;
                    continue;
                }
                
                controlObjects.push_back (ControlType (row + i, column + j, copyBufferData [a].symbol, true));
            }
            else if (copyBuffer [i][j] >= 33)    // Logic gate.
                controlObjects.push_back (ControlType (row + i, column + j));
            
            objectData [row + i][column + j] = copyBuffer [i][j];
        }
    }
    
    if (duplicateObject)
    {
        string command;
        cout << "\nSome duplicate switches/buttons were detected and have been skipped while pasting data.\n(Press enter to return to board)" << endl;
        getline (cin, command);
    }
    
    return 0;    // Paste successful.
}

int runCommand ()    // Requests for and executes a command, returns a code representing the command status.
{
    string command;
    getline (cin, command);    // Get a command from input.
    parseCommand (command);    // Split into subcommands.
    
    if (subCommands.size () == 0)
        return 1;    // Command error.
    
    
    else if (isdigit (subCommands [0][0]))    // <row> <column> <ID number> [<switch or button ID>]    | Places an object on the board.
    {
        if (subCommands.size () < 3 || subCommands.size () > 4)
            return 1;
        
        int row, column;
        short ID;
        char symbol = '0';
        
        stringstream (subCommands [0]) >> row;
        --row;
        column = columnToInt (subCommands [1]);
        stringstream (subCommands [2]) >> ID;
        
        if (row < 0 || row >= height || column < 0 || column >= width || ID < 0 || ID > 96)
            return 1;
        else if (objectData [row][column] != 0)
        {
            int a = findControlObject (row, column);    // Delete data for the old object at the selected position to avoid problems if a command error happens.
            if (a != -1)
                controlObjects.erase (controlObjects.begin () + a);
            
            objectData [row][column] = 0;
        }
        
        if (ID >= 27 && ID <= 30)    // Create new controlObjects entry if switch/button detected.
        {
            if (subCommands.size () == 4)
                symbol = subCommands [3][0];
            
            int a = findControlObject (symbol, short ((ID - 1) / 2));
            if (a != -1)
            {
                if ((controlObjects [a].ID () - 1) / 2 == 13)
                    cout << "Error: Switch";
                else
                    cout << "Error: Button";
                
                cout << " with ID \'" << symbol << "\' already in use at position (" << controlObjects [a].row + 1 << ", " << controlObjects [a].column + 1 << "). Object at selected position has been deleted." << endl;
                return 1;
            }
            
            controlObjects.push_back (ControlType (row, column, symbol, true));
        }
        else if (subCommands.size () == 4)
        {
            cout << "Error: Switch/button ID specified for non-input object. Object at selected position has been deleted." << endl;
            return 1;
        }
        else if (ID >= 33)       // Create new controlObjects entry if logic object detected.
            controlObjects.push_back (ControlType (row, column));
        
        objectData [row][column] = ID;   // Set ID for the new object.
    }
    
    
    else if (subCommands [0] == "run" || subCommands [0] == "r")    // run/r [<number of iterations>] [no-print/np]    | Simulate for a number of frames, no-print disables screen updates until finished.
    {
        if (subCommands.size () > 3)
            return 1;
        
        int remainingFrames = 30;
        bool noPrint = false;
        
        if (subCommands.size () >= 2)
        {
            if (isdigit (subCommands [1][0]))
            {
                stringstream (subCommands [1]) >> remainingFrames;
                
                if (remainingFrames < 1 || remainingFrames > 60)
                {
                    cout << "Error: Number of iterations to simulate must be between 1 and 60 inclusive." << endl;
                    return 1;
                }
                else if (subCommands.size () == 3)
                {
                    if (subCommands [2] != "no-print" && subCommands [2] != "np")
                        return 1;
                    
                    noPrint = true;
                }
            }
            else if (subCommands.size () == 3 || (subCommands [1] != "no-print" && subCommands [1] != "np"))
                return 1;
            else
                noPrint = true;
        }
        
        if (noPrint)
        {
            while (remainingFrames > 0)
            {
                if (simulateFrame () == 0)
                    return 0;
                
                --remainingFrames;
            }
            
            cout << "\nFrame iteration done, simulation did not stop.\n(Press enter to return to board)" << endl;
            getline (cin, command);
        }
        else
        {
            if (simulateFrame () == 0)
                return 0;
            
            --remainingFrames;
            while (remainingFrames > 0)
            {
                drawBoard ();
                cout << "\nRemaining iterations: " << remainingFrames << endl;
                this_thread::sleep_for (chrono::milliseconds (500));
                if (simulateFrame () == 0)
                    return 0;
                
                --remainingFrames;
            }
        }
    }
    
    
    else if (subCommands [0] == "switch" || subCommands [0] == "s")    // switch/s <switch ID>    | Toggles a switch with switch ID.
    {
        if (subCommands.size () != 2)
            return 1;
        
        char symbol = subCommands [1][0];
        int a = findControlObject (symbol, short (13));
        if (a == -1)
        {
            cout << "Error: No switch with ID \'" << symbol << "\'." << endl;
            return 1;
        }
        
        controlObjects [a].setID (27 + !controlObjects [a].active ());    // Toggle object state.
        controlObjects [a].changed = true;     // Set to "changed".
    }
    
    
    else if (subCommands [0] == "button" || subCommands [0] == "b")    // button/b <button ID>    | Activates a button with button ID.
    {
        if (subCommands.size () != 2)
            return 1;
        
        char symbol = subCommands [1][0];
        int a = findControlObject (symbol, short (14));
        if (a == -1)
        {
            cout << "Error: No button with ID \'" << symbol << "\'." << endl;
            return 1;
        }
        
        controlObjects [a].setID (29 + !controlObjects [a].active ());    // Toggle object state.
        controlObjects [a].changed = true;     // Set to "changed".
    }
    
    
    else if (subCommands [0] == "resize")    // resize <new width> <new height>    | Resizes the board, preserves data currently on the board.
    {
        if (subCommands.size () != 3)
            return 1;
        
        int newWidth, newHeight, copyWidth = width - 1, copyHeight = height - 1;
        
        stringstream (subCommands [1]) >> newWidth;
        stringstream (subCommands [2]) >> newHeight;
        
        if (newWidth < 1 || newHeight < 1)
        {
            cout << "Error: Invalid dimensions for board." << endl;
            return 1;
        }
        else if (newWidth < width || newHeight < height)
        {
            cout << "\nAre you sure you want to truncate the board? Any objects that do not fit on the new board will be deleted (y/n):" << endl;
            getline (cin, command);
            
            if (command != "y" && command != "Y" && command != "yes" && command != "Yes")
                return 0;
            
            if (newWidth < width)
                copyWidth = newWidth - 1;
            if (newHeight < height)
                copyHeight = newHeight - 1;
        }
        
        if (copyRegion (0, 0, copyHeight, copyWidth) == 1)
        {
            cout << "Error: Unable to copy region for resizing." << endl;
            return 1;
        }
        
        width = newWidth;
        height = newHeight;
        resetData (boardName);
        
        if (pasteRegion (0, 0) == 1)
        {
            cout << "Error: Unable to paste region after resizing." << endl;
            return 1;
        }
    }
    
    
    else if (subCommands [0] == "new")    // new <width> <height>    | Creates a new board.
    {
        if (subCommands.size () != 3)
            return 1;
        
        int newWidth, newHeight;
        stringstream (subCommands [1]) >> newWidth;
        stringstream (subCommands [2]) >> newHeight;
        
        if (newWidth < 1 || newHeight < 1)
        {
            cout << "Error: Invalid dimensions for board." << endl;
            return 1;
        }
        
        cout << "\nAre you sure you want to create a new board? All changes to the current board will be discarded (y/n):" << endl;
        getline (cin, command);
        
        if (command != "y" && command != "Y" && command != "yes" && command != "Yes")
            return 0;
        
        width = newWidth;
        height = newHeight;
        resetData ();
    }
    
    
    else if (subCommands [0] == "copy" || subCommands [0] == "c")    // copy/c <row 1> <column 1> <row 2> <column 2>    | Copies the selected area to a buffer.
    {
        if (subCommands.size () != 5)
            return 1;
        
        int row1, column1, row2, column2;
        
        stringstream (subCommands [1]) >> row1;
        --row1;
        column1 = columnToInt (subCommands [2]);
        stringstream (subCommands [3]) >> row2;
        --row2;
        column2 = columnToInt (subCommands [4]);
        
        if (copyRegion (row1, column1, row2, column2) == 1)
            return 1;
    }
    
    
    else if (subCommands [0] == "paste" || subCommands [0] == "p")    // paste/p <row> <column>    | Pastes data in the buffer using the row and column as the top left corner.
    {
        if (subCommands.size () != 3)
            return 1;
        
        int row, column;
        
        stringstream (subCommands [1]) >> row;
        --row;
        column = columnToInt (subCommands [2]);
        
        if (pasteRegion (row, column) == 1)
            return 1;
    }
    
    
    else if (subCommands [0] == "fill" || subCommands [0] == "f")    // fill/f <row 1> <column 1> <row 2> <column 2> <ID number>    | Fills the specified area with objects.
    {
        if (subCommands.size () != 6)
            return 1;
        
        int row1, column1, row2, column2;
        short ID;
        
        stringstream (subCommands [1]) >> row1;
        --row1;
        column1 = columnToInt (subCommands [2]);
        stringstream (subCommands [3]) >> row2;
        --row2;
        column2 = columnToInt (subCommands [4]);
        stringstream (subCommands [5]) >> ID;
        
        if (row1 < 0 || row1 >= height || column1 < 0 || column1 >= width || row2 < 0 || row2 >= height || column2 < 0 || column2 >= width)
        {
            cout << "Error: Specified region extends off the board." << endl;
            return 1;
        }
        else if (ID >= 27 && ID <= 30)    // Switch/button.
        {
            cout << "Error: Filling a region is not compatible with switches/buttons." << endl;
            return 1;
        }
        
        if (row2 < row1)    // Switch row values so that row1 is the min value.
        {
            int tempRow = row2;
            row2 = row1;
            row1 = tempRow;
        }
        if (column2 < column1)    // Switch column values so that column1 is the min value.
        {
            int tempColumn = column2;
            column2 = column1;
            column1 = tempColumn;
        }
        
        if (ID >= 33)    // Logic gate.
        {
            for (int i = row1; i <= row2; ++i)
            {
                for (int j = column1; j <= column2; ++j)
                {
                    int a = findControlObject (i, j);    // Delete data for the old object at the selected position.
                    if (a != -1)
                        controlObjects.erase (controlObjects.begin () + a);
                    
                    objectData [i][j] = ID;
                    controlObjects.push_back (ControlType (i, j));
                }
            }
        }
        else    // Non-control object.
        {
            for (int i = row1; i <= row2; ++i)
            {
                for (int j = column1; j <= column2; ++j)
                {
                    int a = findControlObject (i, j);    // Delete data for the old object at the selected position.
                    if (a != -1)
                        controlObjects.erase (controlObjects.begin () + a);
                    
                    objectData [i][j] = ID;
                }
            }
        }
    }
    
    
    else if (subCommands [0] == "rename")    // rename <new board name>    | Renames the current board.
    {
        if (subCommands.size () != 2)
            return 1;
        
        boardName = subCommands [1];
    }
    
    
    else if (subCommands [0] == "save")    // save [<new board name>]    | Saves the current board to a file.
    {
        if (subCommands.size () > 2)
            return 1;
        else if (subCommands.size () == 2)
            boardName = subCommands [1];
        
        ofstream saveFile (BOARD_DIR_SAVE + boardName + ".txt", ios::trunc);
        
        if (!saveFile.is_open ())
        {
            cout << "Error: IO file error." << endl;
            return 1;
        }
        
        saveFile << width << endl << height << endl;
        
        saveFile << setfill ('*') << setw (width * 2 + 2) << "*" << setfill (' ') << endl;
        for (int i = 0; i < height; ++i)
        {
            saveFile << "*";
            for (int j = 0; j < width; ++j)
            {
                saveFile << SYMBOL_INFO_SAVE [objectData [i][j]];
                
                if (objectData [i][j] >= 27 && objectData [i][j] <= 30)
                {
                    int a = findControlObject (i, j);
                    if (a == -1)
                    {
                        cout << "Error: Could not find controlObjects data for switch/button at position (" << i + 1 << ", " << j + 1 << ")." << endl;
                        saveFile.close ();
                        return 1;
                    }
                    
                    saveFile << controlObjects [a].symbol;
                }
            }
            saveFile << "*" << endl;
        }
        saveFile << setfill ('*') << setw (width * 2 + 2) << "*" << setfill (' ');
        
        saveFile.close ();
        cout << "Saved to: " << BOARD_DIR_SAVE << boardName << ".txt";
    }
    
    
    else if (subCommands [0] == "load")    // load <board name>    | Loads a board from a file.
    {
        if (subCommands.size () > 2)
            return 1;
        else if (subCommands.size () == 1)
        {
            DIR * directoryPtr = opendir (BOARD_DIR_LOAD.c_str ());       // https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
            struct dirent * direntPtr;
            
            if (directoryPtr != nullptr)     // Check if directory opened successfully.
            {
                cout << "\nAvailable boards in: " << BOARD_DIR_LOAD << endl;
                
                while ((direntPtr = readdir (directoryPtr)) != nullptr)     // List contents that end in ".txt" only.
                {
                    string filename = direntPtr->d_name;
                    
                    if (filename.find (".txt") != string::npos)
                        cout << "  " << filename.substr (0, filename.size () - 4) << endl;
                }
                
                closedir (directoryPtr);
                cout << "(Press enter to return to board)" << endl;
                getline (cin, command);
            }
            else
            {
                cout << "Error: IO file error." << endl;     // Failed to open the directory.
                return 1;
            }
        }
        else
        {
            cout << "\nAre you sure you want to load this board? All changes to the current board will be discarded (y/n):" << endl;
            getline (cin, command);
            
            if (command != "y" && command != "Y" && command != "yes" && command != "Yes")
                return 0;
            
            cout << "Loading: " << BOARD_DIR_LOAD << subCommands [1] << ".txt" << endl;
            ifstream loadFile (BOARD_DIR_LOAD + subCommands [1] + ".txt");
            
            if (!loadFile.is_open ())
            {
                cout << "Error: IO file error." << endl;
                return 1;
            }
            
            int newWidth, newHeight;
            getline (loadFile, command);      // The command string will now be used to store each line in the loadFile.
            stringstream (command) >> newWidth;
            getline (loadFile, command);
            stringstream (command) >> newHeight;
            
            if (newWidth < 1 || newHeight < 1)
            {
                cout << "Error: Invalid dimensions for board in load file." << endl;
                loadFile.close ();
                return 1;
            }
            
            width = newWidth;
            height = newHeight;
            resetData (subCommands [1]);
            
            getline (loadFile, command);
            for (int i = 0; i < height; ++i)
            {
                getline (loadFile, command);
                int a, position = 1;
                for (int j = 0; j < width; ++j)
                {
                    if (command.substr (position, 2) == "  ")    // Check for empty space.
                        position += 2;
                    else if ((a = findSymbolInfo (command.substr (position, 1), 27, 30)) != -1)    // Else, check for switch/button.
                    {
                        if (findControlObject (command [position + 1], short ((a - 1) / 2)) != -1)
                        {
                            cout << "Error: Duplicate switch/button in load file at position (" << i + 1 << ", " << j + 1 << "). Not all of the data could be loaded." << endl;
                            loadFile.close ();
                            return 1;
                        }
                        
                        objectData [i][j] = a;
                        controlObjects.push_back (ControlType (i, j, command [position + 1], true));
                        position += 2;
                    }
                    
                    #if SYMBOL_INFO_LOAD == SYMBOL_INFO_UNIX    // In Linux, textures use special characters that are represented with multi-strings. Windows textures do not use these special characters.
                    else if ((a = findSymbolInfo (command.substr (position, 2), 33)) != -1)    // Else, check for logic object.
                    {
                        objectData [i][j] = a;
                        controlObjects.push_back (ControlType (i, j));
                        position += 2;
                    }
                    else if ((a = findSymbolInfo (command.substr (position, 4), 1, 18)) != -1)    // Else, check for wire next to space.
                    {
                        objectData [i][j] = a;
                        position += 4;
                    }
                    else if ((a = findSymbolInfo (command.substr (position, 6), 3, 32)) != -1)    // Else, check for two wires or a lamp.
                    {
                        objectData [i][j] = a;
                        position += 6;
                    }
                    
                    #else    // Alternate searching mechanics for Windows.
                    else if ((a = findSymbolInfo (command.substr (position, 2), 1)) != -1)    // Else, check for non-input object.
                    {
                        if (a >= 33)    // Logic gate.
                            controlObjects.push_back (ControlType (i, j));
                        
                        objectData [i][j] = a;
                        position += 2;
                    }
                    
                    #endif
                    
                    else      // Else, object does not exist.
                    {
                        cout << "Error: Invalid symbol in load file at position (" << i + 1 << ", " << j + 1 << "). Not all of the data could be loaded." << endl;
                        loadFile.close ();
                        return 1;
                    }
                }
            }
            
            loadFile.close ();
        }
    }
    
    
    else if (subCommands [0] == "reset")    // reset    | Deactivates all objects on the board to fix half-powered wires.
    {
        if (subCommands.size () != 1)
            return 1;
        
        cout << "\nAre you sure you want to reset the board? This will deactivate all objects (y/n):" << endl;
        getline (cin, command);
        
        if (command != "y" && command != "Y" && command != "yes" && command != "Yes")
            return 0;
        
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                if (objectData [i][j] >= 24 && objectData [i][j] <= 26)    // If object is a powered crossover wire, turn it off.
                    objectData [i][j] = 23;
                else if (objectData [i][j] != 0 && objectData [i][j] % 2 == 0)    // Else if the object is not an empty space and is powered, turn it off.
                    --objectData [i][j];
            }
        }
    }
    
    
    else if (subCommands [0] == "view" || subCommands [0] == "v")    // view/v [resize] <width/row> <height/column>    | Changes the view to show only a part of the board, useful for smaller screens.
    {
        if (subCommands.size () < 3)
            return 1;
        else if (subCommands [1] == "resize")     // Check if view is being resized.
        {
            if (subCommands.size () != 4)
                return 1;
            
            int width, height;
            stringstream (subCommands [2]) >> width;
            stringstream (subCommands [3]) >> height;
            
            if (width < 0 || height < 0)
            {
                cout << "Error: Minimum value for width and height is zero." << endl;
                return 1;
            }
            
            viewWidth = width;
            viewHeight = height;
        }
        else     // Otherwise, view is being moved to a new location.
        {
            if (subCommands.size () != 3)
                return 1;
            
            int row, column;
            stringstream (subCommands [1]) >> row;
            --row;
            column = columnToInt (subCommands [2]);
            
            if (row < 0 || row >= height || column < 0 || column >= width)
                return 1;
            
            viewRow = row;
            viewColumn = column;
        }
    }
    
    
    else if (subCommands [0] == "trim")    // trim <extra width> <extra height>    | Crops the board and adds a border of extra space, does not delete objects.
    {
        if (subCommands.size () != 3)
            return 1;
        
        int extraWidth, extraHeight, topBorder = -1, bottomBorder = -1, leftBorder = -1, rightBorder = -1;
        
        stringstream (subCommands [1]) >> extraWidth;
        stringstream (subCommands [2]) >> extraHeight;
        
        if (extraWidth < 0 || extraHeight < 0)
        {
            cout << "Error: Minimum value for extra width and height is zero." << endl;
            return 1;
        }
        
        for (int i = 0; i < height; ++i)    // Scan left-to-right, top-to-bottom, and check for top border.
        {
            for (int j = 0; j < width; ++j)
            {
                if (objectData [i][j] != 0)
                {
                    topBorder = i;
                    i = height;
                    j = width;
                }
            }
        }
        
        if (topBorder == -1)
        {
            cout << "Error: Nothing to trim, the board is empty." << endl;
            return 1;
        }
        
        for (int i = height - 1; i >= 0; --i)    // Scan left-to-right, bottom-to-top, and check for bottom border.
        {
            for (int j = 0; j < width; ++j)
            {
                if (objectData [i][j] != 0)
                {
                    bottomBorder = i;
                    i = -1;
                    j = width;
                }
            }
        }
        
        for (int j = 0; j < width; ++j)    // Scan top-to-bottom, left-to-right, and check for left border.
        {
            for (int i = 0; i < height; ++i)
            {
                if (objectData [i][j] != 0)
                {
                    leftBorder = j;
                    i = height;
                    j = width;
                }
            }
        }
        
        for (int j = width - 1; j >= 0; --j)    // Scan top-to-bottom, right-to-left, and check for right border.
        {
            for (int i = 0; i < height; ++i)
            {
                if (objectData [i][j] != 0)
                {
                    rightBorder = j;
                    i = height;
                    j = -1;
                }
            }
        }
        
        if (copyRegion (topBorder, leftBorder, bottomBorder, rightBorder) == 1)
        {
            cout << "Error: Unable to copy region for trimming." << endl;
            return 1;
        }
        
        width = rightBorder - leftBorder + 1 + extraWidth * 2;
        height = bottomBorder - topBorder + 1 + extraHeight * 2;
        resetData (boardName);
        
        if (pasteRegion (extraHeight, extraWidth) == 1)
        {
            cout << "Error: Unable to paste region after trimming." << endl;
            return 1;
        }
    }
    
    
    else if (subCommands [0] == "table" || subCommands [0] == "t")    // table/t    | Prints a table showing the ID numbers of objects.
    {
        if (subCommands.size () != 1)
            return 1;
        
        #if SYMBOL_INFO_DISP == SYMBOL_INFO_UNIX    // Lookup table for Linux.
        cout << "\n\
Object lookup table:\n\
  [Name]                | [ID] | [Symbol] | [Name]               | [ID] | [Symbol]\n\
  Empty space           |  0   | \"  \"     |                      |      |\n\
  Wire vertical off     |  1   | \"│ \"     | Wire vertical on     |  2   | \"║ \"\n\
  Wire horizontal off   |  3   | \"──\"     | Wire horizontal on   |  4   | \"══\"\n\
  Wire up right off     |  5   | \"└─\"     | Wire up right on     |  6   | \"╚═\"\n\
  Wire down right off   |  7   | \"┌─\"     | Wire down right on   |  8   | \"╔═\"\n\
  Wire down left off    |  9   | \"┐ \"     | Wire down left on    | 10   | \"╗ \"\n\
  Wire up left off      | 11   | \"┘ \"     | Wire up left on      | 12   | \"╝ \"\n\
  Wire vert. right off  | 13   | \"├─\"     | Wire vert. right on  | 14   | \"╠═\"\n\
  Wire horiz. down off  | 15   | \"┬─\"     | Wire horiz. down on  | 16   | \"╦═\"\n\
  Wire vert. left off   | 17   | \"┤ \"     | Wire vert. left on   | 18   | \"╣ \"\n\
  Wire horiz. up off    | 19   | \"┴─\"     | Wire horiz. up on    | 20   | \"╩═\"\n\
  Wire junction off     | 21   | \"┼─\"     | Wire junction on     | 22   | \"╬═\"\n\
  Wire crossover off    | 23   | \"│─\"     | Wire cross. vert. on | 24   | \"║─\"\n\
  Wire cross. horiz. on | 25   | \"│═\"     | Wire cross. both on  | 26   | \"║═\"\n\
  Switch off            | 27   | \"s#\"     | Switch on            | 28   | \"S#\"\n\
  Button off            | 29   | \"t#\"     | Button on            | 30   | \"T#\"\n\
  Lamp off              | 31   | \"░░\"     | Lamp on              | 32   | \"██\"\n\
  Diode up off          | 33   | \"^d\"     | Diode up on          | 34   | \"^D\"\n\
  Diode right off       | 35   | \">d\"     | Diode right on       | 36   | \">D\"\n\
  Diode down off        | 37   | \"vd\"     | Diode down on        | 38   | \"vD\"\n\
  Diode left off        | 39   | \"<d\"     | Diode left on        | 40   | \"<D\"\n\
  NOT gate up off       | 41   | \"^n\"     | NOT gate up on       | 42   | \"^N\"\n\
  NOT gate right off    | 43   | \">n\"     | NOT gate right on    | 44   | \">N\"\n\
  NOT gate down off     | 45   | \"vn\"     | NOT gate down on     | 46   | \"vN\"\n\
  NOT gate left off     | 47   | \"<n\"     | NOT gate left on     | 48   | \"<N\"\n\
  AND gate up off       | 49   | \"^a\"     | AND gate up on       | 50   | \"^A\"\n\
  AND gate right off    | 51   | \">a\"     | AND gate right on    | 52   | \">A\"\n\
  AND gate down off     | 53   | \"va\"     | AND gate down on     | 54   | \"vA\"\n\
  AND gate left off     | 55   | \"<a\"     | AND gate left on     | 56   | \"<A\"\n\
  OR gate up off        | 57   | \"^o\"     | OR gate up on        | 58   | \"^O\"\n\
  OR gate right off     | 59   | \">o\"     | OR gate right on     | 60   | \">O\"\n\
  OR gate down off      | 61   | \"vo\"     | OR gate down on      | 62   | \"vO\"\n\
  OR gate left off      | 63   | \"<o\"     | OR gate left on      | 64   | \"<O\"\n\
  XOR gate up off       | 65   | \"^x\"     | XOR gate up on       | 66   | \"^X\"\n\
  XOR gate right off    | 67   | \">x\"     | XOR gate right on    | 68   | \">X\"\n\
  XOR gate down off     | 69   | \"vx\"     | XOR gate down on     | 70   | \"vX\"\n\
  XOR gate left off     | 71   | \"<x\"     | XOR gate left on     | 72   | \"<X\"\n\
  NAND gate up off      | 73   | \"^b\"     | NAND gate up on      | 74   | \"^B\"\n\
  NAND gate right off   | 75   | \">b\"     | NAND gate right on   | 76   | \">B\"\n\
  NAND gate down off    | 77   | \"vb\"     | NAND gate down on    | 78   | \"vB\"\n\
  NAND gate left off    | 79   | \"<b\"     | NAND gate left on    | 80   | \"<B\"\n\
  NOR gate up off       | 81   | \"^p\"     | NOR gate up on       | 82   | \"^P\"\n\
  NOR gate right off    | 83   | \">p\"     | NOR gate right on    | 84   | \">P\"\n\
  NOR gate down off     | 85   | \"vp\"     | NOR gate down on     | 86   | \"vP\"\n\
  NOR gate left off     | 87   | \"<p\"     | NOR gate left on     | 88   | \"<P\"\n\
  XNOR gate up off      | 89   | \"^y\"     | XNOR gate up on      | 90   | \"^Y\"\n\
  XNOR gate right off   | 91   | \">y\"     | XNOR gate right on   | 92   | \">Y\"\n\
  XNOR gate down off    | 93   | \"vy\"     | XNOR gate down on    | 94   | \"vY\"\n\
  XNOR gate left off    | 95   | \"<y\"     | XNOR gate left on    | 96   | \"<Y\"\n\
(Press enter to return to board)" << endl;
        
        #else    // Lookup table for Windows.
        cout << "\n\
Object lookup table:\n\
  [Name]                | [ID] | [Symbol] | [Name]               | [ID] | [Symbol]\n\
  Empty space           |  0   | \"  \"     |                      |      |\n\
  Wire vertical off     |  1   | \"| \"     | Wire vertical on     |  2   | \"[ \"\n\
  Wire horizontal off   |  3   | \"--\"     | Wire horizontal on   |  4   | \"==\"\n\
  Wire up right off     |  5   | \"\'-\"     | Wire up right on     |  6   | \"\"=\"\n\
  Wire down right off   |  7   | \",-\"     | Wire down right on   |  8   | \";=\"\n\
  Wire down left off    |  9   | \", \"     | Wire down left on    | 10   | \"; \"\n\
  Wire up left off      | 11   | \"\' \"     | Wire up left on      | 12   | \"\" \"\n\
  Wire vert. right off  | 13   | \">-\"     | Wire vert. right on  | 14   | \">=\"\n\
  Wire horiz. down off  | 15   | \"v-\"     | Wire horiz. down on  | 16   | \"v=\"\n\
  Wire vert. left off   | 17   | \"< \"     | Wire vert. left on   | 18   | \"<.\"\n\
  Wire horiz. up off    | 19   | \"^-\"     | Wire horiz. up on    | 20   | \"^=\"\n\
  Wire junction off     | 21   | \"+-\"     | Wire junction on     | 22   | \"#=\"\n\
  Wire crossover off    | 23   | \"|-\"     | Wire cross. vert. on | 24   | \"[-\"\n\
  Wire cross. horiz. on | 25   | \"|=\"     | Wire cross. both on  | 26   | \"[=\"\n\
  Switch off            | 27   | \"s#\"     | Switch on            | 28   | \"S#\"\n\
  Button off            | 29   | \"t#\"     | Button on            | 30   | \"T#\"\n\
  Lamp off              | 31   | \"..\"     | Lamp on              | 32   | \"##\"\n\
  Diode up off          | 33   | \"^d\"     | Diode up on          | 34   | \"^D\"\n\
  Diode right off       | 35   | \">d\"     | Diode right on       | 36   | \">D\"\n\
  Diode down off        | 37   | \"vd\"     | Diode down on        | 38   | \"vD\"\n\
  Diode left off        | 39   | \"<d\"     | Diode left on        | 40   | \"<D\"\n\
  NOT gate up off       | 41   | \"^n\"     | NOT gate up on       | 42   | \"^N\"\n\
  NOT gate right off    | 43   | \">n\"     | NOT gate right on    | 44   | \">N\"\n\
  NOT gate down off     | 45   | \"vn\"     | NOT gate down on     | 46   | \"vN\"\n\
  NOT gate left off     | 47   | \"<n\"     | NOT gate left on     | 48   | \"<N\"\n\
  AND gate up off       | 49   | \"^a\"     | AND gate up on       | 50   | \"^A\"\n\
  AND gate right off    | 51   | \">a\"     | AND gate right on    | 52   | \">A\"\n\
  AND gate down off     | 53   | \"va\"     | AND gate down on     | 54   | \"vA\"\n\
  AND gate left off     | 55   | \"<a\"     | AND gate left on     | 56   | \"<A\"\n\
  OR gate up off        | 57   | \"^o\"     | OR gate up on        | 58   | \"^O\"\n\
  OR gate right off     | 59   | \">o\"     | OR gate right on     | 60   | \">O\"\n\
  OR gate down off      | 61   | \"vo\"     | OR gate down on      | 62   | \"vO\"\n\
  OR gate left off      | 63   | \"<o\"     | OR gate left on      | 64   | \"<O\"\n\
  XOR gate up off       | 65   | \"^x\"     | XOR gate up on       | 66   | \"^X\"\n\
  XOR gate right off    | 67   | \">x\"     | XOR gate right on    | 68   | \">X\"\n\
  XOR gate down off     | 69   | \"vx\"     | XOR gate down on     | 70   | \"vX\"\n\
  XOR gate left off     | 71   | \"<x\"     | XOR gate left on     | 72   | \"<X\"\n\
  NAND gate up off      | 73   | \"^b\"     | NAND gate up on      | 74   | \"^B\"\n\
  NAND gate right off   | 75   | \">b\"     | NAND gate right on   | 76   | \">B\"\n\
  NAND gate down off    | 77   | \"vb\"     | NAND gate down on    | 78   | \"vB\"\n\
  NAND gate left off    | 79   | \"<b\"     | NAND gate left on    | 80   | \"<B\"\n\
  NOR gate up off       | 81   | \"^p\"     | NOR gate up on       | 82   | \"^P\"\n\
  NOR gate right off    | 83   | \">p\"     | NOR gate right on    | 84   | \">P\"\n\
  NOR gate down off     | 85   | \"vp\"     | NOR gate down on     | 86   | \"vP\"\n\
  NOR gate left off     | 87   | \"<p\"     | NOR gate left on     | 88   | \"<P\"\n\
  XNOR gate up off      | 89   | \"^y\"     | XNOR gate up on      | 90   | \"^Y\"\n\
  XNOR gate right off   | 91   | \">y\"     | XNOR gate right on   | 92   | \">Y\"\n\
  XNOR gate down off    | 93   | \"vy\"     | XNOR gate down on    | 94   | \"vY\"\n\
  XNOR gate left off    | 95   | \"<y\"     | XNOR gate left on    | 96   | \"<Y\"\n\
(Press enter to return to board)" << endl;
        
        #endif
        
        getline (cin, command);
    }
    
    
    else if (subCommands [0] == "help" || subCommands [0] == "h")    // help/h    | Displays command help.
    {
        if (subCommands.size () != 1)
            return 1;
        
        cout << "\n\
Commands (arguments in <angle brackets> mean \"fill in this field\", arguments in [brackets] are optional):\n\
  <row> <column> <ID number> [<switch or button ID>]       | Places an object on the board.\n\
  run/r [<number of iterations>] [no-print/np]             | Simulate a number of frames, no-print disables screen updates until finished.\n\
  switch/s <switch ID>                                     | Toggles a switch with switch ID.\n\
  button/b <button ID>                                     | Activates a button with button ID.\n\
  resize <new width> <new height>                          | Resizes the board, preserves data currently on the board.\n\
  new <width> <height>                                     | Creates a new board.\n\
  copy/c <row 1> <column 1> <row 2> <column 2>             | Copies the selected area to a buffer.\n\
  paste/p <row> <column>                                   | Pastes data in the buffer using the row and column as the top left corner.\n\
  fill/f <row 1> <column 1> <row 2> <column 2> <ID number> | Fills the specified area with objects.\n\
  rename <new board name>                                  | Renames the current board.\n\
  save [<new board name>]                                  | Saves the current board to a file.\n\
  load [<board name>]                                      | Loads a board from a file. Excluding the board name lists available boards.\n\
  reset                                                    | Deactivates all objects on the board to fix half-powered wires.\n\
  view/v [resize] <width/row> <height/column>              | Changes the view to show only a part of the board, useful for smaller screens.\n\
  trim <extra width> <extra height>                        | Crops the board and adds a border of extra space, does not delete objects.\n\
  table/t                                                  | Prints a table showing the ID numbers of objects.\n\
  help/h                                                   | Displays command help.\n\
  exit                                                     | Exits program.\n\
(Press enter to return to board)" << endl;
        
        getline (cin, command);
    }
    
    
    else if (subCommands [0] == "exit")    // exit    | Exits program.
    {
        if (subCommands.size () != 1)
            return 1;
        else
            return 2;    // Exit program.
    }
    
    
    else if (subCommands [0] == "data")    // data    | Displays controlObjects data.     ############################### FOR DEVELOPMENT ##########################################
    {
        if (subCommands.size () != 1)
            return 1;
        
        cout << "\nContents of controlObjects:" << endl;
        for (unsigned int i = 0; i < controlObjects.size (); ++i)
            cout << "  [" << i << "] -> (" << controlObjects [i].row + 1 << ", " << controlObjects [i].column + 1 << "), sym = " << controlObjects [i].symbol << ", ch = " << controlObjects [i].changed << ", ID = " << controlObjects [i].ID () << ", state = " << controlObjects [i].active () << endl;
        cout << "(Press enter to return to board)" << endl;
        
        getline (cin, command);
    }
    
    
    else
        return 1;    // Command error.
    
    return 0;    // Command success.
}

int main ()    // Main loop.
{
    clearScreen;
    cout << "\
  /----------------------------\\\n\
  |     Circuit Simulator      |\n\
  |  Created by: Thomas Depke  |\n\
  \\----------------------------/\n\
\n\
This program allows you to create virtual digital logic circuits and\n\
simulate their behavior.\n\
\n\
Ready to create a new board.\n\
Enter the width and height, separated with a space:" << endl;
    
    string command;
    getline (cin, command);
    parseCommand (command);
    
    if (subCommands.size () != 2)
    {
        cout << "Error creating board. Exiting program." << endl;
        
        #ifndef __unix__
        system ("PAUSE");
        
        #endif
        
        return 0;
    }
    
    stringstream (subCommands [0]) >> width;
    stringstream (subCommands [1]) >> height;
    
    if (width < 1 || height < 1)
    {
        cout << "Error: Invalid dimensions. Exiting program." << endl;
        
        #ifndef __unix__
        system ("PAUSE");
        
        #endif
        
        return 0;
    }
    
    resetData ();
    
    int returnCode;
    do
    {
        cout << "\n _________________";
        drawBoard ();
        cout << "\nEnter command, type \"help\" for a command list:" << endl;
        
        returnCode = runCommand ();
        
        while (returnCode == 1)
        {
            cout << "Command error, please try again:" << endl;
            returnCode = runCommand ();
        }
    } while (returnCode != 2);
    
    cout << "\nExiting program." << endl;
    
    if (objectData != nullptr)
    {
        for (int i = 0; i < objectDataSize; ++i)    // Deallocate objectData.
            delete [] objectData [i];
        
        delete [] objectData;
    }
    
    if (copyBuffer != nullptr)
    {
        for (int i = 0; i < copyBufferHeight; ++i)    // Deallocate copyBuffer.
            delete [] copyBuffer [i];
        
        delete [] copyBuffer;
    }
    
    return 0;
}