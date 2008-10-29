//------------------------------------------------------------------------------
// emAvFileControlPanel.h
//
// Copyright (C) 2008 Oliver Hamann.
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

#ifndef emAvFileControlPanel_h
#define emAvFileControlPanel_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emAvFileModel_h
#include <emAv/emAvFileModel.h>
#endif


class emAvFileControlPanel : public emTkGroup {

public:

	emAvFileControlPanel(ParentArg parent, const emString & name,
	                     emAvFileModel * fileModel);

	virtual ~emAvFileControlPanel();

protected:

	virtual bool Cycle();

private:

	void UpdateControls();

	static void TextOfPlayPos(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);
	static void TextOfAudioVolume(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);
	static void TextOfAudioVisu(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);
	static void TextOfAudioChannel(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);
	static void TextOfSpuChannel(
		char * buf, int bufSize, emInt64 value, emUInt64 markInterval,
		void * context
	);

	emRef<emAvFileModel> Mdl;

	emTkTextField * TfInfo;
	emTkTextField * TfWarning;

	emTkScalarField * SfPlayPos;

	emTkRadioButton::Group * RgPlayState;
	emTkRadioButton * RbStop;
	emTkRadioButton * RbPause;
	emTkRadioButton * RbSlow;
	emTkRadioButton * RbPlay;
	emTkRadioButton * RbFast;

	emTkCheckButton * CbAudioMute;
	emTkScalarField * SfAudioVolume;
	emTkScalarField * SfAudioVisu;
	emTkScalarField * SfAudioChannel;
	emTkScalarField * SfSpuChannel;
};


#endif
