//------------------------------------------------------------------------------
// emPanel.cpp
//
// Copyright (C) 2004-2008,2011 Oliver Hamann.
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

#include <emCore/emList.h>
#include <emCore/emPanel.h>


emPanel::emPanel(ParentArg parent, const emString & name)
	: emEngine(parent.GetView().GetScheduler()),
	View(parent.GetView()),
	Name(name)
{

#	define EM_PANEL_DEFAULT_AE_THRESHOLD 100.0 //???

	if (parent.GetPanel()) {
		AvlTree=NULL;
		Parent=parent.GetPanel();
		FirstChild=NULL;
		LastChild=NULL;
		Prev=Parent->LastChild;
		Next=NULL;
		if (Prev) Prev->Next=this; else Parent->FirstChild=this;
		Parent->LastChild=this;
		NoticeNode.Prev=NULL;
		NoticeNode.Next=NULL;
		LayoutX=-2;
		LayoutY=-2;
		LayoutWidth=1;
		LayoutHeight=1;
		ViewedX=-1;
		ViewedY=-1;
		ViewedWidth=1;
		ViewedHeight=1;
		ClipX1=0;
		ClipY1=0;
		ClipX2=0;
		ClipY2=0;
		AEThresholdValue=EM_PANEL_DEFAULT_AE_THRESHOLD;
		CreationNumber=++View.PanelCreationNumber;
		CanvasColor=0;
		PendingNoticeFlags=0;
		Viewed=0;
		InViewedPath=0;
		EnableSwitch=1;
		Enabled=Parent->Enabled;
		Focusable=1;
		Visited=0;
		InVisitedPath=0;
		Active=0;
		InActivePath=0;
		PendingInput=0;
		ChildrenLayoutInvalid=0;
		AEEnabled=0;
		AEDecisionInvalid=0;
		AECalling=0;
		AEExpanded=0;
		CreatedByAE=Parent->AECalling;
		AEThresholdType=VCT_AREA;
		Parent->AvlInsertChild(this);
		Parent->AddPendingNotice(NF_CHILD_LIST_CHANGED);
		AddPendingNotice(
			NF_CHILD_LIST_CHANGED |
			NF_LAYOUT_CHANGED |
			NF_VIEWING_CHANGED |
			NF_ENABLE_CHANGED |
			NF_VISIT_CHANGED |
			NF_ACTIVE_CHANGED |
			NF_FOCUS_CHANGED |
			NF_VIEW_FOCUS_CHANGED |
			NF_UPDATE_PRIORITY_CHANGED |
			NF_MEMORY_LIMIT_CHANGED |
			NF_SOUGHT_NAME_CHANGED
		);
	}
	else {
		if (View.RootPanel) {
			emFatalError(
				"Root panel created for an emView which has already a root panel."
			);
		}
		View.RootPanel=this;
		View.SupremeViewedPanel=this;
		View.MinSVP=this;
		View.MaxSVP=this;
		View.VisitedPanel=this;
		View.ActivePanel=this;
		AvlTree=NULL;
		Parent=NULL;
		FirstChild=NULL;
		LastChild=NULL;
		Prev=NULL;
		Next=NULL;
		NoticeNode.Prev=NULL;
		NoticeNode.Next=NULL;
		LayoutX=0;
		LayoutY=0;
		LayoutWidth=1.0;
		LayoutHeight=View.GetHomeTallness();
		ViewedX=View.CurrentX;
		ViewedY=View.CurrentY;
		ViewedWidth=View.CurrentWidth;
		ViewedHeight=View.CurrentHeight;
		ClipX1=ViewedX;
		ClipY1=ViewedY;
		ClipX2=ViewedX+ViewedWidth;
		ClipY2=ViewedY+ViewedHeight;
		AEThresholdValue=EM_PANEL_DEFAULT_AE_THRESHOLD;
		CreationNumber=++View.PanelCreationNumber;
		CanvasColor=0;
		PendingNoticeFlags=0;
		Viewed=1;
		InViewedPath=1;
		EnableSwitch=1;
		Enabled=1;
		Focusable=1;
		Visited=1;
		InVisitedPath=1;
		Active=1;
		InActivePath=1;
		PendingInput=0;
		ChildrenLayoutInvalid=0;
		AEEnabled=0;
		AEDecisionInvalid=0;
		AECalling=0;
		AEExpanded=0;
		CreatedByAE=0;
		AEThresholdType=VCT_AREA;
		InvalidatePainting();
		AddPendingNotice(
			NF_CHILD_LIST_CHANGED |
			NF_LAYOUT_CHANGED |
			NF_VIEWING_CHANGED |
			NF_ENABLE_CHANGED |
			NF_VISIT_CHANGED |
			NF_ACTIVE_CHANGED |
			NF_FOCUS_CHANGED |
			NF_VIEW_FOCUS_CHANGED |
			NF_UPDATE_PRIORITY_CHANGED |
			NF_MEMORY_LIMIT_CHANGED |
			NF_SOUGHT_NAME_CHANGED
		);
		View.TitleInvalid=true;
		View.CursorInvalid=true;
		View.UpdateEngine->WakeUp();
	}
}


