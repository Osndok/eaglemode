#include <emCore/emGUIFramework.h>
#include <emCore/emPanel.h>


class MyPanel : public emPanel {
public:
	MyPanel(ParentArg parent, const emString & name, emColor bgcolor=0xFFFFFFFF);
protected:
	virtual bool IsOpaque();
	virtual void Paint(const emPainter & painter, emColor canvasColor);
	virtual void AutoExpand();
	virtual void LayoutChildren();
private:
	emColor BackgroundColor;
	emCrossPtr<MyPanel> Child[4];
};

MyPanel::MyPanel(ParentArg parent, const emString & name, emColor bgcolor)
	: emPanel(parent,name)
{
	BackgroundColor=bgcolor;
	EnableAutoExpansion(); // Don't forget this one!!!
}

bool MyPanel::IsOpaque()
{
	return BackgroundColor.IsOpaque();
}

void MyPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	painter.Clear(BackgroundColor,canvasColor);
}

void MyPanel::AutoExpand()
{
	for (int i=0; i<4; i++) {
		Child[i]=new MyPanel(
			this,
			emString::Format("%d",i),
			BackgroundColor.Get()^0xFFFFFF00
		);
	}
}

void MyPanel::LayoutChildren()
{
	for (int i=0; i<4; i++) {
		if (Child[i]) {
			Child[i]->Layout(
				0.1+(i&1)*0.5,
				(0.1+(i&2)*0.25)*GetHeight(),
				0.3,
				0.3*GetHeight(),
				BackgroundColor
			);
		}
	}
}


MAIN_OR_WINMAIN_HERE

static int wrapped_main(int argc, char * argv[])
{
	emInitLocale();

	emGUIFramework framework;
	framework.EnableAutoTermination();

	emWindow * window=new emWindow(framework.GetRootContext());
	window->SetWindowFlags(emWindow::WF_AUTO_DELETE);
	window->SetViewFlags(emView::VF_ROOT_SAME_TALLNESS);

	new MyPanel(window,"root");

	return framework.Run();
}
