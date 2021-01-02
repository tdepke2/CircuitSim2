/*
Assembly compiler for my computer! :D
Compiles .ncc files (Null Computer Code).

Sample code (each instruction in order):
A = 0
A = @0
@0 = A
A = B + C
A = B - C
A = B +>> C
DATAIO A B receive input send output
A = B AND C
A = B OR C
A = B NOR C
A = B XOR C
JUMPIF A == B
JUMPIF A != B
JUMPIF A < B
JUMPIF A >= B
HALT
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

//// Class Definitions. ////

class ControlType    // Class for input and logic objects. Stores position and update/ID data for each of these objects.
{
    public:
    int row, column;   // Stores object position in objectData.
    char symbol;      // For switches and buttons, stores the ID character.
    bool changed;    // Indicates if the state of the object has changed recently.
    
    ControlType (int r, int c, char s, bool ch);
    short ID ();
};

//// Global Constants. ////

const int INSTR_WIDTH = 16;
const int TOTAL_BYTES_RAM = 16;   // 256 max (for Null 1.0).
const int TOTAL_NIBBLES_ROM = 96;    // 256 max.
const int PASTE_X = 29;
const int PASTE_Y = 166;
const string REGISTER_KEY [16] = {"0", "1", "PC", "JR", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L"};

const string BOARD_DIRECTORY = "Boards (Windows)/Computer.txt";
const string SYMBOL_INFO_TABLE [97] = {    // Windows CMD textures.
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

//// Global Variables. ////

bool binaryData [TOTAL_NIBBLES_ROM][INSTR_WIDTH] = {};
int dataLineNumber = 0;
short ** objectData = nullptr;
int width, height, objectDataSize = 0;
vector <ControlType> controlObjects;

//// Class Member Functions. ////

ControlType::ControlType (int r, int c, char s = '\0', bool ch = false)
{
    row = r;
    column = c;
    symbol = s;
    changed = ch;
}

short ControlType::ID ()    // Returns the object ID.
{
    return objectData [row][column];
}

//// Main Functions. ////

int findRegister (const string& ID)    // Converts a register code to its integer value. Returns -1 if no match found.
{
    for (int i = 0; i < 16; ++i)
    {
        if (ID == REGISTER_KEY [i])
            return i;
    }
    
    return -1;
}

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

int findSymbolInfo (const string& symbol, int start = 0, int end = 96)    // Find and return the ID of an object in SYMBOL_INFO_TABLE using symbol, from start to end inclusive. Returns -1 if no match found.
{
    for (int i = start; i <= end; ++i)
    {
        if (SYMBOL_INFO_TABLE [i] == symbol)
            return i;
    }
    
    return -1;
}

void writeBits (int column, int width, int data)    // Writes data to binaryData at column with the specified width.
{
    --width;
    
    if (dataLineNumber < 0 || dataLineNumber >= TOTAL_NIBBLES_ROM || column < 0 || column + width >= 16)
    {
        cout << "Memory Access Error: Attempted to write to (" << dataLineNumber << ", " << column << ")." << endl;
        return;
    }
    
    bool negative = false;
    if (data < 0)
    {
        negative = true;
        data *= -1;
        --data;
    }
    
    while (width >= 0)
    {
        if (data % 2 == 1)
        {
            if (!negative)
                binaryData [dataLineNumber][column + width] = 1;
        }
        else if (negative)
            binaryData [dataLineNumber][column + width] = 1;
        
        data /= 2;
        --width;
    }
}

vector <string> parseCommand (const string& command)    // Takes a command and breaks it up into substrings without spaces, returns the sub-commands as a vector.
{
    vector <string> subCommands;    // subCommands is a vector that stores a list of the commands.
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
    
    return subCommands;
}

int encodeInstr (vector <string>& instrData)    // Takes a vector of sub-commands, encodes it to binary, and saves the data into binaryData.
{
    if (instrData.size () == 0)    // Empty line.
        return 0;
    
    if (dataLineNumber >= TOTAL_NIBBLES_ROM)    // Out of ROM error.
        return 2;
    
    cout << "Instr " << dataLineNumber << ": ";
    for (unsigned int i = 0; i < instrData.size (); ++i)
        cout << "[" << instrData [i] << "] ";
    cout << endl;
    
    if (instrData [0] == "DATAIO")    // Data I/O.
    {
        if (instrData.size () < 3)
            return 1;
        
        int dest = findRegister (instrData [1]), b = findRegister (instrData [2]);
        if (dest < 0 || b < 0)
            return 1;
        
        for (unsigned int i = 3; i < instrData.size (); ++i)
        {
            if (instrData [i] == "receive")
                binaryData [dataLineNumber][7] = 1;
            else if (instrData [i] == "input")
                binaryData [dataLineNumber][6] = 1;
            else if (instrData [i] == "send")
                binaryData [dataLineNumber][5] = 1;
            else if (instrData [i] == "output")
                binaryData [dataLineNumber][4] = 1;
            else
                return 1;
        }
        
        writeBits (0, 4, 6);
        writeBits (8, 4, b);
        writeBits (12, 4, dest);
    }
    else if (instrData [0] == "JUMPIF")    // Conditional jump.
    {
        if (instrData.size () != 4)
            return 1;
        
        int a = findRegister (instrData [1]), b = findRegister (instrData [3]);
        if (a < 0 || b < 0)
            return 1;
        
        writeBits (4, 4, a);
        writeBits (8, 4, b);
        
        if (instrData [2] == "==")
            writeBits (0, 4, 11);
        else if (instrData [2] == "!=")
            writeBits (0, 4, 12);
        else if (instrData [2] == "<")
            writeBits (0, 4, 13);
        else if (instrData [2] == ">=")
            writeBits (0, 4, 14);
        else
            return 1;
    }
    else if (instrData [0] == "HALT")    // Halt program.
    {
        if (instrData.size () != 1)
            return 1;
        
        writeBits (0, 4, 15);
    }
    else if (instrData [0][0] == '@')    // Store register to memory.
    {
        if (instrData.size () != 3 || instrData [1] != "=")
            return 1;
        
        int address, a = findRegister (instrData [2]);
        stringstream (instrData [0].substr (1)) >> address;
        if (address < 0 || address >= TOTAL_BYTES_RAM || a < 0)
            return 1;
        
        writeBits (0, 4, 2);
        writeBits (4, 4, a);
        writeBits (8, 8, address);
    }
    else if (instrData.size () == 3)    // Store value/memory to register.
    {
        if (instrData [1] != "=")
            return 1;
        
        int a = findRegister (instrData [0]);
        if (a < 0)
            return 1;
        
        writeBits (4, 4, a);
        
        if (instrData [2][0] == '@')    // Store memory to register.
        {
            int address;
            stringstream (instrData [2].substr (1)) >> address;
            if (address < 0 || address >= 256)
                return 1;
            
            writeBits (0, 4, 1);
            writeBits (8, 8, address);
        }
        else    // Store value to register.
        {
            int b = findRegister (instrData [2]);    // If third token is a register instead of a value, throw an error.
            if (b >= 2)
                return 1;
            
            int value;
            stringstream (instrData [2]) >> value;
            if (value < -128 || value >= 128)
                return 1;
            
            writeBits (0, 4, 0);
            writeBits (8, 8, value);
        }
    }
    else if (instrData.size () == 5)    // Math function.
    {
        if (instrData [1] != "=")
            return 1;
        
        int dest = findRegister (instrData [0]), a = findRegister (instrData [2]), b = findRegister (instrData [4]);
        if (dest < 0 || a < 0 || b < 0)
            return 1;
        
        writeBits (4, 4, a);
        writeBits (8, 4, b);
        writeBits (12, 4, dest);
        
        if (instrData [3] == "+")
            writeBits (0, 4, 3);
        else if (instrData [3] == "-")
            writeBits (0, 4, 4);
        else if (instrData [3] == "+>>")
            writeBits (0, 4, 5);
        else if (instrData [3] == "AND")
            writeBits (0, 4, 7);
        else if (instrData [3] == "OR")
            writeBits (0, 4, 8);
        else if (instrData [3] == "NOR")
            writeBits (0, 4, 9);
        else if (instrData [3] == "XOR")
            writeBits (0, 4, 10);
        else
            return 1;
    }
    else    // Invalid instruction.
        return 1;
    
    ++dataLineNumber;
    return 0;
}

int addROMData ()
{
    cout << "\nLoading board data file: " << BOARD_DIRECTORY << endl;    // Load board data.
    ifstream loadFile (BOARD_DIRECTORY);
    
    if (!loadFile.is_open ())
    {
        cout << "Error: Unable to open " << BOARD_DIRECTORY << " for reading." << endl;
        return 1;
    }
    
    string line;
    getline (loadFile, line);
    stringstream (line) >> width;
    getline (loadFile, line);
    stringstream (line) >> height;
    
    if (width < 1 || height < 1)
    {
        cout << "Error: Invalid dimensions for board in load file." << endl;
        loadFile.close ();
        return 1;
    }
    
    objectDataSize = height;    // Set objectDataSize to remember the original size.
    
    objectData = new short * [height];   // Allocate objectData.
    for (int i = 0; i < height; ++i)
    {
        objectData [i] = new short [width];
        
        for (int j = 0; j < width; ++j)   // Fill with zeros.
            objectData [i][j] = 0;
    }
    
    getline (loadFile, line);
    for (int i = 0; i < height; ++i)
    {
        getline (loadFile, line);
        int a, position = 1;
        for (int j = 0; j < width; ++j)
        {
            if (line.substr (position, 2) == "  ")    // Check for empty space.
                position += 2;
            else if ((a = findSymbolInfo (line.substr (position, 1), 27, 30)) != -1)    // Else, check for switch/button.
            {
                if (findControlObject (line [position + 1], short ((a - 1) / 2)) != -1)
                {
                    cout << "Error: Duplicate switch/button in load file at position (" << i + 1 << ", " << j + 1 << ")." << endl;
                    return 1;
                }
                
                objectData [i][j] = a;
                controlObjects.push_back (ControlType (i, j, line [position + 1], true));
                position += 2;
            }
            else if ((a = findSymbolInfo (line.substr (position, 2), 1)) != -1)    // Else, check for non-input object (Alternate searching mechanics for Windows).
            {
                if (a >= 33)    // Logic gate.
                    controlObjects.push_back (ControlType (i, j));
                
                objectData [i][j] = a;
                position += 2;
            }
            else      // Else, object does not exist.
            {
                cout << "Error: Invalid symbol in load file at position (" << i + 1 << ", " << j + 1 << ")." << endl;
                return 1;
            }
        }
    }
    
    loadFile.close ();
    cout << "Finished loading board data." << endl;
    
    
    cout << "\nPasting ROM image data at position (" << PASTE_Y + 1 << ", " << PASTE_X + 1 << ")." << endl;    // Paste ROM image.
    if (PASTE_X < 0 || PASTE_X + INSTR_WIDTH * 2 >= width || PASTE_Y < 0 || PASTE_Y + TOTAL_NIBBLES_ROM * 3 >= height)
    {
        cout << "Error: ROM image extends off the board." << endl;
        return 1;
    }
    
    for (int i = 0; i < TOTAL_NIBBLES_ROM; ++i)
    {
        for (int j = 0; j < INSTR_WIDTH; ++j)
        {
            objectData [PASTE_Y + i * 3][PASTE_X + j * 2] = 0;
            objectData [PASTE_Y + i * 3][PASTE_X + j * 2 + 1] = 1;
        }
        
        for (int j = 0; j < INSTR_WIDTH; ++j)
        {
            objectData [PASTE_Y + i * 3 + 1][PASTE_X + j * 2] = short (binaryData [i][j] * 35);
            objectData [PASTE_Y + i * 3 + 1][PASTE_X + j * 2 + 1] = 17;
        }
        
        for (int j = 0; j < INSTR_WIDTH; ++j)
        {
            objectData [PASTE_Y + i * 3 + 2][PASTE_X + j * 2] = 19;
            objectData [PASTE_Y + i * 3 + 2][PASTE_X + j * 2 + 1] = 23;
        }
    }
    
    
    cout << "\nSaving board data to: " << BOARD_DIRECTORY << endl;    // Save board data.
    ofstream saveFile (BOARD_DIRECTORY, ios::trunc);
    
    if (!saveFile.is_open ())
    {
        cout << "Error: Unable to open " << BOARD_DIRECTORY << " for saving." << endl;
        return 1;
    }
    
    saveFile << width << endl << height << endl;
    
    saveFile << setfill ('*') << setw (width * 2 + 2) << "*" << setfill (' ') << endl;
    for (int i = 0; i < height; ++i)
    {
        saveFile << "*";
        for (int j = 0; j < width; ++j)
        {
            saveFile << SYMBOL_INFO_TABLE [objectData [i][j]];
            
            if (objectData [i][j] >= 27 && objectData [i][j] <= 30)
            {
                int a = findControlObject (i, j);
                if (a == -1)
                {
                    cout << "Error: Could not find controlObjects data for switch/button at position (" << i + 1 << ", " << j + 1 << ")." << endl;
                    return 1;
                }
                
                saveFile << controlObjects [a].symbol;
            }
        }
        saveFile << "*" << endl;
    }
    saveFile << setfill ('*') << setw (width * 2 + 2) << "*" << setfill (' ');
    
    saveFile.close ();
    cout << "Finished saving data. Process completed." << endl;
    
    return 0;
}

int main (int argc, char* argv [])    // Main loop.
{
    if (argc != 2)
    {
        cout << "\nError: Please include the filename of the program to compile as an argument." << endl;
        return EXIT_FAILURE;
    }
    
    string filename = argv [1];
    cout << "\nReading data from: " << filename << endl;
    ifstream loadFile (filename);
    
    if (!loadFile.is_open ())
    {
        cout << "Error: Unable to open " << filename << " for reading." << endl;
        return EXIT_FAILURE;
    }
    
    int lineNumber = 1;
    string line;
    bool commentBlock = false;
    cout << "Tokenizing..." << endl;
    while (!loadFile.eof ())
    {
        getline (loadFile, line);
        
        size_t commentPos = line.find ("//");
        if (commentPos != string::npos)
            line.resize (commentPos);
        
        if (commentBlock)
        {
            size_t blockPos = line.find ("*/");   // The /* and */ should not be placed on the same line for comment blocks to work.
            if (blockPos != string::npos)
            {
                line = line.substr (blockPos + 2);
                commentBlock = false;
            }
        }
        
        if (!commentBlock)
        {
            size_t blockPos = line.find ("/*");
            if (blockPos != string::npos)
            {
                line.resize (blockPos);
                commentBlock = true;
            }
            
            vector <string> instrData = parseCommand (line);
            
            int encodeResult = encodeInstr (instrData);
            if (encodeResult == 1)
            {
                cout << "Error: Invalid instruction at line " << lineNumber << "." << endl;
                loadFile.close ();
                return EXIT_FAILURE;
            }
            else if (encodeResult == 2)
            {
                cout << "Error: Out of program ROM for instruction at line " << lineNumber << "." << endl;
                loadFile.close ();
                return EXIT_FAILURE;
            }
        }
        
        ++lineNumber;
    }
    
    loadFile.close ();
    cout << "Finished reading data." << endl;
    
    
    cout << "\nGenerated machine code:" << endl;    // Output machine code data.
    cout << "Op   A    B    Dest" << endl;
    for (int i = 0; i < TOTAL_NIBBLES_ROM; ++i)
    {
        for (int j = 0; j < INSTR_WIDTH; ++j)
        {
            if (j > 0 && j % 4 == 0)
                cout << " ";
            
            cout << binaryData [i][j];
        }
        
        cout << endl;
    }
    
    int finalStatus = addROMData ();
    
    if (objectData != nullptr)
    {
        for (int i = 0; i < objectDataSize; ++i)    // Deallocate objectData.
            delete [] objectData [i];
        
        delete [] objectData;
    }
    
    if (finalStatus == 0)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}