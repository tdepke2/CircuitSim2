#include <gui/Container.h>
#include <gui/Widget.h>

namespace gui {

void Container::addChild(std::shared_ptr<Widget> c) {
    children_.push_back(c);
}

const std::vector<std::shared_ptr<Widget>>& Container::getChildren() const {
    return children_;
}

}
