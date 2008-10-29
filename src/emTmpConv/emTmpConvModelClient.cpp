//------------------------------------------------------------------------------
// emTmpConvModelClient.cpp
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

#include <emTmpConv/emTmpConvModelClient.h>


emTmpConvModelClient::emTmpConvModelClient(emTmpConvModel * model)
{
	ConversionWanted=false;
	Priority=0.0;
	ThisPtrInList=NULL;
	NextInList=NULL;
	if (model) SetModel(model);
}


emTmpConvModelClient::~emTmpConvModelClient()
{
	SetModel(NULL);
}


void emTmpConvModelClient::SetModel(emTmpConvModel * model)
{
	if (Model==model) return;
	if (Model) {
		*ThisPtrInList=NextInList;
		if (NextInList) NextInList->ThisPtrInList=ThisPtrInList;
		ThisPtrInList=NULL;
		NextInList=NULL;
		Model->ClientsChanged();
		Model=NULL;
	}
	if (model) {
		Model=model;
		NextInList=Model->ClientList;
		if (NextInList) NextInList->ThisPtrInList=&NextInList;
		Model->ClientList=this;
		ThisPtrInList=&Model->ClientList;
		Model->ClientsChanged();
	}
}


void emTmpConvModelClient::SetConversionWanted(bool conversionWanted)
{
	if (ConversionWanted!=conversionWanted) {
		ConversionWanted=conversionWanted;
		if (Model) Model->ClientsChanged();
	}
}


void emTmpConvModelClient::SetPriority(double priority)
{
	if (Priority!=priority) {
		Priority=priority;
		if (Model) Model->ClientsChanged();
	}
}
