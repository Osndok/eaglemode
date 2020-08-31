//------------------------------------------------------------------------------
// emAvImageConverter.h
//
// Copyright (C) 2019-2020 Oliver Hamann.
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

#ifndef emAvImageConverter_h
#define emAvImageConverter_h

#ifndef emImage_h
#include <emCore/emImage.h>
#endif

#ifndef emRenderThreadPool_h
#include <emCore/emRenderThreadPool.h>
#endif

class emCoreConfig;


class emAvImageConverter {

public:

	emAvImageConverter(emContext & context);
	~emAvImageConverter();

	void SetSourceRGB(
		int width, int height, int bytesPerLine, const emByte * plane
	);

	void SetSourceI420(
		int width, int height, int bytesPerLineY, int bytesPerLineUV,
		const emByte * planeY, const emByte * planeU, const emByte * planeV
	);

	void SetSourceYUY2(
		int width, int height, int bytesPerLine, const emByte * plane
	);

	void SetTarget(emImage * image);

	void Convert(emRenderThreadPool * renderThreadPool);

private:

	static void ThreadFunc(void * data, int index);
	void ThreadRun();

	void ConvertRGB(int y1, int y2);
	void ConvertI420(int y1, int y2);
	void ConvertYUY2(int y1, int y2);

#	if EM_HAVE_X86_INTRINSICS
		void ConvertI420_AVX2(int y1, int y2);
#	endif

	emRef<emCoreConfig> CoreConfig;
	int Format;
	int Width;
	int Height;
	int BPL;
	int BPL2;
	const emByte * Plane;
	const emByte * Plane2;
	const emByte * Plane3;
	emImage * Image;
	emThreadMiniMutex Mutex;
	int RowsAtOnce;
	int PosY;
#	if EM_HAVE_X86_INTRINSICS
		bool CanCpuDoAvx2;
#	endif
};


#endif
