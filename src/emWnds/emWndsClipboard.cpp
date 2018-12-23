//------------------------------------------------------------------------------
// emWndsClipboard.cpp
//
// Copyright (C) 2006-2008,2014,2018 Oliver Hamann.
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
	LPWSTR mem;
	int len;

	if (selection) {
		SelectionText=str;
		CurrentSelectionId++;
		return CurrentSelectionId;
	}

	if (!OpenClipboard(NULL)) {
		return 0;
	}

	EmptyClipboard();

	if (!str.IsEmpty()) {
		len=MultiByteToWideChar(CP_ACP,0,str.Get(),str.GetLen(),NULL,0);
		if (len>0) {
			hglobal=GlobalAlloc(GMEM_MOVEABLE,(len+1)*sizeof(wchar_t));
			if (hglobal) {
				mem=(LPWSTR)GlobalLock(hglobal);
				if (mem) {
					memset(mem,0,(len+1)*sizeof(wchar_t));
					MultiByteToWideChar(CP_ACP,0,str.Get(),str.GetLen(),mem,len);
					GlobalUnlock(hglobal);
					if (SetClipboardData(CF_UNICODETEXT,hglobal)) hglobal=NULL;
				}
				if (hglobal) {
					GlobalFree(hglobal);
				}
			}
		}
	}

	CloseClipboard();
	return 0;
}


void emWndsClipboard::Clear(bool selection, emInt64 selectionId)
{
	if (selection) {
		if (CurrentSelectionId==selectionId) {
			SelectionText.Clear();
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
	LPWSTR mem;
	int lenW,lenC;
	char * buf;
	emString str;

	if (selection) {
		return SelectionText;
	}

	str="";
	if (!OpenClipboard(NULL)) {
		return str;
	}

	hglobal=GetClipboardData(CF_UNICODETEXT);
	if (hglobal) {
		mem=(LPWSTR)GlobalLock(hglobal);
		if (mem) {
			lenW=(int)GlobalSize(hglobal)/2;
			if (lenW>0) {
				lenC=WideCharToMultiByte(CP_ACP,0,mem,lenW,NULL,0,NULL,NULL);
				if (lenC>0) {
					buf=(char*)malloc(lenC+1);
					memset(buf,0,lenC+1);
					WideCharToMultiByte(CP_ACP,0,mem,lenW,buf,lenC,NULL,NULL);
					str=buf;
					free(buf);
				}
			}
			GlobalUnlock(hglobal);
		}
	}

	CloseClipboard();
	return str;
}


emWndsClipboard::emWndsClipboard(emContext & context, const emString & name)
	: emClipboard(context,name)
{
	CurrentSelectionId=1;
}


emWndsClipboard::~emWndsClipboard()
{
}
