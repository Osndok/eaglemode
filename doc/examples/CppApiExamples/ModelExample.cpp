#include <emCore/emGUIFramework.h>
#include <emCore/emToolkit.h>


//================================== MyModel ===================================

class MyModel : public emModel {
public:
	static emRef<MyModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);
	int GetData() const;
	void SetData(int data);
	const emSignal & GetChangeSignal() const;
protected:
	MyModel(emContext & context, const emString & name);
private:
	int Data;
	emSignal ChangeSignal;
};

emRef<MyModel> MyModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(MyModel,context,name,common)
}

int MyModel::GetData() const
{
	return Data;
}

void MyModel::SetData(int data)
{
	if (Data!=data) {
		Data=data;
		Signal(ChangeSignal);
	}
}

const emSignal & MyModel::GetChangeSignal() const
{
	return ChangeSignal;
}

MyModel::MyModel(emContext & context, const emString & name)
	: emModel(context,name)
{
	Data=0;
}


//================================== MyPanel ===================================

class MyPanel : public emTkGroup {
public:
	MyPanel(ParentArg parent, const emString & name);
protected:
	virtual bool Cycle();
private:
	emRef<MyModel> Model;
	emTkButton * BtNewWin;
	emTkScalarField * SField;
};

MyPanel::MyPanel(ParentArg parent, const emString & name)
	: emTkGroup(parent,name,"Model Example")
{
	Model=MyModel::Acquire(GetRootContext(),"test");
	AddWakeUpSignal(Model->GetChangeSignal());

	BtNewWin=new emTkButton(this,"bnw","New Window");
	AddWakeUpSignal(BtNewWin->GetClickSignal());

	SField=new emTkScalarField(this,"sf","Data");
	SField->SetEditable();
	SField->SetValue(Model->GetData());
	AddWakeUpSignal(SField->GetValueSignal());

	SetSpace(0.2,0.8,0.2,0.8);
}

bool MyPanel::Cycle()
{
	if (IsSignaled(BtNewWin->GetClickSignal())) {
		emWindow * window=new emWindow(GetRootContext());
		window->SetWindowFlags(emWindow::WF_AUTO_DELETE);
		window->SetViewFlags(emView::VF_ROOT_SAME_TALLNESS);
		new MyPanel(window,"root");
	}
	if (IsSignaled(Model->GetChangeSignal())) {
		SField->SetValue(Model->GetData());
	}
	if (IsSignaled(SField->GetValueSignal())) {
		Model->SetData((int)SField->GetValue());
	}
	return emTkGroup::Cycle();
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
	window->SetViewFlags(emView::VF_ROOT_SAME_TALLNESS);

	new MyPanel(window,"root");

	return framework.Run();
}
