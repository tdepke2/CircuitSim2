/**
 * Important note on lambda captures:
 * 
 * When wiring widget signals to functions, it is easy to use the `[=]` capture
 * to capture all nested variables by value. The widgets are std::shared_ptr
 * objects though, so a cyclic reference is created if a widget refers to itself
 * within the lambda and will never get deleted! Using a capture by reference
 * with `[&]` is not an option as the std::shared_ptr goes out of scope, so
 * either create a temporary raw pointer or std::weak_ptr to the widget and pass
 * this to the lambda:
 * 
 * ```
 * auto greetButton = gui::Button::create(theme);
 * auto greetButtonPtr = greetButton.get();
 * greetButton->onClick.connect([greetButtonPtr]{
 *     greetButtonPtr->setLabel("nice to meet you");
 * });
 * ```
 * 
 * In general it is not recommended to put std::shared_ptr in a lambda capture
 * as it is difficult to track down potential cycles, neither is it recommended
 * to use default captures (`[&]` and `[=]`). Some of this demo code does this
 * anyways (there's a memory leak detector in here that makes sure the widgets
 * actually get cleaned up).
 * 
 * In C++14 the lambda can be created without a temporary by using a capture
 * with an initializer:
 * 
 * ```
 * greetButton->onClick.connect([button = std::weak_ptr<gui::Button>(greetButton)]{
 *     button.lock()->setLabel("nice to meet you");
 * });
 * ```
 */

#include <gui/Gui.h>
#include <gui/themes/DefaultTheme.h>
#include <gui/Timer.h>
#include <gui/widgets/Button.h>
#include <gui/widgets/ChatBox.h>
#include <gui/widgets/CheckBox.h>
#include <gui/widgets/ColorPicker.h>
#include <gui/widgets/DialogBox.h>
#include <gui/widgets/Group.h>
#include <gui/widgets/Label.h>
#include <gui/widgets/MenuBar.h>
#include <gui/widgets/MultilineTextBox.h>
#include <gui/widgets/Panel.h>
#include <gui/widgets/RadioButton.h>
#include <gui/widgets/Slider.h>
#include <gui/widgets/TextBox.h>

#include <array>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

std::map<gui::Widget*, std::string> widgetNames;
std::map<gui::Widget*, std::weak_ptr<gui::Widget>> widgetsAllocated;

// Signal callbacks (for debugging).
void mouseEnter(gui::Widget* w) {
    std::cout << "  onMouseEnter(\t\t" << w << " " << widgetNames[w] << ")\n";
}
void mouseLeave(gui::Widget* w) {
    std::cout << "  onMouseLeave(\t\t" << w << " " << widgetNames[w] << ")\n";
}
void focusGained(gui::Widget* w) {
    std::cout << "  onFocusGained(\t" << w << " " << widgetNames[w] << ")\n";
}
void focusLost(gui::Widget* w) {
    std::cout << "  onFocusLost(\t\t" << w << " " << widgetNames[w] << ")\n";
}
void mousePress(gui::Widget* w, sf::Mouse::Button button, const sf::Vector2f& pos) {
    std::cout << "  onMousePress(\t\t" << w << " " << widgetNames[w] << ", \t"
    << button << ", \t(" << pos.x << ", " << pos.y << "))\n";
}
void mouseRelease(gui::Widget* w, sf::Mouse::Button button, const sf::Vector2f& pos) {
    std::cout << "  onMouseRelease(\t" << w << " " << widgetNames[w] << ", \t"
    << button << ", \t(" << pos.x << ", " << pos.y << "))\n";
}
void click(gui::Widget* w, const sf::Vector2f& pos) {
    std::cout << "  onClick(\t\t" << w << " " << widgetNames[w] << ", \t\t("
    << pos.x << ", " << pos.y << "))\n";
}
void textChange1(gui::Widget* w, const sf::String& text) {
    std::cout << "  onTextChange(\t" << w << " " << widgetNames[w] << ", \t\""
    << text.toAnsiString() << "\")\n";
}
void textChange2(gui::Widget* w, size_t textLength) {
    std::cout << "  onTextChange(\t" << w << " " << widgetNames[w] << ", \t"
    << textLength << ")\n";
}
void enterPressed(gui::Widget* w, const sf::String& text) {
    std::cout << "  onEnterPressed(\t" << w << " " << widgetNames[w] << ", \t\""
    << text.toAnsiString() << "\")\n";
}
void menuItemClick(gui::Widget* w, const gui::MenuList& menu, size_t index) {
    std::cout << "  onMenuItemClick(\t" << w << " " << widgetNames[w] << ", \t"
    << &menu << ", " << index << ")\n";
}
void valueChange(gui::Widget* w, float value) {
    std::cout << "  onValueChange(\t" << w << " " << widgetNames[w] << ", \t"
    << value << ")\n";
}
void colorChange(gui::Widget* w, const sf::Color& color) {
    std::cout << "  onColorChange(\t" << w << " " << widgetNames[w] << ", \t("
    << static_cast<int>(color.r) << ", \t" << static_cast<int>(color.g) << ", \t"
    << static_cast<int>(color.b) << ", \t" << static_cast<int>(color.a) << ")\n";
}

void connectDebugSignals(const std::shared_ptr<gui::Widget>& widget, const std::string& name) {
    assert(widgetNames.emplace(widget.get(), name).second);
    assert(widgetsAllocated.emplace(widget.get(), widget).second);
    widget->onMouseEnter.connect(mouseEnter);
    widget->onMouseLeave.connect(mouseLeave);
    widget->onFocusGained.connect(focusGained);
    widget->onFocusLost.connect(focusLost);
}
void connectDebugSignals(const std::shared_ptr<gui::Button>& button, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Widget>(button), name);
    button->onMousePress.connect(mousePress);
    button->onMouseRelease.connect(mouseRelease);
    button->onClick.connect(click);
}
void connectDebugSignals(const std::shared_ptr<gui::ChatBox>& chatBox, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Widget>(chatBox), name);
    chatBox->onMousePress.connect(mousePress);
    chatBox->onMouseRelease.connect(mouseRelease);
    chatBox->onClick.connect(click);
}
void connectDebugSignals(const std::shared_ptr<gui::CheckBox>& checkBox, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Button>(checkBox), name);
}
void connectDebugSignals(const std::shared_ptr<gui::ColorPicker>& colorPicker, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Group>(colorPicker), name);
    colorPicker->onColorChange.connect(colorChange);
}
void connectDebugSignals(const std::shared_ptr<gui::DialogBox>& dialogBox, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Group>(dialogBox), name);
}
void connectDebugSignals(const std::shared_ptr<gui::Group>& group, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Widget>(group), name);
}
void connectDebugSignals(const std::shared_ptr<gui::Label>& label, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Widget>(label), name);
}
void connectDebugSignals(const std::shared_ptr<gui::MenuBar>& menuBar, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Widget>(menuBar), name);
    menuBar->onMousePress.connect(mousePress);
    menuBar->onMouseRelease.connect(mouseRelease);
    menuBar->onClick.connect(click);
    menuBar->onMenuItemClick.connect(menuItemClick);
}
void connectDebugSignals(const std::shared_ptr<gui::MultilineTextBox>& multilineTextBox, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Widget>(multilineTextBox), name);
    multilineTextBox->onMousePress.connect(mousePress);
    multilineTextBox->onMouseRelease.connect(mouseRelease);
    multilineTextBox->onClick.connect(click);
    multilineTextBox->onTextChange.connect(textChange2);
}
void connectDebugSignals(const std::shared_ptr<gui::Panel>& panel, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Group>(panel), name);
}
void connectDebugSignals(const std::shared_ptr<gui::RadioButton>& radioButton, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Button>(radioButton), name);
}
void connectDebugSignals(const std::shared_ptr<gui::Slider>& slider, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Widget>(slider), name);
    slider->onValueChange.connect(valueChange);
}
void connectDebugSignals(const std::shared_ptr<gui::TextBox>& textBox, const std::string& name) {
    connectDebugSignals(std::dynamic_pointer_cast<gui::Widget>(textBox), name);
    textBox->onMousePress.connect(mousePress);
    textBox->onMouseRelease.connect(mouseRelease);
    textBox->onClick.connect(click);
    textBox->onTextChange.connect(textChange1);
    textBox->onEnterPressed.connect(enterPressed);
}

