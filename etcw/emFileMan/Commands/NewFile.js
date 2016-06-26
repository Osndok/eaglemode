/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 2.0
# Interpreter = wscript
# Caption = New File
# Descr =Create a new empty file. The name is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The directory in which the new file
# Descr =          shall be created.
# Icon = new_file.tga
# Hotkey = Ctrl+F
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
	newName="newfile"+i;
	if (!IsExistingPath(GetChildPath(dir,newName))) break;
}

newName=Edit(
	"New File",
	"Please enter a name for a new empty file in:\n\n" + dir,
	newName
);
CheckFilename(newName);

var newPath=GetChildPath(dir,newName);
if (IsExistingPath(newPath)) {
	Error("A file or directory with that name already exists.");
}

BatBegin("New file");
BatWriteLineEchoed("<nul (set /p X=) > " + BatQuoteArg(newPath));
BatWriteLine("if exist " + BatQuoteArg(newPath) + " (");
BatWriteSendSelectKS([newPath]);
BatWriteLine(") else (");
BatWriteSetErrored();
BatWriteSendUpdate();
BatWriteLine(")");
BatEnd();
