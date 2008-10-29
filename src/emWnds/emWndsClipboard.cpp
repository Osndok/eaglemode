//------------------------------------------------------------------------------
// emWndsClipboard.cpp
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#include <emWnds/emWndsClipboard.h>
#include <windows.h>


void emWndsClipboard::Install(emContext & context)
{
	emWndsClipboard * m;
	emString name;

	m=(emWndsClipboard*)context.Lookup(typeid(emWndsClipboard),name);
	if (!m) {
		m=new emWndsClipboard(context,name);
		m->Register();
	}
	m->emClipboard::Install();
}


emInt64 emWndsClipboard::PutText(const emString & str, bool selection)
{
	HGLOBAL hglobal;
	LPVOID mem;

	if (selection) {
		SelectionText=str;
		CurrentSelectionId++;
		return CurrentSelectionId;
	}
	else {
		if (OpenClipboard(NULL)) {
			EmptyClipboard();
			if (!str.IsEmpty()) {
				hglobal=GlobalAlloc(GMEM_MOVEABLE,str.GetLen()+1);
				if (hglobal) {
					mem=GlobalLock(hglobal);
					if (mem) {
						memcpy(mem,str.Get(),str.GetLen()+1);
						GlobalUnlock(hglobal);
						if (SetClipboardData(CF_TEXT,hglobal)) hglobal=NULL;
					}
					if (hglobal) {
						GlobalFree(hglobal);
					}
				}
			}
			CloseClipboard();
		}
		return 0;
	}
}


void emWndsClipboard::Clear(bool selection, emInt64 selectionId)
{
	if (selection) {
		if (CurrentSelectionId==selectionId) {
			SelectionText.Empty();
			CurrentSelectionId++;
		}
	}
	else {
		if (OpenClipboard(NULL)) {
			EmptyClipboard();
			CloseClipboard();
		}
	}
}


emString emWndsClipboard::GetText(bool selection)
{
	HGLOBAL hglobal;
	LPVOID mem;
	emString str;

	if (selection) {
		return SelectionText;
	}
	else {
		str="";
		if (OpenClipboard(NULL)) {
			hglobal=GetClipboardData(CF_TEXT);
			if (hglobal) {
				mem=GlobalLock(hglobal);
				if (mem) {
					str=(const char*)mem;
					GlobalUnlock(hglobal);
				}
			}
			CloseClipboard();
		}
		return str;
	}
}


emWndsClipboard::emWndsClipboard(emContext & context, const emString & name)
	: emClipboard(context,name)
{
	CurrentSelectionId=1;
}


emWndsClipboard::~emWndsClipboard()
{
}
