//------------------------------------------------------------------------------
// emAvLibDirCfg.h
//
// Copyright (C) 2020 Oliver Hamann.
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

#ifndef emAvLibDirCfg_h
#define emAvLibDirCfg_h

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif


class emAvLibDirCfg {

public:

	emAvLibDirCfg(const emString & serverProcPath);
	~emAvLibDirCfg();

	const emSignal & GetChangeSignal() const;

	bool IsLibDirNecessary() const;

	bool IsLibDirValid() const;

	const emString & GetLibDirError() const;

	const emString & GetLibDir() const;

	void SetSaveAndSignalLibDir(const emString & libDir, emScheduler & scheduler);

	emArray<emString> GetExtraEnv() const;

	emPanel * CreateFilePanelElement(emPanel * parent, const emString & name);

	emPanel * CreateControlPanelElement(emPanel * parent, const emString & name);

private:

	void LoadConfigFile();
	void SaveConfigFile() const;

	static bool CheckLibDir(const char * libDir, emString * pErr=NULL);
	static bool CheckProcName(const emString & serverProcPath, const char * name);
	static bool IsMatchingBinary(const char * filePath, emString * pErr=NULL);
	static int TryGetWinBinArch(const char * filePath);
	static bool GetVlcInfoFromRegistry(emString * pDir, emString * pVersion=NULL);

	class CfgPanel : public emLinearGroup {
	public:
		CfgPanel(emPanel * parent, const emString & name, emAvLibDirCfg & cfg);
		virtual ~CfgPanel();
	protected:
		virtual bool Cycle();
		virtual void AutoExpand();
		virtual void AutoShrink();
	private:
		void UpdateFromCfg();
		void UpdateStatusLabel(bool autoDetectFailed=false);
		emAvLibDirCfg & Cfg;
		emLabel * TfDesc;
		emFileSelectionBox * FsBox;
		emButton * BtAutoDetect;
		emTextField * TfStatus;
		emButton * BtSave;
		bool LibDirValid;
		emString LibDirError;
		emString LibDir;
	};

	static const int RequiredVlcArch;
	static const char * RequiredVlcArchString;
	static const int RequiredVlcVersion;
	static const int MinRequiredVlcSubVersion;
	static const int MaxRequiredVlcSubVersion;
	static const char * RequiredVlcVersionString;

	emSignal ChangeSignal;
	bool LibDirNecessary;
	bool LibDirValid;
	emString LibDirError;
	emString LibDir;
};

inline const emSignal & emAvLibDirCfg::GetChangeSignal() const
{
	return ChangeSignal;
}

inline bool emAvLibDirCfg::IsLibDirNecessary() const
{
	return LibDirNecessary;
}

inline bool emAvLibDirCfg::IsLibDirValid() const
{
	return LibDirValid;
}

inline const emString & emAvLibDirCfg::GetLibDirError() const
{
	return LibDirError;
}

inline const emString & emAvLibDirCfg::GetLibDir() const
{
	return LibDir;
}


#endif
