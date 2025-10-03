//------------------------------------------------------------------------------
// emOsmTileCache.h
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

#ifndef emOsmTileCache_h
#define emOsmTileCache_h

#ifndef emImageFile_h
#include <emCore/emImageFile.h>
#endif

#ifndef emOsmTileCacheCleaner_h
#include <emOsm/emOsmTileCacheCleaner.h>
#endif

#ifndef emOsmTileDownloader_h
#include <emOsm/emOsmTileDownloader.h>
#endif


class emOsmTileCache : public emModel {

public:

	static emRef<emOsmTileCache> Acquire(emRootContext & rootContext);

	class LoadJob : public emJob {
	public:
		LoadJob(
			const emString & tilesUrl, int tileZ, int tileX, int tileY,
			double priority=0.0
		);
		virtual ~LoadJob();
		const emImage & GetImage() const;
	private:
		friend class emOsmTileCache;
		enum LoadStateEnum {
			LS_IDLE,
			LS_MK_CACHE_DIR,
			LS_START_DOWNLOAD,
			LS_DOWNLOADING,
			LS_START_LOAD_FILE,
			LS_LOADING_FILE
		};
		emString TilesUrl;
		int TileZ,TileX,TileY;
		LoadStateEnum LoadState;
		emString TileFilePath;
		emRef<emOsmTileDownloader::DownloadJob> DownloadJob;
		emRef<emImageFileModel> FileModel;
		emOwnPtr<emFileModelClient> FileModelClient;
		emImage Image;
	};

	void EnqueueJob(LoadJob & job);
	void AbortJob(LoadJob & job);

	void AllowBusyCleaner();

protected:

	emOsmTileCache(emContext & context, const emString & name);
	virtual ~emOsmTileCache();

	virtual bool Cycle();

private:

	class MyFileModelClient : public emFileModelClient {
	public:
		MyFileModelClient(LoadJob & job);
		void SetPriorityFromJob();
		virtual emUInt64 GetMemoryLimit() const;
		virtual double GetPriority() const;
		virtual bool IsReloadAnnoying() const;
	private:
		LoadJob & Job;
		double Priority;
	};

	void UpdateLoadJob(LoadJob & job);

	static const char * TryGetTileFileType(const emString & tilesUrl);

	static emString TryGetTileUrl(
		const emString & tilesUrl, int tileZ, int tileX, int tileY
	);

	static emString TryGetTileFilePath(
		const emString & tilesUrl, int tileZ, int tileX, int tileY
	);

	emJobQueue JobQueue;
	emOsmTileDownloader Downloader;
	emOsmTileCacheCleaner Cleaner;
};


inline const emImage & emOsmTileCache::LoadJob::GetImage() const
{
	return Image;
}


#endif
