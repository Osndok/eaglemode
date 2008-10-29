//------------------------------------------------------------------------------
// emPsPagePanel.h
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#ifndef emPsPagePanel_h
#define emPsPagePanel_h

#ifndef emPanel_h
#include <emCore/emPanel.h>
#endif

#ifndef emPsRenderer_h
#include <emPs/emPsRenderer.h>
#endif


class emPsPagePanel : public emPanel {

public:

	emPsPagePanel(ParentArg parent, const emString & name,
	              const emPsDocument & document, int pageIndex);

	virtual ~emPsPagePanel();

	const emPsDocument & GetDocument() const;
	int GetPageIndex() const;

	void SetPage(const emPsDocument & document, int pageIndex);

protected:

	virtual void Notice(NoticeFlags flags);
	virtual bool Cycle();
	virtual bool IsOpaque();
	virtual void Paint(const emPainter & painter, emColor canvasColor);

private:

	void UpdateJobAndImage();

	emPsDocument Document;
	int PageIndex;
	emRef<emPsRenderer> Renderer;
	emPsRenderer::JobHandle Job;
	emImage Image;
	emPsRenderer::JobState JobState;
	emString JobErrorText;
	emImage WaitIcon,RenderIcon;
};

inline const emPsDocument & emPsPagePanel::GetDocument() const
{
	return Document;
}

inline int emPsPagePanel::GetPageIndex() const
{
	return PageIndex;
}


#endif
