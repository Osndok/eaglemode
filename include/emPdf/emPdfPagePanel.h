//------------------------------------------------------------------------------
// emPdfPagePanel.h
//
// Copyright (C) 2011,2016,2023-2024 Oliver Hamann.
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

#ifndef emPdfPagePanel_h
#define emPdfPagePanel_h

#ifndef emDialog_h
#include <emCore/emDialog.h>
#endif

#ifndef emPdfSelection_h
#include <emPdf/emPdfSelection.h>
#endif


class emPdfPagePanel : public emPanel {

public:

	emPdfPagePanel(ParentArg parent, const emString & name,
	               emPdfFileModel * fileModel, int pageIndex,
	               emPdfSelection & selection);

	virtual ~emPdfPagePanel();

	emPdfFileModel * GetFileModel() const;
	int GetPageIndex() const;

protected:

	virtual bool Cycle();
	virtual void Notice(NoticeFlags flags);
	virtual void Input(emInputEvent & event, const emInputState & state,
	                   double mx, double my);
	virtual emCursor GetCursor() const;
	virtual bool IsOpaque() const;
	virtual void Paint(const emPainter & painter, emColor canvasColor) const;

private:

	enum LayerType {
		LT_PREVIEW=0,
		LT_CONTENT=1,
		LT_SELECTION=2
	};

	struct Layer {
		Layer();
		~Layer();

		emImage Img;
		double SrcX,SrcY,SrcW,SrcH;

		emPdfServerModel::JobHandle Job;
		emString JobErrorText;

		emImage JobImg;
		double JobSrcX,JobSrcY,JobSrcW,JobSrcH;

		union {
			emUInt64 JobDelayStartTime;
			emUInt64 JobStartTime;
		};

		bool CoordinatesUpToDate;
		bool ContentUpToDate;
		bool JobDelayStartTimeSet;
		LayerType Type;
	};

	enum IconStateType {
		IS_NONE,
		IS_WAITING,
		IS_RENDERING
	};

	enum RectType {
		RT_NONE,
		RT_TEXT,
		RT_URI,
		RT_REF
	};

	void ResetLayer(Layer & layer, bool clearImage);
	bool UpdateLayer(Layer & layer);
	void PaintLayer(
		const emPainter & painter, const Layer & layer, emColor * canvasColor
	) const;
	void UpdateIconState();
	void UpdateCurrentRect();
	void TriggerCurrectRect();
	void TriggerRef(const emPdfServerModel::RefRect & refRect);
	void TriggerUri(const emPdfServerModel::UriRect & uriRect);
	void OpenCurrentUrl();

	emRef<emPdfServerModel> Server;
	emRef<emPdfFileModel> FileModel;
	int PageIndex;
	emPdfSelection & Selection;
	emPdfSelection::PageSelection PageSelection;
	Layer Layers[3];
	emImage WaitIcon,RenderIcon;
	IconStateType IconState;
	double CurrentMX,CurrentMY;
	RectType CurrentRectType;
	int CurrentRectIndex;
	RectType PressedRectType;
	int PressedRectIndex;
	bool ForceTextCursor;
	emCrossPtr<emDialog> OpenUrlDialog;
	emString CurrentUrl;
};

inline emPdfFileModel * emPdfPagePanel::GetFileModel() const
{
	return FileModel;
}

inline int emPdfPagePanel::GetPageIndex() const
{
	return PageIndex;
}


#endif
