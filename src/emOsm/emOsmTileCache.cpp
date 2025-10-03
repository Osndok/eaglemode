//------------------------------------------------------------------------------
// emOsmTileCache.cpp
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

#include <emOsm/emOsmTileCache.h>
#include <emJpeg/emJpegImageFileModel.h>
#include <emPng/emPngImageFileModel.h>
#include <emOsm/emOsmConfig.h>


emRef<emOsmTileCache> emOsmTileCache::Acquire(emRootContext & rootContext)
{
	EM_IMPL_ACQUIRE_COMMON(emOsmTileCache,rootContext,"")
}


emOsmTileCache::LoadJob::LoadJob(
	const emString & tilesUrl, int tileZ, int tileX, int tileY,
	double priority
)
	: emJob(priority),
	TilesUrl(tilesUrl),
	TileZ(tileZ),
	TileX(tileX),
	TileY(tileY),
	LoadState(LS_IDLE)
{
}


emOsmTileCache::LoadJob::~LoadJob()
{
}


void emOsmTileCache::EnqueueJob(LoadJob & job)
{
	JobQueue.EnqueueJob(job);
	WakeUp();
}


void emOsmTileCache::AbortJob(LoadJob & job)
{
	job.FileModelClient=NULL;
	if (job.FileModel) {
		RemoveWakeUpSignal(job.FileModel->GetFileStateSignal());
		job.FileModel=NULL;
	}
	if (job.LoadState!=LoadJob::LS_IDLE) {
		Cleaner.UnlockFilePath(job.TileFilePath);
	}
	if (job.DownloadJob) {
		Downloader.AbortJob(*job.DownloadJob);
		job.DownloadJob=NULL;
	}
	if (job.LoadState!=LoadJob::LS_IDLE) {
		job.Image.Clear();
		job.LoadState=LoadJob::LS_IDLE;
	}
	JobQueue.AbortJob(job);
}


void emOsmTileCache::AllowBusyCleaner()
{
	Cleaner.AllowToStart();
}


emOsmTileCache::emOsmTileCache(emContext & context, const emString & name)
	: emModel(context,name),
	JobQueue(GetScheduler()),
	Downloader(GetScheduler()),
	Cleaner(GetRootContext())
{
	SetMinCommonLifetime(10);
}


emOsmTileCache::~emOsmTileCache()
{
	emJob * job;

	for (;;) {
		job=JobQueue.GetFirstWaitingJob();
		if (!job) job=JobQueue.GetFirstRunningJob();
		if (!job) break;
		AbortJob(*(LoadJob*)job);
	}
}


bool emOsmTileCache::Cycle()
{
	emJob * job;
	LoadJob * loadJob;

	while (JobQueue.StartNextJob()) {}

	for (job=JobQueue.GetFirstRunningJob(); job;) {
		loadJob=(LoadJob*)job;
		job=job->GetNext();
		UpdateLoadJob(*loadJob);
		if (IsTimeSliceAtEnd()) break;
	}

	return !JobQueue.IsEmpty();
}


emOsmTileCache::MyFileModelClient::MyFileModelClient(LoadJob & job)
 :
	emFileModelClient(job.FileModel),
	Job(job),
	Priority(job.GetPriority())
{
}


void emOsmTileCache::MyFileModelClient::SetPriorityFromJob()
{
	if (Priority != Job.GetPriority()) {
		Priority = Job.GetPriority();
		InvalidatePriority();
	}
}


emUInt64 emOsmTileCache::MyFileModelClient::GetMemoryLimit() const
{
	return 16777216;
}


double emOsmTileCache::MyFileModelClient::GetPriority() const
{
	return Priority;
}


bool emOsmTileCache::MyFileModelClient::IsReloadAnnoying() const
{
	return true;
}


