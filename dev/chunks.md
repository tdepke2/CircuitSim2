A board has two display options for the circuit:
* Fixed size, the board size is a multiple of the chunk width.
* Unlimited size.

## Tile Data ##

Tiles should have a 32-bit integer position on the board. Tile positions are relative to their chunk and chunk coordinates will also use 32-bit integers.

Each tile is represented with a 32-bit value:

Bits:   fedcba98 76543210       f           e            dc           ba        98        765         43210
Count:  16b                     1b          1b           2b           2b        2b        3b          5b
Data:   Metadata (unique ID)    Reserved    Highlight    Direction    State2    State1    Reserved    Tile type

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

override operator= to allow a tile to change its type?
