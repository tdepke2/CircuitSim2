#ifndef _USERINTERFACE_H
#define _USERINTERFACE_H

class Simulator;

#include <functional>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <windows.h>

using namespace std;
using namespace sf;

struct TextButton : public Drawable, public Transformable {
    Text text;
    RectangleShape button;
    Color color1, color2;
    function<void(void)> action;
    bool visible, selected;
    
    TextButton();
    TextButton(const Font& font, const string& buttonText, const Color& textColor, unsigned int charSize, float x, float y, const Color& color1, const Color& color2, function<void(void)> action);
    bool update(int mouseX, int mouseY, bool clicked);
    
    private:
    virtual void draw (RenderTarget& target, RenderStates states) const;
};

struct DropdownMenu : public Drawable, public Transformable {
    TextButton button;
    RectangleShape background;
    vector<TextButton> menuButtons;
    float maxMenuButtonWidth;
    bool visible;
    
    DropdownMenu();
    DropdownMenu(const TextButton& button, const Color& backgroundColor);
    void addMenuButton(const TextButton& menuButton);
    void update(int mouseX, int mouseY, bool clicked);
    
    private:
    virtual void draw (RenderTarget& target, RenderStates states) const;
};

class UserInterface : public Drawable, public Transformable {
    public:
    Font font;
    RectangleShape topBar;
    DropdownMenu fileMenu;
    
    UserInterface();
    void update(int mouseX, int mouseY, bool clicked);
    
    private:
    virtual void draw (RenderTarget& target, RenderStates states) const;
};

#endif