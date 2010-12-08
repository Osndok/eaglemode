#include <emCore/emRecFileModel.h>
#include <emCore/emFilePanel.h>
#include <emCore/emToolkit.h>
#include <emCore/emFpPlugin.h>


//=============================== PlExFileModel ================================

class PlExFileModel : public emRecFileModel, public emStructRec
{
public:

	static emRef<PlExFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	virtual const char * GetFormatName() const;

	class LineRec : public emStructRec {
	public:
		LineRec();
		emDoubleRec X1,Y1,X2,Y2;
		emDoubleRec Thickness;
		emColorRec Color;
	};

	emColorRec CurrentColor;
	emTArrayRec<LineRec> Lines;

protected:

	PlExFileModel(emContext & context, const emString & name);
};


emRef<PlExFileModel> PlExFileModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(PlExFileModel,context,name,common)
}

const char * PlExFileModel::GetFormatName() const
{
	return "PlEx";
}

PlExFileModel::LineRec::LineRec()
	: emStructRec(),
	X1(this,"X1"),
	Y1(this,"Y1"),
	X2(this,"X2"),
	Y2(this,"Y2"),
	Thickness(this,"Thickness"),
	Color(this,"Color")
{
}

PlExFileModel::PlExFileModel(emContext & context, const emString & name)
	: emRecFileModel(context,name), emStructRec(),
	CurrentColor(this,"CurrentColor"),
	Lines(this,"Lines")
{
	PostConstruct(*this);
}


//============================== PlExControlPanel ==============================

class PlExControlPanel : public emTkGroup {
public:
	PlExControlPanel(ParentArg parent, const emString & name,
	                 emRef<PlExFileModel> model);
protected:
	virtual bool Cycle();
private:
	emRef<PlExFileModel> Model;
	emTkColorField * ColorField;
	emTkButton * ClearButton;
};

PlExControlPanel::PlExControlPanel(
	ParentArg parent, const emString & name, emRef<PlExFileModel> model
)
	: emTkGroup(parent,name,"PlEx")
{
	Model=model;
	SetPrefChildTallness(0.2);
	SetPrefChildTallness(0.4,1);
	new emTkLabel(
		new emTkGroup(this,"about","About"),
		"about",
		"This is PlEx - a plugin example.\n"
		"It allows to draw lines with the\n"
		"left mouse button.\n"
	);
	ColorField=new emTkColorField(this,"color","Color");
	ColorField->SetColor(Model->CurrentColor);
	ColorField->SetEditable();
	ClearButton=new emTkButton(
		new emTkTunnel(this,"tunnel","Clear"),
		"clear","Clear"
	);
	AddWakeUpSignal(ColorField->GetColorSignal());
	AddWakeUpSignal(ClearButton->GetClickSignal());
	AddWakeUpSignal(Model->GetChangeSignal());
}

bool PlExControlPanel::Cycle()
{
	if (IsSignaled(ColorField->GetColorSignal())) {
		Model->CurrentColor.Set(ColorField->GetColor());
		Model->Save(true);
	}
	if (IsSignaled(ClearButton->GetClickSignal())) {
		Model->Lines.SetCount(0);
		Model->Save(true);
	}
	if (IsSignaled(Model->GetChangeSignal())) {
		ColorField->SetColor(Model->CurrentColor);
	}
	return emTkGroup::Cycle();
}


//=============================== PlExFilePanel ================================

class PlExFilePanel : public emFilePanel {
public:
	PlExFilePanel(ParentArg parent, const emString & name,
	              emRef<PlExFileModel> model);
protected:
	virtual bool Cycle();
	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);
	virtual bool IsOpaque();
	virtual void Paint(const emPainter & painter, emColor canvasColor);
	virtual emPanel * CreateControlPanel(ParentArg parent,
	                                     const emString & name);
private:
	emRef<PlExFileModel> Model;
	bool Drawing;
	double X1,Y1,X2,Y2;
	double Thickness;
	bool ControlPanelShown;
};

PlExFilePanel::PlExFilePanel(
	ParentArg parent, const emString & name,
	emRef<PlExFileModel> model
)
	: emFilePanel(parent,name,model)
{
	Model=model;
	Drawing=false;
	ControlPanelShown=false;
	AddWakeUpSignal(GetVirFileStateSignal());
	AddWakeUpSignal(Model->GetChangeSignal());
}

bool PlExFilePanel::Cycle()
{
	if (IsSignaled(GetVirFileStateSignal())) {
		if (ControlPanelShown!=IsVFSGood()) {
			ControlPanelShown=IsVFSGood();
			InvalidateControlPanel();
		}
	}
	if (IsSignaled(Model->GetChangeSignal())) {
		InvalidatePainting();
	}
	return emFilePanel::Cycle();
}

void PlExFilePanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	if (IsVFSGood()) {
		if (event.IsKey(EM_KEY_LEFT_BUTTON)) {
			Drawing=true;
			X1=X2=mx;
			Y1=Y2=my;
			Thickness=5.0/(GetViewedWidth()+GetViewedHeight());
			InvalidatePainting();
		}
		if (Drawing && (X2!=mx || Y2!=mx)) {
			X2=mx;
			Y2=my;
			InvalidatePainting();
		}
		if (Drawing && !state.Get(EM_KEY_LEFT_BUTTON)) {
			Drawing=false;
			int i=Model->Lines.GetCount();
			Model->Lines.SetCount(i+1);
			Model->Lines[i].X1=X1;
			Model->Lines[i].Y1=Y1;
			Model->Lines[i].X2=X2;
			Model->Lines[i].Y2=Y2;
			Model->Lines[i].Thickness=Thickness;
			Model->Lines[i].Color=Model->CurrentColor.Get();
			Model->Save(true);
			InvalidatePainting();
		}
	}
	emFilePanel::Input(event,state,mx,my);
}

bool PlExFilePanel::IsOpaque()
{
	if (IsVFSGood()) return true;
	else return emFilePanel::IsOpaque();
}

void PlExFilePanel::Paint(const emPainter & painter, emColor canvasColor)
{
	if (IsVFSGood()) {
		painter.Clear(emColor::WHITE,canvasColor);
		for (int i=0; i<Model->Lines.GetCount(); i++) {
			painter.PaintLine(
				Model->Lines[i].X1,
				Model->Lines[i].Y1,
				Model->Lines[i].X2,
				Model->Lines[i].Y2,
				Model->Lines[i].Thickness,
				emPainter::LC_ROUND,emPainter::LC_ROUND,
				Model->Lines[i].Color
			);
		}
		if (Drawing) {
			painter.PaintLine(
				X1,Y1,X2,Y2,Thickness,
				emPainter::LC_ROUND,emPainter::LC_ROUND,
				Model->CurrentColor
			);
		}
	}
	else {
		emFilePanel::Paint(painter,canvasColor);
	}
}

emPanel * PlExFilePanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	if (IsVFSGood()) {
		return new PlExControlPanel(parent,name,Model);
	}
	else {
		return emFilePanel::CreateControlPanel(parent,name);
	}
}


//============================== PlExFpPluginFunc ==============================

extern "C" {
	emPanel * PlExFpPluginFunc(
		emPanel::ParentArg parent, const emString & name,
		const emString & path, emFpPlugin * plugin,
		emString * errorBuf
	)
	{
		return new PlExFilePanel(
			parent,
			name,
			PlExFileModel::Acquire(parent.GetRootContext(),path)
		);
	}
}