emPanel::~emPanel()
{
	InvalidatePainting();
	if (View.SeekPosPanel==this) View.SetSeekPos(NULL,NULL);
	DeleteAllChildren();
	if (!Parent) {
		if (View.IsPoppedUp()) View.ZoomOut();
		View.RootPanel=NULL;
		View.SupremeViewedPanel=NULL;
		View.MinSVP=NULL;
		View.MaxSVP=NULL;
		View.VisitedPanel=NULL;
		View.ActivePanel=NULL;
		View.VisitAdherent=false;
		View.TitleInvalid=true;
		View.CursorInvalid=true;
		View.UpdateEngine->WakeUp();
	}
	else {
		if (InVisitedPath) {
			if (Parent->Viewed) {
				Focusable=true;
				View.VisitImmobile(Parent,false);
			}
			else {
				LayoutX=-2.0;
				LayoutY=-2.0;
				LayoutWidth=1.0;
				LayoutHeight=1.0;
				CanvasColor=0;
				Focusable=true;
				View.ProtectSeeking++;
				View.VisitFullsized(Parent,!InActivePath && View.VisitAdherent);
				View.ProtectSeeking--;
			}
			if (InVisitedPath) {
				emFatalError("emPanel::~emPanel: Could not to get rid of the visit.");
			}
		}
		View.RestartInputRecursion=true;
		if (InViewedPath) {
			View.SVPChoiceInvalid=true;
			View.TitleInvalid=true;
			View.CursorInvalid=true;
			View.UpdateEngine->WakeUp();
		}
		Parent->AvlRemoveChild(this);
		Parent->AddPendingNotice(NF_CHILD_LIST_CHANGED);
		if (Next) Next->Prev=Prev;
		else Parent->LastChild=Prev;
		if (Prev) Prev->Next=Next;
		else Parent->FirstChild=Next;
		Next=NULL;
		Prev=NULL;
	}
	if (NoticeNode.Next) {
		NoticeNode.Next->Prev=NoticeNode.Prev;
		NoticeNode.Prev->Next=NoticeNode.Next;
		NoticeNode.Next=NULL;
		NoticeNode.Prev=NULL;
	}
	if (View.ActivationCandidate==this) {
		View.SetActivationCandidate(NULL);
	}
}


emString emPanel::GetIdentity() const
{
	const emPanel * p;
	emArray<emString> a;
	int i;

	p=this;
	i=0;
	do {
		i++;
		p=p->Parent;
	} while (p);
	a.SetTuningLevel(1);
	a.SetCount(i);
	p=this;
	do {
		i--;
		a.Set(i,p->Name);
		p=p->Parent;
	} while (p);
	return EncodeIdentity(a);
}


emString emPanel::EncodeIdentity(const emArray<emString> & names)
{
	emString res;
	const char * r, * s;
	char * t;
	int i,cnt,len;
	char c;

	cnt=names.GetCount();
	len=cnt-1;
	for (i=0; i<cnt; i++) {
		r=s=names[i].Get();
		c=*s;
		if (c) {
			do {
				if (c==':' || c=='\\') len++;
				c=*++s;
			} while (c);
			len+=s-r;
		}
	}
	t=res.SetLenGetWritable(len);
	for (i=0; i<cnt; i++) {
		if (i>0) *t++=':';
		s=names[i].Get();
		c=*s;
		while (c) {
			if (c==':' || c=='\\') *t++='\\';
			*t++=c;
			c=*++s;
		}
	}
	return res;
}


emArray<emString> emPanel::DecodeIdentity(const char * identity)
{
	emArray<emString> a;
	const char * p, * t;
	char * s;
	int i,q;

	a.SetTuningLevel(1);
	p=identity;
	for (i=0; ; i++, p++) {
		a.SetCount(i+1);
		if (!*p) break;
		if (*p==':') continue;
		t=p;
		q=0;
		do {
			if (*t=='\\') {
				t++;
				q++;
				if (!*t) break;
			}
			t++;
		} while (*t && *t!=':');
		s=a.GetWritable(i).SetLenGetWritable(t-p-q);
		do {
			if (*p=='\\') {
				p++;
				if (!*p) break;
			}
			*s++=*p++;
		} while (*p && *p!=':');
		if (!*p) break;
	}
	return a;
}


emString emPanel::GetTitle()
{
	if (Parent) return Parent->GetTitle();
	else return "untitled";
}


emPanel * emPanel::GetChild(const char * name)
{
	EM_AVL_SEARCH_VARS(emPanel)
	int d;

	EM_AVL_SEARCH_BEGIN(emPanel,AvlNode,AvlTree)
		d=strcmp(name,element->Name.Get());
		if (d<0) EM_AVL_SEARCH_GO_LEFT
		else if (d>0) EM_AVL_SEARCH_GO_RIGHT
	EM_AVL_SEARCH_END
	return element;
}


void emPanel::BeFirst()
{
	if (Prev) {
		Prev->Next=Next;
		if (Next) Next->Prev=Prev;
		else Parent->LastChild=Prev;
		Prev=NULL;
		Next=Parent->FirstChild;
		Next->Prev=this;
		Parent->FirstChild=this;
		Parent->AddPendingNotice(NF_CHILD_LIST_CHANGED);
		View.RestartInputRecursion=true;
		if (InViewedPath) {
			InvalidatePainting();
			View.SVPChoiceInvalid=true;
			View.CursorInvalid=true;
			View.UpdateEngine->WakeUp();
		}
	}
}


