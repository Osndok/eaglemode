//------------------------------------------------------------------------------
// emPdfPageAreasMap.h
//
// Copyright (C) 2023 Oliver Hamann.
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

#ifndef emPdfPageAreasMap_h
#define emPdfPageAreasMap_h

#ifndef emPdfServerModel_h
#include <emPdf/emPdfServerModel.h>
#endif


class emPdfPageAreasMap : public emEngine {

public:

	emPdfPageAreasMap(emScheduler & scheduler);
	virtual ~emPdfPageAreasMap();

	void Setup(emPdfServerModel & serverModel,
	           emPdfServerModel::PdfHandle pdfHandle);

	void Reset();

	bool RequestPageAreas(int page, double priority);

	const emPdfServerModel::PageAreas * GetPageAreas(int page) const;

	const emString * GetError(int page) const;

	const emSignal & GetPageAreasSignal() const;

protected:

	virtual bool Cycle();

private:

	struct Entry {
		Entry();
		~Entry();
		bool Requested;
		emPdfServerModel::JobHandle JobHandle;
		emPdfServerModel::PageAreas Areas;
		emString ErrorText;
	};

	emPdfServerModel * ServerModel;
	emPdfServerModel::PdfHandle PdfHandle;
	emArray<Entry> Entries;
	emSignal PageAreasSignal;
};


inline const emSignal & emPdfPageAreasMap::GetPageAreasSignal() const
{
	return PageAreasSignal;
}


#endif
