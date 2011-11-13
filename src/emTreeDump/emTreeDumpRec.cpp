//------------------------------------------------------------------------------
// emTreeDumpRec.cpp
//
// Copyright (C) 2007-2008,2011 Oliver Hamann.
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

#include <emTreeDump/emTreeDumpRec.h>


emTreeDumpRec::emTreeDumpRec()
	: emStructRec(),
	Frame(
		this,"frame",FRAME_RECTANGLE,
		"none","rectangle","roundrect","ellipse","hexagon",
		NULL
	),
	BgColor(this,"bgcolor",emColor::WHITE,true),
	FgColor(this,"fgcolor",emColor::BLACK,true),
	Title(this,"title"),
	Text(this,"text"),
	Commands(this,"commands"),
	Files(this,"files"),
	Children(this,"children")
{
}


emTreeDumpRec::~emTreeDumpRec()
{
}


const char * emTreeDumpRec::GetFormatName() const
{
	return "emTreeDump";
}


emTreeDumpRec::CommandRec::CommandRec()
	: emStructRec(),
	Caption(this,"caption"),
	Args(this,"args")
{
}


emTreeDumpRec::CommandRec::~CommandRec()
{
}
