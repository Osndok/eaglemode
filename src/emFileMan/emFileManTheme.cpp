//------------------------------------------------------------------------------
// emFileManTheme.cpp
//
// Copyright (C) 2010-2011 Oliver Hamann.
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

#include <emCore/emInstallInfo.h>
#include <emCore/emRes.h>
#include <emFileMan/emFileManTheme.h>


//==============================================================================
//=============================== emFileManTheme ===============================
//==============================================================================

emString emFileManTheme::GetThemesDirPath()
{
	return emGetInstallPath(EM_IDT_RES,"emFileMan","themes");
}


const char * emFileManTheme::ThemeFileEnding = ".emFileManTheme";


emRef<emFileManTheme> emFileManTheme::Acquire(
	emRootContext & rootContext, const emString & name
)
{
	EM_IMPL_ACQUIRE_COMMON(emFileManTheme,rootContext,name)
}


emFileManTheme::ImageFileRec::ImageFileRec(
	emFileManTheme * parent, const char * varIdentifier, emRootContext & rootContext
)
	: emStringRec(parent,varIdentifier),
	emRecListener(this),
	RootContext(rootContext)
{
}


const emImage & emFileManTheme::ImageFileRec::GetImage() const
{
	if (Image.IsEmpty() && !Get().IsEmpty()) {
		((emFileManTheme::ImageFileRec*)this)->Image=emGetResImage(
			RootContext,
			emGetChildPath(
				emGetParentPath(
					((const emFileManTheme*)GetParent())->GetInstallPath()
				),
				Get()
			)
		);
	}
	return Image;
}


void emFileManTheme::ImageFileRec::OnRecChanged()
{
	Image.Empty();
}


const char * emFileManTheme::GetFormatName() const
{
	return "emFileManTheme";
}


