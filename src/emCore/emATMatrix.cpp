//------------------------------------------------------------------------------
// emATMatrix.cpp
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

#include <emCore/emATMatrix.h>


//==============================================================================
//================================= emATMatrix =================================
//==============================================================================

emATMatrix::emATMatrix(
	double a00, double a01,
	double a10, double a11,
	double a20, double a21
)
{
	A[0][0]=a00;
	A[0][1]=a01;
	A[1][0]=a10;
	A[1][1]=a11;
	A[2][0]=a20;
	A[2][1]=a21;
}


emATMatrix::emATMatrix(const emATMatrix & m)
{
	A[0][0]=m.A[0][0];
	A[0][1]=m.A[0][1];
	A[1][0]=m.A[1][0];
	A[1][1]=m.A[1][1];
	A[2][0]=m.A[2][0];
	A[2][1]=m.A[2][1];
}


emATMatrix & emATMatrix::operator = (const emATMatrix & m)
{
	A[0][0]=m.A[0][0];
	A[0][1]=m.A[0][1];
	A[1][0]=m.A[1][0];
	A[1][1]=m.A[1][1];
	A[2][0]=m.A[2][0];
	A[2][1]=m.A[2][1];
	return *this;
}


double emATMatrix::InverseTransX(double tx, double ty) const
{
	return
		(A[1][1]*tx-A[1][0]*ty+A[1][0]*A[2][1]-A[1][1]*A[2][0]) /
		(A[0][0]*A[1][1]-A[0][1]*A[1][0])
	;
}


double emATMatrix::InverseTransY(double tx, double ty) const
{
	return
		(A[0][0]*ty-A[0][1]*tx+A[0][1]*A[2][0]-A[0][0]*A[2][1]) /
		(A[0][0]*A[1][1]-A[0][1]*A[1][0])
	;
}


bool emATMatrix::operator == (const emATMatrix & m) const
{
	return
		A[0][0]==m.A[0][0] &&
		A[0][1]==m.A[0][1] &&
		A[1][0]==m.A[1][0] &&
		A[1][1]==m.A[1][1] &&
		A[2][0]==m.A[2][0] &&
		A[2][1]==m.A[2][1]
	;
}


bool emATMatrix::operator != (const emATMatrix & m) const
{
	return
		A[0][0]!=m.A[0][0] ||
		A[0][1]!=m.A[0][1] ||
		A[1][0]!=m.A[1][0] ||
		A[1][1]!=m.A[1][1] ||
		A[2][0]!=m.A[2][0] ||
		A[2][1]!=m.A[2][1]
	;
}


emATMatrix & emATMatrix::operator *= (const emATMatrix & m)
{
	double a00,a10,a20;

	a00=A[0][0];
	A[0][0]=a00*m.A[0][0]+A[0][1]*m.A[1][0];
	A[0][1]=a00*m.A[0][1]+A[0][1]*m.A[1][1];
	a10=A[1][0];
	A[1][0]=a10*m.A[0][0]+A[1][1]*m.A[1][0];
	A[1][1]=a10*m.A[0][1]+A[1][1]*m.A[1][1];
	a20=A[2][0];
	A[2][0]=a20*m.A[0][0]+A[2][1]*m.A[1][0]+m.A[2][0];
	A[2][1]=a20*m.A[0][1]+A[2][1]*m.A[1][1]+m.A[2][1];
	return *this;
}


emATMatrix emATMatrix::operator * (const emATMatrix & m) const
{
	return emATMatrix(
		A[0][0]*m.A[0][0]+A[0][1]*m.A[1][0],
		A[0][0]*m.A[0][1]+A[0][1]*m.A[1][1],
		A[1][0]*m.A[0][0]+A[1][1]*m.A[1][0],
		A[1][0]*m.A[0][1]+A[1][1]*m.A[1][1],
		A[2][0]*m.A[0][0]+A[2][1]*m.A[1][0]+m.A[2][0],
		A[2][0]*m.A[0][1]+A[2][1]*m.A[1][1]+m.A[2][1]
	);
}