void emPanel::BeLast()
{
	if (Next) {
		Next->Prev=Prev;
		if (Prev) Prev->Next=Next;
		else Parent->FirstChild=Next;
		Next=NULL;
		Prev=Parent->LastChild;
		Prev->Next=this;
		Parent->LastChild=this;
		Parent->AddPendingNotice(NF_CHILD_LIST_CHANGED);
		View.RestartInputRecursion=true;
		if (Parent->InViewedPath) {
			InvalidatePainting();
			View.SVPChoiceInvalid=true;
			View.CursorInvalid=true;
			View.UpdateEngine->WakeUp();
		}
	}
}


void emPanel::BePrevOf(emPanel * sister)
{
	if (!sister) {
		BeLast();
		return;
	}
	if (sister==this || sister==Next || sister->Parent!=Parent) return;
	if (Prev) Prev->Next=Next;
	else Parent->FirstChild=Next;
	if (Next) Next->Prev=Prev;
	else Parent->LastChild=Prev;
	Next=sister;
	Prev=sister->Prev;
	sister->Prev=this;
	if (Prev) Prev->Next=this; else Parent->FirstChild=this;
	Parent->AddPendingNotice(NF_CHILD_LIST_CHANGED);
	View.RestartInputRecursion=true;
	if (Parent->InViewedPath) {
		InvalidatePainting();
		View.SVPChoiceInvalid=true;
		View.CursorInvalid=true;
		View.UpdateEngine->WakeUp();
	}
}


void emPanel::BeNextOf(emPanel * sister)
{
	if (!sister) {
		BeFirst();
		return;
	}
	if (sister==this || sister==Prev || sister->Parent!=Parent) return;
	if (Next) Next->Prev=Prev;
	else Parent->LastChild=Prev;
	if (Prev) Prev->Next=Next;
	else Parent->FirstChild=Next;
	Prev=sister;
	Next=sister->Next;
	sister->Next=this;
	if (Next) Next->Prev=this; else Parent->LastChild=this;
	Parent->AddPendingNotice(NF_CHILD_LIST_CHANGED);
	View.RestartInputRecursion=true;
	if (Parent->InViewedPath) {
		InvalidatePainting();
		View.SVPChoiceInvalid=true;
		View.CursorInvalid=true;
		View.UpdateEngine->WakeUp();
	}
}


void emPanel::SortChildren(
	int(*compare)(emPanel * c1, emPanel * c2, void * context),
	void * context
)
{
	if (
		emSortDoubleLinkedList(
			(void**)(void*)&FirstChild,
			(void**)(void*)&LastChild,
			offsetof(emPanel,Next),
			offsetof(emPanel,Prev),
			(int(*)(void*,void*,void*))compare,
			context
		)
	) {
		AddPendingNotice(NF_CHILD_LIST_CHANGED);
		View.RestartInputRecursion=true;
		if (InViewedPath) {
			InvalidatePainting();
			View.SVPChoiceInvalid=true;
			View.CursorInvalid=true;
			View.UpdateEngine->WakeUp();
		}
	}
}


void emPanel::DeleteAllChildren()
{
	while (LastChild) delete LastChild;
}


