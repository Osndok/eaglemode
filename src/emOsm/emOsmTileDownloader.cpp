//------------------------------------------------------------------------------
// emOsmTileDownloader.cpp
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

#include <emOsm/emOsmTileDownloader.h>


emOsmTileDownloader::emOsmTileDownloader(emScheduler & scheduler)
	: emEngine(scheduler),
	JobQueue(scheduler),
	ProcStarted(false)
{
	SetEnginePriority(HIGH_PRIORITY);
}


emOsmTileDownloader::~emOsmTileDownloader()
{
	if (Process.IsRunning()) Process.Terminate();
	FailAllRunningJobs("Downloader destructed.");
}


emOsmTileDownloader::DownloadJob::DownloadJob(
	const emString & url, const emString & filePath, double priority
)
	: emJob(priority),
	Url(url),
	FilePath(filePath)
{
}


emOsmTileDownloader::DownloadJob::~DownloadJob()
{
}


void emOsmTileDownloader::EnqueueJob(DownloadJob & job)
{
	JobQueue.EnqueueJob(job);
	WakeUp();
}


void emOsmTileDownloader::AbortJob(DownloadJob & job)
{
	if (job.GetState()!=emJob::ST_RUNNING) {
		JobQueue.AbortJob(job);
	}
}


bool emOsmTileDownloader::Cycle()
{
	emArray<emString> args;
	emJob * job;
	DownloadJob * downloadJob;
	char buf[256];
	int i,maxDownloadsPerRun;

	if (ProcStarted) {
		do {
			try {
				i=Process.TryReadErr(buf,sizeof(buf));
			}
			catch (const emException &) {
				i=0;
			}
			if (i>0 && ProcErr.GetCount()<1000) ProcErr.Add(buf,i);
		} while (i>0);

		if (Process.IsRunning()) return true;

		if (Process.GetExitStatus()==0) {
			while ((job=JobQueue.GetFirstRunningJob())!=NULL) {
				JobQueue.SucceedJob(*job);
			}
		}
		else {
			if (ProcErr.IsEmpty()) ProcErr="Download failed.";
			FailAllRunningJobs(ProcErr);
		}

		ProcErr.Clear();
		ProcStarted=false;
	}

	if (!JobQueue.GetFirstWaitingJob()) return false;

	args.Add("curl");
	args.Add("--silent");
	args.Add("--user-agent");
	args.Add("EagleMode");

	maxDownloadsPerRun=10;
	for (i=0; i<maxDownloadsPerRun; i++) {
		job=JobQueue.StartNextJob();
		if (!job) break;
		downloadJob=(DownloadJob*)job;
		args.Add("--output");
		args.Add(downloadJob->FilePath);
		args.Add(downloadJob->Url);
		emDLog("emOsmTileDownloader: Downloading %s",downloadJob->Url.Get());
	}
	emDLog("emOsmTileDownloader: Downloading %d files with one connection",i);

	try {
		Process.TryStart(
			args,
			emArray<emString>(),
			NULL,
			emProcess::SF_SHARE_STDOUT|
			emProcess::SF_PIPE_STDERR|
			emProcess::SF_NO_WINDOW|
			emProcess::SF_USE_CTRL_BREAK
		);
	}
	catch (const emException & exception) {
		FailAllRunningJobs(exception.GetText());
		return true;
	}

	ProcStarted=true;

	return true;
}


void emOsmTileDownloader::FailAllRunningJobs(emString errorText)
{
	const emJob * job;
	const DownloadJob * downloadJob;

	for (job=JobQueue.GetFirstRunningJob(); job; job=job->GetNext()) {
		downloadJob=(DownloadJob*)job;
		if (emIsExistingPath(downloadJob->FilePath)) {
			try {
				emTryRemoveFile(downloadJob->FilePath);
			}
			catch (const emException &) {
			}
		}
	}

	JobQueue.FailAllRunningJobs(errorText);
}
