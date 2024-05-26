//------------------------------------------------------------------------------
// emFileManSelInfoPanel.cpp
//
// Copyright (C) 2007-2009,2014-2016,2019,2021,2024 Oliver Hamann.
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

#include <emFileMan/emFileManSelInfoPanel.h>


emFileManSelInfoPanel::emFileManSelInfoPanel(
	ParentArg parent, const emString & name
)
	: emPanel(parent,name)
{
	FileMan=emFileManModel::Acquire(GetRootContext());
	AllowBusiness=false;
	DirStack.SetTuningLevel(1);
	InitialDirStack.SetTuningLevel(1);
	SelList.SetTuningLevel(1);
	DirHandle=NULL;
	ResetDetails();
	SetRectangles();
	AddWakeUpSignal(FileMan->GetSelectionSignal());
}


emFileManSelInfoPanel::~emFileManSelInfoPanel()
{
	if (DirHandle) {
		emCloseDir(DirHandle);
		DirHandle=NULL;
	}
}


bool emFileManSelInfoPanel::Cycle()
{
	bool busy;

	if (IsSignaled(FileMan->GetSelectionSignal())) {
		ResetDetails();
		InvalidatePainting();
	}

	do {
		busy=WorkOnDetails();
	} while (busy && !IsTimeSliceAtEnd());

	return busy;
}


void emFileManSelInfoPanel::Notice(NoticeFlags flags)
{
	double x1,y1,x2,y2;

	if (flags&NF_LAYOUT_CHANGED) {
		SetRectangles();
	}

	if (flags&NF_VIEWING_CHANGED) {
		if (IsViewed()) {
			x1=PanelToViewX(DetailsX);
			y1=PanelToViewY(DetailsY);
			x2=PanelToViewX(DetailsX+DetailsW);
			y2=PanelToViewY(DetailsY+DetailsH);
			if (
				(x2-x1)*(y2-y1)>40000 &&
				x1<GetClipX2() &&
				y1<GetClipY2() &&
				x2>GetClipX1() &&
				y2>GetClipY1()
			) {
				if (!AllowBusiness) {
					AllowBusiness=true;
					WakeUp();
				}
			}
			else {
				AllowBusiness=false;
			}
		}
		else {
			AllowBusiness=false;
		}
	}
}


bool emFileManSelInfoPanel::IsOpaque() const
{
	return false;
}


