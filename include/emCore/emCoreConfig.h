//------------------------------------------------------------------------------
// emCoreConfig.h
//
// Copyright (C) 2006-2010,2014,2016 Oliver Hamann.
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

#ifndef emCoreConfig_h
#define emCoreConfig_h

#ifndef emConfigModel_h
#include <emCore/emConfigModel.h>
#endif


//==============================================================================
//================================ emCoreConfig ================================
//==============================================================================

class emCoreConfig : public emConfigModel, public emStructRec {

public:

	static emRef<emCoreConfig> Acquire(emRootContext & rootContext);

	emBoolRec StickMouseWhenNavigating;
	emBoolRec EmulateMiddleButton;
	emBoolRec PanFunction;
	emDoubleRec MouseZoomSpeed;
	emDoubleRec MouseScrollSpeed;
	emDoubleRec MouseWheelZoomSpeed;
	emDoubleRec MouseWheelZoomAcceleration;
	emDoubleRec KeyboardZoomSpeed;
	emDoubleRec KeyboardScrollSpeed;
	emDoubleRec KineticZoomingAndScrolling;
	emDoubleRec MagnetismRadius;
	emDoubleRec MagnetismSpeed;
	emDoubleRec VisitSpeed;
	emIntRec MaxMegabytesPerView;
	emIntRec MaxRenderThreads;

	virtual const char * GetFormatName() const;

protected:

	emCoreConfig(emContext & context, const emString & name);
	virtual ~emCoreConfig();

private:

	void TryLoadOldVersion() throw(emException);
};


#endif