void emPanel::Layout(
	double layoutX, double layoutY, double layoutWidth, double layoutHeight,
	emColor canvasColor
)
{
	emPanel * p;
	double x1,y1,x2,y2,rx,ry,ra,ra2;
	bool adherent,zoomedOut;

	if (LayoutWidth<1E-100) LayoutWidth=1E-100;
	if (LayoutHeight<1E-100) LayoutHeight=1E-100;

	if (!Parent) {
		layoutX=0.0;
		layoutY=0.0;
		if ((View.VFlags&emView::VF_ROOT_SAME_TALLNESS)!=0) {
			layoutHeight=View.GetHomeTallness();
		}
		else {
			layoutHeight/=layoutWidth;
		}
		layoutWidth=1.0;
	}

	if (
		LayoutX==layoutX && LayoutY==layoutY &&
		LayoutWidth==layoutWidth && LayoutHeight==layoutHeight
	) {
		if (CanvasColor!=canvasColor) {
			CanvasColor=canvasColor;
			AddPendingNotice(NF_LAYOUT_CHANGED);
			InvalidatePainting();
		}
		return;
	}

	AddPendingNotice(NF_LAYOUT_CHANGED);
	View.RestartInputRecursion=true;
	if (!Parent || Parent->InViewedPath) {
		InvalidatePainting();
		View.SVPChoiceInvalid=true;
		View.CursorInvalid=true;
		View.UpdateEngine->WakeUp();
	}

	if (!Parent) {
		zoomedOut=View.IsZoomedOut();
		p=View.GetVisitedPanel(&rx,&ry,&ra,&adherent);
		LayoutX=layoutX;
		LayoutY=layoutY;
		LayoutWidth=layoutWidth;
		LayoutHeight=layoutHeight;
		CanvasColor=canvasColor;
		if (!View.SettingGeometry) {
			if (zoomedOut) {
				ra=View.HomeWidth*GetHeight()/View.HomePixelTallness/View.HomeHeight;
				ra2=View.HomeHeight/GetHeight()*View.HomePixelTallness/View.HomeWidth;
				if (ra<ra2) ra=ra2;
				View.VisitRelBy(this,0.0,0.0,ra,true);
			}
			else if (p) {
				View.VisitRel(p,rx,ry,ra,adherent,true);
			}
		}
	}
	else if (InVisitedPath && !View.SettingGeometry && !View.IsZoomedOut()) {
		p=View.GetVisitedPanel(&rx,&ry,&ra,&adherent);
		LayoutX=layoutX;
		LayoutY=layoutY;
		LayoutWidth=layoutWidth;
		LayoutHeight=layoutHeight;
		CanvasColor=canvasColor;
		View.VisitRel(p,rx,ry,ra,adherent,true);
	}
	else if (Parent->Viewed) {
		LayoutX=layoutX;
		LayoutY=layoutY;
		LayoutWidth=layoutWidth;
		LayoutHeight=layoutHeight;
		CanvasColor=canvasColor;
		x1=Parent->ViewedX+LayoutX*Parent->ViewedWidth;
		x2=LayoutWidth*Parent->ViewedWidth;
		y1=Parent->ViewedY+LayoutY*(Parent->ViewedWidth/View.CurrentPixelTallness);
		y2=LayoutHeight*(Parent->ViewedWidth/View.CurrentPixelTallness);
		ViewedX=x1;
		ViewedY=y1;
		ViewedWidth=x2;
		ViewedHeight=y2;
		x2+=x1;
		y2+=y1;
		if (x1<Parent->ClipX1) x1=Parent->ClipX1;
		if (x2>Parent->ClipX2) x2=Parent->ClipX2;
		if (y1<Parent->ClipY1) y1=Parent->ClipY1;
		if (y2>Parent->ClipY2) y2=Parent->ClipY2;
		ClipX1=x1;
		ClipX2=x2;
		ClipY1=y1;
		ClipY2=y2;
		if (x1<x2 && y1<y2) {
			InViewedPath=1;
			Viewed=1;
			AddPendingNotice(
				NF_VIEWING_CHANGED |
				NF_UPDATE_PRIORITY_CHANGED |
				NF_MEMORY_LIMIT_CHANGED
			);
			InvalidatePainting();
			UpdateChildrenViewing();
		}
		else if (InViewedPath) {
			InViewedPath=0;
			Viewed=0;
			AddPendingNotice(
				NF_VIEWING_CHANGED |
				NF_UPDATE_PRIORITY_CHANGED |
				NF_MEMORY_LIMIT_CHANGED
			);
			UpdateChildrenViewing();
		}
	}
	else {
		LayoutX=layoutX;
		LayoutY=layoutY;
		LayoutWidth=layoutWidth;
		LayoutHeight=layoutHeight;
		CanvasColor=canvasColor;
	}
}


void emPanel::GetEssenceRect(
	double * pX, double * pY, double * pW, double * pH
)
{
	*pX=0.0;
	*pY=0.0;
	*pW=1.0;
	*pH=GetHeight();
}


double emPanel::GetViewCondition(ViewConditionType vcType) const
{
	if (Viewed) {
		switch (vcType) {
			case VCT_AREA   : return ViewedWidth*ViewedHeight;
			case VCT_WIDTH  : return ViewedWidth;
			case VCT_HEIGHT : return ViewedHeight;
			case VCT_MIN_EXT: return emMin(ViewedWidth, ViewedHeight);
			case VCT_MAX_EXT: return emMax(ViewedWidth, ViewedHeight);
		};
	}
	else if (InViewedPath) {
		return 1E100;
	}
	return 0.0;
}


void emPanel::SetAutoExpansionThreshold(
	double thresholdValue, ViewConditionType vcType
)
{
	if (
		AEThresholdValue!=thresholdValue ||
		((ViewConditionType)AEThresholdType)!=vcType
	) {
		AEThresholdValue=thresholdValue;
		AEThresholdType=vcType;
		AEDecisionInvalid=1;
		if (!NoticeNode.Next) View.AddToNoticeList(&NoticeNode);
	}
}


void emPanel::SetEnableSwitch(bool enableSwitch)
{
	emPanel * p;

	if (enableSwitch) {
		if (!EnableSwitch) {
			EnableSwitch=1;
			if (!Parent || Parent->Enabled) {
				p=this;
				for (;;) {
					if (p->EnableSwitch) {
						p->Enabled=1;
						p->AddPendingNotice(NF_ENABLE_CHANGED);
						if (p->FirstChild) {
							p=p->FirstChild;
							continue;
						}
					}
					while (p!=this && !p->Next) {
						p=p->Parent;
					}
					if (p==this) break;
					p=p->Next;
				}
			}
		}
	}
	else {
		if (EnableSwitch) {
			EnableSwitch=0;
			p=this;
			for (;;) {
				if (p->Enabled) {
					p->Enabled=0;
					p->AddPendingNotice(NF_ENABLE_CHANGED);
					if (p->FirstChild) {
						p=p->FirstChild;
						continue;
					}
				}
				while (p!=this && !p->Next) {
					p=p->Parent;
				}
				if (p==this) break;
				p=p->Next;
			}
		}
	}
}


void emPanel::SetFocusable(bool focusable)
{
	if (Parent && ((bool)Focusable)!=focusable) {
		Focusable=focusable;
		if (Focusable) {
			if (InVisitedPath && !InActivePath) {
				View.VisitImmobile(View.ActivePanel,View.VisitAdherent);
			}
		}
		else {
			if (Active) {
				View.VisitImmobile(Parent,false);
			}
		}
	}
}


