//------------------------------------------------------------------------------
// emVirtualCosmos.cpp
//
// Copyright (C) 2007-2009 Oliver Hamann.
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

#include <emMain/emVirtualCosmos.h>
#include <emMain/emStarFieldPanel.h>
#include <emCore/emInstallInfo.h>
#include <emCore/emRes.h>
#include <emCore/emFpPlugin.h>


//==============================================================================
//=========================== emVirtualCosmosItemRec ===========================
//==============================================================================

emVirtualCosmosItemRec::emVirtualCosmosItemRec(const emString & name)
	: emStructRec(),
	Title(this,"Title"),
	PosX(this,"PosX",0.0,0.0,1.0),
	PosY(this,"PosY",0.0,0.0,1.0),
	Width(this,"Width",0.1,1E-10,1.0),
	ContentTallness(this,"ContentTallness",1.0,1E-10,1E+10),
	BorderScaling(this,"BorderScaling",1.0,0.0,1E+10),
	BackgroundColor(this,"BackgroundColor",emColor(0xAAAAAAFF),true),
	BorderColor(this,"BorderColor",emColor(0xAAAAAAFF),true),
	TitleColor(this,"TitleColor",emColor(0x000000FF),true),
	Focusable(this,"Focusable",true),
	FileName(this,"FileName","unnamed"),
	CopyToUser(this,"CopyToUser",false),
	Alternative(this,"Alternative",0,0,INT_MAX)
{
	Name=name;
}


emVirtualCosmosItemRec::~emVirtualCosmosItemRec()
{
}


const char * emVirtualCosmosItemRec::GetFormatName() const
{
	return "emVirtualCosmosItem";
}


void emVirtualCosmosItemRec::TryPrepareItemFile(
	const emString & origDir, const emString & userDir
) throw(emString)
{
	emString srcPath,tgtDir;

	srcPath=emGetChildPath(origDir,FileName.Get());

	if (!CopyToUser.Get()) {
		ItemFilePath=srcPath;
		return;
	}

	ItemFilePath=emGetChildPath(userDir,FileName.Get());

	if (!emIsExistingPath(ItemFilePath)) {
		emTryMakeDirectories(userDir);
		emTryCopyFileOrTree(ItemFilePath,srcPath);
	}
}


//==============================================================================
//============================ emVirtualCosmosModel ============================
//==============================================================================

emRef<emVirtualCosmosModel> emVirtualCosmosModel::Acquire(
	emRootContext & rootContext
)
{
	EM_IMPL_ACQUIRE_COMMON(emVirtualCosmosModel,rootContext,"")
}


emVirtualCosmosModel::emVirtualCosmosModel(
	emContext & context, const emString & name
)
	: emModel(context,name)
{
	ItemRecs.SetTuningLevel(4);
	FileUpdateSignalModel=emFileModel::AcquireUpdateSignalModel(GetRootContext());
	AddWakeUpSignal(FileUpdateSignalModel->Sig);
	Reload();
}


emVirtualCosmosModel::~emVirtualCosmosModel()
{
	int i;

	for (i=Items.GetCount()-1; i>=0; i--) {
		if (Items[i].ItemRec) delete Items[i].ItemRec;
	}
}


bool emVirtualCosmosModel::Cycle()
{
	if (IsSignaled(FileUpdateSignalModel->Sig)) Reload();
	return emModel::Cycle();
}


