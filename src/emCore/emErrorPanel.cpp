//------------------------------------------------------------------------------
// emErrorPanel.cpp
//
// Copyright (C) 2004-2008 Oliver Hamann.
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

#include <emCore/emErrorPanel.h>


emErrorPanel::emErrorPanel(
	ParentArg parent, const emString & name, const emString & errorMessage
)
	: emPanel(parent,name),
	ErrorMessage(errorMessage)
{
}


emErrorPanel::~emErrorPanel()
{
}


bool emErrorPanel::IsOpaque()
{
	return true;
}


void emErrorPanel::Paint(const emPainter & painter, emColor canvasColor)
{
	static const emColor bgColor(128,0,0);
	static const emColor fgColor(255,255,0);

	painter.PaintRect(0,0,1,GetHeight(),bgColor,canvasColor);
	painter.PaintTextBoxed(
		0.05,
		GetHeight()*0.05,
		0.9,
		GetHeight()*0.9,
		ErrorMessage,
		GetHeight()/2,
		fgColor,
		bgColor,
		EM_ALIGN_CENTER,
		EM_ALIGN_LEFT,
		1.0
	);
}