//==============================================================================
//=============================== emIdentityATM ================================
//==============================================================================

emIdentityATM::emIdentityATM()
{
	A[0][0]=1.0;
	A[0][1]=0.0;
	A[1][0]=0.0;
	A[1][1]=1.0;
	A[2][0]=0.0;
	A[2][1]=0.0;
}


//==============================================================================
//================================ emInvertATM =================================
//==============================================================================

emInvertATM::emInvertATM(const emATMatrix & m)
{
	double p,n;

	p=1.0/(m.A[0][0]*m.A[1][1]-m.A[0][1]*m.A[1][0]);
	n=-p;
	A[0][0]=m.A[1][1]*p;
	A[0][1]=m.A[0][1]*n;
	A[1][0]=m.A[1][0]*n;
	A[1][1]=m.A[0][0]*p;
	A[2][0]=(m.A[1][0]*m.A[2][1]-m.A[1][1]*m.A[2][0])*p;
	A[2][1]=(m.A[0][0]*m.A[2][1]-m.A[0][1]*m.A[2][0])*n;
}


//==============================================================================
//=============================== emMultiplyATM ================================
//==============================================================================

emMultiplyATM::emMultiplyATM(const emATMatrix & m1, const emATMatrix & m2)
{
	A[0][0]=m1.A[0][0]*m2.A[0][0]+m1.A[0][1]*m2.A[1][0];
	A[0][1]=m1.A[0][0]*m2.A[0][1]+m1.A[0][1]*m2.A[1][1];
	A[1][0]=m1.A[1][0]*m2.A[0][0]+m1.A[1][1]*m2.A[1][0];
	A[1][1]=m1.A[1][0]*m2.A[0][1]+m1.A[1][1]*m2.A[1][1];
	A[2][0]=m1.A[2][0]*m2.A[0][0]+m1.A[2][1]*m2.A[1][0]+m2.A[2][0];
	A[2][1]=m1.A[2][0]*m2.A[0][1]+m1.A[2][1]*m2.A[1][1]+m2.A[2][1];
}


emMultiplyATM::emMultiplyATM(
	const emATMatrix & m1, const emATMatrix & m2, const emATMatrix & m3
)
{
	double a00,a01,a10,a11,a20,a21;

	a00=m1.A[0][0]*m2.A[0][0]+m1.A[0][1]*m2.A[1][0];
	a01=m1.A[0][0]*m2.A[0][1]+m1.A[0][1]*m2.A[1][1];
	A[0][0]=a00*m3.A[0][0]+a01*m3.A[1][0];
	A[0][1]=a00*m3.A[0][1]+a01*m3.A[1][1];
	a10=m1.A[1][0]*m2.A[0][0]+m1.A[1][1]*m2.A[1][0];
	a11=m1.A[1][0]*m2.A[0][1]+m1.A[1][1]*m2.A[1][1];
	A[1][0]=a10*m3.A[0][0]+a11*m3.A[1][0];
	A[1][1]=a10*m3.A[0][1]+a11*m3.A[1][1];
	a20=m1.A[2][0]*m2.A[0][0]+m1.A[2][1]*m2.A[1][0]+m2.A[2][0];
	a21=m1.A[2][0]*m2.A[0][1]+m1.A[2][1]*m2.A[1][1]+m2.A[2][1];
	A[2][0]=a20*m3.A[0][0]+a21*m3.A[1][0]+m3.A[2][0];
	A[2][1]=a20*m3.A[0][1]+a21*m3.A[1][1]+m3.A[2][1];
}


