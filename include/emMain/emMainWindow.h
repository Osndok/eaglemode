//------------------------------------------------------------------------------
// emMainWindow.h
//
// Copyright (C) 2006-2012,2016-2017 Oliver Hamann.
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

#ifndef emMainWindow_h
#define emMainWindow_h

#ifndef emViewAnimator_h
#include <emCore/emViewAnimator.h>
#endif

#ifndef emWindowStateSaver_h
#include <emCore/emWindowStateSaver.h>
#endif

#ifndef emAutoplay_h
#include <emMain/emAutoplay.h>
#endif

#ifndef emBookmarks_h
#include <emMain/emBookmarks.h>
#endif

#ifndef emMainPanel_h
#include <emMain/emMainPanel.h>
#endif


class emMainWindow : public emWindow {

public:

	emMainWindow(
		emContext & parentContext,
		const char * visitIdentity=NULL,
		double visitRelX=0.0,
		double visitRelY=0.0,
		double visitRelA=0.0,
		bool visitAdherent=false,
		const char * visitSubject=NULL,
		emColor ceColor=0
	);
	virtual ~emMainWindow();

	virtual emString GetTitle() const;

	emMainWindow * Duplicate();
	void ToggleFullscreen();
	void ReloadFiles();
	void ToggleControlView();
	void Close();
	void Quit();

protected:

	virtual bool Cycle();

	virtual void Input(emInputEvent & event, const emInputState & state);

	virtual void DoCustomCheat(const char * func);

private:

	static void RecreateContentPanels(emScreen & screen);
	void CreateControlWindow();

	class StartupEngineClass : public emEngine {
	public:
		StartupEngineClass(
			emMainWindow & mainWin,
			const char * visitIdentity,
			double visitRelX,
			double visitRelY,
			double visitRelA,
			bool visitAdherent,
			const char * visitSubject,
			emColor ceColor
		);
		virtual ~StartupEngineClass();
	protected:
		virtual bool Cycle();
	private:
		emMainWindow & MainWin;
		bool VisitValid;
		emString VisitIdentity;
		double VisitRelX;
		double VisitRelY;
		double VisitRelA;
		bool VisitAdherent;
		emString VisitSubject;
		emColor CeColor;
		emVisitingViewAnimator * VisitingVA;
		int State;
		emUInt64 Clk;
	};
	friend class StartupEngineClass;

	emWindowStateSaver WindowStateSaver;
	emRef<emBookmarksModel> BookmarksModel;
	emRef<emAutoplayViewModel> AutoplayViewModel;
	bool ToClose;
	emMainPanel * MainPanel;
	emPanel * ControlPanel;
	emPanel * ContentPanel;
	emCrossPtr<emWindow> ControlWindow;
	StartupEngineClass * StartupEngine;
};


#endif
