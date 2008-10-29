//------------------------------------------------------------------------------
// emAvServerModel.h
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

#ifndef emAvServerModel_h
#define emAvServerModel_h

#ifndef emImage_h
#include <emCore/emImage.h>
#endif

#ifndef emTimer_h
#include <emCore/emTimer.h>
#endif

#ifndef emProcess_h
#include <emCore/emProcess.h>
#endif

#ifndef emModel_h
#include <emCore/emModel.h>
#endif

class emAvClient;


class emAvServerModel : public emModel {

public:

	static emRef<emAvServerModel> Acquire(
		emRootContext & rootContext, const emString & serverProcPath
	);

protected:

	emAvServerModel(emContext & context, const emString & serverProcPath);
	virtual ~emAvServerModel();

	virtual bool Cycle();

private:

	friend class emAvClient;

	enum {
		MAX_INSTANCES           =512,
		MAX_IN_BUF_SIZE         =65536,
		MAX_OUT_BUF_SIZE        =1048576,
		PROC_HOLD_MILLISECS     =5000,
		NORMAL_PROC_TERM_TIMEOUT=3000,
		DTOR_PROC_TERM_TIMEOUT  =1000,
		MIN_LIFETIME_MILLISECS  =PROC_HOLD_MILLISECS+NORMAL_PROC_TERM_TIMEOUT+5000
	};

	enum ShmAttachStateType {
		SA_DETACHED,
		SA_ATTACHING,
		SA_ATTACHED,
		SA_DETACHING
	};

	struct Instance {
		int Index;
		bool OldProc;
		emAvClient * Client;
		ShmAttachStateType ShmAttachState;
		int MinShmSize;
		int ShmSize;
		int ShmId;
		int * ShmAddr;
		emImage Image;
	};

	enum StateType {
		STATE_IDLE,
		STATE_BUSY,
		STATE_HOLD_PROC,
		STATE_TERM_PROC
	};

	Instance * TryOpenInstance(
		const char * audioDrv, const char * videoDrv,
		const char * filePath
	) throw(emString);

	void DeleteInstance(int index);

	void SendMessage(Instance * inst, const char * tag, const char * data);

	void TryDoPipeIO() throw(emString);

	void HandleMessage(int instIndex, const char * tag, const char * data);

	void UpdateShm(Instance * inst);

	void TryCreateShm(Instance * inst) throw(emString);
	void DeleteShm(Instance * inst);

	void TransferFrames();
	void TransferFrame(Instance * inst);

	Instance * Instances[MAX_INSTANCES];
	int InstanceCount;
	StateType State;
	emTimer StateTimer;
	emProcess ServerProc;
	emArray<char> InBuf;
	emArray<char> OutBuf;
	int InBufFill;
	int OutBufFill;
	bool OutBufOverflowed;

};


#endif
