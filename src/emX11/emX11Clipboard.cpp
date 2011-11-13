//------------------------------------------------------------------------------
// emX11Clipboard.cpp
//
// Copyright (C) 2005-2008,2010 Oliver Hamann.
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

#include <emX11/emX11Clipboard.h>
#include <X11/Xatom.h>


void emX11Clipboard::Install(emContext & context)
{
	emX11Clipboard * m;
	emString name;

	m=(emX11Clipboard*)context.Lookup(typeid(emX11Clipboard),name);
	if (!m) {
		m=new emX11Clipboard(context,name);
		m->Register();
	}
	m->emClipboard::Install();
}


emInt64 emX11Clipboard::PutText(const emString & str, bool selection)
{
	::Window owner;
	int idx;

	idx=selection?1:0;
	LocalText[idx]=str;
	LocalTimestamp[idx]=Screen->LastKnownTime;
	if (str.IsEmpty()) owner=None; else owner=Win;
	XMutex->Lock();
	XSetSelectionOwner(Disp,SelAtom[idx],owner,LocalTimestamp[idx]);
	XMutex->Unlock();
	if (selection) {
		LocalSelectionId++;
		return LocalSelectionId;
	}
	else {
		return 0;
	}
}


void emX11Clipboard::Clear(bool selection, emInt64 selectionId)
{
	int idx;

	idx=selection?1:0;
	if (selection) {
		if (LocalSelectionId==selectionId) {
			LocalText[idx].Empty();
			LocalSelectionId++;
			XMutex->Lock();
			if (XGetSelectionOwner(Disp,SelAtom[idx])==Win) {
				XSetSelectionOwner(Disp,SelAtom[idx],None,LocalTimestamp[idx]);
			}
			XMutex->Unlock();
			LocalTimestamp[idx]=Screen->LastKnownTime;
		}
	}
	else {
		LocalText[idx].Empty();
		LocalTimestamp[idx]=Screen->LastKnownTime;
		XMutex->Lock();
		XSetSelectionOwner(Disp,SelAtom[idx],None,LocalTimestamp[idx]);
		XMutex->Unlock();
	}
}


emString emX11Clipboard::GetText(bool selection)
{
	::Window w;
	emArray<emByte> array;
	int idx,format;
	unsigned long items,l;
	Atom target,type;
	const XSelectionEvent * se;
	emString str;

	idx=selection?1:0;
	XMutex->Lock();
	w=XGetSelectionOwner(Disp,SelAtom[idx]);
	XMutex->Unlock();
	if (w==Win) return LocalText[idx];
	if (w==None) return emString();

	XMutex->Lock();
	XConvertSelection(
		Disp,
		SelAtom[idx],
		MY_XA_TARGETS,
		SelAtom[idx],
		Win,
		Screen->LastKnownTime
	);
	XMutex->Unlock();
	se=WaitSelectionEvent(SelAtom[idx],MY_XA_TARGETS);
	if (se && se->property!=None) {
		array=GetLargeWindowProperty(
			Disp,
			Win,
			se->property,
			true,
			XA_ATOM,
			&type,
			&format,
			&items
		);
		if (type==XA_ATOM && format==32 && items>0) {
			target=None;
			for (l=0; l<items; l++) {
				if (((const Atom*)array.Get())[l]==MY_XA_UTF8_STRING) {
					target=MY_XA_UTF8_STRING;
				}
			}
			if (target==None) {
				for (l=0; l<items; l++) {
					if (((const Atom*)array.Get())[l]==XA_STRING) {
						target=XA_STRING;
					}
				}
			}
			if (target==None) return emString();
		}
		else {
			target=XA_STRING;
		}
	}
	else {
		target=XA_STRING;
	}

	XMutex->Lock();
	XConvertSelection(
		Disp,
		SelAtom[idx],
		target,
		SelAtom[idx],
		Win,
		Screen->LastKnownTime
	);
	XMutex->Unlock();
	se=WaitSelectionEvent(SelAtom[idx],target);
	if (!se || se->property==None) {
		return emString();
	}
	array=GetLargeWindowProperty(
		Disp,
		Win,
		se->property,
		true,
		AnyPropertyType,
		&type,
		&format,
		&items
	);
	if (format!=8) return emString();
	str=emString((const char*)array.Get(),array.GetCount());
	array.Empty();
	if (emIsUtf8System()) {
		if (target==XA_STRING) str=Latin1ToUtf8(str);
	}
	else {
		if (target==MY_XA_UTF8_STRING) str=Utf8ToLatin1(str);
	}
	return str;
}


