#pragma once

#include <memory>
#include <SFML/Graphics.hpp>
#include <vector>

namespace gui {
    class Widget;
}

namespace gui {

class Container {
public:
    void addChild(std::shared_ptr<Widget> c);
    const std::vector<std::shared_ptr<Widget>>& getChildren() const;

    void forwardMousePress(sf::Mouse::Button button, int x, int y);

private:
    std::vector<std::shared_ptr<Widget>> children_;
};

}
