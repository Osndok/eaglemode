//------------------------------------------------------------------------------
// emDialog.cpp
//
// Copyright (C) 2005-2011,2014-2016 Oliver Hamann.
//
// Homepage: http://eaglemode.sourceforge.net/
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3 as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
// more details.
//
// You should have received a copy of the GNU General Public License version 3
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <emCore/emDialog.h>
#include <emCore/emLabel.h>


emDialog::emDialog(
	emContext & parentContext, ViewFlags viewFlags, WindowFlags windowFlags,
	const emString & wmResName
)
	: emWindow(parentContext,viewFlags,windowFlags,wmResName),
	PrivateEngine(*this)
{
	Result=NEGATIVE;
	ButtonNum=0;
	CustomRes=CUSTOM1;
	FinishState=0;
	ADEnabled=false;
	PrivateEngine.SetEnginePriority(HIGH_PRIORITY);
	PrivateEngine.AddWakeUpSignal(GetCloseSignal());
	new DlgPanel(this,"root");
}


emDialog::~emDialog()
{
	if (GetRootPanel()) delete GetRootPanel();
}


void emDialog::SetRootTitle(const emString & title)
{
	((DlgPanel*)GetRootPanel())->SetTitle(title);
}


void emDialog::AddPositiveButton(
	const emString & caption, const emString & description, const emImage & icon
)
{
	DlgButton * bt;

	bt=new DlgButton(
		((DlgPanel*)GetRootPanel())->ButtonsPanel,
		emString::Format("%d",ButtonNum),
		caption,description,icon,
		POSITIVE
	);
	bt->Activate();
	ButtonNum++;
}


void emDialog::AddNegativeButton(
	const emString & caption, const emString & description, const emImage & icon
)
{
	new DlgButton(
		((DlgPanel*)GetRootPanel())->ButtonsPanel,
		emString::Format("%d",ButtonNum),
		caption,description,icon,
		NEGATIVE
	);
	ButtonNum++;
}


void emDialog::AddCustomButton(
	const emString & caption, const emString & description, const emImage & icon
)
{
	new DlgButton(
		((DlgPanel*)GetRootPanel())->ButtonsPanel,
		emString::Format("%d",ButtonNum),
		caption,description,icon,
		CustomRes
	);
	ButtonNum++;
	CustomRes++;
}


void emDialog::AddOKButton()
{
	AddPositiveButton("OK");
}


void emDialog::AddCancelButton()
{
	AddNegativeButton("Cancel");
}


void emDialog::AddOKCancelButtons()
{
	AddOKButton();
	AddCancelButton();
}


emButton * emDialog::GetButton(int index) const
{
	return dynamic_cast<emButton*>(
		((DlgPanel*)GetRootPanel())->ButtonsPanel->GetChild(
			emString::Format("%d",index)
		)
	);
}


emButton * emDialog::GetButtonForResult(int result) const
{
	emPanel * buttonsPanel, * p;
	DlgButton * db;

	buttonsPanel=((DlgPanel*)GetRootPanel())->ButtonsPanel;
	for (p=buttonsPanel->GetFirstChild(); p; p=p->GetNext()) {
		db=dynamic_cast<DlgButton*>(p);
		if (db && db->GetResult() == result) {
			return db;
		}
	}
	return NULL;
}


bool emDialog::Finish(int result)
{
	if (!CheckFinish(result)) return false;
	Result=result;
	FinishState=1;
	PrivateEngine.WakeUp();
	return true;
}


void emDialog::EnableAutoDeletion(bool autoDelete)
{
	ADEnabled=autoDelete;
}


void emDialog::ShowMessage(
	emContext & parentContext, const emString & title, const emString & message,
	const emString & description, const emImage & icon
)
{
	emDialog * d;

	d=new emDialog(parentContext);
	d->SetRootTitle(title);
	d->AddOKButton();
	new emLabel(
		d->GetContentPanel(),
		"l",
		message,
		description,
		icon
	);
	d->EnableAutoDeletion();
}


bool emDialog::CheckFinish(int result)
{
	return true;
}


void emDialog::Finished(int result)
{
}


bool emDialog::PrivateCycle()
{
	if (PrivateEngine.IsSignaled(GetCloseSignal())) {
		Finish(NEGATIVE);
	}

	if (FinishState<=0) {
		return false;
	}
	else if (FinishState==1) {
		FinishState=2;
		Signal(FinishSignal);
		Finished(Result); // may destruct this dialog
		return true;
	}
	else if (!ADEnabled) {
		FinishState=0;
		return false;
	}
	else if (FinishState<3) {
		FinishState++;
		return true;
	}
	else {
		delete this;
		return false;
	}
}


emDialog::DlgButton::DlgButton(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon, int result
)
	: emButton(parent,name,caption,description,icon)
{
	Result=result;
}


void emDialog::DlgButton::Clicked()
{
	((emDialog*)GetWindow())->Finish(Result);
}


emDialog::DlgPanel::DlgPanel(ParentArg parent, const emString & name)
	: emBorder(parent,name)
{
	ContentPanel=new emLinearLayout(this,"content");
	ContentPanel->SetInnerBorderType(IBT_CUSTOM_RECT);
	ButtonsPanel=new emLinearLayout(this,"buttons");
	ButtonsPanel->SetChildTallness(0.3);
	ButtonsPanel->SetInnerSpace(0.1,0.1);
	if ((GetView().GetViewFlags())&emView::VF_POPUP_ZOOM) {
		SetOuterBorderType(OBT_POPUP_ROOT);
	}
	else {
		SetOuterBorderType(OBT_FILLED);
	}
}


emDialog::DlgPanel::~DlgPanel()
{
}


void emDialog::DlgPanel::SetTitle(const emString & title)
{
	if (Title!=title) {
		Title=title;
		InvalidateTitle();
	}
}


emString emDialog::DlgPanel::GetTitle() const
{
	return Title;
}


void emDialog::DlgPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	emBorder::Input(event,state,mx,my);

	switch (event.GetKey()) {
	case EM_KEY_ENTER:
		if (state.IsNoMod()) {
			((emDialog*)GetWindow())->Finish(POSITIVE);
			event.Eat();
		}
		break;
	case EM_KEY_ESCAPE:
		if (state.IsNoMod()) {
			((emDialog*)GetWindow())->Finish(NEGATIVE);
			event.Eat();
		}
		break;
	default:
		break;
	}
}


void emDialog::DlgPanel::LayoutChildren()
{
	double x,y,w,h,sp,bh;
	emColor cc;

	emBorder::LayoutChildren();

	GetContentRectUnobscured(&x,&y,&w,&h,&cc);
	bh=emMin(w*0.08,h*0.3);
	sp=bh*0.25;
	x+=sp;
	y+=sp;
	w-=2*sp;
	h-=2*sp;
	ContentPanel->Layout(
		x,y,w,h-sp-bh,cc
	);
	ButtonsPanel->Layout(
		x,y+h-bh,w,bh,cc
	);
}


emDialog::PrivateEngineClass::PrivateEngineClass(emDialog & dlg)
	: emEngine(dlg.GetScheduler()),
	Dlg(dlg)
{
}


bool emDialog::PrivateEngineClass::Cycle()
{
	return Dlg.PrivateCycle();
}
