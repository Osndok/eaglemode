//------------------------------------------------------------------------------
// cmd-util.js
//
// Copyright (C) 2008-2012,2016,2018-2019,2021-2022 Oliver Hamann.
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


//=============================== Configuration ================================

// Detect Windows version
var WinVersion;
var CurVerRegPath="HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
try {
	WinVersion=WshShell.RegRead(CurVerRegPath+"\\CurrentMajorVersionNumber");
}
catch (e) {
	try {
		WinVersion=WshShell.RegRead(CurVerRegPath+"\\CurrentVersion");
	}
	catch (e) {
		WinVersion=0;
	}
}

// Terminal colors.
var TermBgFg ='70';
var TCNormal =(WinVersion>=10 ? "\033[30m" : "");
var TCInfo   =(WinVersion>=10 ? "\033[34m" : "");
var TCSuccess=(WinVersion>=10 ? "\033[32m" : "");
var TCError  =(WinVersion>=10 ? "\033[31m" : "");
var TCClose  =(WinVersion>=10 ? "\033[97m" : "");


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

function BasicQuoteArg(arg)
{
	var str,res,i,k,c;

	str=arg.toString();
	for (i=str.length-1; i>=0; i--) {
		c=str.charAt(i);
		if ((c<'0' || c>'9') && (c<'A' || c>'Z') && (c<'a' || c>'z') &&
		    c!='\\' && c!='/' && c!='.' && c!=':' && c!='-') break;
	}
	if (i<0 && str.length>0) {
		res=str;
	}
	else {
		res='"';
		k=0;
		for (i=0; i<str.length; i++) {
			c=str.charAt(i);
			if (c=='"') {
				while (k>0) { res+='\\'; k--; }
				res+='\\';
			}
			else if (c=='\\') k++;
			else k=0;
			res+=c;
		}
		while (k>0) { res+='\\'; k--; }
		res+='"';
	}
	return (res);
}


function BatQuoteArg(arg)
{
	var str,i;

	str=BasicQuoteArg(arg);
	for (i=str.length-1; i>=0; i--) {
		if (str.charAt(i)=='%') {
			str=str.substr(0,i)+'%'+str.substr(i);
		}
	}
	return (str);
}


function BatQuoteForEcho(msg)
{
	var str,res,i,c;

	str=msg.toString();
	res="";
	for (i=0; i<str.length; i++) {
		c=str.charAt(i);
		if (c=='%') {
			res+='%';
		}
		else if (
			c=='!' || c=='"' || c=='&' || c=='(' || c==')' || c==',' || c==';' ||
			c=='<' || c=='=' || c=='>' || c=='[' || c=='\'' || c==']' || c=='^' ||
			c=='`' || c=='{' || c=='|' || c=='}' || c=='~'
		) {
			res+='^';
		}
		res+=c;
	}
	return (res);
}


function WshShellQuoteArg(arg)
{
	var str,i;

	str=BasicQuoteArg(arg);
	for (i=str.length-1; i>=0; i--) {
		if (str.charAt(i)=='%') {
			var env=WshShell.Environment("PROCESS");
			if (env("PERCENT_SIGN")!="%") env("PERCENT_SIGN")="%";
			str=str.substr(0,i)+'%PERCENT_SIGN'+str.substr(i);
		}
	}
	return (str);
}


function BatCmdFromArgs(argsArray)
{
	var cmd,i;

	cmd="";
	for (i=0; i<argsArray.length; i++) {
		if (i>0) cmd+=" ";
		cmd+=BatQuoteArg(argsArray[i]);
	}
	return (cmd);
}


function WshShellCmdFromArgs(argsArray)
{
	var cmd,i;

	cmd="";
	for (i=0; i<argsArray.length; i++) {
		if (i>0) cmd+=" ";
		cmd+=WshShellQuoteArg(argsArray[i]);
	}
	return (cmd);
}