emPanel * emPanel::GetFocusableParent()
{
	emPanel * p;

	p=this;
	do {
		p=p->Parent;
	} while (p && !p->Focusable);
	return p;
}


emPanel * emPanel::GetFocusableFirstChild()
{
	emPanel * p;

	p=FirstChild;
	if (!p) return NULL;
	for (;;) {
		if (p->Focusable) return p;
		if (p->FirstChild) p=p->FirstChild;
		else {
			while (!p->Next) {
				p=p->Parent;
				if (p==this) return NULL;
			}
			p=p->Next;
		}
	}
}


emPanel * emPanel::GetFocusableLastChild()
{
	emPanel * p;

	p=LastChild;
	if (!p) return NULL;
	for (;;) {
		if (p->Focusable) return p;
		if (p->LastChild) p=p->LastChild;
		else {
			while (!p->Prev) {
				p=p->Parent;
				if (p==this) return NULL;
			}
			p=p->Prev;
		}
	}
}


emPanel * emPanel::GetFocusablePrev()
{
	emPanel * p;

	for (p=this;;) {
		while (!p->Prev) {
			p=p->Parent;
			if (!p || p->Focusable) return NULL;
		}
		p=p->Prev;
		for (;;) {
			if (p->Focusable) return p;
			if (!p->LastChild) break;
			p=p->LastChild;
		}
	}
}


emPanel * emPanel::GetFocusableNext()
{
	emPanel * p;

	for (p=this;;) {
		while (!p->Next) {
			p=p->Parent;
			if (!p || p->Focusable) return NULL;
		}
		p=p->Next;
		for (;;) {
			if (p->Focusable) return p;
			if (!p->FirstChild) break;
			p=p->FirstChild;
		}
	}
}


void emPanel::Activate()
{
	View.VisitLazy(this,true);
}


void emPanel::ActivateLater()
{
	View.SetActivationCandidate(this);
}


void emPanel::Focus()
{
	View.Focus();
	View.VisitLazy(this,true);
}


double emPanel::GetUpdatePriority() const
{
	double x1,y1,x2,y2,vx,vy,vw,vh,k,pri;

	if (Viewed) {
		vx=View.GetCurrentX();
		vw=View.GetCurrentWidth();
		vy=View.GetCurrentY();
		vh=View.GetCurrentHeight();
		x1=(ClipX1-vx)/vw-0.5;
		x2=(ClipX2-vx)/vw-0.5;
		y1=(ClipY1-vy)/vh-0.5;
		y2=(ClipY2-vy)/vh-0.5;
		if (x1<x2 && y1<y2) {
			k=0.5;
			pri=
				(((x1*x1*x1-x2*x2*x2)+(x2-x1)*(k+0.25))/k) *
				(((y1*y1*y1-y2*y2*y2)+(y2-y1)*(k+0.25))/k)
			;
			// The above formula results in the range of 0.0 to 1.0.
			pri*=0.49;
			if (IsViewFocused()) pri+=0.5;
			return pri;
		}
		else {
			return 0.0;
		}
	}
	else if (InViewedPath) {
		if (IsViewFocused()) return 1.0;
		else return 0.5;
	}
	else {
		return 0.0;
	}
}


