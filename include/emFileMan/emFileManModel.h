//------------------------------------------------------------------------------
// emFileManModel.h
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

#ifndef emFileManModel_h
#define emFileManModel_h

#ifndef emFileModel_h
#include <emCore/emFileModel.h>
#endif

#ifndef emMiniIpc_h
#include <emCore/emMiniIpc.h>
#endif

#ifndef emToolkit_h
#include <emCore/emToolkit.h>
#endif

#ifndef emDirEntry_h
#include <emFileMan/emDirEntry.h>
#endif


class emFileManModel : public emModel {

public:

	static emRef<emFileManModel> Acquire(emRootContext & rootContext);

	const emSignal & GetSelectionSignal() const;

	void ClearSourceSelection();
	void ClearTargetSelection();

	void SelectAsSource(const emString & path);
	void SelectAsTarget(const emString & path);

	void DeselectAsSource(const char * path);
	void DeselectAsTarget(const char * path);

	bool IsSelectedAsSource(const char * path) const;
	bool IsSelectedAsTarget(const char * path) const;

	bool IsAnySelectionInDirTree(const char * dirPath) const;
		// Slow linear search

	int GetSourceSelectionCount() const;
	int GetTargetSelectionCount() const;

	emString GetSourceSelection(int index) const;
	emString GetTargetSelection(int index) const;

	emArray<emDirEntry> CreateSortedSrcSelDirEntries(emView & contentView) const;
	emArray<emDirEntry> CreateSortedTgtSelDirEntries(emView & contentView) const;

	const emString & GetShiftTgtSelPath() const;
	void SetShiftTgtSelPath(const emString & path);

	void SwapSelection();
	void UpdateSelection();

	void SelectionToClipboard(emView & contentView, bool source=false,
	                          bool namesOnly=false);

	const emString & GetMiniIpcServerName();

	enum CommandType {
		CT_COMMAND,
		CT_GROUP,
		CT_SEPARATOR
	};

	struct CommandNode {
		CommandNode();
		~CommandNode();
		emString CmdPath;
		CommandType Type;
		double Order;
		emString Interpreter;
		emString Dir;
		emString DefaultFor;
		emString Caption;
		emString Description;
		emImage Icon;
		emTkLook Look;
		emInputHotkey Hotkey;
		double BorderScaling;
		double PrefChildTallness;
		emArray<const CommandNode *> Children;
		emUInt64 DirCRC;
	};

	const emSignal & GetCommandsSignal() const;
	const CommandNode * GetCommandRoot() const;
	const CommandNode * GetCommand(const emString & cmdPath) const;
	const CommandNode * SearchDefaultCommandFor(const emString & filePath) const;
	const CommandNode * SearchHotkeyCommand(const emInputHotkey & hotkey) const;
	void RunCommand(const CommandNode * cmd, emView & contentView);

	void HotkeyInput(emView & contentView, emInputEvent & event,
	                 const emInputState & state);

protected:

	emFileManModel(emContext & context, const emString & name);
	virtual ~emFileManModel();
	virtual bool Cycle();

private:

	struct SelEntry {
		int HashCode;
		emString Path;
	};

	struct CmdEntry {
		int HashCode;
		CommandNode * Node;
	};

	class IpcServerClass : public emMiniIpcServer {
	public:
		IpcServerClass(emFileManModel & fmModel);
	protected:
		virtual void OnReception(int argc, const char * const argv[]);
	private:
		emFileManModel & FmModel;
	};

	friend class IpcServerClass;

	void OnIpcReception(int argc, const char * const argv[]);

	static int SearchSelection(const emArray<SelEntry> & sel, int hashCode,
	                           const char * path);
	emArray<emDirEntry> CreateSortedSelDirEntries(
		emView & contentView, const emArray<SelEntry> & sel
	) const;
	static int CmpDEs(
		const emDirEntry * de1, const emDirEntry * de2,
		void * context // The emFileManViewConfig
	);

	void UpdateCommands();
	bool CheckCRCs(const CommandNode * parent);
	emUInt64 CalcDirCRC(const emString & dir, const emArray<emString> & names);
	void ClearCommands();
	void LoadCommands(const emString & rootDir);
	void LoadChildCommands(CommandNode * parent);
	void LoadCommand(CommandNode * cmd, const emString & cmdPath);
	static int CompareCmds(
		const CommandNode * const * n1, const CommandNode * const * n2,
		void * context
	);
	int SearchCommand(int hashCode, const char * path) const;
	const CommandNode * SearchDefaultCommandFor(
		const CommandNode * parent, const emString & filePath,
		int * pPriority=NULL
	) const;
	const CommandNode * SearchHotkeyCommand(
		const CommandNode * parent, const emInputHotkey & hotkey
	) const;
	int CheckDefaultCommand(
		const CommandNode * cmd, const emString & filePath
	) const;
	emString GetCommandRunId() const;


	emSignal SelectionSignal;
	emArray<SelEntry> Sel[2];
	emString ShiftTgtSelPath;
	unsigned int SelCmdCounter;

	IpcServerClass * IpcServer;

	emRef<emSigModel> FileUpdateSignalModel;

	emSignal CommandsSignal;

	CommandNode * CmdRoot;
	emArray<CmdEntry> Cmds;
};

inline const emSignal & emFileManModel::GetSelectionSignal() const
{
	return SelectionSignal;
}

inline bool emFileManModel::IsSelectedAsSource(const char * path) const
{
	return SearchSelection(Sel[0],emCalcHashCode(path),path)>=0;
}

inline bool emFileManModel::IsSelectedAsTarget(const char * path) const
{
	return SearchSelection(Sel[1],emCalcHashCode(path),path)>=0;
}

inline int emFileManModel::GetSourceSelectionCount() const
{
	return Sel[0].GetCount();
}

inline int emFileManModel::GetTargetSelectionCount() const
{
	return Sel[1].GetCount();
}

inline emString emFileManModel::GetSourceSelection(int index) const
{
	return Sel[0][index].Path;
}

inline emString emFileManModel::GetTargetSelection(int index) const
{
	return Sel[1][index].Path;
}

inline emArray<emDirEntry> emFileManModel::CreateSortedSrcSelDirEntries(
	emView & contentView
) const
{
	return CreateSortedSelDirEntries(contentView,Sel[0]);
}

inline emArray<emDirEntry> emFileManModel::CreateSortedTgtSelDirEntries(
	emView & contentView
) const
{
	return CreateSortedSelDirEntries(contentView,Sel[1]);
}

inline const emString & emFileManModel::GetShiftTgtSelPath() const
{
	return ShiftTgtSelPath;
}

inline const emString & emFileManModel::GetMiniIpcServerName()
{
	return IpcServer->GetServerName();
}

inline const emSignal & emFileManModel::GetCommandsSignal() const
{
	return CommandsSignal;
}

inline const emFileManModel::CommandNode * emFileManModel::GetCommandRoot() const
{
	return CmdRoot;
}


#endif
