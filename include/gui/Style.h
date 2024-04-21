#pragma once

namespace gui {
    class Gui;
}

namespace gui {

class Style {
public:
    Style(const Gui& gui);
    virtual ~Style() = default;

protected:
    const Gui& gui_;
};

} // namespace gui
