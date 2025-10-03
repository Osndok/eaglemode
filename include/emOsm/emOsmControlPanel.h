//------------------------------------------------------------------------------
// emOsmControlPanel.h
//
// Copyright (C) 2024 Oliver Hamann.
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

#ifndef emOsmControlPanel_h
#define emOsmControlPanel_h


#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emOsmConfig_h
#include <emOsm/emOsmConfig.h>
#endif

#ifndef emOsmFileModel_h
#include <emOsm/emOsmFileModel.h>
#endif


class emOsmControlPanel : public emLinearGroup {

public:

	emOsmControlPanel(
		ParentArg parent, const emString & name,
		emOsmFileModel & fileModel
	);

	~emOsmControlPanel();

protected:

	virtual bool Cycle();

	virtual void AutoExpand();
	virtual void AutoShrink();

private:

	struct TimeEntry {
		int Seconds;
		const char * Text;
	};

	static emInt64 ScalarFieldValueOfMegabytes(int megabytes);
	static int MegabytesOfScalarFieldValue(emInt64 value);
	static void ScalarFieldTextOfMegabytesValue(
		char * buf, int bufSize, emInt64 value,
		emUInt64 markInterval, void * context
	);

	static emInt64 ScalarFieldValueOfSeconds(int seconds);
	static int SecondsOfScalarFieldValue(emInt64 value);
	static void ScalarFieldTextOfSecondsValue(
		char * buf, int bufSize, emInt64 value,
		emUInt64 markInterval, void * context
	);

	void UpdateControls();
	void UpdateFileParamChanged();
	void Apply();

	emRef<emOsmFileModel> FileModel;
	emRef<emOsmConfig> Config;

	bool FileParamChanged;

	emTextField * TfTilesUrl;
	emScalarField * SfMaxZ;
	emButton * BtApply;

	emTextField * TfCacheDirectory;
	emScalarField * SfMaxCacheMegabytes;
	emScalarField * SfMaxCacheAgeDays;

	static const TimeEntry TimeTable[];
	static const int TimeTableSize;
};


#endif