void emFileManSelInfoPanel::Paint(const emPainter & painter, emColor canvasColor) const
{
	double xy[4*2];
	emColor fgColor,bgColor1,bgColor2,fgColor1,fgColor2;
	char tmp[256];
	double x,y,s,h;

	fgColor=emColor(128,224,128);
	sprintf(tmp,"Sources:%4d",FileMan->GetSourceSelectionCount());
	painter.PaintTextBoxed(
		TextX,
		TextY,
		TextW,
		TextH*0.5,
		tmp,
		TextH*0.5,
		fgColor,
		canvasColor,
		EM_ALIGN_LEFT
	);
	fgColor=emColor(224,128,128);
	sprintf(tmp,"Targets:%4d",FileMan->GetTargetSelectionCount());
	painter.PaintTextBoxed(
		TextX,
		TextY+TextH*0.5,
		TextW,
		TextH*0.5,
		tmp,
		TextH*0.5,
		fgColor,
		canvasColor,
		EM_ALIGN_LEFT
	);

	xy[0]=DetailsFrameX;                 xy[1]=DetailsFrameY;
	xy[2]=DetailsFrameX+DetailsFrameW;   xy[3]=DetailsFrameY;
	xy[4]=DetailsX+DetailsW;             xy[5]=DetailsY;
	xy[6]=DetailsX;                      xy[7]=DetailsY;
	painter.PaintPolygon(xy,4,0x00000030,canvasColor);

	xy[0]=DetailsFrameX;                 xy[1]=DetailsFrameY;
	xy[2]=DetailsX;                      xy[3]=DetailsY;
	xy[4]=DetailsX;                      xy[5]=DetailsY+DetailsH;
	xy[6]=DetailsFrameX;                 xy[7]=DetailsFrameY+DetailsFrameH;
	painter.PaintPolygon(xy,4,0x00000014,canvasColor);

	xy[0]=DetailsX+DetailsW;             xy[1]=DetailsY;
	xy[2]=DetailsFrameX+DetailsFrameW;   xy[3]=DetailsFrameY;
	xy[4]=DetailsFrameX+DetailsFrameW;   xy[5]=DetailsFrameY+DetailsFrameH;
	xy[6]=DetailsX+DetailsW;             xy[7]=DetailsY+DetailsH;
	painter.PaintPolygon(xy,4,0xFFFFFF14,canvasColor);

	xy[0]=DetailsX;                      xy[1]=DetailsY+DetailsH;
	xy[2]=DetailsX+DetailsW;             xy[3]=DetailsY+DetailsH;
	xy[4]=DetailsFrameX+DetailsFrameW;   xy[5]=DetailsFrameY+DetailsFrameH;
	xy[6]=DetailsFrameX;                 xy[7]=DetailsFrameY+DetailsFrameH;
	painter.PaintPolygon(xy,4,0xFFFFFF30,canvasColor);

	x=DetailsX;
	y=DetailsY;
	s=DetailsW;
	h=s*0.48;
	if (h>DetailsH) {
		s*=DetailsH/h;
		x+=(DetailsW-s)*0.5;
	}
	else {
		y+=(DetailsH-h)*0.5;
	}

	if (GetViewedWidth()*s>10.0) {

		bgColor1=emColor(136,0,0);
		fgColor1=emColor(224,224,224);
		bgColor2=fgColor1;
		fgColor2=emColor(0,0,0);

		painter.PaintTextBoxed(
			x,y,s,s*0.1,
			"Target Selection Details",
			s*0.1,
			bgColor1,
			canvasColor
		);

		painter.PaintRoundRect(
			x+s*0.15,
			y+s*0.13,
			s*0.84,
			s*0.34,
			s*0.03,
			s*0.03,
			bgColor2,
			canvasColor
		);

		painter.PaintRoundRectOutline(
			x+s*0.15,
			y+s*0.13,
			s*0.84,
			s*0.34,
			s*0.03,
			s*0.03,
			s*0.01,
			bgColor1
		);

		painter.PaintRoundRect(
			x+s*0.0,
			y+s*0.22,
			s*0.28,
			s*0.16,
			s*0.02,
			s*0.02,
			bgColor1
		);

		if (GetViewedWidth()*s>20.0) {
			PaintDetails(
				painter,
				x+s*0.01,
				y+s*0.23,
				s*0.26,
				s*0.14,
				"Direct",
				DirectDetails,
				fgColor1,
				bgColor1
			);
			PaintDetails(
				painter,
				x+s*0.33,
				y+s*0.15,
				s*0.52,
				s*0.28,
				"Recursive",
				RecursiveDetails,
				fgColor2,
				bgColor2
			);
		}

	}
}


