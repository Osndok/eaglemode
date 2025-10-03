//------------------------------------------------------------------------------
// emOsmTileDownloader.h
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

#ifndef emOsmTileDownloader_h
#define emOsmTileDownloader_h

#ifndef emEngine_h
#include <emCore/emEngine.h>
#endif

#ifndef emJob_h
#include <emCore/emJob.h>
#endif

#ifndef emProcess_h
#include <emCore/emProcess.h>
#endif


class emOsmTileDownloader : public emEngine {

public:

	emOsmTileDownloader(emScheduler & scheduler);
	virtual ~emOsmTileDownloader();

	class DownloadJob : public emJob {
	public:
		DownloadJob(
			const emString & url, const emString & filePath,
			double priority=0.0
		);
		virtual ~DownloadJob();
	private:
		friend class emOsmTileDownloader;
		emString Url;
		emString FilePath;
	};

	void EnqueueJob(DownloadJob & job);
	void AbortJob(DownloadJob & job);

protected:

	virtual bool Cycle();

private:

	void FailAllRunningJobs(emString errorText);

	emJobQueue JobQueue;
	emProcess Process;
	bool ProcStarted;
	emString ProcErr;
};


#endif
