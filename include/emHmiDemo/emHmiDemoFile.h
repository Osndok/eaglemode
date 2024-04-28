//------------------------------------------------------------------------------
// emHmiDemoFile.h
//
// Copyright (C) 2012,2014,2024 Oliver Hamann.
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

#ifndef emHmiDemoFile_h
#define emHmiDemoFile_h

#ifndef emFpPlugin_h
#include <emCore/emFpPlugin.h>
#endif

#ifndef emInstallInfo_h
#include <emCore/emInstallInfo.h>
#endif

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif


class emHmiDemoFile : public emBorder {
public:
	emHmiDemoFile(
		ParentArg parent, const emString & name, const emString & path
	);
	emHmiDemoFile(
		ParentArg parent, const emString & name, emInstallDirType idt,
		const char * prj, const char * subPath=NULL,
		const char * subPath2=NULL
	);
	virtual ~emHmiDemoFile();
protected:
	virtual void AutoExpand();
	virtual void LayoutChildren();
private:
	emRef<emFpPluginList> FpPluginList;
	emString Path;
};


#endif