void emFileManSelInfoPanel::PaintDetails(
	const emPainter & painter, double x, double y, double w, double h,
	const char * caption, const DetailsType & details, emColor color,
	emColor canvasColor
)
{
	char tmp[256];
	double d,w2;

	painter.PaintTextBoxed(
		x,y,w,h*0.3,
		caption,
		h*0.3,
		color,
		canvasColor
	);
	y+=h*0.3;
	h-=h*0.3;

	if (details.State!=STATE_SUCCESS) {
		switch (details.State) {
		case STATE_COSTLY:
			strcpy(tmp,"Costly");
			color=color.GetBlended(emColor(136,102,102),50.0F);
			break;
		case STATE_WAIT:
			strcpy(tmp,"Wait...");
			color=color.GetBlended(emColor(136,136,0),50.0F);
			break;
		case STATE_SCANNING:
			strcpy(tmp,"Scanning...");
			color=color.GetBlended(emColor(0,136,0),50.0F);
			break;
		default:
			snprintf(tmp,sizeof(tmp),"ERROR:\n\n%s",details.ErrorMessage.Get());
			tmp[sizeof(tmp)-1]=0;
			color=color.GetBlended(emColor(255,0,0),50.0F);
			break;
		}
		painter.PaintTextBoxed(
			x,y,w,h,tmp,h*0.1,color,canvasColor,EM_ALIGN_CENTER,EM_ALIGN_CENTER
		);
		return;
	}

	d=h/32.0;

	sprintf(tmp,"Entries: %d",details.Entries);
	painter.PaintTextBoxed(x,y,w,d*8,tmp,d*8,color,canvasColor,EM_ALIGN_LEFT);

	sprintf(tmp,"Hidden Entries: %d",details.HiddenEntries);
	painter.PaintTextBoxed(x,y+d*9,w,d*2,tmp,d*2,color,canvasColor,EM_ALIGN_LEFT);

	sprintf(tmp,"Symbolic Links: %d",details.SymbolicLinks);
	painter.PaintTextBoxed(x,y+d*12,w,d*2,tmp,d*2,color,canvasColor,EM_ALIGN_LEFT);

	sprintf(tmp,"Regular Files : %d",details.RegularFiles);
	painter.PaintTextBoxed(x,y+d*14,w,d*2,tmp,d*2,color,canvasColor,EM_ALIGN_LEFT);

	sprintf(tmp,"Subdirectories: %d",details.Subdirectories);
	painter.PaintTextBoxed(x,y+d*16,w,d*2,tmp,d*2,color,canvasColor,EM_ALIGN_LEFT);

	sprintf(tmp,"Other Types   : %d",details.OtherTypes);
	painter.PaintTextBoxed(x,y+d*18,w,d*2,tmp,d*2,color,canvasColor,EM_ALIGN_LEFT);

	strcpy(tmp,"Size: ");
	w2=emPainter::GetTextSize(tmp,d*8);
	if (w2>w*0.5) w2=w*0.5;
	painter.PaintTextBoxed(x,y+d*21,w2,d*8,tmp,d*8,color,canvasColor,EM_ALIGN_LEFT);
	PaintSize(painter,x+w2,y+d*21,w-w2,d*8,details.Size,color,canvasColor);

	strcpy(tmp,"Disk Usage: ");
	if (details.DiskUsageUnknown) {
		strcat(tmp,"unknown");
		painter.PaintTextBoxed(x,y+d*30,w,d*2,tmp,d*2,color,canvasColor,EM_ALIGN_LEFT);
	}
	else {
		w2=emPainter::GetTextSize(tmp,d*2);
		if (w2>w*0.5) w2=w*0.5;
		painter.PaintTextBoxed(x,y+d*30,w2,d*2,tmp,d*2,color,canvasColor,EM_ALIGN_LEFT);
		PaintSize(painter,x+w2,y+d*30,w-w2,d*2,details.DiskUsage,color,canvasColor);
	}
}


void emFileManSelInfoPanel::PaintSize(
	const emPainter & painter, double x, double y, double w, double h,
	emUInt64 size, emColor color, emColor canvasColor
)
{
	emColor unitColor;
	char tmp[128];
	double cw,ws;
	int i,j,k,len;

	len=emUInt64ToStr(tmp,sizeof(tmp),size);
	cw=emPainter::GetTextSize("X",h,false);
	ws=w/(cw*len*16/15);
	if (ws>1.0) ws=1.0;
	unitColor=emColor(color,(emByte)(color.GetAlpha()*2/3));
	for (i=0; i<len; i+=j) {
		j=(len-i)-(len-i-1)/3*3;
		painter.PaintText(x,y,tmp+i,h,ws,color,canvasColor,j);
		x+=cw*j*ws;
		k=(len-i-j)/3-1;
		if (k>=0) {
			painter.PaintText(
				x,
				y+h*0.75,
				&"kMGTPEZY"[k],
				h/5,
				ws,
				color,
				canvasColor,
				1
			);
		}
		x+=cw/5*ws;
	}
}


