/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 5.0
# Interpreter = wscript
# Caption = Move
# Descr =Move one or more files and/or directories into another
# Descr =directory.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The files and directories to be moved.
# Descr =
# Descr =  Target: The target directory, or a single target file
# Descr =          to be overwritten by a single source file.
# ButtonBgColor = #BB5
# ButtonFgColor = #000
# Hotkey = Ctrl+M
#[[END PROPERTIES]]
*/

var FileSys=WScript.CreateObject("Scripting.FileSystemObject");
var WshShell=WScript.CreateObject("WScript.Shell");
var incFile=FileSys.OpenTextFile(WshShell.ExpandEnvironmentStrings(
	"%EM_DIR%\\res\\emFileMan\\scripts\\cmd-util.js"
));
eval(incFile.ReadAll());
incFile.Close();

ErrorIfNoSources();
ErrorIfNotSingleTarget();
ErrorIfRootSources();

if (IsRegularFile(Tgt[0])) {
	if (Src.length>1) {
		Error("The source is not a single file but the target is.");
	}
	else if (IsDirectory(Src[0])) {
		Error("The source is a directory but the target is a file.");
	}
}

ConfirmIfSourcesAccrossDirs();

var message=
	"Are you sure to move, overwriting any existing target files?\n"+
	"\n"+
	"From:\n"+
	GetSrcListing()+
	"\n"+
	"To:\n"+
	GetTgtListing()
;
Confirm("Move",message);

var newTgt=new Array;
for (var i=0; i<Src.length; i++) {
	newTgt[i]=GetChildPath(Tgt[0],GetNameInPath(Src[i]));
}

BatBegin("Move");
for (var i=0; i<Src.length; i++) {
	BatWriteCmdEchoed(["move","/Y",Src[i],Tgt[0]]); //??? Fails on hidden files.
	BatWriteLine("if errorlevel 1 (");
	BatWriteSetErrored();
	BatWriteLine("goto L_END");
	BatWriteLine(")");
}
BatWriteLine(":L_END");
if (IsDirectory(Tgt[0])) {
	BatWriteLine("if exist " + QuoteArg(newTgt[0]) + " (");
	BatWriteSendSelectKS(newTgt);
	BatWriteLine(") else (");
	BatWriteSendUpdate();
	BatWriteLine(")");
}
else {
	BatWriteSendUpdate();
}
BatEnd();