emX11Clipboard::emX11Clipboard(emContext & context, const emString & name)
	: emClipboard(context,name)
{
	XSetWindowAttributes xswa;

	Screen=(emX11Screen*)context.Lookup(typeid(emX11Screen),"");
	if (!Screen) {
		emFatalError("emX11Clipboard: An emX11Screen is required in same context.");
	}
	XMutex=&Screen->XMutex;
	Disp=Screen->Disp;

	XMutex->Lock();
	MY_XA_TARGETS    =XInternAtom(Disp,"TARGETS"    ,False);
	MY_XA_TIMESTAMP  =XInternAtom(Disp,"TIMESTAMP"  ,False);
	MY_XA_UTF8_STRING=XInternAtom(Disp,"UTF8_STRING",False);
	MY_XA_CLIPBOARD  =XInternAtom(Disp,"CLIPBOARD"  ,False);
	XMutex->Unlock();

	SelAtom[0]=MY_XA_CLIPBOARD;
	SelAtom[1]=XA_PRIMARY;
	LocalTimestamp[0]=CurrentTime;
	LocalTimestamp[1]=CurrentTime;
	LocalSelectionId=1;
	memset(&LastSelectionEvent,0,sizeof(LastSelectionEvent));

	memset(&xswa,0,sizeof(xswa));
	xswa.override_redirect=True;

	XMutex->Lock();
	Win=XCreateWindow(
		Disp,Screen->RootWin,-100,-100,1,1,0,CopyFromParent,
		InputOnly,CopyFromParent,CWOverrideRedirect,&xswa
	);
	XStoreName(Disp,Win,"EM Clipboard");
	XMutex->Unlock();

	if (Screen->Clipboard) {
		emFatalError("Only one emX11Clipboard can be installed per context.");
	}
	Screen->Clipboard=this;
}


emX11Clipboard::~emX11Clipboard()
{
	Screen->Clipboard=NULL;
	XMutex->Lock();
	XDestroyWindow(Disp,Win);
	XMutex->Unlock();
}


XSelectionEvent * emX11Clipboard::WaitSelectionEvent(Atom selection, Atom target)
{
	XEvent e;
	Bool b;
	int t;

	memset(&LastSelectionEvent,0,sizeof(LastSelectionEvent));
	for (t=0;;) {
		XMutex->Lock();
		b=XCheckTypedWindowEvent(Disp,Win,SelectionNotify,&e);
		XMutex->Unlock();
		if (b) {
			HandleEvent(e);
			if (
				LastSelectionEvent.requestor==Win &&
				LastSelectionEvent.selection==selection &&
				LastSelectionEvent.target==target
			) return &LastSelectionEvent;
		}
		else {
			t++;
			if (t>50) return NULL;
			emSleepMS(40);
		}
	}
}


void emX11Clipboard::HandleEvent(XEvent & event)
{
	switch (event.type) {
	case SelectionClear:
		HandleSelectionClear(event.xselectionclear);
		break;
	case SelectionRequest:
		HandleSelectionRequest(event.xselectionrequest);
		break;
	case SelectionNotify:
		HandleSelectionNotify(event.xselection);
		break;
	}
}


void emX11Clipboard::HandleSelectionClear(XSelectionClearEvent & sce)
{
	if (sce.selection==SelAtom[0]) {
		if (sce.time>=LocalTimestamp[0]) {
			LocalText[0].Empty();
		}
	}
	else if (sce.selection==SelAtom[1]) {
		if (sce.time>=LocalTimestamp[1]) {
			LocalText[1].Empty();
			LocalSelectionId++;
		}
	}
}


void emX11Clipboard::HandleSelectionRequest(XSelectionRequestEvent & sre)
{
	XEvent e;
	Atom result;
	Atom atoms[16];
	int idx,i;
	emString str;

	if (sre.selection==SelAtom[0]) idx=0;
	else if (sre.selection==SelAtom[1]) idx=1;
	else idx=-1;

	if (idx<0 || LocalText[idx].IsEmpty()) {
		result=None;
	}
	else if (sre.target==MY_XA_TARGETS) {
		i=0;
		atoms[i++]=MY_XA_TARGETS;
		atoms[i++]=MY_XA_TIMESTAMP;
		atoms[i++]=MY_XA_UTF8_STRING;
		atoms[i++]=XA_STRING;
		XMutex->Lock();
		XChangeProperty(
			Disp,
			sre.requestor,
			sre.property,
			XA_ATOM,
			32,
			PropModeReplace,
			(unsigned char*)atoms,
			i
		);
		XMutex->Unlock();
		result=sre.property;
	}
	else if (sre.target==MY_XA_TIMESTAMP) {
		XMutex->Lock();
		XChangeProperty(
			Disp,
			sre.requestor,
			sre.property,
			MY_XA_TIMESTAMP,
			32,
			PropModeReplace,
			(unsigned char*)&LocalTimestamp[idx],
			1
		);
		XMutex->Unlock();
		result=sre.property;
	}
	else if (sre.target==MY_XA_UTF8_STRING) {
		str=LocalText[idx];
		if (!emIsUtf8System()) str=Latin1ToUtf8(str);
		XMutex->Lock();
		XChangeProperty(
			Disp,
			sre.requestor,
			sre.property,
			MY_XA_UTF8_STRING,
			8,
			PropModeReplace,
			(unsigned char*)str.Get(),
			str.GetLen()
		);
		XMutex->Unlock();
		result=sre.property;
	}
	else if (sre.target==XA_STRING) {
		str=LocalText[idx];
		if (emIsUtf8System()) str=Utf8ToLatin1(str);
		XMutex->Lock();
		XChangeProperty(
			Disp,
			sre.requestor,
			sre.property,
			XA_STRING,
			8,
			PropModeReplace,
			(unsigned char*)str.Get(),
			str.GetLen()
		);
		XMutex->Unlock();
		result=sre.property;
	}
	else {
		result=None;
	}

	memset(&e,0,sizeof(e));
	e.xselection.type     = SelectionNotify;
	e.xselection.display  = sre.display;
	e.xselection.requestor= sre.requestor;
	e.xselection.selection= sre.selection;
	e.xselection.target   = sre.target;
	e.xselection.property = result;
	e.xselection.time     = sre.time;
	XMutex->Lock();
	XSendEvent(Disp,sre.requestor,False,0,&e);
	XFlush(Disp);
	XMutex->Unlock();
}


