/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 1.0
# Interpreter = wscript
# Caption = New Dir
# Descr =Create a new empty subdirectory. The name is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The directory in which the new subdirectory
# Descr =          shall be created.
# ButtonBgColor = #88C
# ButtonFgColor = #000
# Hotkey = Ctrl+D
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
ErrorIfTargetsNotDirs();

var dir=Tgt[0];
var newName;
for (var i=1; ; i++) {
	newName="newdir"+i;
	if (!IsExistingPath(GetChildPath(dir,newName))) break;
}

newName=Edit(
	"New Dir",
	"Please enter a name for a new subdirectory in:\n\n" + dir,
	newName
);
CheckFilename(newName);

var newPath=GetChildPath(dir,newName);
if (IsExistingPath(newPath)) {
	Error("A file or directory with that name already exists.");
}

BatBegin("New Dir");
BatWriteCmdEchoed(["mkdir",newPath]);
BatWriteCheckError();
BatWriteLine("if exist " + QuoteArg(newPath) + " (");
BatWriteSendSelectKS([newPath]);
BatWriteLine(") else (");
BatWriteSendUpdate();
BatWriteLine(")");
BatEnd();