emUInt64 emPanel::GetMemoryLimit() const
{

#define MEMORY_LIMIT_VARIANT 3 //???

#if MEMORY_LIMIT_VARIANT==1

	// This is the stupid one. It produces too much flicker by
	// loading/unloading of panels.

	double maxPerView;

	if (!InViewedPath) return 0;
	maxPerView=View.CoreConfig->MaxMegabytesPerView*1000000.0;
	if (!Viewed || View.SeekPosPanel==this) return (emUInt64)maxPerView;
	return (emUInt64)(
		(ClipX2-ClipX1)*(ClipY2-ClipY1)*maxPerView/
		(View.GetCurrentWidth()*View.GetCurrentHeight())
	);

#elif MEMORY_LIMIT_VARIANT==2

	// This trivial one did it for years. It assumes that memory intensive
	// panels are making up no more than about 36% of the areas (like with
	// emDirEntryPanel), and that the view size is about 1280*1024 pixels.

	double maxPerView,maxPerPanel,maxPerPixel,m;

	if (!InViewedPath) return 0;
	maxPerView=View.CoreConfig->MaxMegabytesPerView*1000000.0;
	maxPerPanel=maxPerView/600.0*75.0;
	maxPerPixel=maxPerView/600000000.0*500.0;
	if (!Viewed || View.SeekPosPanel==this) return (emUInt64)maxPerPanel;
	m=ViewedWidth*ViewedHeight*maxPerPixel;
	if (m>maxPerPanel) m=maxPerPanel;
	return (emUInt64)m;

#elif MEMORY_LIMIT_VARIANT==3

	// This algorithm solves a good balance between reducing flicker and not
	// wasting too much memory for invisible things. But it assumes that
	// memory intensive panels are never overlapping each other. And it does
	// not take advantage of unused space between memory intensive panels.
	// And the limit is still for the sub-tree, not for the node itself.

	double maxPerViewByUser,maxPerView,maxPerPanel;
	double viewExtension,viewExtensionValence;
	double vx,vy,vw,vh,evx1,evy1,evx2,evy2,ecx1,ecy1,ecx2,ecy2,fe,fn,f;

	if (!InViewedPath) return 0;
	maxPerViewByUser=View.CoreConfig->MaxMegabytesPerView*1000000.0;
	maxPerView =maxPerViewByUser*2.0;
	maxPerPanel=maxPerViewByUser*0.33;
		// Explanation of the above two lines: When there are many small
		// panels, it is unlikely that every panel would be at the
		// limit, and the average memory usage is assumed to be small.
		// But with just one big panel, the limit can be easily reached.
		// On the other hand, it's the case of many panels for which a
		// user normally wants to configure a higher limit.
	if (!Viewed || View.SeekPosPanel==this) return (emUInt64)maxPerPanel;
	viewExtension=0.5;
	viewExtensionValence=0.5;
	vx=View.GetCurrentX();
	vy=View.GetCurrentY();
	vw=View.GetCurrentWidth();
	vh=View.GetCurrentHeight();
	evx1=vx-vw*(viewExtension*0.5);
	evy1=vy-vh*(viewExtension*0.5);
	evx2=evx1+vw*(1.0+viewExtension);
	evy2=evy1+vh*(1.0+viewExtension);
	ecx1=ViewedX;
	ecy1=ViewedY;
	ecx2=ecx1+ViewedWidth;
	ecy2=ecy1+ViewedHeight;
	if (ecx1<evx1) ecx1=evx1;
	if (ecy1<evy1) ecy1=evy1;
	if (ecx2>evx2) ecx2=evx2;
	if (ecy2>evy2) ecy2=evy2;
	fe=(ecx2-ecx1)*(ecy2-ecy1)/((evx2-evx1)*(evy2-evy1));
	fn=(ClipX2-ClipX1)*(ClipY2-ClipY1)/(vw*vh);
	f=fe*viewExtensionValence+fn*(1.0-viewExtensionValence);
	f*=maxPerView;
	if (f>maxPerPanel) f=maxPerPanel;
	if (f<0.0) f=0.0;
	return (emUInt64)f;

#elif MEMORY_LIMIT_VARIANT==4

	// This experimental algorithm distributes the memory limit of a parent
	// to its children depending on their visible areas and sizes. It solves
	// the problem of overlapped panels and it takes advantage of unused
	// space between panels. The result is quite nice, but if the value for
	// condensationLimit is not some less than the actual condensation of
	// memory intensive panels, we get the flicker problem again. Also,
	// before really using this algorithm, it should get a good optimization
	// by some caching or an update mechanism, because in its current form
	// it is very expensive (see the loop and the recursive call). And in
	// addition, the sending of NF_MEMORY_LIMIT_CHANGED would have to be
	// reviewed and adapted.

	double maxPerView,viewExtension,viewExtensionValence,maxPerPanelFactor;
	double condensationLimit,ftotal,fself;
	double vx,vy,vw,vh,evx1,evy1,evx2,evy2,ecx1,ecy1,ecx2,ecy2,fe,fn,f;
	const emPanel * p;

	if (!InViewedPath) return 0;
	maxPerView=View.CoreConfig->MaxMegabytesPerView*1000000.0;
	if (!Viewed || !Parent || View.SeekPosPanel==this) return (emUInt64)maxPerView;
	maxPerPanelFactor=0.75;
	viewExtension=0.5;
	viewExtensionValence=0.5;
	condensationLimit=2.0; // 2.0 is good for emDirEntryPanel
	ftotal=0.0;
	fself=0.0;
	for (p=Parent->FirstChild; p; p=p->Next) {
		if (!p->Viewed) continue;
		vx=View.GetCurrentX();
		vy=View.GetCurrentY();
		vw=View.GetCurrentWidth();
		vh=View.GetCurrentHeight();
		evx1=vx-vw*(viewExtension*0.5);
		evy1=vy-vh*(viewExtension*0.5);
		evx2=evx1+vw*(1.0+viewExtension);
		evy2=evy1+vh*(1.0+viewExtension);
		ecx1=p->ViewedX;
		ecy1=p->ViewedY;
		ecx2=ecx1+p->ViewedWidth;
		ecy2=ecy1+p->ViewedHeight;
		if (ecx1<evx1) ecx1=evx1;
		if (ecy1<evy1) ecy1=evy1;
		if (ecx2>evx2) ecx2=evx2;
		if (ecy2>evy2) ecy2=evy2;
		fe=(ecx2-ecx1)*(ecy2-ecy1)/((evx2-evx1)*(evy2-evy1));
		fn=(p->ClipX2-p->ClipX1)*(p->ClipY2-p->ClipY1)/(vw*vh);
		f=fe*viewExtensionValence+fn*(1.0-viewExtensionValence);
		if (f>maxPerPanelFactor) f=maxPerPanelFactor;
		if (f<0.0) f=0.0;
		ftotal+=f;
		if (p==this) fself=f;
	}
	f=Parent->GetMemoryLimit()*fself/ftotal/maxPerView;
	if (f>fself*condensationLimit) f=fself*condensationLimit;
	if (f>maxPerPanelFactor) f=maxPerPanelFactor;
	if (f<0.0) f=0.0;
	return (emUInt64)(maxPerView*f);

#endif
}


double emPanel::GetTouchEventPriority(double touchX, double touchY)
{
	return Focusable ? 1.0 : 0.0;
}


