//------------------------------------------------------------------------------
// emFileManControlPanel.h
//
// Copyright (C) 2006-2008,2010 Oliver Hamann.
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


class emFileManControlPanel : public emTkGroup {

public:

	emFileManControlPanel(ParentArg parent, const emString & name,
	                      emView & contentView);

	virtual ~emFileManControlPanel();

protected:

	virtual bool Cycle();

private:

	void UpdateButtonStates();

	class Group : public emTkGroup {
	public:
		Group(ParentArg parent, const emString & name,
		      emView & contentView, emFileManModel * fmModel,
		      const emFileManModel::CommandNode * cmd);
		virtual ~Group();
	protected:
		virtual bool Cycle();
		virtual void AutoExpand();
	private:
		class Button : public emTkButton {
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

	emTkGroup * GrView;
		emTkRadioButton * RbSortByName;
		emTkRadioButton * RbSortByEnding;
		emTkRadioButton * RbSortByClass;
		emTkRadioButton * RbSortByVersion;
		emTkRadioButton * RbSortByDate;
		emTkRadioButton * RbSortBySize;
		emTkRadioButton * RbPerLocale;
		emTkRadioButton * RbCaseSensitive;
		emTkRadioButton * RbCaseInsensitive;
		emTkCheckButton * CbSortDirectoriesFirst;
		emTkCheckButton * CbShowHiddenFiles;
		emTkRadioButton::Group * RbgTheme;
		emTkCheckButton * CbAutosave;
		emTkButton * BtSaveAsDefault;
	emTkGroup * GrSelection;
		emTkButton * BtSelectAll;
		emTkButton * BtClearSelection;
		emTkButton * BtSwapSelection;
		emTkButton * BtPaths2Clipboard;
		emTkButton * BtNames2Clipboard;
		emTkGroup * GrSelInfo;
			emPanel * SelInfo;
	emTkGroup * GrCommand;
};


#endif
