#ifndef _USERINTERFACE_H
#define _USERINTERFACE_H

class Simulator;

#include <functional>
#include <SFML/Graphics.hpp>
#include <windows.h>

using namespace std;
using namespace sf;

struct TextButton {
    Text text;
    RectangleShape button;
    Color color1, color2;
    function<void(void)> action;
    bool visible, selected;
    
    TextButton();
    TextButton(const Font& font, const string& buttonText, const Color& textColor, unsigned int charSize, float x, float y, const Color& color1, const Color& color2, function<void(void)> action);
    bool update(int mouseX, int mouseY, bool clicked);
};

class UserInterface : public Drawable, public Transformable {
    public:
    Font font;
    RectangleShape topBar;
    TextButton testButton;
    
    UserInterface();
    void update(int mouseX, int mouseY, bool clicked);
    
    private:
    virtual void draw (RenderTarget& target, RenderStates states) const;
};

#endif