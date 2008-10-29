//------------------------------------------------------------------------------
// emDirPanel.cpp
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
	FileManViewConfig=emFileManViewConfig::Acquire(GetView());
	ContentComplete=false;
	KeyWalkState=NULL;
	AddWakeUpSignal(GetVirFileStateSignal());
	AddWakeUpSignal(FileManViewConfig->GetChangeSignal());
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
		IsSignaled(FileManViewConfig->GetChangeSignal())
	) {
		UpdateChildren();
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
	emPanel * c;
	emScreen * screen;
	emDirEntryPanel * dep;
	emString str;
	int len;

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

	if (
		!event.GetChars().IsEmpty() &&
		!state.GetCtrl() &&
		!state.GetAlt() &&
		!state.GetMeta()
	) {
		if (IsContentComplete()) {
			if (KeyWalkState) str=KeyWalkState->String+event.GetChars();
			else str=event.GetChars();
			len=str.GetLen();
			for (c=GetFirstChild(); c; c=c->GetNext()) {
				if ((dep=dynamic_cast<emDirEntryPanel*>(c))!=NULL) {
					if (strncasecmp(str,dep->GetDirEntry().GetName(),len)==0) break;
				}
			}
			if (c!=NULL) {
				GetView().Visit(c,true);
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
		}
		else {
			screen=GetScreen();
			if (screen) screen->Beep();
		}
		event.Eat();
	}
}


bool emDirPanel::IsOpaque()
{
	if (GetVirFileState()!=VFS_LOADED) return emFilePanel::IsOpaque();
	return false;
}


void emDirPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	if (GetVirFileState()!=VFS_LOADED) emFilePanel::Paint(painter,canvasColor);
}


void emDirPanel::LayoutChildren()
{
	emPanel * p;
	int i,sz,cnt;
	double w,h;

	if (!IsContentComplete()) return;

	for (cnt=0, p=GetFirstChild(); p; cnt++, p=p->GetNext());
	if (cnt>0) {
		for (sz=1; sz*sz<cnt; sz++);
		w=GetWidth()/sz;
		h=GetHeight()/sz;
		for (i=0, p=GetFirstChild(); p; i++, p=p->GetNext()) {
			p->Layout(w*(i/sz),h*(i%sz),w,h,GetCanvasColor());
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
			if (de->IsHidden() && !FileManViewConfig->GetShowHiddenFiles()) {
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
				if (!de->IsHidden() || FileManViewConfig->GetShowHiddenFiles()) {
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
	return ((emDirPanel*)context)->FileManViewConfig->CompareDirEntries(
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
