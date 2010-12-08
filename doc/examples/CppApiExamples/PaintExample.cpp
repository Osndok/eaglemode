#include <emCore/emGUIFramework.h>
#include <emCore/emPanel.h>
#include <emCore/emRes.h>


class MyPanel : public emPanel {
public:
	MyPanel(ParentArg parent, const emString & name);
protected:
	virtual bool IsOpaque();
	virtual void Paint(const emPainter & painter, emColor canvasColor);
private:
	emImage EagleIcon;
};

MyPanel::MyPanel(ParentArg parent, const emString & name)
	: emPanel(parent,name)
{
	EagleIcon=emGetInsResImage(GetRootContext(),"icons","eaglemode.tga");
}

bool MyPanel::IsOpaque()
{
	return true;
}

void MyPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	painter.Clear(emColor::WHITE,canvasColor);
	canvasColor=emColor::WHITE;

	painter.PaintImage(0.05,0.05,0.3,0.3,EagleIcon,255,canvasColor);

	painter.PaintRect(0.4,0.1,0.2,0.2,emColor::GREEN,canvasColor);

	painter.PaintRectOutline(0.69,0.09,0.22,0.22,0.01,0x0080C0FF,canvasColor);
	painter.PaintTextBoxed(
		0.7,0.1,0.2,0.2,
		"Centered text\nin\nthe bottom-right\nof a box",
		0.017,0x0080C0FF,canvasColor,
		EM_ALIGN_BOTTOM_RIGHT,EM_ALIGN_CENTER
	);

	static const double poly1[]={
		0.1, 0.4,
		0.05, 0.7,
		0.3, 0.5
	};
	painter.PaintPolygon(poly1,3,emColor(255,128,0),canvasColor);

	painter.PaintLine(
		0.3,0.4,0.5,0.7,0.05,
		emPainter::LC_ROUND,emPainter::LC_ROUND,
		emColor(255,0,0,128),canvasColor
	);

	painter.PaintEllipse(0.5,0.4,0.3,0.3,0x33CC88FF,canvasColor);
	painter.PaintEllipse(0.6,0.4,0.3,0.3,0xFF55AAFF);
	painter.PaintEllipse(0.55,0.35,0.3,0.3,0xFFFF3388);
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
