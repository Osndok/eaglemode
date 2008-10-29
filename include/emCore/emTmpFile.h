//------------------------------------------------------------------------------
// emTmpFile.h
//
// Copyright (C) 2006-2008 Oliver Hamann.
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

#ifndef emTmpFile_h
#define emTmpFile_h

#ifndef emModel_h
#include <emCore/emModel.h>
#endif

#ifndef emMiniIpc_h
#include <emCore/emMiniIpc.h>
#endif


//==============================================================================
//================================= emTmpFile ==================================
//==============================================================================

class emTmpFile : public emUncopyable {

public:

	// An object of this class can invent and hold the path of a temporary
	// file (or directory). The file (or directory tree) is deleted
	// automatically by the destructor. But the file is not created by this.

	emTmpFile();
		// Construct with an empty path.

	emTmpFile(emRootContext & rootContext, const char * postfix=NULL);
		// Like Setup.

	emTmpFile(const emString & customPath);
		// Like SetupCustomPath.

	~emTmpFile();
		// Like Discard.

	void Setup(emRootContext & rootContext, const char * postfix=NULL);
		// Invent and set a new path for a temporary file or directory,
		// which does not yet exist. Before that, Discard is called.
		//
		// *** DANGER ***
		// The path is invented via an emTmpFileMaster. Please read the
		// comments on emTmpFileMaster::InventPath.
		//
		// Arguments:
		//   rootContext - The root context. The context must live
		//                 longer than this emTmpFile object.
		//   postfix     - A postfix to be added to the end of the path
		//                 (file name suffix), or NULL.

	void SetupCustomPath(const emString & customPath);
		// Like Setup, but use the given path.

	const emString & GetPath() const;
		// Get the path of the temporary file.

	void Discard();
		// If the path is not empty, the file or directory tree is
		// deleted, and the path is set empty.

private:

	emString Path;
};

inline const emString & emTmpFile::GetPath() const
{
	return Path;
}


//==============================================================================
//============================== emTmpFileMaster ===============================
//==============================================================================

class emTmpFileMaster : public emModel {

public:

	// Class for a central instance inventing temporary file paths, and for
	// cleaning up.

	static emRef<emTmpFileMaster> Acquire(emRootContext & rootContext);
		// Acquire the emTmpFileMaster.

	emString InventPath(const char * postfix=NULL);
		// Invent a path for a temporary file or directory which does
		// not yet exist. The path ends with the given postfix, and it
		// lies in a special directory which is created solely for this
		// emTmpFileMaster. That directory and all its contents are
		// deleted on destruction of this emTmpFileMaster (= destruction
		// of root context). If that deletion does not take place
		// because of a program crash or so, the deletion is performed
		// at next construction of an emTmpFileMaster if it is a process
		// run by the same user on the same host. An emMiniIpcServer is
		// managed to find out whether the emTmpFileMaster for a
		// directory still exists.
		//
		// *** DANGER ***
		// If it ever fails to access the emMiniIpcServer of an existing
		// emTmpFileMaster from any process of the same user and host
		// for some reason, the files are deleted falsely. Therefore,
		// the temporary files should not be too valuable! And besides,
		// no one else should ever create a file in the directory, than
		// those who use a path invented with this method.

protected:

	emTmpFileMaster(emContext & context, const emString & name);

	virtual ~emTmpFileMaster();

private:

	emString GetCommonPath();
	void DeleteDeadDirectories();
	void StartOwnDirectory();

	class IpcServerClass : public emMiniIpcServer {
	public:
		IpcServerClass(emScheduler & scheduler);
	protected:
		virtual void OnReception(int argc, const char * const argv[]);
	};

	IpcServerClass IpcServer;
	emString DirPath;
	unsigned FileNameCounter;
	static const char * DirNameEnding;
};


#endif
