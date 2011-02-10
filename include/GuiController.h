#pragma once

#include <iosfwd>
#include <list>
#include <boost/smart_ptr.hpp>
#include <boost/iostreams/categories.hpp>
#include <map>

#include <iostream>
#include <sstream>
#include <string>

#include "cinder/app/KeyEvent.h"
#include "cinder/app/MouseEvent.h"
#include "cinder/Color.h"
#include "cinder/Vector.h"
#include "cinder/gl/Texture.h"

namespace netphy
{

struct Shared;

class GuiController;

class GuiRenderer {
    GuiRenderer(GuiController& controller);
    void draw();

    GuiController& mGui;
};

struct GuiCallback
{
    virtual bool operator()(std::string) = 0;
};

struct GuiCallbackGG : public GuiCallback
{
    Shared& GG;
    GuiCallbackGG(Shared& shared) : GG(shared) { }
    explicit GuiCallbackGG();
    virtual bool operator()(std::string) = 0;
};
typedef boost::shared_ptr<GuiCallback> GuiCallbackPtr;


class GuiWidget;
typedef boost::shared_ptr<GuiWidget> GuiWidgetPtr;

class GuiWidget 
{
protected:
    std::map<std::string, GuiCallbackPtr> mCallbacks;

    explicit GuiWidget();
    GuiController&          mGui;
    GuiWidget*              mParent;
    std::list<GuiWidgetPtr> mChildren;
    ci::Vec2f               mPos;
    ci::Vec2f               mSize;

    virtual void drawImpl() { }
    virtual void updateImpl() { }

    bool mPurge;

public:
    GuiWidget(GuiController& gui, GuiWidget* parent=0) : mGui(gui), mParent(0), mPos(0,0), mPurge(false) { }
    virtual ~GuiWidget() {}

    void draw();
    void update();

    ci::Vec2f getPos();
    void      setPos(const ci::Vec2f& pos);
    ci::Vec2f getWorldPos();
    ci::Vec2f getSize();

    void addChild(GuiWidgetPtr child);
    GuiWidgetPtr getFirstChild() { return *(mChildren.begin()); }

    //  Detach from controller
    void detach();
    //  Purged widgets are safely detached from the controller at the end of update()
    void purge(bool value=true) { mPurge = value; }
    bool isPurged() { return mPurge; }


    virtual bool keyDown(ci::app::KeyEvent event) { return false; }
    virtual bool mouseDown(ci::app::MouseEvent event) { return false; }
    virtual bool mouseUp(ci::app::MouseEvent event)   { return false; }
    virtual void mouseMove(ci::app::MouseEvent event) { }

    void signal(std::string name);
    //  attach a callback to a given slot
    void slot(std::string name, GuiCallbackPtr callback);
    //  remove any callbacks from a given slot
    void resetSlot(std::string name);
};

struct GuiLabelData
{
    std::string Text;
    int         Justify;
    std::string Font;
    float       FontSize;
    ci::ColorA  FgColor;
    ci::ColorA  BgColor;

    GuiLabelData() 
        : Text(""), 
          Justify(0), 
          Font("Droid Sans"), 
          FontSize(16.0f), 
          FgColor(1.0f, 1.0f, 1.0f, 1.0f), 
          BgColor(0, 0, 0, 1.0f)
    { }
};

class GuiLabelWidget;
typedef boost::shared_ptr<GuiLabelWidget> GuiLabelWidgetPtr;

class GuiLabelWidget : public GuiWidget
{
protected:
    virtual void updateImpl();
    virtual void drawImpl();

    GuiLabelData mData;

    ci::gl::Texture          mTexture;
    std::vector<std::string> mText;

public:
    GuiLabelWidget(GuiController& gui, const GuiLabelData& spec);
    ~GuiLabelWidget();

    GuiLabelData& getData() { return mData; }
};

struct GuiQuadData
{
    ci::Rectf Rect;
    ci::ColorA Color;
};

class GuiQuadWidget : public GuiWidget
{
protected:
    virtual void updateImpl();
    virtual void drawImpl();

    GuiQuadData mData;

    ci::gl::Texture mTexture;

public:
    GuiQuadWidget(GuiController& gui, const GuiQuadData& spec);
    ~GuiQuadWidget();
    
    GuiQuadData& getData() { return mData; }
};
typedef boost::shared_ptr<GuiQuadWidget> GuiQuadWidgetPtr;

struct GuiBoxData
{
    float Padding;
};

class GuiBoxWidget : public GuiQuadWidget
{
protected:
    GuiBoxData mBoxData;
    virtual void updateImpl();

public:
    GuiBoxWidget(GuiController& gui, const GuiQuadData& quadData, const GuiBoxData& boxData);
    ~GuiBoxWidget();

