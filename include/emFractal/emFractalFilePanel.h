//------------------------------------------------------------------------------
// emFractalFilePanel.h
//
// Copyright (C) 2004-2008,2016-2017 Oliver Hamann.
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

#ifndef emFractalFilePanel_h
#define emFractalFilePanel_h

#ifndef emFilePanel_h
#include <emCore/emFilePanel.h>
#endif

#ifndef emRenderThreadPool_h
#include <emCore/emRenderThreadPool.h>
#endif

#ifndef emFractalFileModel_h
#include <emFractal/emFractalFileModel.h>
#endif


class emFractalFilePanel : public emFilePanel {

public:

	emFractalFilePanel(
		ParentArg parent, const emString & name,
		emFractalFileModel * fileModel
	);

	virtual ~emFractalFilePanel();

	virtual emString GetTitle() const;
	virtual emString GetIconFileName() const;

protected:

	virtual bool Cycle();

	virtual void Notice(NoticeFlags flags);

	virtual bool IsOpaque() const;

	virtual void Paint(const emPainter & painter, emColor canvasColor) const;

private:

	struct CommonRenderVars {
		emFractalFilePanel * Panel;
		emThreadMiniMutex Mutex;
		int InvX1,InvY1,InvX2,InvY2;
	};

	struct ThreadRenderVars {
		int ImgWidth,ImgHeight;
		emByte * ImgMap;
		int InvX1,InvY1,InvX2,InvY2;
	};

	void Prepare();

	static void ThreadRenderFunc(void * data, int index);

	void ThreadRenderRun(CommonRenderVars & crv);

	emColor CalcPixel(double pixX, double pixY) const;

	static void PutPixel(
		ThreadRenderVars & trv, int x, int y, int s,
		emColor color
	);

	static emColor PeekPixel(const ThreadRenderVars & trv, int x, int y);

	emRef<emRenderThreadPool> RenderThreadPool;
	const emFractalFileModel * Mdl;
	emArray<emColor> Colors;
	double ImgX1,ImgY1,ImgX2,ImgY2;
	emImage Image;
	double FrcX,FrcY,FrcSX,FrcSY;
	int PixX,PixY,PixStep;
};


#endif
