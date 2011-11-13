//------------------------------------------------------------------------------
// emFileManTheme.h
//
// Copyright (C) 2010 Oliver Hamann.
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

#ifndef emFileManTheme_h
#define emFileManTheme_h

#ifndef emImage_h
#include <emCore/emImage.h>
#endif

#ifndef emConfigModel_h
#include <emCore/emConfigModel.h>
#endif


//==============================================================================
//=============================== emFileManTheme ===============================
//==============================================================================

class emFileManTheme : public emConfigModel, public emStructRec {

public:

	static emString GetThemesDirPath();

	static const char * ThemeFileEnding;

	static emRef<emFileManTheme> Acquire(
		emRootContext & rootContext, const emString & name
	);

	class ImageFileRec : public emStringRec, private emRecListener {

	public:

		ImageFileRec(
			emFileManTheme * parent, const char * varIdentifier,
			emRootContext & rootContext
		);

		const emImage & GetImage() const;

	private:

		virtual void OnRecChanged();

		emRootContext & RootContext;
		emImage Image;
	};

	emStringRec DisplayName;
	emColorRec BackgroundColor;
	emColorRec SourceSelectionColor;
	emColorRec TargetSelectionColor;
	emColorRec NormalNameColor;
	emColorRec ExeNameColor;
	emColorRec DirNameColor;
	emColorRec FifoNameColor;
	emColorRec BlkNameColor;
	emColorRec ChrNameColor;
	emColorRec SockNameColor;
	emColorRec OtherNameColor;
	emColorRec PathColor;
	emColorRec SymLinkColor;
	emColorRec LabelColor;
	emColorRec InfoColor;
	emColorRec FileContentColor;
	emColorRec DirContentColor;
	emDoubleRec Height;
	emDoubleRec BackgroundX;
	emDoubleRec BackgroundY;
	emDoubleRec BackgroundW;
	emDoubleRec BackgroundH;
	emDoubleRec BackgroundRX;
	emDoubleRec BackgroundRY;
	emDoubleRec OuterBorderX;
	emDoubleRec OuterBorderY;
	emDoubleRec OuterBorderW;
	emDoubleRec OuterBorderH;
	emDoubleRec OuterBorderL;
	emDoubleRec OuterBorderT;
	emDoubleRec OuterBorderR;
	emDoubleRec OuterBorderB;
	ImageFileRec OuterBorderImg;
	emDoubleRec OuterBorderImgL;
	emDoubleRec OuterBorderImgT;
	emDoubleRec OuterBorderImgR;
	emDoubleRec OuterBorderImgB;
	emDoubleRec NameX;
	emDoubleRec NameY;
	emDoubleRec NameW;
	emDoubleRec NameH;
	emAlignmentRec NameAlignment;
	emDoubleRec PathX;
	emDoubleRec PathY;
	emDoubleRec PathW;
	emDoubleRec PathH;
	emAlignmentRec PathAlignment;
	emDoubleRec InfoX;
	emDoubleRec InfoY;
	emDoubleRec InfoW;
	emDoubleRec InfoH;
	emAlignmentRec InfoAlignment;
	emDoubleRec FileInnerBorderX;
	emDoubleRec FileInnerBorderY;
	emDoubleRec FileInnerBorderW;
	emDoubleRec FileInnerBorderH;
	emDoubleRec FileInnerBorderL;
	emDoubleRec FileInnerBorderT;
	emDoubleRec FileInnerBorderR;
	emDoubleRec FileInnerBorderB;
	ImageFileRec FileInnerBorderImg;
	emDoubleRec FileInnerBorderImgL;
	emDoubleRec FileInnerBorderImgT;
	emDoubleRec FileInnerBorderImgR;
	emDoubleRec FileInnerBorderImgB;
	emDoubleRec FileContentX;
	emDoubleRec FileContentY;
	emDoubleRec FileContentW;
	emDoubleRec FileContentH;
	emDoubleRec DirInnerBorderX;
	emDoubleRec DirInnerBorderY;
	emDoubleRec DirInnerBorderW;
	emDoubleRec DirInnerBorderH;
	emDoubleRec DirInnerBorderL;
	emDoubleRec DirInnerBorderT;
	emDoubleRec DirInnerBorderR;
	emDoubleRec DirInnerBorderB;
	ImageFileRec DirInnerBorderImg;
	emDoubleRec DirInnerBorderImgL;
	emDoubleRec DirInnerBorderImgT;
	emDoubleRec DirInnerBorderImgR;
	emDoubleRec DirInnerBorderImgB;
	emDoubleRec DirContentX;
	emDoubleRec DirContentY;
	emDoubleRec DirContentW;
	emDoubleRec DirContentH;
	emDoubleRec AltX;
	emDoubleRec AltY;
	emDoubleRec AltW;
	emDoubleRec AltH;
	emDoubleRec AltLabelX;
	emDoubleRec AltLabelY;
	emDoubleRec AltLabelW;
	emDoubleRec AltLabelH;
	emAlignmentRec AltLabelAlignment;
	emDoubleRec AltPathX;
	emDoubleRec AltPathY;
	emDoubleRec AltPathW;
	emDoubleRec AltPathH;
	emAlignmentRec AltPathAlignment;
	emDoubleRec AltAltX;
	emDoubleRec AltAltY;
	emDoubleRec AltAltW;
	emDoubleRec AltAltH;
	emDoubleRec AltInnerBorderX;
	emDoubleRec AltInnerBorderY;
	emDoubleRec AltInnerBorderW;
	emDoubleRec AltInnerBorderH;
	emDoubleRec AltInnerBorderL;
	emDoubleRec AltInnerBorderT;
	emDoubleRec AltInnerBorderR;
	emDoubleRec AltInnerBorderB;
	ImageFileRec AltInnerBorderImg;
	emDoubleRec AltInnerBorderImgL;
	emDoubleRec AltInnerBorderImgT;
	emDoubleRec AltInnerBorderImgR;
	emDoubleRec AltInnerBorderImgB;
	emDoubleRec AltContentX;
	emDoubleRec AltContentY;
	emDoubleRec AltContentW;
	emDoubleRec AltContentH;
	emDoubleRec MinContentVW;
	emDoubleRec MinAltVW;

	virtual const char * GetFormatName() const;

protected:

	emFileManTheme(emContext & context, const emString & name);
	virtual ~emFileManTheme();
};


//==============================================================================
//============================ emFileManThemeNames =============================
//==============================================================================

class emFileManThemeNames : public emModel {

public:

	static emRef<emFileManThemeNames> Acquire(
		emRootContext & rootContext
	);

	int GetThemeCount() const;
	const emString & GetThemeName(int index) const;
	const emString & GetThemeDisplayName(int index) const;

protected:

	emFileManThemeNames(emContext & context, const emString & name);
	virtual ~emFileManThemeNames();

private:

	struct ThemeInfo {
		emString Name;
		emString DisplayName;
	};

	emArray<ThemeInfo> ThemeInfos;
};

inline int emFileManThemeNames::GetThemeCount() const
{
	return ThemeInfos.GetCount();
}

inline const emString & emFileManThemeNames::GetThemeName(int index) const
{
	return ThemeInfos[index].Name;
}

inline const emString & emFileManThemeNames::GetThemeDisplayName(int index) const
{
	return ThemeInfos[index].DisplayName;
}


#endif
