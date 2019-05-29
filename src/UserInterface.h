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
    function<void(int)> action;
    int actionOption;
    bool visible, selected;
    
    TextButton();
    TextButton(const string& buttonText, const Color& textColor, unsigned int charSize, float x, float y, const Color& color1, const Color& color2, function<void(int)> action, int actionOption = 0);
    bool update(int mouseX, int mouseY, bool clicked);
    
    private:
    virtual void draw(RenderTarget& target, RenderStates states) const;
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
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

struct TextField : public Drawable, public Transformable {
    Text label, field;
    RectangleShape background, caret;
    int caretPosition, maxCharacters;
    bool visible, selected;
    
    TextField();
    TextField(const string& labelText, const string& initialFieldText, const Color& textColor, unsigned int charSize, float x, float y, const Color& fillColor, const Color& outlineColor, int maxCharacters);
    void update(int mouseX, int mouseY, bool clicked);
    void update(Event::TextEvent textEvent);
    void clear();
    
    private:
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

struct DialogPrompt : public Drawable, public Transformable {
    Text text;
    RectangleShape background;
    vector<TextButton> optionButtons;
    vector<TextField> optionFields;
    bool visible;
    
    DialogPrompt();
    DialogPrompt(const string& dialogText, const Color& textColor, unsigned int charSize, float x, float y, const Color& fillColor, const Color& outlineColor, const Vector2f& size);
    ~DialogPrompt();
    void update(int mouseX, int mouseY, bool clicked);
    void update(Event::TextEvent textEvent);
    void clearFields();
    void show();
    
    private:
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

class UserInterface : public Drawable, public Transformable {
    friend struct DialogPrompt;
    public:
    static bool isDialogPromptOpen();
    static void closeAllDialogPrompts(int option = 0);
    RectangleShape topBar;
    DropdownMenu fileMenu, viewMenu, runMenu, toolsMenu, wireMenu, inputMenu, outputMenu, gateMenu;
    TextButton upsDisplay;
    DialogPrompt savePrompt, renamePrompt, resizePrompt, relabelPrompt;
    
    UserInterface();
    void update(int mouseX, int mouseY, bool clicked);
    void update(Event::TextEvent textEvent);
    
    private:
    static bool _dialogPromptOpen;
    static vector<DialogPrompt*> _dialogPrompts;
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

#endif