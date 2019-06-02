# CircuitSim2 #

![image1](/image1.png)

## About This Project ##

CircuitSim2 is a developement and testing platform for digital logic circuits. Circuits are designed by placing tiles onto a grid, each element (such as a wire, switch, LED, or logic gate) takes up one of the grid squares. Tiles that are adjacent to one another and connect together will transfer a logic signal (either LOW or HIGH). By combining lots of these gates and wires into a module to abstract the design, some complex circuitry can be created such as state machines, computational hardware, and even computers.

## Platform and Usage ##

Currently, CircuitSim2 is only available for Windows but a Linux/MacOS build may be available in the future.

The latest build [can be found here](https://github.com/tdepke2/CircuitSim2/releases) and includes all required dependencies in the application so no installation is required. Just unzip the file and it should be good to go.

### Getting Started ###

To learn the basics of building circuits with this application, some tutorials are provided in the boards/tutorials directory (use File->Open... to view these). to be continued...

Dev notes:

![image2](/image2.png)

__Controls list__
* Left click to drag view.
* Scroll to zoom.
* Right click to deselect all and select a tile or place down current selection.
    * For selection, if right click spans multiple tiles the rectangular area is selected.
        * Arrow keys to move selection, does not perform a physical cut-paste and will overwrite underlying tiles.
        * Ctrl+C to copy selection (keeps area selected and prepares to paste).
        * If a tile is placed into the selection then all tiles in selection are filled with the tile, similar thing applies when placing copy buffer into selection.
* Escape to deselect all.
* E to interact with object (changes state or type with most tiles), this is applied to tile under cursor if nothing selected, else the action is applied to all selected tiles if nothing is getting pasted, else the action is applied to the copy buffer.
* Ctrl+V to paste, Ctrl+X to cut, keeps selection as is and prepares to paste, right click actually places it.
    * Allowed to paste over blank tiles only, not allowed when selection would extend outside the board.
    * Shift+Right click will override this behavior, in the case that a selection extends the bounds only part of the selection is copied over.
