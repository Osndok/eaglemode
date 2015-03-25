#include <emCore/emGUIFramework.h>
#include <emCore/emToolkit.h>


class MyPanel : public emRasterGroup {
public:
	MyPanel(ParentArg parent, const emString & name);
protected:
	virtual bool Cycle();
private:
	static void ScalarField2_TextOfValue(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);
	emTextField * Message;
	emButton * Button;
	emCheckButton * CheckButton;
	emRadioButton::RasterGroup * RadioGroup;
	emTextField * TextField;
	emScalarField * ScalarField1;
	emScalarField * ScalarField2;
};

MyPanel::MyPanel(ParentArg parent, const emString & name)
	: emRasterGroup(parent,name,"Toolkit Example")
{
	Message=new emTextField(this,"msg","Message");
	Message->SetDescription("This text field shows messages about your input.");

	Button=new emButton(this,"b","Button");
	AddWakeUpSignal(Button->GetClickSignal());

	CheckButton=new emCheckButton(this,"cb","Check Button");
	AddWakeUpSignal(CheckButton->GetCheckSignal());

	RadioGroup=new emRadioButton::RasterGroup(this,"rg","Radio Group");
	RadioGroup->SetBorderScaling(4.0);
	new emRadioBox(RadioGroup,"rb1","Radio Box 1");
	new emRadioBox(RadioGroup,"rb2","Radio Box 2");
	new emRadioBox(RadioGroup,"rb3","Radio Box 3");
	AddWakeUpSignal(RadioGroup->GetCheckSignal());

	TextField=new emTextField(this,"tf","Text Field");
	TextField->SetEditable();
	TextField->SetMultiLineMode();
	AddWakeUpSignal(TextField->GetTextSignal());

	ScalarField1=new emScalarField(this,"sf1","Scalar Field 1");
	ScalarField1->SetEditable();
	ScalarField1->SetMinMaxValues(0,100);
	ScalarField1->SetScaleMarkIntervals(50,10,5,1,0);
	AddWakeUpSignal(ScalarField1->GetValueSignal());

	ScalarField2=new emScalarField(this,"sf2","Scalar Field 2");
	ScalarField2->SetEditable();
	ScalarField2->SetMinMaxValues(-1,1);
	ScalarField2->SetTextOfValueFunc(ScalarField2_TextOfValue);
	AddWakeUpSignal(ScalarField2->GetValueSignal());
}

bool MyPanel::Cycle()
{
	if (IsSignaled(Button->GetClickSignal())) {
		Message->SetText("Button clicked");
	}
	if (IsSignaled(CheckButton->GetCheckSignal())) {
		Message->SetText(emString::Format(
			"Check Button switched %s",
			CheckButton->IsChecked() ? "on" : "off"
		));
	}
	if (IsSignaled(RadioGroup->GetCheckSignal())) {
		Message->SetText(emString::Format(
			"Radio Box %d set",
			RadioGroup->GetCheckIndex() + 1
		));
	}
	if (IsSignaled(TextField->GetTextSignal())) {
		Message->SetText(emString::Format(
			"Text Field changed to \"%s\"",
			TextField->GetText().Get()
		));
	}
	if (IsSignaled(ScalarField1->GetValueSignal())) {
		Message->SetText(emString::Format(
			"Scalar Field 1 changed to %d",
			(int)ScalarField1->GetValue()
		));
	}
	if (IsSignaled(ScalarField2->GetValueSignal())) {
		Message->SetText(emString::Format(
			"Scalar Field 2 changed to %d",
			(int)ScalarField2->GetValue()
		));
	}
	return emRasterGroup::Cycle();
}

void MyPanel::ScalarField2_TextOfValue(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval, void * context
)
{
	if (value<0) strcpy(buf,"left");
	else if (value==0) strcpy(buf,"middle");
	else strcpy(buf,"right");
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