sf::Color getRandColor() {
    return {static_cast<uint8_t>(std::rand() % 256), static_cast<uint8_t>(std::rand() % 256), static_cast<uint8_t>(std::rand() % 256)};
}

void createButtonDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto greetButton = gui::Button::create(theme);
    connectDebugSignals(greetButton, "greetButton");
    greetButton->setLabel(sf::String("hello!"));
    greetButton->setRotation(18.0f);
    greetButton->setOrigin(greetButton->getSize() * 0.5f);
    greetButton->setPosition(130.0f, 100.0f);
    greetButton->setScale(1.6f, 2.5f);
    auto greetButtonPtr = greetButton.get();
    greetButton->onClick.connect([greetButtonPtr]{ greetButtonPtr->setLabel("nice to meet you"); });
    myGui.addChild(greetButton);

    auto hideButton = gui::Button::create(theme);
    connectDebugSignals(hideButton, "hideButton");
    hideButton->setLabel("click to hide");
    hideButton->setPosition(10.0f, 10.0f);
    auto hideButtonPtr = hideButton.get();
    hideButton->onClick.connect([hideButtonPtr]{ hideButtonPtr->setVisible(false); });
    myGui.addChild(hideButton);

    auto disableButton = gui::Button::create(theme);
    connectDebugSignals(disableButton, "disableButton");
    disableButton->setLabel("click to disable");
    disableButton->setPosition(200.0f, 10.0f);
    auto disableButtonPtr = disableButton.get();
    disableButton->onClick.connect([disableButtonPtr]{ disableButtonPtr->setEnabled(false); });
    myGui.addChild(disableButton);

    auto resetButton = gui::Button::create(theme);
    connectDebugSignals(resetButton, "resetButton");
    resetButton->setLabel("click to reset others");
    resetButton->setPosition(10.0f, 70.0f);
    resetButton->onClick.connect([=]{ hideButton->setVisible(true); disableButton->setEnabled(true); });
    myGui.addChild(resetButton);

    auto buttonA = gui::Button::create(theme);
    connectDebugSignals(buttonA, "buttonA");
    buttonA->setLabel("A (front)");
    buttonA->setPosition(10.0f, 170.0f);
    auto buttonAPtr = buttonA.get();
    buttonA->onClick.connect([buttonAPtr]{ buttonAPtr->sendToFront(); });
    myGui.addChild(buttonA);

    auto buttonB = gui::Button::create(theme);
    connectDebugSignals(buttonB, "buttonB");
    buttonB->setLabel("B (front)");
    buttonB->setPosition(50.0f, 170.0f);
    auto buttonBPtr = buttonB.get();
    buttonB->onClick.connect([buttonBPtr]{ buttonBPtr->sendToFront(); });
    myGui.addChild(buttonB);

    auto buttonC = gui::Button::create(theme);
    connectDebugSignals(buttonC, "buttonC");
    buttonC->setLabel("C (back)");
    buttonC->setPosition(30.0f, 185.0f);
    auto buttonCPtr = buttonC.get();
    buttonC->onClick.connect([buttonCPtr]{ buttonCPtr->sendToBack(); });
    myGui.addChild(buttonC);


    auto moveButton = gui::Button::create(theme);
    connectDebugSignals(moveButton, "moveButton");
    moveButton->setLabel("Hover to move");
    moveButton->setPosition(200.0f, 70.0f);
    auto moveButtonPtr = moveButton.get();
    moveButton->onMouseEnter.connect([moveButtonPtr]{ moveButtonPtr->setPosition(200.0f, 60.0f); moveButtonPtr->setRotation(15); });
    moveButton->onMouseLeave.connect([moveButtonPtr]{ moveButtonPtr->setPosition(200.0f, 70.0f); moveButtonPtr->setRotation(0); });
    myGui.addChild(moveButton);


    auto customStyled1 = gui::Button::create(theme);
    connectDebugSignals(customStyled1, "customStyled1");
    auto customButtonStyle = customStyled1->getStyle();
    customButtonStyle->setTextPadding({0, 0, customButtonStyle->getTextPadding().z});
    customStyled1->setLabel("custom");
    customStyled1->setPosition(200.0f, 120.0f);
    customStyled1->onMouseEnter.connect([=]{ customButtonStyle->setTextStyle(sf::Text::Bold); });
    customStyled1->onMouseLeave.connect([=]{ customButtonStyle->setTextStyle(sf::Text::Regular); });
    myGui.addChild(customStyled1);

    auto customStyled2 = gui::Button::create(customStyled1->getStyle());
    connectDebugSignals(customStyled2, "customStyled2");
    customStyled2->setLabel("styled");
    customStyled2->setPosition(200.0f, 150.0f);
    customStyled2->onMouseEnter.connect([=]{ customStyled1->getStyle()->setTextStyle(sf::Text::Bold); });
    customStyled2->onMouseLeave.connect([=]{ customStyled1->getStyle()->setTextStyle(sf::Text::Regular); });
    myGui.addChild(customStyled2);

    // This button switches to a different style on hover and also disables auto-resize.
    auto customStyled3 = gui::Button::create(customStyled1->getStyle());
    connectDebugSignals(customStyled3, "customStyled3");
    customStyled3->setLabel("buttons");
    customStyled3->setAutoResize(false);
    customStyled3->setPosition(200.0f, 180.0f);
    auto customStyled3Ptr = customStyled3.get();
    customStyled3->onMouseEnter.connect([customStyled3Ptr]{ customStyled3Ptr->getStyle()->setTextStyle(sf::Text::Underlined); customStyled3Ptr->getStyle()->setFillColor({90, 90, 200}); });
    customStyled3->onMouseLeave.connect([customStyled3Ptr]{ customStyled3Ptr->getStyle()->setTextStyle(sf::Text::Regular); });
    myGui.addChild(customStyled3);

    customButtonStyle->setFillColor({20, 20, 200});


    auto panel = gui::Panel::create(theme);
    connectDebugSignals(panel, "panel");
    panel->setSize({100, 90});
    panel->setOrigin(5.0f, 15.0f);
    panel->setPosition(50.0f, 250.0f);
    //panel->setRotation(30.0f);
    myGui.addChild(panel);

    auto panelLabel = gui::Label::create(theme);
    connectDebugSignals(panelLabel, "panelLabel");
    panelLabel->setLabel("Pixel Paint");
    panelLabel->setPosition(0, 5);
    panel->addChild(panelLabel);

    // Ensure that lifetime of image and texture are bound to the button by
    // using shared_ptr. Custom deleter is for debugging to prove this works.
    std::shared_ptr<sf::Image> paintImage(new sf::Image(), [](sf::Image* p) {
        std::cout << "Custom deleter called for paintImage.\n";
        delete p;
    });
    std::shared_ptr<sf::Texture> paintTexture(new sf::Texture(), [](sf::Texture* p) {
        std::cout << "Custom deleter called for paintTexture.\n";
        delete p;
    });
    paintImage->create(70, 50, sf::Color(25, 25, 25));

    auto paintButton = gui::Button::create(theme);
    connectDebugSignals(paintButton, "paintButton");
    auto style = paintButton->getStyle();
    style->setFillColor(sf::Color::White);
    paintTexture->loadFromImage(*paintImage);
    style->setTexture(paintTexture.get(), true);
    paintButton->setSize(sf::Vector2f(paintImage->getSize()));
    paintButton->setPosition(10, 30);
    paintButton->onClick.connect([=](gui::Widget* /*w*/, const sf::Vector2f& mouseLocal) {
        const sf::Vector2u point = {
            std::min(static_cast<unsigned int>(mouseLocal.x), paintImage->getSize().x - 1),
            std::min(static_cast<unsigned int>(mouseLocal.y), paintImage->getSize().y - 1)
        };
        std::cout << "Paint point at (" << point.x << ", " << point.y << ").\n";
        paintImage->setPixel(point.x, point.y, getRandColor());
        paintTexture->loadFromImage(*paintImage);
    });
    panel->addChild(paintButton);
}

void createChatBoxDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto chatTest = gui::ChatBox::create(theme);
    connectDebugSignals(chatTest, "chatTest");
    chatTest->setSizeCharacters({20, 7});
    chatTest->setMaxLines(8);
    chatTest->setPosition(10.0f, 10.0f);
    myGui.addChild(chatTest);

    auto textBox = gui::TextBox::create(theme);
    connectDebugSignals(textBox, "textBox");
    textBox->setPosition(chatTest->getPosition() + sf::Vector2f(0.0f, chatTest->getSize().y + 5.0f));
    textBox->setWidthCharacters(chatTest->getSizeCharacters().x);
    textBox->setDefaultText("enter message");
    myGui.addChild(textBox);

    auto autoHideToggle = gui::CheckBox::create(theme);
    connectDebugSignals(autoHideToggle, "autoHideToggle");
    autoHideToggle->setLabel("Auto Hide");
    autoHideToggle->setPosition(textBox->getPosition() + sf::Vector2f(0.0f, textBox->getSize().y + 5.0f));
    auto autoHideTogglePtr = autoHideToggle.get();
    autoHideToggle->onClick.connect([chatTest,autoHideTogglePtr]() {
        chatTest->setAutoHide(autoHideTogglePtr->isChecked());
    });
    myGui.addChild(autoHideToggle);

    const std::array<std::string, 10> responses = {
        "Is that so?",
        "Unbelievable",
        "Hmm, I would tend to agree.",
        "You can\'t be serious.",
        "Oh for sure.",
        "No way...",
        "Real",
        "Nuh uh",
        "Please stop talking.",
        "Are you a bot? Get out of my chat bro."
    };

    auto textBoxPtr = textBox.get();
    textBox->onEnterPressed.connect([chatTest,textBoxPtr,responses]() {
        chatTest->addLine(textBoxPtr->getText(), sf::Color::Blue);
        textBoxPtr->setText("");
        gui::Timer::create([=]() {
            chatTest->addLine(responses[std::rand() % responses.size()], sf::Color::Red, sf::Text::Italic);
        }, std::chrono::milliseconds(500 + std::rand() % 1000));
    });

    gui::Timer::create([=]() {
        chatTest->addLine("user entered chat", sf::Color::Red);
    }, std::chrono::milliseconds(1000));

    /*auto t1 = gui::Timer::create([=](gui::Timer* timer) {
        std::cout << "t1 called at time " << std::chrono::duration_cast<std::chrono::milliseconds>(gui::Timer::Clock::now().time_since_epoch()).count() << "\n";
        chatTest->addLine("[timer count is " + std::to_string(timer->getCount()) + "]");
    }, std::chrono::milliseconds(1000), 5);*/
}

void createCheckBoxDemo(gui::Gui& myGui, const gui::Theme& theme) {
    // Check boxes.
    auto testCheckBox = gui::CheckBox::create(theme);
    connectDebugSignals(testCheckBox, "testCheckBox");
    testCheckBox->setLabel(sf::String("check box"));
    testCheckBox->setPosition(10.0f, 10.0f);
    auto testCheckBoxPtr = testCheckBox.get();
    testCheckBox->onClick.connect([testCheckBoxPtr]() {
        if (testCheckBoxPtr->isChecked()) {
            testCheckBoxPtr->setLabel("the box is checked!");
        } else {
            testCheckBoxPtr->setLabel("the box is not checked");
        }
    });
    myGui.addChild(testCheckBox);

    auto disabledCheckBox = gui::CheckBox::create(theme);
    connectDebugSignals(disabledCheckBox, "disabledCheckBox");
    disabledCheckBox->setLabel(sf::String("check box (disabled)"));
    disabledCheckBox->setPosition(10.0f, 70.0f);
    disabledCheckBox->setEnabled(false);
    myGui.addChild(disabledCheckBox);

    auto checkBoxPanel = gui::Panel::create(theme);
    connectDebugSignals(checkBoxPanel, "checkBoxPanel");
    checkBoxPanel->setPosition(10.0f, 130.0f);
    myGui.addChild(checkBoxPanel);

    auto checkBoxA = gui::CheckBox::create(theme);
    connectDebugSignals(checkBoxA, "checkBoxA");
    checkBoxA->setLabel(sf::String("Option A"));
    checkBoxA->setPosition(8.0f, 8.0f);
    checkBoxPanel->addChild(checkBoxA);

    auto checkBoxB = gui::CheckBox::create(theme);
    connectDebugSignals(checkBoxB, "checkBoxB");
    checkBoxB->setLabel(sf::String("Option B"));
    checkBoxB->setPosition(checkBoxA->getPosition() + sf::Vector2f(0.0f, checkBoxA->getSize().y + 8.0f));
    checkBoxPanel->addChild(checkBoxB);

    auto checkBoxC = gui::CheckBox::create(theme);
    connectDebugSignals(checkBoxC, "checkBoxC");
    checkBoxC->setLabel(sf::String("Option C"));
    checkBoxC->setPosition(checkBoxB->getPosition() + sf::Vector2f(0.0f, checkBoxB->getSize().y + 8.0f));
    checkBoxPanel->addChild(checkBoxC);

    checkBoxPanel->setSize(checkBoxC->getPosition() + checkBoxC->getSize() + sf::Vector2f(8.0f, 8.0f));

    // Radio buttons.
    auto testRadioButton = gui::RadioButton::create(theme);
    connectDebugSignals(testRadioButton, "testRadioButton");
    testRadioButton->setLabel(sf::String("radio button"));
    testRadioButton->setPosition(230.0f, 10.0f);
    auto testRadioButtonPtr = testRadioButton.get();
    testRadioButton->onClick.connect([testRadioButtonPtr]() {
        if (testRadioButtonPtr->isChecked()) {
            testRadioButtonPtr->setLabel("the button is checked!");
        } else {
            testRadioButtonPtr->setLabel("the button is not checked");
        }
    });
    myGui.addChild(testRadioButton);

    auto disabledRadioButton = gui::RadioButton::create(theme);
    connectDebugSignals(disabledRadioButton, "disabledRadioButton");
    disabledRadioButton->setLabel(sf::String("radio button (disabled)"));
    disabledRadioButton->setPosition(230.0f, 70.0f);
    disabledRadioButton->setEnabled(false);
    myGui.addChild(disabledRadioButton);

    auto radioButtonPanel = gui::Panel::create(theme);
    connectDebugSignals(radioButtonPanel, "radioButtonPanel");
    radioButtonPanel->setPosition(230.0f, 130.0f);
    myGui.addChild(radioButtonPanel);

    auto radioButtonGroup = gui::Group::create();
    connectDebugSignals(radioButtonGroup, "radioButtonGroup");
    radioButtonPanel->addChild(radioButtonGroup);

    auto radioButtonA = gui::RadioButton::create(theme);
    connectDebugSignals(radioButtonA, "radioButtonA");
    radioButtonA->setLabel(sf::String("Option A"));
    radioButtonA->setPosition(8.0f, 8.0f);
    radioButtonGroup->addChild(radioButtonA);

    auto radioButtonB = gui::RadioButton::create(theme);
    connectDebugSignals(radioButtonB, "radioButtonB");
    radioButtonB->setLabel(sf::String("Option B"));
    radioButtonB->setPosition(radioButtonA->getPosition() + sf::Vector2f(0.0f, radioButtonA->getSize().y + 8.0f));
    radioButtonGroup->addChild(radioButtonB);

    auto radioButtonC = gui::RadioButton::create(theme);
    connectDebugSignals(radioButtonC, "radioButtonC");
    radioButtonC->setLabel(sf::String("Option C"));
    radioButtonC->setPosition(radioButtonB->getPosition() + sf::Vector2f(0.0f, radioButtonB->getSize().y + 8.0f));
    radioButtonGroup->addChild(radioButtonC);

    auto radioButtonD = gui::RadioButton::create(theme);
    connectDebugSignals(radioButtonD, "radioButtonD");
    radioButtonD->setLabel(sf::String("Option D (outside group)"));
    radioButtonD->setPosition(radioButtonC->getPosition() + sf::Vector2f(0.0f, radioButtonC->getSize().y + 8.0f));
    radioButtonPanel->addChild(radioButtonD);

    radioButtonPanel->setSize(radioButtonD->getPosition() + radioButtonD->getSize() + sf::Vector2f(8.0f, 8.0f));

    auto radioResetButton = gui::Button::create(theme);
    connectDebugSignals(radioResetButton, "radioResetButton");
    radioResetButton->setLabel("Reset");
    radioResetButton->setPosition(radioButtonPanel->getPosition() + sf::Vector2f(0.0f, radioButtonPanel->getSize().y + 8.0f));
    radioResetButton->onClick.connect([=]() {
        radioButtonA->uncheckRadioButtons();
        radioButtonD->uncheckRadioButtons();
    });
    myGui.addChild(radioResetButton);
}

void createColorPickerDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto panel = gui::Panel::create(theme);
    connectDebugSignals(panel, "panel");
    panel->setPosition(10.0f, 10.0f);
    panel->getStyle()->setOutlineThickness(0.0f);
    myGui.addChild(panel);

    auto colorPickerTest = gui::ColorPicker::create(theme);
    connectDebugSignals(colorPickerTest, "colorPickerTest");
    panel->addChild(colorPickerTest);

    auto paletteButton1 = gui::Button::create(theme);
    connectDebugSignals(paletteButton1, "paletteButton1");
    paletteButton1->getStyle()->setFillColor({240, 182, 58});
    paletteButton1->setLabel("240, 182, 58");
    auto paletteButton1Ptr = paletteButton1.get();
    paletteButton1->onClick.connect([paletteButton1Ptr,colorPickerTest]() {
        colorPickerTest->setColor(paletteButton1Ptr->getStyle()->getFillColor());
    });
    myGui.addChild(paletteButton1);

    auto paletteButton2 = gui::Button::create(theme);
    connectDebugSignals(paletteButton2, "paletteButton2");
    paletteButton2->getStyle()->setFillColor({90, 222, 242});
    paletteButton2->setLabel("90, 222, 242");
    auto paletteButton2Ptr = paletteButton2.get();
    paletteButton2->onClick.connect([paletteButton2Ptr,colorPickerTest]() {
        colorPickerTest->setColor(paletteButton2Ptr->getStyle()->getFillColor());
    });
    myGui.addChild(paletteButton2);

    auto colorPreview = gui::Button::create(theme);
    connectDebugSignals(colorPreview, "colorPreview");
    colorPreview->setLabel(" Color ");
    myGui.addChild(colorPreview);

    colorPickerTest->onColorChange.connect([=](gui::Widget* /*w*/, const sf::Color& color) {
        colorPreview->getStyle()->setFillColor(color);
    });

    // Resize the GUI on window size changed.
    myGui.onWindowResized.connect([=](gui::Gui* gui, sf::RenderWindow& window, const sf::Vector2u& size){
        std::cout << "Window size changed!\n";
        gui->setSize(size);
        window.setView(sf::View(sf::FloatRect({0.0f, 0.0f}, static_cast<sf::Vector2f>(size))));

        gui->getChild<gui::Button>("sceneButton")->setPosition(0.0f, size.y - 20.0f);
        colorPickerTest->setSize({size.x - 100.0f, size.y - 100.0f});
        panel->setSize(colorPickerTest->getSize());
        paletteButton1->setPosition(panel->getPosition() + sf::Vector2f(0.0f, panel->getSize().y + 10.0f));
        paletteButton2->setPosition(panel->getPosition() + sf::Vector2f(120.0f, panel->getSize().y + 10.0f));
        colorPreview->setPosition(panel->getPosition() + sf::Vector2f(240.0f, panel->getSize().y + 10.0f));
    });

    // Trigger a fake event to initialize the size.
    sf::Event initSizeEvent;
    initSizeEvent.type = sf::Event::Resized;
    initSizeEvent.size.width = myGui.getSize().x;
    initSizeEvent.size.height = myGui.getSize().y;
    myGui.processEvent(initSizeEvent);
}

void createDialogBoxDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto dialogTest = gui::DialogBox::create(theme);
    connectDebugSignals(dialogTest, "dialogTest");
    dialogTest->setSize({200.0f, 100.0f});
    dialogTest->setPosition(80.0f, 80.0f);
    myGui.addChild(dialogTest);
    auto dialogTestPtr = dialogTest.get();

    /*auto panelThing = gui::Panel::create(theme);
    connectDebugSignals(panelThing, "panelThing");
    panelThing->setSize({400.0f, 300.0f});
    panelThing->setRotation(14.0f);
    panelThing->setScale(1.8f, 1.3f);
    panelThing->setPosition(90.0f, 70.0f);
    panelThing->addChild(dialogTest);
    myGui.addChild(panelThing);

    auto dialogTest2 = gui::DialogBox::create(theme);
    connectDebugSignals(dialogTest2, "dialogTest2");
    dialogTest2->setSize({130.0f, 70.0f});
    dialogTest2->setPosition(140.0f, 230.0f);
    panelThing->addChild(dialogTest2);*/

    auto dialogTestTitle = gui::Label::create(theme);
    connectDebugSignals(dialogTestTitle, "dialogTestTitle");
    dialogTestTitle->setLabel("my title");
    dialogTest->setTitle(dialogTestTitle);

    auto dialogTestLabel = gui::Label::create(theme);
    connectDebugSignals(dialogTestLabel, "dialogTestLabel");
    dialogTestLabel->setLabel("Name:");
    dialogTestLabel->setPosition(10.0f, 20.0f);
    dialogTest->addChild(dialogTestLabel);

    auto dialogTestTextBox = gui::TextBox::create(theme);
    connectDebugSignals(dialogTestTextBox, "dialogTestTextBox");
    dialogTestTextBox->setWidthCharacters(8);
    dialogTestTextBox->setPosition(100.0f, 20.0f);
    dialogTest->addChild(dialogTestTextBox);

    auto dialogTestTextBox2 = gui::MultilineTextBox::create(theme);
    connectDebugSignals(dialogTestTextBox2, "dialogTestTextBox2");
    dialogTestTextBox2->setSizeCharacters({8, 1});
    dialogTestTextBox2->setMaxLines(1);
    dialogTestTextBox2->setTabPolicy(gui::MultilineTextBox::TabPolicy::ignoreTab);
    dialogTestTextBox2->setPosition(100.0f, 40.0f);
    dialogTest->addChild(dialogTestTextBox2);

    auto dialogTestCancel = gui::Button::create(theme);
    connectDebugSignals(dialogTestCancel, "dialogTestCancel");
    dialogTestCancel->setLabel("Cancel");
    dialogTestCancel->setPosition(10.0f, 60.0f);
    dialogTestCancel->onClick.connect([dialogTestPtr]() {
        std::cout << "Dialog cancelled!\n";
        dialogTestPtr->setSize(dialogTestPtr->getSize() - sf::Vector2f(100.0f, 70.0f));
    });
    dialogTest->setCancelButton(0, dialogTestCancel);

    auto dialogTestSubmit = gui::Button::create(theme);
    connectDebugSignals(dialogTestSubmit, "dialogTestSubmit");
    dialogTestSubmit->setLabel("Rename");
    dialogTestSubmit->setPosition(100.0f, 60.0f);
    dialogTestSubmit->onClick.connect([dialogTestPtr]() {
        std::cout << "Dialog submitted!\n";
        dialogTestPtr->setSize(dialogTestPtr->getSize() + sf::Vector2f(100.0f, 70.0f));
    });
    dialogTest->setSubmitButton(1, dialogTestSubmit);

    const std::string alignmentToString[3] = {
        "left",
        "center",
        "right"
    };

    auto dialogTitleAlignment = gui::Button::create(theme);
    connectDebugSignals(dialogTitleAlignment, "dialogTitleAlignment");
    dialogTitleAlignment->setLabel("Title: " + alignmentToString[static_cast<int>(dialogTest->getTitleAlignment())]);
    dialogTitleAlignment->setPosition(10.0f, 10.0f);
    auto dialogTitleAlignmentPtr = dialogTitleAlignment.get();
    dialogTitleAlignment->onClick.connect([dialogTestPtr,dialogTitleAlignmentPtr,alignmentToString]() {
        dialogTestPtr->setTitleAlignment(static_cast<gui::DialogBox::Alignment>((static_cast<int>(dialogTestPtr->getTitleAlignment()) + 1) % 3));
        dialogTitleAlignmentPtr->setLabel("Title: " + alignmentToString[static_cast<int>(dialogTestPtr->getTitleAlignment())]);
    });
    myGui.addChild(dialogTitleAlignment);

    auto dialogButtonAlignment = gui::Button::create(theme);
    connectDebugSignals(dialogButtonAlignment, "dialogButtonAlignment");
    dialogButtonAlignment->setLabel("Buttons: " + alignmentToString[static_cast<int>(dialogTest->getButtonAlignment())]);
    dialogButtonAlignment->setPosition(10.0f, 40.0f);
    auto dialogButtonAlignmentPtr = dialogButtonAlignment.get();
    dialogButtonAlignment->onClick.connect([dialogTestPtr,dialogButtonAlignmentPtr,alignmentToString]() {
        dialogTestPtr->setButtonAlignment(static_cast<gui::DialogBox::Alignment>((static_cast<int>(dialogTestPtr->getButtonAlignment()) + 1) % 3));
        dialogButtonAlignmentPtr->setLabel("Buttons: " + alignmentToString[static_cast<int>(dialogTestPtr->getButtonAlignment())]);
    });
    myGui.addChild(dialogButtonAlignment);

    // Trigger focus on the text box.
    dialogTest->setVisible(true);
}

void createTextBoxDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto hideBox = gui::TextBox::create(theme);
    connectDebugSignals(hideBox, "hideBox");
    hideBox->setWidthCharacters(16);
    hideBox->setText("right click hide");
    hideBox->setPosition(10.0f, 10.0f);
    auto hideBoxPtr = hideBox.get();
    hideBox->onMousePress.connect([hideBoxPtr](gui::Widget* /*w*/, sf::Mouse::Button button, const sf::Vector2f& /*pos*/){
        if (button == sf::Mouse::Right) {
            hideBoxPtr->setVisible(false);
        }
    });
    myGui.addChild(hideBox);

    auto disableBox = gui::TextBox::create(theme);
    connectDebugSignals(disableBox, "disableBox");
    disableBox->setWidthCharacters(16);
    disableBox->setText("right click disable");
    disableBox->setPosition(200.0f, 10.0f);
    auto disableBoxPtr = disableBox.get();
    disableBox->onMousePress.connect([disableBoxPtr](gui::Widget* /*w*/, sf::Mouse::Button button, const sf::Vector2f& /*pos*/){
        if (button == sf::Mouse::Right) {
            disableBoxPtr->setEnabled(false);
        }
    });
    myGui.addChild(disableBox);

    auto resetBox = gui::TextBox::create(theme);
    connectDebugSignals(resetBox, "resetBox");
    resetBox->setWidthCharacters(16);
    resetBox->setReadOnly(true);
    resetBox->setDefaultText("enter to reset  (trimmed text)");
    resetBox->setPosition(10.0f, 70.0f);
    resetBox->onEnterPressed.connect([=]{ hideBox->setVisible(true); disableBox->setEnabled(true); });
    myGui.addChild(resetBox);

    auto maxCharBox = gui::TextBox::create(theme);
    connectDebugSignals(maxCharBox, "maxCharBox");
    maxCharBox->setWidthCharacters(16);
    maxCharBox->setMaxCharacters(9);
    maxCharBox->setDefaultText("max chars");
    maxCharBox->setPosition(200.0f, 90.0f);
    maxCharBox->setRotation(-33.0f);
    myGui.addChild(maxCharBox);
    maxCharBox->sendToBack();

    auto multilineBox = gui::MultilineTextBox::create(theme);
    connectDebugSignals(multilineBox, "multilineBox");
    multilineBox->setSizeCharacters({16, 3});
    multilineBox->setText("This is my really long batch\nof text for testing selections within a\ntext box,\nhopefully it works?\nwe'll see...");
    multilineBox->setDefaultText("Enter text:");
    multilineBox->setPosition(10.0f, 130.0f);
    //multilineBox->setOrigin(-40.0f, -90.0f);
    //multilineBox->setRotation(2.0f);
    myGui.addChild(multilineBox);

    auto multilineBox2 = gui::MultilineTextBox::create(theme);
    connectDebugSignals(multilineBox2, "multilineBox2");
    multilineBox2->setSizeCharacters({16, 3});
    multilineBox2->setMaxLines(3);
    multilineBox2->setDefaultText("Here is some default\ntext, this box\nhas max of 3 lines\nin it.");
    multilineBox2->setPosition(200.0f, 130.0f);
    myGui.addChild(multilineBox2);

    auto readOnlyCheckBox = gui::CheckBox::create(theme);
    connectDebugSignals(readOnlyCheckBox, "readOnlyCheckBox");
    readOnlyCheckBox->setLabel("Read Only");
    readOnlyCheckBox->setPosition(multilineBox2->getPosition() + sf::Vector2f(0.0f, multilineBox2->getSize().y + 15.0f));
    auto readOnlyCheckBoxPtr = readOnlyCheckBox.get();
    readOnlyCheckBox->onClick.connect([readOnlyCheckBoxPtr,multilineBox2]() {
        multilineBox2->setReadOnly(readOnlyCheckBoxPtr->isChecked());
    });
    myGui.addChild(readOnlyCheckBox);
}

void createMenuBarDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto menuBar = gui::MenuBar::create(theme);
    connectDebugSignals(menuBar, "menuBar");
    menuBar->setWidth(300.0f);
    menuBar->setPosition(150.0f, 50.0f);
    menuBar->setRotation(-9.0f);
    menuBar->setOrigin(100.0f, 15.0f);

    gui::MenuList menuList("menu1");
    menuList.items.emplace_back("hi i am:", "menu1");
    menuList.items.emplace_back("insert left", "");
    menuList.items.emplace_back("insert right", "");
    menuList.items.emplace_back("remove left", "");
    menuList.items.emplace_back("remove right", "");
    menuList.items.emplace_back("remove me", "");
    menuList.items.emplace_back("???", "", false);
    menuList.items.emplace_back("ping", "");
    menuList.items.emplace_back("pong", "", false);
    menuBar->insertMenu(menuList);

    auto menuBarPtr = menuBar.get();
    menuBar->onMenuItemClick.connect([menuBarPtr](gui::Widget* /*w*/, const gui::MenuList& menu, size_t index){
        size_t menuIndex = menuBarPtr->findMenuIndex(menu.name);
        gui::MenuList newMenu = menu;
        if (newMenu.name.getSize() > 4) {
            newMenu.name = "menu" + std::to_string(std::stoi(newMenu.name.substring(4).toAnsiString()) + 1);
            newMenu.items[0].rightText = newMenu.name;
        }

        if (menu.items[index].leftText == "insert left") {
            menuBarPtr->insertMenu(newMenu, menuIndex);
        } else if (menu.items[index].leftText == "insert right") {
            menuBarPtr->insertMenu(newMenu, menuIndex + 1);
        } else if (menu.items[index].leftText == "remove left") {
            if (menuIndex > 0) {
                menuBarPtr->removeMenu(menuIndex - 1);
            } else {
                std::cout << "No menu on the left side to remove!\n";
            }
        } else if (menu.items[index].leftText == "remove right") {
            menuBarPtr->removeMenu(menuIndex + 1);
        } else if (menu.items[index].leftText == "remove me") {
            menuBarPtr->removeMenu(menuIndex);
        } else if (menu.items[index].leftText == "ping") {
            newMenu = menu;
            newMenu.items[index].rightText = "bounced";
            newMenu.items[index].enabled = false;
            newMenu.items[index + 1].enabled = true;
            menuBarPtr->setMenu(newMenu, menuIndex);
        } else if (menu.items[index].leftText == "pong") {
            newMenu = menu;
            newMenu.items[index].rightText = "bounced again";
            newMenu.items[index].enabled = false;
            newMenu.items[index - 1].enabled = true;
            menuBarPtr->setMenu(newMenu, menuIndex);
        } else {
            std::cout << "That menu option doesn't do anything :(\n";
        }
    });

    menuList.name = "file";
    menuList.items.clear();
    menuList.items.emplace_back("quit", "Alt+Q");
    menuBar->insertMenu(menuList);

    myGui.addChild(menuBar);
}

void createSliderDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto slider1 = gui::Slider::create(theme);
    connectDebugSignals(slider1, "slider1");
    slider1->setSize({150.0f, 20.0f});
    slider1->setValue(0.5f);
    slider1->setPosition(10.0f, 10.0f);
    myGui.addChild(slider1);

    auto slider1Label = gui::Label::create(theme);
    connectDebugSignals(slider1Label, "slider1Label");
    auto slider1Ptr = slider1.get();
    slider1->onValueChange.connect([=](gui::Widget* /*w*/, float value) {
        slider1Label->setLabel(std::to_string(value));
        slider1Label->setPosition(slider1Ptr->getSize() / 2.0f);
        slider1Label->setOrigin(slider1Label->getSize() / 2.0f);
    });
    slider1->setLabel(slider1Label);

    auto slider2 = gui::Slider::create(theme);
    connectDebugSignals(slider2, "slider2");
    slider2->setSize({150.0f, 20.0f});
    slider2->setRange({-7.0f, 11.0f});
    slider2->setPosition(10.0f, 70.0f);
    myGui.addChild(slider2);

    auto slider2Label = gui::Label::create(theme);
    connectDebugSignals(slider2Label, "slider2Label");
    auto slider2Ptr = slider2.get();
    slider2->onValueChange.connect([=](gui::Widget* /*w*/, float value) {
        slider2Label->setLabel(std::to_string(value));
        slider2Label->setPosition(slider2Ptr->getSize() / 2.0f);
        slider2Label->setOrigin(slider2Label->getSize() / 2.0f);
    });
    slider2->setLabel(slider2Label);

    auto slider3 = gui::Slider::create(theme);
    connectDebugSignals(slider3, "slider3");
    slider3->setSize({150.0f, 20.0f});
    slider3->setStep(0.1f);
    slider3->setPosition(10.0f, 130.0f);
    myGui.addChild(slider3);

    auto slider3Label = gui::Label::create(theme);
    connectDebugSignals(slider3Label, "slider3Label");
    auto slider3Ptr = slider3.get();
    slider3->onValueChange.connect([=](gui::Widget* /*w*/, float value) {
        slider3Label->setLabel(std::to_string(value));
        slider3Label->setPosition(slider3Ptr->getSize() / 2.0f);
        slider3Label->setOrigin(slider3Label->getSize() / 2.0f);
    });
    slider3->setLabel(slider3Label);

    const std::array<std::string, 4> elements = {
        "Fire", "Earth", "Air", "Water"
    };

    auto sliderEnum = gui::Slider::create(theme);
    connectDebugSignals(sliderEnum, "sliderEnum");
    sliderEnum->setSize({200.0f, 30.0f});
    sliderEnum->setRange({0.0f, elements.size() - 1.0f});
    sliderEnum->setStep(1.0f);
    sliderEnum->setPosition(-30.0f, 195.0f);
    sliderEnum->setOrigin(-40.0f, 5.0f);
    myGui.addChild(sliderEnum);

    auto sliderEnumLabel = gui::Label::create(theme);
    connectDebugSignals(sliderEnumLabel, "sliderEnumLabel");
    auto sliderEnumPtr = sliderEnum.get();
    sliderEnum->onValueChange.connect([=](gui::Widget* /*w*/, float value) {
        sliderEnumLabel->setLabel(elements[static_cast<unsigned int>(value)]);
        sliderEnumLabel->setPosition(sliderEnumPtr->getSize() / 2.0f);
        sliderEnumLabel->setOrigin(sliderEnumLabel->getSize() / 2.0f);
    });
    sliderEnum->setLabel(sliderEnumLabel);
}

void createFullDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto menuBar = gui::MenuBar::create(theme, "menuBar");
    connectDebugSignals(menuBar, "menuBar");
    //menuBar->setWidth(800.0f);
    menuBar->setPosition(0.0f, 0.0f);

    gui::MenuList fileMenu("File");
    fileMenu.items.emplace_back("New", "Ctrl+N", false);
    fileMenu.items.emplace_back("Open...", "Ctrl+O", false);
    fileMenu.items.emplace_back("Save", "Ctrl+S", false);
    fileMenu.items.emplace_back("Save As...", "", false);
    fileMenu.items.emplace_back("Rename...");
    fileMenu.items.emplace_back("Resize...", "", false);
    fileMenu.items.emplace_back("Configuration...", "", false);
    fileMenu.items.emplace_back("Exit", "", false);
    menuBar->insertMenu(fileMenu);

    gui::MenuList viewMenu("View");
    viewMenu.items.emplace_back("Zoom In", "Mouse Wheel Up");
    viewMenu.items.emplace_back("Zoom Out", "Mouse Wheel Down");
    viewMenu.items.emplace_back("Default Zoom");
    menuBar->insertMenu(viewMenu);

    gui::MenuList runMenu("Run");
    runMenu.items.emplace_back("Step One Tick", "Tab");
    menuBar->insertMenu(runMenu);

    gui::MenuList toolsMenu("Tools");
    toolsMenu.items.emplace_back("Select All", "Ctrl+A");
    toolsMenu.items.emplace_back("Deselect All", "ESC");
    toolsMenu.items.emplace_back("Cut", "Ctrl+X");
    toolsMenu.items.emplace_back("Copy", "Ctrl+C");
    toolsMenu.items.emplace_back("Paste", "Ctrl+V");
    toolsMenu.items.emplace_back("  (Shift+Right Click = Force Paste)", "", false);
    toolsMenu.items.emplace_back("Selection Query", "Q");
    menuBar->insertMenu(toolsMenu);

    myGui.addChild(menuBar);

    auto modalBackground = gui::Panel::create(theme, "modalBackground");
    connectDebugSignals(modalBackground, "modalBackground");
    modalBackground->setSize(static_cast<sf::Vector2f>(myGui.getSize()));
    modalBackground->setVisible(false);
    modalBackground->setFocusable(false);
    modalBackground->getStyle()->setFillColor({0, 0, 0, 175});
    modalBackground->getStyle()->setOutlineThickness(0.0f);
    myGui.addChild(modalBackground);
    auto modalBackgroundPtr = modalBackground.get();

    auto renameDialog = gui::DialogBox::create(theme, "renameDialog");
    connectDebugSignals(renameDialog, "renameDialog");
    renameDialog->setSize({200.0f, 100.0f});
    modalBackground->addChild(renameDialog);
    auto renameDialogPtr = renameDialog.get();

    auto renameTitle = gui::Label::create(theme, "renameTitle");
    connectDebugSignals(renameTitle, "renameTitle");
    renameTitle->setLabel("Rename");
    renameDialog->setTitle(renameTitle);

    auto renameLabel = gui::Label::create(theme, "renameLabel");
    connectDebugSignals(renameLabel, "renameLabel");
    renameLabel->setLabel("Name:");
    renameLabel->setPosition(10.0f, 30.0f);
    renameDialog->addChild(renameLabel);

    auto renameTextBox = gui::MultilineTextBox::create(theme, "renameTextBox");
    connectDebugSignals(renameTextBox, "renameTextBox");
    renameTextBox->setSizeCharacters({8, 1});
    renameTextBox->setMaxLines(1);
    renameTextBox->setTabPolicy(gui::MultilineTextBox::TabPolicy::ignoreTab);
    renameTextBox->setPosition(100.0f, 30.0f);
    renameDialog->addChild(renameTextBox);

    auto renameCancelButton = gui::Button::create(theme, "renameCancelButton");
    connectDebugSignals(renameCancelButton, "renameCancelButton");
    renameCancelButton->setLabel("Cancel");
    renameCancelButton->onClick.connect([modalBackgroundPtr,renameDialogPtr](){
        renameDialogPtr->setVisible(false);
        modalBackgroundPtr->setVisible(false);
    });
    renameDialog->setCancelButton(0, renameCancelButton);

    auto renameSubmitButton = gui::Button::create(theme, "renameSubmitButton");
    connectDebugSignals(renameSubmitButton, "renameSubmitButton");
    renameSubmitButton->setLabel("Ok");
    renameDialog->setSubmitButton(1, renameSubmitButton);

    // Look up the rename button by name as a test.
    myGui.getChild<gui::Button>("renameSubmitButton")->onClick.connect([modalBackgroundPtr,renameDialogPtr](){
        renameDialogPtr->setVisible(false);
        modalBackgroundPtr->setVisible(false);
        std::cout << "Rename dialog submitted!\n";
    });
    assert(myGui.getChild<gui::Button>("renameSubmitButton", false) == nullptr);
    assert(myGui.getChild("renameSubmitButton", false) == nullptr);
    assert(myGui.getChild("idk") == nullptr);

    menuBar->onMenuItemClick.connect([modalBackgroundPtr,renameDialogPtr](gui::Widget* /*w*/, const gui::MenuList& menu, size_t index){
        if (menu.name == "File") {
            if (menu.items[index].leftText == "Rename...") {
                modalBackgroundPtr->sendToFront();
                modalBackgroundPtr->setVisible(true);
                renameDialogPtr->setVisible(true);
                renameDialogPtr->setPosition(
                    std::min(80.0f, modalBackgroundPtr->getSize().x / 2.0f),
                    std::min(80.0f, modalBackgroundPtr->getSize().y / 2.0f)
                );
            }
        }
    });

    auto centeredPanel = gui::Panel::create(theme, "centeredPanel");
    connectDebugSignals(centeredPanel, "centeredPanel");
    myGui.addChild(centeredPanel);

    auto centeredLabel = gui::Label::create(theme, "centeredLabel");
    connectDebugSignals(centeredLabel, "centeredLabel");
    centeredLabel->setLabel("Note: this scene allows the GUI to\nresize when the window size changes.\nThis label will stay centered.");
    centeredPanel->addChild(centeredLabel);
    centeredPanel->setSize(centeredLabel->getSize());
    centeredPanel->setOrigin(centeredPanel->getSize() / 2.0f);

    // Resize the GUI on window size changed.
    myGui.onWindowResized.connect([=](gui::Gui* gui, sf::RenderWindow& window, const sf::Vector2u& size){
        std::cout << "Window size changed!\n";
        gui->setSize(size);
        window.setView(sf::View(sf::FloatRect({0.0f, 0.0f}, static_cast<sf::Vector2f>(size))));

        gui->getChild<gui::Button>("sceneButton")->setPosition(0.0f, size.y - 20.0f);
        menuBar->setWidth(static_cast<float>(size.x));
        modalBackground->setSize(static_cast<sf::Vector2f>(size));
        centeredPanel->setPosition(static_cast<sf::Vector2f>(size) / 2.0f);
    });

    // Trigger a fake event to initialize the size.
    sf::Event initSizeEvent;
    initSizeEvent.type = sf::Event::Resized;
    initSizeEvent.size.width = myGui.getSize().x;
    initSizeEvent.size.height = myGui.getSize().y;
    myGui.processEvent(initSizeEvent);

    // Print the tree of Widgets.
    std::function<void(const gui::ContainerBase* container, const std::string& prefix)> printWidgetNames;
    printWidgetNames = [&printWidgetNames](const gui::ContainerBase* container, const std::string& prefix) {
        for (const auto& child : container->getChildren()) {
            std::cout << prefix << child->getName().toAnsiString() << "\n";
            auto childContainer = dynamic_cast<gui::ContainerBase*>(child.get());
            if (childContainer != nullptr) {
                printWidgetNames(childContainer, prefix + "  ");
            }
        }
    };
    std::cout << "Widget hierarchy:\n";
    std::cout << "myGui\n";
    printWidgetNames(&myGui, "  ");
}

