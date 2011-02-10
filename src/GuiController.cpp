#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/Text.h"
#include "boost/algorithm/string.hpp"

#include "GuiController.h"

using namespace ci;
using namespace ci::app;
using std::list;
using std::vector;
using std::string;
using std::map;
using boost::shared_ptr;

using namespace netphy;

GuiController::GuiController()
{
}

void GuiController::update()
{
    list<GuiWidgetPtr> purged;
    for (list<GuiWidgetPtr>::iterator it = mWidgets.begin(); it != mWidgets.end(); ++it) {
        if (!(*it)->isPurged()) {
            (*it)->update();
        }
        else {
            purged.push_back(*it);
        }
    }

    for (list<GuiWidgetPtr>::iterator it = purged.begin(); it != purged.end(); ++it) {
        (*it)->detach();
    }
}

void GuiController::draw()
{
    for (list<GuiWidgetPtr>::iterator it = mWidgets.begin(); it != mWidgets.end(); ++it) {
        if (!(*it)->isPurged()) {
            (*it)->draw();
        }
    }
}

struct FindWidget {
    GuiWidget* target;
    FindWidget(GuiWidget* target) : target(target) { }
    bool operator() (GuiWidgetPtr& widget) { return widget.get() == target; }
};

void GuiController::detach(GuiWidget* ptr)
{
    list<GuiWidgetPtr>::iterator it = std::find_if(mWidgets.begin(), mWidgets.end(), FindWidget(ptr));
    if (it != mWidgets.end()) {
        mWidgets.erase(it);
    }
}

void GuiController::detachAll()
{
    //  release all root widgets
    mWidgets.clear();
}

GuiLabelWidgetPtr GuiController::createLabel(const GuiLabelData& spec, bool attachWidget)
{
    GuiLabelWidgetPtr label(new GuiLabelWidget(*this, spec));
    if (attachWidget) {
        attach(label);
    }
    return label;
}

GuiButtonWidgetPtr GuiController::createButton(vector<GuiWidgetPtr> children, bool attachWidget)
{
    GuiButtonWidgetPtr button(new GuiButtonWidget(*this));
    assert("Must supply child widgets for a button" && !children.empty());
    for (vector<GuiWidgetPtr>::iterator it = children.begin(); it != children.end(); ++it) {
        button->addChild(*it);
    }

    if (attachWidget) {
        attach(button);
    }

    return button;
}

GuiQuadWidgetPtr GuiController::createQuad(const GuiQuadData& data, bool attachWidget)
{
    GuiQuadWidgetPtr quad(new GuiQuadWidget(*this, data));
    if (attachWidget) {
        attach(quad);
    }
    return quad;
}

GuiBoxWidgetPtr GuiController::createBox(const GuiQuadData& data, const GuiBoxData& boxData, bool attachWidget)
{
    GuiBoxWidgetPtr quad(new GuiBoxWidget(*this, data, boxData));
    if (attachWidget) {
        attach(quad);
    }
    return quad;
}


void GuiController::attach(GuiWidgetPtr widget)
{
    //  ensure the widget is not purged before attaching
    widget->purge(false);
    mWidgets.push_back(widget);
}

bool GuiController::keyDown(KeyEvent event)
{
    for (list<GuiWidgetPtr>::iterator it=mWidgets.begin(); it != mWidgets.end(); ++it) {
        if ((*it)->keyDown(event)) {
            return true;
        }
    }
    return false;
}

void GuiController::mouseMove(MouseEvent event)
{
    for (list<GuiWidgetPtr>::iterator it=mWidgets.begin(); it != mWidgets.end(); ++it) {
        (*it)->mouseMove(event);
    }
}

bool GuiController::mouseDown(MouseEvent event)
{
    for (list<GuiWidgetPtr>::iterator it=mWidgets.begin(); it != mWidgets.end(); ++it) {
        if ((*it)->mouseDown(event)) {
            return true;
        }
    }
    return false;
}

bool GuiController::mouseUp(MouseEvent event)
{
    for (list<GuiWidgetPtr>::iterator it=mWidgets.begin(); it != mWidgets.end(); ++it) {
        if ((*it)->mouseUp(event)) {
            return true;
        }
    }
    return false;
}

void GuiController::mouseDrag(MouseEvent event)
{
}

bool GuiController::mouseWheel(MouseEvent event)
{
    return false;
}

Vec2f GuiWidget::getPos()
{
    return mPos;
}

Vec2f GuiWidget::getWorldPos()
{
    return mParent ? mParent->getWorldPos() + mPos : mPos;
}

