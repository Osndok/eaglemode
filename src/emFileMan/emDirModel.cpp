//------------------------------------------------------------------------------
// emDirModel.cpp
//
// Copyright (C) 2005-2010 Oliver Hamann.
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
#include <emFileMan/emDirModel.h>


emRef<emDirModel> emDirModel::Acquire(
	emContext & context, const emString & name, bool common
)
{
	EM_IMPL_ACQUIRE(emDirModel,context,name,common)
}


int emDirModel::GetEntryIndex(const char * fileName) const
{
	int i1, i2, im, d;

	i1=0;
	i2=EntryCount;
	while (i1<i2) {
		im=(i1+i2)/2;
		d=strcmp(fileName,Entries[im].GetName().Get());
		if (d<0) i2=im;
		else if (d>0) i1=im+1;
		else return im;
	}
	return -1;
}


emDirModel::emDirModel(emContext & context, const emString & name)
	: emFileModel(context,name)
{
	DirHandle=NULL;
	CurrentBlock=NULL;
	CurrentBlockFill=0;
	Names=NULL;
	NameCount=0;
	EntryCount=0;
	Entries=NULL;
}


emDirModel::~emDirModel()
{
	emDirModel::QuitLoading();
	emDirModel::ResetData();
}


void emDirModel::ResetData()
{
	EntryCount=0;
	if (Entries) {
		delete [] Entries;
		Entries=NULL;
	}
}


void emDirModel::TryStartLoading() throw(emString)
{
#if defined(__APPLE__)
	char buf[PATH_MAX+1];
	if (
		realpath(GetFilePath().Get(),buf)!=NULL &&
		strcmp(buf,"/net")==0
	) throw emString(
		"Loading this directory would probably stall the process."
	);
#endif
	DirHandle=emTryOpenDir(GetFilePath());
}


bool emDirModel::TryContinueLoading() throw(emString)
{
	NamesBlock * block;
	NameNode * node;
	emString name;

	if (DirHandle) {
		name=emTryReadDir(DirHandle);
		if (name.IsEmpty()) {
			emCloseDir(DirHandle);
			DirHandle=NULL;
		}
		else {
			if (!CurrentBlock || CurrentBlockFill>=NamesBlock::MaxNames) {
				block=new NamesBlock;
				block->Prev=CurrentBlock;
				CurrentBlock=block;
				CurrentBlockFill=0;
			}
			node=&CurrentBlock->Names[CurrentBlockFill++];
			node->Name=name;
			node->Next=Names;
			Names=node;
			NameCount++;
		}
		return false;
	}
	else if (!Entries && NameCount>0) {
		emSortSingleLinkedList(
			(void**)(void*)&Names,offsetof(NameNode,Next),CompareName,NULL
		);
		// I saw double names in Cygwin proc fs, in Linux ISO fs and in
		// Windows NTFS. Unbelievable, isn't it? We have to remove those
		// double names here.
		for (node=Names; node->Next; ) {
			if (CompareName(node,node->Next,NULL)==0) {
				node->Next=node->Next->Next;
				NameCount--;
			}
			else node=node->Next;
		}
		Entries=new emDirEntry[NameCount];
		return false;
	}
	else if (EntryCount<NameCount) {
		Entries[EntryCount].Load(GetFilePath(),Names->Name);
		Names=Names->Next;
		EntryCount++;
		return false;
	}
	else {
		return true;
	}
}


void emDirModel::QuitLoading()
{
	NamesBlock * block;

	if (DirHandle) {
		emCloseDir(DirHandle);
		DirHandle=NULL;
	}
	while (CurrentBlock) {
		block=CurrentBlock;
		CurrentBlock=block->Prev;
		delete block;
	}
	CurrentBlockFill=0;
	Names=NULL;
	NameCount=0;
}


void emDirModel::TryStartSaving() throw(emString)
{
	throw emString("Saving an emDirModel is impossible.");
}


bool emDirModel::TryContinueSaving() throw(emString)
{
	return true;
}


void emDirModel::QuitSaving()
{
}


emUInt64 emDirModel::CalcMemoryNeed()
{
	return NameCount*8192; // more a CPU need here
}


double emDirModel::CalcFileProgress()
{
	if (DirHandle) {
		return 20.0*(1.0-10.0/(10+sqrt((double)NameCount)));
	}
	else {
		return NameCount>0 ? 20.0+80.0*EntryCount/NameCount : 100.0;
	}
}


bool emDirModel::IsOutOfDate()
{
	//??? Always true because the modification time may not get
	//??? updated after a change.
	return true;
}


int emDirModel::CompareName(void * node1, void * node2, void * context)
{
	return strcmp(
		((NameNode*)node1)->Name.Get(),
		((NameNode*)node2)->Name.Get()
	);
}
