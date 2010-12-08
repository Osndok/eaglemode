#include <emCore/emGUIFramework.h>
#include <emCore/emPanel.h>


class MyPanel : public emPanel {
public:
	MyPanel(ParentArg parent, const emString & name);
protected:
	virtual bool Cycle();
	virtual void Paint(const emPainter & painter, emColor canvasColor);
private:
	unsigned Counter;
};

MyPanel::MyPanel(ParentArg parent, const emString & name)
	: emPanel(parent,name)
{
	Counter=0;
	WakeUp();
}

bool MyPanel::Cycle()
{
	emPanel::Cycle();
	Counter++;
	InvalidatePainting();
	return true;
}

void MyPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	emString str=emString::Format("%u",Counter);
	painter.PaintTextBoxed(0,0,1,GetHeight(),str,.41,0xFFFF80FF);
}


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
