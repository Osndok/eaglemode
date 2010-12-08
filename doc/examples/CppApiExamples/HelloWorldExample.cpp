#include <emCore/emGUIFramework.h>
#include <emCore/emPanel.h>


class MyPanel : public emPanel {
public:
	MyPanel(ParentArg parent, const emString & name);
	virtual emString GetTitle();
protected:
	virtual void Paint(const emPainter & painter, emColor canvasColor);
};

MyPanel::MyPanel(ParentArg parent, const emString & name)
	: emPanel(parent,name)
{
}

emString MyPanel::GetTitle()
{
	return "Hello World Example";
}

void MyPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	painter.Clear(emColor::BLACK);
	painter.PaintTextBoxed(0,0,1,GetHeight(),"Hello World!",.01,emColor::RED);
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