void emFileManSelInfoPanel::SetRectangles()
{
	double h,useW,useH;

	h=GetHeight();
	if (h < 0.3) {
		useW=1.0;
		useH=0.17;
		if (useH>h) {
			useW*=h/useH;
			useH=h;
		}
		useW-=useH*0.05;
		useW-=useH*0.05;

		TextH=useH;
		TextW=TextH/0.29;
		TextX=(1.0-useW)*0.5;
		TextY=(h-useH)*0.5;

		DetailsFrameH=useH;
		DetailsFrameW=DetailsFrameH/0.56;
		DetailsFrameX=TextX+useW-DetailsFrameW;
		DetailsFrameY=TextY;
	}
	else {
		useW=1.0;
		useH=0.76;
		if (useH>h) {
			useW*=h/useH;
			useH=h;
		}
		useW-=useW*0.05;
		useH-=useH*0.05;

		TextW=useW;
		TextH=TextW*0.29;
		TextX=(1.0-useW)*0.5;
		TextY=(h-useH)*0.5;

		DetailsFrameW=useW;
		DetailsFrameH=DetailsFrameW*0.44;
		DetailsFrameX=TextX;
		DetailsFrameY=TextY+useH-DetailsFrameH;
	}

	DetailsW=DetailsFrameW*0.3;
	DetailsH=DetailsW*0.4667;
	DetailsX=DetailsFrameX+(DetailsFrameW-DetailsW)*0.5;
	DetailsY=DetailsFrameY+(DetailsFrameH-DetailsH)*0.5;
}


void emFileManSelInfoPanel::ResetDetails()
{
	DirectDetails.State=STATE_COSTLY;
	DirectDetails.ErrorMessage.Clear();
	RecursiveDetails.State=STATE_COSTLY;
	RecursiveDetails.ErrorMessage.Clear();
	DirStack.Clear();
	InitialDirStack.Clear();
	SelList.Clear();
	DirPath.Clear();
	if (DirHandle) {
		emCloseDir(DirHandle);
		DirHandle=NULL;
	}
}


