//------------------------------------------------------------------------------
// cmd-util.js
//
// Copyright (C) 2008-2011 Oliver Hamann.
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


//====================== Parse arguments / private stuff =======================

var Pass;
var FirstPassResult;
var Src=new Array;
var Tgt=new Array;


{
	var srcCnt,tgtCnt,i,j;

	i=0;
	if (WScript.Arguments(i)=="pass2")  {
		Pass=2;
		FirstPassResult=WScript.Arguments(i+1);
		i+=2;
	}
	else {
		Pass=1;
		FirstPassResult="";
	}
	if (WScript.Arguments.length<i+2) {
		WScript.Echo("bad arguments");
		WScript.Quit(1);
	}
	srcCnt=parseInt(WScript.Arguments(i++));
	tgtCnt=parseInt(WScript.Arguments(i++));
	if (isNaN(srcCnt) || isNaN(tgtCnt) || WScript.Arguments.length!=i+srcCnt+tgtCnt) {
		WScript.Echo("bad arguments");
		WScript.Quit(1);
	}
	for (j=0; j<srcCnt; j++) Src[j]=WScript.Arguments(i+j);
	for (j=0; j<tgtCnt; j++) Tgt[j]=WScript.Arguments(i+srcCnt+j);
	CheckEnvProblem(Src);
	CheckEnvProblem(Tgt);
}


//============================= First/second pass ==============================

function SecondPassInTerminal(title, waitUser)
	// Restart the whole command script in a terminal.
	// This function does not return.
{
	BatBegin(title);
	BatWriteCmd(
		[
			'cscript',
			WScript.ScriptFullName,
			'//NoLogo',
			'pass2',
			FirstPassResult,
			Src.length.toString(),
			Tgt.length.toString()
		]
		.concat(Src)
		.concat(Tgt)
	);
	BatWriteCheckError();
	if (waitUser) BatWriteExitByUser();
	BatEnd();
	WScript.Quit(0);
}


function IsFirstPass()
	// Returns non-zero if the script has not yet been restarted for the
	// second pass.
{
	return (Pass == 1);
}


function SetFirstPassResult(val)
	// Set a string value. It can be re-get in the second pass.
{
	FirstPassResult=val;
}


function GetFirstPassResult()
	// Get the value set with SetFirstPassResult.
{
	return (FirstPassResult);
}


//============================= Get the selections =============================

function GetSrc()
	// Get list of source-selected files.
{
	return (Src);
}


function GetTgt()
	// Get list of target-selected files.
{
	return (Tgt);
}


function GetSrcListing()
	// Get a string containing a listing of source-selected files, each on a
	// separate line, with some indent.
{
	var i,str;

	str="";
	for (i=0; i<Src.length; i++) {
		str+="  "+Src[i]+"\n";
	}
	return (str);
}


function GetTgtListing()
	// Get a string containing a listing of target-selected files, each on a
	// separate line, with some indent.
{
	var i,str;

	str="";
	for (i=0; i<Tgt.length; i++) {
		str+="  "+Tgt[i]+"\n";
	}
	return (str);
}


//============================== General Helpers ===============================

function CommandFromArgs(argsArray)
{
	var i,j,k,c,cmd,arg;

	cmd="";
	for (i=0; i<argsArray.length; i++) {
		if (i>0) cmd+=" ";
		arg=argsArray[i];
		for (j=arg.length-1; j>=0; j--) {
			c=arg.charAt(j);
			if ((c<'0' || c>'9') && (c<'A' || c>'Z') && (c<'a' || c>'z') &&
			    c!='\\' && c!='/' && c!='.' && c!=':' && c!='-' && c!='+') break;
		}
		if (j<0 && arg.length>0) {
			cmd+=arg;
		}
		else {
			cmd+='"';
			k=0;
			for (j=0; j<arg.length; j++) {
				c=arg.charAt(j);
				if (c=='"') {
					while (k>0) { cmd+='\\'; k--; }
					cmd+='\\';
				}
				else if (c=='\\') k++;
				else k=0;
				cmd+=c;
			}
			while (k>0) { cmd+='\\'; k--; }
			cmd+='"';
		}
	}
	return (cmd);
}


