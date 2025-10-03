//------------------------------------------------------------------------------
// emOsmTileCacheCleaner.cpp
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

#include <emOsm/emOsmTileCacheCleaner.h>


emOsmTileCacheCleaner::emOsmTileCacheCleaner(emRootContext & rootContext)
	: emEngine(rootContext.GetScheduler()),
	Config(emOsmConfig::Acquire(rootContext)),
	State(ST_DISALLOWED),
	KeepCurrentDir(false),
	DirHandle(NULL),
	TotalSize(0),
	CollectedFilesCount(0),
	Timer(rootContext.GetScheduler())
{
	SetEnginePriority(LOW_PRIORITY);
	AddWakeUpSignal(Timer.GetSignal());
}


emOsmTileCacheCleaner::~emOsmTileCacheCleaner()
{
	ResetTraversal();
	ResetCollected();
}


void emOsmTileCacheCleaner::AllowToStart()
{
	if (State==ST_DISALLOWED) {
		StartToTraverseAndDeleteOutdated();
		WakeUp();
	}
}


void emOsmTileCacheCleaner::LockFilePath(const emString & filePath)
{
	LockedFilePaths.Insert(filePath);
}


void emOsmTileCacheCleaner::UnlockFilePath(const emString & filePath)
{
	LockedFilePaths.Remove(filePath);
}


void emOsmTileCacheCleaner::NoticeDownload(emUInt64 size)
{
	if (
		State==ST_DELETE_FOR_MAX_CACHE_SIZE ||
		State==ST_PAUSE
	) {
		TotalSize+=size;
		if (State==ST_PAUSE && !IsTotalSizeWithinLimit()) {
			StartToTraverseAndDeleteOutdated();
			WakeUp();
		}
	}
}


bool emOsmTileCacheCleaner::Cycle()
{
	emUInt64 startTime;

	if (IsSignaled(Timer.GetSignal()) && State==ST_PAUSE) {
		StartToTraverseAndDeleteOutdated();
	}

	startTime=emGetClockMS();
	do {
		switch (State) {
		case ST_TRAVERSE_AND_DELETE_OUTDATED:
			if (StepToTraverseAndDeleteOutdated()) {
				StartToDeleteForMaxCacheSize();
			}
			break;
		case ST_DELETE_FOR_MAX_CACHE_SIZE:
			if (StepToDeleteForMaxCacheSize()) {
				if (IsTotalSizeWithinLimit()) {
					StartToPause();
					return false;
				}
				StartToTraverseAndDeleteOutdated();
			}
			break;
		default:
			return false;
		}
	} while (!IsTimeSliceAtEnd() && emGetClockMS()-startTime<10);

	return true;
}


bool emOsmTileCacheCleaner::CollectedFile::operator < (const CollectedFile & other) const
{
	if (Time<other.Time) return true;
	if (Time>other.Time) return false;
	return Path<other.Path;
}


bool emOsmTileCacheCleaner::CollectedFile::operator > (const CollectedFile & other) const
{
	return other < *this;
}


void emOsmTileCacheCleaner::ResetTraversal()
{
	if (DirHandle) {
		emCloseDir(DirHandle);
		DirHandle=NULL;
	}

	DirStack.Clear();
	CurrentDir.Clear();
	KeepCurrentDir=false;
	Names.Clear();
}


void emOsmTileCacheCleaner::ResetCollected()
{
	CollectedFiles.Clear();
	CollectedFilesCount=0;
}


void emOsmTileCacheCleaner::StartToTraverseAndDeleteOutdated()
{
	emString cacheDir;

	ResetTraversal();
	ResetCollected();
	Timer.Stop(false);

	try {
		cacheDir=emOsmConfig::TryGetCacheDirectory();
	}
	catch (const emException &) {
		StartToPause();
		return;
	}

	DirStack+=cacheDir;
	TotalSize=0;

	State=ST_TRAVERSE_AND_DELETE_OUTDATED;
}


void emOsmTileCacheCleaner::StartToDeleteForMaxCacheSize()
{
	State=ST_DELETE_FOR_MAX_CACHE_SIZE;
}


void emOsmTileCacheCleaner::StartToPause()
{
	ResetTraversal();
	ResetCollected();
	State=ST_PAUSE;
	Timer.Start(60*60*1000);
}


