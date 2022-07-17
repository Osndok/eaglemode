/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 9.0
# Interpreter = wscript
# Caption = Rename
# Descr =Rename a file or directory. The new name is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The file or directory to be renamed.
# Icon = rename_file.tga
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

var newName=FilenameEdit(
	"Rename",
	"Please enter a new name for:\n\n" + oldPath,
	oldName
);

if (oldName == newName) WScript.Quit(0);
var newPath=GetChildPath(dir,newName);
if (IsExistingPath(newPath) && oldName.toUpperCase() != newName.toUpperCase()) {
	Error("A file or directory with that name already exists.");
}

BatBegin("Rename");
BatWriteCmdEchoed(["rename",oldPath,newName]); //??? Fails on hidden files.
BatWriteCheckError();
BatWriteLine("if exist " + BatQuoteArg(newPath) + " (");
BatWriteSendSelectKS([newPath]);
BatWriteLine(") else (");
BatWriteSendUpdate();
BatWriteLine(")");
BatEnd();