Vec2f GuiWidget::getSize()
{
    return mSize;
}

void GuiWidget::setPos(const Vec2f& pos)
{
    mPos = pos;
}

void GuiWidget::addChild(GuiWidgetPtr child)
{
    mChildren.push_back(child);
}

void GuiWidget::detach()
{
    mGui.detach(this);
}

void GuiWidget::draw()
{
    // XXX problem with push/pop of matrix stack and GL transforms

    glPushMatrix();
    gl::translate(getPos());
    drawImpl();
    //  Draw children
    for (list<GuiWidgetPtr>::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
        (*it)->draw();
    }
    glPopMatrix();
}

void GuiWidget::update()
{
    //  Update children first, so parents can perform layout based on child dimensions
    for (list<GuiWidgetPtr>::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
        (*it)->update();
    }
    updateImpl();
}

void GuiWidget::slot(string name, GuiCallbackPtr callback)
{
    mCallbacks[name] = callback;
}

void GuiWidget::resetSlot(std::string name)
{
    mCallbacks.erase(name);
}

void GuiWidget::signal(string name)
{
    map<string, GuiCallbackPtr>::iterator cb = mCallbacks.find(name);
    if (cb != mCallbacks.end()) {
        //  invoke callback
        GuiCallbackPtr& callback = (*cb).second;
        (*callback)(name);
    }
}

GuiLabelWidget::GuiLabelWidget(GuiController& gui, const GuiLabelData& spec)
: GuiWidget(gui),
  mData(spec)
{
}

GuiLabelWidget::~GuiLabelWidget()
{
}

void GuiLabelWidget::updateImpl()
{
    TextLayout layout;
    layout.setFont(Font(mData.Font, mData.FontSize));
    layout.setColor(mData.FgColor);
    if (!mData.Text.empty()) {
        boost::split(mText, mData.Text, boost::is_any_of("\n"));
        for (vector<string>::iterator it = mText.begin(); it != mText.end(); ++it) {
            layout.addLine(*it);
        }
    }
    mTexture = gl::Texture(layout.render(true));
    mSize = mTexture.getSize();
    mTexture.unbind();
}

void GuiLabelWidget::drawImpl()
{
    gl::color(ColorA(1, 1, 1, 1));
    mTexture.bind();
    gl::draw(mTexture, Vec2f::zero());
    mTexture.unbind();
}

GuiButtonWidget::GuiButtonWidget(GuiController& gui)
: GuiWidget(gui), mMouseDown(false), mMouseInside(false)
{
}

void GuiButtonWidget::updateImpl()
{
    if (mChildren.begin() != mChildren.end()) {
        mSize = getFirstChild()->getSize();
    }
}

void GuiButtonWidget::drawImpl()
{
}

bool GuiButtonWidget::mouseDown(MouseEvent event)
{
    Vec2i mousePos = event.getPos();
    Vec2i buttonSize = (*mChildren.begin())->getSize();

    Vec2i pos(getPos());
    Area clickArea(pos, pos + buttonSize);
    if (clickArea.isInside(mousePos)) {
        // XXX call delegate, eat the event, draw/animate
        signal("mouseDown");
        mMouseDown = true;
        return true;
    }

    return false;
}

bool GuiButtonWidget::mouseUp(MouseEvent event)
{
    Vec2i mousePos = event.getPos();
    Vec2i buttonSize = (*mChildren.begin())->getSize();

    Vec2i pos(getPos());
    Area clickArea(pos, pos + buttonSize);
    if (clickArea.isInside(mousePos)) {
        if (mMouseDown) {
            mMouseDown = false;
            signal("mouseClick");
        }
        signal("mouseUp");
        return true;
    }

    return false;
}

void GuiButtonWidget::mouseMove(MouseEvent event)
{
    Vec2f mousePos(event.getPos());

    Vec2i pos(getPos());
    Area buttonArea(pos, pos + getSize());
    if (buttonArea.isInside(mousePos)) {
        if (!mMouseInside) {
            mMouseInside = true;
            signal("mouseEnter");
        }
    }
    else {
        if (mMouseInside) {
            signal("mouseExit");
        }
        mMouseInside = false;
        mMouseDown = false;
    }
}


GuiQuadWidget::GuiQuadWidget(GuiController& gui, const GuiQuadData& spec)
    : GuiWidget(gui), mData(spec)
{
}

GuiQuadWidget::~GuiQuadWidget()
{
}

void GuiQuadWidget::updateImpl()
{
}

