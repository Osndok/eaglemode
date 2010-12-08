#include <emCore/emGUIFramework.h>
#include <emCore/emPanel.h>


//================================== MyButton ==================================

class MyButton : public emPanel {
public:
	MyButton(ParentArg parent, const emString & name);
	const emSignal & GetClickSignal() const;
protected:
	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);
	virtual void Paint(const emPainter & painter, emColor canvasColor);
private:
	emSignal ClickSignal;
};

MyButton::MyButton(ParentArg parent, const emString & name)
	: emPanel(parent,name)
{
}

const emSignal & MyButton::GetClickSignal() const
{
	return ClickSignal;
}

void MyButton::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	if (event.IsLeftButton()) Signal(ClickSignal);
	emPanel::Input(event,state,mx,my);
}

void MyButton::Paint(const emPainter & painter, emColor canvasColor)
{
	painter.Clear(0xC0C0C0FF,canvasColor);
	painter.PaintTextBoxed(0,0,1,GetHeight(),"Click Me",1,emColor::GREEN);
}


//================================== MyPanel ===================================

class MyPanel : public emPanel {
public:
	MyPanel(ParentArg parent, const emString & name);
protected:
	virtual bool Cycle();
	virtual void Paint(const emPainter & painter, emColor canvasColor);
private:
	unsigned Counter1, Counter2;
	MyButton * Button;
	emTimer Timer;
};

MyPanel::MyPanel(ParentArg parent, const emString & name)
	: emPanel(parent,name),
	Timer(GetScheduler())
{
	Counter1=0;
	Counter2=0;
	Button=new MyButton(this,"button");
	Button->Layout(0.1,0.1,0.8,0.1);
	Timer.Start(1000,true);
	AddWakeUpSignal(Button->GetClickSignal());
	AddWakeUpSignal(Timer.GetSignal());
}

bool MyPanel::Cycle()
{
	if (IsSignaled(Button->GetClickSignal())) {
		Counter1++;
		InvalidatePainting();
	}
	if (IsSignaled(Timer.GetSignal())) {
		Counter2++;
		InvalidatePainting();
	}
	return emPanel::Cycle();
}

void MyPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	emString str=emString::Format(
		"Button Signals: %u\nTimer Signals: %u",Counter1,Counter2
	);
	painter.PaintTextBoxed(0,0.3,1,GetHeight()-0.3,str,.1,0xFFFF80FF);
}


//==================================== main ====================================

MAIN_OR_WINMAIN_HERE

static int wrapped_main(int argc, char * argv[])
{
	emInitLocale();

	emGUIFramework framework;
	framework.EnableAutoTermination();

	emWindow * window=new emWindow(framework.GetRootContext());
	window->SetWindowFlags(emWindow::WF_AUTO_DELETE);

	emPanel * panel=new MyPanel(window,"root");
	panel->Layout(0.0,0.0,4.0,3.0);

	return framework.Run();
}
