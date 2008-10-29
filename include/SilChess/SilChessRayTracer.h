//------------------------------------------------------------------------------
// SilChessRayTracer.h
//
// Copyright (C) 2001-2003,2007-2008 Oliver Hamann.
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

#ifndef SilChessRayTracer_h
#define SilChessRayTracer_h

#ifndef SilChessMachine_h
#include <SilChess/SilChessMachine.h>
#endif


class SilChessRayTracer {

public:

	SilChessRayTracer();
	~SilChessRayTracer();

	void SetWorld(const SilChessMachine * machine);

	// --- high-level interface ---

	void SetViewSize(int width, int height);

	void RenderScanline(int y, char * buf, int pixsize,
	                    int rmsk, int gmsk, int bmsk) const;

	void View2Board(int sx, int sy, int * bx, int * by) const;
	void Board2View(float bx, float by, int * sx, int * sy) const;

	// --- low-level interface ---

	struct Color {
		unsigned int Red,Green,Blue;
	};

	bool TraceRay(int depth,
	              float px, float py, float pz,
	              float rx, float ry, float rz,
	              Color * col=NULL) const;

	// ---------------------------

private:

	struct Material {
		Color Col;
		unsigned int Ambient,Diffuse,Reflect,Highlight;
	};

	struct Sphere {
		float x,y,z,r;
	};

	struct Piece {
		const Material * Mat;
		float BndX,BndY,BndRR,BndH;
		int SCnt;
		Sphere S[1];
	};

	static const unsigned char BoardTexture[16*128];
	static const Sphere PawnShape[];
	static const Sphere KnightShape[];
	static const Sphere BishopShape[];
	static const Sphere RookShape[];
	static const Sphere QueenShape[];
	static const Sphere KingShape[];
	static const Material BoardMaterial[2];
	static const Material PieceMaterial[2];
	static const Color SkyColor;
	static const Color GndColor;
	static const float LightX,LightY,LightZ;
	static const float CamPosY,CamPosZ,CamDirY,CamDirZ,CamZoom,CamAsp;

	float UCamDirY,UCamDirZ;
	float ULightX,ULightY,ULightZ;

	int ViewWidth,ViewHeight;
	float ViewCenterX,ViewCenterY,ViewZoom;

	Piece * Board[64];
	bool IsHumanWhite;
	float MaxBndH;

};


#endif
