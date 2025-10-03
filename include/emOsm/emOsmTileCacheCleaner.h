//------------------------------------------------------------------------------
// emOsmTileCacheCleaner.h
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

#ifndef emOsmTileCacheCleaner_h
#define emOsmTileCacheCleaner_h

#ifndef emAvlTreeMap_h
#include <emCore/emAvlTreeMap.h>
#endif

#ifndef emAvlTreeSet_h
#include <emCore/emAvlTreeSet.h>
#endif

#ifndef emEngine_h
#include <emCore/emEngine.h>
#endif

#ifndef emList_h
#include <emCore/emList.h>
#endif

#ifndef emOsmConfig_h
#include <emOsm/emOsmConfig.h>
#endif


class emOsmTileCacheCleaner : public emEngine {

public:

	emOsmTileCacheCleaner(emRootContext & rootContext);
	virtual ~emOsmTileCacheCleaner();

	void AllowToStart();
	void LockFilePath(const emString & filePath);
	void UnlockFilePath(const emString & filePath);
	void NoticeDownload(emUInt64 size);

protected:

	virtual bool Cycle();

private:

	enum StateEnum {
		ST_DISALLOWED,
		ST_TRAVERSE_AND_DELETE_OUTDATED,
		ST_DELETE_FOR_MAX_CACHE_SIZE,
		ST_PAUSE
	};

	struct CollectedFile {
		emString Path;
		time_t Time;
		emUInt64 Size;

		bool operator < (const CollectedFile & other) const;
		bool operator > (const CollectedFile & other) const;
	};

	void ResetTraversal();
	void ResetCollected();
	void StartToTraverseAndDeleteOutdated();
	void StartToDeleteForMaxCacheSize();
	void StartToPause();
	bool IsTotalSizeWithinLimit(bool forContinuingCleanup=false) const;
	bool IsLockedDirectory(const emString& directory) const;
	bool StepToTraverseAndDeleteOutdated();
	bool StepToDeleteForMaxCacheSize();

	emRef<emOsmConfig> Config;
	emAvlTreeSet<emString> LockedFilePaths;
	StateEnum State;
	emList<emString> DirStack;
	emString CurrentDir;
	bool KeepCurrentDir;
	emDirHandle DirHandle;
	emList<emString> Names;
	emUInt64 TotalSize;
	emAvlTreeSet<CollectedFile> CollectedFiles;
	int CollectedFilesCount;
	emTimer Timer;

	static const int MaxCollectedFiles;
};


#endif
