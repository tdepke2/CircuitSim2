A board has two display options for the circuit:
* Fixed size, the board size is a multiple of the chunk width.
* Unlimited size.

Tiles should have a 32-bit integer position on the board. Tile positions are relative to their chunk and chunk coordinates will also use 32-bit integers.

Each tile is represented with a 32-bit value:

Bits:   01234567 89abcdef       01          23           45        67        89abcdef
Count:  16b                     2b          2b           2b        2b        8b
Data:   Metadata (unique ID)    Reserved    Direction    State2    State1    Tile type
