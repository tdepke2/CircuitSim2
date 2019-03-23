CircuitSim2

Circuit Simulator project I built a while ago, now with much better functionality and GUI.

Dev notes:

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