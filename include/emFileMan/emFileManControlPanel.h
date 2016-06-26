//------------------------------------------------------------------------------
// emFileManControlPanel.h
//
// Copyright (C) 2006-2008,2010,2014-2016 Oliver Hamann.
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

#ifndef emFileManControlPanel_h
#define emFileManControlPanel_h

#ifndef emFileManModel_h
#include <emFileMan/emFileManModel.h>
#endif

#ifndef emFileManViewConfig_h
#include <emFileMan/emFileManViewConfig.h>
#endif


class emFileManControlPanel : public emLinearLayout {

public:

	emFileManControlPanel(ParentArg parent, const emString & name,
	                      emView & contentView);

	virtual ~emFileManControlPanel();

protected:

	virtual bool Cycle();

private:

	void UpdateButtonStates();

	class Group : public emRasterGroup {
	public:
		Group(ParentArg parent, const emString & name,
		      emView & contentView, emFileManModel * fmModel,
		      const emFileManModel::CommandNode * cmd);
		virtual ~Group();
	protected:
		virtual bool Cycle();
		virtual void AutoExpand();
	private:
		class Button : public emButton {
		public:
			Button(ParentArg parent, const emString & name,
			       emView & contentView, emFileManModel * fmModel,
			       const emFileManModel::CommandNode * cmd);
			virtual ~Button();
		protected:
			virtual void Clicked();
		private:
			emView & ContentView;
			emRef<emFileManModel> FMModel;
			emString CmdPath;
		};
		emView & ContentView;
		emRef<emFileManModel> FMModel;
		emString CmdPath;
	};

	emView & ContentView;
	emRef<emFileManModel> FMModel;
	emRef<emFileManViewConfig> FMVConfig;
	emRef<emFileManThemeNames> FMThemeNames;

	emPackGroup * GrView;
		emRadioButton::Mechanism RbmAspect;
		emRadioButton::Mechanism RbmTheme;
		emRadioButton * RbSortByName;
		emRadioButton * RbSortByDate;
		emRadioButton * RbSortBySize;
		emRadioButton * RbSortByEnding;
		emRadioButton * RbSortByClass;
		emRadioButton * RbSortByVersion;
		emCheckButton * CbSortDirectoriesFirst;
		emCheckButton * CbShowHiddenFiles;
		emRadioButton * RbPerLocale;
		emRadioButton * RbCaseSensitive;
		emRadioButton * RbCaseInsensitive;
		emCheckButton * CbAutosave;
		emButton * BtSaveAsDefault;
	emRasterGroup * GrSelection;
		emButton * BtSelectAll;
		emButton * BtClearSelection;
		emButton * BtSwapSelection;
		emButton * BtPaths2Clipboard;
		emButton * BtNames2Clipboard;
		emLinearGroup * GrSelInfo;
			emPanel * SelInfo;
	emRasterGroup * GrCommand;
};


#endif
