#include <gui/Gui.h>
#include <gui/themes/DefaultTheme.h>
#include <gui/widgets/Button.h>
#include <gui/widgets/CheckBox.h>
#include <gui/widgets/DialogBox.h>
#include <gui/widgets/Group.h>
#include <gui/widgets/Label.h>
#include <gui/widgets/MenuBar.h>
#include <gui/widgets/MultilineTextBox.h>
#include <gui/widgets/Panel.h>
#include <gui/widgets/RadioButton.h>
#include <gui/widgets/TextBox.h>

#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

std::map<gui::Widget*, std::string> widgetNames;

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

void connectDebugSignals(gui::Widget* widget, const std::string& name) {
    assert(widgetNames.emplace(widget, name).second);
    widget->onMouseEnter.connect(mouseEnter);
    widget->onMouseLeave.connect(mouseLeave);
    widget->onFocusGained.connect(focusGained);
    widget->onFocusLost.connect(focusLost);
}
void connectDebugSignals(gui::Button* button, const std::string& name) {
    connectDebugSignals(dynamic_cast<gui::Widget*>(button), name);
    button->onMousePress.connect(mousePress);
    button->onMouseRelease.connect(mouseRelease);
    button->onClick.connect(click);
}
void connectDebugSignals(gui::CheckBox* checkBox, const std::string& name) {
    connectDebugSignals(dynamic_cast<gui::Button*>(checkBox), name);
}
void connectDebugSignals(gui::Group* group, const std::string& name) {
    connectDebugSignals(dynamic_cast<gui::Widget*>(group), name);
}
void connectDebugSignals(gui::Label* label, const std::string& name) {
    connectDebugSignals(dynamic_cast<gui::Widget*>(label), name);
}
void connectDebugSignals(gui::Panel* panel, const std::string& name) {
    connectDebugSignals(dynamic_cast<gui::Group*>(panel), name);
}
void connectDebugSignals(gui::TextBox* textBox, const std::string& name) {
    connectDebugSignals(dynamic_cast<gui::Widget*>(textBox), name);
    textBox->onMousePress.connect(mousePress);
    textBox->onMouseRelease.connect(mouseRelease);
    textBox->onClick.connect(click);
    textBox->onTextChange.connect(textChange1);
    textBox->onEnterPressed.connect(enterPressed);
}
void connectDebugSignals(gui::MultilineTextBox* multilineTextBox, const std::string& name) {
    connectDebugSignals(dynamic_cast<gui::Widget*>(multilineTextBox), name);
    multilineTextBox->onMousePress.connect(mousePress);
    multilineTextBox->onMouseRelease.connect(mouseRelease);
    multilineTextBox->onClick.connect(click);
    multilineTextBox->onTextChange.connect(textChange2);
}
void connectDebugSignals(gui::MenuBar* menuBar, const std::string& name) {
    connectDebugSignals(dynamic_cast<gui::Widget*>(menuBar), name);
    menuBar->onMousePress.connect(mousePress);
    menuBar->onMouseRelease.connect(mouseRelease);
    menuBar->onClick.connect(click);
    menuBar->onMenuItemClick.connect(menuItemClick);
}

sf::Color getRandColor() {
    return {static_cast<uint8_t>(rand() % 256), static_cast<uint8_t>(rand() % 256), static_cast<uint8_t>(rand() % 256)};
}

void createButtonDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto greetButton = gui::Button::create(theme);
    connectDebugSignals(greetButton.get(), "greetButton");
    greetButton->setLabel(sf::String("hello!"));
    greetButton->setRotation(18.0f);
    greetButton->setOrigin(greetButton->getSize() * 0.5f);
    greetButton->setPosition(130.0f, 100.0f);
    greetButton->setScale(1.6f, 2.5f);
    greetButton->onClick.connect([=]{ greetButton->setLabel("nice to meet you"); });
    myGui.addChild(greetButton);

    auto hideButton = gui::Button::create(theme);
    connectDebugSignals(hideButton.get(), "hideButton");
    hideButton->setLabel("click to hide");
    hideButton->setPosition(10.0f, 10.0f);
    hideButton->onClick.connect([=]{ hideButton->setVisible(false); });
    myGui.addChild(hideButton);

    auto disableButton = gui::Button::create(theme);
    connectDebugSignals(disableButton.get(), "disableButton");
    disableButton->setLabel("click to disable");
    disableButton->setPosition(200.0f, 10.0f);
    disableButton->onClick.connect([=]{ disableButton->setEnabled(false); });
    myGui.addChild(disableButton);

    auto resetButton = gui::Button::create(theme);
    connectDebugSignals(resetButton.get(), "resetButton");
    resetButton->setLabel("click to reset others");
    resetButton->setPosition(10.0f, 70.0f);
    resetButton->onClick.connect([=]{ hideButton->setVisible(true); disableButton->setEnabled(true); });
    myGui.addChild(resetButton);

    auto buttonA = gui::Button::create(theme);
    connectDebugSignals(buttonA.get(), "buttonA");
    buttonA->setLabel("A (front)");
    buttonA->setPosition(10.0f, 170.0f);
    buttonA->onClick.connect([=]{ buttonA->sendToFront(); });
    myGui.addChild(buttonA);

    auto buttonB = gui::Button::create(theme);
    connectDebugSignals(buttonB.get(), "buttonB");
    buttonB->setLabel("B (front)");
    buttonB->setPosition(50.0f, 170.0f);
    buttonB->onClick.connect([=]{ buttonB->sendToFront(); });
    myGui.addChild(buttonB);

    auto buttonC = gui::Button::create(theme);
    connectDebugSignals(buttonC.get(), "buttonC");
    buttonC->setLabel("C (back)");
    buttonC->setPosition(30.0f, 185.0f);
    buttonC->onClick.connect([=]{ buttonC->sendToBack(); });
    myGui.addChild(buttonC);


    auto moveButton = gui::Button::create(theme);
    connectDebugSignals(moveButton.get(), "moveButton");
    moveButton->setLabel("Hover to move");
    moveButton->setPosition(200.0f, 70.0f);
    moveButton->onMouseEnter.connect([=]{ moveButton->setPosition(200.0f, 60.0f); moveButton->setRotation(15); });
    moveButton->onMouseLeave.connect([=]{ moveButton->setPosition(200.0f, 70.0f); moveButton->setRotation(0); });
    myGui.addChild(moveButton);


    auto customStyled1 = gui::Button::create(theme);
    connectDebugSignals(customStyled1.get(), "customStyled1");
    auto customButtonStyle = customStyled1->getStyle();
    customButtonStyle->setTextPadding({0, 0, customButtonStyle->getTextPadding().z});
    customStyled1->setLabel("custom");
    customStyled1->setPosition(200.0f, 120.0f);
    customStyled1->onMouseEnter.connect([=]{ customButtonStyle->setTextStyle(sf::Text::Bold); });
    customStyled1->onMouseLeave.connect([=]{ customButtonStyle->setTextStyle(sf::Text::Regular); });
    myGui.addChild(customStyled1);

    auto customStyled2 = gui::Button::create(customStyled1->getStyle());
    connectDebugSignals(customStyled2.get(), "customStyled2");
    customStyled2->setLabel("styled");
    customStyled2->setPosition(200.0f, 150.0f);
    customStyled2->onMouseEnter.connect([=]{ customStyled1->getStyle()->setTextStyle(sf::Text::Bold); });
    customStyled2->onMouseLeave.connect([=]{ customStyled1->getStyle()->setTextStyle(sf::Text::Regular); });
    myGui.addChild(customStyled2);

    // This button switches to a different style on hover and also disables auto-resize.
    auto customStyled3 = gui::Button::create(customStyled1->getStyle());
    connectDebugSignals(customStyled3.get(), "customStyled3");
    customStyled3->setLabel("buttons");
    customStyled3->setAutoResize(false);
    customStyled3->setPosition(200.0f, 180.0f);
    customStyled3->onMouseEnter.connect([=]{ customStyled3->getStyle()->setTextStyle(sf::Text::Underlined); customStyled3->getStyle()->setFillColor({90, 90, 200}); });
    customStyled3->onMouseLeave.connect([=]{ customStyled3->getStyle()->setTextStyle(sf::Text::Regular); });
    myGui.addChild(customStyled3);

    customButtonStyle->setFillColor({20, 20, 200});


    auto panel = gui::Panel::create(theme);
    connectDebugSignals(panel.get(), "panel");
    panel->setSize({100, 90});
    panel->setOrigin(5.0f, 15.0f);
    panel->setPosition(50.0f, 250.0f);
    //panel->setRotation(30.0f);
    myGui.addChild(panel);

    auto panelLabel = gui::Label::create(theme);
    connectDebugSignals(panelLabel.get(), "panelLabel");
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
    connectDebugSignals(paintButton.get(), "paintButton");
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