emMultiplyATM::emMultiplyATM(
	const emATMatrix & m1, const emATMatrix & m2, const emATMatrix & m3,
	const emATMatrix & m4
)
{
	double a00,a01,a10,a11,a20,a21,b00,b01,b10,b11,b20,b21;

	a00=m1.A[0][0]*m2.A[0][0]+m1.A[0][1]*m2.A[1][0];
	a01=m1.A[0][0]*m2.A[0][1]+m1.A[0][1]*m2.A[1][1];
	b00=a00*m3.A[0][0]+a01*m3.A[1][0];
	b01=a00*m3.A[0][1]+a01*m3.A[1][1];
	A[0][0]=b00*m4.A[0][0]+b01*m4.A[1][0];
	A[0][1]=b00*m4.A[0][1]+b01*m4.A[1][1];
	a10=m1.A[1][0]*m2.A[0][0]+m1.A[1][1]*m2.A[1][0];
	a11=m1.A[1][0]*m2.A[0][1]+m1.A[1][1]*m2.A[1][1];
	b10=a10*m3.A[0][0]+a11*m3.A[1][0];
	b11=a10*m3.A[0][1]+a11*m3.A[1][1];
	A[1][0]=b10*m4.A[0][0]+b11*m4.A[1][0];
	A[1][1]=b10*m4.A[0][1]+b11*m4.A[1][1];
	a20=m1.A[2][0]*m2.A[0][0]+m1.A[2][1]*m2.A[1][0]+m2.A[2][0];
	a21=m1.A[2][0]*m2.A[0][1]+m1.A[2][1]*m2.A[1][1]+m2.A[2][1];
	b20=a20*m3.A[0][0]+a21*m3.A[1][0]+m3.A[2][0];
	b21=a20*m3.A[0][1]+a21*m3.A[1][1]+m3.A[2][1];
	A[2][0]=b20*m4.A[0][0]+b21*m4.A[1][0]+m4.A[2][0];
	A[2][1]=b20*m4.A[0][1]+b21*m4.A[1][1]+m4.A[2][1];
}


//==============================================================================
//=============================== emTranslateATM ===============================
//==============================================================================

emTranslateATM::emTranslateATM(double addX, double addY)
{
	A[0][0]=1.0;
	A[0][1]=0.0;
	A[1][0]=0.0;
	A[1][1]=1.0;
	A[2][0]=addX;
	A[2][1]=addY;
}


emTranslateATM::emTranslateATM(double addX, double addY, const emATMatrix & m)
{
	A[0][0]=m.A[0][0];
	A[0][1]=m.A[0][1];
	A[1][0]=m.A[1][0];
	A[1][1]=m.A[1][1];
	A[2][0]=m.A[2][0]+addX;
	A[2][1]=m.A[2][1]+addY;
}


//==============================================================================
//================================= emScaleATM =================================
//==============================================================================

emScaleATM::emScaleATM(double facX, double facY)
{
	A[0][0]=facX;
	A[0][1]=0.0;
	A[1][0]=0.0;
	A[1][1]=facY;
	A[2][0]=0.0;
	A[2][1]=0.0;
}


emScaleATM::emScaleATM(double facX, double facY, const emATMatrix & m)
{
	A[0][0]=m.A[0][0]*facX;
	A[0][1]=m.A[0][1]*facY;
	A[1][0]=m.A[1][0]*facX;
	A[1][1]=m.A[1][1]*facY;
	A[2][0]=m.A[2][0]*facX;
	A[2][1]=m.A[2][1]*facY;
}


emScaleATM::emScaleATM(double facX, double facY, double fixX, double fixY)
{
	A[0][0]=facX;
	A[0][1]=0.0;
	A[1][0]=0.0;
	A[1][1]=facY;
	A[2][0]=fixX-fixX*facX;
	A[2][1]=fixY-fixY*facY;
}


emScaleATM::emScaleATM(
	double facX, double facY, double fixX, double fixY, const emATMatrix & m
)
{
	A[0][0]=m.A[0][0]*facX;
	A[0][1]=m.A[0][1]*facY;
	A[1][0]=m.A[1][0]*facX;
	A[1][1]=m.A[1][1]*facY;
	A[2][0]=(m.A[2][0]-fixX)*facX+fixX;
	A[2][1]=(m.A[2][1]-fixY)*facY+fixY;
}


