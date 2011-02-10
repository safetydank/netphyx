#ifndef TITLESTATE_H
#define TITLESTATE_H

#include "StateManager.h"
#include "GuiController.h"

namespace netphy
{

class TitleState : public State
{
public:
    TitleState(StateManager& manager, Shared& shared);
    ~TitleState();

    void enter();
    void leave();
    void update();
    void draw();

    virtual void keyDown(ci::app::KeyEvent event);
    virtual void mouseWheel(ci::app::MouseEvent event);

private:
    GuiLabelWidgetPtr  mTitle;
    GuiButtonWidgetPtr mPlayButton;
    GuiButtonWidgetPtr mEditorButton;
    GuiButtonWidgetPtr mServerButton;
    GuiButtonWidgetPtr mClientButton;
};

}

#endif