void createCheckBoxDemo(gui::Gui& myGui, const gui::Theme& theme) {
    // Check boxes.
    auto testCheckBox = gui::CheckBox::create(theme);
    connectDebugSignals(testCheckBox.get(), "testCheckBox");
    testCheckBox->setLabel(sf::String("check box"));
    testCheckBox->setPosition(10.0f, 10.0f);
    testCheckBox->onClick.connect([=]() {
        if (testCheckBox->isChecked()) {
            testCheckBox->setLabel("the box is checked!");
        } else {
            testCheckBox->setLabel("the box is not checked");
        }
    });
    myGui.addChild(testCheckBox);

    auto disabledCheckBox = gui::CheckBox::create(theme);
    connectDebugSignals(disabledCheckBox.get(), "disabledCheckBox");
    disabledCheckBox->setLabel(sf::String("check box (disabled)"));
    disabledCheckBox->setPosition(10.0f, 70.0f);
    disabledCheckBox->setEnabled(false);
    myGui.addChild(disabledCheckBox);

    auto checkBoxPanel = gui::Panel::create(theme);
    connectDebugSignals(checkBoxPanel.get(), "checkBoxPanel");
    checkBoxPanel->setPosition(10.0f, 130.0f);
    myGui.addChild(checkBoxPanel);

    auto checkBoxA = gui::CheckBox::create(theme);
    connectDebugSignals(checkBoxA.get(), "checkBoxA");
    checkBoxA->setLabel(sf::String("Option A"));
    checkBoxA->setPosition(8.0f, 8.0f);
    checkBoxPanel->addChild(checkBoxA);

    auto checkBoxB = gui::CheckBox::create(theme);
    connectDebugSignals(checkBoxB.get(), "checkBoxB");
    checkBoxB->setLabel(sf::String("Option B"));
    checkBoxB->setPosition(checkBoxA->getPosition() + sf::Vector2f(0.0f, checkBoxA->getSize().y + 8.0f));
    checkBoxPanel->addChild(checkBoxB);

    auto checkBoxC = gui::CheckBox::create(theme);
    connectDebugSignals(checkBoxC.get(), "checkBoxC");
    checkBoxC->setLabel(sf::String("Option C"));
    checkBoxC->setPosition(checkBoxB->getPosition() + sf::Vector2f(0.0f, checkBoxB->getSize().y + 8.0f));
    checkBoxPanel->addChild(checkBoxC);

    checkBoxPanel->setSize(checkBoxC->getPosition() + checkBoxC->getSize() + sf::Vector2f(8.0f, 8.0f));

    // Radio buttons.
    auto testRadioButton = gui::RadioButton::create(theme);
    connectDebugSignals(testRadioButton.get(), "testRadioButton");
    testRadioButton->setLabel(sf::String("radio button"));
    testRadioButton->setPosition(230.0f, 10.0f);
    testRadioButton->onClick.connect([=]() {
        if (testRadioButton->isChecked()) {
            testRadioButton->setLabel("the button is checked!");
        } else {
            testRadioButton->setLabel("the button is not checked");
        }
    });
    myGui.addChild(testRadioButton);

    auto disabledRadioButton = gui::RadioButton::create(theme);
    connectDebugSignals(disabledRadioButton.get(), "disabledRadioButton");
    disabledRadioButton->setLabel(sf::String("radio button (disabled)"));
    disabledRadioButton->setPosition(230.0f, 70.0f);
    disabledRadioButton->setEnabled(false);
    myGui.addChild(disabledRadioButton);

    auto radioButtonPanel = gui::Panel::create(theme);
    connectDebugSignals(radioButtonPanel.get(), "radioButtonPanel");
    radioButtonPanel->setPosition(230.0f, 130.0f);
    myGui.addChild(radioButtonPanel);

    auto radioButtonGroup = gui::Group::create();
    connectDebugSignals(radioButtonGroup.get(), "radioButtonGroup");
    radioButtonPanel->addChild(radioButtonGroup);

    auto radioButtonA = gui::RadioButton::create(theme);
    connectDebugSignals(radioButtonA.get(), "radioButtonA");
    radioButtonA->setLabel(sf::String("Option A"));
    radioButtonA->setPosition(8.0f, 8.0f);
    radioButtonGroup->addChild(radioButtonA);

    auto radioButtonB = gui::RadioButton::create(theme);
    connectDebugSignals(radioButtonB.get(), "radioButtonB");
    radioButtonB->setLabel(sf::String("Option B"));
    radioButtonB->setPosition(radioButtonA->getPosition() + sf::Vector2f(0.0f, radioButtonA->getSize().y + 8.0f));
    radioButtonGroup->addChild(radioButtonB);

    auto radioButtonC = gui::RadioButton::create(theme);
    connectDebugSignals(radioButtonC.get(), "radioButtonC");
    radioButtonC->setLabel(sf::String("Option C"));
    radioButtonC->setPosition(radioButtonB->getPosition() + sf::Vector2f(0.0f, radioButtonB->getSize().y + 8.0f));
    radioButtonGroup->addChild(radioButtonC);

    auto radioButtonD = gui::RadioButton::create(theme);
    connectDebugSignals(radioButtonD.get(), "radioButtonD");
    radioButtonD->setLabel(sf::String("Option D (outside group)"));
    radioButtonD->setPosition(radioButtonC->getPosition() + sf::Vector2f(0.0f, radioButtonC->getSize().y + 8.0f));
    radioButtonPanel->addChild(radioButtonD);

    radioButtonPanel->setSize(radioButtonD->getPosition() + radioButtonD->getSize() + sf::Vector2f(8.0f, 8.0f));

    auto radioResetButton = gui::Button::create(theme);
    connectDebugSignals(radioResetButton.get(), "radioResetButton");
    radioResetButton->setLabel("Reset");
    radioResetButton->setPosition(radioButtonPanel->getPosition() + sf::Vector2f(0.0f, radioButtonPanel->getSize().y + 8.0f));
    radioResetButton->onClick.connect([=]() {
        radioButtonA->uncheckRadioButtons();
        radioButtonD->uncheckRadioButtons();
    });
    myGui.addChild(radioResetButton);
}

void createDialogBoxDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto dialogTest = gui::DialogBox::create(theme);
    connectDebugSignals(dialogTest.get(), "dialogTest");
    dialogTest->setSize({200.0f, 100.0f});
    dialogTest->setPosition(80.0f, 80.0f);
    myGui.addChild(dialogTest);

    /*auto panelThing = gui::Panel::create(theme);
    connectDebugSignals(panelThing.get(), "panelThing");
    panelThing->setSize({400.0f, 300.0f});
    panelThing->setRotation(14.0f);
    panelThing->setScale(1.8f, 1.3f);
    panelThing->setPosition(90.0f, 70.0f);
    panelThing->addChild(dialogTest);
    myGui.addChild(panelThing);

    auto dialogTest2 = gui::DialogBox::create(theme);
    connectDebugSignals(dialogTest2.get(), "dialogTest2");
    dialogTest2->setSize({130.0f, 70.0f});
    dialogTest2->setPosition(140.0f, 230.0f);
    panelThing->addChild(dialogTest2);*/

    auto dialogTestTitle = gui::Label::create(theme);
    connectDebugSignals(dialogTestTitle.get(), "dialogTestTitle");
    dialogTestTitle->setLabel("my title");
    dialogTest->setTitle(dialogTestTitle);

    auto dialogTestLabel = gui::Label::create(theme);
    connectDebugSignals(dialogTestLabel.get(), "dialogTestLabel");
    dialogTestLabel->setLabel("Name:");
    dialogTestLabel->setPosition(10.0f, 20.0f);
    dialogTest->addChild(dialogTestLabel);

    auto dialogTestTextBox = gui::TextBox::create(theme);
    connectDebugSignals(dialogTestTextBox.get(), "dialogTestTextBox");
    dialogTestTextBox->setWidthCharacters(8);
    dialogTestTextBox->setPosition(100.0f, 20.0f);
    dialogTest->addChild(dialogTestTextBox);

    auto dialogTestTextBox2 = gui::MultilineTextBox::create(theme);
    connectDebugSignals(dialogTestTextBox2.get(), "dialogTestTextBox2");
    dialogTestTextBox2->setSizeCharacters({8, 1});
    dialogTestTextBox2->setMaxLines(1);
    dialogTestTextBox2->setTabPolicy(gui::MultilineTextBox::TabPolicy::ignoreTab);
    dialogTestTextBox2->setPosition(100.0f, 40.0f);
    dialogTest->addChild(dialogTestTextBox2);

    auto dialogTestCancel = gui::Button::create(theme);
    connectDebugSignals(dialogTestCancel.get(), "dialogTestCancel");
    dialogTestCancel->setLabel("Cancel");
    dialogTestCancel->setPosition(10.0f, 60.0f);
    dialogTestCancel->onClick.connect([=]() {
        std::cout << "Dialog cancelled!\n";
        dialogTest->setSize(dialogTest->getSize() - sf::Vector2f(100.0f, 70.0f));
    });
    dialogTest->setCancelButton(0, dialogTestCancel);

    auto dialogTestSubmit = gui::Button::create(theme);
    connectDebugSignals(dialogTestSubmit.get(), "dialogTestSubmit");
    dialogTestSubmit->setLabel("Rename");
    dialogTestSubmit->setPosition(100.0f, 60.0f);
    dialogTestSubmit->onClick.connect([=]() {
        std::cout << "Dialog submitted!\n";
        dialogTest->setSize(dialogTest->getSize() + sf::Vector2f(100.0f, 70.0f));
    });
    dialogTest->setSubmitButton(1, dialogTestSubmit);

    const std::string alignmentToString[3] = {
        "left",
        "center",
        "right"
    };

    auto dialogTitleAlignment = gui::Button::create(theme);
    connectDebugSignals(dialogTitleAlignment.get(), "dialogTitleAlignment");
    dialogTitleAlignment->setLabel("Title: " + alignmentToString[static_cast<int>(dialogTest->getTitleAlignment())]);
    dialogTitleAlignment->setPosition(10.0f, 10.0f);
    dialogTitleAlignment->onClick.connect([=]() {
        dialogTest->setTitleAlignment(static_cast<gui::DialogBox::Alignment>((static_cast<int>(dialogTest->getTitleAlignment()) + 1) % 3));
        dialogTitleAlignment->setLabel("Title: " + alignmentToString[static_cast<int>(dialogTest->getTitleAlignment())]);
    });
    myGui.addChild(dialogTitleAlignment);

    auto dialogButtonAlignment = gui::Button::create(theme);
    connectDebugSignals(dialogButtonAlignment.get(), "dialogButtonAlignment");
    dialogButtonAlignment->setLabel("Buttons: " + alignmentToString[static_cast<int>(dialogTest->getButtonAlignment())]);
    dialogButtonAlignment->setPosition(10.0f, 40.0f);
    dialogButtonAlignment->onClick.connect([=]() {
        dialogTest->setButtonAlignment(static_cast<gui::DialogBox::Alignment>((static_cast<int>(dialogTest->getButtonAlignment()) + 1) % 3));
        dialogButtonAlignment->setLabel("Buttons: " + alignmentToString[static_cast<int>(dialogTest->getButtonAlignment())]);
    });
    myGui.addChild(dialogButtonAlignment);

    // Trigger focus on the text box.
    dialogTest->setVisible(true);
}

void createTextBoxDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto hideBox = gui::TextBox::create(theme);
    connectDebugSignals(hideBox.get(), "hideBox");
    hideBox->setWidthCharacters(16);
    hideBox->setText("right click hide");
    hideBox->setPosition(10.0f, 10.0f);
    hideBox->onMousePress.connect([=](gui::Widget* /*w*/, sf::Mouse::Button button, const sf::Vector2f& /*pos*/){
        if (button == sf::Mouse::Right) {
            hideBox->setVisible(false);
        }
    });
    myGui.addChild(hideBox);

    auto disableBox = gui::TextBox::create(theme);
    connectDebugSignals(disableBox.get(), "disableBox");
    disableBox->setWidthCharacters(16);
    disableBox->setText("right click disable");
    disableBox->setPosition(200.0f, 10.0f);
    disableBox->onMousePress.connect([=](gui::Widget* /*w*/, sf::Mouse::Button button, const sf::Vector2f& /*pos*/){
        if (button == sf::Mouse::Right) {
            disableBox->setEnabled(false);
        }
    });
    myGui.addChild(disableBox);

    auto resetBox = gui::TextBox::create(theme);
    connectDebugSignals(resetBox.get(), "resetBox");
    resetBox->setWidthCharacters(16);
    resetBox->setReadOnly(true);
    resetBox->setDefaultText("enter to reset  (trimmed text)");
    resetBox->setPosition(10.0f, 70.0f);
    resetBox->onEnterPressed.connect([=]{ hideBox->setVisible(true); disableBox->setEnabled(true); });
    myGui.addChild(resetBox);

    auto maxCharBox = gui::TextBox::create(theme);
    connectDebugSignals(maxCharBox.get(), "maxCharBox");
    maxCharBox->setWidthCharacters(16);
    maxCharBox->setMaxCharacters(9);
    maxCharBox->setDefaultText("max chars");
    maxCharBox->setPosition(200.0f, 90.0f);
    maxCharBox->setRotation(-33.0f);
    myGui.addChild(maxCharBox);
    maxCharBox->sendToBack();

    auto multilineBox = gui::MultilineTextBox::create(theme);
    connectDebugSignals(multilineBox.get(), "multilineBox");
    multilineBox->setSizeCharacters({16, 3});
    multilineBox->setText("This is my really long batch\nof text for testing selections within a\ntext box,\nhopefully it works?\nwe'll see...");
    multilineBox->setDefaultText("Enter text:");
    multilineBox->setPosition(10.0f, 130.0f);
    //multilineBox->setOrigin(-40.0f, -90.0f);
    //multilineBox->setRotation(2.0f);
    myGui.addChild(multilineBox);

    auto multilineBox2 = gui::MultilineTextBox::create(theme);
    connectDebugSignals(multilineBox2.get(), "multilineBox2");
    multilineBox2->setSizeCharacters({16, 3});
    multilineBox2->setMaxLines(3);
    multilineBox2->setDefaultText("Here is some default\ntext, this box\nhas max of 3 lines\nin it.");
    multilineBox2->setPosition(200.0f, 130.0f);
    myGui.addChild(multilineBox2);

    auto readOnlyCheckBox = gui::CheckBox::create(theme);
    connectDebugSignals(readOnlyCheckBox.get(), "readOnlyCheckBox");
    readOnlyCheckBox->setLabel("Read Only");
    readOnlyCheckBox->setPosition(multilineBox2->getPosition() + sf::Vector2f(0.0f, multilineBox2->getSize().y + 15.0f));
    readOnlyCheckBox->onClick.connect([=]() {
        multilineBox2->setReadOnly(readOnlyCheckBox->isChecked());
    });
    myGui.addChild(readOnlyCheckBox);
}

void createMenuBarDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto menuBar = gui::MenuBar::create(theme);
    connectDebugSignals(menuBar.get(), "menuBar");
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

    menuBar->onMenuItemClick.connect([=](gui::Widget* /*w*/, const gui::MenuList& menu, size_t index){
        size_t menuIndex = menuBar->findMenuIndex(menu.name);
        gui::MenuList newMenu = menu;
        if (newMenu.name.getSize() > 4) {
            newMenu.name = "menu" + std::to_string(std::stoi(newMenu.name.substring(4).toAnsiString()) + 1);
            newMenu.items[0].rightText = newMenu.name;
        }

        if (menu.items[index].leftText == "insert left") {
            menuBar->insertMenu(newMenu, menuIndex);
        } else if (menu.items[index].leftText == "insert right") {
            menuBar->insertMenu(newMenu, menuIndex + 1);
        } else if (menu.items[index].leftText == "remove left") {
            if (menuIndex > 0) {
                menuBar->removeMenu(menuIndex - 1);
            } else {
                std::cout << "No menu on the left side to remove!\n";
            }
        } else if (menu.items[index].leftText == "remove right") {
            menuBar->removeMenu(menuIndex + 1);
        } else if (menu.items[index].leftText == "remove me") {
            menuBar->removeMenu(menuIndex);
        } else if (menu.items[index].leftText == "ping") {
            newMenu = menu;
            newMenu.items[index].rightText = "bounced";
            newMenu.items[index].enabled = false;
            newMenu.items[index + 1].enabled = true;
            menuBar->setMenu(newMenu, menuIndex);
        } else if (menu.items[index].leftText == "pong") {
            newMenu = menu;
            newMenu.items[index].rightText = "bounced again";
            newMenu.items[index].enabled = false;
            newMenu.items[index - 1].enabled = true;
            menuBar->setMenu(newMenu, menuIndex);
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

void createFullDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto menuBar = gui::MenuBar::create(theme, "menuBar");
    connectDebugSignals(menuBar.get(), "menuBar");
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
    connectDebugSignals(modalBackground.get(), "modalBackground");
    modalBackground->setSize(static_cast<sf::Vector2f>(myGui.getSize()));
    modalBackground->setVisible(false);
    modalBackground->setFocusable(false);
    modalBackground->getStyle()->setFillColor({0, 0, 0, 175});
    modalBackground->getStyle()->setOutlineThickness(0.0f);
    myGui.addChild(modalBackground);

    auto renameDialog = gui::DialogBox::create(theme, "renameDialog");
    connectDebugSignals(renameDialog.get(), "renameDialog");
    renameDialog->setSize({200.0f, 100.0f});
    modalBackground->addChild(renameDialog);

    auto renameTitle = gui::Label::create(theme, "renameTitle");
    connectDebugSignals(renameTitle.get(), "renameTitle");
    renameTitle->setLabel("Rename");
    renameDialog->setTitle(renameTitle);

    auto renameLabel = gui::Label::create(theme, "renameLabel");
    connectDebugSignals(renameLabel.get(), "renameLabel");
    renameLabel->setLabel("Name:");
    renameLabel->setPosition(10.0f, 30.0f);
    renameDialog->addChild(renameLabel);

    auto renameTextBox = gui::MultilineTextBox::create(theme, "renameTextBox");
    connectDebugSignals(renameTextBox.get(), "renameTextBox");
    renameTextBox->setSizeCharacters({8, 1});
    renameTextBox->setMaxLines(1);
    renameTextBox->setTabPolicy(gui::MultilineTextBox::TabPolicy::ignoreTab);
    renameTextBox->setPosition(100.0f, 30.0f);
    renameDialog->addChild(renameTextBox);

    // FIXME: possible bug here? when dragging mouse outside dialog when open and text box is focused, it picks up drag events like they are hovering the text.
    // may need to check in other places, now that mouse events sent to focused widget too.

    auto renameCancelButton = gui::Button::create(theme, "renameCancelButton");
    connectDebugSignals(renameCancelButton.get(), "renameCancelButton");
    renameCancelButton->setLabel("Cancel");
    renameCancelButton->onClick.connect([=](){
        renameDialog->setVisible(false);
        modalBackground->setVisible(false);
    });
    renameDialog->setCancelButton(0, renameCancelButton);

    auto renameSubmitButton = gui::Button::create(theme, "renameSubmitButton");
    connectDebugSignals(renameSubmitButton.get(), "renameSubmitButton");
    renameSubmitButton->setLabel("Ok");
    renameDialog->setSubmitButton(1, renameSubmitButton);

    // Look up the rename button by name as a test.
    myGui.getChild<gui::Button>("renameSubmitButton")->onClick.connect([=](){
        renameDialog->setVisible(false);
        modalBackground->setVisible(false);
        std::cout << "Rename dialog submitted!\n";
    });
    assert(myGui.getChild<gui::Button>("renameSubmitButton", false) == nullptr);
    assert(myGui.getChild("renameSubmitButton", false) == nullptr);
    assert(myGui.getChild("idk") == nullptr);

    menuBar->onMenuItemClick.connect([=](gui::Widget* /*w*/, const gui::MenuList& menu, size_t index){
        if (menu.name == "File") {
            if (menu.items[index].leftText == "Rename...") {
                modalBackground->sendToFront();
                modalBackground->setVisible(true);
                renameDialog->setVisible(true);
                renameDialog->setPosition(
                    std::min(80.0f, modalBackground->getSize().x / 2.0f),
                    std::min(80.0f, modalBackground->getSize().y / 2.0f)
                );
            }
        }
    });

    auto centeredPanel = gui::Panel::create(theme, "centeredPanel");
    connectDebugSignals(centeredPanel.get(), "centeredPanel");
    myGui.addChild(centeredPanel);

    auto centeredLabel = gui::Label::create(theme, "centeredLabel");
    connectDebugSignals(centeredLabel.get(), "centeredLabel");
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
    testCallbacks();

    sf::RenderWindow window(sf::VideoMode(800, 600), "GUI Test");

    gui::Gui myGui(window);

    gui::DefaultTheme theme(myGui);

    const std::array<std::string, 7> sceneNames = {
        "Empty",
        "ButtonDemo",
        "CheckBoxDemo",
        "DialogBoxDemo",
        "TextBoxDemo",
        "MenuBarDemo",
        "FullDemo"
    };
    size_t currentScene = 0;
    bool sceneChanged = true;

    auto sceneButton = gui::Button::create(theme, "sceneButton");
    connectDebugSignals(sceneButton.get(), "sceneButton");
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

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            myGui.processEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                //window.setView(sf::View(sf::FloatRect({0.0f, 0.0f}, sf::Vector2f(window.getSize()))));
            }
        }

        if (sceneChanged) {
            sceneButton->setLabel("Scene: " + sceneNames[currentScene] + " (left/right click here)");
            myGui.removeAllChildren();
            myGui.onWindowResized.disconnectAll();
            widgetNames.clear();
            assert(widgetNames.emplace(sceneButton.get(), "sceneButton").second);
            myGui.addChild(sceneButton);
            sceneChanged = false;

            std::cout << "Building scene " << sceneNames[currentScene] << "\n";
            if (sceneNames[currentScene] == "Empty") {
            } else if (sceneNames[currentScene] == "ButtonDemo") {
                createButtonDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "CheckBoxDemo") {
                createCheckBoxDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "DialogBoxDemo") {
                createDialogBoxDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "TextBoxDemo") {
                createTextBoxDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "MenuBarDemo") {
                createMenuBarDemo(myGui, theme);
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
