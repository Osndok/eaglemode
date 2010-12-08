/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = wscript
# Caption = Copy
# Descr =Copy one or more files and/or directories into another
# Descr =directory. Directories are copied recursively.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The files and directories to be copied.
# Descr =
# Descr =  Target: The target directory, or a single target file
# Descr =          to be overwritten by a single source file.
# ButtonBgColor = #5A7
# ButtonFgColor = #000
# Hotkey = Ctrl+C
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
	"Are you sure to copy, overwriting any existing target files?\n"+
	"\n"+
	"From:\n"+
	GetSrcListing()+
	"\n"+
	"To:\n"+
	GetTgtListing()
;
Confirm("Copy",message);

var newTgt=new Array;
for (var i=0; i<Src.length; i++) {
	newTgt[i]=GetChildPath(Tgt[0],GetNameInPath(Src[i]));
}

BatBegin("Copy");
for (var i=0; i<Src.length; i++) {
	if (IsDirectory(Src[i])) {
		if (IsRegularFile(newTgt[i])) {
			BatWriteCmdEchoed(["del","/F","/Q",newTgt[i]]);
		}
		BatWriteCmdEchoed(["xcopy","/F","/H","/K","/Y","/E","/I",Src[i],newTgt[i]]);
	}
	else {
		BatWriteCmdEchoed(["xcopy","/F","/H","/K","/Y",Src[i],Tgt[0]]);
	}
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
