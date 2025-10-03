//------------------------------------------------------------------------------
// emAvClient.cpp
//
// Copyright (C) 2008,2014,2018-2019,2024 Oliver Hamann.
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

#include <emAv/emAvClient.h>


emAvClient::emAvClient(emAvServerModel * serverModel)
{
	ServerModel=serverModel;
	Instance=NULL;
	StreamState=STREAM_CLOSED;
}


emAvClient::~emAvClient()
{
	ResetAll();
}


void emAvClient::OpenStream(
	const char * audioDrv, const char * videoDrv, const char * filePath
)
{
	emAvServerModel::Instance * inst;

	ResetAll();

	try {
		inst=ServerModel->TryOpenInstance(audioDrv,videoDrv,filePath);
	}
	catch (const emException & exception) {
		StreamErrorText=exception.GetText();
		StreamState=STREAM_ERRORED;
		StreamStateChanged(StreamState);
		return;
	}

	Instance=inst;
	inst->Client=this;
	StreamState=STREAM_OPENING;
	StreamStateChanged(StreamState);
}


void emAvClient::CloseStream()
{
	StreamStateType oldState;

	oldState=StreamState;
	ResetAll();
	if (oldState!=StreamState) StreamStateChanged(StreamState);
}


void emAvClient::ResetAll()
{
	if (Instance) {
		ServerModel->SendCommand(Instance,"close","");
		Instance->Client=NULL;
		Instance=NULL;
	}
	StreamState=STREAM_CLOSED;
	StreamErrorText.Clear();
	Properties.Clear(true);
}


void emAvClient::SetStreamOpened()
{
	if (StreamState==STREAM_OPENING) {
		StreamState=STREAM_OPENED;
		StreamStateChanged(StreamState);
	}
}


void emAvClient::SetStreamErrored(const emString & errorMessage)
{
	ResetAll();
	StreamErrorText=errorMessage;
	StreamState=STREAM_ERRORED;
	StreamStateChanged(StreamState);
}


void emAvClient::SetProperty(
	const emString & name, const emString & value, bool fromServer
)
{
	Property * prop;
	int i;

	if (!Instance) return;

	i=Properties.BinarySearchByKey((void*)name.Get(),CmpPropName,this);
	if (i<0) {
		i=~i;
		prop=new Property;
		prop->Name=name;
		prop->Value=value;
		prop->Sending=false;
		prop->Resend=false;
		Properties.Insert(i,prop);
	}
	else {
		prop=Properties[i];
		if (prop->Value==value) return;
		if (fromServer && prop->Sending) return;
		prop->Value=value;
	}

	if (!fromServer) {
		if (!prop->Sending) {
			ServerModel->SendCommand(
				Instance,
				"set",
				emString::Format("%s:%s",prop->Name.Get(),prop->Value.Get())
			);
			prop->Sending=true;
		}
		else {
			prop->Resend=true;
		}
	}

	PropertyChanged(prop->Name,prop->Value);
}


void emAvClient::PropertyOKFromServer(const emString & name)
{
	Property * prop;
	int i;

	if (!Instance) return;

	i=Properties.BinarySearchByKey((void*)name.Get(),CmpPropName,this);
	if (i<0) return;
	prop=Properties[i];
	if (prop->Resend) {
		ServerModel->SendCommand(
			Instance,
			"set",
			emString::Format("%s:%s",prop->Name.Get(),prop->Value.Get())
		);
		prop->Resend=false;
	}
	else {
		prop->Sending=false;
	}
}


int emAvClient::CmpPropName(const Property * obj, void * key, void * context)
{
	return strcmp(obj->Name.Get(),(const char*)key);
}