void func() {
    std::cout << "func called\n";
}
void func2(int n) {
    std::cout << "func2 called with " << n << "\n";
}
void testCallbacks() {
    std::cout << "Testing Callback functionality.\n";
    // Value ctors
    gui::Callback<int> cb(&func);
    gui::Callback<int> cb2(&func2);
    // Copy ctor
    gui::Callback<int> cb3 = cb;
    gui::Callback<int> cb4 = cb2;
    // Move ctor
    gui::Callback<int> cb5 = gui::Callback<int>(&func);
    gui::Callback<int> cb6 = gui::Callback<int>(&func2);
    // Swap function
    swap(cb5, cb6);
    swap(cb, cb6);
    swap(cb2, cb5);
    // Copy assignment
    cb3 = cb5;
    cb4 = cb6;
    // Move assignment
    cb3 = gui::Callback<int>(&func);
    cb4 = gui::Callback<int>(&func2);
    // Test with unordered_map and dtor
    std::unordered_map<unsigned int, gui::Callback<int>> stuff;
    stuff.emplace(1, &func);
    stuff.emplace(2, &func2);
    stuff.emplace(3, gui::Callback<int>(&func));
    stuff.clear();
    cb3.value<gui::Callback<int>::Tag::noArgs>()();
    cb4.value<gui::Callback<int>::Tag::withArgs>()(123);
    // Throws runtime error as intended
    //cb3.value<gui::Callback<int>::Tag::withArgs>()(123);
}

int main() {
    // Just use a simple rng for now.
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    /*std::shared_ptr<gui::Timer> t1;
    auto chain4 = [](gui::Timer* timer) {
        std::cout << "callback chain4 at time " << std::chrono::duration_cast<std::chrono::milliseconds>(gui::Timer::Clock::now().time_since_epoch()).count() << ", count = " << timer->getCount() << "\n";
    };
    auto chain3 = [=](gui::Timer* timer) {
        std::cout << "callback chain3 at time " << std::chrono::duration_cast<std::chrono::milliseconds>(gui::Timer::Clock::now().time_since_epoch()).count() << ", count = " << timer->getCount() << "\n";
        timer->setCallback(chain4);
    };
    auto chain2 = [&]() {
        std::cout << "callback chain2 at time " << std::chrono::duration_cast<std::chrono::milliseconds>(gui::Timer::Clock::now().time_since_epoch()).count() << ", count = " << t1->getCount() << "\n";
        t1->setCallback(chain3);
    };
    auto chain1 = [=](gui::Timer* timer) {
        std::cout << "callback chain1 at time " << std::chrono::duration_cast<std::chrono::milliseconds>(gui::Timer::Clock::now().time_since_epoch()).count() << ", count = " << timer->getCount() << "\n";
        timer->setCallback(chain2);
    };
    t1 = gui::Timer::create(chain1, std::chrono::milliseconds(1000), 5);*/

    /*auto t1 = gui::Timer::create([](gui::Timer* timer){
        std::cout << "callback t1 at time " << std::chrono::duration_cast<std::chrono::milliseconds>(gui::Timer::Clock::now().time_since_epoch()).count() << ", count = " << timer->getCount() << "\n";
        //timer->setCount(3);
    }, std::chrono::milliseconds(1000), 5);
    auto t2 = gui::Timer::create([](gui::Timer* timer){
        std::cout << "callback t2 at time " << std::chrono::duration_cast<std::chrono::milliseconds>(gui::Timer::Clock::now().time_since_epoch()).count() << ", count = " << timer->getCount() << "\n";
        //timer->setCount(3);
    });
    auto t3 = gui::Timer::create([](gui::Timer* timer){
        std::cout << "callback t3 at time " << std::chrono::duration_cast<std::chrono::milliseconds>(gui::Timer::Clock::now().time_since_epoch()).count() << ", count = " << timer->getCount() << "\n";
        //timer->setCount(3);
    }, std::chrono::milliseconds(50));
    auto t4 = gui::Timer::create([](gui::Timer* timer){
        std::cout << "callback t4 at time " << std::chrono::duration_cast<std::chrono::milliseconds>(gui::Timer::Clock::now().time_since_epoch()).count() << ", count = " << timer->getCount() << "\n";
        //timer->setCount(3);
    }, std::chrono::milliseconds(1500), 5);*/

    //auto t2 = gui::Timer::create(func, std::chrono::milliseconds(2));
    //auto t3 = gui::Timer::create(func);
    //auto t4 = gui::Timer::create(func, std::chrono::milliseconds(30));

    testCallbacks();

    sf::RenderWindow window(sf::VideoMode(800, 600), "GUI Test");

    gui::Gui myGui(window);

    gui::DefaultTheme theme(myGui);

    const std::array<std::string, 10> sceneNames = {
        "Empty",
        "ButtonDemo",
        "ChatBoxDemo",
        "CheckBoxDemo",
        "ColorPickerDemo",
        "DialogBoxDemo",
        "TextBoxDemo",
        "MenuBarDemo",
        "SliderDemo",
        "FullDemo"
    };
    size_t currentScene = 0;
    bool sceneChanged = true;

    auto sceneButton = gui::Button::create(theme, "sceneButton");
    connectDebugSignals(sceneButton, "sceneButton");
    sceneButton->setPosition(0.0f, window.getSize().y - 20.0f);
    sceneButton->onMousePress.connect([&](gui::Widget* /*w*/, sf::Mouse::Button button, const sf::Vector2f& /*pos*/) {
        if (button == sf::Mouse::Right && currentScene > 0) {
            --currentScene;
            sceneChanged = true;
        } else if (button == sf::Mouse::Left && currentScene + 1 < sceneNames.size()) {
            ++currentScene;
            sceneChanged = true;
        }
    });

    // Ignore allocation of the sceneButton so it isn't counted as a memory leak.
    widgetsAllocated.clear();

    while (window.isOpen()) {
        gui::Timer::updateTimers();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (myGui.processEvent(event)) {
                std::cout << "Event type " << event.type << " was consumed.\n";
            }
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                //window.setView(sf::View(sf::FloatRect({0.0f, 0.0f}, sf::Vector2f(window.getSize()))));
            }
        }

        if (sceneChanged) {
            gui::Timer::clearTimers();
            sceneButton->setLabel("Scene: " + sceneNames[currentScene] + " (left/right click here)");
            myGui.removeAllChildren();
            myGui.onWindowResized.disconnectAll();

            bool foundMemoryLeaks = false;
            for (const auto& allocated : widgetsAllocated) {
                if (allocated.second.expired()) {
                    std::cout << "Widget " << widgetNames[allocated.first] << " has been destroyed properly.\n";
                } else {
                    foundMemoryLeaks = true;
                }
            }
            if (foundMemoryLeaks) {
                std::cout << "--------------------------------------------------\n";
                std::cout << "- Detected memory leaks:                         -\n";
                std::cout << "--------------------------------------------------\n";
                for (const auto& allocated : widgetsAllocated) {
                    if (!allocated.second.expired()) {
                        std::cout << "Widget " << widgetNames[allocated.first] << " not cleaned up! (ref count " << allocated.second.use_count() << ")\n";
                    }
                }
                assert(false);
            }
            widgetNames.clear();
            widgetsAllocated.clear();
            assert(widgetNames.emplace(sceneButton.get(), "sceneButton").second);
            myGui.addChild(sceneButton);
            sceneChanged = false;

            std::cout << "Building scene " << sceneNames[currentScene] << "\n";
            if (sceneNames[currentScene] == "Empty") {
            } else if (sceneNames[currentScene] == "ButtonDemo") {
                createButtonDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "ChatBoxDemo") {
                createChatBoxDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "CheckBoxDemo") {
                createCheckBoxDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "ColorPickerDemo") {
                createColorPickerDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "DialogBoxDemo") {
                createDialogBoxDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "TextBoxDemo") {
                createTextBoxDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "MenuBarDemo") {
                createMenuBarDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "SliderDemo") {
                createSliderDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "FullDemo") {
                createFullDemo(myGui, theme);
            } else {
                std::cerr << "Failed to build scene: name not found.\n";
            }
        }

        window.clear(sf::Color(40, 40, 40));
        window.draw(myGui);
        window.display();
    }

    return 0;
}
