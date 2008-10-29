//------------------------------------------------------------------------------
// emTmpConvModelClient.h
//
// Copyright (C) 2007-2008 Oliver Hamann.
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

#ifndef emTmpConvModelClient_h
#define emTmpConvModelClient_h

#ifndef emTmpConvModel_h
#include <emTmpConv/emTmpConvModel.h>
#endif


class emTmpConvModelClient : public emUncopyable {

public:

	emTmpConvModelClient(emTmpConvModel * model=NULL);
	virtual ~emTmpConvModelClient();

	void SetModel(emTmpConvModel * model=NULL);
	const emTmpConvModel * GetModel() const;
	emTmpConvModel * GetModel();

	void SetConversionWanted(bool conversionWanted);
	bool IsConversionWanted() const;

	double GetPriority() const;
	void SetPriority(double priority);

private: friend class emTmpConvModel;

	emRef<emTmpConvModel> Model;
	bool ConversionWanted;
	double Priority;
	emTmpConvModelClient * * ThisPtrInList;
	emTmpConvModelClient * NextInList;
};

inline const emTmpConvModel * emTmpConvModelClient::GetModel() const
{
	return Model;
}

inline emTmpConvModel * emTmpConvModelClient::GetModel()
{
	return Model;
}

inline bool emTmpConvModelClient::IsConversionWanted() const
{
	return ConversionWanted;
}

inline double emTmpConvModelClient::GetPriority() const
{
	return Priority;
}


#endif