bool emOsmTileCacheCleaner::IsTotalSizeWithinLimit(bool forContinuingCleanup) const
{
	emUInt64 limit;

	limit=((emUInt64)Config->MaxCacheMegabytes.Get())*1000*1000;
	if (forContinuingCleanup) {
		limit=limit*88/100;
	}
	else {
		limit=limit*98/100;
	}
	return TotalSize<=limit;
}


bool emOsmTileCacheCleaner::IsLockedDirectory(const emString& directory) const
{
	emString lockedDirectory;
	const emString * lockedFilePath;
	int oldLen;

	lockedFilePath=LockedFilePaths.GetNearestGreaterOrEqual(directory);
	if (lockedFilePath) {
		lockedDirectory=emGetParentPath(*lockedFilePath);
		while (lockedDirectory.GetLen()>=directory.GetLen()) {
			if (lockedDirectory==directory) return true;
			oldLen=lockedDirectory.GetLen();
			lockedDirectory=emGetParentPath(lockedDirectory);
			if (lockedDirectory.GetLen()>=oldLen) break;
		}
	}
	return false;
}


bool emOsmTileCacheCleaner::StepToTraverseAndDeleteOutdated()
{
	time_t fileTime,minFileTime;
	emString name,path;
	emUInt64 fileSize;
	CollectedFile collectedFile;

	if (CurrentDir.IsEmpty()) {
		if (DirStack.IsEmpty()) return true;
		CurrentDir=*DirStack.GetLast();
		DirStack.RemoveLast();
		KeepCurrentDir=false;
		try {
			DirHandle=emTryOpenDir(CurrentDir);
		}
		catch (const emException &) {
			DirHandle=NULL;
			KeepCurrentDir=true;
		}
		return false;
	}

	if (DirHandle) {
		try {
			name=emTryReadDir(DirHandle);
		}
		catch (const emException &) {
			emCloseDir(DirHandle);
			DirHandle=NULL;
			KeepCurrentDir=true;
			return false;
		}
		if (name.IsEmpty()) {
			emCloseDir(DirHandle);
			DirHandle=NULL;
			return false;
		}
		Names+=name;
		return false;
	}

	if (!Names.IsEmpty()) {
		path=emGetChildPath(CurrentDir,*Names.GetFirst());
		Names.RemoveFirst();

		if (emIsSymLinkPath(path)) {
			KeepCurrentDir=true;
			return false;
		}

		if (emIsDirectory(path)) {
			DirStack+=path;
			KeepCurrentDir=true;
			return false;
		}

		if (!emIsRegularFile(path)) {
			KeepCurrentDir=true;
			return false;
		}

		try {
			fileTime=emTryGetFileTime(path);
			fileSize=emTryGetFileSize(path);
		}
		catch (const emException &) {
			KeepCurrentDir=true;
			return false;
		}

		minFileTime=time(NULL)-Config->MaxCacheAgeDays.Get()*24*60*60;
		if (fileTime<minFileTime && !LockedFilePaths.Contains(path)) {
			try {
				emTryRemoveFile(path);
			}
			catch (const emException &) {
				KeepCurrentDir=true;
			}
			return false;
		}

		KeepCurrentDir=true;
		TotalSize+=fileSize;
		collectedFile.Path=path;
		collectedFile.Time=fileTime;
		collectedFile.Size=fileSize;
		CollectedFiles.Insert(collectedFile);
		CollectedFilesCount++;
		if (CollectedFilesCount>MaxCollectedFiles) {
			CollectedFiles.RemoveLast();
			CollectedFilesCount--;
		}
		return false;
	}

	if (!KeepCurrentDir && !IsLockedDirectory(CurrentDir)) {
		try {
			emTryRemoveDirectory(CurrentDir);
		}
		catch (const emException &) {
		}
	}

	CurrentDir.Clear();
	KeepCurrentDir=false;

	return false;
}


bool emOsmTileCacheCleaner::StepToDeleteForMaxCacheSize()
{
	CollectedFile collectedFile;

	if (IsTotalSizeWithinLimit(true) || CollectedFiles.IsEmpty()) {
		return true;
	}

	collectedFile=*CollectedFiles.GetFirst();
	CollectedFiles.RemoveFirst();

	if (LockedFilePaths.Contains(collectedFile.Path)) {
		return false;
	}

	try {
		emTryRemoveFile(collectedFile.Path);
	}
	catch (const emException &) {
		return false;
	}

	TotalSize-=collectedFile.Size;
	return false;
}


const int emOsmTileCacheCleaner::MaxCollectedFiles=100000;
