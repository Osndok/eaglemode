//------------------------------------------------------------------------------
// emFileModel.h
//
// Copyright (C) 2005-2008 Oliver Hamann.
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

#ifndef emFileModel_h
#define emFileModel_h

#ifndef emPriSchedAgent_h
#include <emCore/emPriSchedAgent.h>
#endif

#ifndef emSigModel_h
#include <emCore/emSigModel.h>
#endif

class emFileModelClient;


//==============================================================================
//================================ emFileModel =================================
//==============================================================================

class emFileModel : public emModel {

public:

	// Abstract base class for a shared file model. Such a file model can
	// represent a file of a certain file format in memory. The most
	// important features are:
	//
	//  - File models can have clients (=> class emFileModelClient). The
	//    clients tell how much memory the model may allocate at most. If
	//    the model would need more, the file is not loaded. Without any
	//    client, the file is never loaded. Thus, if you want a file to be
	//    loaded by a file model, you will have to create and manage at
	//    least one emFileModelClient.
	//
	//  - Loading and unloading is performed automatically, depending on the
	//    the state of the clients.
	//
	//  - Saving is not performed automatically, except a derived class
	//    implements such an automatism.
	//
	//  - Loading and saving are performed step by step in the Cycle method,
	//    so that it does not block the other engines of the program.
	//
	//  - Normally, only one file model is loading at a time (just to avoid
	//    heavy seeking of the hard drive). The order of loading multiple
	//    file models is determined by a priority which can be set at the
	//    clients.

	virtual const emString & GetFilePath() const;
		// Path name of the file. Returns the model name by default.

	const emSignal & GetFileStateSignal() const;
		// This signal is sent on any change in the results of
		// GetFileState(), GetMemoryNeed(), GetFileProgress() and
		// GetErrorText().

	enum FileState {
		// Possibilities for the state of the file model.

		FS_WAITING = 0,
			// The file model wants to load the file, but it is
			// waiting until there are no other file models in a
			// loading state.

		FS_LOADING = 1,
			// The file model is currently reading the file.

		FS_LOADED = 2,
			// The file model is loaded.

		FS_UNSAVED = 3,
			// The file model is loaded and there are unsaved
			// changes in the model data. This state prevents from
			// unloading and reloading.

		FS_SAVING = 4,
			// The file model is currently writing the file.

		FS_TOO_COSTLY = 5,
			// The file model is not loaded, because there is no
			// file model client which accepts the memory need.

		FS_LOAD_ERROR = 6,
			// The file model is not loaded, because there was an
			// error in reading the file.

		FS_SAVE_ERROR = 7,
			// Like FS_UNSAVED, but there was an error in writing
			// the file.

		FS_MAX_VAL = 7
			// Just the maximum possible integer value for the
			// state.
	};

	FileState GetFileState() const;
		// Get current state of this file model.

	emUInt64 GetMemoryNeed() const;
		// Get best known number of memory bytes, which are allocated,
		// or which will be allocated, or which would be allocated, in
		// loaded state. In addition to the memory need of the file
		// model itself, this should also include the memory need of one
		// typical client (e.g. emFilePanel), if not negligible. The
		// result of GetMemoryNeed() is never zero. In unsaved or
		// errored state, it could be nonsense.

	double GetFileProgress() const;
		// Get progress of loading or saving in percent.

	const emString & GetErrorText() const;
		// Get the error description when in state FS_LOAD_ERROR or
		// FS_SAVE_ERROR.

	void Update();
		// Perform an update: If the state is FS_LOAD_ERROR, the error
		// text is cleared and the loading will be tried again. If the
		// state is FS_TOO_COSTLY with a last known memory need greater
		// than one, the loading will be tried again. If the state is
		// FS_LOADED, and if the model is out-of-date (checks file time
		// by default), the file is unloaded for loading it again.
		// Hints:
		//  - A good place to call this method is when creating a new
		//    emFileModelClient.
		//  - Do not call this too often, because some file models are
		//    always reloading.

	static emRef<emSigModel> AcquireUpdateSignalModel(
		emRootContext & rootContext
	);
		// This functions acquires a global signal model. When that
		// signal is signaled, all file models are updated (see method
		// Update()), except for those which have
		// GetIgnoreUpdateSignal()==true. The signal could be used as an
		// application-wide update signal even for reloading files which
		// are not interfaced through emFileModel.