void emX11Clipboard::HandleSelectionNotify(XSelectionEvent & se)
{
	LastSelectionEvent=se;
}


emArray<emByte> emX11Clipboard::GetLargeWindowProperty(
	Display * display,
	Window window,
	Atom property,
	bool deleteProperty,
	Atom type,
	Atom * actual_type_return,
	int * actual_format_return,
	unsigned long * nitems_return
)
{
	emArray<emByte> array;
	unsigned char * buf;
	Atom actualType;
	unsigned long items,offset,bytesAfter;
	int format,len, r;

	array.SetTuningLevel(4);
	*actual_type_return=AnyPropertyType;
	*actual_format_return=0;
	*nitems_return=0;
	offset=0;
	buf=NULL;
	do {
		XMutex->Lock();
		r=XGetWindowProperty(
			display,
			window,
			property,
			offset,
			4000,
			False,
			type,
			&actualType,
			&format,
			&items,
			&bytesAfter,
			&buf
		);
		XMutex->Unlock();
		if (r!= Success) goto L_Error;
		if (*actual_type_return==AnyPropertyType) *actual_type_return=actualType;
		else if (*actual_type_return!=actualType) goto L_Error;
		if (*actual_format_return==0) *actual_format_return=format;
		else if (*actual_format_return!=format) goto L_Error;
		*nitems_return+=items;
		offset+=items*format/32;
		if (format==32) len=((int)items)*sizeof(long);
		else len=((int)items)*format/8;
		array.Add(buf,len);
		XMutex->Lock();
		XFree(buf);
		XMutex->Unlock();
		buf=NULL;
	} while (bytesAfter && items>0);
	if (deleteProperty) {
		XMutex->Lock();
		XDeleteProperty(display,window,property);
		XMutex->Unlock();
	}
	return array;
L_Error:
	if (buf) {
		XMutex->Lock();
		XFree(buf);
		XMutex->Unlock();
	}
	array.Empty();
	*nitems_return=0;
	if (deleteProperty) {
		XMutex->Lock();
		XDeleteProperty(display,window,property);
		XMutex->Unlock();
	}
	return array;
}


emString emX11Clipboard::Latin1ToUtf8(const emString & latin1)
{
	emString res;
	const char * s;
	char * buf, * t;
	int k,n,c;

	s=latin1.Get();
	for (n=0, k=0; s[n]; n++) {
		if ((s[n]&0x80)!=0) k++;
	}
	if (!k) return latin1;
	buf=(char*)malloc(n+k*5+1);
	t=buf;
	do {
		c=(unsigned char)*s;
		s++;
		n=emEncodeUtf8Char(t,c);
		t+=n;
	} while (c);
	res=buf;
	free(buf);
	return res;
}


emString emX11Clipboard::Utf8ToLatin1(const emString & utf8)
{
	emString res;
	const char * s;
	char * buf, * t;
	int k,n,c;

	s=utf8.Get();
	for (n=0, k=0; s[n]; n++) {
		if ((s[n]&0x80)!=0) k++;
	}
	if (!k) return utf8;
	buf=(char*)malloc(n+1);
	t=buf;
	do {
		n=emDecodeUtf8Char(&c,s);
		if (n<1) {
			c=(unsigned char)*s;
			n=1;
		}
		if (c>255) c='?';
		*t=(char)c;
		s+=n;
		t++;
	} while (c);
	res=buf;
	free(buf);
	return res;
}
