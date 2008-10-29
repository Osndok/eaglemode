//------------------------------------------------------------------------------
// emImageFile.cpp
//
// Copyright (C) 2004-2008 Oliver Hamann.
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
	Image.Empty();
	Comment.Empty();
	FileFormatInfo.Empty();
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


void emImageFilePanel::GetEssenceRect(
	double * pX, double * pY, double * pW, double * pH
)
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


bool emImageFilePanel::IsOpaque()
{
	if (IsVFSGood()) {
		return false;
	}
	else {
		return emFilePanel::IsOpaque();
	}
}


void emImageFilePanel::Paint(const emPainter & painter, emColor canvasColor)
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
	emTkGroup * grp;
	emTkTextField * tf;

	if (IsVFSGood()) {
		fm=(emImageFileModel*)GetFileModel();
		grp=new emTkGroup(
			parent,
			name,
			"Image File Info"
		);
		grp->SetFixedColumnCount(1);
		new emTkTextField(
			grp,
			"format",
			"File Format",
			emString(),
			emImage(),
			fm->GetFileFormatInfo()
		);
		new emTkTextField(
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
		tf=new emTkTextField(
			grp,
			"comment",
			"Comment",
			emString(),
			emImage(),
			fm->GetComment()
		);
		tf->SetMultiLineMode();
		return grp;
	}
	else {
		return emFilePanel::CreateControlPanel(parent,name);
	}
}
