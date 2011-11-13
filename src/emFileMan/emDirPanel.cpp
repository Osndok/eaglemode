//------------------------------------------------------------------------------
// emDirPanel.cpp
//
// Copyright (C) 2004-2008,2010 Oliver Hamann.
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

#include <ctype.h>
#include <emFileMan/emDirPanel.h>
#include <emFileMan/emDirEntryPanel.h>
#include <emFileMan/emFileManControlPanel.h>


emDirPanel::emDirPanel(
	ParentArg parent, const emString & name, const emString & path
)
	: emFilePanel(parent, name),
	Path(path)
{
	FileMan=emFileManModel::Acquire(GetRootContext());
	Config=emFileManViewConfig::Acquire(GetView());
	ContentComplete=false;
	KeyWalkState=NULL;
	AddWakeUpSignal(GetVirFileStateSignal());
	AddWakeUpSignal(Config->GetChangeSignal());
}


emDirPanel::~emDirPanel()
{
	ClearKeyWalkState();
}


void emDirPanel::SelectAll()
{
	emPanel * c;
	emDirEntryPanel * dep;

	FileMan->ClearSourceSelection();
	FileMan->SwapSelection();
	for (c=GetFirstChild(); c; c=c->GetNext()) {
		if ((dep=dynamic_cast<emDirEntryPanel*>(c))!=NULL) {
			FileMan->DeselectAsSource(dep->GetDirEntry().GetPath());
			FileMan->SelectAsTarget(dep->GetDirEntry().GetPath());
		}
	}
}


bool emDirPanel::Cycle()
{
	bool busy;

	busy=emFilePanel::Cycle();
	if (
		IsSignaled(GetVirFileStateSignal()) ||
		IsSignaled(Config->GetChangeSignal())
	) {
		InvalidatePainting();
		UpdateChildren();
		if (IsSignaled(Config->GetChangeSignal())) {
			InvalidateChildrenLayout();
		}
	}
	if (KeyWalkState && IsSignaled(KeyWalkState->Timer.GetSignal())) {
		ClearKeyWalkState();
	}
	return busy;
}


void emDirPanel::Notice(NoticeFlags flags)
{
	if ((flags&(NF_VIEWING_CHANGED|NF_SOUGHT_NAME_CHANGED))!=0) {
		if (IsViewed() || GetSoughtName()) {
			if (!GetFileModel()) {
				SetFileModel(emDirModel::Acquire(GetRootContext(),Path));
			}
		}
		else {
			if (GetFileModel()) {
				SetFileModel(NULL);
			}
		}
	}

	if ((flags&NF_FOCUS_CHANGED)!=0) {
		if (KeyWalkState && !IsInFocusedPath()) {
			ClearKeyWalkState();
		}
	}

	emFilePanel::Notice(flags);
}


void emDirPanel::Input(
	emInputEvent & event, const emInputState & state, double mx, double my
)
{
	emScreen * screen;

	emFilePanel::Input(event,state,mx,my);

	switch (event.GetKey()) {
	case EM_KEY_A:
		if (state.IsAltMod()) {
			if (IsContentComplete()) {
				SelectAll();
			}
			else {
				screen=GetScreen();
				if (screen) screen->Beep();
			}
			event.Eat();
		}
		break;
	default:
		break;
	}

	if (event.IsKeyboardEvent()) FileMan->HotkeyInput(GetView(),event,state);

	KeyWalk(event,state);
}


bool emDirPanel::IsOpaque()
{
	if (GetVirFileState()!=VFS_LOADED) return emFilePanel::IsOpaque();
	else return Config->GetTheme().DirContentColor.Get().IsOpaque();
}


void emDirPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	if (GetVirFileState()!=VFS_LOADED) emFilePanel::Paint(painter,canvasColor);
	else painter.Clear(Config->GetTheme().DirContentColor);
}


void emDirPanel::LayoutChildren()
{
	int cnt,col,cols,row,rows;
	double h,t,cw,ch;
	emPanel * p;

	if (IsContentComplete()) {
		for (cnt=0, p=GetFirstChild(); p; cnt++, p=p->GetNext());
		if (cnt) {
			h=GetHeight();
			t=Config->GetTheme().Height;
			for (rows=1; ;rows++) {
				cols=(int)(rows*t/(h*(1.0-0.05/rows)));
				if (cols<=0) cols=1;
				if (rows*cols>=cnt) break;
			}
			cols=(cnt+rows-1)/rows;
			ch=h/rows;
			cw=1.0/cols;
			if (ch>cw*t) ch=cw*t; else cw=ch/t;
			for (col=0, row=0, p=GetFirstChild(); p; p=p->GetNext()) {
				p->Layout(cw*col,ch*row,cw,ch,Config->GetTheme().DirContentColor);
				row++;
				if (row>=rows) { col++; row=0; }
			}
		}
	}
	else {
		for (p=GetFirstChild(); p; p=p->GetNext()) {
			p->Layout(
				p->GetLayoutX(),
				p->GetLayoutY(),
				p->GetLayoutWidth(),
				p->GetLayoutWidth()*Config->GetTheme().Height,
				Config->GetTheme().DirContentColor
			);
		}
	}
}


emPanel * emDirPanel::CreateControlPanel(
	ParentArg parent, const emString & name
)
{
	if (IsActive()) {
		return new emFileManControlPanel(parent,name,GetView());
	}
	else {
		return NULL;
	}
}


