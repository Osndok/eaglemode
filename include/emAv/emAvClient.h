//------------------------------------------------------------------------------
// emAvClient.h
//
// Copyright (C) 2008 Oliver Hamann.
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

#ifndef emAvClient_h
#define emAvClient_h

#ifndef emAvServerModel_h
#include <emAv/emAvServerModel.h>
#endif


class emAvClient : public emUncopyable {

public:

	emAvClient(emAvServerModel * serverModel);
	virtual ~emAvClient();

	enum StreamStateType {
		STREAM_CLOSED,
		STREAM_OPENING,
		STREAM_OPENED,
		STREAM_ERRORED
	};

	StreamStateType GetStreamState() const;
	const emString & GetStreamErrorText() const;

	void OpenStream(
		const char * audioDrv, const char * videoDrv, const char * filePath
	);

	void CloseStream();

	void SetProperty(const emString & name, const emString & value);

protected:

	virtual void StreamStateChanged(StreamStateType streamState) = 0;

	virtual void PropertyChanged(const emString & name, const emString & value) = 0;

	virtual void ShowFrame(const emImage & image, double tallness) = 0;

private:

	friend class emAvServerModel;

	struct Property {
		emString Name;
		emString Value;
		bool Sending;
		bool Resend;
	};

	void ResetAll();

	void SetStreamOpened();
	void SetStreamErrored(const emString & errorMessage);

	void SetProperty(
		const emString & name, const emString & value, bool fromServer
	);

	void PropertyOKFromServer(const emString & name);

	static int CmpPropName(
		Property * const * obj, void * key, void * context
	);

	emRef<emAvServerModel> ServerModel;
	emAvServerModel::Instance * Instance;
	StreamStateType StreamState;
	emString StreamErrorText;
	emArray<Property*> Properties;
};

inline emAvClient::StreamStateType emAvClient::GetStreamState() const
{
	return StreamState;
}

inline const emString & emAvClient::GetStreamErrorText() const
{
	return StreamErrorText;
}

inline void emAvClient::SetProperty(
	const emString & name, const emString & value
)
{
	SetProperty(name,value,false);
}


#endif
