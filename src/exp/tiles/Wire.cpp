#include <tiles/Wire.h>

#include <memory>

/*
Wire::Wire(WireType::t type, Direction::t direction, State::t state1, State::t state2) {
    type_ = type;
    if (type == WireType::straight) {
        direction_ = static_cast<Direction::t>(direction % 2);
    } else if (type == WireType::junction || type == WireType::crossover) {
        direction_ = Direction::north;
    } else {
        direction_ = direction;
    }
    state1_ = state1;
    if (type == WireType::crossover) {
        state2_ = state2;
    } else {
        state2_ = State::low;
    }
}*/

Wire* Wire::instance() {
    static std::unique_ptr<Wire> wire(new Wire());
    return wire.get();
}

bool Wire::getHighlight(Chunk& chunk, unsigned int tileIndex) const {
    std::cout << "getHighlight()\n";
    return false;
}

void Wire::setHighlight(Chunk& chunk, unsigned int tileIndex, bool highlight) {
    std::cout << "setHighlight()\n";
}