bool emPanel::Cycle()
{
	return false;
}


void emPanel::Notice(NoticeFlags flags)
{
}


void emPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	if (Focusable && (event.IsMouseEvent() || event.IsTouchEvent())) {
		Focus();
		event.Eat();
	}
	else if (Active && event.IsKeyboardEvent()) {
		switch (event.GetKey()) {
		case EM_KEY_TAB:
			if (state.IsNoMod()) {
				View.VisitNext();
				event.Eat();
			}
			else if (state.IsShiftMod()) {
				View.VisitPrev();
				event.Eat();
			}
			break;
		case EM_KEY_CURSOR_LEFT:
			if (state.IsNoMod()) {
				View.VisitLeft();
				event.Eat();
			}
			break;
		case EM_KEY_CURSOR_RIGHT:
			if (state.IsNoMod()) {
				View.VisitRight();
				event.Eat();
			}
			break;
		case EM_KEY_CURSOR_UP:
			if (state.IsNoMod()) {
				View.VisitUp();
				event.Eat();
			}
			break;
		case EM_KEY_CURSOR_DOWN:
			if (state.IsNoMod()) {
				View.VisitDown();
				event.Eat();
			}
			break;
		case EM_KEY_PAGE_UP:
			if (state.IsNoMod()) {
				View.VisitIn();
				event.Eat();
			}
			break;
		case EM_KEY_PAGE_DOWN:
			if (state.IsNoMod()) {
				View.VisitOut();
				event.Eat();
			}
			break;
		case EM_KEY_HOME:
			if (state.IsNoMod()) {
				View.VisitFirst();
				event.Eat();
			}
			else if (state.IsAltMod()) {
				View.VisitFullsized(this,View.IsVisitAdherent());
				event.Eat();
			}
			else if (state.IsShiftAltMod()) {
				View.VisitFullsized(this,View.IsVisitAdherent(),true);
				event.Eat();
			}
			break;
		case EM_KEY_END:
			if (state.IsNoMod()) {
				View.VisitLast();
				event.Eat();
			}
			break;
		default:
			break;
		}
	}
}


emCursor emPanel::GetCursor()
{
	if (Parent) return Parent->GetCursor();
	else return emCursor::NORMAL;
}


bool emPanel::IsOpaque()
{
	return false;
}


void emPanel::Paint(const emPainter & painter, emColor canvasColor)
{
}


void emPanel::EnableAutoExpansion()
{
	if (!AEEnabled) {
		AEEnabled=1;
		AEDecisionInvalid=1;
		if (!NoticeNode.Next) View.AddToNoticeList(&NoticeNode);
	}
}


void emPanel::AutoExpand()
{
}


void emPanel::AutoShrink()
{
	emPanel * p, * t;

	for (p=LastChild; p;) {
		t=p;
		p=p->Prev;
		if (t->CreatedByAE) delete t;
	}
}


void emPanel::LayoutChildren()
{
}


emPanel * emPanel::CreateControlPanel(ParentArg parent, const emString & name)
{
	if (Parent) return Parent->CreateControlPanel(parent,name);
	else return NULL;
}


const char * emPanel::GetSoughtName() const
{
	if (View.SeekPosPanel==this) return View.SeekPosChildName;
	else return NULL;
}


bool emPanel::IsHopeForSeeking()
{
	return false;
}


void emPanel::InvalidateTitle()
{
	if (InActivePath) {
		View.TitleInvalid=true;
		View.UpdateEngine->WakeUp();
	}
}


void emPanel::InvalidateCursor()
{
	if (InViewedPath) {
		View.CursorInvalid=true;
		View.UpdateEngine->WakeUp();
	}
}


void emPanel::InvalidatePainting()
{
	if (Viewed) {
		if (!View.SVPChoiceByOpacityInvalid) {
			View.SVPChoiceByOpacityInvalid=true;
			View.UpdateEngine->WakeUp();
		}
		View.InvalidatePainting(ClipX1,ClipY1,ClipX2-ClipX1,ClipY2-ClipY1);
	}
}


void emPanel::InvalidatePainting(double x, double y, double w, double h)
{
	if (Viewed) {
		if (!View.SVPChoiceByOpacityInvalid) {
			View.SVPChoiceByOpacityInvalid=true;
			View.UpdateEngine->WakeUp();
		}
		x=x*ViewedWidth+ViewedX;
		w=w*ViewedWidth;
		y=y*(ViewedWidth/View.CurrentPixelTallness)+ViewedY;
		h=h*(ViewedWidth/View.CurrentPixelTallness);
		if (x<ClipX1) { w+=x-ClipX1; x=ClipX1; }
		if (y<ClipY1) { h+=y-ClipY1; y=ClipY1; }
		if (w>ClipX2-x) w=ClipX2-x;
		if (h>ClipY2-y) h=ClipY2-y;
		View.InvalidatePainting(x,y,w,h);
	}
}


void emPanel::InvalidateAutoExpansion()
{
	if (AEExpanded) {
		AEExpanded=0;
		AEDecisionInvalid=1;
		if (!NoticeNode.Next) View.AddToNoticeList(&NoticeNode);
		AutoShrink();
	}
}


void emPanel::InvalidateControlPanel()
{
	if (InActivePath) Signal(View.ControlPanelSignal);
}


