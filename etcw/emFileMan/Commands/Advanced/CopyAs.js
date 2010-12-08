/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 1.0
# Interpreter = wscript
# Caption = Copy As
# Descr =Copy a single file or directory into any directory while giving
# Descr =the copy another name. The name is asked. Directories are copied
# Descr =recursively.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The file or directory to be copied.
# Descr =
# Descr =  Target: The target directory for the copy.
# ButtonBgColor = #7A8
# ButtonFgColor = #000
# Hotkey = Meta+C
#[[END PROPERTIES]]
*/

var FileSys=WScript.CreateObject("Scripting.FileSystemObject");
var WshShell=WScript.CreateObject("WScript.Shell");
var incFile=FileSys.OpenTextFile(WshShell.ExpandEnvironmentStrings(
	"%EM_DIR%\\res\\emFileMan\\scripts\\cmd-util.js"
));
eval(incFile.ReadAll());
incFile.Close();

ErrorIfNotSingleSource();
ErrorIfNotSingleTarget();
ErrorIfTargetsNotDirs();

var oldPath=Src[0];
var oldName=GetNameInPath(oldPath);
var newDir=Tgt[0];

var newName=Edit(
	"Copy As",
	"Please enter a name for a copy of\n\n  " + oldPath + "\n\nin\n\n  " + newDir,
	oldName
);
CheckFilename(newName);

var newPath=GetChildPath(newDir,newName);

if (IsExistingPath(newPath)) {
	Error("A file or directory with that name already exists.");
}

BatBegin("Copy As");
if (IsDirectory(oldPath)) {
	BatWriteCmdEchoed(["xcopy","/F","/H","/K","/Y","/E","/I",oldPath,newPath]);
	BatWriteCheckError();
	BatWriteLine("if exist " + QuoteArg(newPath) + " (");
	BatWriteSendSelectKS([newPath]);
	BatWriteLine(") else (");
	BatWriteSendUpdate();
	BatWriteLine(")");
}
else {
	BatWriteLineEchoed("<nul (set /p X=) > " + QuoteArg(newPath));
	BatWriteLine("if exist " + QuoteArg(newPath) + " (");
	BatWriteCmdEchoed(["xcopy","/F","/H","/K","/Y",oldPath,newPath]);
	BatWriteCheckError();
	BatWriteSendSelectKS([newPath]);
	BatWriteLine(") else (");
	BatWriteSetErrored();
	BatWriteSendUpdate();
	BatWriteLine(")");
}
BatEnd();
