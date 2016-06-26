//------------------------------------------------------------------------------
// emWindowStateSaver.h
//
// Copyright (C) 2016 Oliver Hamann.
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

#ifndef emWindowStateSaver_h
#define emWindowStateSaver_h

#ifndef emWindow_h
#include <emCore/emWindow.h>
#endif


//==============================================================================
//============================= emWindowStateSaver =============================
//==============================================================================

class emWindowStateSaver : public emEngine {

public:

	// Class for making the geometry of a window (class) persistent. This
	// saves the position, size and maximization state in a file. For using
	// this class, simply create an instance of it as a member of your
	// window. The state is restored up-on construction (if file valid), and
	// it is saved on each geometry change or focusing of the window.
	// Actually the saving happens immediately to a shared internal model
	// and with delay to the file.

	emWindowStateSaver(
		emWindow & window, const emString & filePath,
		bool allowRestoreFullscreen=false
	);
		// Constructor.
		// Arguments:
		//   window                 - The window whose state is to be
		//                            restored and saved by this window
		//                            state saver.
		//   filePath               - Path and name for the file. This
		//                            should be unique for the window
		//                            class. The file has an emRec
		//                            format.
		//   allowRestoreFullscreen - Whether to allow restoring
		//                            fullscreen mode.

	virtual ~emWindowStateSaver();
		// Destructor.

protected:

	virtual bool Cycle();

private:

	void Save();
	void Restore();

	class ModelClass : public emConfigModel, public emStructRec {
	public:

		static emRef<ModelClass> Acquire(
			emRootContext & rootContext,
			const emString & filePath
		);

		emDoubleRec ViewX;
		emDoubleRec ViewY;
		emDoubleRec ViewWidth;
		emDoubleRec ViewHeight;
		emBoolRec Maximized;
		emBoolRec Fullscreen;

		virtual const char * GetFormatName() const;

	protected:

		ModelClass(emContext & context, const emString & filePath);
		virtual ~ModelClass();
	};

	emWindow & Window;
	bool AllowRestoreFullscreen;
	emRef<ModelClass> Model;
	double OwnNormalX,OwnNormalY,OwnNormalW,OwnNormalH;
};


#endif
