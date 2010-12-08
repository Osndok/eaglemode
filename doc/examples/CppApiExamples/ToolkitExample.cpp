#include <emCore/emGUIFramework.h>
#include <emCore/emToolkit.h>


class MyPanel : public emTkGroup {
public:
	MyPanel(ParentArg parent, const emString & name);
protected:
	virtual bool Cycle();
private:
	static void ScalarField2_TextOfValue(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);
	emTkTextField * Message;
	emTkButton * Button;
	emTkCheckButton * CheckButton;
	emTkRadioButton::Group * RadioGroup;
	emTkTextField * TextField;
	emTkScalarField * ScalarField1;
	emTkScalarField * ScalarField2;
};

MyPanel::MyPanel(ParentArg parent, const emString & name)
	: emTkGroup(parent,name,"Toolkit Example")
{
	Message=new emTkTextField(this,"msg","Message");
	Message->SetDescription("This text field shows messages about your input.");

	Button=new emTkButton(this,"b","Button");
	AddWakeUpSignal(Button->GetClickSignal());

	CheckButton=new emTkCheckButton(this,"cb","Check Button");
	AddWakeUpSignal(CheckButton->GetCheckSignal());

	RadioGroup=new emTkRadioButton::Group(this,"rg","Radio Group");
	RadioGroup->SetBorderScaling(4.0);
	new emTkRadioBox(RadioGroup,"rb1","Radio Box 1");
	new emTkRadioBox(RadioGroup,"rb2","Radio Box 2");
	new emTkRadioBox(RadioGroup,"rb3","Radio Box 3");
	AddWakeUpSignal(RadioGroup->GetCheckSignal());

	TextField=new emTkTextField(this,"tf","Text Field");
	TextField->SetEditable();
	TextField->SetMultiLineMode();
	AddWakeUpSignal(TextField->GetTextSignal());

	ScalarField1=new emTkScalarField(this,"sf1","Scalar Field 1");
	ScalarField1->SetEditable();
	ScalarField1->SetMinMaxValues(0,100);
	ScalarField1->SetScaleMarkIntervals(50,10,5,1,0);
	AddWakeUpSignal(ScalarField1->GetValueSignal());

	ScalarField2=new emTkScalarField(this,"sf2","Scalar Field 2");
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
	return emTkGroup::Cycle();
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
