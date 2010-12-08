//------------------------------------------------------------------------------
// emProcess.h
//
// Copyright (C) 2006-2010 Oliver Hamann.
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

#ifndef emProcess_h
#define emProcess_h

#ifndef emStd2_h
#include <emCore/emStd2.h>
#endif

struct emProcessPrivate;


//==============================================================================
//================================= emProcess ==================================
//==============================================================================

class emProcess : public emUncopyable {

public:

	// This class helps in creating and managing system processes.

	emProcess();
		// Construct with no child process running.

	virtual ~emProcess();
		// If a child process is still running, Terminate() is called.

	enum StartFlags {
		SF_SHARE_STDIN  = 1<<0,
		SF_PIPE_STDIN   = 1<<1,
		SF_SHARE_STDOUT = 1<<2,
		SF_PIPE_STDOUT  = 1<<3,
		SF_SHARE_STDERR = 1<<4,
		SF_PIPE_STDERR  = 1<<5
	};

	void TryStart(
		const emArray<emString> & args,
		const emArray<emString> & extraEnv=emArray<emString>(),
		const char * dirPath=NULL,
		int flags=SF_SHARE_STDIN|SF_SHARE_STDOUT|SF_SHARE_STDERR
	) throw(emString);
		// Start a managed child process.
		// Arguments:
		//   args     - Program arguments for the child process. The
		//              first entry is the program name itself, either
		//              as a file path, or just the name (PATH is
		//              searched then). The array must not be empty.
		//   extraEnv - The child process inherits the environment from
		//              this process, and in addition, it gets the
		//              environment variables given here. The array can
		//              have any number of environment variables, each
		//              of the form <Name>[=<value>]. If it's just the
		//              name, the variable is removed (on UNIX: only if
		//              supported by putenv), otherwise the variable is
		//              set or changed to the given value (which can be
		//              empty).
		//   dirPath  - Current working directory for the child process,
		//              or NULL for inheriting the current directory
		//              from this process.
		//   flags    - Combination of start flags from enum StartFlags.
		//              The flags SF_SHARE_STDIN and SF_PIPE_STDIN
		//              control the type of the standard input handle
		//              for the child process. Giving none of these
		//              flags means to have no standard input (closed
		//              handle). SF_SHARE_STDIN means to inherit the
		//              handle from this process. SF_PIPE_STDIN means to
		//              create a pipe (see methods TryWrite and
		//              CloseWrite). Setting both flags is not allowed.
		//              The other flags are for standard output and
		//              standard error analogously.
		// Throws: An error message on failure.

	static void TryStartUnmanaged(
		const emArray<emString> & args,
		const emArray<emString> & extraEnv=emArray<emString>(),
		const char * dirPath=NULL,
		int flags=SF_SHARE_STDIN|SF_SHARE_STDOUT|SF_SHARE_STDERR
	) throw(emString);
		// This function(!) is like the method TryStart, but the new
		// process is not managed by an emProcess object. Pipelining is
		// not possible with this. On UNIX, the new process is detached
		// from this process, so that it does not become a child
		// process.

	int TryWrite(const char * buf, int len) throw(emString);
		// Write to standard input of the child process, without
		// blocking. The child process should have been started with
		// SF_PIPE_STDIN.
		// Arguments:
		//   buf - Array of bytes to be written.
		//   len - Number of bytes to be written.
		// Returns:
		//   >0 - Number of bytes actually written.
		//    0 - No writing possible at this moment, please try again
		//        later (or wait by calling WaitPipes).
		//   -1 - Any end of the pipe has been closed (e.g. child
		//        process exited).
		// Throws: An error message on failure.

	int TryRead(char * buf, int maxLen) throw(emString);
		// Read from standard output of the child process, without
		// blocking. The child process should have been started with
		// SF_PIPE_STDOUT.
		// Arguments:
		//   buf - Array for storing the bytes.
		//   len - Maximum number of bytes to be read.
		// Returns:
		//   >0 - Number of bytes actually read.
		//    0 - No bytes available at this moment, please try again
		//        later (or wait by calling WaitPipes).
		//   -1 - Any end of the pipe has been closed (e.g. child
		//        process exited).
		// Throws: An error message on failure.

	int TryReadErr(char * buf, int maxLen) throw(emString);
		// Like TryRead, but for standard error (SF_PIPE_STDERR).

	enum WaitFlags {
		WF_WAIT_STDIN  = 1<<0,
		WF_WAIT_STDOUT = 1<<1,
		WF_WAIT_STDERR = 1<<2
	};
	void WaitPipes(int waitFlags, unsigned timeoutMS=UINT_MAX);
		// Wait until a pipe is ready for writing and/or reading.
		// Arguments:
		//   waitFlags - Combination of flags from enum WaitFlags. This
		//               specifies the pipes to wait for. The method
		//               returns when at least one of these pipes is
		//               ready.
		//   timeoutMS - After this time-out in milliseconds, the
		//               method returns even if no pipe is ready.
		//               UINT_MAX means infinite.

	void CloseWriting();
	void CloseReading();
	void CloseReadingErr();
		// Close this end of a pipe.

	void SendTerminationSignal();
		// Send a termination request to the child process. On UNIX, the
		// signal SIGTERM is sent to the child process. On Windows, the
		// message WM_QUIT is sent to the primary thread of the child
		// process.

	void SendKillSignal();
		// Hardly kill the child process. Usually this should never be
		// called. On UNIX, the signal SIGKILL is sent to the child
		// process. On Windows, the SDK function TerminateProcess is
		// called.

	bool WaitForTermination(unsigned timeoutMS=UINT_MAX);
		// Wait for the child process to terminate.
		// Arguments:
		//   timeoutMS - Time-out in milliseconds. UINT_MAX means
		//               infinite.
		// Returns:
		//   true  - Child process terminated (or never started).
		//   false - Timed out.

	bool IsRunning();
		// true if a child process has been started and not yet
		// terminated.

	void Terminate(unsigned fatalTimeoutMS=20000);
		// Like SendTerminationSignal plus WaitForTermination. But if
		// the process does not terminate within the given time-out in
		// milliseconds, emFatalError is called (because it is assumed
		// to have a programming error somewhere). The time-out should
		// be quite large, for the case of a badly overloaded system.

	int GetExitStatus() const;
		// Get the exit status of a terminated child process.

private:

	emProcessPrivate * P;

};


#endif