	bool GetIgnoreUpdateSignal() const;
	void SetIgnoreUpdateSignal(bool ignore);
		// If true, this file model does not listen to the global update
		// signal. Should be set at most for private file models.

	void Load(bool immediately);
		// Normally, there is no need to call this method, because
		// loading is performed automatically. If the state is
		// FS_WAITING, Load(false) starts the loading right away,
		// ignoring the priority. If the state is FS_LOADING,
		// Load(false) performs one more step in loading the file.
		// Load(true) blocks until the state is not FS_WAITING and not
		// FS_LOADING. For other states, this method has no effect.

	void Save(bool immediately);
		// This is similar to Load, but remember that saving is not
		// started automatically. If the state is FS_UNSAVED,
		// Save(false) starts the saving (which continues
		// automatically). If the state is FS_SAVING, Save(false)
		// performs one more step in saving the file. Save(true) blocks
		// until the state is not FS_UNSAVED and not FS_SAVING. For
		// other states, this method has no effect.

	void ClearSaveError();
		// Resets the state to FS_UNSAVED, if it was FS_SAVE_ERROR.

	void HardResetFileState();
		// Unload the file and restart the loading logics. This works
		// even in unsaved or errored state, and it is the only way to
		// take back unsaved changes.

protected:

	emFileModel(emContext & context, const emString & name);
		// Constructor.
		// Arguments:
		//   context - Normally, this should be the root context.
		//   name - Normally, this is the path name of the file
		//          (otherwise GetFilePath has to be overloaded).

	virtual ~emFileModel();
		// Destructor.

	virtual bool Cycle();
		// See emEngine::Cycle. This one performs loading and saving.

	void SetUnsavedState();
		// This must be called before or immediately after modifying the
		// data (except through ResetData, TryStartLoading and
		// TryContinueLoading). Hereby, any loading or saving is
		// aborted, and the file state is set to FS_UNSAVED. The caller
		// should take care: if the state was FS_SAVING, the file will
		// be corrupted, and if the state was FS_LOADING, the data may
		// be corrupted.

	virtual void ResetData() = 0;
		// Called for unloading the file. The implementation may free
		// allocated memory or set the data to a default state.

	virtual void TryStartLoading() throw(emString) = 0;
	virtual bool TryContinueLoading() throw(emString) = 0;
	virtual void QuitLoading() = 0;
		// Called for loading the file. First, TryStartLoading is
		// called, and then TryContinueLoading is called again and again
		// until true is returned. For aborting by an error, an
		// exception with a user-readable error message can be thrown.
		// Returning true from TryContinueLoading means to have finished
		// with loading. No call should waist more than about 10
		// milliseconds (if possible somehow). There's no problem if
		// each call waists much fewer time, but if multiple calls
		// cannot really continue because of waiting for a child process
		// or so, they should do an emSleepMS(10) or something similar,
		// otherwise we would end up in busy waiting. Before allocating
		// a worth meaning amount of memory, a call should return and
		// the amount of memory should be reported through the result of
		// CalcMemoryNeed(), so that there is a chance to abort the
		// loading and to enter the state FS_TOO_COSTLY before the
		// memory would be allocated. Best is to determine the whole
		// memory need in TryStartLoading (e.g. through reading just a
		// file header), and to allocate that memory in
		// TryContinueLoading. It is guaranteed that ResetData is called
		// before TryStartLoading. And it is guaranteed that QuitLoading
		// is called at the end, either on success, or for aborting, or
		// after an error - but remember that it can never be called
		// through the destructor of emFileModel (=> prepare the
		// destructor of the derived class accordingly).

	virtual void TryStartSaving() throw(emString) = 0;
	virtual bool TryContinueSaving() throw(emString) = 0;
	virtual void QuitSaving() = 0;
		// This is just like above, but for saving. The memory need is
		// not relevant here.

	virtual emUInt64 CalcMemoryNeed() = 0;
		// While loading, this method is called again and again to
		// calculate the number of memory bytes, which will be allocated
		// by this file model and by one typical file model client in
		// loaded state. If it cannot be calculated, it should be a good
		// approximation (e.g. from file size).

