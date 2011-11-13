//------------------------------------------------------------------------------
// emPdfFileModel.h
//
// Copyright (C) 2011 Oliver Hamann.
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

#ifndef emPdfFileModel_h
#define emPdfFileModel_h

#ifndef emFileModel_h
#include <emCore/emFileModel.h>
#endif

#ifndef emPdfServerModel_h
#include <emPdf/emPdfServerModel.h>
#endif


class emPdfFileModel : public emFileModel {

public:

	static emRef<emPdfFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	emPdfServerModel * GetServerModel() const;
	emPdfServerModel::PdfHandle GetPdfHandle() const;
	int GetPageCount() const;
	const emPdfServerModel::PageInfo & GetPageInfo(int page) const;
	double GetPageWidth(int page) const;
	double GetPageHeight(int page) const;
	const emString & GetPageLabel(int page) const;

protected:

	emPdfFileModel(emContext & context, const emString & name);
	virtual ~emPdfFileModel();
	virtual void ResetData();
	virtual void TryStartLoading() throw(emString);
	virtual bool TryContinueLoading() throw(emString);
	virtual void QuitLoading();
	virtual void TryStartSaving() throw(emString);
	virtual bool TryContinueSaving() throw(emString);
	virtual void QuitSaving();
	virtual emUInt64 CalcMemoryNeed();
	virtual double CalcFileProgress();

private:

	emRef<emPdfServerModel> ServerModel;
	emPdfServerModel::JobHandle JobHandle;
	emPdfServerModel::PdfHandle PdfHandle;
	emUInt64 FileSize;
	emUInt64 StartTime;
	int PageCount;
};

inline emPdfServerModel * emPdfFileModel::GetServerModel() const
{
	return ServerModel;
}

inline emPdfServerModel::PdfHandle emPdfFileModel::GetPdfHandle() const
{
	return PdfHandle;
}

inline int emPdfFileModel::GetPageCount() const
{
	return PageCount;
}

inline const emPdfServerModel::PageInfo & emPdfFileModel::GetPageInfo(int page) const
{
	return ServerModel->GetPageInfo(PdfHandle,page);
}

inline double emPdfFileModel::GetPageWidth(int page) const
{
	return ServerModel->GetPageInfo(PdfHandle,page).Width;
}

inline double emPdfFileModel::GetPageHeight(int page) const
{
	return ServerModel->GetPageInfo(PdfHandle,page).Height;
}

inline const emString & emPdfFileModel::GetPageLabel(int page) const
{
	return ServerModel->GetPageInfo(PdfHandle,page).Label;
}


#endif
