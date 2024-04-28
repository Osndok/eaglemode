//------------------------------------------------------------------------------
// emHmiDemoPiece.cpp
//
// Copyright (C) 2012,2016,2020,2024 Oliver Hamann.
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

#include <emHmiDemo/emHmiDemoPiece.h>
#include <emCore/emRes.h>


emHmiDemoPiece::emHmiDemoPiece(
	ParentArg parent, const emString & name, int rx, int ry, int rw,
	int rh,  const emString & imageName, emColor color
)
	: emPanel(parent,name)
{
	Mask=emGetInsResImage(
		GetRootContext(),
		"emHmiDemo",
		emGetChildPath("Pieces",imageName+"mask.tga")
	);
	Image=emGetInsResImage(
		GetRootContext(),
		"emHmiDemo",
		emGetChildPath("Pieces",imageName+".tga")
	);
	RX=rx;
	RY=ry;
	RW=rw;
	RH=rh;
	Color=color;
	UpdateInnerColor();
	SetFocusable(false);
}


emHmiDemoPiece::~emHmiDemoPiece()
{
}


void emHmiDemoPiece::SetColor(emColor color)
{
	if (Color!=color) {
		Color=color;
		UpdateInnerColor();
		InvalidatePainting();
		InvalidateChildrenLayout();
	}
}


bool emHmiDemoPiece::IsOpaque() const
{
	return false;
}


void emHmiDemoPiece::Paint(const emPainter & painter, emColor canvasColor) const
{
	double h;

	h=GetHeight();
	painter.PaintImageColored(0,0,1,h,Mask,0,Color,canvasColor,emTexture::EXTEND_EDGE);
	painter.PaintImage(0,0,1,h,Image,255,Color,emTexture::EXTEND_EDGE);
}


void emHmiDemoPiece::UpdateInnerColor()
{
	emColor imgC;

	imgC=Image.GetPixel(Image.GetWidth()/2,Image.GetHeight()/2);
	InnerColor=Color.GetBlended(imgC,imgC.GetAlpha()/2.55F);
	InnerColor.SetAlpha(255);
}
