//------------------------------------------------------------------------------
// emToolkit.cpp
//
// Copyright (C) 2005-2011 Oliver Hamann.
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

#include <emCore/emToolkit.h>
#include <emCore/emInstallInfo.h>
#include <emCore/emRes.h>


//==============================================================================
//================================== emTkLook ==================================
//==============================================================================

#ifdef EM_NO_DATA_EXPORT
emTkLook::emTkLook()
{
	Data=&DefaultData;
}
#endif


bool emTkLook::operator == (const emTkLook & look) const
{
	return
		Data==look.Data || (
			Data->BgColor==look.Data->BgColor &&
			Data->FgColor==look.Data->FgColor &&
			Data->ButtonBgColor==look.Data->ButtonBgColor &&
			Data->ButtonFgColor==look.Data->ButtonFgColor &&
			Data->InputBgColor==look.Data->InputBgColor &&
			Data->InputFgColor==look.Data->InputFgColor &&
			Data->InputHlColor==look.Data->InputHlColor &&
			Data->OutputBgColor==look.Data->OutputBgColor &&
			Data->OutputFgColor==look.Data->OutputFgColor &&
			Data->OutputHlColor==look.Data->OutputHlColor
		)
	;
}


void emTkLook::Apply(emPanel * panel, bool recursively) const
{
	emTkBorder * border;

	border=dynamic_cast<emTkBorder*>(panel);
	if (border) {
		// Remember: border->SetLook could be overloaded to stop the
		// recursion.
		border->SetLook(*this,recursively);
	}
	else if (recursively) {
		for (panel=panel->GetFirstChild(); panel; panel=panel->GetNext()) {
			Apply(panel,true);
		}
	}
}


void emTkLook::SetBgColor(emColor bgColor)
{
	if (Data->BgColor!=bgColor) {
		MakeWritable();
		Data->BgColor=bgColor;
	}
}


void emTkLook::SetFgColor(emColor fgColor)
{
	if (Data->FgColor!=fgColor) {
		MakeWritable();
		Data->FgColor=fgColor;
	}
}


void emTkLook::SetButtonBgColor(emColor buttonBgColor)
{
	if (Data->ButtonBgColor!=buttonBgColor) {
		MakeWritable();
		Data->ButtonBgColor=buttonBgColor;
	}
}


void emTkLook::SetButtonFgColor(emColor buttonFgColor)
{
	if (Data->ButtonFgColor!=buttonFgColor) {
		MakeWritable();
		Data->ButtonFgColor=buttonFgColor;
	}
}


void emTkLook::SetInputBgColor(emColor inputBgColor)
{
	if (Data->InputBgColor!=inputBgColor) {
		MakeWritable();
		Data->InputBgColor=inputBgColor;
	}
}


void emTkLook::SetInputFgColor(emColor inputFgColor)
{
	if (Data->InputFgColor!=inputFgColor) {
		MakeWritable();
		Data->InputFgColor=inputFgColor;
	}
}


void emTkLook::SetInputHlColor(emColor inputHlColor)
{
	if (Data->InputHlColor!=inputHlColor) {
		MakeWritable();
		Data->InputHlColor=inputHlColor;
	}
}


void emTkLook::SetOutputBgColor(emColor outputBgColor)
{
	if (Data->OutputBgColor!=outputBgColor) {
		MakeWritable();
		Data->OutputBgColor=outputBgColor;
	}
}


void emTkLook::SetOutputFgColor(emColor outputFgColor)
{
	if (Data->OutputFgColor!=outputFgColor) {
		MakeWritable();
		Data->OutputFgColor=outputFgColor;
	}
}


void emTkLook::SetOutputHlColor(emColor outputHlColor)
{
	if (Data->OutputHlColor!=outputHlColor) {
		MakeWritable();
		Data->OutputHlColor=outputHlColor;
	}
}


unsigned int emTkLook::GetDataRefCount() const
{
	return Data==&DefaultData ? UINT_MAX/2 : Data->RefCount;
}


void emTkLook::DeleteData()
{
	DefaultData.RefCount=UINT_MAX/2;
	if (Data!=&DefaultData) delete Data;
}


void emTkLook::MakeWritable()
{
	SharedData * d;

	if (Data->RefCount>1 || Data==&DefaultData) {
		d=new SharedData(*Data);
		d->RefCount=1;
		Data->RefCount--;
		Data=d;
	}
}


emTkLook::SharedData::SharedData()
	: RefCount(UINT_MAX/2),
	BgColor      (0x2A3F6BFF),
	FgColor      (0xD0D7D2FF),
	ButtonBgColor(0x2E332EFF),
	ButtonFgColor(0xCBCBD6FF),
	InputBgColor (0xFFFFFFFF),
	InputFgColor (0x000000FF),
	InputHlColor (0x0033BBFF),
	OutputBgColor(0xBBBBBBFF),
	OutputFgColor(0x000000FF),
	OutputHlColor(0x0033BBFF)
{
}


emTkLook::SharedData::SharedData(const SharedData & sd)
	: RefCount(sd.RefCount),
	BgColor(sd.BgColor),
	FgColor(sd.FgColor),
	ButtonBgColor(sd.ButtonBgColor),
	ButtonFgColor(sd.ButtonFgColor),
	InputBgColor(sd.InputBgColor),
	InputFgColor(sd.InputFgColor),
	InputHlColor(sd.InputHlColor),
	OutputBgColor(sd.OutputBgColor),
	OutputFgColor(sd.OutputFgColor),
	OutputHlColor(sd.OutputHlColor)
{
}


emTkLook::SharedData emTkLook::DefaultData;


//==============================================================================
//================================= emTkBorder =================================
//==============================================================================