void emDirPanel::SetFileModel(emFileModel * fileModel, bool updateFileModel)
{
	emPanel * p;
	emFilePanel * fp;

	if (updateFileModel) {
		// This is a workaround for a problem:
		// emDirModel::IsOutOfDate always returns true, because it
		// cannot check modification times like with files (at time of
		// writing this comment). Therefore, emDirModel always really
		// reloads on an update. On the other hand, when creating a file
		// panel (or a dir panel), we always want to update the model.
		// Now to the problem: If there are two emDirPanels on a path of
		// panels which share the same model (could happen through an
		// emFileLink), we get infinite reloadings: Child panel is
		// created => model reloading => parent panel destructs child
		// ... model finishes with loading => parent panel constructs
		// child panel again => model reloading => ...
		// The workaround is not to update the model if there is an
		// ancestor panel sharing the same model.
		for (p=GetParent(); p; p=p->GetParent()) {
			fp=dynamic_cast<emFilePanel*>(p);
			if (fp && fp->GetFileModel()==fileModel) {
				updateFileModel=false;
				break;
			}
		}
	}

	emFilePanel::SetFileModel(fileModel,updateFileModel);
}


void emDirPanel::UpdateChildren()
{
	char * foundMap;
	const emDirEntry * de;
	const emDirModel * dm;
	emPanel * p, * np;
	int i, count;

	if (GetVirFileState()==VFS_LOADED) {
		dm=(const emDirModel*)GetFileModel();
		count=dm->GetEntryCount();
		foundMap=new char[count];
		memset(foundMap,0,count);
		for (p=GetFirstChild(); p; ) {
			np=p->GetNext();
			de=&((emDirEntryPanel*)p)->GetDirEntry();
			if (de->IsHidden() && !Config->GetShowHiddenFiles()) {
				delete p;
			}
			else {
				i=dm->GetEntryIndex(de->GetName());
				if (i>=0) {
					foundMap[i]=1;
					((emDirEntryPanel*)p)->UpdateDirEntry(dm->GetEntry(i));
				}
				else delete p;
			}
			p=np;
		}
		for (i=0; i<count; i++) {
			if (!foundMap[i]) {
				de=&dm->GetEntry(i);
				if (!de->IsHidden() || Config->GetShowHiddenFiles()) {
					new emDirEntryPanel(*this,de->GetName(),*de);
				}
			}
		}
		delete [] foundMap;
		SortChildren();
		ContentComplete=true;
		InvalidateChildrenLayout();
	}
	else {
		for (p=GetFirstChild(); p; ) {
			np=p->GetNext();
			if (!p->IsInVisitedPath()) delete p;
			p=np;
		}
		ContentComplete=false;
	}
}


void emDirPanel::SortChildren()
{
	emPanel::SortChildren(&CompareChildren,(void*)this);
}


int emDirPanel::CompareChildren(emPanel * c1, emPanel * c2, void * context)
{
	return ((emDirPanel*)context)->Config->CompareDirEntries(
		((emDirEntryPanel*)c1)->GetDirEntry(),
		((emDirEntryPanel*)c2)->GetDirEntry()
	);
}


void emDirPanel::ClearKeyWalkState()
{
	if (KeyWalkState) {
		delete KeyWalkState;
		KeyWalkState=NULL;
	}
}


void emDirPanel::KeyWalk(emInputEvent & event, const emInputState & state)
{
	const char * s1, * s2;
	emPanel * p;
	emScreen * screen;
	emDirEntryPanel * dep;
	emString str;
	int len, c1, c2, i;

	if (event.GetChars().IsEmpty()) return;
	if (state.GetCtrl() || state.GetAlt() || state.GetMeta()) return;
	if (event.GetChars()==" ") return;

	if (!IsContentComplete()) {
		screen=GetScreen();
		if (screen) screen->Beep();
		event.Eat();
		return;
	}

	if (KeyWalkState) str=KeyWalkState->String+event.GetChars();
	else str=event.GetChars();
	len=str.GetLen();

	if (str[0]=='*') {
		// ??? undocumented feature: e.g. type "*bar" to find "FooBar".
		for (p=GetFirstChild(); p; p=p->GetNext()) {
			dep=dynamic_cast<emDirEntryPanel*>(p);
			if (!dep) continue;
			s1=str.Get()+1;
			s2=dep->GetDirEntry().GetName();
			for (i=0;;) {
				c1=s1[i];
				c2=s2[i];
				if (!c1 || !c2) break;
				if (tolower(c1)==tolower(c2)) i++;
				else { i=0; s2++; }
			}
			if (!c1) break;
		}
	}
	else {
		for (p=GetFirstChild(); p; p=p->GetNext()) {
			dep=dynamic_cast<emDirEntryPanel*>(p);
			if (!dep) continue;
			if (strncasecmp(str,dep->GetDirEntry().GetName(),len)==0) break;
		}
		if (!p) {
			for (p=GetFirstChild(); p; p=p->GetNext()) {
				dep=dynamic_cast<emDirEntryPanel*>(p);
				if (!dep) continue;
				s1=str;
				s2=dep->GetDirEntry().GetName();
				for (;;) {
					c1=tolower(*s1++);
					if (!c1) break;
					do {
						c2=tolower(*s2++);
					} while (c2 && c2!=c1 && (c2==' ' || c2=='-' || c2=='_'));
					if (c2!=c1) break;
				}
				if (!c1) break;
			}
		}
	}

	if (p) {
		GetView().Visit(p,true);
		if (!KeyWalkState) {
			KeyWalkState=new KeyWalkStateType(GetScheduler());
			AddWakeUpSignal(KeyWalkState->Timer.GetSignal());
		}
		else {
			KeyWalkState->Timer.Stop(true);
		}
		KeyWalkState->Timer.Start(1000);
		KeyWalkState->String=str;
	}
	else {
		ClearKeyWalkState();
		screen=GetScreen();
		if (screen) screen->Beep();
	}

	event.Eat();
}