    GuiBoxData& getBoxData() { return mBoxData; }
};
typedef boost::shared_ptr<GuiBoxWidget> GuiBoxWidgetPtr;

class GuiButtonWidget : public GuiWidget 
{
public:
    GuiButtonWidget(GuiController& gui);
    ~GuiButtonWidget() { }

    virtual bool mouseDown(ci::app::MouseEvent event);
    virtual bool mouseUp(ci::app::MouseEvent event);
    virtual void mouseMove(ci::app::MouseEvent event);

protected:
    bool mMouseDown;
    bool mMouseInside;

    virtual void updateImpl();
    virtual void drawImpl();
};
typedef boost::shared_ptr<GuiButtonWidget> GuiButtonWidgetPtr;

class ConsoleInputBuffer
{
public:
    ConsoleInputBuffer();
    ~ConsoleInputBuffer();

    void insertCharAtCursor(char ch);
    void backspace();
    void saveInput();
    std::string getInput();
    std::string getInputBuffer();

protected:
    // std::stringstream mBuffer;
    int mCursorPos;
    std::string mInput;
    std::string mInputBuffer;
};

class GuiConsole;
class GuiConsoleStream
{
public:
    GuiConsoleStream(GuiConsole& console) : mConsole(console) { }
    GuiConsoleStream(const GuiConsoleStream& stream) : mConsole(stream.mConsole) { }

    //  boost::iostreams implementation
    typedef char char_type;
    typedef boost::iostreams::sink_tag category;
    std::streamsize write(const char* s, std::streamsize n);

private:
    GuiConsole& mConsole;
};
typedef boost::iostreams::stream<GuiConsoleStream> GuiConsoleOutput;

class GuiConsole : public GuiWidget
{
public:
    GuiConsole(GuiController& gui, int lineCount, float fontSize);
    boost::iostreams::stream<GuiConsoleStream> output();

    void appendString(const std::string& text);
    //  clears buffer
    void clear();

    //  Set console widget width. Height is based on font height * number of lines
    void setWidth(float width);
    virtual bool keyDown(ci::app::KeyEvent event);

    std::string getInput();

protected:
    int mLineCount;
    std::list<std::string> mBuffer;
    std::list<GuiLabelWidgetPtr> mLines;
    //  The point in the buffer where the display starts
    std::list<std::string>::iterator mConsoleStart;

    ConsoleInputBuffer mConsoleBuffer;
    // std::string mInput;
    // std::stringstream mInputBuffer;
    GuiLabelWidgetPtr mInputLine;
    
    virtual void updateImpl();
    virtual void drawImpl();

private:
    explicit GuiConsole();
    explicit GuiConsole(GuiConsole&);
    GuiConsoleStream mStream;   
};
typedef boost::shared_ptr<GuiConsole> GuiConsolePtr;

class GuiFactory
{
public:
    GuiFactory(GuiController& gui) : mGui(gui) { }
    explicit GuiFactory();
    ~GuiFactory() { }

    // XXX should not be attached by default!!!
    GuiButtonWidgetPtr createTextButton(std::string text, float fontSize, bool attach=false);
    GuiConsolePtr      createConsole(int lines, float fontSize, bool attach=false);
    GuiButtonWidgetPtr createTextLabel(bool attach=false);

private:
    GuiController& mGui;
};
typedef boost::shared_ptr<GuiFactory> GuiFactoryPtr;

class GuiController {
public:
	GuiController();
	void update();
	void draw();

    //  Primitives
    GuiLabelWidgetPtr  createLabel(const GuiLabelData& spec, bool attachWidget=true);
    GuiButtonWidgetPtr createButton(std::vector<GuiWidgetPtr> children, bool attachWidget=false);
    GuiQuadWidgetPtr   createQuad(const GuiQuadData& data, bool attachWidget=true);
    GuiBoxWidgetPtr    createBox(const GuiQuadData& data, const GuiBoxData& boxData, bool attachWidget=true);

    std::vector<GuiWidgetPtr>& widgets();

    void attach(GuiWidgetPtr widget);
    void detach(GuiWidget* ptr);
    void detachAll();

    bool keyDown(ci::app::KeyEvent event);
    void mouseMove(ci::app::MouseEvent event);
    bool mouseDown(ci::app::MouseEvent event);
    bool mouseUp(ci::app::MouseEvent event);
    void mouseDrag(ci::app::MouseEvent event);
    bool mouseWheel(ci::app::MouseEvent event);

    void setShared(boost::shared_ptr<Shared> shared) { mShared = shared; }

protected:
    boost::shared_ptr<Shared> mShared;
    std::list<GuiWidgetPtr> mWidgets;
    boost::shared_ptr<GuiRenderer> mRenderer;
};

}