emFileManTheme::emFileManTheme(emContext & context, const emString & name)
	: emConfigModel(context,name),
	emStructRec(),
	DisplayName(this,"DisplayName"),
	BackgroundColor(this,"BackgroundColor"),
	SourceSelectionColor(this,"SourceSelectionColor"),
	TargetSelectionColor(this,"TargetSelectionColor"),
	NormalNameColor(this,"NormalNameColor"),
	ExeNameColor(this,"ExeNameColor"),
	DirNameColor(this,"DirNameColor"),
	FifoNameColor(this,"FifoNameColor"),
	BlkNameColor(this,"BlkNameColor"),
	ChrNameColor(this,"ChrNameColor"),
	SockNameColor(this,"SockNameColor"),
	OtherNameColor(this,"OtherNameColor"),
	PathColor(this,"PathColor"),
	SymLinkColor(this,"SymLinkColor"),
	LabelColor(this,"LabelColor"),
	InfoColor(this,"InfoColor"),
	FileContentColor(this,"FileContentColor"),
	DirContentColor(this,"DirContentColor"),
	Height(this,"Height"),
	BackgroundX(this,"BackgroundX"),
	BackgroundY(this,"BackgroundY"),
	BackgroundW(this,"BackgroundW"),
	BackgroundH(this,"BackgroundH"),
	BackgroundRX(this,"BackgroundRX"),
	BackgroundRY(this,"BackgroundRY"),
	OuterBorderX(this,"OuterBorderX"),
	OuterBorderY(this,"OuterBorderY"),
	OuterBorderW(this,"OuterBorderW"),
	OuterBorderH(this,"OuterBorderH"),
	OuterBorderL(this,"OuterBorderL"),
	OuterBorderT(this,"OuterBorderT"),
	OuterBorderR(this,"OuterBorderR"),
	OuterBorderB(this,"OuterBorderB"),
	OuterBorderImg(this,"OuterBorderImg",GetRootContext()),
	OuterBorderImgL(this,"OuterBorderImgL"),
	OuterBorderImgT(this,"OuterBorderImgT"),
	OuterBorderImgR(this,"OuterBorderImgR"),
	OuterBorderImgB(this,"OuterBorderImgB"),
	NameX(this,"NameX"),
	NameY(this,"NameY"),
	NameW(this,"NameW"),
	NameH(this,"NameH"),
	NameAlignment(this,"NameAlignment"),
	PathX(this,"PathX"),
	PathY(this,"PathY"),
	PathW(this,"PathW"),
	PathH(this,"PathH"),
	PathAlignment(this,"PathAlignment"),
	InfoX(this,"InfoX"),
	InfoY(this,"InfoY"),
	InfoW(this,"InfoW"),
	InfoH(this,"InfoH"),
	InfoAlignment(this,"InfoAlignment"),
	FileInnerBorderX(this,"FileInnerBorderX"),
	FileInnerBorderY(this,"FileInnerBorderY"),
	FileInnerBorderW(this,"FileInnerBorderW"),
	FileInnerBorderH(this,"FileInnerBorderH"),
	FileInnerBorderL(this,"FileInnerBorderL"),
	FileInnerBorderT(this,"FileInnerBorderT"),
	FileInnerBorderR(this,"FileInnerBorderR"),
	FileInnerBorderB(this,"FileInnerBorderB"),
	FileInnerBorderImg(this,"FileInnerBorderImg",GetRootContext()),
	FileInnerBorderImgL(this,"FileInnerBorderImgL"),
	FileInnerBorderImgT(this,"FileInnerBorderImgT"),
	FileInnerBorderImgR(this,"FileInnerBorderImgR"),
	FileInnerBorderImgB(this,"FileInnerBorderImgB"),
	FileContentX(this,"FileContentX"),
	FileContentY(this,"FileContentY"),
	FileContentW(this,"FileContentW"),
	FileContentH(this,"FileContentH"),
	DirInnerBorderX(this,"DirInnerBorderX"),
	DirInnerBorderY(this,"DirInnerBorderY"),
	DirInnerBorderW(this,"DirInnerBorderW"),
	DirInnerBorderH(this,"DirInnerBorderH"),
	DirInnerBorderL(this,"DirInnerBorderL"),
	DirInnerBorderT(this,"DirInnerBorderT"),
	DirInnerBorderR(this,"DirInnerBorderR"),
	DirInnerBorderB(this,"DirInnerBorderB"),
	DirInnerBorderImg(this,"DirInnerBorderImg",GetRootContext()),
	DirInnerBorderImgL(this,"DirInnerBorderImgL"),
	DirInnerBorderImgT(this,"DirInnerBorderImgT"),
	DirInnerBorderImgR(this,"DirInnerBorderImgR"),
	DirInnerBorderImgB(this,"DirInnerBorderImgB"),
	DirContentX(this,"DirContentX"),
	DirContentY(this,"DirContentY"),
	DirContentW(this,"DirContentW"),
	DirContentH(this,"DirContentH"),
	AltX(this,"AltX"),
	AltY(this,"AltY"),
	AltW(this,"AltW"),
	AltH(this,"AltH"),
	AltLabelX(this,"AltLabelX"),
	AltLabelY(this,"AltLabelY"),
	AltLabelW(this,"AltLabelW"),
	AltLabelH(this,"AltLabelH"),
	AltLabelAlignment(this,"AltLabelAlignment"),
	AltPathX(this,"AltPathX"),
	AltPathY(this,"AltPathY"),
	AltPathW(this,"AltPathW"),
	AltPathH(this,"AltPathH"),
	AltPathAlignment(this,"AltPathAlignment"),
	AltAltX(this,"AltAltX"),
	AltAltY(this,"AltAltY"),
	AltAltW(this,"AltAltW"),
	AltAltH(this,"AltAltH"),
	AltInnerBorderX(this,"AltInnerBorderX"),
	AltInnerBorderY(this,"AltInnerBorderY"),
	AltInnerBorderW(this,"AltInnerBorderW"),
	AltInnerBorderH(this,"AltInnerBorderH"),
	AltInnerBorderL(this,"AltInnerBorderL"),
	AltInnerBorderT(this,"AltInnerBorderT"),
	AltInnerBorderR(this,"AltInnerBorderR"),
	AltInnerBorderB(this,"AltInnerBorderB"),
	AltInnerBorderImg(this,"AltInnerBorderImg",GetRootContext()),
	AltInnerBorderImgL(this,"AltInnerBorderImgL"),
	AltInnerBorderImgT(this,"AltInnerBorderImgT"),
	AltInnerBorderImgR(this,"AltInnerBorderImgR"),
	AltInnerBorderImgB(this,"AltInnerBorderImgB"),
	AltContentX(this,"AltContentX"),
	AltContentY(this,"AltContentY"),
	AltContentW(this,"AltContentW"),
	AltContentH(this,"AltContentH"),
	MinContentVW(this,"MinContentVW"),
	MinAltVW(this,"MinAltVW")
{
	PostConstruct(
		*this,
		emGetChildPath(GetThemesDirPath(), name + ThemeFileEnding)
	);
	LoadOrInstall();
}


emFileManTheme::~emFileManTheme()
{
}


//==============================================================================
//============================ emFileManThemeNames =============================
//==============================================================================

emRef<emFileManThemeNames> emFileManThemeNames::Acquire(
	emRootContext & rootContext
)
{
	EM_IMPL_ACQUIRE_COMMON(emFileManThemeNames,rootContext,"")
}


emFileManThemeNames::emFileManThemeNames(emContext & context, const emString & name)
	: emModel(context,name)
{
	emArray<emString> names;
	int i, eLen, nLen;
	ThemeInfo * ti;
	emRef<emFileManTheme> t;

	try {
		names=emTryLoadDir(emFileManTheme::GetThemesDirPath());
	}
	catch (emString) {
		names.Empty();
	}
	eLen=strlen(emFileManTheme::ThemeFileEnding);
	for (i=0; i<names.GetCount(); ) {
		nLen=strlen(names[i])-eLen;
		if (nLen>0 && strcmp(names[i].Get()+nLen,emFileManTheme::ThemeFileEnding)==0) {
			names.GetWritable(i).Remove(nLen,eLen);
			i++;
		}
		else {
			names.Remove(i);
		}
	}
	names.Sort(emStdComparer<emString>::Compare);

	ThemeInfos.SetCount(names.GetCount());
	for (i=0; i<ThemeInfos.GetCount(); i++) {
		ti=&ThemeInfos.GetWritable(i);
		ti->Name=names[i];
		t=emFileManTheme::Acquire(GetRootContext(),ti->Name);
		ti->DisplayName=t->DisplayName;
		t=NULL;
	}

	SetMinCommonLifetime(INT_MAX);
}


emFileManThemeNames::~emFileManThemeNames()
{
}

