#include <Chunk.h>
#include <tiles/Input.h>

#include <memory>
#include <spdlog/spdlog.h>

namespace tiles {

Input* Input::instance() {
    static std::unique_ptr<Input> input(new Input());
    return input.get();
}

void Input::setKeycode(Chunk& chunk, unsigned int tileIndex, char keycode) {
    modifyTileData(chunk, tileIndex).meta = keycode;
}

char Input::getKeycode(const Chunk& chunk, unsigned int tileIndex) const {
    return static_cast<char>(getTileData(chunk, tileIndex).meta);
}

void Input::setState(Chunk& chunk, unsigned int tileIndex, State::t state) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    tileData.state1 = state;
}

void Input::alternativeTile(Chunk& /*chunk*/, unsigned int /*tileIndex*/) {
    // FIXME need to open up a dialog prompt to allow configuration.
}

void Input::cloneTo(const Chunk& chunk, unsigned int tileIndex, Tile target) {
    const auto& tileData = getTileData(chunk, tileIndex);
    init(target.getChunk(), target.getIndex(), tileData.id, tileData.state1, tileData.meta);
}

Input::Input() {
    spdlog::debug("Input class has been constructed.");
}

void Input::init(Chunk& chunk, unsigned int tileIndex, TileId::t inputId, State::t state, char keycode) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    assert(inputId == TileId::inSwitch || inputId == TileId::inButton);
    tileData.id = inputId;
    tileData.state1 = state;
    tileData.state2 = State::disconnected;
    tileData.dir = Direction::north;
    tileData.meta = keycode;
}

} // namespace tiles
