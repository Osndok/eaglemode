//------------------------------------------------------------------------------
// emBorder.cpp
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

#include <emCore/emBorder.h>
#include <emCore/emInstallInfo.h>
#include <emCore/emRes.h>


emBorder::emBorder(
	ParentArg parent, const emString & name, const emString & caption,
	const emString & description, const emImage & icon
)
	: emPanel(parent,name),
	Caption(caption),
	Description(description),
	Icon(icon)
{
	emBorder * tkp;
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
		r->ImgDir=emGetResImage(*rc,emGetChildPath(resDir,"Dir.tga"));
		r->ImgDirUp=emGetResImage(*rc,emGetChildPath(resDir,"DirUp.tga"));
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
		tkp=dynamic_cast<emBorder*>(p);
		if (tkp) {
			Look=tkp->Look;
			break;
		}
	}

	MaxIconAreaTallness=1.0;
	BorderScaling=1.0;
	LabelAlignment=EM_ALIGN_LEFT;
	CaptionAlignment=EM_ALIGN_LEFT;
	DescriptionAlignment=EM_ALIGN_LEFT;
	OuterBorder=OBT_NONE;
	InnerBorder=IBT_NONE;
	IconAboveCaption=false;
	LabelInBorder=true;
}


emBorder::~emBorder()
{
	if (Aux) delete Aux;
}


