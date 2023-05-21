#include <gui/Signal.h>

namespace gui {

/*
unsigned int Signal::idCounter_ = 0;

unsigned int Signal::connect(std::function<void()> callback) {
    while (callbacks_.count(idCounter_)) {
        ++idCounter_;
    }
    callbacks_[idCounter_] = callback;
    return idCounter_++;
}

bool Signal::disconnect(unsigned int id) {
    auto it = callbacks_.find(id);
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
        return true;
    }
    return false;
}

void Signal::disconnectAll() {
    callbacks_.clear();
}

void Signal::emit() const {
    for (const auto& c : callbacks_) {
        c.second();
    }
}*/

int counter = 0;

}
