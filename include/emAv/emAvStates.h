//------------------------------------------------------------------------------
// emAvStates.h
//
// Copyright (C) 2011 Oliver Hamann.
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

#ifndef emAvStates_h
#define emAvStates_h

#ifndef emConfigModel_h
#include <emCore/emConfigModel.h>
#endif


class emAvStates : public emConfigModel, public emStructRec {

public:

	static emRef<emAvStates> Acquire(emRootContext & rootContext);

	class FileStateRec : public emStructRec {
	public:
		FileStateRec();
		~FileStateRec();
		emStringRec FilePath;
		emIntRec PlayLength;
		emIntRec PlayPos;
		emStringRec AudioChannel;
		emStringRec SpuChannel;
	};

	emIntRec AudioVolume;
	emStringRec AudioVisu;

	emIntRec MaxAudioStates;
	emTArrayRec<FileStateRec> AudioStates;

	emIntRec MaxVideoStates;
	emTArrayRec<FileStateRec> VideoStates;

	virtual const char * GetFormatName() const;

protected:

	emAvStates(emContext & context, const emString & name);
	virtual ~emAvStates();
};


#endif