	virtual double CalcFileProgress() = 0;
		// While loading or saving, this method is called to determine
		// the progress in percent. It's just an information for the
		// user.

	virtual bool IsOutOfDate();
		// Check whether the loaded file should be reloaded. The default
		// implementation checks by file modification time.

private: friend class emFileModelClient;

	void ClientsChanged();
	bool StepLoading();
	bool StepSaving();
	bool UpdateFileProgress();
	void StartPSAgent();
	void EndPSAgent();

	class PSAgentClass : public emPriSchedAgent {
	public:
		PSAgentClass(emFileModel & fileModel);
	protected:
		virtual void GotAccess();
	private:
		emFileModel & FileModel;
	};

	friend class PSAgentClass;

	emSignal FileStateSignal;
	FileState State;
	emUInt64 MemoryNeed;
	double FileProgress;
	emUInt64 FileProgressClock;
	emString ErrorText;
	emFileModelClient * ClientList;
	emUInt64 MemoryLimit;
	time_t FileTime;
	PSAgentClass * PSAgent;
	emRef<emSigModel> UpdateSignalModel; // NULL if ignored
};

inline const emSignal & emFileModel::GetFileStateSignal() const
{
	return FileStateSignal;
}

inline emFileModel::FileState emFileModel::GetFileState() const
{
	return State;
}

inline emUInt64 emFileModel::GetMemoryNeed() const
{
	return MemoryNeed;
}

inline double emFileModel::GetFileProgress() const
{
	return FileProgress;
}

inline const emString & emFileModel::GetErrorText() const
{
	return ErrorText;
}

inline bool emFileModel::GetIgnoreUpdateSignal() const
{
	return UpdateSignalModel==NULL;
}


//==============================================================================
//============================= emFileModelClient ==============================
//==============================================================================

class emFileModelClient : public emUncopyable {

public:

	// Class for a client on an emFileModel. Multiple clients can connect to
	// the same file model. And:
	//
	// - Each client tells about the maximum memory which may be allocated
	//   by the file model and one typical client (e.g. emFilePanel). If no
	//   client accepts the memory need this way, the file is not loaded or
	//   it is unloaded.
	//
	// - Each client tells about the priority which is used for determining
	//   the order of loading file models. The maximum priority of all
	//   clients is taken.
	//
	// On each modification of a memory limit or a priority, the logics of
	// emFileModel reacts automatically.
	//
	// Hint: Even have a look at the class emFilePanel. It's a base class
	// for panels which want to be file mode clients.

	emFileModelClient(emFileModel * model=NULL, emUInt64 memoryLimit=0,
	                  double priority=0.0);
		// Constructor.
		// Arguments:
		//   model        - See SetModel below.
		//   memoryLimit  - See SetMemoryLimit below.
		//   priority     - See SetPriority below.

	virtual ~emFileModelClient();
		// Destructor.

	const emFileModel * GetModel() const;
	emFileModel * GetModel();
	void SetModel(emFileModel * model=NULL);
		// The file model this client is connected to. NULL means to
		// have disconnected state.

	void SetMemoryLimit(emUInt64 bytes);
	emUInt64 GetMemoryLimit() const;
		// Maximum memory need accepted for loading the file, from sight
		// of this client. Usually, this should be set from
		// emPanel::GetMemoryLimit().

	double GetPriority() const;
	void SetPriority(double priority);
		// Priority in loading the file, from sight of this client.
		// Usually, this should be set from
		// emPanel::GetUpdatePriority().

private: friend class emFileModel;

	emRef<emFileModel> Model;
	emUInt64 MemoryLimit;
	double Priority;
	emFileModelClient * * ThisPtrInList;
	emFileModelClient * NextInList;
};

inline const emFileModel * emFileModelClient::GetModel() const
{
	return Model;
}

inline emFileModel * emFileModelClient::GetModel()
{
	return Model;
}

inline emUInt64 emFileModelClient::GetMemoryLimit() const
{
	return MemoryLimit;
}

inline double emFileModelClient::GetPriority() const
{
	return Priority;
}


#endif