void emVirtualCosmosModel::Reload()
{
	static const char * const itemExt = ".emVcItem";
	emArray<emString> fileNames;
	emString itemsDir,itemFilesDir,itemFilesUserDir,path;
	time_t mt;
	bool updateAll;
	int i,j,nameLen,changed;

	changed=0;

	// Note: Before Eagle Mode 0.72.0, we had a directory
	// "~/.eaglemode/emMain/VcFiles". It should never be used again, to
	// avoid conflicts.
	itemsDir=emGetConfigDirOverloadable(GetRootContext(),"emMain","VcItems");
	itemFilesDir=emGetConfigDirOverloadable(GetRootContext(),"emMain","VcItemFiles");
	itemFilesUserDir=emGetInstallPath(EM_IDT_USER_CONFIG,"emMain","VcItemFiles.user");

	if (ItemsDir!=itemsDir || ItemFilesDir!=itemFilesDir) {
		ItemsDir=itemsDir;
		ItemFilesDir=itemFilesDir;
		updateAll=true;
	}
	else {
		updateAll=false;
	}

	try {
		fileNames=emTryLoadDir(itemsDir);
	}
	catch (emString errorMessage) {
		emFatalError("%s",errorMessage.Get());
	}
	fileNames.Sort(emStdComparer<emString>::Compare);

	i=fileNames.GetCount()-1;
	j=Items.GetCount();
	for (; i>=0; i--) {
		nameLen=fileNames[i].GetLen()-strlen(itemExt);
		if (nameLen<=0 || strcasecmp(fileNames[i].Get()+nameLen,itemExt)!=0) {
			continue;
		}
		path=emGetChildPath(itemsDir,fileNames[i]);
		try {
			mt=emTryGetFileTime(path);
		}
		catch (emString errorMessage) {
			emFatalError("%s",errorMessage.Get());
			mt=0;
		}
		for (;;) {
			j--;
			if (j>=0 && fileNames[i]==Items[j].FileName) {
				break;
			}
			if (j<0 || fileNames[i]>Items[j].FileName) {
				j++;
				Items.InsertNew(j);
				Items.GetWritable(j).FileName=fileNames[i];
				Items.GetWritable(j).ItemRec=NULL;
				changed|=1;
				break;
			}
			if (Items[j].ItemRec) delete Items[j].ItemRec;
			Items.Remove(j);
			changed=2;
		}
		if (updateAll || !Items[j].ItemRec || Items[j].MTime!=mt) {
			Items.GetWritable(j).MTime=mt;
			if (!Items[j].ItemRec) {
				Items.GetWritable(j).ItemRec=new emVirtualCosmosItemRec(
					fileNames[i].GetSubString(0,nameLen)
				);
			}
			try {
				Items[j].ItemRec->TryLoad(path);
				Items[j].ItemRec->TryPrepareItemFile(itemFilesDir,itemFilesUserDir);
			}
			catch (emString errorMessage) {
				emWarning("%s",errorMessage.Get());
				delete Items[j].ItemRec;
				Items.Remove(j);
				if (changed==1) changed=0; else changed=2;
				continue;
			}
			changed=2;
		}
	}
	while (j>0) {
		j--;
		if (Items[j].ItemRec) delete Items[j].ItemRec;
		Items.Remove(j);
		changed=2;
	}
	fileNames.Empty(true);
	if (changed) {
		ItemRecs.SetCount(Items.GetCount());
		for (i=Items.GetCount()-1; i>=0; i--) {
			ItemRecs.Set(i,Items[i].ItemRec);
		}
		ItemRecs.Sort(CompareItemRecs,this);
		Signal(ChangeSignal);
	}
}


int emVirtualCosmosModel::CompareItemRecs(
	emVirtualCosmosItemRec * const * pItemRec1,
	emVirtualCosmosItemRec * const * pItemRec2,
	void * model
)
{
	emVirtualCosmosItemRec * itemRec1, * itemRec2;

	itemRec1=*pItemRec1;
	itemRec2=*pItemRec2;
	if (itemRec1->PosY<itemRec2->PosY) return -1;
	if (itemRec1->PosY>itemRec2->PosY) return 1;
	if (itemRec1->PosX<itemRec2->PosX) return -1;
	if (itemRec1->PosX>itemRec2->PosX) return 1;
	return 0;
}


//==============================================================================
//========================== emVirtualCosmosItemPanel ==========================
//==============================================================================

emVirtualCosmosItemPanel::emVirtualCosmosItemPanel(
	ParentArg parent, const emString & name
)
	: emPanel(parent,name)
{
	ContentPanel=NULL;
	Path="";
	Alt=0;
	ItemFocusable=true;
	UpdateFromRecNeeded=true;

	OuterBorderImage=emGetInsResImage(
		GetRootContext(),"emMain","VcItemOuterBorder.tga"
	);
	InnerBorderImage=emGetInsResImage(
		GetRootContext(),"emMain","VcItemInnerBorder.tga"
	);

	EnableAutoExpansion();
	WakeUp();
}


emVirtualCosmosItemPanel::~emVirtualCosmosItemPanel()
{
}


void emVirtualCosmosItemPanel::SetItemRec(emVirtualCosmosItemRec * itemRec)
{
	if (GetItemRec()!=itemRec) {
		SetListenedRec(itemRec);
		UpdateFromRecNeeded=true;
		WakeUp();
	}
}


emString emVirtualCosmosItemPanel::GetTitle()
{
	emVirtualCosmosItemRec * itemRec;

	itemRec=GetItemRec();
	if (itemRec && !itemRec->Title.Get().IsEmpty()) {
		return itemRec->Title.Get();
	}
	else {
		return emPanel::GetTitle();
	}
}


bool emVirtualCosmosItemPanel::Cycle()
{
	if (UpdateFromRecNeeded) UpdateFromRec();
	return emPanel::Cycle();
}


bool emVirtualCosmosItemPanel::IsOpaque()
{
	emVirtualCosmosItemRec * itemRec;

	itemRec=GetItemRec();
	if (!itemRec) return false;

	return
		itemRec->BackgroundColor.Get().IsOpaque() &&
		( itemRec->BorderColor.Get().IsOpaque() || itemRec->BorderScaling.Get()<=1E-200 )
	;
}


void emVirtualCosmosItemPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	double xy[10*2];
	emVirtualCosmosItemRec * itemRec;
	emColor borCol;
	emString str;
	double h,l,t,r,b,x1,y1,x2,y2,d,e,f;

	itemRec=GetItemRec();
	if (!itemRec) return;

	if (itemRec->BorderScaling.Get()<=1E-100) {
		painter.Clear(itemRec->BackgroundColor,canvasColor);
		return;
	}

	borCol=itemRec->BorderColor;
	h=GetHeight();
	CalcBorders(&l,&t,&r,&b);

	if (borCol==itemRec->BackgroundColor) {
		painter.Clear(itemRec->BackgroundColor,canvasColor);
	}
	else {
		x1=l;
		x2=1.0-r;
		y1=t;
		y2=h-b;
		if (borCol.IsOpaque()) {
			x1=painter.RoundDownX(x1);
			y1=painter.RoundDownY(y1);
			x2=painter.RoundUpX(x2);
			y2=painter.RoundUpY(y2);
		}
		painter.PaintRect(
			x1,y1,x2-x1,y2-y1,
			itemRec->BackgroundColor,canvasColor
		);
		xy[ 0]=0.0;   xy[ 1]=0.0;
		xy[ 2]=1.0;   xy[ 3]=0.0;
		xy[ 4]=1.0;   xy[ 5]=h;
		xy[ 6]=0.0;   xy[ 7]=h;
		xy[ 8]=0.0;   xy[ 9]=0.0;
		xy[10]=l;     xy[11]=t;
		xy[12]=l;     xy[13]=h-b;
		xy[14]=1.0-r; xy[15]=h-b;
		xy[16]=1.0-r; xy[17]=t;
		xy[18]=l;     xy[19]=t;
		painter.PaintPolygon(xy,10,borCol,0);
	}

	d=l*0.4;
	painter.PaintBorderImage(
		0.0,0.0,1.0,h,
		d,d,d,d,
		OuterBorderImage,
		82.0,82.0,82.0,82.0,
		255,borCol,0757
	);

	e=l*0.5;
	f=e*(23.0/126);
	painter.PaintBorderImage(
		l-e,t-f,1.0-l-r+e*2.0,h-t-b+f*2.0,
		e,f,e,f,
		InnerBorderImage,
		126.0,23.0,126.0,23.0,
		255,borCol,0757
	);

	painter.PaintTextBoxed(
		d,
		d+(t-d-f)*0.07,
		1.0-d*2.0,
		(t-d-f)*0.8,
		itemRec->Title.Get(),
		h,
		itemRec->TitleColor,
		borCol,
		EM_ALIGN_CENTER,
		EM_ALIGN_CENTER
	);
}


void emVirtualCosmosItemPanel::AutoExpand()
{
	emVirtualCosmosItemRec * itemRec;
	emRef<emFpPluginList> fppl;

	emPanel::AutoExpand();
	itemRec=GetItemRec();
	if (!itemRec) return;
	fppl=emFpPluginList::Acquire(GetRootContext());
	ContentPanel=fppl->CreateFilePanel(this,"",Path,Alt);
	if (IsActive()) {
		LayoutContentPanel();
		if (ContentPanel->IsViewed()) {
			GetView().VisitLazy(ContentPanel,GetView().IsVisitAdherent());
		}
	}
	SetFocusable(false);
	if (!ItemFocusable) {
		ContentPanel->SetFocusable(false);
	}
}


void emVirtualCosmosItemPanel::AutoShrink()
{
	SetFocusable(ItemFocusable);
	ContentPanel=NULL;
	emPanel::AutoShrink();
}


void emVirtualCosmosItemPanel::LayoutChildren()
{
	LayoutContentPanel();
}


void emVirtualCosmosItemPanel::OnRecChanged()
{
	UpdateFromRecNeeded=true;
	WakeUp();
}


void emVirtualCosmosItemPanel::CalcBorders(
	double * pL, double * pT, double * pR, double * pB
)
{
	emVirtualCosmosItemRec * itemRec;
	double t,bs,b;

	itemRec=GetItemRec();
	if (!itemRec) {
		t=1.0;
		bs=1.0;
	}
	else {
		t=itemRec->ContentTallness;
		bs=itemRec->BorderScaling;
	}
	b=emMin(1.0,t)*bs;
	*pL=b*0.03;
	*pT=b*0.05;
	*pR=b*0.03;
	*pB=b*0.03;
}