void emPanel::HandleNotice()
{
	NoticeFlags flags;

	flags=PendingNoticeFlags;
	if (flags) {
		if (AEEnabled) {
			if (flags&(NF_SOUGHT_NAME_CHANGED|NF_VIEWING_CHANGED)) {
				AEDecisionInvalid=1;
			}
			if (AEDecisionInvalid) {
				if (!NoticeNode.Next) View.AddToNoticeList(&NoticeNode);
			}
		}
		if (flags&(NF_LAYOUT_CHANGED|NF_CHILD_LIST_CHANGED)) {
			if (FirstChild) ChildrenLayoutInvalid=1;
		}
		if (ChildrenLayoutInvalid) {
			if (!NoticeNode.Next) View.AddToNoticeList(&NoticeNode);
		}
		PendingNoticeFlags=0;
		Notice(flags);
		return; // Because Notice() is allowed to do a "delete this".
	}

	if (AEDecisionInvalid) {
		AEDecisionInvalid=0;
		if (AEEnabled) {
			if (
				GetSoughtName() ||
				GetViewCondition((ViewConditionType)AEThresholdType)>=AEThresholdValue
			) {
				if (!AEExpanded) {
					AEExpanded=1;
					AECalling=1;
					AutoExpand();
					AECalling=0;
					if (PendingNoticeFlags) return;
				}
			}
			else {
				if (AEExpanded) {
					AEExpanded=0;
					AutoShrink();
					if (PendingNoticeFlags) return;
				}
			}
		}
	}

	if (ChildrenLayoutInvalid) {
		if (FirstChild) LayoutChildren();
		ChildrenLayoutInvalid=0;
	}
}


void emPanel::UpdateChildrenViewing()
{
	emPanel * p;
	double x1,y1,x2,y2;

	if (!Viewed) {
		if (InViewedPath) {
			emFatalError("Illegal use of emPanel::UpdateChildrenViewing.");
		}
		for (p=FirstChild; p; p=p->Next) {
			if (p->InViewedPath) {
				p->Viewed=0;
				p->InViewedPath=0;
				p->AddPendingNotice(
					NF_VIEWING_CHANGED |
					NF_UPDATE_PRIORITY_CHANGED |
					NF_MEMORY_LIMIT_CHANGED
				);
				if (p->FirstChild) p->UpdateChildrenViewing();
			}
		}
	}
	else {
		for (p=FirstChild; p; p=p->Next) {
			x1=ViewedX+p->LayoutX*ViewedWidth;
			x2=p->LayoutWidth*ViewedWidth;
			y1=ViewedY+p->LayoutY*(ViewedWidth/View.CurrentPixelTallness);
			y2=p->LayoutHeight*(ViewedWidth/View.CurrentPixelTallness);
			p->ViewedX=x1;
			p->ViewedY=y1;
			p->ViewedWidth=x2;
			p->ViewedHeight=y2;
			x2+=x1;
			y2+=y1;
			if (x1<ClipX1) x1=ClipX1;
			if (x2>ClipX2) x2=ClipX2;
			if (y1<ClipY1) y1=ClipY1;
			if (y2>ClipY2) y2=ClipY2;
			p->ClipX1=x1;
			p->ClipX2=x2;
			p->ClipY1=y1;
			p->ClipY2=y2;
			if (x1<x2 && y1<y2) {
				p->InViewedPath=1;
				p->Viewed=1;
				p->AddPendingNotice(
					NF_VIEWING_CHANGED |
					NF_UPDATE_PRIORITY_CHANGED |
					NF_MEMORY_LIMIT_CHANGED
				);
				if (p->FirstChild) p->UpdateChildrenViewing();
			}
			else if (p->InViewedPath) {
				p->InViewedPath=0;
				p->Viewed=0;
				p->AddPendingNotice(
					NF_VIEWING_CHANGED |
					NF_UPDATE_PRIORITY_CHANGED |
					NF_MEMORY_LIMIT_CHANGED
				);
				if (p->FirstChild) p->UpdateChildrenViewing();
			}
		}
	}
}


void emPanel::AvlInsertChild(emPanel * child)
{
	EM_AVL_INSERT_VARS(emPanel)
	int d;

	EM_AVL_INSERT_BEGIN_SEARCH(emPanel,AvlNode,AvlTree)
		d=strcmp(child->Name.Get(),element->Name.Get());
		if (d<0) EM_AVL_INSERT_GO_LEFT
		else if (d>0) EM_AVL_INSERT_GO_RIGHT
		else {
			emFatalError(
				"emPanel: Panel name \"%s\" not unique within \"%s\".",
				child->Name.Get(),
				GetIdentity().Get()
			);
		}
	EM_AVL_INSERT_END_SEARCH
		element=child;
	EM_AVL_INSERT_NOW(AvlNode)
}


void emPanel::AvlRemoveChild(emPanel * child)
{
	EM_AVL_REMOVE_VARS(emPanel)
	int d;

	EM_AVL_REMOVE_BEGIN(emPanel,AvlNode,AvlTree)
		d=strcmp(child->Name.Get(),element->Name.Get());
		if (d<0) EM_AVL_REMOVE_GO_LEFT
		else if (d>0) EM_AVL_REMOVE_GO_RIGHT
		else EM_AVL_REMOVE_NOW
	EM_AVL_REMOVE_END
}