emTkBorder::emTkBorder(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emPanel(parent,name),
	Caption(caption),
	Description(description),
	Icon(icon)
{
	emTkBorder * tkp;
	emPanel * p;
	TkResources * r;
	emRootContext * rc;
	emString resDir;

	rc=&GetRootContext();
	TkResVarModel=emVarModel<TkResources>::Acquire(*rc,"");
	r=&TkResVarModel->Var;
	if (r->ImgButton.IsEmpty()) {
		resDir=emGetInstallPath(EM_IDT_RES,"emCore","toolkit");
		r->ImgButton=emGetResImage(*rc,emGetChildPath(resDir,"Button.tga"));
		r->ImgButtonBorder=emGetResImage(*rc,emGetChildPath(resDir,"ButtonBorder.tga"));
		r->ImgButtonChecked=emGetResImage(*rc,emGetChildPath(resDir,"ButtonChecked.tga"));
		r->ImgButtonPressed=emGetResImage(*rc,emGetChildPath(resDir,"ButtonPressed.tga"));
		r->ImgCheckBox=emGetResImage(*rc,emGetChildPath(resDir,"CheckBox.tga"));
		r->ImgCheckBoxPressed=emGetResImage(*rc,emGetChildPath(resDir,"CheckBoxPressed.tga"));
		r->ImgCustomRectBorder=emGetResImage(*rc,emGetChildPath(resDir,"CustomRectBorder.tga"));
		r->ImgGroupBorder=emGetResImage(*rc,emGetChildPath(resDir,"GroupBorder.tga"));
		r->ImgGroupInnerBorder=emGetResImage(*rc,emGetChildPath(resDir,"GroupInnerBorder.tga"));
		r->ImgIOField=emGetResImage(*rc,emGetChildPath(resDir,"IOField.tga"));
		r->ImgPopupBorder=emGetResImage(*rc,emGetChildPath(resDir,"PopupBorder.tga"));
		r->ImgRadioBox=emGetResImage(*rc,emGetChildPath(resDir,"RadioBox.tga"));
		r->ImgRadioBoxPressed=emGetResImage(*rc,emGetChildPath(resDir,"RadioBoxPressed.tga"));
		r->ImgSplitter=emGetResImage(*rc,emGetChildPath(resDir,"Splitter.tga"));
		r->ImgSplitterPressed=emGetResImage(*rc,emGetChildPath(resDir,"SplitterPressed.tga"));
		r->ImgTunnel=emGetResImage(*rc,emGetChildPath(resDir,"Tunnel.tga"));
	}

	Aux=NULL;

	for (p=GetParent(); p; p=p->GetParent()) {
		tkp=dynamic_cast<emTkBorder*>(p);
		if (tkp) {
			Look=tkp->Look;
			break;
		}
	}

	BorderScaling=1.0;
	LabelAlignment=EM_ALIGN_LEFT;
	CaptionAlignment=EM_ALIGN_LEFT;
	DescriptionAlignment=EM_ALIGN_LEFT;
	OuterBorder=OBT_NONE;
	InnerBorder=IBT_NONE;
	LabelInBorder=true;
}


emTkBorder::~emTkBorder()
{
	if (Aux) delete Aux;
}


void emTkBorder::SetCaption(const emString & caption)
{
	if (Caption!=caption) {
		Caption=caption;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkBorder::SetDescription(const emString & description)
{
	if (Description!=description) {
		Description=description;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkBorder::SetIcon(const emImage & icon)
{
	if (Icon!=icon) {
		Icon=icon;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkBorder::SetLabel(
	const emString & caption, const emString & description,
	const emImage & icon
)
{
	SetCaption(caption);
	SetDescription(description);
	SetIcon(icon);
}


void emTkBorder::SetLabelAlignment(emAlignment labelAlignment)
{
	if (LabelAlignment!=labelAlignment) {
		LabelAlignment=labelAlignment;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkBorder::SetCaptionAlignment(emAlignment captionAlignment)
{
	if (CaptionAlignment!=captionAlignment) {
		CaptionAlignment=captionAlignment;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkBorder::SetDescriptionAlignment(emAlignment descriptionAlignment)
{
	if (DescriptionAlignment!=descriptionAlignment) {
		DescriptionAlignment=descriptionAlignment;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkBorder::SetOuterBorderType(OuterBorderType obt)
{
	if (OuterBorder!=(emByte)obt) {
		OuterBorder=(emByte)obt;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkBorder::SetInnerBorderType(InnerBorderType ibt)
{
	if (InnerBorder!=(emByte)ibt) {
		InnerBorder=(emByte)ibt;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkBorder::SetBorderType(OuterBorderType obt, InnerBorderType ibt)
{
	if (OuterBorder!=(emByte)obt || InnerBorder!=(emByte)ibt) {
		OuterBorder=(emByte)obt;
		InnerBorder=(emByte)ibt;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkBorder::SetBorderScaling(double borderScaling)
{
	if (borderScaling<1E-10) borderScaling=1E-10;
	if (BorderScaling!=borderScaling) {
		BorderScaling=borderScaling;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkBorder::SetLook(const emTkLook & look, bool recursively)
{
	emPanel * p;

	if (Look!=look) {
		Look=look;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
	if (recursively) {
		for (p=GetFirstChild(); p; p=p->GetNext()) {
			look.Apply(p,true);
		}
	}
}


void emTkBorder::HaveAux(const emString & panelName, double tallness)
{
	if (!Aux) {
		Aux=new AuxData;
		Aux->PanelName=panelName;
		Aux->Tallness=tallness;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
	else {
		if (Aux->PanelName!=panelName) {
			Aux->PanelName=panelName;
			Aux->PanelPointerCache=NULL;
			InvalidateChildrenLayout();
		}
		if (Aux->Tallness!=tallness) {
			Aux->Tallness=tallness;
			InvalidatePainting();
			InvalidateChildrenLayout();
		}
	}
}


void emTkBorder::RemoveAux()
{
	if (Aux) {
		delete Aux;
		Aux=NULL;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


const emString & emTkBorder::GetAuxPanelName() const
{
	static const emString emptyString;
		// Okay this is thread-safe as long as the string is empty.

	if (!Aux) return emptyString;
	return Aux->PanelName;
}


double emTkBorder::GetAuxTallness() const
{
	if (!Aux) return 1.0;
	return Aux->Tallness;
}


emPanel * emTkBorder::GetAuxPanel()
{
	emPanel * p;

	if (!Aux) return NULL;
	p=Aux->PanelPointerCache;
	if (!p) {
		p=GetChild(Aux->PanelName);
		if (p) Aux->PanelPointerCache=p;
	}
	return p;
}


void emTkBorder::GetAuxRect(
	double * pX, double * pY, double * pW, double * pH,
	emColor * pCanvasColor
)
{
	if (!Aux) {
		if (pX) *pX=0.0;
		if (pY) *pY=0.0;
		if (pW) *pW=1E-100;
		if (pH) *pH=1E-100;
		if (pCanvasColor) *pCanvasColor=0;
		return;
	}
	DoBorder(
		BORDER_FUNC_AUX_RECT,NULL,GetCanvasColor(),
		pX,pY,pW,pH,NULL,pCanvasColor
	);
}


void emTkBorder::GetContentRect(
	double * pX, double * pY, double * pW, double * pH,
	emColor * pCanvasColor
)
{
	DoBorder(
		BORDER_FUNC_CONTENT_RECT,NULL,GetCanvasColor(),
		pX,pY,pW,pH,NULL,pCanvasColor
	);
}


void emTkBorder::GetContentRoundRect(
	double * pX, double * pY, double * pW, double * pH, double * pR,
	emColor * pCanvasColor
)
{
	DoBorder(
		BORDER_FUNC_CONTENT_ROUND_RECT,NULL,GetCanvasColor(),
		pX,pY,pW,pH,pR,pCanvasColor
	);
}


void emTkBorder::Notice(NoticeFlags flags)
{
	if ((flags&(NF_ENABLE_CHANGED|NF_ACTIVE_CHANGED|NF_FOCUS_CHANGED))!=0) {
		InvalidatePainting();
	}
	emPanel::Notice(flags);
}


bool emTkBorder::IsOpaque()
{
	switch (OuterBorder) {
	case OBT_FILLED:
	case OBT_MARGIN_FILLED:
	case OBT_POPUP_ROOT:
		return Look.GetBgColor().IsOpaque();
	default:
		return false;
	}
}


void emTkBorder::Paint(const emPainter & painter, emColor canvasColor)
{
	DoBorder(BORDER_FUNC_PAINT,&painter,canvasColor,NULL,NULL,NULL,NULL,NULL,NULL);
}


void emTkBorder::LayoutChildren()
{
	emColor cc;
	double x,y,w,h;
	emPanel * p;

	if (!Aux) return;
	p=Aux->PanelPointerCache;
	if (!p) {
		p=GetChild(Aux->PanelName);
		if (!p) return;
		Aux->PanelPointerCache=p;
	}
	GetAuxRect(&x,&y,&w,&h,&cc);
	p->Layout(x,y,w,h,cc);
}


bool emTkBorder::HasHowTo()
{
	return false;
}


emString emTkBorder::GetHowTo()
{
	emString h;

	h=HowToPreface;
	if (!IsEnabled()) h+=HowToDisabled;
	if (IsFocusable()) h+=HowToFocus;
	return h;
}


void emTkBorder::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
)
{
}


bool emTkBorder::HasLabel()
{
	return !Caption.IsEmpty() || !Description.IsEmpty() || !Icon.IsEmpty();
}


double emTkBorder::GetBestLabelTallness()
{
	double bestTallness;

	DoLabel(
		LABEL_FUNC_GET_BEST_TALLNESS,NULL,0.0,0.0,1.0,1.0,
		0,0,&bestTallness
	);
	return bestTallness;
}


void emTkBorder::PaintLabel(
	const emPainter & painter, double x, double y, double w, double h,
	emColor color, emColor canvasColor
)
{
	DoLabel(
		LABEL_FUNC_PAINT,&painter,x,y,w,h,color,canvasColor,NULL
	);
}


void emTkBorder::SetLabelInBorder(bool labelInBorder)
{
	if (LabelInBorder!=labelInBorder) {
		LabelInBorder=labelInBorder;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


emTkBorder::TkResources::TkResources()
{
}


emTkBorder::TkResources::~TkResources()
{
}


void emTkBorder::DoBorder(
	DoBorderFunc func, const emPainter * painter, emColor canvasColor,
	double * pX, double * pY, double * pW, double * pH, double * pR,
	emColor * pCanvasColor
)
{
	double s,d,e,f,g,h,r,tx,ty,tw,th,tr;
	double minSpace,howToSpace,labelSpace;
	double rndX,rndY,rndW,rndH,rndR,recX,recY,recW,recH;
	emColor color,color2;

	h=GetHeight();

	switch ((OuterBorderType)OuterBorder) {
	default: // OBT_NONE or OBT_FILLED
		rndX=0.0;
		rndY=0.0;
		rndW=1.0;
		rndH=h;
		rndR=0.0;
		minSpace=0.0;
		howToSpace=0.023;
		labelSpace=0.17;
		if ((OuterBorderType)OuterBorder==OBT_FILLED) {
			color=Look.GetBgColor();
			if (!color.IsTotallyTransparent()) {
				if (func==BORDER_FUNC_PAINT) painter->Clear(color,canvasColor);
				canvasColor=color;
			}
		}
		break;
	case OBT_MARGIN:
	case OBT_MARGIN_FILLED:
		d=emMin(1.0,h)*BorderScaling*0.04;
		rndX=d;
		rndY=d;
		rndW=1.0-2*d;
		rndH=h-2*d;
		rndR=0.0;
		minSpace=0.0;
		howToSpace=0.023;
		labelSpace=0.17;
		if ((OuterBorderType)OuterBorder==OBT_MARGIN_FILLED) {
			color=Look.GetBgColor();
			if (!color.IsTotallyTransparent()) {
				if (func==BORDER_FUNC_PAINT) painter->Clear(color,canvasColor);
				canvasColor=color;
			}
		}
		break;
	case OBT_RECT:
		s=emMin(1.0,h)*BorderScaling;
		d=s*0.023;
		e=s*0.02;
		f=d+e;
		rndX=f;
		rndY=f;
		rndW=1.0-2*f;
		rndH=h-2*f;
		rndR=0.0;
		minSpace=howToSpace=0.023;
		labelSpace=0.17;
		color=Look.GetBgColor();
		if (!color.IsTotallyTransparent()) {
			if (func==BORDER_FUNC_PAINT) {
				painter->PaintRect(
					d,d,1.0-2*d,h-2*d,
					color,canvasColor
				);
			}
			canvasColor=color;
		}
		if (func==BORDER_FUNC_PAINT) {
			d+=e*0.5;
			color=Look.GetFgColor();
			if (!IsEnabled()) color=color.GetTransparented(75.0F);
			painter->PaintRectOutline(
				d,d,1.0-2*d,h-2*d,e,
				color,canvasColor
			);
		}
		break;
	case OBT_ROUND_RECT:
		s=emMin(1.0,h)*BorderScaling;
		d=s*0.023;
		e=s*0.02;
		f=s*0.22;
		g=d+e;
		rndX=g;
		rndY=g;
		rndW=1.0-2*g;
		rndH=h-2*g;
		rndR=f-e;
		minSpace=howToSpace=0.023;
		labelSpace=0.17;
		color=Look.GetBgColor();
		if (!color.IsTotallyTransparent()) {
			if (func==BORDER_FUNC_PAINT) {
				painter->PaintRoundRect(
					d,d,1.0-2*d,h-2*d,f,f,
					color,canvasColor
				);
			}
			canvasColor=color;
		}
		if (func==BORDER_FUNC_PAINT) {
			d+=e*0.5;
			f-=e*0.5;
			color=Look.GetFgColor();
			if (!IsEnabled()) color=color.GetTransparented(75.0F);
			painter->PaintRoundRectOutline(
				d,d,1.0-2*d,h-2*d,f,f,e,
				color,canvasColor
			);
		}
		break;
	case OBT_GROUP:
		s=emMin(1.0,h)*BorderScaling;
		d=s*0.0104;
		rndX=d;
		rndY=d;
		rndW=1.0-2*d;
		rndH=h-2*d;
		rndR=s*0.0188;
		minSpace=howToSpace=0.0046;
		labelSpace=0.05;
		color=Look.GetBgColor();
		if (func==BORDER_FUNC_PAINT) {
			color2=canvasColor;
			if (
				!color.IsTotallyTransparent() &&
				(!color2.IsOpaque() || color2!=color)
			) {
				r=rndR*(98.0/73.0);
				e=r-rndR;
				painter->PaintRoundRect(
					rndX-e,rndY-e,rndW+2*e,rndH+2*e,
					r,r,
					color,
					color2
				);
				color2=0;
			}
			r=rndR*(100.0/73.0);
			e=r-rndR;
			painter->PaintBorderImage(
				rndX-e,rndY-e,rndW+2*e,rndH+2*e,
				r,r,r,r,
				GetTkResources().ImgGroupBorder,
				100.0,100.0,100.0,100.0,
				255,color2,0757
			);
		}
		if (!color.IsTotallyTransparent()) canvasColor=color;
		break;
	case OBT_INSTRUMENT:
		s=emMin(1.0,h)*BorderScaling;
		d=s*0.052;
		rndX=d;
		rndY=d;
		rndW=1.0-2*d;
		rndH=h-2*d;
		rndR=s*0.094;
		minSpace=howToSpace=0.023;
		labelSpace=0.17;
		color=Look.GetBgColor();
		if (func==BORDER_FUNC_PAINT) {
			color2=canvasColor;
			if (
				!color.IsTotallyTransparent() &&
				(!color2.IsOpaque() || color2!=color)
			) {
				r=rndR*(98.0/73.0);
				e=r-rndR;
				painter->PaintRoundRect(
					rndX-e,rndY-e,rndW+2*e,rndH+2*e,
					r,r,
					color,
					color2
				);
				color2=0;
			}
			r=rndR*(100.0/73.0);
			e=r-rndR;
			painter->PaintBorderImage(
				rndX-e,rndY-e,rndW+2*e,rndH+2*e,
				r,r,r,r,
				GetTkResources().ImgGroupBorder,
				100.0,100.0,100.0,100.0,
				255,color2,0757
			);
		}
		if (!color.IsTotallyTransparent()) canvasColor=color;
		break;
	case OBT_INSTRUMENT_MORE_ROUND:
		s=emMin(1.0,h)*BorderScaling;
		d=s*0.052;
		rndX=d;
		rndY=d;
		rndW=1.0-2*d;
		rndH=h-2*d;
		rndR=s*0.223;
		minSpace=howToSpace=0.023;
		labelSpace=0.17;
		color=Look.GetBgColor();
		if (func==BORDER_FUNC_PAINT) {
			color2=canvasColor;
			if (
				!color.IsTotallyTransparent() &&
				(!color2.IsOpaque() || color2!=color)
			) {
				r=rndR*(125.9/110.5);
				e=r-rndR;
				painter->PaintRoundRect(
					rndX-e,rndY-e,rndW+2*e,rndH+2*e,
					r,r,
					color,
					color2
				);
				color2=0;
			}
			r=rndR*(127.0/110.5);
			e=r-rndR;
			painter->PaintBorderImage(
				rndX-e,rndY-e,rndW+2*e,rndH+2*e,
				r,r,r,r,
				GetTkResources().ImgButtonBorder,
				127.0,127.0,127.0,127.0,
				255,color2,0757
			);
		}
		if (!color.IsTotallyTransparent()) canvasColor=color;
		break;
	case OBT_POPUP_ROOT:
		s=emMin(1.0,h)*BorderScaling;
		d=s*0.006;
		rndX=d;
		rndY=d;
		rndW=1.0-2*d;
		rndH=h-2*d;
		rndR=0.0;
		minSpace=0.0;
		howToSpace=0.023;
		labelSpace=0.17;
		color=Look.GetBgColor();
		if (!color.IsTotallyTransparent()) {
			if (func==BORDER_FUNC_PAINT) {
				painter->Clear(color,canvasColor);
			}
			canvasColor=color;
		}
		if (func==BORDER_FUNC_PAINT) {
			r=d*(78.0/62.0);
			painter->PaintBorderImage(
				0.0,0.0,1.0,h,
				r,r,r,r,
				GetTkResources().ImgPopupBorder,
				78.0,78.0,78.0,78.0,
				255,canvasColor,0757
			);
		}
		break;
	}

	s=emMin(rndW,rndH)*BorderScaling;
	minSpace*=s;

	if (HasHowTo()) {
		howToSpace*=s;
		if (func==BORDER_FUNC_PAINT) {
			tw=howToSpace*0.9;
			th=tw*2.0;
			tx=rndX+(howToSpace-tw)*0.5;
			ty=rndY+(rndH-th)*0.5;
			painter->PaintRoundRect(
				tx,ty,tw,th,tw*0.01,tw*0.01,
				Look.GetFgColor().GetTransparented(90.0F),
				canvasColor
			);
			if (PanelToViewDeltaX(tw)*PanelToViewDeltaY(th)>100.0) {
				d=tw*0.01;
				painter->PaintTextBoxed(
					tx+d,ty+d,tw-d*2,th-d*2,
					GetHowTo(),
					th,
					Look.GetFgColor().GetTransparented(35.0F),
					canvasColor,
					EM_ALIGN_TOP_LEFT,
					EM_ALIGN_LEFT,
					0.9
				);
			}
		}
		if (howToSpace>minSpace) {
			rndX+=howToSpace-minSpace;
			rndW-=howToSpace-minSpace;
		}
	}

	if (LabelInBorder && HasLabel()) {
		labelSpace*=s;
		if (func==BORDER_FUNC_PAINT || func==BORDER_FUNC_AUX_RECT) {
			d=labelSpace*0.1;
			ty=rndY+d;
			th=labelSpace-2*d;
			e=emMax(d,minSpace);
			if (e<rndR) {
				f=d*0.77;
				r=rndR-f;
				g=r-d+f;
				f=rndR-sqrt(r*r-g*g);
				if (e<f) e=f;
			}
			tx=rndX+e;
			tw=rndW-2*e;
			if (Aux) {
				d=th*0.2;
				e=th/Aux->Tallness;
				f=d+e+th/GetBestLabelTallness()*0.5;
				if (tw<f) {
					f=tw/f;
					d*=f;
					e*=f;
					ty+=(th-th*f)*0.5;
					th*=f;
				}
				if (func==BORDER_FUNC_AUX_RECT) {
					if (pX) *pX=tx+tw-e;
					if (pY) *pY=ty;
					if (pW) *pW=e;
					if (pH) *pH=th;
					if (pCanvasColor) *pCanvasColor=canvasColor;
					return;
				}
				tw-=d+e;
			}
			PaintLabel(
				*painter,
				tx,ty,tw,th,
				IsEnabled() ?
					Look.GetFgColor()
				:
					Look.GetFgColor().GetTransparented(75.0F),
				canvasColor
			);
		}
		rndX+=minSpace;
		rndW-=2*minSpace;
		rndY+=labelSpace;
		rndH-=labelSpace+minSpace;
		rndR-=minSpace;
		if (rndR>0.0) {
			recX=rndX+rndR*0.5;
			recW=rndW-rndR;
			recY=rndY;
			recH=rndH-rndR*0.5;
			d=minSpace+rndR*0.5-labelSpace;
			if (d>0) { recY+=d; recH-=d; }
		}
		else {
			rndR=0.0;
			recX=rndX;
			recW=rndW;
			recY=rndY;
			recH=rndH;
		}
	}
	else if (Aux) {
		s=emMin(rndW,rndH);
		tw=s*0.2;
		th=tw*Aux->Tallness;
		d=s*0.03;
		e=rndH-2*emMax(rndR,d);
		if (th>e) {
			th=emMax(1E-100,e);
			tw=th/Aux->Tallness;
		}
		if (func==BORDER_FUNC_AUX_RECT) {
			if (pX) *pX=rndX+rndW-tw-d;
			if (pY) *pY=rndY+(rndH-th)*0.5;
			if (pW) *pW=tw;
			if (pH) *pH=th;
			if (pCanvasColor) *pCanvasColor=canvasColor;
			return;
		}
		e=s*0.04;
		tw+=e+d;
		if (tw<minSpace) tw=minSpace;
		rndX+=minSpace;
		rndW-=minSpace+tw;
		rndY+=minSpace;
		rndH-=2*minSpace;
		rndR-=minSpace;
		if (rndR>0.0) {
			recX=rndX+rndR*0.5;
			recW=rndW-rndR*0.5;
			recY=rndY+rndR*0.5;
			recH=rndH-rndR;
			d=minSpace+rndR*0.5-tw;
			if (d>0) recW-=d;
		}
		else {
			rndR=0.0;
			recX=rndX;
			recW=rndW;
			recY=rndY;
			recH=rndH;
		}
	}
	else {
		rndX+=minSpace;
		rndY+=minSpace;
		rndW-=2*minSpace;
		rndH-=2*minSpace;
		rndR-=minSpace;
		if (rndR>0.0) {
			recX=rndX+rndR*0.5;
			recY=rndY+rndR*0.5;
			recW=rndW-rndR;
			recH=rndH-rndR;
		}
		else {
			rndR=0.0;
			recX=rndX;
			recW=rndW;
			recY=rndY;
			recH=rndH;
		}
	}

	switch (InnerBorder) {
	case IBT_GROUP:
		r=emMin(1.0,h)*BorderScaling*0.0125;
		if (rndR<r) rndR=r;
		if (func==BORDER_FUNC_PAINT) {
			painter->PaintBorderImage(
				rndX,rndY,rndW,rndH,
				rndR,rndR,rndR,rndR,
				GetTkResources().ImgGroupInnerBorder,
				136,136,136,136,
				255,canvasColor,0757
			);
		}
		d=rndR*(10.0/136.0);
		rndX+=d;
		rndY+=d;
		rndW-=2*d;
		rndH-=2*d;
		rndR-=d;
		recX=rndX+rndR*0.5;
		recY=rndY+rndR*0.5;
		recW=rndW-rndR;
		recH=rndH-rndR;
		break;
	case IBT_INPUT_FIELD:
	case IBT_OUTPUT_FIELD:
		r=emMin(1.0,h)*BorderScaling*0.0125;
		if (rndR<r) rndR=r;
		d=(1-(119.0-9.0)/119.0)*rndR;
		tx=rndX+d;
		ty=rndY+d;
		tw=rndW-2*d;
		th=rndH-2*d;
		tr=rndR-d;
		recX=tx+tr*0.5;
		recY=ty+tr*0.5;
		recW=tw-tr;
		recH=th-tr;
		if ((InnerBorderType)InnerBorder==IBT_INPUT_FIELD) color=Look.GetInputBgColor();
		else color=Look.GetOutputBgColor();
		if (func==BORDER_FUNC_PAINT) {
			painter->PaintRoundRect(tx,ty,tw,th,tr,tr,color,canvasColor);
			canvasColor=color;
			PaintContent(*painter,recX,recY,recW,recH,canvasColor);
			painter->PaintBorderImage(
				rndX,rndY,rndW,rndH,
				171.0/119*rndR,197.0/119*rndR,120.0/119*rndR,120.0/119*rndR,
				GetTkResources().ImgIOField,
				171.0,197.0,120.0,120.0,
				255,0,0757
			);
			return;
		}
		rndX=tx;
		rndY=ty;
		rndW=tw;
		rndH=th;
		rndR=tr;
		canvasColor=color;
		break;
	case IBT_CUSTOM_RECT:
		d=rndR*0.25;
		rndX+=d;
		rndY+=d;
		rndW-=2*d;
		rndH-=2*d;
		rndR-=d;
		r=emMin(1.0,h)*BorderScaling*0.0125;
		if (rndR<r) rndR=r;
		if (func==BORDER_FUNC_PAINT) {
			painter->PaintBorderImage(
				rndX,rndY,rndW,rndH,
				rndR,rndR,rndR,rndR,
				GetTkResources().ImgCustomRectBorder,
				39,39,39,39,
				255,canvasColor,0757
			);
		}
		d=rndR;
		rndX+=d;
		rndY+=d;
		rndW-=2*d;
		rndH-=2*d;
		rndR=0;
		recX=rndX;
		recY=rndY;
		recW=rndW;
		recH=rndH;
		break;
	default:
		break;
	}

	if (func==BORDER_FUNC_CONTENT_ROUND_RECT) {
		if (pX) *pX=rndX;
		if (pY) *pY=rndY;
		if (pW) *pW=rndW;
		if (pH) *pH=rndH;
		if (pR) *pR=rndR;
		if (pCanvasColor) *pCanvasColor=canvasColor;
	}
	else if (func==BORDER_FUNC_CONTENT_RECT) {
		if (pX) *pX=recX;
		if (pY) *pY=recY;
		if (pW) *pW=recW;
		if (pH) *pH=recH;
		if (pCanvasColor) *pCanvasColor=canvasColor;
	}
	else {
		PaintContent(*painter,recX,recY,recW,recH,canvasColor);
	}
}


void emTkBorder::DoLabel(
	DoLabelFunc func, const emPainter * painter, double x, double y,
	double w, double h, emColor color, emColor canvasColor,
	double * pBestTallness
)
{
	double iconW,capW,capAndIconH,descW,descH,hGap,vGap,totalW,totalH;
	double minTotalW,minWS,f,w2,h2;

	iconW=0.0;
	capW=0.0;
	capAndIconH=0.0;
	descW=0.0;
	descH=0.0;
	hGap=0.0;
	vGap=0.0;
	if (!Icon.IsEmpty()) {
		capAndIconH=1.0;
		iconW=((double)Icon.GetWidth())/Icon.GetHeight();
		if (!Caption.IsEmpty()) hGap=0.1;
	}
	if (!Caption.IsEmpty()) {
		capAndIconH=1.0;
		capW=emPainter::GetTextSize(Caption,1.0,true,0.0,&f);
		capW/=f;
	}
	if (!Description.IsEmpty()) {
		descH=1.0;
		descW=emPainter::GetTextSize(Description,1.0,true,0.0,&f);
		descW/=f;
		if (!Icon.IsEmpty() || !Caption.IsEmpty()) {
			totalW=iconW+hGap+capW;
			f=emMin(totalW/descW,0.15);
			descW*=f;
			descH*=f;
			vGap=descH*0.05;
			totalH=capAndIconH+vGap+descH;
		}
		else {
			totalW=descW;
			totalH=descH;
		}
	}
	else {
		if (!Icon.IsEmpty() || !Caption.IsEmpty()) {
			totalW=iconW+hGap+capW;
			totalH=capAndIconH;
		}
		else {
			totalW=1.0;
			totalH=1.0;
		}
	}

	if (func==LABEL_FUNC_GET_BEST_TALLNESS) {
		*pBestTallness=totalH/totalW;
		return;
	}

	minWS=0.5;

	f=h/totalH;
	w2=f*totalW;
	if (w2<=w) {
		if (!(LabelAlignment&EM_ALIGN_LEFT)) {
			if (!(LabelAlignment&EM_ALIGN_RIGHT)) {
				x+=(w-w2)*0.5;
			}
			else {
				x+=w-w2;
			}
		}
		w=w2;
	}
	else {
		if (!Caption.IsEmpty()) minTotalW=iconW+hGap+capW*minWS;
		else if (!Icon.IsEmpty()) minTotalW=totalW;
		else minTotalW=descW*minWS;
		w2=f*minTotalW;
		if (w2>w) {
			f=w/minTotalW;
			h2=f*totalH;
			if (!(LabelAlignment&EM_ALIGN_TOP)) {
				if (!(LabelAlignment&EM_ALIGN_BOTTOM)) {
					y+=(h-h2)*0.5;
				}
				else {
					y+=h-h2;
				}
			}
			h=h2;
		}
	}

	if (!Icon.IsEmpty()) {
		if (Icon.GetChannelCount()==1) {
			painter->PaintShape(
				x,
				y,
				iconW*f,
				capAndIconH*f,
				Icon,
				0,
				color,
				canvasColor
			);
		}
		else {
			painter->PaintImage(
				x,
				y,
				iconW*f,
				capAndIconH*f,
				Icon,
				color.GetAlpha(),
				canvasColor
			);
		}
	}
	if (!Caption.IsEmpty()) {
		painter->PaintTextBoxed(
			x+(iconW+hGap)*f,
			y,
			w-(iconW+hGap)*f,
			capAndIconH*f,
			Caption,
			capAndIconH*f,
			color,
			canvasColor,
			EM_ALIGN_CENTER,
			CaptionAlignment,
			minWS
		);
	}
	if (!Description.IsEmpty()) {
		painter->PaintTextBoxed(
			x,
			y+(capAndIconH+vGap)*f,
			w,
			h-(capAndIconH+vGap)*f,
			Description,
			h-(capAndIconH+vGap)*f,
			color,
			canvasColor,
			EM_ALIGN_CENTER, //???LabelAlignment&~(EM_ALIGN_TOP|EM_ALIGN_BOTTOM),
			DescriptionAlignment,
			minWS
		);
	}
}


const char * emTkBorder::HowToPreface=
	"How to use this panel\n"
	"#####################\n"
	"\n"
	"Here is some text describing the usage of this panel. The text consists of\n"
	"multiple sections which may come from different parts of the program based on\n"
	"each other. If something is contradictory, the later section should count.\n"
;


const char * emTkBorder::HowToDisabled=
	"\n"
	"\n"
	"DISABLED\n"
	"\n"
	"This panel is currently disabled, because the panel is probably irrelevant for\n"
	"the current state of the program or data. Any try to modify data or to trigger a\n"
	"function may silently be ignored.\n"
;


const char * emTkBorder::HowToFocus=
	"\n"
	"\n"
	"FOCUS\n"
	"\n"
	"This panel is focusable. Only one panel can be focused at a time. The focus is\n"
	"indicated by small arrows pointing to the focused panel. If a panel is focused,\n"
	"it gets the keyboard input. If the focused panel does not know what to do with a\n"
	"certain input key, it may even forward the input to its ancestor panels.\n"
	"\n"
	"How to move or set the focus:\n"
	"\n"
	"* Just zoom and scroll around - the focus is moved automatically by that.\n"
	"\n"
	"* Click with the left or right mouse button on a panel to give it the focus.\n"
	"\n"
	"* Press Tab or Shift+Tab to move the focus to the next or previous sister\n"
	"  panel.\n"
	"\n"
	"* Press the cursor keys to move the focus to a sister panel in the desired\n"
	"  direction.\n"
	"\n"
	"* Press Page-Up or -Down to move the focus to a child or parent panel.\n"
;


//==============================================================================
//================================= emTkLabel ==================================
//==============================================================================

emTkLabel::emTkLabel(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emTkBorder(parent,name,caption,description,icon)
{
	SetOuterBorderType(OBT_MARGIN);
	SetLabelInBorder(false);
	SetFocusable(false);
}


void emTkLabel::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
)
{
	PaintLabel(
		painter,
		x,y,w,h,
		IsEnabled() ?
			GetLook().GetFgColor()
		:
			GetLook().GetFgColor().GetTransparented(75.0F),
		canvasColor
	);
}


//==============================================================================
//================================= emTkTiling =================================
//==============================================================================

emTkTiling::emTkTiling(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emTkBorder(parent,name,caption,description,icon)
{
	SpaceL=0.0;
	SpaceT=0.0;
	SpaceH=0.0;
	SpaceV=0.0;
	SpaceR=0.0;
	SpaceB=0.0;
	PCT=0.2;
	PCTPos.SetTuningLevel(4);
	PCTNeg.SetTuningLevel(4);
	FixedColumnCount=0;
	FixedRowCount=0;
	MinCellCount=0;
	FCTColumn=-1;
	FCTRow=-1;
	Alignment=EM_ALIGN_CENTER;
	RowByRow=0;
	SetFocusable(false);
}


emTkTiling::~emTkTiling()
{
}


void emTkTiling::SetRowByRow(bool rowByRow)
{
	if ((bool)RowByRow!=rowByRow) {
		RowByRow=rowByRow?1:0;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetFixedColumnCount(int fixedColumnCount)
{
	if (fixedColumnCount<0) fixedColumnCount=0;
	if (FixedColumnCount!=fixedColumnCount) {
		FixedColumnCount=fixedColumnCount;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetFixedRowCount(int fixedRowCount)
{
	if (fixedRowCount<0) fixedRowCount=0;
	if (FixedRowCount!=fixedRowCount) {
		FixedRowCount=fixedRowCount;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetMinCellCount(int minCellCount)
{
	if (minCellCount<0) minCellCount=0;
	if (MinCellCount!=minCellCount) {
		MinCellCount=minCellCount;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetChildTallness(double ct)
{
	SetPrefChildTallness(ct);
	SetChildTallnessForced();
}


void emTkTiling::SetPrefChildTallness(double pct)
{
	SetPrefChildTallness(pct,0,true);
}


double emTkTiling::GetPrefChildTallness(int idx) const
{
	if (idx!=0) {
		if (idx>0) {
			if (idx>PCTPos.GetCount()) idx=PCTPos.GetCount();
			if (idx>0) return PCTPos[idx-1];
		}
		else if (idx<0) {
			idx=-idx;
			if (idx>PCTNeg.GetCount()) idx=PCTNeg.GetCount();
			if (idx>0) return PCTNeg[idx-1];
		}
	}
	return PCT;
}


void emTkTiling::SetPrefChildTallness(double pct, int idx, bool allFurther)
{
	emArray<double> * arr;
	bool modified;
	double last;
	int n, m;

	if (pct<1E-100) pct=1E-100;

	modified=false;
	if (idx==0) {
		if (allFurther) {
			if (!PCTPos.IsEmpty()) { PCTPos.Empty(); modified=true; }
			if (!PCTNeg.IsEmpty()) { PCTNeg.Empty(); modified=true; }
		}
		if (PCT!=pct) {
			if (!allFurther) {
				if (PCTPos.IsEmpty()) PCTPos.Add(PCT);
				if (PCTNeg.IsEmpty()) PCTNeg.Add(PCT);
			}
			PCT=pct;
			modified=true;
		}
	}
	else {
		if (idx>0) arr=&PCTPos;
		else { idx=-idx; arr=&PCTNeg; }
		n=arr->GetCount();
		if (idx<n) {
			if (arr->Get(idx-1)!=pct) {
				arr->Set(idx-1,pct);
				modified=true;
			}
			if (allFurther) {
				arr->SetCount(idx);
				modified=true;
			}
		}
		else {
			last=n>0?arr->Get(n-1):PCT;
			if (pct!=last) {
				m=idx-n;
				if (!allFurther) m++;
				if (m>0) arr->Add(last,m);
				arr->Set(idx-1,pct);
				modified=true;
			}
		}
	}

	if (modified) InvalidateChildrenLayout();
}


void emTkTiling::SetChildTallnessForced(bool forced)
{
	SetForcedChildTallnessColumn(forced ? 0 : -1);
	SetForcedChildTallnessRow(forced ? 0 : -1);
}


void emTkTiling::SetForcedChildTallnessColumn(int column)
{
	if (FCTColumn!=column) {
		FCTColumn=column;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetForcedChildTallnessRow(int row)
{
	if (FCTRow!=row) {
		FCTRow=row;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetAlignment(emAlignment alignment)
{
	if (Alignment!=alignment) {
		Alignment=alignment;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetSpaceL(double l)
{
	if (l<0.0) l=0.0;
	if (SpaceL!=l) {
		SpaceL=l;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetSpaceT(double t)
{
	if (t<0.0) t=0.0;
	if (SpaceT!=t) {
		SpaceT=t;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetSpaceH(double h)
{
	if (h<0.0) h=0.0;
	if (SpaceH!=h) {
		SpaceH=h;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetSpaceV(double v)
{
	if (v<0.0) v=0.0;
	if (SpaceV!=v) {
		SpaceV=v;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetSpaceR(double r)
{
	if (r<0.0) r=0.0;
	if (SpaceR!=r) {
		SpaceR=r;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetSpaceB(double b)
{
	if (b<0.0) b=0.0;
	if (SpaceB!=b) {
		SpaceB=b;
		InvalidateChildrenLayout();
	}
}


void emTkTiling::SetSpace(
	double l, double t, double h, double v, double r, double b
)
{
	SetSpaceL(l);
	SetSpaceT(t);
	SetSpaceH(h);
	SetSpaceV(v);
	SetSpaceR(r);
	SetSpaceB(b);
}


void emTkTiling::SetSpace(double lr, double tb, double h, double v)
{
	SetSpace(lr,tb,h,v,lr,tb);
}


void emTkTiling::SetInnerSpace(double h, double v)
{
	SetSpaceH(h);
	SetSpaceV(v);
}


void emTkTiling::SetOuterSpace(double l, double t, double r, double b)
{
	SetSpaceL(l);
	SetSpaceT(t);
	SetSpaceR(r);
	SetSpaceB(b);
}


void emTkTiling::SetOuterSpace(double lr, double tb)
{
	SetOuterSpace(lr,tb,lr,tb);
}


void emTkTiling::LayoutChildren()
{
	emPanel * p, * aux;
	double x,y,w,h,t,err,errBest,cx,cy,cw,ch,tw,th,fx,fy,sx,sy,ux,uy;
	int n,m,cells,cols,rows,rowsBest,col,row;
	emColor cc;

	emTkBorder::LayoutChildren();

	aux=GetAuxPanel();

	for (cells=0, p=GetFirstChild(); p; p=p->GetNext()) {
		if (p!=aux) cells++;
	}
	if (!cells) return;
	if (cells<MinCellCount) cells=MinCellCount;

	GetContentRect(&x,&y,&w,&h,&cc);
	if (w<1E-100) w=1E-100;
	if (h<1E-100) h=1E-100;

	if (FixedColumnCount>0) {
		cols=FixedColumnCount;
		rows=(cells+cols-1)/cols;
		if (rows<FixedRowCount) rows=FixedRowCount;
	}
	else if (FixedRowCount>0) {
		rows=FixedRowCount;
		cols=(cells+rows-1)/rows;
	}
	else {
		rowsBest=1;
		errBest=0.0;
		t=h/w;
		for (rows=1;;) {
			cols=(cells+rows-1)/rows;
			cw=1.0;
			m=PCTPos.GetCount();
			if (m) {
				n=cols-1;
				if (n>m) { n=m-1; cw+=PCT/PCTPos[n]*(cols-m); }
				while (n>0) cw+=PCT/PCTPos[--n];
			}
			else cw*=cols;
			ch=PCT;
			m=PCTNeg.GetCount();
			if (m) {
				n=rows-1;
				if (n>m) { n=m-1; ch+=PCTNeg[n]*(rows-m); }
				while (n>0) ch+=PCTNeg[--n];
			}
			else ch*=rows;
			cw=((SpaceL+SpaceR+SpaceH*(cols-1))/cols+1.0)*cw;
			ch=((SpaceT+SpaceB+SpaceV*(rows-1))/rows+1.0)*ch;
			err=fabs(log(t*cw/ch));
			if (rows==1 || err<errBest) {
				rowsBest=rows;
				errBest=err;
			}
			if (cols==1) break;
			rows=(cells+cols-2)/(cols-1); // skip rows until cols decreases
		}
		rows=rowsBest;
		cols=(cells+rows-1)/rows;
	}

	cw=1.0;
	m=PCTPos.GetCount();
	if (m) {
		n=cols-1;
		if (n>m) { n=m-1; cw+=PCT/PCTPos[n]*(cols-m); }
		while (n>0) cw+=PCT/PCTPos[--n];
	}
	else cw*=cols;

	ch=PCT;
	m=PCTNeg.GetCount();
	if (m) {
		n=rows-1;
		if (n>m) { n=m-1; ch+=PCTNeg[n]*(rows-m); }
		while (n>0) ch+=PCTNeg[--n];
	}
	else ch*=rows;

	sx=SpaceL+SpaceR+SpaceH*(cols-1);
	sy=SpaceT+SpaceB+SpaceV*(rows-1);
	ux=sx/cols+1.0;
	uy=sy/rows+1.0;
	cw*=ux;
	ch*=uy;
	fx=w/cw;
	fy=h/ch;
	tw=w;
	th=h;
	if (FCTColumn>=0 && FCTColumn<cols) {
		if (cols==1 || (FCTRow>=0 && FCTRow<rows)) {
			if (fx>fy) { fx=fy; tw=cw*fx; }
			else if (fx<fy) { fy=fx; th=ch*fy; }
		}
		else {
			t=ux*PCT/GetPrefChildTallness(FCTColumn);
			fx=(w-t*fy)/(cw-t);
			if (fx<0.0) { fx=0.0; fy=w/t; th=ch*fy; }
		}
	}
	else if (FCTRow>=0 && FCTRow<rows) {
		if (rows==1) {
			if (fx>fy) { fx=fy; tw=cw*fx; }
			else if (fx<fy) { fy=fx; th=ch*fy; }
		}
		else {
			t=uy*GetPrefChildTallness(-FCTRow);
			fy=(h-t*fx)/(ch-t);
			if (fy<0.0) { fy=0.0; fx=h/t; tw=cw*fx; }
		}
	}

	if (Alignment&EM_ALIGN_RIGHT) x+=w-tw;
	else if (!(Alignment&EM_ALIGN_LEFT)) x+=(w-tw)*0.5;

	if (Alignment&EM_ALIGN_BOTTOM) y+=h-th;
	else if (!(Alignment&EM_ALIGN_TOP)) y+=(h-th)*0.5;

	if (sx>=1E-100) {
		sx=(tw-tw/ux)/sx;
		x+=sx*SpaceL;
		sx*=SpaceH;
	}
	else sx=0.0;

	if (sy>=1E-100) {
		sy=(th-th/uy)/sy;
		y+=sy*SpaceT;
		sy*=SpaceV;
	}
	else sy=0.0;

	for (cx=x, cy=y, row=0, col=0, p=GetFirstChild(); p; p=p->GetNext()) {
		if (p==aux) continue;
		n=emMin(col,PCTPos.GetCount());
		if (n==0) cw=1.0; else cw=PCT/PCTPos[n-1];
		n=emMin(row,PCTNeg.GetCount());
		if (n==0) ch=PCT; else ch=PCTNeg[n-1];
		if (col==FCTColumn) cw*=fy; else cw*=fx;
		if (row==FCTRow) ch*=fx; else ch*=fy;
		p->Layout(cx,cy,cw,ch,cc);
		if (RowByRow) {
			cx+=cw+sx;
			col++;
			if (col>=cols) {
				row++;
				cy+=ch+sy;
				col=0;
				cx=x;
			}
		}
		else {
			cy+=ch+sy;
			row++;
			if (row>=rows) {
				col++;
				cx+=cw+sx;
				row=0;
				cy=y;
			}
		}
	}
}


//==============================================================================
//================================= emTkGroup ==================================
//==============================================================================

emTkGroup::emTkGroup(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emTkTiling(parent,name,caption,description,icon)
{
	SetFocusable(true);
	SetBorderType(OBT_GROUP,IBT_GROUP);
}


//==============================================================================
//================================= emTkTunnel =================================
//==============================================================================

emTkTunnel::emTkTunnel(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emTkBorder(parent,name,caption,description,icon)
{
	ChildTallness=0.0;
	Depth=10.0;
	SetBorderType(OBT_INSTRUMENT,IBT_GROUP);
}


void emTkTunnel::SetChildTallness(double childTallness)
{
	if (ChildTallness!=childTallness) {
		ChildTallness=childTallness;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkTunnel::SetDepth(double depth)
{
	if (depth<1E-10) depth=1E-10;
	if (Depth!=depth) {
		Depth=depth;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkTunnel::GetChildRect(
	double * pX, double * pY, double * pW, double * pH,
	emColor * pCanvasColor
)
{
	DoTunnel(TUNNEL_FUNC_CHILD_RECT,NULL,0,pX,pY,pW,pH,pCanvasColor);
}


void emTkTunnel::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
)
{
	DoTunnel(TUNNEL_FUNC_PAINT,&painter,canvasColor,NULL,NULL,NULL,NULL,NULL);
}


void emTkTunnel::LayoutChildren()
{
	emPanel * p, * aux;
	double x,y,w,h;
	emColor cc;

	emTkBorder::LayoutChildren();

	p=GetFirstChild();
	if (!p) return;

	aux=GetAuxPanel();
	if (p==aux) {
		p=p->GetNext();
		if (!p) return;
	}

	GetChildRect(&x,&y,&w,&h,&cc);
	p->Layout(x,y,w,h,cc);
}


void emTkTunnel::DoTunnel(
	DoTunnelFunc func, const emPainter * painter, emColor canvasColor,
	double * pX, double * pY, double * pW, double * pH,
	emColor * pCanvasColor
)
{
	double d,f,ax,ay,aw,ah,ar,bx,by,bw,bh,br,circleQuality,imgRY,imgRX,dx,dy;
	double xy[4*2];
	int i,n,m,ja,jb;
	const emImage * img;
	emColor cc;

	GetContentRoundRect(&ax,&ay,&aw,&ah,&ar,&cc);

	d=1.0/(Depth+1.0);
	bw=aw*d;
	bh=ah*d;
	br=ar*d;
	if (ChildTallness>1E-100) {
		bw=sqrt((bw-br)*(bh-br)/ChildTallness);
		bh=bw*ChildTallness;
		br=ar/(emMin(aw,ah)-ar)*emMin(bw,bh);
		bw+=br;
		bh+=br;
		f=aw*0.999999/bw;
		if (f<1.0) { bw*=f; bh*=f; br*=f; }
		f=ah*0.999999/bh;
		if (f<1.0) { bw*=f; bh*=f; br*=f; }
	}
	bx=ax+(aw-bw)*0.5;
	by=ay+(ah-bh)*0.5;

	if (func==TUNNEL_FUNC_CHILD_RECT) {
		if (pX) *pX=bx+0.5*br;
		if (pY) *pY=by+0.5*br;
		if (pW) *pW=bw-br;
		if (pH) *pH=bh-br;
		if (pCanvasColor) *pCanvasColor=cc;
		return;
	}

	img=&GetTkResources().ImgTunnel;
	imgRX=img->GetWidth()*0.5;
	imgRY=img->GetHeight()*0.5;

	circleQuality=4.5;
	f=circleQuality*sqrt(ar*(painter->GetScaleX()+painter->GetScaleY()));
	if (f>256.0) f=256.0;
	f*=0.25;
	if (f<=1.0) n=1;
	else if (f>=64.0) n=64;
	else n=(int)(f+0.5);
	for (i=0, m=n*4, ja=0, jb=1; i<=m; i++, ja^=3, jb^=3) {
		f=(i+0.5)*(M_PI*2.0)/m;
		dx=cos(f);
		dy=sin(f);
		if ((i/n+1)&2) {
			xy[ja*2]=ax+(dx+1.0)*ar;
			xy[jb*2]=bx+(dx+1.0)*br;
		}
		else {
			xy[ja*2]=ax+aw+(dx-1.0)*ar;
			xy[jb*2]=bx+bw+(dx-1.0)*br;
		}
		if ((i/n)&2) {
			xy[ja*2+1]=ay+(dy+1.0)*ar;
			xy[jb*2+1]=by+(dy+1.0)*br;
		}
		else {
			xy[ja*2+1]=ay+ah+(dy-1.0)*ar;
			xy[jb*2+1]=by+bh+(dy-1.0)*br;
		}
		if (i>0) {
			f=i*(M_PI*2.0)/m;
			dx=cos(f);
			dy=sin(f);
			painter->PaintPolygon(
				xy,4,
				img->GetPixel(
					(int)(imgRX+(imgRX-0.6)*dx+0.5),
					(int)(imgRY+(imgRY-0.6)*dy+0.5)
				),
				canvasColor
			);
		}
	}
}


//==============================================================================
//================================= emTkButton =================================
//==============================================================================

emTkButton::emTkButton(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emTkBorder(parent,name,caption,description,icon)
{
	Pressed=false;
	NoEOI=false;
	ShownChecked=false;
	ShownBoxed=false;
	ShownRadioed=false;
	SetOuterBorderType(OBT_INSTRUMENT_MORE_ROUND);
	SetLabelInBorder(false);
	SetLabelAlignment(EM_ALIGN_CENTER);
}


emTkButton::~emTkButton()
{
}


void emTkButton::SetNoEOI(bool noEOI)
{
	NoEOI=noEOI;
}


void emTkButton::Click(bool shift)
{
	if (IsEnabled()) {
		if (!shift && !NoEOI) GetView().SignalEOIDelayed();
		Signal(ClickSignal);
		Clicked();
	}
}


void emTkButton::Clicked()
{
}


void emTkButton::PressStateChanged()
{
}


void emTkButton::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	static const double minExt=2.5;
	double vmx,vmy;

	if (
		event.IsLeftButton() && (state.IsNoMod() || state.IsShiftMod()) &&
		IsEnabled() && CheckMouse(mx,my) && GetViewCondition(VCT_MIN_EXT)>=minExt
	) {
		Focus();
		event.Eat();
		Pressed=true;
		InvalidatePainting();
		Signal(PressStateSignal);
		PressStateChanged();
	}

	if (Pressed && !state.GetLeftButton()) {
		Pressed=false;
		InvalidatePainting();
		Signal(PressStateSignal);
		PressStateChanged();
		if (CheckMouse(mx,my) && IsEnabled() && IsViewed()) {
			vmx=PanelToViewX(mx);
			vmy=PanelToViewY(my);
			if (
				vmx>=GetClipX1() && vmx<GetClipX2() &&
				vmy>=GetClipY1() && vmy<GetClipY2()
			) {
				Click(state.IsShiftMod());
			}
		}
	}

	if (
		event.IsKey(EM_KEY_ENTER) &&
		(state.IsNoMod() || state.IsShiftMod()) &&
		IsFocused() && IsEnabled() && GetViewCondition(VCT_MIN_EXT)>=minExt
	) {
		event.Eat();
		Click(state.IsShiftMod());
	}

	emPanel::Input(event,state,mx,my);
}


bool emTkButton::HasHowTo()
{
	return true;
}


emString emTkButton::GetHowTo()
{
	emString h;

	h=emTkBorder::GetHowTo();
	h+=HowToButton;
	if (!NoEOI) h+=HowToEOIButton;
	return h;
}


void emTkButton::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
)
{
	DoButton(BUTTON_FUNC_PAINT,&painter,canvasColor,0,0,NULL);
}


void emTkButton::PaintBoxSymbol(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
)
{
	double d;

	if (ShownChecked) {
		if (ShownRadioed) {
			d=w*0.25;
			painter.PaintEllipse(
				x+d,y+d,w-2*d,h-2*d,
				GetLook().GetInputFgColor(),
				canvasColor
			);
		}
		else {
			painter.PaintLine(
				x+w*0.2,
				y+h*0.6,
				x+w*0.4,
				y+h*0.8,
				w*0.16,
				emPainter::LC_ROUND,
				emPainter::LC_ROUND,
				GetLook().GetInputFgColor(),
				canvasColor
			);
			painter.PaintLine(
				x+w*0.4,
				y+h*0.8,
				x+w*0.8,
				y+h*0.2,
				w*0.16,
				emPainter::LC_ROUND,
				emPainter::LC_ROUND,
				GetLook().GetInputFgColor()
			);
		}
	}
}


bool emTkButton::CheckMouse(double mx, double my)
{
	bool b;

	DoButton(BUTTON_FUNC_CHECK_MOUSE,NULL,0,mx,my,&b);
	return b;
}


void emTkButton::SetShownChecked(bool shownChecked)
{
	if (((bool)ShownChecked)!=shownChecked) {
		ShownChecked=shownChecked;
		InvalidatePainting();
	}
}


void emTkButton::SetShownBoxed(bool shownBoxed)
{
	if (((bool)ShownBoxed)!=shownBoxed) {
		ShownBoxed=shownBoxed;
		InvalidatePainting();
	}
}


void emTkButton::SetShownRadioed(bool shownRadioed)
{
	if (((bool)ShownRadioed)!=shownRadioed) {
		ShownRadioed=shownRadioed;
		InvalidatePainting();
	}
}


void emTkButton::DoButton(
	DoButtonFunc func, const emPainter * painter,
	emColor canvasColor, double mx, double my, bool * pHit
)
{
	double x,y,w,h,r,d,f,bx,by,bw,bh,fx,fy,fw,fh,fr,lx,ly,lw,lh,dx,dy;
	emColor color;

	if (ShownBoxed) {

		GetContentRect(&x,&y,&w,&h);

		if (HasLabel()) {
			lw=1.0;
			lh=emMax(0.2,GetBestLabelTallness());
			bw=lh;
			d=bw*0.1;
			f=emMin(w/(bw+d+lw),h/lh);
			bw*=f;
			d*=f;
			lw=w-bw-d;
			lh=bw;
			lx=x+w-lw;
			ly=y+(h-lh)*0.5;
		}
		else {
			bw=emMin(w,h);
			lx=x;
			ly=y;
			lw=1E-100;
			lh=1E-100;
		}
		bh=bw;
		bx=x;
		by=y+(h-bh)*0.5;

		d=bw*0.13;
		bx+=d;
		by+=d;
		bw-=2*d;
		bh-=2*d;

		d=bw*8.0/120;
		fx=bx+d;
		fy=by+d;
		fw=bw-2*d;
		fh=bh-2*d;
		if (ShownRadioed) fr=fw*0.5;
		else fr=bw*24.0/120-d;

		if (func==BUTTON_FUNC_CHECK_MOUSE) {
			dx=emMax(emMax(fx-mx,mx-fx-fw)+fr,0.0);
			dy=emMax(emMax(fy-my,my-fy-fh)+fr,0.0);
			*pHit = dx*dx+dy*dy <= fr*fr;
			return;
		}

		color=GetLook().GetFgColor();
		if (!IsEnabled()) color=color.GetTransparented(75.0F);
		PaintLabel(
			*painter,
			lx,ly,lw,lh,
			color,
			canvasColor
		);

		color=GetLook().GetInputBgColor();
		painter->PaintRoundRect(fx,fy,fw,fh,fr,fr,color,canvasColor);
		canvasColor=color;

		PaintBoxSymbol(*painter,fx,fy,fw,fh,canvasColor);

		if (!IsEnabled()) painter->PaintRoundRect(fx,fy,fw,fh,fr,fr,0x888888E0);

		painter->PaintImage(
			bx,by,bw,bh,
			ShownRadioed ? (
				Pressed ?
					GetTkResources().ImgRadioBoxPressed :
					GetTkResources().ImgRadioBox
			)
			: (
				Pressed ?
					GetTkResources().ImgCheckBoxPressed :
					GetTkResources().ImgCheckBox
			),
			255
		);

	}
	else {

		GetContentRoundRect(&x,&y,&w,&h,&r);

		d=(1-(98.0-5.0)/98.0)*r;
		fx=x+d;
		fy=y+d;
		fw=w-2*d;
		fh=h-2*d;
		fr=r-d;
		if (func==BUTTON_FUNC_CHECK_MOUSE) {
			dx=emMax(emMax(fx-mx,mx-fx-fw)+fr,0.0);
			dy=emMax(emMax(fy-my,my-fy-fh)+fr,0.0);
			*pHit = dx*dx+dy*dy <= fr*fr;
			return;
		}
		color=GetLook().GetButtonBgColor();
		painter->PaintRoundRect(
			fx,fy,fw,fh,
			fr,fr,
			color,
			canvasColor
		);
		canvasColor=color;

		d=emMin(fw,fh)*0.1;
		dx=emMax(r*0.7,d);
		dy=emMax(r*0.4,d);
		lx=fx+dx;
		ly=fy+dy;
		lw=fw-2*dx;
		lh=fh-2*dy;
		if (Pressed || ShownChecked) {
			d=Pressed ? 0.98 : 0.983;
			lx+=(1-d)*0.5*lw;
			lw*=d;
			ly+=(1-d)*0.5*lh;
			lh*=d;
		}
		color=GetLook().GetButtonFgColor();
		if (!IsEnabled()) color=color.GetTransparented(75.0F);
		PaintLabel(
			*painter,
			lx,ly,lw,lh,
			color,
			canvasColor
		);

		if (Pressed) {
			painter->PaintBorderImage(
				x,y,w,h,
				135.0/98*r,151.0/98*r,99.0/98*r,99.0/98*r,
				GetTkResources().ImgButtonPressed,
				135.0,151.0,99.0,99.0,
				255,0,0757
			);
		}
		else if (ShownChecked) {
			painter->PaintBorderImage(
				x,y,w,h,
				132.0/98*r,146.0/98*r,99.0/98*r,99.0/98*r,
				GetTkResources().ImgButtonChecked,
				132.0,146.0,99.0,99.0,
				255,0,0757
			);
		}
		else {
			painter->PaintBorderImage(
				x,y,
				w+(353.0-340.0)/98*r,
				h+(365.0-340.0)/98*r,
				112.0/98*r,128.0/98*r,111.0/98*r,123.0/98*r,
				GetTkResources().ImgButton,
				112.0,128.0,111.0,123.0,
				255,0,0757
			);
		}
	}
}


const char * emTkButton::HowToButton=
	"\n"
	"\n"
	"BUTTON\n"
	"\n"
	"This is a button. Buttons can be triggered to perform an application defined\n"
	"function.\n"
	"\n"
	"In order to trigger a button, move the mouse pointer over the button and click\n"
	"the left mouse button. The function is triggered when releasing the mouse\n"
	"button, but only if the mouse pointer is still over the button.\n"
	"\n"
	"Alternatively, a button can be triggered by giving it the focus and pressing the\n"
	"Enter key.\n"
;


const char * emTkButton::HowToEOIButton=
	"\n"
	"\n"
	"EOI BUTTON\n"
	"\n"
	"This is an End Of Interaction button. The exact behavior is application defined,\n"
	"but it usually means that if the button is in a view that has popped up, the\n"
	"view is popped down automatically when the button is triggered. If you want to\n"
	"bypass that, hold the Shift key while triggering the button.\n"
;


//==============================================================================
//============================== emTkCheckButton ===============================
//==============================================================================

emTkCheckButton::emTkCheckButton(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emTkButton(parent,name,caption,description,icon)
{
	Checked=false;
}


emTkCheckButton::~emTkCheckButton()
{
}


void emTkCheckButton::SetChecked(bool checked)
{
	if (Checked!=checked) {
		Checked=checked;
		SetShownChecked(checked);
		InvalidatePainting();
		Signal(CheckSignal);
		CheckChanged();
	}
}


void emTkCheckButton::Clicked()
{
	SetChecked(!IsChecked());
}


void emTkCheckButton::CheckChanged()
{
}


emString emTkCheckButton::GetHowTo()
{
	emString h;

	h=emTkButton::GetHowTo();
	h+=HowToCheckButton;
	if (IsChecked()) h+=HowToChecked;
	else h+=HowToNotChecked;
	return h;
}


const char * emTkCheckButton::HowToCheckButton=
	"\n"
	"\n"
	"CHECK BUTTON\n"
	"\n"
	"This button can have checked or unchecked state. Usually this is a yes-or-no\n"
	"answer to a question. Whenever the button is triggered, the check state toggles.\n"
;


const char * emTkCheckButton::HowToChecked=
	"\n"
	"\n"
	"CHECKED\n"
	"\n"
	"Currently this check button is checked.\n"
;


const char * emTkCheckButton::HowToNotChecked=
	"\n"
	"\n"
	"UNCHECKED\n"
	"\n"
	"Currently this check button is not checked.\n"
;


//==============================================================================
//============================== emTkRadioButton ===============================
//==============================================================================

emTkRadioButton::emTkRadioButton(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emTkCheckButton(parent,name,caption,description,icon)
{
	Group * grp;

	SetShownRadioed(true);
	Mech=NULL;
	MechIndex=-1;
	if (GetParent()) {
		grp=dynamic_cast<emTkRadioButton::Group*>(GetParent());
		if (grp) grp->Add(this);
	}
}


emTkRadioButton::~emTkRadioButton()
{
	if (Mech) Mech->Remove(this);
}


emTkRadioButton::Mechanism::Mechanism()
{
	Array.SetTuningLevel(4);
	CheckIndex=-1;
}


emTkRadioButton::Mechanism::~Mechanism()
{
	RemoveAll();
}


void emTkRadioButton::Mechanism::Add(emTkRadioButton * radioButton)
{
	if (radioButton->Mech) radioButton->Mech->Remove(radioButton);
	radioButton->Mech=this;
	radioButton->MechIndex=Array.GetCount();
	Array.Add(radioButton);
	if (radioButton->IsChecked()) {
		if (CheckIndex>=0) {
			radioButton->SetChecked(false);
		}
		else {
			CheckIndex=Array.GetCount()-1;
			CheckSignal.Signal(radioButton->GetScheduler());
			CheckChanged();
		}
	}
}


void emTkRadioButton::Mechanism::AddAll(emPanel * parent)
{
	emTkRadioButton * rb;
	emPanel * p;

	for (p=parent->GetFirstChild(); p; p=p->GetNext()) {
		rb=dynamic_cast<emTkRadioButton*>(p);
		if (rb) Add(rb);
	}
}


void emTkRadioButton::Mechanism::Remove(emTkRadioButton * radioButton)
{
	RemoveByIndex(GetIndexOf(radioButton));
}


void emTkRadioButton::Mechanism::RemoveByIndex(int index)
{
	emScheduler * scheduler;
	emTkRadioButton * rb;
	int i;

	if (index>=0 && index<Array.GetCount()) {
		rb=Array[index];
		rb->Mech=NULL;
		rb->MechIndex=-1;
		scheduler=&rb->GetScheduler();
		Array.Remove(index);
		for (i=Array.GetCount()-1; i>=index; i--) Array[i]->MechIndex=i;
		if (CheckIndex>=index) {
			if (CheckIndex==index) CheckIndex=-1;
			else CheckIndex--;
			CheckSignal.Signal(*scheduler);
			CheckChanged();
		}
	}
}


void emTkRadioButton::Mechanism::RemoveAll()
{
	emScheduler * scheduler;
	emTkRadioButton * rb;
	int i;

	i=Array.GetCount()-1;
	if (i>=0) {
		scheduler=&Array[0]->GetScheduler();
		do {
			rb=Array[i];
			rb->Mech=NULL;
			rb->MechIndex=-1;
			i--;
		} while (i>=0);
		Array.Empty();
		if (CheckIndex>=0) {
			CheckIndex=-1;
			CheckSignal.Signal(*scheduler);
			CheckChanged();
		}
	}
}


void emTkRadioButton::Mechanism::SetChecked(emTkRadioButton * radioButton)
{
	SetCheckIndex(GetIndexOf(radioButton));
}


void emTkRadioButton::Mechanism::SetCheckIndex(int index)
{
	emScheduler * scheduler;
	int old;

	if (index<-1 || index>=Array.GetCount()) index=-1;
	if (CheckIndex!=index) {
		// Remember, this could be called recursively!
		scheduler=&Array[0]->GetScheduler();
		if (CheckIndex>=0 && Array[CheckIndex]->IsChecked()) {
			old=CheckIndex;
			CheckIndex=-1;
			Array[old]->SetChecked(false);
			if (CheckIndex!=-1) return;
		}
		CheckIndex=index;
		if (CheckIndex>=0 && !Array[CheckIndex]->IsChecked()) {
			Array[CheckIndex]->SetChecked(true);
			if (CheckIndex!=index) return;
		}
		CheckSignal.Signal(*scheduler);
		CheckChanged();
	}
}


void emTkRadioButton::Mechanism::CheckChanged()
{
}


emTkRadioButton::Group::Group(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emTkGroup(parent,name,caption,description,icon)
{
}


emTkRadioButton::Group::~Group()
{
}


void emTkRadioButton::Clicked()
{
	if (Mech) Mech->SetChecked(this);
}


void emTkRadioButton::CheckChanged()
{
	if (Mech) {
		if (IsChecked()) Mech->SetChecked(this);
		else if (Mech->GetChecked()==this) Mech->SetChecked(NULL);
	}
}


emString emTkRadioButton::GetHowTo()
{
	emString h;

	h=emTkCheckButton::GetHowTo();
	h+=HowToRadioButton;
	return h;
}


const char * emTkRadioButton::HowToRadioButton=
	"\n"
	"\n"
	"RADIO BUTTON\n"
	"\n"
	"This is a radio button. It is a check button with changed behavior: In a set of\n"
	"radio buttons, only one button can have checked state. When triggering a radio\n"
	"button, that button is checked and all the other radio buttons of the set are\n"
	"unchecked. There is no way to uncheck a radio button directly.\n"
;


//==============================================================================
//================================ emTkCheckBox ================================
//==============================================================================

emTkCheckBox::emTkCheckBox(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emTkCheckButton(parent,name,caption,description,icon)
{
	SetOuterBorderType(OBT_MARGIN);
	SetLabelAlignment(EM_ALIGN_LEFT);
	SetShownBoxed(true);
}


//==============================================================================
//================================ emTkRadioBox ================================
//==============================================================================

emTkRadioBox::emTkRadioBox(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emTkRadioButton(parent,name,caption,description,icon)
{
	SetOuterBorderType(OBT_MARGIN);
	SetLabelAlignment(EM_ALIGN_LEFT);
	SetShownBoxed(true);
}


//==============================================================================
//=============================== emTkTextField ================================
//==============================================================================

emTkTextField::emTkTextField(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon,
	const emString & text, bool editable
)
	: emTkBorder(parent,name,caption,description,icon)
{
	Clipboard=emClipboard::LookupInherited(GetView());
	if (!Clipboard) {
		emFatalError("emTkTextField: No emClipboard available.");
	}
	Editable=editable;
	MultiLineMode=false;
	PasswordMode=false;
	OverwriteMode=false;
	Text=text;
	TextLen=Text.GetLen();
	CursorIndex=TextLen;
	SelectionStartIndex=0;
	SelectionEndIndex=0;
	MagicCursorColumn=-1;
	SelectionId=-1;
	CursorBlinkTime=emGetClockMS();
	CursorBlinkOn=true;
	DragMode=DM_NONE;
	DragPosC=0.0;
	DragPosR=0.0;
	SetBorderType(OBT_INSTRUMENT,Editable?IBT_INPUT_FIELD:IBT_OUTPUT_FIELD);
}


emTkTextField::~emTkTextField()
{
}


void emTkTextField::SetEditable(bool editable)
{
	if (Editable!=editable) {
		Editable=editable;
		if (editable) {
			if (GetInnerBorderType()==IBT_OUTPUT_FIELD) {
				SetInnerBorderType(IBT_INPUT_FIELD);
			}
		}
		else {
			if (GetInnerBorderType()==IBT_INPUT_FIELD) {
				SetInnerBorderType(IBT_OUTPUT_FIELD);
			}
		}
		InvalidatePainting();
	}
}


void emTkTextField::SetMultiLineMode(bool multiLineMode)
{
	if (MultiLineMode!=multiLineMode) {
		MultiLineMode=multiLineMode;
		InvalidatePainting();
	}
}


void emTkTextField::SetPasswordMode(bool passwordMode)
{
	if (PasswordMode!=passwordMode) {
		PasswordMode=passwordMode;
		InvalidatePainting();
	}
}


void emTkTextField::SetOverwriteMode(bool overwriteMode)
{
	if (OverwriteMode!=overwriteMode) {
		OverwriteMode=overwriteMode;
		InvalidatePainting();
	}
}


void emTkTextField::SetText(const emString & text)
{
	if (Text==text) return;
	EmptySelection();
	Text=text;
	TextLen=Text.GetLen();
	CursorIndex=TextLen;
	MagicCursorColumn=-1;
	InvalidatePainting();
	Signal(TextSignal);
	TextChanged();
}


void emTkTextField::SetCursorIndex(int index)
{
	if (index<0) index=0;
	if (index>TextLen) index=TextLen;
	if (CursorIndex!=index) {
		index=GetNormalizedIndex(index);
		if (CursorIndex!=index) {
			CursorIndex=index;
			InvalidatePainting();
		}
	}
}


void emTkTextField::Select(int startIndex, int endIndex, bool publish)
{
	if (startIndex<0) startIndex=0;
	if (endIndex>TextLen) endIndex=TextLen;
	if (startIndex>=endIndex) {
		startIndex=0;
		endIndex=0;
	}
	if (SelectionStartIndex==startIndex && SelectionEndIndex==endIndex) return;
	startIndex=GetNormalizedIndex(startIndex);
	endIndex=GetNormalizedIndex(endIndex);
	if (SelectionStartIndex==startIndex && SelectionEndIndex==endIndex) return;
	if (SelectionId!=-1) {
		Clipboard->Clear(true,SelectionId);
		SelectionId=-1;
	}
	SelectionStartIndex=startIndex;
	SelectionEndIndex=endIndex;
	InvalidatePainting();
	if (publish) PublishSelection();
	Signal(SelectionSignal);
	SelectionChanged();
}


void emTkTextField::EmptySelection()
{
	Select(0,0,false);
}


void emTkTextField::SelectAll(bool publish)
{
	Select(0,TextLen,publish);
}


void emTkTextField::PublishSelection()
{
	emString str;
	int len;

	len=SelectionEndIndex-SelectionStartIndex;
	if (len>0 && SelectionId==-1) {
		if (PasswordMode) str=emString('*',len);
		else str=Text.GetSubString(SelectionStartIndex,len);
		SelectionId=Clipboard->PutText(str,true);
	}
}


void emTkTextField::CutSelectedTextToClipboard()
{
	CopySelectedTextToClipboard();
	DeleteSelectedText();
}


void emTkTextField::CopySelectedTextToClipboard()
{
	emString str;
	int len;

	len=SelectionEndIndex-SelectionStartIndex;
	if (len>0) {
		if (PasswordMode) str=emString('*',len);
		else str=Text.GetSubString(SelectionStartIndex,len);
		Clipboard->PutText(str);
	}
}


void emTkTextField::PasteSelectedTextFromClipboard()
{
	PasteSelectedText(Clipboard->GetText());
}


void emTkTextField::PasteSelectedText(const emString & text)
{
	int i,l,d;

	if (text.IsEmpty()) return;
	if (SelectionStartIndex<SelectionEndIndex) {
		i=SelectionStartIndex;
		l=SelectionEndIndex-SelectionStartIndex;
		d=TextLen-SelectionEndIndex;
		EmptySelection();
	}
	else {
		i=CursorIndex;
		l=0;
		d=TextLen-CursorIndex;
	}
	Text.Replace(i,l,text);
	TextLen=Text.GetLen();
	CursorIndex=TextLen-d;
	MagicCursorColumn=-1;
	InvalidatePainting();
	Signal(TextSignal);
	TextChanged();
}


void emTkTextField::DeleteSelectedText()
{
	int i,l;

	i=SelectionStartIndex;
	l=SelectionEndIndex-SelectionStartIndex;
	if (l<=0) return;
	CursorIndex=i;
	EmptySelection();
	Text.Remove(i,l);
	TextLen=Text.GetLen();
	MagicCursorColumn=-1;
	InvalidatePainting();
	Signal(TextSignal);
	TextChanged();
}


void emTkTextField::TextChanged()
{
}


void emTkTextField::SelectionChanged()
{
}


bool emTkTextField::Cycle()
{
	emUInt64 clk;
	bool busy;

	clk=emGetClockMS();
	busy=false;

	if (IsInFocusedPath()) {
		if (clk>=CursorBlinkTime+1000) {
			CursorBlinkTime=clk;
			if (!CursorBlinkOn) {
				CursorBlinkOn=true;
				InvalidatePainting();
			}
		}
		else if (clk>=CursorBlinkTime+500) {
			if (CursorBlinkOn) {
				CursorBlinkOn=false;
				InvalidatePainting();
			}
		}
		busy=true;
	}
	else {
		CursorBlinkTime=clk;
		if (!CursorBlinkOn) {
			CursorBlinkOn=true;
			InvalidatePainting();
		}
	}

	if (emTkBorder::Cycle()) busy=true;
	return busy;
}


void emTkTextField::Notice(NoticeFlags flags)
{
	if ((flags&(NF_FOCUS_CHANGED))!=0 && IsInFocusedPath()) {
		RestartCursorBlinking();
		WakeUp();
	}
	emTkBorder::Notice(flags);
}


void emTkTextField::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	static const double minExt=4.5;
	double mc,mr;
	int col,row,i,i1,i2,j1,j2;
	bool inArea;
	emString str;

	inArea=CheckMouse(mx,my,&mc,&mr);

	switch (DragMode) {
	case DM_NONE:
		if (
			inArea && event.IsKey(EM_KEY_LEFT_BUTTON) &&
			!state.GetAlt() && !state.GetMeta() &&
			GetViewCondition(VCT_MIN_EXT)>=minExt
		) {
			MagicCursorColumn=-1;
			if (state.GetCtrl()) {
				if (IsEditable() && IsEnabled()) {
					i=ColRow2Index(mc,mr,false);
					if (i<SelectionStartIndex || i>=SelectionEndIndex) {
						SetCursorIndex(ColRow2Index(mc,mr,true));
						SetDragMode(DM_INSERT);
					}
					else {
						SetCursorIndex(SelectionEndIndex);
						Index2ColRow(SelectionStartIndex,&col,&row);
						DragPosC=mc-col;
						DragPosR=mr-row;
						SetDragMode(DM_MOVE);
					}
				}
			}
			else if (event.GetRepeat()==0) {
				i=ColRow2Index(mc,mr,true);
				if (state.GetShift()) ModifySelection(i,i,false);
				else EmptySelection();
				SetCursorIndex(i);
				SetDragMode(DM_SELECT);
			}
			else if (event.GetRepeat()==1) {
				i2=GetNextWordBoundaryIndex(ColRow2Index(mc,mr,false));
				i1=GetPrevWordBoundaryIndex(i2);
				if (!state.GetShift() || IsSelectionEmpty()) {
					Select(i1,i2,false);
					SetCursorIndex(i2);
				}
				else if (i2>SelectionEndIndex) {
					ModifySelection(i2,i2,false);
					SetCursorIndex(i2);
				}
				else {
					ModifySelection(i1,i1,false);
					SetCursorIndex(i1);
				}
				SetDragMode(DM_SELECT_BY_WORDS);
			}
			else if (event.GetRepeat()==2) {
				i2=GetNextRowIndex(ColRow2Index(mc,mr,false));
				i1=GetPrevRowIndex(i2);
				if (!state.GetShift() || IsSelectionEmpty()) {
					Select(i1,i2,false);
					SetCursorIndex(i2);
				}
				else if (i2>SelectionEndIndex) {
					ModifySelection(i2,i2,false);
					SetCursorIndex(i2);
				}
				else {
					ModifySelection(i1,i1,false);
					SetCursorIndex(i1);
				}
				SetDragMode(DM_SELECT_BY_ROWS);
			}
			else {
				SelectAll(true);
				SetCursorIndex(TextLen);
			}
			RestartCursorBlinking();
			Focus();
			event.Eat();
		}
		break;
	case DM_SELECT:
		i=ColRow2Index(mc,mr,true);
		if (i!=CursorIndex) {
			MagicCursorColumn=-1;
			ModifySelection(CursorIndex,i,false);
			SetCursorIndex(i);
			RestartCursorBlinking();
		}
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			PublishSelection();
			SetDragMode(DM_NONE);
		}
		break;
	case DM_SELECT_BY_WORDS:
		i2=GetNextWordBoundaryIndex(ColRow2Index(mc,mr,false));
		i1=GetPrevWordBoundaryIndex(i2);
		if (IsSelectionEmpty()) {
			Select(i1,i2,false);
			SetCursorIndex(i2);
		}
		else {
			j1=SelectionStartIndex;
			j2=SelectionEndIndex;
			if (CursorIndex<=j1) j1=GetPrevWordBoundaryIndex(j2);
			else j2=GetNextWordBoundaryIndex(j1);
			if (j1<=i1) {
				Select(j1,i2,false);
				SetCursorIndex(i2);
			}
			else {
				Select(i1,j2,false);
				SetCursorIndex(i1);
			}
		}
		MagicCursorColumn=-1;
		RestartCursorBlinking();
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			PublishSelection();
			SetDragMode(DM_NONE);
		}
		break;
	case DM_SELECT_BY_ROWS:
		i2=GetNextRowIndex(ColRow2Index(mc,mr,false));
		i1=GetPrevRowIndex(i2);
		if (IsSelectionEmpty()) {
			Select(i1,i2,false);
			SetCursorIndex(i2);
		}
		else {
			j1=SelectionStartIndex;
			j2=SelectionEndIndex;
			if (CursorIndex<=j1) j1=GetPrevRowIndex(j2);
			else j2=GetNextRowIndex(j1);
			if (j1<=i1) {
				Select(j1,i2,false);
				SetCursorIndex(i2);
			}
			else {
				Select(i1,j2,false);
				SetCursorIndex(i1);
			}
		}
		MagicCursorColumn=-1;
		RestartCursorBlinking();
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			PublishSelection();
			SetDragMode(DM_NONE);
		}
		break;
	case DM_INSERT:
		i=ColRow2Index(mc,mr,true);
		if (i!=CursorIndex) {
			SetCursorIndex(i);
			MagicCursorColumn=-1;
			RestartCursorBlinking();
		}
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			if (inArea && IsEditable() && IsEnabled()) {
				SelectionId=-1;
				EmptySelection();
				PasteSelectedText(Clipboard->GetText(true));
			}
			SetDragMode(DM_NONE);
		}
		break;
	case DM_MOVE:
		// When extending this for moving the text to somewhere else,
		// don't forget to disable that in password mode.
		i1=SelectionStartIndex;
		i2=SelectionEndIndex;
		if (i1<i2 && IsEditable() && IsEnabled()) {
			str=Text.Extract(i1,i2-i1);
			TextLen=Text.GetLen();
			i=ColRow2Index(mc-DragPosC,mr-DragPosR+0.5,true);
			Text.Insert(i,str);
			TextLen=Text.GetLen();
			if (i!=i1) {
				SelectionStartIndex+=i-i1;
				SelectionEndIndex+=i-i1;
				CursorIndex=SelectionEndIndex;
				MagicCursorColumn=-1;
				RestartCursorBlinking();
				InvalidatePainting();
				Signal(TextSignal);
				TextChanged();
				Signal(SelectionSignal);
				SelectionChanged();
			}
		}
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			SetDragMode(DM_NONE);
		}
		break;
	}

	if (
		!event.IsEmpty() && DragMode==DM_NONE &&
		GetViewCondition(VCT_MIN_EXT)>=minExt
	) {
		if (event.IsKey(EM_KEY_CURSOR_LEFT) && !state.GetAlt() && !state.GetMeta()) {
			if (state.GetCtrl()) i=GetPrevWordIndex(CursorIndex);
			else i=GetPrevIndex(CursorIndex);
			if (state.GetShift()) ModifySelection(CursorIndex,i,true);
			else EmptySelection();
			SetCursorIndex(i);
			MagicCursorColumn=-1;
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_CURSOR_RIGHT) && !state.GetAlt() && !state.GetMeta()) {
			if (state.GetCtrl()) i=GetNextWordIndex(CursorIndex);
			else i=GetNextIndex(CursorIndex);
			if (state.GetShift()) ModifySelection(CursorIndex,i,true);
			else EmptySelection();
			SetCursorIndex(i);
			MagicCursorColumn=-1;
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (MultiLineMode) {
			if (event.IsKey(EM_KEY_CURSOR_UP) && !state.GetAlt() && !state.GetMeta()) {
				if (state.GetCtrl()) {
					i=GetPrevParagraphIndex(CursorIndex);
					MagicCursorColumn=-1;
				}
				else {
					Index2ColRow(CursorIndex,&col,&row);
					if (MagicCursorColumn<0) MagicCursorColumn=col;
					i=ColRow2Index(MagicCursorColumn,row-1,true);
				}
				if (state.GetShift()) ModifySelection(CursorIndex,i,true);
				else EmptySelection();
				SetCursorIndex(i);
				RestartCursorBlinking();
				ScrollToCursor();
				event.Eat();
			}
			if (event.IsKey(EM_KEY_CURSOR_DOWN) && !state.GetAlt() && !state.GetMeta()) {
				if (state.GetCtrl()) {
					i=GetNextParagraphIndex(CursorIndex);
					MagicCursorColumn=-1;
				}
				else {
					Index2ColRow(CursorIndex,&col,&row);
					if (MagicCursorColumn<0) MagicCursorColumn=col;
					i=ColRow2Index(MagicCursorColumn,row+1,true);
				}
				if (state.GetShift()) ModifySelection(CursorIndex,i,true);
				else EmptySelection();
				SetCursorIndex(i);
				RestartCursorBlinking();
				ScrollToCursor();
				event.Eat();
			}
		}
		if (event.IsKey(EM_KEY_HOME) && !state.GetAlt() && !state.GetMeta()) {
			if (state.GetCtrl()) i=0;
			else i=GetRowStartIndex(CursorIndex);
			if (state.GetShift()) ModifySelection(CursorIndex,i,true);
			else EmptySelection();
			SetCursorIndex(i);
			MagicCursorColumn=-1;
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_END) && !state.GetAlt() && !state.GetMeta()) {
			if (state.GetCtrl()) i=TextLen;
			else i=GetRowEndIndex(CursorIndex);
			if (state.GetShift()) ModifySelection(CursorIndex,i,true);
			else EmptySelection();
			SetCursorIndex(i);
			MagicCursorColumn=-1;
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_A) && state.IsCtrlMod()) {
			SelectAll(true);
			SetCursorIndex(TextLen);
			MagicCursorColumn=-1;
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_INSERT) && state.IsNoMod()) {
			SetOverwriteMode(!GetOverwriteMode());
			RestartCursorBlinking();
			event.Eat();
		}
		if ((event.IsKey(EM_KEY_INSERT) && state.IsCtrlMod()) ||
		    (event.IsKey(EM_KEY_C) && state.IsCtrlMod())) {
			CopySelectedTextToClipboard();
			RestartCursorBlinking();
			event.Eat();
		}
	}

	if (
		!event.IsEmpty() && DragMode==DM_NONE &&
		IsEditable() && IsEnabled() &&
		GetViewCondition(VCT_MIN_EXT)>=minExt
	) {
		if (event.IsKey(EM_KEY_BACKSPACE) && state.IsNoMod()) {
			if (IsSelectionEmpty()) {
				Select(GetPrevIndex(CursorIndex),CursorIndex,false);
			}
			DeleteSelectedText();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_DELETE) && state.IsNoMod()) {
			if (IsSelectionEmpty()) {
				Select(CursorIndex,GetNextIndex(CursorIndex),false);
			}
			DeleteSelectedText();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if ((event.IsKey(EM_KEY_DELETE) && state.IsShiftMod()) ||
		    (event.IsKey(EM_KEY_X) && state.IsCtrlMod())) {
			CutSelectedTextToClipboard();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if ((event.IsKey(EM_KEY_INSERT) && state.IsShiftMod()) ||
		    (event.IsKey(EM_KEY_V) && state.IsCtrlMod())) {
			PasteSelectedTextFromClipboard();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_BACKSPACE) && state.IsCtrlMod()) {
			if (IsSelectionEmpty()) {
				Select(GetPrevWordIndex(CursorIndex),CursorIndex,false);
			}
			DeleteSelectedText();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_DELETE) && state.IsCtrlMod()) {
			if (IsSelectionEmpty()) {
				Select(
					CursorIndex,
					GetNextWordIndex(CursorIndex),
					false
				);
			}
			DeleteSelectedText();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_BACKSPACE) && state.IsShiftCtrlMod()) {
			if (IsSelectionEmpty()) {
				Select(GetRowStartIndex(CursorIndex),CursorIndex,false);
			}
			DeleteSelectedText();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (event.IsKey(EM_KEY_DELETE) && state.IsShiftCtrlMod()) {
			if (IsSelectionEmpty()) {
				Select(CursorIndex,GetRowEndIndex(CursorIndex),false);
			}
			DeleteSelectedText();
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (MultiLineMode && event.IsKey(EM_KEY_ENTER) && state.IsNoMod()) {
			PasteSelectedText("\n");
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
		if (
			!event.GetChars().IsEmpty() &&
			(unsigned char)(event.GetChars()[0])>=32 &&
			event.GetChars()[0]!=127
		) {
			if (
				OverwriteMode && IsSelectionEmpty() &&
				CursorIndex<GetRowEndIndex(CursorIndex)
			) {
				Select(CursorIndex,GetNextIndex(CursorIndex),false);
			}
			PasteSelectedText(event.GetChars());
			RestartCursorBlinking();
			ScrollToCursor();
			event.Eat();
		}
	}

	emTkBorder::Input(event,state,mx,my);
}


bool emTkTextField::HasHowTo()
{
	return true;
}


emString emTkTextField::GetHowTo()
{
	emString h;

	h=emTkBorder::GetHowTo();
	h+=HowToTextField;
	if (MultiLineMode) h+=HowToMultiLineOn; else h+=HowToMultiLineOff;
	if (!IsEditable()) h+=HowToReadOnly;
	return h;
}


void emTkTextField::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
)
{
	DoTextField(TEXT_FIELD_FUNC_PAINT,&painter,canvasColor,0.0,0.0,NULL,NULL,NULL);
}


bool emTkTextField::CheckMouse(double mx, double my, double * pCol, double * pRow)
{
	bool b;

	DoTextField(TEXT_FIELD_FUNC_XY2CR,NULL,0,mx,my,pCol,pRow,&b);
	return b;
}


void emTkTextField::DoTextField(
	DoTextFieldFunc func, const emPainter * painter, emColor canvasColor,
	double xIn, double yIn, double * pXOut, double * pYOut, bool * pHit
)
{
	emColor bgColor,fgColor,hlColor,selColor,curColor;
	emString txt;
	double xy[10*2];
	double x,y,w,h,r,ws,d,d1,d2,cx,cy,cw,ch,dx,dy,tx,ty,tw,th;
	int i,i0,j,n,c,rows,cols,col,row,col0,row0,selIdx,selEnd;
	bool selected,selected0;

	GetContentRoundRect(&x,&y,&w,&h,&r);

	d=emMin(h,w)*0.1+r*0.5;
	tx=x+d;
	ty=y+d;
	tw=w-2*d;
	th=h-2*d;

	CalcTotalColsRows(&cols,&rows);
	if (OverwriteMode && IsInFocusedPath()) {
		Index2ColRow(CursorIndex,&col,&row);
		if (col==cols) cols++;
	}
	ch=th/rows;
	cw=emPainter::GetTextSize("X",ch,false);

	ws=1.0;
	if (cw*cols>tw) {
		ws=tw/(cw*cols);
		cw=tw/cols;
		d=0.66;
		if (ws<d) {
			ty+=(ch-ch*ws/d)*0.5;
			ch-=ch-ch*ws/d;
			ws=d;
		}
	}

	if (func!=TEXT_FIELD_FUNC_PAINT) {
		if (func==TEXT_FIELD_FUNC_XY2CR) {
			*pXOut=(xIn-tx)/cw;
			*pYOut=(yIn-ty)/ch;
		}
		else {
			xIn=tx+xIn*cw;
			yIn=ty+yIn*ch;
			*pXOut=xIn;
			*pYOut=yIn;
		}
		dx=emMax(emMax(x-xIn,xIn-x-w)+r,0.0);
		dy=emMax(emMax(y-yIn,yIn-y-h)+r,0.0);
		*pHit = dx*dx+dy*dy <= r*r;
		return;
	}

	if (IsEditable()) {
		bgColor=GetLook().GetInputBgColor();
		fgColor=GetLook().GetInputFgColor();
		hlColor=GetLook().GetInputHlColor();
	}
	else {
		bgColor=GetLook().GetOutputBgColor();
		fgColor=GetLook().GetOutputFgColor();
		hlColor=GetLook().GetOutputHlColor();
	}

	selColor=hlColor;
	selIdx=GetSelectionStartIndex();
	selEnd=GetSelectionEndIndex();
	if (selIdx<selEnd) {
		if (!IsInFocusedPath()) {
			selColor=bgColor.GetBlended(fgColor,40.0);
		}
		Index2ColRow(selIdx,&col0,&row0);
		Index2ColRow(selEnd,&col,&row);
		xy[ 0]=tx+col0*cw; xy[ 1]=ty+row0*ch;
		xy[ 2]=tx+tw;      xy[ 3]=ty+row0*ch;
		xy[ 4]=tx+tw;      xy[ 5]=ty+row*ch;
		xy[ 6]=tx+col*cw;  xy[ 7]=ty+row*ch;
		xy[ 8]=tx+col*cw;  xy[ 9]=ty+row*ch+ch;
		xy[10]=tx;         xy[11]=ty+row*ch+ch;
		xy[12]=tx;         xy[13]=ty+row0*ch+ch;
		xy[14]=tx+col0*cw; xy[15]=ty+row0*ch+ch;
		painter->PaintPolygon(xy,8,selColor,canvasColor);
	}

	row0=0;
	col0=0;
	i0=0;
	selected0=(i0>=selIdx && i0<selEnd);
	row=0;
	col=0;
	for (i=0;;) {
		n=emDecodeChar(&c,Text.Get()+i);
		selected=(i>=selIdx && i<selEnd);
		if (
			(
				c<=0x0d &&
				(c==0 || (MultiLineMode && (c==0x09 || c==0x0d || c==0x0a)))
			) ||
			selected0!=selected
		) {
			if (i0<i) {
				if (PasswordMode) {
					for (j=0; j<col-col0; j++) {
						painter->PaintText(
							tx+(col0+j)*cw,ty+row0*ch,"*",ch,ws,
							selected0 ? bgColor : fgColor,
							selected0 ? selColor : canvasColor
						);
					}
				}
				else {
					painter->PaintText(
						tx+col0*cw,ty+row0*ch,Text.Get()+i0,ch,ws,
						selected0 ? bgColor : fgColor,
						selected0 ? selColor : canvasColor,
						i-i0
					);
				}
			}
			if (c==0) break;
			row0=row;
			col0=col;
			i0=i;
			selected0=selected;
		}
		i+=n;
		col++;
		if (c<=0x0d && MultiLineMode) {
			if (c==0x09) {
				col=(col+7)&~7;
				col0=col;
				i0=i;
				selected0=(i0>=selIdx && i0<selEnd);
			}
			else if (c==0x0a || c==0x0d) {
				if (c==0x0d && Text[i]==0x0a) i++;
				col=0;
				row++;
				row0=row;
				col0=col;
				i0=i;
				selected0=(i0>=selIdx && i0<selEnd);
			}
		}
	}

	if (IsInFocusedPath()) {
		curColor=fgColor;
		if (!IsEditable()) curColor=curColor.GetTransparented(75.0F);
		else if (!IsCursorBlinkOn()) curColor=curColor.GetTransparented(88.0F);
		Index2ColRow(CursorIndex,&col,&row);
		cx=tx+cw*col;
		cy=ty+ch*row;
		if (GetOverwriteMode()) {
			d=ch*0.07;
			xy[ 0]=cx-d;    xy[ 1]=cy-d;
			xy[ 2]=cx+cw+d; xy[ 3]=cy-d;
			xy[ 4]=cx+cw+d; xy[ 5]=cy+ch+d;
			xy[ 6]=cx-d;    xy[ 7]=cy+ch+d;
			xy[ 8]=cx-d;    xy[ 9]=cy-d;
			xy[10]=cx;      xy[11]=cy;
			xy[12]=cx;      xy[13]=cy+ch;
			xy[14]=cx+cw;   xy[15]=cy+ch;
			xy[16]=cx+cw;   xy[17]=cy;
			xy[18]=cx;      xy[19]=cy;
			painter->PaintPolygon(xy,10,curColor);
		}
		else {
			d=ch*0.07;
			d1=d*0.5;
			d2=d*2.2;
			xy[ 0]=cx-d2; xy[ 1]=cy-d;
			xy[ 2]=cx+d2; xy[ 3]=cy-d;
			xy[ 4]=cx+d1; xy[ 5]=cy;
			xy[ 6]=cx+d1; xy[ 7]=cy+ch;
			xy[ 8]=cx+d2; xy[ 9]=cy+ch+d;
			xy[10]=cx-d2; xy[11]=cy+ch+d;
			xy[12]=cx-d1; xy[13]=cy+ch;
			xy[14]=cx-d1; xy[15]=cy;
			painter->PaintPolygon(xy,8,curColor);
		}
	}

	if (!IsEnabled()) {
		painter->PaintRoundRect(
			x,y,w,h,r,r,
			GetLook().GetBgColor().GetTransparented(20.0F)
		);
	}
}


void emTkTextField::SetDragMode(DragModeType dragMode)
{
	if (DragMode!=dragMode) {
		DragMode=dragMode;
		InvalidatePainting();
		RestartCursorBlinking();
	}
}


void emTkTextField::RestartCursorBlinking()
{
	CursorBlinkTime=emGetClockMS();
	if (!CursorBlinkOn) {
		CursorBlinkOn=true;
		InvalidatePainting();
	}
}


void emTkTextField::ScrollToCursor()
{
	int col,row;
	double x1,y1,x2,y2,dx,dy;
	bool b;

	if (!IsViewed() || !IsActive()) return;
	Index2ColRow(CursorIndex,&col,&row);
	DoTextField(TEXT_FIELD_FUNC_CR2XY,NULL,0,col-0.5,row-0.2,&x1,&y1,&b);
	DoTextField(TEXT_FIELD_FUNC_CR2XY,NULL,0,col+0.5,row+1.2,&x2,&y2,&b);
	b=false;
	dx=PanelToViewX(x1)-GetView().GetCurrentX();
	if (dx<0.0) b=true;
	else {
		dx=PanelToViewX(x2)-GetView().GetCurrentX()-GetView().GetCurrentWidth();
		if (dx>0.0) b=true; else dx=0.0;
	}
	dy=PanelToViewY(y1)-GetView().GetCurrentY();
	if (dy<0.0) b=true;
	else {
		dy=PanelToViewY(y2)-GetView().GetCurrentY()-GetView().GetCurrentHeight();
		if (dy>0.0) b=true; else dy=0.0;
	}
	if (b) {
		GetView().Scroll(dx,dy);
		if (!IsActive()) Activate();
	}
}


int emTkTextField::ColRow2Index(double column, double row, bool forCursor) const
{
	int i,j,k,n,c;

	if (!MultiLineMode) {
		for (i=0; ; i+=n, column-=1.0) {
			if (forCursor) { if (column<0.5) break; }
			else           { if (column<1.0) break; }
			n=emDecodeChar(&c,Text.Get()+i);
			if (c==0) break;
		}
	}
	else {
		for (j=0, i=0; row>=1.0; ) {
			j+=emDecodeChar(&c,Text.Get()+j);
			if (c==0x0a || c==0x0d) {
				if (c==0x0d && Text[j]==0x0a) j++;
				i=j;
				row-=1.0;
			}
			if (c==0) break;
		}
		for (j=0; ; i+=n, j=k) {
			n=emDecodeChar(&c,Text.Get()+i);
			if (c==0x0a || c==0x0d || c==0) break;
			k=j+1;
			if (c==0x09) k=(k+7)&~7;
			if (column<=k) {
				if (forCursor) { if (k-column<column-j) i+=n; }
				else           { if (column==k)         i+=n; }
				break;
			}
		}
	}
	return i;
}


void emTkTextField::Index2ColRow(int index, int * pColumn, int * pRow) const
{
	int i,col,row,n,c;

	if (!MultiLineMode) {
		col=emGetDecodedCharCount(Text,index);
		row=0;
	}
	else {
		col=0;
		row=0;
		for (i=0; i<index; i+=n) {
			n=emDecodeChar(&c,Text.Get()+i);
			if (c==0x09) {
				col=(col+8)&~7;
			}
			else if (c==0x0a || c==0x0d) {
				if (c==0x0d && Text[i+1]==0x0a) n++;
				col=0;
				row++;
			}
			else if (c==0) {
				break;
			}
			else {
				col++;
			}
		}
	}
	*pColumn=col;
	*pRow=row;
}


void emTkTextField::CalcTotalColsRows(int * pCols, int * pRows) const
{
	int i,cols,rows,rowcols,n,c;

	if (!MultiLineMode) {
		cols=emGetDecodedCharCount(Text);
		rows=1;
	}
	else {
		cols=0;
		rows=1;
		rowcols=0;
		for (i=0; ; i+=n) {
			n=emDecodeChar(&c,Text.Get()+i);
			if (c==0x09) {
				rowcols=(rowcols+8)&~7;
			}
			else if (c==0x0a || c==0x0d) {
				if (cols<rowcols) cols=rowcols;
				if (c==0x0d && Text[i+1]==0x0a) n++;
				rowcols=0;
				rows++;
			}
			else if (c==0) {
				break;
			}
			else {
				rowcols++;
			}
		}
		if (cols<rowcols) cols=rowcols;
	}

	if (cols<1) cols=1;
	if (rows<1) rows=1;
	*pCols=cols;
	*pRows=rows;
}


int emTkTextField::GetNormalizedIndex(int index) const
{
	int i,j;

	for (i=0; ; i=j) {
		j=GetNextIndex(i);
		if (j>index || j==i) return i;
	}
}


void emTkTextField::ModifySelection(int oldIndex, int newIndex, bool publish)
{
	int d1,d2;

	if (SelectionStartIndex<SelectionEndIndex) {
		d1=oldIndex-SelectionStartIndex; if (d1<0) d1=-d1;
		d2=oldIndex-SelectionEndIndex; if (d2<0) d2=-d2;
		if (d1<d2) oldIndex=SelectionEndIndex;
		else oldIndex=SelectionStartIndex;
	}
	if (oldIndex<newIndex) Select(oldIndex,newIndex,publish);
	else Select(newIndex,oldIndex,publish);
}


int emTkTextField::GetNextIndex(int index) const
{
	int c;

	index+=emDecodeChar(&c,Text.Get()+index);
	if (c==0x0d && Text[index]==0x0a && MultiLineMode) index++;
	return index;
}


int emTkTextField::GetPrevIndex(int index) const
{
	int i,j;

	for (i=0; ; i=j) {
		j=GetNextIndex(i);
		if (j>=index || j==i) return i;
	}
}


int emTkTextField::GetNextWordBoundaryIndex(
	int index, bool * pIsDelimiter
) const
{
	const char * p;
	int i,n,c;
	bool prevDelim,delim,first;

	p=Text.Get();
	i=index;
	delim=false;
	first=true;
	for (;;) {
		n=emDecodeChar(&c,p+i);
		if (n<=0) {
			delim=true;
			break;
		}
		prevDelim=delim;
		if (
			PasswordMode ||
			(c>='0' && c<='9') ||
			(c>='A' && c<='Z') ||
			(c>='a' && c<='z') ||
			c=='_' ||
			c>127
		) {
			delim=false;
		}
		else {
			delim=true;
		}
		if (!first && delim!=prevDelim) break;
		i+=n;
		first=false;
	}
	if (pIsDelimiter) *pIsDelimiter=delim;
	return i;
}


int emTkTextField::GetPrevWordBoundaryIndex(
	int index, bool * pIsDelimiter
) const
{
	int i,j;

	for (i=0; ; i=j) {
		j=GetNextWordBoundaryIndex(i,pIsDelimiter);
		if (j>=index || j==i) return i;
	}
}


int emTkTextField::GetNextWordIndex(int index) const
{
	bool isDelim;

	for (;;) {
		index=GetNextWordBoundaryIndex(index,&isDelim);
		if (!isDelim || index>=TextLen) break;
	}
	return index;
}


int emTkTextField::GetPrevWordIndex(int index) const
{
	int i,j;

	for (i=0; ; i=j) {
		j=GetNextWordIndex(i);
		if (j>=index || j==i) return i;
	}
}


int emTkTextField::GetRowStartIndex(int index) const
{
	int i,j,c;

	if (!MultiLineMode) return 0;
	for (i=0, j=0; ; ) {
		i+=emDecodeChar(&c,Text.Get()+i);
		if (c==0x0d && Text[i]==0x0a) i++;
		if (c==0 || i>index) return j;
		if (c==0x0a || c==0x0d) j=i;
	}
}


int emTkTextField::GetRowEndIndex(int index) const
{
	int n,c;

	if (!MultiLineMode) return TextLen;
	for (;; index+=n) {
		n=emDecodeChar(&c,Text.Get()+index);
		if (c==0 || c==0x0a || c==0x0d) return index;
	}
}


int emTkTextField::GetNextRowIndex(int index) const
{
	int c;

	if (!MultiLineMode) return TextLen;
	for (;;) {
		index+=emDecodeChar(&c,Text.Get()+index);
		if (c==0 || c==0x0a || c==0x0d) {
			if (c==0x0d && Text[index]==0x0a) index++;
			return index;
		}
	}
}


int emTkTextField::GetPrevRowIndex(int index) const
{
	int i,j;

	for (i=0; ; i=j) {
		j=GetNextRowIndex(i);
		if (j>=index || j==i) return i;
	}
}


int emTkTextField::GetNextParagraphIndex(int index) const
{
	bool e;

	if (!MultiLineMode) return TextLen;
	for (e=false; index<TextLen; ) {
		index=GetNextRowIndex(index);
		if (Text[index]==0x0a || Text[index]==0x0d) e=true;
		else if (e) break;
	}
	return index;
}


int emTkTextField::GetPrevParagraphIndex(int index) const
{
	int i,j;

	for (i=0; ; i=j) {
		j=GetNextParagraphIndex(i);
		if (j>=index || j==i) return i;
	}
}


const char * emTkTextField::HowToTextField=
	"\n"
	"\n"
	"TEXT FIELD\n"
	"\n"
	"This is a text field. In such a field, a text can be viewed and edited.\n"
	"\n"
	"Quick hint about an incompatibility against other user interfaces: For inserting\n"
	"selected text, press Ctrl + left mouse button instead of the middle mouse\n"
	"button.\n"
	"\n"
	"Mouse control:\n"
	"\n"
	"  Left-Button-Click        - Set cursor position, clear selection.\n"
	"\n"
	"  Left-Button-Double-Click - Select a word.\n"
	"\n"
	"  Left-Button-Triple-Click - Select a row.\n"
	"\n"
	"  Left-Button-Quad-Click   - Select all.\n"
	"\n"
	"  Left-Button-Drag         - Select passed characters.\n"
	"\n"
	"  Shift+Left-Button-Drag   - Extend or reduce selection by passed characters.\n"
	"\n"
	"  Ctrl+Left-Button-Click on non-selected area - Insert a copy of common selected\n"
	"                                                text.\n"
	"\n"
	"  Ctrl+Left-Button-Drag on selected area      - Move selected text.\n"
	"\n"
	"\n"
	"Keyboard control:\n"
	"\n"
	"  Normal key input inserts the corresponding character at the cursor position.\n"
	"  Any selected text is replaced by the character. Special key combinations are:\n"
	"\n"
	"  Cursor-Keys             - Move the cursor.\n"
	"\n"
	"  Ctrl+Cursor-Keys        - Move the cursor by words or paragraphs.\n"
	"\n"
	"  Home or End             - Move the cursor to beginning or end of row.\n"
	"\n"
	"  Ctrl+Home or Ctrl+End   - Move the cursor to beginning or end of all.\n"
	"\n"
	"  Shift+<Cursor Movement> - Select text: Hold the Shift key while moving the\n"
	"                            cursor with one of the above key combinations, to\n"
	"                            select the passed characters.\n"
	"\n"
	"  Ctrl+A                  - Select the whole text.\n"
	"\n"
	"  Insert                  - Switch between insert mode and replace mode.\n"
	"\n"
	"  Backspace               - Delete the selected text, or the character on the\n"
	"                            left side of the cursor.\n"
	"\n"
	"  Delete                  - Delete the selected text, or the character on the\n"
	"                            right side of the cursor.\n"
	"\n"
	"  Shift+Delete or Ctrl+X  - Cut operation: Copy the selected text to the\n"
	"                            clipboard and delete it.\n"
	"\n"
	"  Ctrl+Insert or Ctrl+C   - Copy operation: Copy the selected text to the\n"
	"                            clipboard.\n"
	"\n"
	"  Shift+Insert or Ctrl+V  - Paste operation: Insert text from the clipboard. Any\n"
	"                            selected text is replaced by the insertion.\n"
	"\n"
	"  Ctrl+Backspace          - Delete to the left until beginning of a word.\n"
	"\n"
	"  Ctrl+Delete             - Delete to the right until beginning of a word.\n"
	"\n"
	"  Shift+Ctrl+Backspace    - Delete all on the left side of the cursor.\n"
	"\n"
	"  Shift+Ctrl+Delete       - Delete all on the right side of the cursor.\n"
;


const char * emTkTextField::HowToMultiLineOff=
	"\n"
	"\n"
	"MULTI-LINE: DISABLED\n"
	"\n"
	"This text field has the multi-line mode disabled. You can view or edit only\n"
	"a single line.\n"
;


const char * emTkTextField::HowToMultiLineOn=
	"\n"
	"\n"
	"MULTI-LINE: ENABLED\n"
	"\n"
	"This text field has the multi-line mode enabled. You may view or edit multiple\n"
	"lines.\n"
;


const char * emTkTextField::HowToReadOnly=
	"\n"
	"\n"
	"READ-ONLY\n"
	"\n"
	"This text field is read-only. You cannot edit the text.\n"
;


//==============================================================================
//============================== emTkScalarField ===============================
//==============================================================================

emTkScalarField::emTkScalarField(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon, emInt64 minValue,
	emInt64 maxValue, emInt64 value, bool editable
)
	: emTkBorder(parent,name,caption,description,icon)
{
	Editable=editable;
	MinValue=minValue;
	if (maxValue<minValue) maxValue=minValue;
	MaxValue=maxValue;
	if (value<minValue) value=minValue;
	if (value>maxValue) value=maxValue;
	Value=value;
	ScaleMarkIntervals.SetTuningLevel(4);
	ScaleMarkIntervals.Add(1);
	MarksNeverHidden=false;
	TextOfValueFunc=DefaultTextOfValue;
	TextOfValueFuncContext=NULL;
	TextBoxTallness=0.5;
	KBInterval=0;
	Pressed=false;
	SetBorderType(OBT_INSTRUMENT,Editable?IBT_INPUT_FIELD:IBT_OUTPUT_FIELD);
}


emTkScalarField::~emTkScalarField()
{
}


void emTkScalarField::SetEditable(bool editable)
{
	if (Editable!=editable) {
		Editable=editable;
		if (editable) {
			if (GetInnerBorderType()==IBT_OUTPUT_FIELD) {
				SetInnerBorderType(IBT_INPUT_FIELD);
			}
		}
		else {
			if (GetInnerBorderType()==IBT_INPUT_FIELD) {
				SetInnerBorderType(IBT_OUTPUT_FIELD);
			}
		}
		InvalidatePainting();
	}
}


void emTkScalarField::SetMinValue(emInt64 minValue)
{
	if (MinValue!=minValue) {
		MinValue=minValue;
		if (MaxValue<MinValue) MaxValue=MinValue;
		InvalidatePainting();
		if (Value<MinValue) SetValue(MinValue);
	}
}


void emTkScalarField::SetMaxValue(emInt64 maxValue)
{
	if (MaxValue!=maxValue) {
		MaxValue=maxValue;
		if (MinValue>MaxValue) MinValue=MaxValue;
		InvalidatePainting();
		if (Value>MaxValue) SetValue(MaxValue);
	}
}


void emTkScalarField::SetMinMaxValues(emInt64 minValue, emInt64 maxValue)
{
	SetMinValue(minValue);
	SetMaxValue(maxValue);
}


void emTkScalarField::SetValue(emInt64 value)
{
	if (value<MinValue) value=MinValue;
	if (value>MaxValue) value=MaxValue;
	if (Value!=value) {
		Value=value;
		InvalidatePainting();
		Signal(ValueSignal);
		ValueChanged();
	}
}


void emTkScalarField::SetScaleMarkIntervals(const emArray<emUInt64> & intervals)
{
	int i;

	for (i=0; i<intervals.GetCount(); i++) {
		if (intervals[i]==0 || (i>0 && intervals[i]>=intervals[i-1])) {
			emFatalError("emTkScalarField::SetScaleMarkIntervals: Illegal argument.");
		}
	}
	if (ScaleMarkIntervals.GetCount()==intervals.GetCount()) {
		for (i=intervals.GetCount()-1; i>=0; i--) {
			if (ScaleMarkIntervals[i]!=intervals[i]) break;
		}
		if (i<0) return;
	}
	ScaleMarkIntervals=intervals;
	InvalidatePainting();
}


void emTkScalarField::SetScaleMarkIntervals(
	unsigned interval1, unsigned interval2, ...
)
{
	emArray<emUInt64> intervals;
	va_list ap;
	unsigned u;

	intervals.SetTuningLevel(4);
	if (interval1) {
		intervals.Add((emUInt64)interval1);
		if (interval2) {
			intervals.Add((emUInt64)interval2);
			va_start(ap,interval2);
			for (;;) {
				u=va_arg(ap,unsigned);
				if (!u) break;
				intervals.Add((emUInt64)u);
			}
			va_end(ap);
		}
	}
	SetScaleMarkIntervals(intervals);
}


void emTkScalarField::SetNeverHideMarks(bool neverHide)
{
	if (MarksNeverHidden!=neverHide) {
		MarksNeverHidden=neverHide;
		InvalidatePainting();
	}
}


void emTkScalarField::TextOfValue(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval
) const
{
	TextOfValueFunc(buf,bufSize,value,markInterval,TextOfValueFuncContext);
}


void emTkScalarField::SetTextOfValueFunc(
	void(*textOfValueFunc)(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	),
	void * context
)
{
	if (
		TextOfValueFunc!=textOfValueFunc ||
		TextOfValueFuncContext!=context
	) {
		TextOfValueFunc=textOfValueFunc;
		TextOfValueFuncContext=context;
		InvalidatePainting();
	}
}


void emTkScalarField::DefaultTextOfValue(
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	int l;

	l=emInt64ToStr(buf,bufSize,value);
	if (l<0 || l>=bufSize) l=0;
	buf[l]=0;
}


void emTkScalarField::SetTextBoxTallness(double textBoxTallness)
{
	if (TextBoxTallness!=textBoxTallness) {
		TextBoxTallness=textBoxTallness;
		InvalidatePainting();
	}
}


void emTkScalarField::SetKeyboardInterval(emUInt64 kbInterval)
{
	KBInterval=kbInterval;
}


void emTkScalarField::ValueChanged()
{
}


void emTkScalarField::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	bool inArea;
	emInt64 mv;

	inArea=CheckMouse(mx,my,&mv);

	if (Pressed) {
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			Pressed=false;
			InvalidatePainting();
		}
		if (Value!=mv && IsEditable() && IsEnabled()) {
			SetValue(mv);
		}
	}
	else if (
		inArea && event.IsKey(EM_KEY_LEFT_BUTTON) && IsEditable() &&
		IsEnabled() && GetViewCondition(VCT_MIN_EXT)>=4.0
	) {
		Pressed=true;
		InvalidatePainting();
		if (Value!=mv) {
			SetValue(mv);
		}
	}
	else if (strcmp(event.GetChars(),"+")==0 && IsEditable() && IsEnabled()) {
		StepByKeyboard(1);
		event.Eat();
	}
	else if (strcmp(event.GetChars(),"-")==0 && IsEditable() && IsEnabled()) {
		StepByKeyboard(-1);
		event.Eat();
	}

	emTkBorder::Input(event,state,mx,my);
}


bool emTkScalarField::HasHowTo()
{
	return true;
}


emString emTkScalarField::GetHowTo()
{
	emString h;

	h=emTkBorder::GetHowTo();
	h+=HowToScalarField;
	if (!IsEditable()) h+=HowToReadOnly;
	return h;
}


void emTkScalarField::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
)
{
	DoScalarField(
		SCALAR_FIELD_FUNC_PAINT,&painter,canvasColor,0.0,0.0,NULL,NULL
	);
}


bool emTkScalarField::CheckMouse(
	double mx, double my, emInt64 * pValue
)
{
	bool b;

	DoScalarField(SCALAR_FIELD_FUNC_CHECK_MOUSE,NULL,0,mx,my,pValue,&b);
	return b;
}


void emTkScalarField::DoScalarField(
	DoScalarFieldFunc func, const emPainter * painter, emColor canvasColor,
	double mx, double my, emInt64 * pValue, bool * pHit
)
{
	emColor bgCol,fgCol,col;
	double x,y,w,h,r,tx,ty,tw,th,s,d,e,f,ax,ay,aw,ah,dx,dy;
	double rx,ry,rw,rh,x3,w3,h4,h5,mw,mtw,mth,mah;
	char buf[256];
	double xy[5*2];
	const emUInt64 * ivals;
	emUInt64 ivalSum,vRange;
	emInt64 v,k1,k2;
	int ivalCnt,l;

	GetContentRoundRect(&x,&y,&w,&h,&r);

	vRange=GetMaxValue()-GetMinValue();
	ivals=GetScaleMarkIntervals().Get();
	ivalCnt=GetScaleMarkIntervals().GetCount();
	if (!IsNeverHidingMarks()) {
		while (ivalCnt>1 && ivals[0]>vRange) { ivals++; ivalCnt--; }
	}
	ivalSum=0;
	for (l=0; l<ivalCnt; l++) ivalSum+=ivals[l];

	mtw=1.0;
	mth=GetTextBoxTallness();
	mah=emMin(mtw,mth)*0.5;
	d=1.0/(mth+mah);
	mtw*=d;
	mth*=d;
	mah*=d;
	mw=mtw*1.5;

	rx=x+r*0.5;
	ry=y+r*0.5;
	rw=w-r;
	rh=h-r;

	s=emMin(rh,rw);
	d=s*0.04;
	ax=rx+d;
	ay=ry+d;
	aw=rw-2*d;
	ah=rh-2*d;

	e=s*0.3*0.5;
	d=e-d;
	if (d<0.0) d=0.0;
	if (ivalCnt>0 && vRange) {
		th=ah;
		f=th*ivals[0]/ivalSum;
		tw=f*mw*vRange/ivals[0];
		f*=mtw;
		if (tw+f>aw) f*=aw/(tw+f);
		f*=0.5;
		if (d<f) d=f;
		f=aw*0.2;
		if (d>f) d=f;
		if (tw>aw-2*d) th*=(aw-2*d)/tw;
		ay+=ah-th;
		ah=th;
	}
	ax+=d;
	aw-=2*d;

	if (func==SCALAR_FIELD_FUNC_CHECK_MOUSE) {
		dx=emMax(emMax(x-mx,mx-x-w)+r,0.0);
		dy=emMax(emMax(y-my,my-y-h)+r,0.0);
		*pHit = dx*dx+dy*dy <= r*r;
		d=(mx-ax)/aw;
		d=d*vRange+GetMinValue();
		if (d<(double)GetMinValue()) d=(double)GetMinValue();
		if (d>(double)GetMaxValue()) d=(double)GetMaxValue();
		v=(emInt64)floor(d+0.5);
		if (v<GetMinValue()) v=GetMinValue();
		if (v>GetMaxValue()) v=GetMaxValue();
		*pValue=v;
		return;
	}

	if (GetInnerBorderType()==IBT_INPUT_FIELD) {
		bgCol=GetLook().GetInputBgColor();
		fgCol=GetLook().GetInputFgColor();
	}
	else if (GetInnerBorderType()==IBT_OUTPUT_FIELD) {
		bgCol=GetLook().GetOutputBgColor();
		fgCol=GetLook().GetOutputFgColor();
	}
	else {
		bgCol=GetLook().GetBgColor();
		fgCol=GetLook().GetFgColor();
	}

	col=bgCol.GetBlended(fgCol,25);
	painter->PaintRect(rx,ry,ax-rx,rh,col,canvasColor);
	painter->PaintRect(ax+aw,ry,rx+rw-ax-aw,rh,col,canvasColor);
	canvasColor=0;

	if (!vRange) {
		tx=ax+aw*0.5;
	}
	else {
		tx=ax+aw*(((double)GetValue())-GetMinValue())/vRange;
	}
	if (e>ay+ah-ry) e=ay+ah-ry;
	xy[0]=tx-e; xy[1]=ry;
	xy[2]=tx+e; xy[3]=ry;
	xy[4]=tx+e; xy[5]=ay+ah-e;
	xy[6]=tx;   xy[7]=ay+ah;
	xy[8]=tx-e; xy[9]=ay+ah-e;
	painter->PaintPolygon(xy,5,fgCol,canvasColor);
	canvasColor=0;

	if (ivalCnt>0 && vRange) {
		f=aw/vRange;
		col=bgCol.GetBlended(fgCol,66);
		for (l=0, ty=ay; l<ivalCnt; l++) {
			th=ah/ivalSum*ivals[l];
			tw=mtw*th;
			if (tw*painter->GetScaleX()>1.0) {
				h4=mth*th;
				h5=mah*th;
				x3=painter->GetUserClipX1()-tw*0.5;
				w3=painter->GetUserClipX2()+tw*0.5-x3;
				if (x3<ax) x3=ax;
				if (w3>ax+aw-x3) w3=ax+aw-x3;
				k1=(emInt64)ceil(((x3-ax)/f+GetMinValue()-0.01)/ivals[l]);
				k2=(emInt64)floor(((x3+w3-ax)/f+GetMinValue()+0.01)/ivals[l]);
				while (k1<=k2) {
					v=k1*ivals[l];
					tx=(v-GetMinValue())*f+ax;
					TextOfValue(buf,sizeof(buf),v,ivals[l]);
					buf[sizeof(buf)-1]=0;
					painter->PaintTextBoxed(
						tx-tw*0.5,ty,tw,h4,
						buf,h4,
						col,canvasColor
					);
					xy[0]=tx-h5*0.5; xy[1]=ty+h4;
					xy[2]=tx+h5*0.5; xy[3]=ty+h4;
					xy[4]=tx;        xy[5]=ty+h4+h5;
					painter->PaintPolygon(xy,3,col,canvasColor);
					k1++;
				}
			}
			ty+=th;
		}
	}

	if (!IsEnabled()) {
		painter->PaintRoundRect(
			x,y,w,h,r,r,
			GetLook().GetBgColor().GetTransparented(20.0F)
		);
	}
}


void emTkScalarField::StepByKeyboard(int dir)
{
	emUInt64 r,dv,mindv;
	emInt64 v;
	int i;

	if (KBInterval>0) dv=KBInterval;
	else {
		r=MaxValue-MinValue;
		mindv=r/129;
		if (mindv<1) mindv=1;
		dv=mindv;
		for (i=0; i<ScaleMarkIntervals.GetCount(); i++) {
			if (ScaleMarkIntervals[i]>=mindv || i==0) {
				dv=ScaleMarkIntervals[i];
			}
		}
	}
	if (dir<0) {
		v=Value-dv;
		if (v<0) v=-(-v/dv)*dv; else v=(v+dv-1)/dv*dv;
	}
	else {
		v=Value+dv;
		if (v<0) v=-((-v+dv-1)/dv)*dv; else v=(v/dv)*dv;
	}
	SetValue(v);
}


const char * emTkScalarField::HowToScalarField=
	"\n"
	"\n"
	"SCALAR FIELD\n"
	"\n"
	"This is a scalar field. In such a field, a scalar value can be viewed and\n"
	"edited. Usually it is a number, but it can even be a choice of a series of\n"
	"possibilities.\n"
	"\n"
	"To move the needle to a desired value, click or drag with the left mouse button.\n"
	"Alternatively, you can move the needle by pressing the + and - keys.\n"
;


const char * emTkScalarField::HowToReadOnly=
	"\n"
	"\n"
	"READ-ONLY\n"
	"\n"
	"This scalar field is read-only. You cannot move the needle.\n"
;


//==============================================================================
//=============================== emTkColorField ===============================
//==============================================================================

emTkColorField::emTkColorField(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon, emColor color,
	bool editable, bool alphaEnabled
)
	: emTkBorder(parent,name,caption,description,icon)
{
	Exp=NULL;
	Color=color;
	Editable=editable;
	AlphaEnabled=alphaEnabled;
	Pressed=false;
	SetBorderType(OBT_INSTRUMENT,Editable?IBT_INPUT_FIELD:IBT_OUTPUT_FIELD);
	EnableAutoExpansion();
	SetAutoExpansionThreshold(9,VCT_MIN_EXT);
}


emTkColorField::~emTkColorField()
{
	if (Exp) delete Exp;
}


void emTkColorField::SetEditable(bool editable)
{
	if (Editable!=editable) {
		Editable=editable;
		InvalidatePainting();
		UpdateExpAppearance();
		if (editable) {
			if (GetInnerBorderType()==IBT_OUTPUT_FIELD) {
				SetInnerBorderType(IBT_INPUT_FIELD);
			}
		}
		else {
			if (GetInnerBorderType()==IBT_INPUT_FIELD) {
				SetInnerBorderType(IBT_OUTPUT_FIELD);
			}
		}
	}
}


void emTkColorField::SetAlphaEnabled(bool alphaEnabled)
{
	if (AlphaEnabled!=alphaEnabled) {
		AlphaEnabled=alphaEnabled;
		InvalidatePainting();
		UpdateExpAppearance();
		if (!alphaEnabled && Color.GetAlpha()!=255) {
			Color.SetAlpha(255);
			UpdateRGBAOutput();
			Signal(ColorSignal);
			ColorChanged();
		}
	}
}


void emTkColorField::SetColor(emColor color)
{
	if (Color!=color) {
		Color=color;
		UpdateRGBAOutput();
		UpdateHSVOutput();
		UpdateNameOutput();
		InvalidatePainting();
		Signal(ColorSignal);
		ColorChanged();
	}
}


void emTkColorField::ColorChanged()
{
}


bool emTkColorField::Cycle()
{
	bool busy,hsvChanged,rgbaChanged,textChanged;
	emColor oldColor;
	emString str;
	emInt64 v;

	busy=emTkBorder::Cycle();

	if (!Exp) return busy;

	hsvChanged=false;
	rgbaChanged=false;
	textChanged=false;
	oldColor=Color;

	if (IsSignaled(Exp->SfRed->GetValueSignal())) {
		v=Exp->SfRed->GetValue();
		if (Exp->RedOut!=v) {
			Exp->RedOut=v;
			Color.SetRed((emByte)((v*255+5000)/10000));
			rgbaChanged=true;
		}
	}
	if (IsSignaled(Exp->SfGreen->GetValueSignal())) {
		v=Exp->SfGreen->GetValue();
		if (Exp->GreenOut!=v) {
			Exp->GreenOut=v;
			Color.SetGreen((emByte)((v*255+5000)/10000));
			rgbaChanged=true;
		}
	}
	if (IsSignaled(Exp->SfBlue->GetValueSignal())) {
		v=Exp->SfBlue->GetValue();
		if (Exp->BlueOut!=v) {
			Exp->BlueOut=v;
			Color.SetBlue((emByte)((v*255+5000)/10000));
			rgbaChanged=true;
		}
	}
	if (IsSignaled(Exp->SfAlpha->GetValueSignal())) {
		v=Exp->SfAlpha->GetValue();
		if (Exp->AlphaOut!=v) {
			Exp->AlphaOut=v;
			Color.SetAlpha((emByte)((v*255+5000)/10000));
			rgbaChanged=true;
		}
	}
	if (IsSignaled(Exp->SfHue->GetValueSignal())) {
		v=Exp->SfHue->GetValue();
		if (Exp->HueOut!=v) {
			Exp->HueOut=v;
			Color.SetHSVA(
				((float)Exp->HueOut)/100.0F,
				((float)Exp->SatOut)/100.0F,
				((float)Exp->ValOut)/100.0F,
				Color.GetAlpha()
			);
			hsvChanged=true;
		}
	}
	if (IsSignaled(Exp->SfSat->GetValueSignal())) {
		v=Exp->SfSat->GetValue();
		if (Exp->SatOut!=v) {
			Exp->SatOut=v;
			Color.SetHSVA(
				((float)Exp->HueOut)/100.0F,
				((float)Exp->SatOut)/100.0F,
				((float)Exp->ValOut)/100.0F,
				Color.GetAlpha()
			);
			hsvChanged=true;
		}
	}
	if (IsSignaled(Exp->SfVal->GetValueSignal())) {
		v=Exp->SfVal->GetValue();
		if (Exp->ValOut!=v) {
			Exp->ValOut=v;
			Color.SetHSVA(
				((float)Exp->HueOut)/100.0F,
				((float)Exp->SatOut)/100.0F,
				((float)Exp->ValOut)/100.0F,
				Color.GetAlpha()
			);
			hsvChanged=true;
		}
	}
	if (IsSignaled(Exp->TfName->GetTextSignal())) {
		str=Exp->TfName->GetText();
		if (Exp->NameOut!=str) {
			Exp->NameOut=str;
			try {
				Color.TryParse(str);
			}
			catch (emString) {
				Color=oldColor;
			}
			Color.SetAlpha(oldColor.GetAlpha());
			textChanged=true;
		}
	}

	if (Color!=oldColor) {
		if (hsvChanged || textChanged) UpdateRGBAOutput();
		if (rgbaChanged || textChanged) UpdateHSVOutput();
		if (rgbaChanged || hsvChanged) UpdateNameOutput();
		InvalidatePainting();
		Signal(ColorSignal);
		ColorChanged();
	}

	return busy;
}


void emTkColorField::AutoExpand()
{
	emArray<emUInt64> percentIntervals;
	emTkTiling * ti;
	emTkScalarField * sf;
	emTkTextField * tf;

	emTkBorder::AutoExpand();

	Exp=new Expansion;

	ti=new emTkTiling(this,"emTkColorField::InnerStuff");
	ti->BeFirst();
	ti->SetSpace(0.08,0.2,0.04,0.1);
	ti->SetFixedColumnCount(2);
	ti->SetChildTallness(0.2);
	ti->SetAlignment(EM_ALIGN_RIGHT);
	Exp->Tiling=ti;

	percentIntervals.Add(2500);
	percentIntervals.Add(500);
	percentIntervals.Add(100);

	sf=new emTkScalarField(
		ti,"r","Red",emString(),emImage(),0,10000,0,true
	);
	sf->SetScaleMarkIntervals(percentIntervals);
	sf->SetTextOfValueFunc(&TextOfPercentValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfRed=sf;

	sf=new emTkScalarField(
		ti,"g","Green",emString(),emImage(),0,10000,0,true
	);
	sf->SetScaleMarkIntervals(percentIntervals);
	sf->SetTextOfValueFunc(&TextOfPercentValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfGreen=sf;

	sf=new emTkScalarField(
		ti,"b","Blue",emString(),emImage(),0,10000,0,true
	);
	sf->SetScaleMarkIntervals(percentIntervals);
	sf->SetTextOfValueFunc(&TextOfPercentValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfBlue=sf;

	sf=new emTkScalarField(
		ti,"a","Alpha",
		"The lower the more transparent."
		,emImage(),0,10000,0,true
	);
	sf->SetScaleMarkIntervals(percentIntervals);
	sf->SetTextOfValueFunc(&TextOfPercentValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfAlpha=sf;

	sf=new emTkScalarField(
		ti,"h","Hue",emString(),emImage(),0,36000,0,true
	);
	sf->SetScaleMarkIntervals(6000,1500,500,100,0);
	sf->SetTextOfValueFunc(&TextOfHueValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	sf->SetTextBoxTallness(0.35);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfHue=sf;

	sf=new emTkScalarField(
		ti,"s","Saturation",emString(),emImage(),0,10000,0,true
	);
	sf->SetScaleMarkIntervals(percentIntervals);
	sf->SetTextOfValueFunc(&TextOfPercentValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfSat=sf;

	sf=new emTkScalarField(
		ti,"v","Value (brightness)",emString(),emImage(),0,10000,0,true
	);
	sf->SetScaleMarkIntervals(percentIntervals);
	sf->SetTextOfValueFunc(&TextOfPercentValue);
	sf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	sf->SetBorderScaling(2.0);
	AddWakeUpSignal(sf->GetValueSignal());
	Exp->SfVal=sf;

	tf=new emTkTextField(
		ti,"n",
		"Name",
		"Here you can enter a color name like 'powder blue',\n"
		"or a hexadecimal RGB value like '#c88' or '#73c81D'.",
		emImage(),emString(),true
	);
	tf->SetBorderType(OBT_RECT,IBT_CUSTOM_RECT);
	tf->SetBorderScaling(2.0);
	AddWakeUpSignal(tf->GetTextSignal());
	Exp->TfName=tf;

	UpdateExpAppearance();
	UpdateRGBAOutput();
	UpdateHSVOutput(true);
	UpdateNameOutput();
}


void emTkColorField::AutoShrink()
{
	emTkBorder::AutoShrink();
	delete Exp;
	Exp=NULL;
}


void emTkColorField::LayoutChildren()
{
	double d,x,y,w,h;

	emTkBorder::LayoutChildren();
	if (Exp) {
		GetContentRect(&x,&y,&w,&h);
		d=emMin(w,h)*0.1;
		x+=d;
		y+=d;
		w-=2*d;
		h-=2*d;
		Exp->Tiling->Layout(x+w*0.5,y,w*0.5,h);
	}
}


bool emTkColorField::HasHowTo()
{
	return true;
}


emString emTkColorField::GetHowTo()
{
	emString h;

	h=emTkBorder::GetHowTo();
	h+=HowToColorField;
	if (!IsEditable()) h+=HowToReadOnly;
	return h;
}


void emTkColorField::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
)
{
	double d,r;

	GetContentRoundRect(&x,&y,&w,&h,&r);
	d=emMin(w,h)*0.1;
	if (!IsEnabled()) {
		painter.PaintRoundRect(
			x,y,w,h,r,r,
			GetLook().GetBgColor().GetTransparented(20.0F)
		);
		canvasColor=0;
	}
	if (!Color.IsOpaque()) {
		painter.PaintTextBoxed(
			x+d,y+d,w-2*d,h-2*d,
			"transparent",
			h,
			Editable ?
				GetLook().GetInputFgColor()
			:
				GetLook().GetOutputFgColor(),
			canvasColor,
			EM_ALIGN_CENTER,
			EM_ALIGN_CENTER
		);
		canvasColor=0;
	}
	painter.PaintRect(
		x+d,y+d,w-2*d,h-2*d,
		Color,
		canvasColor
	);
	painter.PaintRectOutline(
		x+d,y+d,w-2*d,h-2*d,d*0.08,
		GetLook().GetInputFgColor()
	);
}


void emTkColorField::UpdateRGBAOutput()
{
	if (!Exp) return;
	Exp->RedOut=(Color.GetRed()*10000+127)/255;
	Exp->SfRed->SetValue(Exp->RedOut);
	Exp->GreenOut=(Color.GetGreen()*10000+127)/255;
	Exp->SfGreen->SetValue(Exp->GreenOut);
	Exp->BlueOut=(Color.GetBlue()*10000+127)/255;
	Exp->SfBlue->SetValue(Exp->BlueOut);
	Exp->AlphaOut=(Color.GetAlpha()*10000+127)/255;
	Exp->SfAlpha->SetValue(Exp->AlphaOut);
}


void emTkColorField::UpdateHSVOutput(bool initial)
{
	float h,s,v;

	if (!Exp) return;
	h=Color.GetHue();
	s=Color.GetSat();
	v=Color.GetVal();
	if (v>0.0F || initial) {
		if (s>0.0F || initial) {
			Exp->HueOut=(emInt64)(h*100.0F+0.5F);
			Exp->SfHue->SetValue(Exp->HueOut);
		}
		Exp->SatOut=(emInt64)(s*100.0F+0.5F);
		Exp->SfSat->SetValue(Exp->SatOut);
	}
	Exp->ValOut=(emInt64)(v*100.0F+0.5F);
	Exp->SfVal->SetValue(Exp->ValOut);
}


void emTkColorField::UpdateNameOutput()
{
	if (!Exp) return;
	Exp->NameOut=emString::Format(
		"#%02X%02X%02X",
		(int)Color.GetRed(),
		(int)Color.GetGreen(),
		(int)Color.GetBlue()
	);
	Exp->TfName->SetText(Exp->NameOut);
}


void emTkColorField::UpdateExpAppearance()
{
	emTkLook look;
	emColor bg,fg;

	if (!Exp) return;

	look=GetLook();
	if (IsEnabled()) {
		if (Editable) {
			bg=look.GetInputBgColor();
			fg=look.GetInputFgColor();
		}
		else {
			bg=look.GetOutputBgColor();
			fg=look.GetOutputFgColor();
		}
		look.SetBgColor(bg);
		look.SetFgColor(fg);
	}
	Exp->Tiling->SetLook(look,true);

	Exp->SfRed  ->SetEditable(Editable);
	Exp->SfGreen->SetEditable(Editable);
	Exp->SfBlue ->SetEditable(Editable);
	Exp->SfAlpha->SetEditable(Editable);
	Exp->SfHue  ->SetEditable(Editable);
	Exp->SfSat  ->SetEditable(Editable);
	Exp->SfVal  ->SetEditable(Editable);
	Exp->TfName ->SetEditable(Editable);

	Exp->SfAlpha->SetEnableSwitch(AlphaEnabled);
}


void emTkColorField::TextOfPercentValue (
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	snprintf(buf,bufSize,"%G%%",value/100.0);
}


void emTkColorField::TextOfHueValue (
	char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
	void * context
)
{
	const char * name;

	if (markInterval>=6000) {
		switch ((int)value) {
			case  6000: name="Yellow" ; break;
			case 12000: name="Green"  ; break;
			case 18000: name="Cyan"   ; break;
			case 24000: name="Blue"   ; break;
			case 30000: name="Magenta"; break;
			default   : name="Red"    ; break;
		}
		snprintf(buf,bufSize,"%s",name);
	}
	else {
		snprintf(
			buf,bufSize,
			emIsUtf8System() ? "%G\302\260" : "%G\260",
			value/100.0
		);
	}
}


const char * emTkColorField::HowToColorField=
	"\n"
	"\n"
	"COLOR FIELD\n"
	"\n"
	"This panel is for viewing and editing a color. For editing, refer to the inner\n"
	"fields.\n"
;


const char * emTkColorField::HowToReadOnly=
	"\n"
	"\n"
	"READ-ONLY\n"
	"\n"
	"This color field is read-only. You cannot edit the color.\n"
;


//==============================================================================
//================================ emTkSplitter ================================
//==============================================================================

emTkSplitter::emTkSplitter(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon, bool vertical,
	double minPos, double maxPos, double pos
)
	: emTkBorder(parent,name,caption,description,icon)
{
	Vertical=vertical;
	if (minPos<0.0) minPos=0.0;
	if (minPos>1.0) minPos=1.0;
	if (maxPos<0.0) maxPos=0.0;
	if (maxPos>1.0) maxPos=1.0;
	if (minPos>maxPos) {
		minPos=maxPos=(minPos+maxPos)*0.5;
	}
	MinPos=minPos;
	MaxPos=maxPos;
	if (pos<MinPos) pos=MinPos;
	if (pos>MaxPos) pos=MaxPos;
	Pos=pos;
	Pressed=false;
	MousePosInGrip=0.0;
	MouseInGrip=false;
}


emTkSplitter::~emTkSplitter()
{
}


void emTkSplitter::SetVertical(bool vertical)
{
	if (Vertical!=vertical) {
		Vertical=vertical;
		InvalidateCursor();
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkSplitter::SetMinMaxPos(double minPos, double maxPos)
{
	if (minPos<0.0) minPos=0.0;
	if (minPos>1.0) minPos=1.0;
	if (maxPos<0.0) maxPos=0.0;
	if (maxPos>1.0) maxPos=1.0;
	if (minPos>maxPos) {
		minPos=maxPos=(minPos+maxPos)*0.5;
	}
	MinPos=minPos;
	MaxPos=maxPos;
	if (Pos<MinPos) SetPos(MinPos);
	if (Pos>MaxPos) SetPos(MaxPos);
}


void emTkSplitter::SetPos(double pos)
{
	if (pos<MinPos) pos=MinPos;
	if (pos>MaxPos) pos=MaxPos;
	if (Pos!=pos) {
		Pos=pos;
		Signal(PosSignal);
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emTkSplitter::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	double cx,cy,cw,ch,gx,gy,gw,gh,mig,d;

	GetContentRect(&cx,&cy,&cw,&ch);
	CalcGripRect(cx,cy,cw,ch,&gx,&gy,&gw,&gh);

	if (mx>=gx && my>=gy && mx<gx+gw && my<gy+gh) {
		if (!MouseInGrip) {
			MouseInGrip=true;
			InvalidateCursor();
		}
	}
	else {
		if (MouseInGrip) {
			MouseInGrip=false;
			InvalidateCursor();
		}
	}

	if (Pressed) {
		if (Vertical) {
			mig=my-gy;
			if (MousePosInGrip!=mig) {
				d=ch-gh;
				if (d>0.0001) {
					if (IsInActivePath() && !IsActive()) Activate();
					SetPos((gy-cy-MousePosInGrip+mig)/d);
				}
			}
		}
		else {
			mig=mx-gx;
			if (MousePosInGrip!=mig) {
				d=cw-gw;
				if (d>0.0001) {
					if (IsInActivePath() && !IsActive()) Activate();
					SetPos((gx-cx-MousePosInGrip+mig)/d);
				}
			}
		}
		if (!state.Get(EM_KEY_LEFT_BUTTON)) {
			Pressed=false;
			InvalidateCursor();
			InvalidatePainting();
		}
	}
	else if (MouseInGrip && event.IsKey(EM_KEY_LEFT_BUTTON) && IsEnabled()) {
		Pressed=true;
		if (Vertical) MousePosInGrip=my-gy; else MousePosInGrip=mx-gx;
		InvalidateCursor();
		InvalidatePainting();
		Focus();
		event.Eat();
	}
	emTkBorder::Input(event,state,mx,my);
}


emCursor emTkSplitter::GetCursor()
{
	if ((!MouseInGrip && !Pressed) || !IsEnabled()) return emTkBorder::GetCursor();
	else if (Vertical) return emCursor::UP_DOWN_ARROW;
	else return emCursor::LEFT_RIGHT_ARROW;
}


void emTkSplitter::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
)
{
	double gx,gy,gw,gh,d;
	emColor btBgCol;

	CalcGripRect(x,y,w,h,&gx,&gy,&gw,&gh);
	btBgCol=GetLook().GetButtonBgColor();
	painter.PaintRect(
		gx,gy,gw,gh,
		btBgCol,
		canvasColor
	);
	d=emMin(gw,gh)*0.5;
	painter.PaintBorderImage(
		gx,gy,gw,gh,
		d,d,d,d,
		Pressed ?
			GetTkResources().ImgSplitterPressed :
			GetTkResources().ImgSplitter,
		50.0,50.0,50.0,50.0,
		IsEnabled() ? 255 : 64,
		btBgCol,
		0757
	);
}


void emTkSplitter::LayoutChildren()
{
	double cx,cy,cw,ch,gx,gy,gw,gh,x,y,w,h;
	emColor canvasColor;
	emPanel * p, * aux;

	emTkBorder::LayoutChildren();

	p=GetFirstChild();
	if (!p) return;
	aux=GetAuxPanel();
	if (p==aux) {
		p=p->GetNext();
		if (!p) return;
	}
	GetContentRect(&cx,&cy,&cw,&ch,&canvasColor);
	CalcGripRect(cx,cy,cw,ch,&gx,&gy,&gw,&gh);
	if (Vertical) {
		x=cx;
		y=cy;
		w=cw;
		h=gy-cy;
	}
	else {
		x=cx;
		y=cy;
		w=gx-cx;
		h=ch;
	}
	p->Layout(x,y,w,h,canvasColor);

	p=p->GetNext();
	if (!p) return;
	if (p==aux) {
		p=p->GetNext();
		if (!p) return;
	}
	if (Vertical) {
		x=cx;
		y=gy+gh;
		w=cw;
		h=cy+ch-y;
	}
	else {
		x=gx+gw;
		y=cy;
		w=cx+cw-x;
		h=ch;
	}
	p->Layout(x,y,w,h,canvasColor);
}


void emTkSplitter::CalcGripRect(
	double contentX, double contentY, double contentW, double contentH,
	double * pX, double * pY, double * pW, double * pH
)
{
	double gs;

	gs=0.015*GetBorderScaling();
	if (Vertical) {
		gs*=contentH;
		if (gs>contentH*0.5) gs=contentH*0.5;
		*pX=contentX;
		*pY=contentY+Pos*(contentH-gs);
		*pW=contentW;
		*pH=gs;
	}
	else {
		gs*=contentW;
		if (gs>contentW*0.5) gs=contentW*0.5;
		*pX=contentX+Pos*(contentW-gs);
		*pY=contentY;
		*pW=gs;
		*pH=contentH;
	}
}


//==============================================================================
//================================= emTkDialog =================================
//==============================================================================

emTkDialog::emTkDialog(
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


emTkDialog::~emTkDialog()
{
	if (GetRootPanel()) delete GetRootPanel();
}


void emTkDialog::SetRootTitle(const emString & title)
{
	((DlgPanel*)GetRootPanel())->SetTitle(title);
}


void emTkDialog::AddPositiveButton(
	const emString & caption, const emString & description, const emImage & icon
)
{
	DlgButton * bt;

	bt=new DlgButton(
		((DlgPanel*)GetRootPanel())->ButtonTiling,
		emString::Format("%d",ButtonNum),
		caption,description,icon,
		POSITIVE
	);
	bt->ActivateLater();
	ButtonNum++;
}


void emTkDialog::AddNegativeButton(
	const emString & caption, const emString & description, const emImage & icon
)
{
	new DlgButton(
		((DlgPanel*)GetRootPanel())->ButtonTiling,
		emString::Format("%d",ButtonNum),
		caption,description,icon,
		NEGATIVE
	);
	ButtonNum++;
}


void emTkDialog::AddCustomButton(
	const emString & caption, const emString & description, const emImage & icon
)
{
	new DlgButton(
		((DlgPanel*)GetRootPanel())->ButtonTiling,
		emString::Format("%d",ButtonNum),
		caption,description,icon,
		CustomRes
	);
	ButtonNum++;
	CustomRes++;
}


void emTkDialog::AddOKButton()
{
	AddPositiveButton("OK");
}


void emTkDialog::AddCancelButton()
{
	AddNegativeButton("Cancel");
}


void emTkDialog::AddOKCancelButtons()
{
	AddOKButton();
	AddCancelButton();
}


emTkButton * emTkDialog::GetButton(int index)
{
	return dynamic_cast<emTkButton*>(
		((DlgPanel*)GetRootPanel())->ButtonTiling->GetChild(
			emString::Format("%d",index)
		)
	);
}


void emTkDialog::Finish(int result)
{
	Result=result;
	FinishState=1;
	PrivateEngine.WakeUp();
}


void emTkDialog::EnableAutoDeletion(bool autoDelete)
{
	ADEnabled=autoDelete;
}


void emTkDialog::ShowMessage(
	emContext & parentContext, const emString & title, const emString & message,
	const emString & description, const emImage & icon
)
{
	emTkDialog * d;

	d=new emTkDialog(parentContext);
	d->SetRootTitle(title);
	d->AddOKButton();
	new emTkLabel(
		d->GetContentTiling(),
		"l",
		message,
		description,
		icon
	);
	d->EnableAutoDeletion();
}


void emTkDialog::Finished(int result)
{
}


bool emTkDialog::PrivateCycle()
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


emTkDialog::DlgButton::DlgButton(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon, int result
)
	: emTkButton(parent,name,caption,description,icon)
{
	Result=result;
}


void emTkDialog::DlgButton::Clicked()
{
	((emTkDialog*)GetWindow())->Finish(Result);
}


emTkDialog::DlgPanel::DlgPanel(ParentArg parent, const emString & name)
	: emTkBorder(parent,name)
{
	ContentTiling=new emTkTiling(this,"content");
	ContentTiling->SetInnerBorderType(IBT_CUSTOM_RECT);
	ButtonTiling=new emTkTiling(this,"buttons");
	ButtonTiling->SetChildTallness(0.3);
	ButtonTiling->SetInnerSpace(0.1,0.1);
	if ((GetView().GetViewFlags())&emView::VF_POPUP_ZOOM) {
		SetOuterBorderType(OBT_POPUP_ROOT);
	}
	else {
		SetOuterBorderType(OBT_FILLED);
	}
}


emTkDialog::DlgPanel::~DlgPanel()
{
}


void emTkDialog::DlgPanel::SetTitle(const emString & title)
{
	if (Title!=title) {
		Title=title;
		InvalidateTitle();
	}
}


emString emTkDialog::DlgPanel::GetTitle()
{
	return Title;
}


void emTkDialog::DlgPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	emTkBorder::Input(event,state,mx,my);

	switch (event.GetKey()) {
	case EM_KEY_ENTER:
		if (state.IsNoMod()) {
			((emTkDialog*)GetWindow())->Finish(POSITIVE);
			event.Eat();
		}
		break;
	case EM_KEY_ESCAPE:
		if (state.IsNoMod()) {
			((emTkDialog*)GetWindow())->Finish(NEGATIVE);
			event.Eat();
		}
		break;
	default:
		break;
	}
}


void emTkDialog::DlgPanel::LayoutChildren()
{
	double x,y,w,h,sp,bh;
	emColor cc;

	emTkBorder::LayoutChildren();

	GetContentRect(&x,&y,&w,&h,&cc);
	bh=emMin(w*0.08,h*0.3);
	sp=bh*0.25;
	x+=sp;
	y+=sp;
	w-=2*sp;
	h-=2*sp;
	ContentTiling->Layout(
		x,y,w,h-sp-bh,cc
	);
	ButtonTiling->Layout(
		x,y+h-bh,w,bh,cc
	);
}


emTkDialog::PrivateEngineClass::PrivateEngineClass(emTkDialog & dlg)
	: emEngine(dlg.GetScheduler()),
	Dlg(dlg)
{
}


bool emTkDialog::PrivateEngineClass::Cycle()
{
	return Dlg.PrivateCycle();
}