//==============================================================================
//================================ emRotateATM =================================
//==============================================================================

emRotateATM::emRotateATM(double angle)
{
	double s,c;

	angle*=M_PI/180.0;
	c=cos(angle);
	s=sin(angle);
	A[0][0]=c;
	A[0][1]=s;
	A[1][0]=-s;
	A[1][1]=c;
	A[2][0]=0.0;
	A[2][1]=0.0;
}


emRotateATM::emRotateATM(double angle, const emATMatrix & m)
{
	double s,c;

	angle*=M_PI/180.0;
	c=cos(angle);
	s=sin(angle);
	A[0][0]=m.A[0][0]*c-m.A[0][1]*s;
	A[0][1]=m.A[0][0]*s+m.A[0][1]*c;
	A[1][0]=m.A[1][0]*c-m.A[1][1]*s;
	A[1][1]=m.A[1][0]*s+m.A[1][1]*c;
	A[2][0]=m.A[2][0]*c-m.A[2][1]*s;
	A[2][1]=m.A[2][0]*s+m.A[2][1]*c;
}


emRotateATM::emRotateATM(double angle, double fixX, double fixY)
{
	double s,c;

	angle*=M_PI/180.0;
	c=cos(angle);
	s=sin(angle);
	A[0][0]=c;
	A[0][1]=s;
	A[1][0]=-s;
	A[1][1]=c;
	A[2][0]=fixX-fixX*c+fixY*s;
	A[2][1]=fixY-fixX*s-fixY*c;
}


emRotateATM::emRotateATM(
	double angle, double fixX, double fixY, const emATMatrix & m
)
{
	double s,c;

	angle*=M_PI/180.0;
	c=cos(angle);
	s=sin(angle);
	A[0][0]=m.A[0][0]*c-m.A[0][1]*s;
	A[0][1]=m.A[0][0]*s+m.A[0][1]*c;
	A[1][0]=m.A[1][0]*c-m.A[1][1]*s;
	A[1][1]=m.A[1][0]*s+m.A[1][1]*c;
	A[2][0]=(m.A[2][0]-fixX)*c-(m.A[2][1]-fixY)*s+fixX;
	A[2][1]=(m.A[2][0]-fixX)*s+(m.A[2][1]-fixY)*c+fixY;
}


//==============================================================================
//================================= emShearATM =================================
//==============================================================================

emShearATM::emShearATM(double shX, double shY)
{
	A[0][0]=1.0;
	A[0][1]=shY;
	A[1][0]=shX;
	A[1][1]=1.0;
	A[2][0]=0.0;
	A[2][1]=0.0;
}


emShearATM::emShearATM(double shX, double shY, const emATMatrix & m)
{
	A[0][0]=m.A[0][1]*shX+m.A[0][0];
	A[0][1]=m.A[0][0]*shY+m.A[0][1];
	A[1][0]=m.A[1][1]*shX+m.A[1][0];
	A[1][1]=m.A[1][0]*shY+m.A[1][1];
	A[2][0]=m.A[2][1]*shX+m.A[2][0];
	A[2][1]=m.A[2][0]*shY+m.A[2][1];
}


emShearATM::emShearATM(double shX, double shY, double fixX, double fixY)
{
	A[0][0]=1.0;
	A[0][1]=shY;
	A[1][0]=shX;
	A[1][1]=1.0;
	A[2][0]=-fixY*shX;
	A[2][1]=-fixX*shY;
}


emShearATM::emShearATM(
	double shX, double shY, double fixX, double fixY, const emATMatrix & m
)
{
	A[0][0]=m.A[0][1]*shX+m.A[0][0];
	A[0][1]=m.A[0][0]*shY+m.A[0][1];
	A[1][0]=m.A[1][1]*shX+m.A[1][0];
	A[1][1]=m.A[1][0]*shY+m.A[1][1];
	A[2][0]=(m.A[2][1]-fixY)*shX+m.A[2][0];
	A[2][1]=(m.A[2][0]-fixX)*shY+m.A[2][1];
}
