//------------------------------------------------------------------------------
// emTestPackLayout.cpp
//
// Copyright (C) 2015 Oliver Hamann.
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

#include <emCore/emGUIFramework.h>
#include <emCore/emToolkit.h>


class emTestPackLayout : public emPackLayout {
public:
	emTestPackLayout(ParentArg parent, const emString & name, int panelCount);
};


emTestPackLayout::emTestPackLayout(
	ParentArg parent, const emString & name, int panelCount
) :
	emPackLayout(parent,name)
{
	emString text;
	emColor color;
	emLook look;
	emLabel * label;
	double weight,pct;
	int i;

	for (i=0; i<panelCount; i++) {
		weight=emGetDblRandom(1.0,100.0);
		pct=exp(emGetDblRandom(-2.5,2.5));
		SetChildWeight(i,weight);
		SetPrefChildTallness(i,pct);
		color.SetHSVA(emGetIntRandom(0,359),50.0,50.0);
		look.SetBgColor(color);
		text=emString::Format("%g",pct);
		label=new emLabel(this,emString::Format("%06d",i),text);
		label->SetBorderType(OBT_FILLED,IBT_NONE);
		label->SetLook(look);
	}
}


MAIN_OR_WINMAIN_HERE

static int wrapped_main(int argc, char * argv[])
{
	int panelCount;

	emInitLocale();

	emEnableDLog();

	if (argc != 2) {
		fprintf(stderr,"Usage: %s <panel count>\n", argv[0]);
		return 1;
	}
	panelCount=atoi(argv[1]);

	emGUIFramework framework;
	framework.EnableAutoTermination();

	emWindow * window=new emWindow(framework.GetRootContext());
	window->SetWindowFlags(emWindow::WF_AUTO_DELETE);
	window->SetViewFlags(emView::VF_ROOT_SAME_TALLNESS);

	new emTestPackLayout(window,"root",panelCount);

	return framework.Run();
}
