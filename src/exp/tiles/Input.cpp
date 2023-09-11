#include <Chunk.h>
#include <tiles/Input.h>

#include <memory>

namespace tiles {

Input* Input::instance() {
    static std::unique_ptr<Input> input(new Input());
    return input.get();
}

void Input::setState(Chunk& chunk, unsigned int tileIndex, State::t state) {
    auto& tileData = getTileData(chunk, tileIndex);
    tileData.state1 = state;
}

void Input::flip(Chunk& /*chunk*/, unsigned int /*tileIndex*/, bool /*acrossHorizontal*/) {}

void Input::alternativeTile(Chunk& /*chunk*/, unsigned int /*tileIndex*/) {
    // FIXME need to open up a dialog prompt to allow configuration.
}

void Input::init(Chunk& chunk, unsigned int tileIndex, TileId::t inputId, State::t state, char keycode) {
    auto& tileData = getTileData(chunk, tileIndex);
    assert(inputId == TileId::inSwitch || inputId == TileId::inButton);
    tileData.id = inputId;
    tileData.state1 = state;
    tileData.state2 = State::disconnected;
    tileData.dir = Direction::north;
    tileData.meta = keycode;
}

}
