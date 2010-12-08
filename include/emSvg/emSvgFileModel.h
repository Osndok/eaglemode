//------------------------------------------------------------------------------
// emSvgFileModel.h
//
// Copyright (C) 2010 Oliver Hamann.
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

#ifndef emSvgFileModel_h
#define emSvgFileModel_h

#ifndef emFileModel_h
#include <emCore/emFileModel.h>
#endif

#ifndef emSvgServerModel_h
#include <emSvg/emSvgServerModel.h>
#endif


class emSvgFileModel : public emFileModel {

public:

	static emRef<emSvgFileModel> Acquire(
		emContext & context, const emString & name, bool common=true
	);

	double GetWidth() const;
	double GetHeight() const;
	const emString & GetTitle() const;
	const emString & GetDescription() const;

	emSvgServerModel * GetServerModel() const;
	emSvgServerModel::SvgHandle GetSvgHandle() const;

protected:

	emSvgFileModel(emContext & context, const emString & name);
	virtual ~emSvgFileModel();
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

	emRef<emSvgServerModel> ServerModel;
	emSvgServerModel::JobHandle JobHandle;
	emSvgServerModel::SvgHandle SvgHandle;
	emUInt64 FileSize;
	double Width;
	double Height;
	emString Title;
	emString Description;
};

inline double emSvgFileModel::GetWidth() const
{
	return Width;
}

inline double emSvgFileModel::GetHeight() const
{
	return Height;
}

inline const emString & emSvgFileModel::GetTitle() const
{
	return Title;
}

inline const emString & emSvgFileModel::GetDescription() const
{
	return Description;
}

inline emSvgServerModel * emSvgFileModel::GetServerModel() const
{
	return ServerModel;
}

inline emSvgServerModel::SvgHandle emSvgFileModel::GetSvgHandle() const
{
	return SvgHandle;
}


#endif
