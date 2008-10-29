//------------------------------------------------------------------------------
// emMiniIpc.h - Minimalistic support for interprocess communication
//
// Copyright (C) 2004-2008 Oliver Hamann.
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

#ifndef emMiniIpc_h
#define emMiniIpc_h

#ifndef emTimer_h
#include <emCore/emTimer.h>
#endif


//==============================================================================
//============================== emMiniIpcClient ===============================
//==============================================================================

class emMiniIpcClient : public emUnconstructable {

public:

	// Client side of the minimalistic interprocess communication.

	static void TrySend(const char * serverName, int argc,
	                    const char * const argv[]) throw(emString);
		// Send a message to a server which runs on the same host by the
		// same user. The server must not be run by the same process and
		// thread, otherwise a deadlock could happen. This is
		// unidirectional and asynchronous and it does not wait for the
		// server to handle the message. For bidirectional
		// communication, you could have servers on both ends and invent
		// a suitable protocol (e.g. send the own server name with every
		// message).
		// Arguments:
		//   serverName - Name of the server.
		//   argc       - Number of arguments in the message.
		//   argv       - The arguments of the message.
		// Throws: An error message if the server cannot be reached.
};


//==============================================================================
//============================== emMiniIpcServer ===============================
//==============================================================================

class emMiniIpcServer : public emUncopyable {

public:

	// Server side of the minimalistic interprocess communication.

	emMiniIpcServer(emScheduler & scheduler);
		// Construct a server which is not yet serving.
		// Arguments:
		//   scheduler - The scheduler serving this object (or its
		//               internal stuff).

	virtual ~emMiniIpcServer();
		// Destructor: stops any serving.

	const emScheduler & GetScheduler() const;
	emScheduler & GetScheduler();
		// Get the scheduler.

	void StartServing(const char * userDefinedServerName=NULL);
		// Start serving. Serving means to listen to client messages.
		// The server gets a server name for identification within the
		// same host and user. If the argument userDefinedServerName is
		// not NULL, it is taken as the server name. Otherwise a new
		// generic server name is invented automatically, for which no
		// server already exists (recommended where possible). If there
		// is already another server with the same user-defined name
		// within the same host and user, this server will simply not
		// get any messages before the other one stops serving.

	void StopServing();
		// Stop serving.

	bool IsServing() const;
		// Whether this server is serving.

	const emString & GetServerName() const;
		// Get the server name (empty string if not serving).

protected:

	virtual void OnReception(int argc, const char * const argv[]) = 0;
		// Called on reception of a message from a client.
		// Arguments:
		//   argc - Number of arguments in the message.
		//   argv - The arguments of the message.

private:

	void Poll();

	class SEClass : public emEngine {
	public:
		SEClass(emMiniIpcServer & server);
		virtual ~SEClass();
	protected:
		virtual bool Cycle();
	private:
		emMiniIpcServer & Server;
		emTimer Timer;
	};

	friend class SEClass;

	emScheduler & Scheduler;
	emString ServerName;
	void * Instance;
	emArray<char> Buffer;
	SEClass * ServerEngine;
	bool * PtrStoppedOrDestructed;
};

inline const emScheduler & emMiniIpcServer::GetScheduler() const
{
	return Scheduler;
}

inline emScheduler & emMiniIpcServer::GetScheduler()
{
	return Scheduler;
}

inline bool emMiniIpcServer::IsServing() const
{
	return ServerEngine!=NULL;
}

inline const emString & emMiniIpcServer::GetServerName() const
{
	return ServerName;
}


#endif
