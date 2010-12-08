/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 4.0
# Interpreter = wscript
# Caption = Clone
# Descr =Create a copy of a file or directory within the same
# Descr =parent directory. The name for the copy is asked. If
# Descr =a directory is copied, it is copied recursively.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The file or directory to be cloned.
# ButtonBgColor = #796
# ButtonFgColor = #000
# Hotkey = Ctrl+V
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
	"Clone",
	"Please enter a name for a copy of:\n\n" + oldPath,
	oldName
);
CheckFilename(newName);

var newPath=GetChildPath(dir,newName);

if (IsExistingPath(newPath)) {
	Error("A file or directory with that name already exists.");
}

BatBegin("Clone");
if (IsDirectory(oldPath)) {
	BatWriteCmdEchoed(["xcopy","/F","/H","/K","/Y","/E","/I",oldPath,newPath]);
	BatWriteCheckError();
	BatWriteLine("if exist " + QuoteArg(newPath) + " (");
	BatWriteSendSelect([newPath]);
	BatWriteLine(") else (");
	BatWriteSendUpdate();
	BatWriteLine(")");
}
else {
	BatWriteLineEchoed("<nul (set /p X=) > " + QuoteArg(newPath));
	BatWriteLine("if exist " + QuoteArg(newPath) + " (");
	BatWriteCmdEchoed(["xcopy","/F","/H","/K","/Y",oldPath,newPath]);
	BatWriteCheckError();
	BatWriteSendSelect([newPath]);
	BatWriteLine(") else (");
	BatWriteSetErrored();
	BatWriteSendUpdate();
	BatWriteLine(")");
}
BatEnd();