bool emFileManSelInfoPanel::WorkOnDetails()
{
	emString name;
	int i,cnt;


	if (!AllowBusiness) {
		switch (DirectDetails.State) {
		case STATE_WAIT:
			DirectDetails.State=STATE_COSTLY;
			InvalidatePainting();
			break;
		case STATE_SCANNING:
			DirectDetails.State=STATE_COSTLY;
			DirStack.Clear();
			SelList.Clear();
			InvalidatePainting();
			break;
		default:
			break;
		}
		switch (RecursiveDetails.State) {
		case STATE_WAIT:
			RecursiveDetails.State=STATE_COSTLY;
			InvalidatePainting();
			break;
		case STATE_SCANNING:
			RecursiveDetails.State=STATE_COSTLY;
			DirStack.Clear();
			DirPath.Clear();
			if (DirHandle) {
				emCloseDir(DirHandle);
				DirHandle=NULL;
			}
			InvalidatePainting();
			break;
		default:
			break;
		}
		return false;
	}

	switch (DirectDetails.State) {
	case STATE_COSTLY:
	case STATE_WAIT:
		DirectDetails.State=STATE_SCANNING;
		DirectDetails.ErrorMessage.Clear();
		DirectDetails.Entries=0;
		DirectDetails.HiddenEntries=0;
		DirectDetails.SymbolicLinks=0;
		DirectDetails.RegularFiles=0;
		DirectDetails.Subdirectories=0;
		DirectDetails.OtherTypes=0;
		DirectDetails.Size=0;
		DirectDetails.DiskUsage=0;
		DirectDetails.DiskUsageUnknown=false;
		RecursiveDetails.State=STATE_WAIT;
		cnt=FileMan->GetTargetSelectionCount();
		SelList.SetCount(cnt);
		for (i=0; i<cnt; i++) {
			SelList.Set(i,FileMan->GetTargetSelection(i));
		}
		DirStack.Clear();
		SelIndex=0;
		InvalidatePainting();
		return true;
	case STATE_SCANNING:
		if (SelIndex>=SelList.GetCount()) {
			DirectDetails.State=STATE_SUCCESS;
			SelList.Clear();
			InitialDirStack=DirStack;
			DirStack.Clear();
			InvalidatePainting();
			return true;
		}
		WorkOnDetailEntry(&DirectDetails,emDirEntry(SelList[SelIndex]));
		SelIndex++;
		if (DirectDetails.State==STATE_ERROR) {
			RecursiveDetails.State=STATE_ERROR;
			RecursiveDetails.ErrorMessage=DirectDetails.ErrorMessage;
			SelList.Clear();
			DirStack.Clear();
			InvalidatePainting();
			return false;
		}
		return true;
	default:
		break;
	}

	switch (RecursiveDetails.State) {
	case STATE_COSTLY:
	case STATE_WAIT:
		RecursiveDetails.State=STATE_SCANNING;
		RecursiveDetails.ErrorMessage.Clear();
		RecursiveDetails.Entries=DirectDetails.Entries;
		RecursiveDetails.HiddenEntries=DirectDetails.HiddenEntries;
		RecursiveDetails.SymbolicLinks=DirectDetails.SymbolicLinks;
		RecursiveDetails.RegularFiles=DirectDetails.RegularFiles;
		RecursiveDetails.Subdirectories=DirectDetails.Subdirectories;
		RecursiveDetails.OtherTypes=DirectDetails.OtherTypes;
		RecursiveDetails.Size=DirectDetails.Size;
		RecursiveDetails.DiskUsage=DirectDetails.DiskUsage;
		RecursiveDetails.DiskUsageUnknown=DirectDetails.DiskUsageUnknown;
		DirStack=InitialDirStack;
		InvalidatePainting();
		return true;
	case STATE_SCANNING:
		if (!DirHandle) {
			cnt=DirStack.GetCount();
			if (cnt<=0) {
				RecursiveDetails.State=STATE_SUCCESS;
				InitialDirStack.Clear();
				InvalidatePainting();
				return false;
			}
			cnt--;
			DirPath=DirStack[cnt];
			DirStack.SetCount(cnt);
			try {
				DirHandle=emTryOpenDir(DirPath);
			}
			catch (const emException & exception) {
				RecursiveDetails.State=STATE_ERROR;
				RecursiveDetails.ErrorMessage=exception.GetText();
				DirStack.Clear();
				InitialDirStack.Clear();
				DirPath.Clear();
				InvalidatePainting();
				return false;
			}
			return true;
		}
		try {
			name=emTryReadDir(DirHandle);
		}
		catch (const emException & exception) {
			RecursiveDetails.State=STATE_ERROR;
			RecursiveDetails.ErrorMessage=exception.GetText();
			DirStack.Clear();
			InitialDirStack.Clear();
			DirPath.Clear();
			emCloseDir(DirHandle);
			DirHandle=NULL;
			InvalidatePainting();
			return false;
		}
		if (name.IsEmpty()) {
			DirPath.Clear();
			emCloseDir(DirHandle);
			DirHandle=NULL;
			return true;
		}
		WorkOnDetailEntry(&RecursiveDetails,emDirEntry(DirPath,name));
		if (RecursiveDetails.State==STATE_ERROR) {
			DirStack.Clear();
			InitialDirStack.Clear();
			DirPath.Clear();
			emCloseDir(DirHandle);
			DirHandle=NULL;
			InvalidatePainting();
			return false;
		}
		return true;
	default:
		break;
	}

	return false;
}


void emFileManSelInfoPanel::WorkOnDetailEntry(DetailsType * details, emDirEntry dirEntry)
{
	if (dirEntry.GetLStatErrNo()!=0) {
		details->State=STATE_ERROR;
		details->ErrorMessage=emString::Format(
			"Failed to lstat \"%s\": %s",
			dirEntry.GetPath().Get(),
			emGetErrorText(dirEntry.GetLStatErrNo()).Get()
		);
	}

	details->Entries++;

	if (dirEntry.IsHidden()) {
		details->HiddenEntries++;
	}

	if (dirEntry.IsSymbolicLink()) {
		details->SymbolicLinks++;
	}
	else if (dirEntry.IsRegularFile()) {
		details->RegularFiles++;
	}
	else if (dirEntry.IsDirectory()) {
		details->Subdirectories++;
		DirStack.Add(dirEntry.GetPath());
	}
	else {
		details->OtherTypes++;
	}

	details->Size+=dirEntry.GetLStat()->st_size;

	#if defined(__linux__)
		details->DiskUsage+=((emUInt64)dirEntry.GetLStat()->st_blocks)*512;
	#else
		details->DiskUsageUnknown=true;
	#endif
}