void emOsmTileCache::UpdateLoadJob(LoadJob & job)
{
	emString url;
	const char * fileType;
	emUInt64 fileSize;

	for (;;) {
		switch (job.LoadState) {
		case LoadJob::LS_IDLE:
			try {
				job.TileFilePath=TryGetTileFilePath(
					job.TilesUrl,job.TileZ,job.TileX,job.TileY
				);
			}
			catch (const emException & exception) {
				JobQueue.FailJob(job,exception.GetText());
				return;
			}
			Cleaner.LockFilePath(job.TileFilePath);
			if (emIsExistingPath(job.TileFilePath)) {
				job.LoadState=LoadJob::LS_START_LOAD_FILE;
				break;
			}
			job.LoadState=LoadJob::LS_MK_CACHE_DIR;
			break;
		case LoadJob::LS_MK_CACHE_DIR:
			try {
				emTryMakeDirectories(emGetParentPath(job.TileFilePath));
			}
			catch (const emException & exception) {
				Cleaner.UnlockFilePath(job.TileFilePath);
				JobQueue.FailJob(job,exception.GetText());
				return;
			}
			job.LoadState=LoadJob::LS_START_DOWNLOAD;
			break;
		case LoadJob::LS_START_DOWNLOAD:
			try {
				url=TryGetTileUrl(job.TilesUrl,job.TileZ,job.TileX,job.TileY);
			}
			catch (const emException & exception) {
				Cleaner.UnlockFilePath(job.TileFilePath);
				JobQueue.FailJob(job,exception.GetText());
				return;
			}
			job.DownloadJob=new emOsmTileDownloader::DownloadJob(
				url,job.TileFilePath,job.GetPriority()
			);
			Downloader.EnqueueJob(*job.DownloadJob);
			job.LoadState=LoadJob::LS_DOWNLOADING;
			break;
		case LoadJob::LS_DOWNLOADING:
			switch (job.DownloadJob->GetState()) {
			case emJob::ST_WAITING:
			case emJob::ST_RUNNING:
				return;
			case emJob::ST_ERROR:
				Cleaner.UnlockFilePath(job.TileFilePath);
				JobQueue.FailJob(job,job.DownloadJob->GetErrorText());
				return;
			case emJob::ST_SUCCESS:
				break;
			default:
				Cleaner.UnlockFilePath(job.TileFilePath);
				JobQueue.FailJob(job,"Aborted");
				return;
			}
			try {
				fileSize=emTryGetFileSize(job.TileFilePath);
			}
			catch (const emException & exception) {
				Cleaner.UnlockFilePath(job.TileFilePath);
				JobQueue.FailJob(job,exception.GetText());
				return;
			}
			Cleaner.NoticeDownload(fileSize);
			job.DownloadJob=NULL;
			job.LoadState=LoadJob::LS_START_LOAD_FILE;
			break;
		case LoadJob::LS_START_LOAD_FILE:
			try {
				fileType=TryGetTileFileType(job.TilesUrl);
			}
			catch (const emException & exception) {
				Cleaner.UnlockFilePath(job.TileFilePath);
				JobQueue.FailJob(job,exception.GetText());
				return;
			}
			if (strcasecmp(fileType,"png")==0) {
				job.FileModel=emPngImageFileModel::Acquire(
					GetRootContext(),job.TileFilePath
				);
			}
			else if (strcasecmp(fileType,"jpg")==0 || strcasecmp(fileType,"jpeg")==0) {
				job.FileModel=emJpegImageFileModel::Acquire(
					GetRootContext(),job.TileFilePath
				);
			}
			else {
				Cleaner.UnlockFilePath(job.TileFilePath);
				JobQueue.FailJob(job,"Unsupported tile file type: " + emString(fileType));
				return;
			}
			AddWakeUpSignal(job.FileModel->GetFileStateSignal());
			job.FileModelClient=new MyFileModelClient(job);
			job.LoadState=LoadJob::LS_LOADING_FILE;
			break;
		case LoadJob::LS_LOADING_FILE:
			switch (job.FileModel->GetFileState()) {
			case emFileModel::FS_WAITING:
			case emFileModel::FS_LOADING:
			case emFileModel::FS_TOO_COSTLY:
				((MyFileModelClient*)job.FileModelClient.Get())->SetPriorityFromJob();
				break;
			case emFileModel::FS_LOADED:
			case emFileModel::FS_UNSAVED:
			case emFileModel::FS_SAVING:
				job.Image=job.FileModel->GetImage();
				job.FileModelClient=NULL;
				RemoveWakeUpSignal(job.FileModel->GetFileStateSignal());
				job.FileModel=NULL;
				job.LoadState=LoadJob::LS_IDLE;
				Cleaner.UnlockFilePath(job.TileFilePath);
				JobQueue.SucceedJob(job);
				break;
			default:
				job.FileModelClient=NULL;
				RemoveWakeUpSignal(job.FileModel->GetFileStateSignal());
				job.FileModel=NULL;
				Cleaner.UnlockFilePath(job.TileFilePath);
				JobQueue.FailJob(job,"Failed to load cached tile.");
				break;
			}
			return;
		}
		if (IsTimeSliceAtEnd()) break;
	}
}


const char * emOsmTileCache::TryGetTileFileType(const emString & tilesUrl)
{
	const char * p1, * p2;

	if (tilesUrl.IsEmpty()) {
		throw emException("No tiles URL configured");
	}

	p1=tilesUrl.Get();
	p2=p1+strlen(p1);
	while (p2>p1) {
		p2--;
		if (*p2=='.') return p2+1;
		if (*p2=='/') break;
	}

	throw emException("Tiles URL has no file type at the end.");
}


emString emOsmTileCache::TryGetTileUrl(
	const emString & tilesUrl, int tileZ, int tileX, int tileY
)
{
	const char * p;
	emString url;
	char number[64];

	url=tilesUrl;
	if (url.IsEmpty()) {
		throw emException("No tiles URL configured");
	}

	sprintf(number,"%d",tileZ);
	p=strstr(url.Get(),"{z}");
	if (!p) goto L_PlaceHolderMissing;
	url.Replace(p-url.Get(),3,number);

	sprintf(number,"%d",tileX);
	p=strstr(url.Get(),"{x}");
	if (!p) goto L_PlaceHolderMissing;
	url.Replace(p-url.Get(),3,number);

	sprintf(number,"%d",tileY);
	p=strstr(url.Get(),"{y}");
	if (!p) goto L_PlaceHolderMissing;
	url.Replace(p-url.Get(),3,number);

	if (
		strstr(url.Get(),"{z}") || strstr(url.Get(),"{x}") ||
		strstr(url.Get(),"{y}")
	) {
		throw emException("Tiles URL has duplicated place holders.");
	}

	return url;

L_PlaceHolderMissing:
	throw emException(
		"The tiles URL must contain place holders {z}, {x}, and {y} for the tile coordinates."
	);
}


emString emOsmTileCache::TryGetTileFilePath(
	const emString & tilesUrl, int tileZ, int tileX, int tileY
)
{
	emString path;
	emString tilesUrlHash;
	const char * fileType;

	fileType=TryGetTileFileType(tilesUrl);
	tilesUrlHash=emCalcHashName(tilesUrl.Get(),tilesUrl.GetCount(),24);

	path=emGetChildPath(emOsmConfig::TryGetCacheDirectory(),tilesUrlHash);
	path=emGetChildPath(path,emString::Format("%d",tileZ));
	path=emGetChildPath(path,emString::Format("%d",tileX));
	path=emGetChildPath(path,emString::Format("%d.%s",tileY,fileType));
	return path;
}