void GuiQuadWidget::drawImpl()
{
    gl::color(mData.Color);
    gl::drawSolidRect(mData.Rect, false);
}

GuiBoxWidget::GuiBoxWidget(GuiController& gui, const GuiQuadData& quadData, const GuiBoxData& boxData)
: GuiQuadWidget(gui, quadData), mBoxData(boxData)
{
}

GuiBoxWidget::~GuiBoxWidget()
{
}

void GuiBoxWidget::updateImpl()
{
    //  update size based on child size + padding
    if (mChildren.begin() != mChildren.end()) {
        GuiWidgetPtr child = *(mChildren.begin());
        Vec2f childSize(child->getSize());
        const float padding = mBoxData.Padding;
        mSize = Vec2f(2.0f*padding + childSize.x, 2.0f*padding + childSize.y);
        mData.Rect = Rectf(Vec2f::zero(), mSize);
        child->setPos(Vec2f(padding, padding));
    }
}

struct TextButtonHighlight : public GuiCallback
{
    GuiButtonWidgetPtr mButton;

    TextButtonHighlight::TextButtonHighlight(GuiButtonWidgetPtr button)
        : mButton(button) {}

    bool operator()(string signal) {
        GuiLabelData& label = boost::static_pointer_cast<GuiLabelWidget, GuiWidget>(mButton->getFirstChild()->getFirstChild())->getData();
        if (signal == "mouseEnter") {
            label.FgColor = ColorA(1.0f, 1.0f, 0, 1.0f);
        }
        else if (signal == "mouseExit") {
            label.FgColor = ColorA(1.0f, 1.0f, 1.0f, 1.0f);
        }

        return false;
    }
};

GuiButtonWidgetPtr GuiFactory::createTextButton(string text, float fontSize, bool attach)
{
    vector<GuiWidgetPtr> buttonWidgets;

    GuiQuadData buttonBox;
    buttonBox.Color = ColorA(0, 0, 0, 0.75f);
    GuiBoxData  boxData;
    boxData.Padding = 8.0f;

    GuiBoxWidgetPtr boxWidget = mGui.createBox(buttonBox, boxData, false);
    boxWidget->setPos(Vec2f::zero());
    buttonWidgets.push_back(boxWidget);

    GuiLabelData buttonLabel;
    buttonLabel.FontSize = fontSize;
    buttonLabel.Text = text;
    GuiLabelWidgetPtr label = mGui.createLabel(buttonLabel, false);
    label->setPos(Vec2f(0, 0));
    boxWidget->addChild(label);
    
    GuiButtonWidgetPtr button = mGui.createButton(buttonWidgets, attach);

    //  Add text button label highlight
    GuiCallbackPtr highlight(new TextButtonHighlight(button));
    button->slot("mouseEnter", highlight);
    button->slot("mouseExit", highlight);

    return button;
}

ConsoleInputBuffer::ConsoleInputBuffer()
: mInputBuffer(""), mCursorPos(0), mInput("")
{
}

ConsoleInputBuffer::~ConsoleInputBuffer()
{
}

void ConsoleInputBuffer::insertCharAtCursor(char ch)
{
    int buflen = mInputBuffer.length();
    if (mCursorPos == buflen) {
        mInputBuffer += ch;
        ++mCursorPos;
    }
    else {
        mInputBuffer.insert(mCursorPos, &ch, 1);
        ++mCursorPos;
    }
}

void ConsoleInputBuffer::backspace()
{
    if (mCursorPos > 0) {
        mInputBuffer.erase(--mCursorPos, 1);
    }
}

void ConsoleInputBuffer::saveInput()
{
    mInput = mInputBuffer;
    mInputBuffer = "";
    mCursorPos = 0;
}

string ConsoleInputBuffer::getInputBuffer()
{
    return mInputBuffer;
}

string ConsoleInputBuffer::getInput()
{
    return mInput;
}

GuiConsolePtr GuiFactory::createConsole(int lines, float fontSize, bool attach)
{
    GuiConsolePtr console(new GuiConsole(mGui, lines, fontSize));
    if (attach) {
        mGui.attach(console);
    }
    return console;
}

//  linecount includes the bottom input line
GuiConsole::GuiConsole(GuiController& gui, int lineCount, float fontSize)
    : GuiWidget(gui), mLineCount(lineCount-1), mStream(*this) // , mInputBuffer("")
{
    GuiLabelData labelData;
    labelData.Font = "Droid Sans Mono";
    labelData.FontSize = fontSize;
    labelData.FgColor = ColorA(1.0f, 1.0f, 1.0f, 1.0f);

    for (int i=0; i < mLineCount; ++i) {
        GuiLabelWidgetPtr label = gui.createLabel(labelData, false);
        mLines.push_back(label);
        addChild(label);
    }

    //  User input line
    mInputLine = gui.createLabel(labelData, false);
    addChild(mInputLine);
}

