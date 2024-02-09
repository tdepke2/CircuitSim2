A board has two display options for the circuit:

* Fixed size, the board size is a multiple of the chunk width.
* Unlimited size.

## Tile Data ##

Tiles should have a 32-bit integer position on the board. Tile positions are relative to their chunk and chunk coordinates will also use 32-bit integers.

Each tile is represented with a 32-bit value:

```
Bits:   fedcba98 76543210       f           e            dc           ba        98        765         43210
Count:  16b                     1b          1b           2b           2b        2b        3b          5b
Data:   Metadata (unique ID)    Reserved    Highlight    Direction    State2    State1    Reserved    Tile type
```

## Highlighting ##

Individual tiles can be highlighted. Can use Ctrl+Drag to add another highlighted section or Ctrl+RightClick to add a single tile.
This allows copying disjunctive areas. These could copy as a single rectangular area and then a key could be held to paste tiles besides blanks (ctrl or alt? I think shift does force-paste).

The cursor may need to use a different color and render as an overlay to avoid conflicts (or we could use an extra bit in the tile data).

Highlights are tracked per chunk instead of individually in a global data structure (assume that a highlighted area may be very large).



Simulation: chunks with scheduled updates check their gates, wires are updated and new updates queued (this is all done with raw data, no fancy Tile structures).

Copy/paste: tiles are copied between chunks, wires will need to get new id's and same for labels.

Select and rotate: each tile in selected chunk area needs to rotate, three options:

1. convert tile to child class of specific type, call rotate, and convert back
2. convert tile to single tile class, call rotate, and convert back
3. call rotate function in chunk

what if we could get a tile from a chunk, and it would give an interface to modify the tile data.
maybe have some tile "modifiers" (stateless classes) and when a tile is accessed from chunk, return a small object containing the reference to the tile and the type of "modifier".

```cpp
class Modifier {
    rotate(ref);
    highlight(ref);
    altTile(ref);
}

class WireMod : public Modifier {
    ...
}

class Tile {
public:
    rotate() {
        mod->rotate(ref);
    }
    highlight();
    altTile();

private:
    Reference ref;
    Modifier* mod;
}
```

override operator= to allow a tile to change its type?

## Rendering ##

Tiles are rendered in the rectangular region the user is viewing within the application window.
Some things to note about the view and the window:

* The window can change size.
* The view can zoom (change scale).
* The view can move (change center).

In order to render the tiles, we'll use a RenderTexture to cache previously rendered chunks and a VertexBuffer to map to the chunks in the RenderTexture.
This will be done for multiple levels-of-detail (selected based on the zoom level).
Chunks still render by drawing a VertexArray to the RenderTexture, updates should be infrequent enough to not matter.
The size of the RT and VBO is dependent on the window size, if the window width increases then double the width of RT and VBO as needed.
For LOD 0, the size of RT and VBO is equal and large enough to fit the window size (at the max before the next LOD) with one extra row and column of chunks.
For the next LOD, VBO should double in width and height but the texture size will not change (the texture gets subdivided).

Chunks need a dirty flag to track when redraw is needed (per LOD, make it a bitset?).
When drawing chunks we should iterate through visible ones and check for the dirty flags.
Chunks render to a reserved spot in the LOD texture (maybe tracked as part of the chunk data?), this is also per LOD.
New chunks that appear on screen need to be allocated a spot in LOD texture, when we run out of space some may need to be deallocated.
Deallocation can be done for off-screen chunks. We could select the furthest one (optimal), the oldest drawn, or the first we find.
We will need to reserve a spot for the empty chunk, storing it as a member of the board could allow it to be treated the same way as the rest of the chunks instead of needing a special spot for it.

Allocation data structure: `std::map<unsigned int, uint64_t>` (the int is an index into the texture)
Wait, maybe just make it a vector?
Easy to find first available (just loop through) and probably still fast enough to find furthest by searching for largest Manhattan distance (or is it actually the Chebyshev distance?).

## New File Format ##

```
<board-name>
|-- region
|   |-- 0.0.dat
|   |-- 1.1.dat
|   '-- -1.-1.dat
'-- board.txt
```

The `board.txt` file is similar to the legacy format, only storing version and some metadata. Each "region" is a 32 by 32 chunk area where the chunk data is saved to a file in the region directory.

Each region file is a binary file containing a 4096 byte header followed by data for each chunk. The header is a lookup table matching each of the 1024 chunks in the region to a 1024 byte sector offset in the file (follows an X -> Y ordering starting from zero, offset is 32 bits). This is based very similarly to the MC Region file format: https://minecraft.wiki/w/Region_file_format

## Undo and Redo ##

The editor needs the ability to undo/redo edits made to the board, use the command design pattern to accomplish this.

The following should be undoable operations:
* Place single tile (or fill area).
* Paste tiles (or fill area).
* Flip/rotate/toggle/edit tile (or area).
* Delete (and therefore cut too).
* Placing wire with wire tool.
* Running the simulation.
    * This could be undone by caching recently modified chunks when the sim pauses. If we track modification times for each chunk it should be easy, using the save state of a chunk may be an alternative option but probably has drawbacks.
    * What if an edit is made while the sim is running? How can the edit be undone without reverting back to when it was paused?

The following should not be undoable:
* Selections (with the exceptions that undo/redo should deselect everything, and delete should re-select the area).
* Copying an area.
* Save/load.

Example: place single tile on the board.
1. Create a commands::WriteTile, passing the state required to write the tile.
2. Command is executed and appended to the edit history deque (could use vector instead?).
3. Execution clones the target tile to a temporary chunk (managed by a class the Editor knows about).
    1. This class has an array of chunks to store the tiles, the chunks are filled in a linear order until full.
    2. We could keep indices to the first and last allocated tiles across the group of chunks. When a command destructs, it should replace any allocated tiles with blanks and potentially update these indices.
    3. Keep in mind that the command may clone a blank into the temp chunk, so we can't rely on checking for blanks to advance the lower index.
4. Execution swaps the cloned target tile with the to-be-replaced one.
5. To undo, we just swap them again. To redo, we swap again.

Edit grouping: it would be efficient to group similar edits together into one command instead of a lot of nearly identical ones. Edits should be grouped based on the time they occurred. Grouping could be done by modifying the current command in the history to include an additional change. This would be ideal for implementing the fill-tiles-in-area and wire tool commands.
