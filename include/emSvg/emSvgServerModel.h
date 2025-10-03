//------------------------------------------------------------------------------
// emSvgServerModel.h
//
// Copyright (C) 2010,2014,2017-2018,2022,2024 Oliver Hamann.
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

#ifndef emSvgServerModel_h
#define emSvgServerModel_h

#ifndef emImage_h
#include <emCore/emImage.h>
#endif

#ifndef emJob_h
#include <emCore/emJob.h>
#endif

#ifndef emModel_h
#include <emCore/emModel.h>
#endif

#ifndef emProcess_h
#include <emCore/emProcess.h>
#endif


class emSvgServerModel : public emModel {

public:

	static emRef<emSvgServerModel> Acquire(emRootContext & rootContext);

	class SvgInstance : public emRefTarget, public emUncopyable {
	public:
		virtual ~SvgInstance();
		double GetWidth() const;
		double GetHeight() const;
		const emString & GetTitle() const;
		const emString & GetDescription() const;
	private:
		friend class emSvgServerModel;
		SvgInstance(emSvgServerModel & svgServerModel);
		emCrossPtr<emSvgServerModel> SvgServerModel;
		emUInt64 ProcRunId;
		int InstanceId;
		double Width;
		double Height;
		emString Title;
		emString Description;
	};

	class OpenJob : public emJob {
	public:
		OpenJob(const emString & filePath, double priority=0.0);
		const emRef<SvgInstance>& GetSvgInstance() const;
	private:
		friend class emSvgServerModel;
		emString FilePath;
		emRef<SvgInstance> SvgInst;
	};

	class RenderJob : public emJob {
	public:
		RenderJob(
			SvgInstance & svgInstance, double srcX, double srcY,
			double srcWidth, double srcHeight, emColor bgColor,
			int tgtWidth, int tgtHeight, double priority=0.0
		);
		double GetSrcX() const;
		double GetSrcY() const;
		double GetSrcWidth() const;
		double GetSrcHeight() const;
		const emImage & GetImage() const;
	private:
		friend class emSvgServerModel;
		emRef<SvgInstance> SvgInst;
		double SrcX, SrcY, SrcWidth, SrcHeight;
		emColor BgColor;
		int TgtW, TgtH;
		int ShmOffset;
		emImage Image;
	};

	void EnqueueJob(emJob & job);
	void AbortJob(emJob & job);

	void Poll(unsigned maxMillisecs);

protected:

	emSvgServerModel(emContext & context, const emString & name);
	virtual ~emSvgServerModel();

	virtual bool Cycle();

private:

	friend class SvgInstance;

	class CloseJob : public emJob {
	public:
		CloseJob(emUInt64 procRunId, int instanceId);
	private:
		friend class emSvgServerModel;
		emUInt64 ProcRunId;
		int InstanceId;
	};

	void TryStartJobs();
	void TryStartOpenJob(OpenJob & openJob);
	bool TryStartRenderJob(RenderJob & renderJob);
	void TryStartCloseJob(CloseJob & closeJob);

	void TryFinishJobs();
	void TryFinishOpenJob(OpenJob & openJob, const char * args);
	void TryFinishRenderJob(RenderJob & renderJob);

	void TryWriteAttachShm();

	void WriteLineToProc(const char * str);
	emString ReadLineFromProc();
	bool TryProcIO();

	void TryAllocShm(int size);
	void FreeShm();

	emProcess Process;
	emUInt64 ProcRunId;
	emUInt64 ProcSvgInstCount;
	emUInt64 ProcIdleClock;
	bool ProcTerminating;
	emArray<char> ReadBuf;
	emArray<char> WriteBuf;

	emJobQueue JobQueue;

	int ShmSize;

#if defined(_WIN32) || defined(__CYGWIN__)
	char ShmId[256];
	void * ShmHdl;
#else
	int ShmId;
#endif

	emByte * ShmPtr;
	int ShmAllocBegin;
	int ShmAllocEnd;

	static const int MinShmSize;
};


inline double emSvgServerModel::SvgInstance::GetWidth() const
{
	return Width;
}

inline double emSvgServerModel::SvgInstance::GetHeight() const
{
	return Height;
}

inline const emString & emSvgServerModel::SvgInstance::GetTitle() const
{
	return Title;
}

inline const emString & emSvgServerModel::SvgInstance::GetDescription() const
{
	return Description;
}

inline const emRef<emSvgServerModel::SvgInstance>&
	emSvgServerModel::OpenJob::GetSvgInstance() const
{
	return SvgInst;
}

inline double emSvgServerModel::RenderJob::GetSrcX() const
{
	return SrcX;
}

inline double emSvgServerModel::RenderJob::GetSrcY() const
{
	return SrcY;
}

inline double emSvgServerModel::RenderJob::GetSrcWidth() const
{
	return SrcWidth;
}

inline double emSvgServerModel::RenderJob::GetSrcHeight() const
{
	return SrcHeight;
}

inline const emImage & emSvgServerModel::RenderJob::GetImage() const
{
	return Image;
}


#endif
