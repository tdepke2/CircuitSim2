#ifndef _USERINTERFACE_H
#define _USERINTERFACE_H

#include <functional>
#include <list>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

using namespace std;
using namespace sf;

class UIComponent : public Drawable, public Transformable {    // Abstract base class for all drawable elements of the user interface.
    public:
    bool visible;
    
    UIComponent();
    virtual ~UIComponent();
    virtual bool update(int mouseX, int mouseY, bool clicked);
    virtual bool update(Event::TextEvent textEvent);
    
    private:
    virtual void draw(RenderTarget& target, RenderStates states) const = 0;
};

class TextButton : public UIComponent {    // Represents a clickable button with text on it. The button can call an action when pressed.
    public:
    Text text;
    RectangleShape button;
    Color color1, color2;
    function<void(int)> action;
    int actionOption;
    bool selected;
    
    TextButton();
    TextButton(const string& buttonText, const Color& textColor, unsigned int charSize, float x, float y, const Color& color1, const Color& color2, function<void(int)> action, int actionOption = 0);
    bool update(int mouseX, int mouseY, bool clicked);
    
    private:
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

class DropdownMenu : public UIComponent {    // A dropdown menu for buttons.
    public:
    TextButton button;
    RectangleShape background;
    vector<TextButton> menuButtons;
    float maxMenuButtonWidth;
    
    DropdownMenu();
    DropdownMenu(const TextButton& button, const Color& backgroundColor);
    void addMenuButton(const TextButton& menuButton);
    bool update(int mouseX, int mouseY, bool clicked);
    
    private:
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

class TextField : public UIComponent {    // A text field for string input. Text can be entered when selected and the position of the caret can be moved by clicking.
    public:
    Text label, field;
    RectangleShape background, caret;
    int caretPosition, maxCharacters;
    bool selected, isEditable;
    
    TextField();
    TextField(const string& labelText, const string& initialFieldText, const Color& textColor, unsigned int charSize, float x, float y, const Color& fillColor, const Color& outlineColor, int maxCharacters, bool isEditable = true);
    void setString(const string& s);
    bool update(int mouseX, int mouseY, bool clicked);
    bool update(Event::TextEvent textEvent);
    
    private:
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

class CheckBox : public UIComponent {    // A check box that can be clicked to toggle a setting.
    public:
    Text text;
    RectangleShape button, check;
    Color checkColor;
    
    CheckBox();
    CheckBox(const string& buttonText, const Color& textColor, unsigned int charSize, float x, float y, const Color& buttonColor, const Color& checkColor, bool startChecked = false);
    bool isChecked() const;
    void setChecked(bool checked);
    bool update(int mouseX, int mouseY, bool clicked);
    
    private:
    bool _checked;
    
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

class DialogPrompt : public UIComponent {    // A dialog box with buttons and text fields.
    public:
    Text text;
    RectangleShape background;
    vector<TextButton> optionButtons;
    vector<TextField> optionFields;
    vector<CheckBox> optionChecks;
    
    DialogPrompt();
    DialogPrompt(const string& dialogText, const Color& textColor, unsigned int charSize, float x, float y, const Color& fillColor, const Color& outlineColor, const Vector2f& size);
    ~DialogPrompt();
    bool update(int mouseX, int mouseY, bool clicked);
    bool update(Event::TextEvent textEvent);
    void clearFields();
    void show();
    
    private:
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

class UserInterface : public UIComponent {    // The window UI. Includes the top bar menus and all prompts.
    friend class DialogPrompt;
    
    public:
    static TextField* fieldToSelectPtr;
    static bool isDialogPromptOpen();
    static void closeAllDialogPrompts(int option = 0);
    static void pushMessage(const string& s, bool isError = false);
    static void updateMessages();
    RectangleShape topBar;
    DropdownMenu fileMenu, viewMenu, runMenu, toolsMenu, wireMenu, inputMenu, outputMenu, gateMenu;
    TextButton tpsDisplay, tickCounter;
    DialogPrompt savePrompt, renamePrompt, resizePrompt, relabelPrompt, configPrompt, queryPrompt;
    
    UserInterface();
    bool update(int mouseX, int mouseY, bool clicked);
    bool update(Event::TextEvent textEvent);
    void update(Event::SizeEvent sizeEvent);
    
    private:
    static bool _dialogPromptOpen;
    static vector<DialogPrompt*> _dialogPrompts;
    static list<TextButton> _messageList;
    static Clock _messageClock;
    Tile* _relabelTargetTile;
    
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

#endif