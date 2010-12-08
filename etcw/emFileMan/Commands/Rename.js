/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 6.0
# Interpreter = wscript
# Caption = Rename
# Descr =Rename a file or directory. The new name is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The file or directory to be renamed.
# ButtonBgColor = #C85
# ButtonFgColor = #000
# Hotkey = Backspace
#[[END PROPERTIES]]
*/

var FileSys=WScript.CreateObject("Scripting.FileSystemObject");
var WshShell=WScript.CreateObject("WScript.Shell");
var incFile=FileSys.OpenTextFile(WshShell.ExpandEnvironmentStrings(
	"%EM_DIR%\\res\\emFileMan\\scripts\\cmd-util.js"
));
eval(incFile.ReadAll());
incFile.Close();

ErrorIfNotSingleTarget();
ErrorIfRootTargets();

var oldPath=Tgt[0];
var dir=GetParentPath(oldPath);
var oldName=GetNameInPath(oldPath);

var newName=Edit(
	"Rename",
	"Please enter a new name for:\n\n" + oldPath,
	oldName
);
CheckFilename(newName);

if (oldName == newName) WScript.Quit(0);
var newPath=GetChildPath(dir,newName);
if (IsExistingPath(newPath)) {
	Error("A file or directory with that name already exists.");
}

BatBegin("Rename");
BatWriteCmdEchoed(["rename",oldPath,newName]); //??? Fails on hidden files.
BatWriteCheckError();
BatWriteLine("if exist " + QuoteArg(newPath) + " (");
BatWriteSendSelectKS([newPath]);
BatWriteLine(") else (");
BatWriteSendUpdate();
BatWriteLine(")");
BatEnd();