void emBorder::SetCaption(const emString & caption)
{
	if (Caption!=caption) {
		Caption=caption;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emBorder::SetDescription(const emString & description)
{
	if (Description!=description) {
		Description=description;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emBorder::SetIcon(const emImage & icon)
{
	if (Icon!=icon) {
		Icon=icon;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emBorder::SetLabel(
	const emString & caption, const emString & description,
	const emImage & icon
)
{
	SetCaption(caption);
	SetDescription(description);
	SetIcon(icon);
}


void emBorder::SetLabelAlignment(emAlignment labelAlignment)
{
	if (LabelAlignment!=labelAlignment) {
		LabelAlignment=labelAlignment;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emBorder::SetCaptionAlignment(emAlignment captionAlignment)
{
	if (CaptionAlignment!=captionAlignment) {
		CaptionAlignment=captionAlignment;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emBorder::SetDescriptionAlignment(emAlignment descriptionAlignment)
{
	if (DescriptionAlignment!=descriptionAlignment) {
		DescriptionAlignment=descriptionAlignment;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emBorder::SetIconAboveCaption(bool iconAboveCaption)
{
	if (IconAboveCaption!=iconAboveCaption) {
		IconAboveCaption=iconAboveCaption;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emBorder::SetMaxIconAreaTallness(double maxIconAreaTallness)
{
	if (maxIconAreaTallness<1E-10) maxIconAreaTallness=1E-10;
	if (MaxIconAreaTallness!=maxIconAreaTallness) {
		MaxIconAreaTallness=maxIconAreaTallness;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emBorder::SetOuterBorderType(OuterBorderType obt)
{
	if (OuterBorder!=(emByte)obt) {
		OuterBorder=(emByte)obt;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emBorder::SetInnerBorderType(InnerBorderType ibt)
{
	if (InnerBorder!=(emByte)ibt) {
		InnerBorder=(emByte)ibt;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emBorder::SetBorderType(OuterBorderType obt, InnerBorderType ibt)
{
	if (OuterBorder!=(emByte)obt || InnerBorder!=(emByte)ibt) {
		OuterBorder=(emByte)obt;
		InnerBorder=(emByte)ibt;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emBorder::SetBorderScaling(double borderScaling)
{
	if (borderScaling<1E-10) borderScaling=1E-10;
	if (BorderScaling!=borderScaling) {
		BorderScaling=borderScaling;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


void emBorder::SetLook(const emLook & look, bool recursively)
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


void emBorder::HaveAux(const emString & panelName, double tallness)
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


void emBorder::RemoveAux()
{
	if (Aux) {
		delete Aux;
		Aux=NULL;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


const emString & emBorder::GetAuxPanelName() const
{
	static const emString emptyString;
		// Okay this is thread-safe as long as the string is empty.

	if (!Aux) return emptyString;
	return Aux->PanelName;
}


double emBorder::GetAuxTallness() const
{
	if (!Aux) return 1.0;
	return Aux->Tallness;
}


emPanel * emBorder::GetAuxPanel()
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


void emBorder::GetAuxRect(
	double * pX, double * pY, double * pW, double * pH,
	emColor * pCanvasColor
) const
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


void emBorder::GetSubstanceRect(
	double * pX, double * pY, double * pW, double * pH, double * pR
) const
{
	DoBorder(
		BORDER_FUNC_SUBSTANCE_ROUND_RECT,NULL,GetCanvasColor(),
		pX,pY,pW,pH,pR,NULL
	);
}


void emBorder::GetContentRoundRect(
	double * pX, double * pY, double * pW, double * pH, double * pR,
	emColor * pCanvasColor
) const
{
	DoBorder(
		BORDER_FUNC_CONTENT_ROUND_RECT,NULL,GetCanvasColor(),
		pX,pY,pW,pH,pR,pCanvasColor
	);
}


void emBorder::GetContentRect(
	double * pX, double * pY, double * pW, double * pH,
	emColor * pCanvasColor
) const
{
	DoBorder(
		BORDER_FUNC_CONTENT_RECT,NULL,GetCanvasColor(),
		pX,pY,pW,pH,NULL,pCanvasColor
	);
}


void emBorder::GetContentRectUnobscured(
	double * pX, double * pY, double * pW, double * pH,
	emColor * pCanvasColor
) const
{
	DoBorder(
		BORDER_FUNC_CONTENT_RECT_UNOBSCURED,NULL,GetCanvasColor(),
		pX,pY,pW,pH,NULL,pCanvasColor
	);
}


void emBorder::Notice(NoticeFlags flags)
{
	if ((flags&(NF_ENABLE_CHANGED|NF_ACTIVE_CHANGED|NF_FOCUS_CHANGED))!=0) {
		InvalidatePainting();
	}
	if ((flags&NF_ENABLE_CHANGED)!=0) {
		if (InnerBorder==(emByte)IBT_INPUT_FIELD || InnerBorder==(emByte)IBT_OUTPUT_FIELD) {
			InvalidateChildrenLayout();
		}
	}
	emPanel::Notice(flags);
}


bool emBorder::IsOpaque() const
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


void emBorder::Paint(const emPainter & painter, emColor canvasColor) const
{
	DoBorder(BORDER_FUNC_PAINT,&painter,canvasColor,NULL,NULL,NULL,NULL,NULL,NULL);
}


void emBorder::LayoutChildren()
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


bool emBorder::HasHowTo() const
{
	return false;
}


emString emBorder::GetHowTo() const
{
	emString h;

	h=HowToPreface;
	if (!IsEnabled()) h+=HowToDisabled;
	if (IsFocusable()) h+=HowToFocus;
	return h;
}


void emBorder::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
) const
{
}


bool emBorder::HasLabel() const
{
	return !Caption.IsEmpty() || !Description.IsEmpty() || !Icon.IsEmpty();
}


double emBorder::GetBestLabelTallness() const
{
	double bestTallness;

	DoLabel(
		LABEL_FUNC_GET_BEST_TALLNESS,NULL,0.0,0.0,1.0,1.0,
		0,0,&bestTallness
	);
	return bestTallness;
}


void emBorder::PaintLabel(
	const emPainter & painter, double x, double y, double w, double h,
	emColor color, emColor canvasColor
) const
{
	DoLabel(
		LABEL_FUNC_PAINT,&painter,x,y,w,h,color,canvasColor,NULL
	);
}


void emBorder::SetLabelInBorder(bool labelInBorder)
{
	if (LabelInBorder!=labelInBorder) {
		LabelInBorder=labelInBorder;
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


emBorder::TkResources::TkResources()
{
}


emBorder::TkResources::~TkResources()
{
}


void emBorder::GetContentRoundRect(
	double * pX, double * pY, double * pW, double * pH, double * pR,
	emColor * pCanvasColor
)
{
	((const emBorder*)this)->GetContentRoundRect(pX,pY,pW,pH,pR,pCanvasColor);
}


void emBorder::GetContentRect(
	double * pX, double * pY, double * pW, double * pH,
	emColor * pCanvasColor
)
{
	((const emBorder*)this)->GetContentRect(pX,pY,pW,pH,pCanvasColor);
}


void emBorder::GetContentRectUnobscured(
	double * pX, double * pY, double * pW, double * pH,
	emColor * pCanvasColor
)
{
	((const emBorder*)this)->GetContentRectUnobscured(pX,pY,pW,pH,pCanvasColor);
}

bool emBorder::HasHowTo()
{
	return ((const emBorder*)this)->HasHowTo();
}


emString emBorder::GetHowTo()
{
	return ((const emBorder*)this)->GetHowTo();
}


void emBorder::PaintContent(
	const emPainter & painter, double x, double y, double w, double h,
	emColor canvasColor
)
{
	((const emBorder*)this)->PaintContent(painter,x,y,w,h,canvasColor);
}


bool emBorder::HasLabel()
{
	return ((const emBorder*)this)->HasLabel();
}


double emBorder::GetBestLabelTallness()
{
	return ((const emBorder*)this)->GetBestLabelTallness();
}


void emBorder::PaintLabel(
	const emPainter & painter, double x, double y, double w, double h,
	emColor color, emColor canvasColor
)
{
	((const emBorder*)this)->PaintLabel(painter,x,y,w,h,color,canvasColor);
}


void emBorder::DoBorder(
	DoBorderFunc func, const emPainter * painter, emColor canvasColor,
	double * pX, double * pY, double * pW, double * pH, double * pR,
	emColor * pCanvasColor
) const
{
	double s,d,e,f,g,h,r,tx,ty,tw,th,tr;
	double minSpace,howToSpace,labelSpace;
	double rndX,rndY,rndW,rndH,rndR,recX,recY,recW,recH;
	emColor color,color2;

	h=GetHeight();

	switch ((OuterBorderType)OuterBorder) {
	default: // OBT_NONE or OBT_FILLED
		if (func==BORDER_FUNC_SUBSTANCE_ROUND_RECT) {
			if (pX) *pX=0.0;
			if (pY) *pY=0.0;
			if (pW) *pW=1.0;
			if (pH) *pH=h;
			if (pR) *pR=0.0;
			return;
		}
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
		if (func==BORDER_FUNC_SUBSTANCE_ROUND_RECT) {
			if (pX) *pX=d;
			if (pY) *pY=d;
			if (pW) *pW=1.0-2*d;
			if (pH) *pH=h-2*d;
			if (pR) *pR=0.0;
			return;
		}
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
		if (func==BORDER_FUNC_SUBSTANCE_ROUND_RECT) {
			if (pX) *pX=d;
			if (pY) *pY=d;
			if (pW) *pW=1.0-2*d;
			if (pH) *pH=h-2*d;
			if (pR) *pR=0.0;
			return;
		}
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
		if (func==BORDER_FUNC_SUBSTANCE_ROUND_RECT) {
			if (pX) *pX=d;
			if (pY) *pY=d;
			if (pW) *pW=1.0-2*d;
			if (pH) *pH=h-2*d;
			if (pR) *pR=f;
			return;
		}
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
		if (func==BORDER_FUNC_SUBSTANCE_ROUND_RECT) {
			r=rndR*(280.0/209.0);
			e=r-rndR;
			if (pX) *pX=rndX-e;
			if (pY) *pY=rndY-e;
			if (pW) *pW=rndW+2*e;
			if (pH) *pH=rndH+2*e;
			if (pR) *pR=r;
			return;
		}
		if (func==BORDER_FUNC_PAINT) {
			color2=canvasColor;
			if (
				!color.IsTotallyTransparent() &&
				(!color2.IsOpaque() || color2!=color)
			) {
				r=rndR*(280.0/209.0);
				e=r-rndR;
				painter->PaintRoundRect(
					rndX-e,rndY-e,rndW+2*e,rndH+2*e,
					r,r,
					color,
					color2
				);
				color2=0;
			}
			r=rndR*(286.0/209.0);
			e=r-rndR;
			painter->PaintBorderImage(
				rndX-e,rndY-e,rndW+2*e,rndH+2*e,
				r,r,r,r,
				GetTkResources().ImgGroupBorder,
				286.0,286.0,286.0,286.0,
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
		if (func==BORDER_FUNC_SUBSTANCE_ROUND_RECT) {
			r=rndR*(280.0/209.0);
			e=r-rndR;
			if (pX) *pX=rndX-e;
			if (pY) *pY=rndY-e;
			if (pW) *pW=rndW+2*e;
			if (pH) *pH=rndH+2*e;
			if (pR) *pR=r;
			return;
		}
		if (func==BORDER_FUNC_PAINT) {
			color2=canvasColor;
			if (
				!color.IsTotallyTransparent() &&
				(!color2.IsOpaque() || color2!=color)
			) {
				r=rndR*(280.0/209.0);
				e=r-rndR;
				painter->PaintRoundRect(
					rndX-e,rndY-e,rndW+2*e,rndH+2*e,
					r,r,
					color,
					color2
				);
				color2=0;
			}
			r=rndR*(286.0/209.0);
			e=r-rndR;
			painter->PaintBorderImage(
				rndX-e,rndY-e,rndW+2*e,rndH+2*e,
				r,r,r,r,
				GetTkResources().ImgGroupBorder,
				286.0,286.0,286.0,286.0,
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
		if (func==BORDER_FUNC_SUBSTANCE_ROUND_RECT) {
			r=rndR*(336/293.4);
			e=r-rndR;
			if (pX) *pX=rndX-e;
			if (pY) *pY=rndY-e;
			if (pW) *pW=rndW+2*e;
			if (pH) *pH=rndH+2*e;
			if (pR) *pR=r;
			return;
		}
		if (func==BORDER_FUNC_PAINT) {
			color2=canvasColor;
			if (
				!color.IsTotallyTransparent() &&
				(!color2.IsOpaque() || color2!=color)
			) {
				r=rndR*(336/293.4);
				e=r-rndR;
				painter->PaintRoundRect(
					rndX-e,rndY-e,rndW+2*e,rndH+2*e,
					r,r,
					color,
					color2
				);
				color2=0;
			}
			r=rndR*(340.0/293.4);
			e=r-rndR;
			painter->PaintBorderImage(
				rndX-e,rndY-e,rndW+2*e,rndH+2*e,
				r,r,r,r,
				GetTkResources().ImgButtonBorder,
				340.0,340.0,340.0,340.0,
				255,color2,0757
			);
		}
		if (!color.IsTotallyTransparent()) canvasColor=color;
		break;
	case OBT_POPUP_ROOT:
		if (func==BORDER_FUNC_SUBSTANCE_ROUND_RECT) {
			if (pX) *pX=0.0;
			if (pY) *pY=0.0;
			if (pW) *pW=1.0;
			if (pH) *pH=h;
			if (pR) *pR=0.0;
			return;
		}
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
			r=d*(159.0/159.0);
			painter->PaintBorderImage(
				0.0,0.0,1.0,h,
				r,r,r,r,
				GetTkResources().ImgPopupBorder,
				159.0,159.0,159.0,159.0,
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
		tw=s*0.1;
		th=tw*Aux->Tallness;
		d=s*0.01;
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
		e=s*0.015;
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
		r=emMin(rndW,rndH)*BorderScaling*0.0188;
		if (rndR<r) rndR=r;
		if (func==BORDER_FUNC_PAINT) {
			painter->PaintBorderImage(
				rndX,rndY,rndW,rndH,
				rndR,rndR,rndR,rndR,
				GetTkResources().ImgGroupInnerBorder,
				225,225,225,225,
				255,canvasColor,0757
			);
		}
		d=rndR*(17.0/225.0);
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
		r=emMin(rndW,rndH)*BorderScaling*0.094;
		if (rndR<r) rndR=r;
		d=(1-(216.0-16.0)/216.0)*rndR;
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
		if (!IsEnabled()) color=color.GetBlended(Look.GetBgColor(),80.0F);
		if (func==BORDER_FUNC_PAINT) {
			painter->PaintRoundRect(tx,ty,tw,th,tr,tr,color,canvasColor);
			canvasColor=color;
			PaintContent(*painter,recX,recY,recW,recH,canvasColor);
			painter->PaintBorderImage(
				rndX,rndY,rndW,rndH,
				300.0/216*rndR,346.0/216*rndR,216.0/216*rndR,216.0/216*rndR,
				GetTkResources().ImgIOField,
				300.0,346.0,216.0,216.0,
				255,0,0757
			);
			return;
		}
		if (func==BORDER_FUNC_CONTENT_RECT_UNOBSCURED) {
			d=220.0/216.0*rndR;
			if (pX) *pX=rndX+d;
			if (pY) *pY=rndY+d;
			if (pW) *pW=rndW-2*d;
			if (pH) *pH=rndH-2*d;
			if (pCanvasColor) *pCanvasColor=color;
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
				200,200,200,200,
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
	else if (
		func==BORDER_FUNC_CONTENT_RECT ||
		func==BORDER_FUNC_CONTENT_RECT_UNOBSCURED
	) {
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


void emBorder::DoLabel(
	DoLabelFunc func, const emPainter * painter, double x, double y,
	double w, double h, emColor color, emColor canvasColor,
	double * pBestTallness
) const
{
	double iconX,iconY,iconW,iconH,capX,capY,capW,capH,descX,descY,descW,descH;
	double gap1,gap2,totalW,totalH,minTotalW,minWS,f,w2,h2;

	totalW=1.0;
	totalH=1.0;

	if (!Caption.IsEmpty()) {
		capW=emPainter::GetTextSize(Caption,1.0,true,0.0,&capH);
		totalW=capW;
		totalH=capH;
	}
	else {
		capW=0.0;
		capH=0.0;
	}

	if (!Icon.IsEmpty()) {
		iconW=Icon.GetWidth();
		iconH=Icon.GetHeight();
		if (iconH > iconW*MaxIconAreaTallness) {
			iconH = iconW*MaxIconAreaTallness;
		}
		if (!Caption.IsEmpty()) {
			if (IconAboveCaption) {
				f=capH*3.0;
				iconW*=f/iconH;
				iconH=f;
				gap1=capH*0.1;
				totalW=emMax(iconW,capW);
				totalH=iconH+gap1+capH;
			}
			else {
				iconW*=capH/iconH;
				iconH=capH;
				gap1=capH*0.1;
				totalW=iconW+gap1+capW;
				totalH=capH;
			}
		}
		else {
			totalW=iconW;
			totalH=iconH;
			gap1=0.0;
		}
	}
	else {
		iconW=0.0;
		iconH=0.0;
		gap1=0.0;
	}

	if (!Description.IsEmpty()) {
		descW=emPainter::GetTextSize(Description,1.0,true,0.0,&descH);
		if (!Icon.IsEmpty() || !Caption.IsEmpty()) {
			if (!Caption.IsEmpty()) {
				f=capH*0.15;
				descW=f/descH;
				descH=f;
			}
			else {
				f=iconH*0.05;
				descW=f/descH;
				descH=f;
			}
			if (descW>totalW) {
				descH*=totalW/descW;
				descW=totalW;
			}
			gap2=descH*0.05;
			totalH=totalH+gap2+descH;
		}
		else {
			totalW=descW;
			totalH=descH;
			gap2=0.0;
		}
	}
	else {
		descW=0.0;
		descH=0.0;
		gap2=0.0;
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
		if (!Icon.IsEmpty()) {
			if (IconAboveCaption) {
				minTotalW=iconW;
			}
			else {
				minTotalW=iconW+gap1+capW*minWS;
			}
		}
		else {
			minTotalW=totalW*minWS;
		}
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

	iconW*=f;
	iconH*=f;
	gap1*=f;
	iconY=y;
	capH*=f;
	if (IconAboveCaption) {
		iconX=x+(w-iconW)*0.5;
		capX=x;
		capY=iconY+iconH+gap1;
		capW=w;
	}
	else {
		iconX=x;
		capX=iconX+iconW+gap1;
		capY=y;
		capW=x+w-capX;
	}
	gap2*=f;
	descX=x;
	descY=emMax(iconY+iconH,capY+capH)+gap2;
	descW=w;
	descH*=f;

	if (!Icon.IsEmpty()) {
		f=iconH*Icon.GetWidth()/Icon.GetHeight();
		iconX+=(iconW-f)*0.5;
		iconW=f;
		if (Icon.GetChannelCount()==1) {
			painter->PaintShape(
				iconX,
				iconY,
				iconW,
				iconH,
				Icon,
				0,
				color,
				canvasColor
			);
		}
		else {
			painter->PaintImage(
				iconX,
				iconY,
				iconW,
				iconH,
				Icon,
				color.GetAlpha(),
				canvasColor
			);
		}
	}
	if (!Caption.IsEmpty()) {
		painter->PaintTextBoxed(
			capX,
			capY,
			capW,
			capH,
			Caption,
			capH,
			color,
			canvasColor,
			EM_ALIGN_CENTER,
			CaptionAlignment,
			minWS
		);
	}
	if (!Description.IsEmpty()) {
		painter->PaintTextBoxed(
			descX,
			descY,
			descW,
			descH,
			Description,
			descH,
			color,
			canvasColor,
			EM_ALIGN_CENTER,
			DescriptionAlignment,
			minWS
		);
	}
}


const char * emBorder::HowToPreface=
	"How to use this panel\n"
	"#####################\n"
	"\n"
	"Here is some text describing the usage of this panel. The text consists of\n"
	"multiple sections which may come from different parts of the program based on\n"
	"each other. If something is contradictory, the later section should count.\n"
;


const char * emBorder::HowToDisabled=
	"\n"
	"\n"
	"DISABLED\n"
	"\n"
	"This panel is currently disabled, because the panel is probably irrelevant for\n"
	"the current state of the program or data. Any try to modify data or to trigger a\n"
	"function may silently be ignored.\n"
;


const char * emBorder::HowToFocus=
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