function CheckFilename(name)
{
	if (/["*?:<>/|\\]/.test(name)) {
		Error(
			"File names must not contain any of these\n"+
			"characters: \" *  ? : < > / | \\"
		);
	}
	if (/^\.?\.?$/.test(name)) {
		Error("File names must not be empty or consist of just one or two periods.");
	}
	if (/[ .]$/.test(name)) {
		Error("File names must not end with a space or a period.");
	}
	if (/^(aux|(com[0-9])|con|(lpt[0-9])|nul|prn)($|\.)/i.test(name)) {
		Error("The file name is not allowed because it matches a device name.");
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
	var ret=WshShell.Run(WshShellCmdFromArgs(args),1,true);
	if (ret != 0) {
		// emSendMiniIpc itself failed. If the arg list would be too
		// long, WshShell.Run prints an error and exits.
		if (cmd == "update") {
			Warning(
				"Failed to perform update command with emSendMiniIpc.\n"+
				"\n"+
				"This means that the operation may be successful but\n"+
				"the Eagle Mode view could not be updated."
			);
		}
		else {
			Warning(
				"Failed to perform select command with emSendMiniIpc.\n"+
				"\n"+
				"This means that the operation may be successful but\n"+
				"the selection in Eagle Mode could not be updated."
			);
			// At least try an update:
			SendUpdate();
		}
	}
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
	var env,minW,minH,x,y,w,h,allArgs;

	env=WshShell.Environment("PROCESS");

	minW=400;
	minH=280;
	w=parseInt(env("EM_WIDTH"))*0.5;
	h=parseInt(env("EM_HEIGHT"))*0.4;
	if (w>h*minW/minH) w=h*minW/minH;
	if (w<minW) w=minW;
	w=Math.round(w);
	h=Math.round(w*minH/minW);
	x=Math.round(parseInt(env("EM_X"))+(parseInt(env("EM_WIDTH"))-w)/2);
	y=Math.round(parseInt(env("EM_Y"))+(parseInt(env("EM_HEIGHT"))-h)/2);
	if (x<0) x=0;
	if (y<0) y=0;

	allArgs=new Array;
	allArgs[0]=env("EM_DIR") + "\\bin\\emShowStdDlg.exe";
	allArgs[1]="-geometry";
	allArgs[2]=w+"x"+h+"+"+x+"+"+y;
	for (i=0; i<argsArray.length; i++) allArgs[3+i]=argsArray[i];
	return (WshShell.Exec(WshShellCmdFromArgs(allArgs)));
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
	if (/[\x00-\x1F\x7F]/.test(res)) {
		Error("The edited text contains a control character. That is not allowed.");
	}
	return (res);
}


function FilenameEdit(title,text,initial)
	// Like Edit, but for editing a file or directory name.
{
	var name=Edit(title,text,initial);
	CheckFilename(name);
	return (name);
}


function PasswordEdit(title,text,initial)
	// Like Edit, but for editing a password.
{
	var res;

	res=DlgRead(["pwedit",title,text,initial]);
	if (res==null) WScript.Quit(1);
	if (/[\x00-\x1F\x7F]/.test(res)) {
		Error("The password contains a control character. That is not allowed.");
	}
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


function ErrorIfSourcesAcrossDirs()
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


function ErrorIfTargetsAcrossDirs()
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

function ConfirmIfSourcesAcrossDirs()
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


function ConfirmIfTargetsAcrossDirs()
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
		"color "+TermBgFg+"\n"+
		"chcp " + acp + "\n"+
		"if errorlevel 1 goto _L_ERROR\n"+
		"title " + title + "\n"+
		"cls\n"+
		"if %1==key892345289 goto _L_START\n"+
		"echo Bad args\n"+
		":_L_ERROR\n"+
		"echo.\n"+
		"echo "+TCError+"ERROR!"+TCNormal+"\n"+
		":_L_WAIT_USER\n"+
		"echo.\n"+
		"echo "+TCClose+"Read the messages, then press enter or close the window."+TCNormal+"\n"+
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
		"echo "+TCSuccess+"SUCCESS!"+TCNormal+"\n"+
		"echo.\n"+
		"cscript " +
			BatQuoteArg(
				WshShell.ExpandEnvironmentStrings(
					"%EM_DIR%\\res\\emFileMan\\scripts\\msleep.js"
				)
			) +
			" //Nologo 1000\n"
	);
	if (BatFileHandle) {
		BatFileHandle.Close();
		BatFileHandle=undefined;
	}
	if (BatFilePath) {
		WshShell.Run(WshShellCmdFromArgs(
			["cmd","/E:ON","/F:OFF","/V:OFF","/C",BatFilePath,"key892345289"]
		),1,true);
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
	if (BatFileHandle) {
		// Line breaks must have carraige returns. Otherwise cmd.exe
		// misinterprets UTF-8 bat (cp 65001) in a very dangerous way
		// (seen in a test on Windows 10).
		var strCrLf=str.replace(/[\n]/g,"\r\n").replace(/[\r][\r]/g,"\r");
		BatFileHandle.Write(strCrLf);
	}
}


function BatWriteLine(line)
{
	BatWrite(line+"\n");
}


function BatWriteLineEchoed(line)
{
	BatWriteLine("echo.");
	BatWriteLine("echo " + TCInfo + BatQuoteForEcho("Running: " + line) + TCNormal);
	BatWriteLine("echo.");
	BatWriteLine(line);
}


function BatWriteCmd(argsArray)
{
	BatWriteLine(BatCmdFromArgs(argsArray));
}


function BatWriteCmdEchoed(argsArray)
{
	BatWriteLineEchoed(BatCmdFromArgs(argsArray));
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
	BatWriteLine("if errorlevel 1 (");
	BatWriteSetErrored();
	BatWriteLine("echo.");
	if (cmd == "update") {
		BatWriteLine(
			"echo ERROR: Failed to perform update command with emSendMiniIpc."+
			" This means that the operation may be successful but"+
			" the Eagle Mode view could not be updated."
		);
	}
	else {
		BatWriteLine(
			"echo ERROR: Failed to perform select command with emSendMiniIpc."+
			" This means that the operation may be successful but"+
			" the selection in Eagle Mode could not be updated."
		);
		// Maybe arg list too long. At least try an update:
		BatWriteSendUpdate();
	}
	BatWriteLine(")");
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
	BatWriteLine("if errorlevel 1 (");
	BatWriteSetErrored();
	BatWriteLine("goto _L_PACK_END");
	BatWriteLine(")");
	BatWriteCmdEchoed(
		ConcatArrays(
			[
				"cscript",
				WshShell.ExpandEnvironmentStrings(
					"%EM_DIR%\\res\\emFileMan\\scripts\\emArch.js"
				),
				"//Nologo",
				"pack",
				"-f",
				format,
				"--",
				archive
			],
			names
		)
	);
	BatWriteCheckError();
	BatWriteLine(":_L_PACK_END");
}


function BatWriteUnpack(format,archive,dir)
{
	BatWriteCmdEchoed(['cd','/D',dir]);
	BatWriteLine("if errorlevel 1 (");
	BatWriteSetErrored();
	BatWriteLine("goto _L_UNPACK_END");
	BatWriteLine(")");
	BatWriteCmdEchoed(
		[
			"cscript",
			WshShell.ExpandEnvironmentStrings(
				"%EM_DIR%\\res\\emFileMan\\scripts\\emArch.js"
			),
			"//Nologo",
			"unpack",
			"-f",
			format,
			"--",
			archive
		]
	);
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
	WshShell.Run(WshShellCmdFromArgs(argsArray.concat(Tgt)));
	WScript.Quit(0);
}


function OpenSingleTargetDirWith(argsArray)
{
	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotDirs();
	RestoreOrigPath();
	WshShell.Run(WshShellCmdFromArgs(argsArray.concat(Tgt)));
	WScript.Quit(0);
}


function OpenSingleTargetWith(argsArray)
{
	ErrorIfNotSingleTarget();
	RestoreOrigPath();
	WshShell.Run(WshShellCmdFromArgs(argsArray.concat(Tgt)));
	WScript.Quit(0);
}


function OpenTargetFilesWith(argsArray)
{
	ErrorIfNoTargets();
	ErrorIfTargetsNotFiles();
	ConfirmToOpenIfManyTargets();
	WshShell.CurrentDirectory=GetParentPath(Tgt[0]);
	RestoreOrigPath();
	WshShell.Run(WshShellCmdFromArgs(argsArray.concat(Tgt)));
	WScript.Quit(0);
}


function OpenTargetDirsWith(argsArray)
{
	ErrorIfNoTargets();
	ErrorIfTargetsNotDirs();
	ConfirmToOpenIfManyTargets();
	RestoreOrigPath();
	WshShell.Run(WshShellCmdFromArgs(argsArray.concat(Tgt)));
	WScript.Quit(0);
}


function OpenTargetsWith(argsArray)
{
	ErrorIfNoTargets();
	ConfirmToOpenIfManyTargets();
	RestoreOrigPath();
	WshShell.Run(WshShellCmdFromArgs(argsArray.concat(Tgt)));
	WScript.Quit(0);
}


function PackType(type)
{
	ErrorIfNoSources();
	ErrorIfSourcesAcrossDirs();
	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotDirs();

	var srcDir=GetParentPath(Src[0]);

	var srcNames=new Array;
	for (var i=0; i<Src.length; i++) srcNames[i]=GetNameInPath(Src[i]);

	var name = "archive";
	if (Src.length == 1) name = srcNames[0];
	name = name + '.' + type;

	name=FilenameEdit(
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
	BatWriteLine("if exist " + BatQuoteArg(path) + " (");
	BatWriteSendSelect(path);
	BatWriteLine(") else (");
	BatWriteSetErrored();
	BatWriteSendUpdate();
	BatWriteLine(")");
	BatEnd();
}
