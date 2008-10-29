//------------------------------------------------------------------------------
// emTmpConvModel.h
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

#ifndef emTmpConvModel_h
#define emTmpConvModel_h

#ifndef emTmpFile_h
#include <emCore/emTmpFile.h>
#endif

#ifndef emProcess_h
#include <emCore/emProcess.h>
#endif

#ifndef emFileManModel_h
#include <emFileMan/emFileManModel.h>
#endif

class emTmpConvModelClient;


class emTmpConvModel : public emModel {

public:

	static emString MakeName(
		const emString & inputFilePath, const emString & command,
		const emString & outputFileEnding
	);

	static emRef<emTmpConvModel> Acquire(
		emContext & context, const emString & inputFilePath,
		const emString & command, const emString & outputFileEnding,
		bool common=true
	);

	const emString & GetInputFilePath() const;
	const emString & GetCommand() const;
	const emString & GetOutputFileEnding() const;

	const emSignal & GetChangeSignal() const;

	enum ConversionState {
		CS_DOWN,
		CS_WAITING,
		CS_CONVERTING,
		CS_UP,
		CS_ERROR
	};

	ConversionState GetConversionState() const;

	const emString & GetErrorText() const;

	const emString & GetOutputFilePath() const;

protected:

	emTmpConvModel(
		emContext & context, const emString & name,
		const emString & inputFilePath, const emString & command,
		const emString & outputFileEnding
	);

	virtual ~emTmpConvModel();

	virtual bool Cycle();

private:

	friend class emTmpConvModelClient;

	void TryStepConversion();
	void ClientsChanged();
	void StartPSAgent();
	void EndPSAgent();

	class PSAgentClass : public emPriSchedAgent {
	public:
		PSAgentClass(emTmpConvModel & model);
	protected:
		virtual void GotAccess();
	private:
		emTmpConvModel & Model;
	};
	friend class PSAgentClass;

	emRef<emFileManModel> FileManModel;
	emRef<emSigModel> UpdateSignalModel;
	emString InputFilePath;
	emString Command;
	emString OutputFileEnding;
	emSignal ChangeSignal;
	ConversionState State;
	int ConversionStage;
	emString ErrorText;
	emTmpFile TmpFile;
	bool TmpSelected;
	time_t FileTime;
	PSAgentClass * PSAgent;
	emProcess Process;
	emArray<char> ErrPipeBuf;
	emTmpConvModelClient * ClientList;
	bool ConversionWanted;
	double Priority;
};

inline const emString & emTmpConvModel::GetInputFilePath() const
{
	return InputFilePath;
}

inline const emString & emTmpConvModel::GetCommand() const
{
	return Command;
}

inline const emString & emTmpConvModel::GetOutputFileEnding() const
{
	return OutputFileEnding;
}

inline const emSignal & emTmpConvModel::GetChangeSignal() const
{
	return ChangeSignal;
}

inline emTmpConvModel::ConversionState emTmpConvModel::GetConversionState() const
{
	return State;
}

inline const emString & emTmpConvModel::GetErrorText() const
{
	return ErrorText;
}

inline const emString & emTmpConvModel::GetOutputFilePath() const
{
	return TmpFile.GetPath();
}


#endif
