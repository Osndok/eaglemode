//------------------------------------------------------------------------------
// emFileManTheme.cpp
//
// Copyright (C) 2010-2011,2014-2016 Oliver Hamann.
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
	Image.Clear();
}


const char * emFileManTheme::GetFormatName() const
{
	return "emFileManTheme";
}


emFileManTheme::emFileManTheme(emContext & context, const emString & name)
	: emConfigModel(context,name),
	emStructRec(),
	DisplayName(this,"DisplayName"),
	DisplayIcon(this,"DisplayIcon"),
	BackgroundColor(this,"BackgroundColor"),
	SourceSelectionColor(this,"SourceSelectionColor"),
	TargetSelectionColor(this,"TargetSelectionColor"),
	NormalNameColor(this,"NormalNameColor",emColor(0,0,0,255),true),
	ExeNameColor(this,"ExeNameColor",emColor(0,0,0,255),true),
	DirNameColor(this,"DirNameColor",emColor(0,0,0,255),true),
	FifoNameColor(this,"FifoNameColor",emColor(0,0,0,255),true),
	BlkNameColor(this,"BlkNameColor",emColor(0,0,0,255),true),
	ChrNameColor(this,"ChrNameColor",emColor(0,0,0,255),true),
	SockNameColor(this,"SockNameColor",emColor(0,0,0,255),true),
	OtherNameColor(this,"OtherNameColor",emColor(0,0,0,255),true),
	PathColor(this,"PathColor",emColor(0,0,0,255),true),
	SymLinkColor(this,"SymLinkColor",emColor(0,0,0,255),true),
	LabelColor(this,"LabelColor",emColor(0,0,0,255),true),
	InfoColor(this,"InfoColor",emColor(0,0,0,255),true),
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
	MinAltVW(this,"MinAltVW"),
	DirPaddingL(this,"DirPaddingL"),
	DirPaddingT(this,"DirPaddingT"),
	DirPaddingR(this,"DirPaddingR"),
	DirPaddingB(this,"DirPaddingB"),
	LnkPaddingL(this,"LnkPaddingL"),
	LnkPaddingT(this,"LnkPaddingT"),
	LnkPaddingR(this,"LnkPaddingR"),
	LnkPaddingB(this,"LnkPaddingB")
{
	PostConstruct(
		*this,
		emGetChildPath(GetThemesDirPath(), name + ThemeFileEnding)
	);
	Load();
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
	int i, j, eLen, nLen;
	emRef<emFileManTheme> t;
	ThemeStyle * s;
	ThemeAR * a;

	try {
		names=emTryLoadDir(emFileManTheme::GetThemesDirPath());
	}
	catch (emException &) {
		names.Clear();
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

	for (i=0; i<names.GetCount(); i++) {
		t=emFileManTheme::Acquire(GetRootContext(),names[i]);
		for (j=ThemeStyles.GetCount()-1; j>=0; j--) {
			if (ThemeStyles[j].DisplayName==t->DisplayName.Get()) break;
		}
		if (j<0) {
			j=ThemeStyles.GetCount();
			ThemeStyles.AddNew();
		}
		s=&ThemeStyles.GetWritable(j);
		s->DisplayName=t->DisplayName.Get();
		s->DisplayIcon=t->DisplayIcon.Get();
		for (j=0; j<s->ThemeARs.GetCount(); j++) {
			if (s->ThemeARs[j].Height>t->Height.Get()) break;
		}
		s->ThemeARs.InsertNew(j);
		a=&s->ThemeARs.GetWritable(j);
		a->Name=names[i];
		a->Height=t->Height.Get();
		a->AspectRatio=HeightToAspectRatioString(a->Height);
		t=NULL;
	}

	for (i=0; i<ThemeStyles.GetCount(); i++) {
		for (j=0; j<ThemeStyles[i].ThemeARs.GetCount(); j++) {
			NameToPackedIndex.Insert(
				ThemeStyles[i].ThemeARs[j].Name,
				(i<<16)|j
			);
		}
	}

	SetMinCommonLifetime(INT_MAX);
}


emFileManThemeNames::~emFileManThemeNames()
{
}


int emFileManThemeNames::GetThemeAspectRatioCount(int styleIndex) const
{
	if (styleIndex<0 || styleIndex>=ThemeStyles.GetCount()) {
		return 0;
	}
	return ThemeStyles[styleIndex].ThemeARs.GetCount();
}


emString emFileManThemeNames::GetThemeName(
	int styleIndex, int aspectRatioIndex
) const
{
	if (styleIndex<0 || styleIndex>=ThemeStyles.GetCount()) {
		return emString();
	}
	const ThemeStyle & style=ThemeStyles[styleIndex];
	if (aspectRatioIndex<0 || aspectRatioIndex>=style.ThemeARs.GetCount()) {
		return emString();
	}
	return style.ThemeARs[aspectRatioIndex].Name;
}


emString emFileManThemeNames::GetDefaultThemeName() const
{
	emString name;

	name="Glass1";
	if (!NameToPackedIndex.Contains(name)) {
		name=GetThemeName(0,0);
	}
	return name;
}


emString emFileManThemeNames::GetThemeStyleDisplayName(
	int styleIndex
) const
{
	if (styleIndex<0 || styleIndex>=ThemeStyles.GetCount()) {
		return emString();
	}
	return ThemeStyles[styleIndex].DisplayName;
}


emString emFileManThemeNames::GetThemeStyleDisplayIcon(
	int styleIndex
) const
{
	if (styleIndex<0 || styleIndex>=ThemeStyles.GetCount()) {
		return emString();
	}
	return ThemeStyles[styleIndex].DisplayIcon;
}


emString emFileManThemeNames::GetThemeAspectRatio(
	int styleIndex, int aspectRatioIndex
) const
{
	if (styleIndex<0 || styleIndex>=ThemeStyles.GetCount()) {
		return emString();
	}
	const ThemeStyle & style=ThemeStyles[styleIndex];
	if (aspectRatioIndex<0 || aspectRatioIndex>=style.ThemeARs.GetCount()) {
		return emString();
	}
	return style.ThemeARs[aspectRatioIndex].AspectRatio;
}


bool emFileManThemeNames::IsExistingThemeName(const emString & themeName) const
{
	return NameToPackedIndex.Contains(themeName);
}


int emFileManThemeNames::GetThemeStyleIndex(
	const emString & themeName
) const
{
	const emAvlTreeMap<emString,int>::Element * e;

	e=NameToPackedIndex.Get(themeName);
	if (!e) return -1;
	return e->Value>>16;
}


int emFileManThemeNames::GetThemeAspectRatioIndex(
	const emString & themeName
) const
{
	const emAvlTreeMap<emString,int>::Element * e;

	e=NameToPackedIndex.Get(themeName);
	if (!e) return -1;
	return e->Value&0xFFFF;
}


emString emFileManThemeNames::HeightToAspectRatioString(double height)
{
	int n,d,bestN,bestD;

	bestN=bestD=1;
	for (d=1; d<=10; d++) {
		n=(int)(d/height+0.5);
		if (n<1) n=1;
		if (fabs(height*n/d-1.0)<fabs(height*bestN/bestD-1.0)-0.001) {
			bestN=n;
			bestD=d;
		}
	}
	return emString::Format("%d:%d",bestN,bestD);
}
