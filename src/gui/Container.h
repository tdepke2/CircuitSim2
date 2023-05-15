#pragma once

#include <memory>
#include <vector>

namespace gui {
    class Widget;
}

namespace gui {

class Container {
public:
    void addChild(std::shared_ptr<Widget> c);
    const std::vector<std::shared_ptr<Widget>>& getChildren() const;

private:
    std::vector<std::shared_ptr<Widget>> children_;
};

}
