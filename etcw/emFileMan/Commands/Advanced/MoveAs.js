/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 2.0
# Interpreter = wscript
# Caption = Move As
# Descr =Move a single file or directory into any directory while giving it
# Descr =another name. The name is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The file or directory to be moved and renamed.
# Descr =
# Descr =  Target: The target directory into which the file or directory
# Descr =          shall be moved.
# ButtonBgColor = #BB8
# Hotkey = Meta+M
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
ErrorIfRootSources();

var oldPath=Src[0];
var oldName=GetNameInPath(oldPath);
var newDir=Tgt[0];

var newName=Edit(
	"Move As",
	"Please enter a name for a movement of\n\n  " + oldPath + "\n\ninto\n\n  " + newDir,
	oldName
);
CheckFilename(newName);

var newPath=GetChildPath(newDir,newName);

if (IsExistingPath(newPath)) {
	Error("A file or directory with that name already exists.");
}

BatBegin("Move As");
BatWriteCmdEchoed(["move","/Y",oldPath,newPath]);
BatWriteCheckError();
BatWriteLine("if exist " + QuoteArg(newPath) + " (");
BatWriteSendSelectKS([newPath]);
BatWriteLine(") else (");
BatWriteSendUpdate();
BatWriteLine(")");
BatEnd();
