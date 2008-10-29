//------------------------------------------------------------------------------
// emTmpConvModel.cpp
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

#include <emCore/emInstallInfo.h>
#include <emTmpConv/emTmpConvModel.h>
#include <emTmpConv/emTmpConvModelClient.h>


emString emTmpConvModel::MakeName(
	const emString & inputFilePath, const emString & command,
	const emString & outputFileEnding
)
{
	return emString::Format(
		"%d:%s,%d:%s,%d:%s",
		inputFilePath.GetLen(),
		inputFilePath.Get(),
		command.GetLen(),
		command.Get(),
		outputFileEnding.GetLen(),
		outputFileEnding.Get()
	);
}


emRef<emTmpConvModel> emTmpConvModel::Acquire(
	emContext & context, const emString & inputFilePath,
	const emString & command, const emString & outputFileEnding,
	bool common
)
{
	emTmpConvModel * m;
	emString name;

	name=MakeName(inputFilePath,command,outputFileEnding);
	if (!common) {
		m=new emTmpConvModel(context,name,inputFilePath,command,outputFileEnding);
	}
	else {
		m=(emTmpConvModel*)context.Lookup(typeid(emTmpConvModel),name);
		if (!m) {
			m=new emTmpConvModel(context,name,inputFilePath,command,outputFileEnding);
			m->Register();
		}
	}
	return emRef<emTmpConvModel>(m);
}


emTmpConvModel::emTmpConvModel(
	emContext & context, const emString & name,
	const emString & inputFilePath, const emString & command,
	const emString & outputFileEnding
)
	: emModel(context,name)
{
	FileManModel=emFileManModel::Acquire(GetRootContext());
	UpdateSignalModel=emFileModel::AcquireUpdateSignalModel(GetRootContext());
	InputFilePath=inputFilePath;
	Command=command;
	OutputFileEnding=outputFileEnding;
	State=CS_DOWN;
	ConversionStage=0;
	TmpSelected=false;
	FileTime=0;
	PSAgent=NULL;
	ErrPipeBuf.SetTuningLevel(4);
	ClientList=NULL;
	ConversionWanted=false;
	Priority=0.0;

	AddWakeUpSignal(FileManModel->GetSelectionSignal());
	AddWakeUpSignal(UpdateSignalModel->Sig);
}


emTmpConvModel::~emTmpConvModel()
{
	EndPSAgent();
	Process.Terminate();
	TmpFile.Discard();
}


bool emTmpConvModel::Cycle()
{
	if (IsSignaled(UpdateSignalModel->Sig)) {
		switch (State) {
		case CS_UP:
			try {
				if (FileTime!=emTryGetFileTime(InputFilePath)) {
					TmpFile.Discard();
					TmpSelected=false;
					State=CS_DOWN;
					Signal(ChangeSignal);
				}
			}
			catch (emString) {
			}
			break;
		case CS_ERROR:
			ErrorText.Empty();
			State=CS_DOWN;
			Signal(ChangeSignal);
			break;
		default:
			break;
		}
	}

	if (IsSignaled(FileManModel->GetSelectionSignal())) {
		if (State==CS_UP) {
			TmpSelected=FileManModel->IsAnySelectionInDirTree(TmpFile.GetPath());
		}
	}

	if (
		!ConversionWanted && !TmpSelected &&
		State!=CS_DOWN && State!=CS_ERROR
	) {
		EndPSAgent();
		Process.Terminate();
		ErrPipeBuf.Empty(true);
		TmpFile.Discard();
		TmpSelected=false;
		State=CS_DOWN;
		Signal(ChangeSignal);
	}

	if (ConversionWanted && State==CS_DOWN) {
		StartPSAgent();
		State=CS_WAITING;
		Signal(ChangeSignal);
	}

	if (State==CS_WAITING && PSAgent->HasAccess()) {
		ConversionStage=0;
		State=CS_CONVERTING;
		Signal(ChangeSignal);
	}

	if (State==CS_CONVERTING) {
		try {
			TryStepConversion();
		}
		catch (emString errorMessage) {
			EndPSAgent();
			Process.Terminate();
			ErrPipeBuf.Empty(true);
			TmpFile.Discard();
			TmpSelected=false;
			ErrorText=errorMessage;
			State=CS_ERROR;
			Signal(ChangeSignal);
		}
	}

	if (TmpSelected && State==CS_UP) {
		SetMinCommonLifetime(UINT_MAX);
	}
	else {
		SetMinCommonLifetime(0);
	}

	return State==CS_CONVERTING;
}


void emTmpConvModel::TryStepConversion()
{
	emArray<emString> args,extraEnv;
	char buf[256];
	int l,r;

	switch (ConversionStage) {
	case 0:
		FileTime=emTryGetFileTime(InputFilePath);
		ConversionStage=1;
		if (IsTimeSliceAtEnd()) break;
	case 1:
		TmpFile.Setup(GetRootContext(),OutputFileEnding);
		ConversionStage=2;
		if (IsTimeSliceAtEnd()) break;
	case 2:
		args=emGetInstallPath(EM_IDT_LIB,"emTmpConv","emTmpConv/emTmpConvProc");
		args+=Command;
		extraEnv=emString::Format("INFILE=%s",InputFilePath.Get());
		extraEnv+=emString::Format("OUTFILE=%s",TmpFile.GetPath().Get());
		Process.TryStart(
			args,
			extraEnv,
			NULL,
			emProcess::SF_PIPE_STDIN|emProcess::SF_PIPE_STDERR
		);
		ConversionStage=3;
		break;
	case 3:
		do {
			l=Process.TryReadErr(buf,sizeof(buf)-1);
			if (l<=0) break;
			r=ErrPipeBuf.GetCount()-2000;
			if (r>0) {
				ErrPipeBuf.Replace(0,r,"...",3);
			}
			ErrPipeBuf.Add(buf,l);
		} while (!IsTimeSliceAtEnd());
		if (l<0 && !Process.IsRunning()) {
			if (Process.GetExitStatus()!=0) {
				throw
					emString("Child process returned bad status:\n\n") +
					emString(ErrPipeBuf.Get(),ErrPipeBuf.GetCount())
				;
			}
			EndPSAgent();
			Process.Terminate();
			ErrPipeBuf.Empty(true);
			State=CS_UP;
			Signal(ChangeSignal);
		}
		break;
	}
}


void emTmpConvModel::ClientsChanged()
{
	emTmpConvModelClient * c;

	ConversionWanted=false;
	Priority=0.0;
	for (c=ClientList; c; c=c->NextInList) {
		if (c->ConversionWanted) ConversionWanted=true;
		if (Priority<c->Priority) Priority=c->Priority;
	}
	if (PSAgent) PSAgent->SetAccessPriority(Priority);
	WakeUp();
}


void emTmpConvModel::StartPSAgent()
{
	if (!PSAgent) PSAgent=new PSAgentClass(*this);
	PSAgent->RequestAccess();
}


void emTmpConvModel::EndPSAgent()
{
	if (PSAgent) {
		delete PSAgent;
		PSAgent=NULL;
	}
}


emTmpConvModel::PSAgentClass::PSAgentClass(emTmpConvModel & model)
	: emPriSchedAgent(model.GetRootContext(),"cpu",model.Priority),
	Model(model)
{
}


void emTmpConvModel::PSAgentClass::GotAccess()
{
	Model.WakeUp();
}
