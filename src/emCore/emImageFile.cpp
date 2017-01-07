//------------------------------------------------------------------------------
// emImageFile.cpp
//
// Copyright (C) 2004-2008,2014-2016 Oliver Hamann.
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

#include <emCore/emImageFile.h>
#include <emCore/emToolkit.h>


//==============================================================================
//============================== emImageFileModel ==============================
//==============================================================================

void emImageFileModel::SetImage(const emImage & image)
{
	if (Image!=image) {
		SetUnsavedState();
		Image=image;
		Signal(ChangeSignal);
	}
}


void emImageFileModel::SetComment(const emString & comment)
{
	if (Comment!=comment) {
		SetUnsavedState();
		Comment=comment;
		Signal(ChangeSignal);
	}
}


void emImageFileModel::SetFileFormatInfo(const emString & fileFormatInfo)
{
	if (FileFormatInfo!=fileFormatInfo) {
		SetUnsavedState();
		FileFormatInfo=fileFormatInfo;
		Signal(ChangeSignal);
	}
}


emImageFileModel::emImageFileModel(emContext & context, const emString & name)
	: emFileModel(context,name)
{
}


emImageFileModel::~emImageFileModel()
{
}


void emImageFileModel::ResetData()
{
	Image.Clear();
	Comment.Clear();
	FileFormatInfo.Clear();
	Signal(ChangeSignal);
}


//==============================================================================
//============================== emImageFilePanel ==============================
//==============================================================================

emImageFilePanel::emImageFilePanel(
	ParentArg parent, const emString & name,
	emImageFileModel * fileModel, bool updateFileModel
)
	: emFilePanel(parent,name)
{
	AddWakeUpSignal(GetVirFileStateSignal());
	SetFileModel(fileModel,updateFileModel);
}


void emImageFilePanel::SetFileModel(
	emFileModel * fileModel, bool updateFileModel
)
{
	if (fileModel && (dynamic_cast<emImageFileModel*>(fileModel))==NULL) {
		fileModel=NULL;
	}

	if (GetFileModel()) {
		RemoveWakeUpSignal(
			((const emImageFileModel*)GetFileModel())->GetChangeSignal()
		);
	}

	emFilePanel::SetFileModel(fileModel,updateFileModel);

	if (GetFileModel()) {
		AddWakeUpSignal(
			((const emImageFileModel*)GetFileModel())->GetChangeSignal()
		);
	}
}


emString emImageFilePanel::GetIconFileName() const
{
	return "picture.tga";
}


void emImageFilePanel::GetEssenceRect(
	double * pX, double * pY, double * pW, double * pH
) const
{
	emImageFileModel * fm;
	const emImage * img;
	double x,y,w,h,d;
	int iw,ih;

	if (IsVFSGood()) {
		fm=(emImageFileModel*)GetFileModel();
		img=&fm->GetImage();
		if (img) {
			x=0;
			y=0;
			w=1;
			h=GetHeight();
			iw=img->GetWidth();
			ih=img->GetHeight();
			if (iw>0 && ih>0) {
				if (iw*h>=ih*w) {
					d=w*ih/iw;
					y+=(h-d)/2;
					h=d;
				}
				else {
					d=h*iw/ih;
					x+=(w-d)/2;
					w=d;
				}
				*pX=x;
				*pY=y;
				*pW=w;
				*pH=h;
				return;
			}
		}
	}
	emFilePanel::GetEssenceRect(pX,pY,pW,pH);
}


bool emImageFilePanel::Cycle()
{
	if (IsSignaled(((emImageFileModel*)GetFileModel())->GetChangeSignal())) {
		if (IsVFSGood()) {
			InvalidatePainting();
			InvalidateControlPanel(); //??? very cheap solution, but okay for now.
		}
	}
	if (IsSignaled(GetVirFileStateSignal())) {
		InvalidateControlPanel(); //??? very cheap solution, but okay for now.
	}
	return emFilePanel::Cycle();
}


bool emImageFilePanel::IsOpaque() const
{
	if (IsVFSGood()) {
		return false;
	}
	else {
		return emFilePanel::IsOpaque();
	}
}


void emImageFilePanel::Paint(
	const emPainter & painter, emColor canvasColor
) const
{
	emImageFileModel * fm;
	const emImage * img;
	double x,y,w,h,d;
	int iw,ih;

	if (IsVFSGood()) {
		fm=(emImageFileModel*)GetFileModel();
		img=&fm->GetImage();
		if (img) {
			x=0;
			y=0;
			w=1;
			h=GetHeight();
			iw=img->GetWidth();
			ih=img->GetHeight();
			if (iw>0 && ih>0) {
				if (iw*h>=ih*w) {
					d=w*ih/iw;
					y+=(h-d)/2;
					h=d;
				}
				else {
					d=h*iw/ih;
					x+=(w-d)/2;
					w=d;
				}
				painter.PaintImage(
					x,y,w,h,
					*img,
					255,
					canvasColor
				);
			}
		}
	}
	else {
		emFilePanel::Paint(painter,canvasColor);
	}
}


emPanel * emImageFilePanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	emImageFileModel * fm;
	emLinearLayout * mainLayout;
	emLinearGroup * grp;
	emTextField * tf;

	if (IsVFSGood()) {
		fm=(emImageFileModel*)GetFileModel();
		mainLayout=new emLinearLayout(parent,name);
		mainLayout->SetMinChildTallness(0.03);
		mainLayout->SetMaxChildTallness(0.6);
		mainLayout->SetAlignment(EM_ALIGN_TOP_LEFT);
		grp=new emLinearGroup(
			mainLayout,
			"",
			"Image File Info"
		);
		grp->SetOrientationThresholdTallness(0.07);
		new emTextField(
			grp,
			"format",
			"File Format",
			emString(),
			emImage(),
			fm->GetFileFormatInfo()
		);
		new emTextField(
			grp,
			"size",
			"Size",
			emString(),
			emImage(),
			emString::Format(
				"%dx%d pixels",
				fm->GetImage().GetWidth(),
				fm->GetImage().GetHeight()
			)
		);
		tf=new emTextField(
			grp,
			"comment",
			"Comment",
			emString(),
			emImage(),
			fm->GetComment()
		);
		tf->SetMultiLineMode();
		return mainLayout;
	}
	else {
		return emFilePanel::CreateControlPanel(parent,name);
	}
}