void emVirtualCosmosItemPanel::UpdateFromRec()
{
	emVirtualCosmosItemRec * itemRec;
	emString newPath;
	int newAlt;
	bool newFoc;
	double l,t,r,b,x,y,w,h;

	UpdateFromRecNeeded=false;
	itemRec=GetItemRec();
	if (!itemRec) return;
	x=itemRec->PosX;
	y=itemRec->PosY;
	w=itemRec->Width;
	CalcBorders(&l,&t,&r,&b);
	h=w*((1.0-l-r)*itemRec->ContentTallness+t+b);
	Layout(x,y,w,h);
	InvalidateTitle();
	InvalidatePainting();
	InvalidateChildrenLayout();
	newPath=itemRec->GetItemFilePath();
	newAlt=itemRec->Alternative.Get();
	newFoc=itemRec->Focusable.Get();
	if (Path!=newPath || Alt!=newAlt || ItemFocusable!=newFoc) {
		Path=newPath;
		Alt=newAlt;
		ItemFocusable=newFoc;
		if (!ContentPanel) SetFocusable(ItemFocusable);
		InvalidateAutoExpansion();
	}
}


void emVirtualCosmosItemPanel::LayoutContentPanel()
{
	emVirtualCosmosItemRec * itemRec;
	double l,t,r,b,x,y,w,h;
	emColor cc;

	if (!ContentPanel) return;
	itemRec=GetItemRec();
	if (!itemRec) return;
	cc=itemRec->BackgroundColor.Get();
	if (cc.IsTotallyTransparent()) cc=GetCanvasColor();
	CalcBorders(&l,&t,&r,&b);
	x=l;
	y=t;
	w=1.0-l-r;
	h=w*itemRec->ContentTallness;
	ContentPanel->Layout(x,y,w,h,cc);
}


//==============================================================================
//============================ emVirtualCosmosPanel ============================
//==============================================================================

emVirtualCosmosPanel::emVirtualCosmosPanel(
	ParentArg parent, const emString & name
)
	: emPanel(parent,name)
{
	Model=emVirtualCosmosModel::Acquire(GetRootContext());
	BackgroundPanel=NULL;
	AddWakeUpSignal(Model->GetChangeSignal());
}


emVirtualCosmosPanel::~emVirtualCosmosPanel()
{
}


emString emVirtualCosmosPanel::GetTitle()
{
	return emString("Virtual Cosmos");
}


bool emVirtualCosmosPanel::Cycle()
{
	if (IsSignaled(Model->GetChangeSignal())) {
		UpdateChildren();
	}
	return emPanel::Cycle();
}


void emVirtualCosmosPanel::Notice(NoticeFlags flags)
{
	if (flags&NF_VIEWING_CHANGED) {
		UpdateChildren();
	}
}


bool emVirtualCosmosPanel::IsOpaque()
{
	return true; // By the background panel.
}


void emVirtualCosmosPanel::UpdateChildren()
{
	emArray<emVirtualCosmosItemRec*> itemRecs;
	emVirtualCosmosItemRec * itemRec;
	emPanel * p, * np, * q;
	int i;

	itemRecs=Model->GeItemRecs();

	if (!IsViewed() && (!BackgroundPanel || !BackgroundPanel->IsInViewedPath())) {
		for (p=GetFirstChild(); p; p=np) {
			np=p->GetNext();
			if (p->IsInViewedPath()) {
				if (p==BackgroundPanel) continue;
				for (i=itemRecs.GetCount()-1; i>=0; i--) {
					if (p->GetName()==itemRecs[i]->GetName()) break;
				}
				if (i>=0) {
					((emVirtualCosmosItemPanel*)p)->SetItemRec(itemRecs[i]);
					continue;
				}
			}
			if (BackgroundPanel==p) BackgroundPanel=NULL;
			delete p;
		}
		return;
	}

	if (!BackgroundPanel) {
		BackgroundPanel=new emStarFieldPanel(*this,"_StarField");
		BackgroundPanel->SetFocusable(false);
	}
	BackgroundPanel->Layout(0.0,0.0,1.0,1.0,0);
	BackgroundPanel->BeFirst();
	p=BackgroundPanel->GetNext();
	for (i=0; i<itemRecs.GetCount(); i++) {
		itemRec=itemRecs[i];
		if (!p || p->GetName()!=itemRec->GetName()) {
			q=GetChild(itemRec->GetName());
			if (q) {
				while (p!=q) {
					np=p->GetNext();
					delete p;
					p=np;
				}
			}
			else {
				q=new emVirtualCosmosItemPanel(
					this,
					itemRec->GetName()
				);
				if (p) q->BePrevOf(p);
				p=q;
			}
		}
		((emVirtualCosmosItemPanel*)p)->SetItemRec(itemRec);
		p=p->GetNext();
	}
	while (p) {
		np=p->GetNext();
		delete p;
		p=np;
	}
}
