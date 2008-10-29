//------------------------------------------------------------------------------
// emATMatrix.h
//
// Copyright (C) 2005-2008 Oliver Hamann.
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

#ifndef emATMatrix_h
#define emATMatrix_h

#ifndef emStd1_h
#include <emCore/emStd1.h>
#endif


//==============================================================================
//================================= emATMatrix =================================
//==============================================================================

class emATMatrix {

public:

	// Class for an affine transformation matrix. It is a 3x3 matrix with
	// constant elements in the last column:
	//    _                 _
	//   |   a00  a01  0.0   |
	//   |   a10  a11  0.0   |
	//   |_  a20  a21  1.0  _|
	//
	// Transforming a source coordinate (sx,sy) to a target coordinate
	// (tx,ty) is:
	//
	//   tx = a00*sx + a10*sy + a20
	//   ty = a01*sx + a11*sy + a21
	//
	// Please see the classes emIdentityATM, emInvertATM, emMultiplyATM,
	// emTranslateATM, emScaleATM, emRotateATM and emShearATM more below.
	// They provide lots of constructors for creating typical matrices. Here
	// is an example of constructing a matrix which performs rotation first,
	// then scaling and finally translation:
	//
	//   emTranslateATM(5.0,7.0,emScaleATM(2.0,3.0,emRotateATM(90.0)))
	//
	// The following constructing would result equal, but a bit slower:
	//
	//   emRotateATM(90.0) * emScaleATM(2.0,3.0) * emTranslateATM(5.0,7.0)


	emATMatrix();
		// Performs no initialization (this is a "primitive" type).

	emATMatrix(double a00, double a01,
	           double a10, double a11,
	           double a20, double a21);
		// Construct from the given elements.

	emATMatrix(const emATMatrix & m);
		// Construct a copied matrix.

	emATMatrix & operator = (const emATMatrix & m);
		// Copy a matrix.

	double Get(int i, int j) const;
	void Set(int i, int j, double aij);
		// Get or set an element of this matrix. i is the row index and
		// must be 0, 1 or 2. j is the column index and must be 0 or 1.

	double TransX(double sx, double sy) const;
	double TransY(double sx, double sy) const;
		// Transform a point from source coordinates to target
		// coordinates.

	double InverseTransX(double tx, double ty) const;
	double InverseTransY(double tx, double ty) const;
		// Transform a point from target coordinates back to source
		// coordinates. This is quite slower than TransX and TransY. For
		// inverse-transforming more than about 4 points, it would be
		// faster to create and use an emInvertATM.

	bool operator == (const emATMatrix & m) const;
	bool operator != (const emATMatrix & m) const;
		// Compare matrices.

	emATMatrix & operator *= (const emATMatrix & m);
		// Like *this = emMultiplyATM(*this,m).

	emATMatrix operator * (const emATMatrix & m) const;
		// Like emMultiplyATM(*this,m).

private:
	friend class emIdentityATM;
	friend class emInvertATM;
	friend class emMultiplyATM;
	friend class emTranslateATM;
	friend class emScaleATM;
	friend class emRotateATM;
	friend class emShearATM;
	double A[3][2];
};


//==============================================================================
//=============================== emIdentityATM ================================
//==============================================================================

class emIdentityATM : public emATMatrix {

public:

	emIdentityATM();
		// Construct an identity matrix. This means, source coordinates
		// are equal to target coordinates.
};


//==============================================================================
//================================ emInvertATM =================================
//==============================================================================

class emInvertATM : public emATMatrix {

public:

	emInvertATM(const emATMatrix & m);
		// Construct an inverse matrix. It performs inverse
		// transformations in compare to m.
};


//==============================================================================
//=============================== emMultiplyATM ================================
//==============================================================================

class emMultiplyATM : public emATMatrix {

public:

	emMultiplyATM(const emATMatrix & m1, const emATMatrix & m2);
		// Construct a multiplied matrix. Transforming a point with this
		// matrix gives the same result as transforming the point first
		// with m1 and then with m2.

	emMultiplyATM(const emATMatrix & m1, const emATMatrix & m2,
	              const emATMatrix & m3);
		// Like emMultiplyATM(emMultiplyATM(m1,m2),m3)

	emMultiplyATM(const emATMatrix & m1, const emATMatrix & m2,
	              const emATMatrix & m3, const emATMatrix & m4);
		// Like emMultiplyATM(emMultiplyATM(m1,m2,m3),m4)
};


//==============================================================================
//=============================== emTranslateATM ===============================
//==============================================================================

class emTranslateATM : public emATMatrix {

public:

