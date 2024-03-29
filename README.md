# CircuitSim2 #

![image1](image1.png)

## About This Project ##

CircuitSim2 is an open-source development platform for digital logic circuits. Circuits are designed by placing tiles onto a grid, each element (such as a wire, switch, LED, or logic gate) takes up one of the grid squares. Tiles that are adjacent to one another and connect together will transfer a logic signal (either low, high, or tri-state). By combining lots of these gates and wires into a module to abstract the design, some complex circuitry can be created such as state machines, computational hardware, and even computers.

This project is ideal if you need a quick prototype for a logic circuit when working closely with low level hardware. Using Verilog or just building a circuit on a breadboard are other alternatives to this, but these methods can take some time. As a bonus, it is very easy to see the inner workings of circuits built with this tool and this makes debugging a breeze. If you've never even heard of digital logic before then you might be interested in this if you like to create stuff. Some tutorials are provided to cover the basics.

## Platform and Usage ##

~~Currently, CircuitSim2 is only available for Windows but a Linux/MacOS build may be available in the future.~~

[The latest build can be found here.](https://github.com/tdepke2/CircuitSim2/releases/download/v1.1/CircuitSim2-1.1-x86.zip)

Just unzip the file and it should be good to go. If running it gives an error that MSVCPxyz.dll is missing then just run the installer for the Visual C++ runtime in "redist/vcredist_x86.exe" and try again. If you prefer to compile this project yourself, the latest build was compiled with Visual Studio 2017 and SFML 2.5.1 (but newer versions should work as well).

Update: CMake is now used for building. A build can be run with the following after cloning this repository:

For single-configuration generator (such as Linux and MacOS with Make/clang):
```
# On my Arch Linux machine, I needed to install: kdialog
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

For multi-configuration generator (such as Windows with Visual Studio):
```
cmake -S . -B build
cmake --build build --config Release
```

### Getting Started ###

To learn about some of the basic controls and tools that are available with this application, some tutorials are provided in the "boards/tutorials" directory (use File->Open... to view these). If you want to see some of the larger examples, the calculator and computer boards are cool ones to check out.

That's all for now, happy circuit building!

## Screenshots ##

![image2](image2.png)

Some examples of components that can be used to construct circuits, and the wire tool making a new path.

![image3](image3.gif)

Quick demo of a binary-to-decimal converter in action. The binary input is provided at the bottom of the circuit.

![image4](image4.png)

A carry lookahead adder computing 31 + 9. The result can be easily converted to decimal and checked with the query tool.