function QuoteArg(arg)
{
	return (CommandFromArgs([arg]));
}


function CheckEnvProblem(argOrArgs)
{
	if (typeof argOrArgs == "string") {
		if (argOrArgs.indexOf('%')>=0) {
			WScript.Echo(
				"ERROR: An argument contains a percent character: "+
				argOrArgs+". "+
				"This is not handled because of the danger of "+
				"accidentally resolving of an environment variable."
			);
			WScript.Quit(1);
		}
	}
	else {
		for (var i=0; i<argOrArgs.length; i++) {
			CheckEnvProblem(argOrArgs[i]);
		}
	}
}


function CheckFilename(name)
{
	if (/["*?:<>/|\\]/.test(name)) {
		Error(
			"File names must not contain any of these\n"+
			"characters: \" *  ? : < > / | \\"
		);
	}
	if (/[ .]$/.test(name)) {
		Error("File names must not end with a space or a period.\n");
	}
	if (/^(aux|(com[0-9])|con|(lpt[0-9])|nul|prn)($|\.)/i.test(name)) {
		Error("The file name is not allowed because it matches a device name.\n");
	}
}


function GetParentPath(path)
{
	if (IsRootDirectory(path)) return (path);
	else return (FileSys.GetParentFolderName(path));
}


function GetChildPath(parent,child)
{
	if (
		parent.length<=0 ||
		parent.charAt(parent.length-1)=='\\' ||
		child.length<=0 ||
		child.charAt(0)=='\\'
	) {
		return (parent + child);
	}
	else {
		return (parent + '\\' + child);
	}
}


function GetNameInPath(path)
{
	return (FileSys.GetFileName(path));
}


function IsRootDirectory(path)
{
	return (
		(path.length==2 && path.charAt(1)==':') ||
		(path.length==3 && path.charAt(1)==':' && path.charAt(2)=='\\')
	);
}


function IsDirectory(path)
{
	return (FileSys.FolderExists(path) || IsRootDirectory(path));
}


function IsRegularFile(path)
{
	return (FileSys.FileExists(path));
}


function IsExistingPath(path)
{
	return (IsRegularFile(path) || IsDirectory(path));
}


function IsHiddenPath(path)
{
	var entry;
	if (IsDirectory(path)) entry=FileSys.GetFolder(path);
	else entry=FileSys.GetFile(path);
	return ((entry.Attributes&2)!=0);
}


function TestEnding(path,ending)
{
	if (ending.length>0 && ending.charAt(0)=='.') ending=ending.substr(1);
	var i=path.length-ending.length;
	return (
		i>=2 &&
		path.charAt(i-1)=='.' &&
		path.substr(i).toLowerCase()==ending.toLowerCase()
	);
}


function HasAnyEnding(path)
{
	var name=GetNameInPath(path);
	var i=name.indexOf('.');
	return (i>=1 && i<name.length-1);
}


function IsDosFilename(name)
{
	var i=name.indexOf('.');
	var l=name.length;
	return (i>=0 && l<=12 && i>=l-4);
}


function ConcatArrays(array1,array2)
{
	return (array1.concat(array2));
}


function RestoreOrigPath()
{
	var env=WshShell.Environment("PROCESS");
	var origPath=env("EM_ORIG_PATH");
	if (origPath) {
		env("PATH")=origPath;
		env.Remove("EM_ORIG_PATH");
	}
}


//======================= Sending commands to eaglemode ========================

function Send(cmd,files)
{
	var env=WshShell.Environment("PROCESS");
	var args=new Array;
	args[0]=env("EM_DIR") + "\\bin\\emSendMiniIpc.exe";
	args[1]=env("EM_FM_SERVER_NAME");
	args[2]=cmd;
	if (files != null) {
		args[3]=env("EM_COMMAND_RUN_ID");
		args=args.concat(files);
	}
	WshShell.Run(CommandFromArgs(args),1,true);
}


function SendUpdate()
	// Require Eagle Mode to reload changed files and directories.
{
	Send("update",null);
}


function SendSelect(files)
	// Require Eagle Mode to select other targets. It will even reload
	// changed files and directories. As usual, the source selection is set
	// from the old target selection.
{
	Send("select",files);
}


function SendSelectKS(files)
	// Like SendSelect, but the source selection is not modified.
{
	Send("selectks",files);
}


function SendSelectCS(files)
	// Like SendSelect, but the source selection is cleared.
{
	Send("selectcs",files);
}


//=============================== Basic dialogs ================================

function StartDlg(argsArray)
{
	var env,x,y,w,h,allArgs;

	env=WshShell.Environment("PROCESS");
	w=400;
	h=300;
	x=Math.round(parseInt(env("EM_X"))+(parseInt(env("EM_WIDTH"))-w)/2);
	y=Math.round(parseInt(env("EM_Y"))+(parseInt(env("EM_HEIGHT"))-h)/2);
	if (x<0) x=0;
	if (y<0) y=0;
	allArgs=new Array;
	allArgs[0]=env("EM_DIR") + "\\bin\\emShowStdDlg.exe";
	allArgs[1]="-geometry";
	allArgs[2]=w+"x"+h+"+"+x+"+"+y;
	for (i=0; i<argsArray.length; i++) allArgs[3+i]=argsArray[i];
	return (WshShell.Exec(CommandFromArgs(allArgs)));
}


function Dlg(argsArray)
{
	var e;

	e=StartDlg(argsArray);
	while (e.Status==0) WScript.Sleep(100);
	return (e.ExitCode==0 ? true : false);
}


function DlgRead(argsArray)
{
	var e,str;

	e=StartDlg(argsArray);
	str=e.StdOut.ReadLine();
	while (e.Status==0) WScript.Sleep(100);
	if (e.ExitCode!=0) str=null;
	return (str);
}


function Message(title,text)
	// Show a message dialog.
{
	Dlg(["message",title,text]);
}


function Error(text)
	// Show an error message in a dialog box and exit.
{
	Message("Error",text);
	WScript.Quit(1);
}


function Warning(text)
	// Show a warning message in a dialog box (does not exit).
{
	Message("Warning",text);
}


function Confirm(title,text)
	// Show a message dialog with OK and Cancel buttons. Exits on
	// cancellation.
{
	if (!Dlg(["confirm",title,text])) WScript.Quit(1);
}


function Edit(title,text,initial)
	// Show a dialog for editing a string. Exits on cancellation.
{
	var res;

	res=DlgRead(["edit",title,text,initial]);
	if (res==null) WScript.Quit(1);
	CheckEnvProblem(res);
	return (res);
}


function PasswordEdit(title,text,initial)
	// Like Edit, but for editing a password.
{
	var res;

	res=DlgRead(["pwedit",title,text,initial]);
	if (res==null) WScript.Quit(1);
	return (res);
}


//============================== Selection errors ==============================

function ErrorIfNoSources()
{
	if (Src.length<1) Error("No source selected.");
}


function ErrorIfNoTargets()
{
	if (Tgt.length<1) Error("No target selected.");
}


function ErrorIfMultipleSources()
{
	if (Src.length>1) Error("Multiple sources selected.");
}


function ErrorIfMultipleTargets()
{
	if (Tgt.length>1) Error("Multiple targets selected.");
}


function ErrorIfNotSingleSource()
{
	ErrorIfNoSources();
	ErrorIfMultipleSources();
}


function ErrorIfNotSingleTarget()
{
	ErrorIfNoTargets();
	ErrorIfMultipleTargets();
}


function ErrorIfSourcesNotDirs()
{
	var i;

	for (i=0; i<Src.length; i++) {
		if (!IsDirectory(Src[i])) Error("Non-directory selected as source.");
	}
}


function ErrorIfTargetsNotDirs()
{
	var i;

	for (i=0; i<Tgt.length; i++) {
		if (!IsDirectory(Tgt[i])) Error("Non-directory selected as target.");
	}
}


function ErrorIfSourcesNotFiles()
{
	var i;

	for (i=0; i<Src.length; i++) {
		if (!IsRegularFile(Src[i])) Error("Non-file selected as source.");
	}
}


function ErrorIfTargetsNotFiles()
{
	var i;

	for (i=0; i<Tgt.length; i++) {
		if (!IsRegularFile(Tgt[i])) Error("Non-file selected as target.");
	}
}


function ErrorIfSourcesAccrossDirs()
{
	var d,i;

	if (Src.length>1) {
		d=GetParentPath(Src[0]);
		for (i=1; i<Src.length; i++) {
			if (d!=GetParentPath(Src[i])) {
				Error(
					"Sources selected from different directories."
				);
			}
		}
	}
}


function ErrorIfTargetsAccrossDirs()
{
	var d,i;

	if (Tgt.length>1) {
		d=GetParentPath(Tgt[0]);
		for (i=1; i<Tgt.length; i++) {
			if (d!=GetParentPath(Tgt[i])) {
				Error(
					"Targets selected from different directories."
				);
			}
		}
	}
}


function ErrorIfRootSources()
{
	var i;

	for (i=0; i<Src.length; i++) {
		if (IsRootDirectory(Src[i])) Error("Root directory selected as source.");
	}
}


function ErrorIfRootTargets()
{
	var i;

	for (i=0; i<Tgt.length; i++) {
		if (IsRootDirectory(Tgt[i])) Error("Root directory selected as target.");
	}
}


//========================== Selection confirmations ===========================

function ConfirmIfSourcesAccrossDirs()
{
	var d,i;

	if (Src.length>1) {
		d=GetParentPath(Src[0]);
		for (i=1; i<Src.length; i++) {
			if (d!=GetParentPath(Src[i])) {
				Confirm("Warning",
					"Sources are selected from different directories.\n"+
					"Are you sure this is correct?"
				);
				break;
			}
		}
	}
}


function ConfirmIfTargetsAccrossDirs()
{
	var d,i;

	if (Tgt.length>1) {
		d=GetParentPath(Tgt[0]);
		for (i=1; i<Tgt.length; i++) {
			if (d!=GetParentPath(Tgt[i])) {
				Confirm("Warning",
					"Targets are selected from different directories.\n"+
					"Are you sure this is correct?"
				);
				break;
			}
		}
	}
}


function ConfirmToOpenIfManyTargets()
{
	if (Tgt.length>10) {
		Confirm("Warning",
			"Do you really want to open "+
			Tgt.length+
			" files at once?"
		);
	}
}


//=================== Write and execute temporary batch file ===================

var BatFilePath;
var BatFileHandle;

function BatBegin(title)
{
	var env=WshShell.Environment("PROCESS");
	var acp=env("EM_ACP");
	if (acp.length<=0) Error("EM_ACP not set");
	var tempPath=FileSys.GetSpecialFolder(2).Path;
	for (i=0; ; i++) {
		BatFilePath=GetChildPath(tempPath,"emFmCmdTmp"+i+".bat");
		if (!IsExistingPath(BatFilePath)) {
			BatFileHandle=FileSys.CreateTextFile(BatFilePath);
			break;
		}
	}
	BatWrite(
		"@echo off\n"+
		"color 70\n"+
		"chcp " + acp + "\n"+
		"if errorlevel 1 goto _L_ERROR\n"+
		"title " + title + "\n"+
		"if %1==key892345289 goto _L_START\n"+
		"echo Bad args\n"+
		":_L_ERROR\n"+
		"echo.\n"+
		"echo ERROR!\n"+
		":_L_WAIT_USER\n"+
		"echo.\n"+
		"echo Read the messages, then press enter or close the window.\n"+
		"set /P X=\n"+
		"exit 1\n"+
		":_L_START\n"+
		"set ANY_ERROR=0\n"
	);
}


function BatEnd()
{
	BatWrite(
		"if not %ANY_ERROR%==0 goto _L_ERROR\n"+
		"echo.\n"+
		"echo SUCCESS!\n"+
		"echo.\n"+
		"cscript /nologo " +
			QuoteArg(
				WshShell.ExpandEnvironmentStrings(
					"%EM_DIR%\\res\\emFileMan\\scripts\\msleep.js"
				)
			) +
			" 1000\n"
	);
	if (BatFileHandle) {
		BatFileHandle.Close();
		BatFileHandle=undefined;
	}
	if (BatFilePath) {
		WshShell.Run(CommandFromArgs(["cmd","/C",BatFilePath,"key892345289"]),1,true);
		FileSys.GetFile(BatFilePath).Delete(true);
		BatFilePath=undefined;
	}
}


function BatAbort()
{
	if (BatFileHandle) {
		BatFileHandle.Close();
		BatFileHandle=undefined;
	}
	if (BatFilePath) {
		FileSys.GetFile(BatFilePath).Delete(true);
		BatFilePath=undefined;
	}
}


function BatWrite(str)
{
	if (BatFileHandle) BatFileHandle.Write(str);
}


function BatWriteLine(line)
{
	BatWrite(line+"\n");
}


function BatWriteLineEchoed(line)
{
	BatWriteLine("echo.");
	if (line.indexOf('>')>=0 || line.indexOf('<')>=0 || line.indexOf('|')>=0) {
		BatWriteLine("echo Running: \"" + line + "\"");
	}
	else {
		BatWriteLine("echo Running: " + line);
	}
	BatWriteLine("echo.");
	BatWriteLine(line);
}


function BatWriteCmd(argsArray)
{
	BatWriteLine(CommandFromArgs(argsArray));
}


function BatWriteCmdEchoed(argsArray)
{
	BatWriteLineEchoed(CommandFromArgs(argsArray));
}


function BatWriteCheckError()
{
	BatWriteLine("if errorlevel 1 set ANY_ERROR=1");
}


function BatWriteSetErrored()
{
	BatWriteLine("set ANY_ERROR=1");
}


function BatWriteExitWithError()
{
	BatWriteLine("goto _L_ERROR");
}


function BatWriteExitByUser()
{

	BatWrite(
		"if not %ANY_ERROR%==0 goto _L_ERROR\n"+
		"goto _L_WAIT_USER\n"
	);
}


function BatWriteSend(cmd,files)
{
	var env=WshShell.Environment("PROCESS");
	var args=new Array;
	args[0]=env("EM_DIR") + "\\bin\\emSendMiniIpc.exe";
	args[1]=env("EM_FM_SERVER_NAME");
	args[2]=cmd;
	if (files != null) {
		args[3]=env("EM_COMMAND_RUN_ID");
		args=args.concat(files);
	}
	BatWriteCmd(args);
}


function BatWriteSendUpdate()
	// Require Eagle Mode to reload changed files and directories.
{
	BatWriteSend("update",null);
}


function BatWriteSendSelect(files)
	// Require Eagle Mode to select other targets. It will even reload
	// changed files and directories. As usual, the source selection is set
	// from the old target selection.
{
	BatWriteSend("select",files);
}


function BatWriteSendSelectKS(files)
	// Like BatWriteSendSelect, but the source selection is not modified.
{
	BatWriteSend("selectks",files);
}


function BatWriteSendSelectCS(files)
	// Like BatWriteSendSelect, but the source selection is cleared.
{
	BatWriteSend("selectcs",files);
}


function BatWritePack(format,archive,dir,names)
{
	BatWriteCmdEchoed(['cd','/D',dir]);
	BatWriteLine("if errorlevel 1 goto _L_PACK_ERROR");
	BatWriteLine("if exist " + QuoteArg(archive) + " (");
	BatWriteCmdEchoed(["del","/F","/Q",archive]);
	BatWriteLine("if errorlevel 1 goto _L_PACK_ERROR");
	BatWriteLine(")");
	var f='x.'+format;
	if (TestEnding(f,'7z')) {
		if (!HasAnyEnding(archive)) {
			BatAbort();
			Error("Archive file name has no suffix (e.g. \".7z\").");
			// Otherwise 7z automatically appends ".7z" (would
			// break the semantics of this script interface).
		}
		BatWriteCmdEchoed(ConcatArrays(['7za','a',archive],names));
	}
	else if (TestEnding(f,'arc')) {
		if (!HasAnyEnding(archive) || !IsDosFilename(GetNameInPath(archive))) {
			BatAbort();
			Error("arc archive file name must be a DOS file name (<max 8 non-dot chars><dot><max 3 chars>).");
			// Otherwise arc automatically shortens the file name and
			// possibly appends ".arc" (would break the semantics of this
			// script interface).
		}
		for (var i=0; i<names.length; i++) {
			if (IsDirectory(GetChildPath(dir,names[i]))) {
				BatAbort();
				Error("Cannot pack a directory with arc.");
			}
		}
		BatWriteCmdEchoed(ConcatArrays(['arc','a',archive],names));
	}
	else if (TestEnding(f,'arj')) {
		if (!HasAnyEnding(archive)) {
			BatAbort();
			Error("Archive file name has no suffix (e.g. \".arj\").");
			// Otherwise arj automatically appends ".arj" (would
			// break the semantics of this script interface).
		}
		for (var i=0; i<names.length; i++) {
			BatWriteCmdEchoed(['arj','a','-r',archive,names[i]]);
			BatWriteLine("if errorlevel 1 goto _L_PACK_ERROR");
		}
	}
	else if (TestEnding(f,'lzh') || TestEnding(f,'lha')) {
		BatWriteCmdEchoed(ConcatArrays(['lha','av',archive],names));
	}
	else if (TestEnding(f,'tar')) {
		BatWriteLineEchoed("tar cvf - "+CommandFromArgs(names)+" > "+QuoteArg(archive));
	}
	else if (TestEnding(f,'tar.bz2') || TestEnding(f,'tbz2') ||
	         TestEnding(f,'tgj')) {
		BatWriteLineEchoed("tar cvf - "+CommandFromArgs(names)+" | bzip2 -c > "+QuoteArg(archive));
	}
	else if (TestEnding(f,'tar.gz') || TestEnding(f,'tgz')) {
		BatWriteLineEchoed("tar cvf - "+CommandFromArgs(names)+" | gzip -c > "+QuoteArg(archive));
	}
	else if (TestEnding(f,'tar.lzma') || TestEnding(f,'tlz')) {
		BatWriteLineEchoed("tar cvf - "+CommandFromArgs(names)+" | xz --stdout --format=lzma > "+QuoteArg(archive));
	}
	else if (TestEnding(f,'tar.xz') || TestEnding(f,'txz')) {
		BatWriteLineEchoed("tar cvf - "+CommandFromArgs(names)+" | xz --stdout > "+QuoteArg(archive));
	}
	else if (TestEnding(f,'zip') || TestEnding(f,'jar')) {
		if (!HasAnyEnding(archive)) {
			BatAbort();
			Error("Archive file name has no suffix (e.g. \".zip\").");
			// Otherwise zip automatically appends ".zip" (would
			// break the semantics of this script interface).
		}
		BatWriteCmdEchoed(ConcatArrays(['zip','-r','-9',archive],names));
	}
	else if (TestEnding(f,'bz2')) {
		if (names.length>1) {
			BatAbort();
			Error("Cannot pack multiple files with bzip2.");
		}
		if (IsDirectory(GetChildPath(dir,names[0]))) {
			BatAbort();
			Error("Cannot pack a directory with bzip2.");
		}
		BatWriteLineEchoed("bzip2 -c "+QuoteArg(names[0])+" > "+QuoteArg(archive));
	}
	else if (TestEnding(f,'gz')) {
		if (names.length>1) {
			BatAbort();
			Error("Cannot pack multiple files with gzip.");
		}
		if (IsDirectory(GetChildPath(dir,names[0]))) {
			BatAbort();
			Error("Cannot pack a directory with gzip.");
		}
		BatWriteLineEchoed("gzip -c "+QuoteArg(names[0])+" > "+QuoteArg(archive));
	}
	else if (TestEnding(f,'lzma')) {
		if (names.length>1) {
			BatAbort();
			Error("Cannot pack multiple files with lzma.");
		}
		if (IsDirectory(GetChildPath(dir,names[0]))) {
			BatAbort();
			Error("Cannot pack a directory with lzma.");
		}
		BatWriteLineEchoed("xz --stdout --format=lzma "+QuoteArg(names[0])+" > "+QuoteArg(archive));
	}
	else if (TestEnding(f,'xz')) {
		if (names.length>1) {
			BatAbort();
			Error("Cannot pack multiple files with xz.");
		}
		if (IsDirectory(GetChildPath(dir,names[0]))) {
			BatAbort();
			Error("Cannot pack a directory with xz.");
		}
		BatWriteLineEchoed("xz --stdout "+QuoteArg(names[0])+" > "+QuoteArg(archive));
	}
	else {
		BatAbort();
		Error("Packing of "+format+" not supported");
	}
	BatWriteCheckError();
	BatWriteLine("goto _L_PACK_END");
	BatWriteLine(":_L_PACK_ERROR");
	BatWriteSetErrored();
	BatWriteLine(":_L_PACK_END");
}


function BatWriteUnpack(format,archive,dir)
{
	BatWriteCmdEchoed(['cd','/D',dir]);
	BatWriteLine("if errorlevel 1 (");
	BatWriteSetErrored();
	BatWriteLine("goto _L_UNPACK_END");
	BatWriteLine(")");
	var f='x.'+format;
	if (TestEnding(f,'7z')) {
		BatWriteCmdEchoed(['7za','x',archive]);
	}
	else if (TestEnding(f,'arc')) {
		BatWriteCmdEchoed(['arc','x',archive]);
	}
	else if (TestEnding(f,'arj')) {
		BatWriteCmdEchoed(['arj','x','-y',archive]);
	}
	else if (TestEnding(f,'lzh') || TestEnding(f,'lha')) {
		BatWriteCmdEchoed(['lha','xv',archive]);
	}
	else if (TestEnding(f,'tar')) {
		BatWriteLineEchoed("tar xvf - < "+QuoteArg(archive));
	}
	else if (TestEnding(f,'tar.bz2') || TestEnding(f,'tbz2') ||
	         TestEnding(f,'tgj') || TestEnding(f,'tar.bz') ||
	         TestEnding(f,'tbz')) {
		BatWriteLineEchoed("bzip2 -d -c < "+QuoteArg(archive)+" | tar xvf -");
	}
	else if (TestEnding(f,'tar.gz') || TestEnding(f,'tgz') ||
	         TestEnding(f,'tar.z') || TestEnding(f,'taz')) {
		BatWriteLineEchoed("gzip -d -c < "+QuoteArg(archive)+" | tar xvf -");
	}
	else if (TestEnding(f,'tar.lzma') || TestEnding(f,'tlz') ||
	         TestEnding(f,'tar.xz') || TestEnding(f,'txz')) {
		BatWriteLineEchoed("xz --decompress --stdout < "+QuoteArg(archive)+" | tar xvf -");
	}
	else if (TestEnding(f,'zip') || TestEnding(f,'jar')) {
		BatWriteCmdEchoed(['unzip',archive]);
	}
	else if (TestEnding(f,'bz2') || TestEnding(f,'bz')) {
		var n=GetNameInPath(archive);
		var e='';
		i=n.lastIndexOf('.');
		if (i>0) {
			e=n.substr(i);
			n=n.substr(0,i);
		}
		if (e.toLowerCase()=='.bz2') e='';
		else if (e.toLowerCase()=='.bz') e='';
		else if (e.toLowerCase()=='.tbz2') e='.tar';
		else if (e.toLowerCase()=='.tbz') e='.tar';
		else if (e.toLowerCase()=='.tgz')  e='.tar';
		else e=e+'.unpacked';
		n+=e;
		if (IsExistingPath(n)) {
			BatAbort();
			Error("File already exists: "+n);
		}
		BatWriteLineEchoed("bzip2 -d -c < "+QuoteArg(archive)+" > "+QuoteArg(n));
	}
	else if (TestEnding(f,'gz') || TestEnding(f,'z')) {
		var n=GetNameInPath(archive);
		var e='';
		i=n.lastIndexOf('.');
		if (i>0) {
			e=n.substr(i);
			n=n.substr(0,i);
		}
		if (e.toLowerCase()=='.gz') e='';
		else if (e.toLowerCase()=='.z') e='';
		else if (e.toLowerCase()=='.tgz') e='.tar';
		else if (e.toLowerCase()=='.taz') e='.tar';
		else e=e+'.unpacked';
		n+=e;
		if (IsExistingPath(n)) {
			BatAbort();
			Error("File already exists: "+n);
		}
		BatWriteLineEchoed("gzip -d -c < "+QuoteArg(archive)+" > "+QuoteArg(n));
	}
	else if (TestEnding(f,'lzma') || TestEnding(f,'xz')) {
		var n=GetNameInPath(archive);
		var e='';
		i=n.lastIndexOf('.');
		if (i>0) {
			e=n.substr(i);
			n=n.substr(0,i);
		}
		if (e.toLowerCase()=='.lzma') e='';
		else if (e.toLowerCase()=='.xz') e='';
		else if (e.toLowerCase()=='.tlz') e='.tar';
		else if (e.toLowerCase()=='.txz') e='.tar';
		else e=e+'.unpacked';
		n+=e;
		if (IsExistingPath(n)) {
			BatAbort();
			Error("File already exists: "+n);
		}
		BatWriteLineEchoed("xz --decompress --stdout < "+QuoteArg(archive)+" > "+QuoteArg(n));
	}
	else {
		BatAbort();
		Error("Unpacking of "+format+" not supported");
	}
	BatWriteCheckError();
	BatWriteLine(":_L_UNPACK_END");
}


//=================== Hi-level functions for frequent cases ====================

function OpenSingleTargetFileWith(argsArray)
{
	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotFiles();
	WshShell.CurrentDirectory=GetParentPath(Tgt[0]);
	RestoreOrigPath();
	WshShell.Run(CommandFromArgs(argsArray.concat(Tgt)));
	WScript.Quit(0);
}


function OpenSingleTargetDirWith(argsArray)
{
	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotDirs();
	RestoreOrigPath();
	WshShell.Run(CommandFromArgs(argsArray.concat(Tgt)));
	WScript.Quit(0);
}


function OpenSingleTargetWith(argsArray)
{
	ErrorIfNotSingleTarget();
	RestoreOrigPath();
	WshShell.Run(CommandFromArgs(argsArray.concat(Tgt)));
	WScript.Quit(0);
}


function OpenTargetFilesWith(argsArray)
{
	ErrorIfNoTargets();
	ErrorIfTargetsNotFiles();
	ConfirmToOpenIfManyTargets();
	WshShell.CurrentDirectory=GetParentPath(Tgt[0]);
	RestoreOrigPath();
	WshShell.Run(CommandFromArgs(argsArray.concat(Tgt)));
	WScript.Quit(0);
}


function OpenTargetDirsWith(argsArray)
{
	ErrorIfNoTargets();
	ErrorIfTargetsNotDirs();
	ConfirmToOpenIfManyTargets();
	RestoreOrigPath();
	WshShell.Run(CommandFromArgs(argsArray.concat(Tgt)));
	WScript.Quit(0);
}


function OpenTargetsWith(argsArray)
{
	ErrorIfNoTargets();
	ConfirmToOpenIfManyTargets();
	RestoreOrigPath();
	WshShell.Run(CommandFromArgs(argsArray.concat(Tgt)));
	WScript.Quit(0);
}


function PackType(type)
{
	ErrorIfNoSources();
	ErrorIfSourcesAccrossDirs();
	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotDirs();

	var srcDir=GetParentPath(Src[0]);

	var srcNames=new Array;
	for (var i=0; i<Src.length; i++) srcNames[i]=GetNameInPath(Src[i]);

	var name = "archive";
	if (Src.length == 1) name = srcNames[0];
	name = name + '.' + type;

	name=Edit(
		"Pack "+type,
		"Please enter a name for the new "+type+" archive in:\n\n"+Tgt[0],
		name
	);

	var path=GetChildPath(Tgt[0],name);
	if (IsExistingPath(path)) {
		Error("A file or directory of that name already exists.");
	}

	BatBegin("Pack "+type);
	BatWritePack(type,path,srcDir,srcNames);
	BatWriteLine("if exist " + QuoteArg(path) + " (");
	BatWriteSendSelect(path);
	BatWriteLine(") else (");
	BatWriteSetErrored();
	BatWriteSendUpdate();
	BatWriteLine(")");
	BatEnd();
}