	emTranslateATM(double addX, double addY);
		// Construct a matrix for translating. This matrix transforms
		// source coordinates (sx,sy) to target coordinates (tx,ty)
		// with:
		//   tx = sx + addX
		//   ty = sy + addY

	emTranslateATM(double addX, double addY, const emATMatrix & m);
		// Like:
		//   emMultiplyATM(
		//     m,
		//     emTranslateATM(addX,addY)
		//   )
};


//==============================================================================
//================================= emScaleATM =================================
//==============================================================================

class emScaleATM : public emATMatrix {

public:

	emScaleATM(double facX, double facY);
		// Construct a matrix for scaling. This matrix transforms source
		// coordinates (sx,sy) to target coordinates (tx,ty) with:
		//   tx = sx * facX
		//   ty = sy * facY

	emScaleATM(double facX, double facY, const emATMatrix & m);
		// Like:
		//   emMultiplyATM(
		//     m,
		//     emScaleATM(facX,facY)
		//   )

	emScaleATM(double facX, double facY, double fixX, double fixY);
		// Like:
		//   emMultiplyATM(
		//     emTranslateATM(-fixX,-fixY),
		//     emScaleATM(facX,facY),
		//     emTranslateATM(fixX,fixY)
		//   )

	emScaleATM(double facX, double facY, double fixX, double fixY,
	           const emATMatrix & m);
		// Like:
		//   emMultiplyATM(
		//     m,
		//     emTranslateATM(-fixX,-fixY),
		//     emScaleATM(facX,facY),
		//     emTranslateATM(fixX,fixY)
		//   )
};


//==============================================================================
//================================ emRotateATM =================================
//==============================================================================

class emRotateATM : public emATMatrix {

public:

	emRotateATM(double angle);
		// Construct a matrix for rotating. The angle is in degrees.
		// This matrix transforms source coordinates (sx,sy) to target
		// coordinates (tx,ty) with:
		//   tx = sx*cos(angle/180.0*M_PI) - sy*sin(angle/180.0*M_PI)
		//   ty = sy*cos(angle/180.0*M_PI) + sx*sin(angle/180.0*M_PI)

	emRotateATM(double angle, const emATMatrix & m);
		// Like:
		//   emMultiplyATM(
		//     m,
		//     emRotateATM(angle)
		//   )

	emRotateATM(double angle, double fixX, double fixY);
		// Like:
		//   emMultiplyATM(
		//     emTranslateATM(-fixX,-fixY),
		//     emRotateATM(angle),
		//     emTranslateATM(fixX,fixY)
		//   )

	emRotateATM(double angle, double fixX, double fixY,
	            const emATMatrix & m);
		// Like:
		//   emMultiplyATM(
		//     m,
		//     emTranslateATM(-fixX,-fixY),
		//     emRotateATM(angle),
		//     emTranslateATM(fixX,fixY)
		//   )
};


//==============================================================================
//================================= emShearATM =================================
//==============================================================================

class emShearATM : public emATMatrix {

public:

	emShearATM(double shX, double shY);
		// Construct a matrix for shearing. This matrix transforms
		// source coordinates (sx,sy) to target coordinates (tx,ty)
		// with:
		//    tx = sx + sy*shX
		//    ty = sy + sx*shY

	emShearATM(double shX, double shY, const emATMatrix & m);
		// Like:
		//   emMultiplyATM(
		//     m,
		//     emShearATM(shX,shY)
		//   )

	emShearATM(double shX, double shY, double fixX, double fixY);
		// Like:
		//   emMultiplyATM(
		//     emTranslateATM(-fixX,-fixY),
		//     emShearATM(shX,shY),
		//     emTranslateATM(fixX,fixY)
		//   )

	emShearATM(double shX, double shY, double fixX, double fixY,
	           const emATMatrix & m);
		// Like:
		//   emMultiplyATM(
		//     m,
		//     emTranslateATM(-fixX,-fixY),
		//     emShearATM(shX,shY),
		//     emTranslateATM(fixX,fixY)
		//   )
};


//==============================================================================
//=========================== Inline implementations ===========================
//==============================================================================

inline emATMatrix::emATMatrix()
{
}

inline double emATMatrix::Get(int i, int j) const
{
	return A[i][j];
}

inline void emATMatrix::Set(int i, int j, double aij)
{
	A[i][j]=aij;
}

inline double emATMatrix::TransX(double sx, double sy) const
{
	return A[0][0]*sx + A[1][0]*sy + A[2][0];
}

inline double emATMatrix::TransY(double sx, double sy) const
{
	return A[0][1]*sx + A[1][1]*sy + A[2][1];
}


#endif