void GuiConsole::appendString(const std::string& text)
{
    if (mBuffer.empty()) {
        mBuffer.push_back(std::string(""));
    }

    string& last = mBuffer.back();
    vector<string> lines;
    boost::split(lines, text, boost::is_any_of("\n"));
    for (vector<string>::iterator it = lines.begin(); it != lines.end(); ++it) {
        if (it == lines.begin()) {
            //  append to last line in buffer
            std::stringstream ss;
            ss << last << (*it);
            last = ss.str();
        }
        else {
            //  append to new line
            mBuffer.push_back(*it);
        }
    }
}

std::streamsize GuiConsoleStream::write(const char* s, std::streamsize n)
{
    string fragment(s, s + n);
    // mConsole.addString(fragment);
    mConsole.appendString(fragment);
    return n;
}

GuiConsoleOutput GuiConsole::output()
{
    return boost::iostreams::stream<GuiConsoleStream>(mStream);
}

void GuiConsole::updateImpl()
{
    assert(mLineCount > 0 && mLineCount < 512);

    //  set mConsoleStart to beginning of display buffer
    if (!mBuffer.empty()) {
        int linecount = mLineCount;
        for (mConsoleStart = --(mBuffer.end()); 
             linecount-- && mConsoleStart != mBuffer.begin();
             --mConsoleStart) ;
    }
    else {
        mConsoleStart = mBuffer.begin();
    }

    list<string>::iterator buf = mConsoleStart;
    const float lineSpacing = 1.0f;
    float yy = 0;
    float lineHeight = mInputLine->getSize().y;

    for (list<GuiLabelWidgetPtr>::iterator it = mLines.begin(); it != mLines.end(); ++it) {
        Vec2f labelSize((*it)->getSize());
        GuiLabelData& labelData = (*it)->getData();
        (*it)->setPos(Vec2f(0, yy));
        yy += lineSpacing + lineHeight;

        if (buf != mBuffer.end()) {
            labelData.Text = *buf++;
        }
        else {
            labelData.Text = "";
        }
    }

    //  Input line
    mInputLine->setPos(Vec2f(0, yy));
    GuiLabelData& inputData = mInputLine->getData();
    inputData.Text = string("> ") + mConsoleBuffer.getInputBuffer() + string("_");
    inputData.FgColor = ColorA(1.0f, 1.0f, 0, 1.0f);
    yy += lineSpacing + lineHeight;

    //  Set height
    mSize = Vec2f(mSize.x, yy);
}

void GuiConsole::drawImpl()
{
    //  drawing automatically triggered on child widgets
    Vec2f size(getSize());
    gl::color(ColorA(0.1f, 0.1f, 0.2f, 0.8));
    gl::drawSolidRect(Rectf(0, 0, size.x, size.y));
}

void GuiConsole::clear()
{
    mBuffer.clear();
}

void GuiConsole::setWidth(float width)
{
    mSize.x = width;
}

bool GuiConsole::keyDown(KeyEvent event)
{
    // XXX shouldn't have to write this for every widget
    if (isPurged()) return false;

    char ch = event.getChar();
    int keycode = event.getCode();
    bool ret = true;

    if (keycode == app::KeyEvent::KEY_RETURN) {
        // echo output
        mConsoleBuffer.saveInput();
        string input = mConsoleBuffer.getInput();
        appendString(input + "\n");
        //mInput = mInputBuffer.str();
        //appendString(mInput + "\n");
        //mInputBuffer.str("");
        // trigger textInput callbacks
        signal("textInput");
    }
    else if (keycode == app::KeyEvent::KEY_BACKQUOTE) {
        // XXX can't detach because it modifies the 
        // detach();
        purge();
    }
    else if (keycode == app::KeyEvent::KEY_BACKSPACE) {
        mConsoleBuffer.backspace();
    }
    else if (keycode >= app::KeyEvent::KEY_SPACE && keycode < app::KeyEvent::KEY_DELETE) {
        mConsoleBuffer.insertCharAtCursor(ch);
        //mInputBuffer << ch;
    }
    else {
        ret = false;
    }

    return ret;
}

std::string GuiConsole::getInput()
{
    return mConsoleBuffer.getInput();
}

