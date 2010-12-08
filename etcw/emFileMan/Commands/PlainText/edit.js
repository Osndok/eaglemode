/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = wscript
# Caption = edit
# Descr =Open plain text files in the MS-DOS Editor.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The plain text files to be opened.
#[[END PROPERTIES]]
*/

var FileSys=WScript.CreateObject("Scripting.FileSystemObject");
var WshShell=WScript.CreateObject("WScript.Shell");
var incFile=FileSys.OpenTextFile(WshShell.ExpandEnvironmentStrings(
	"%EM_DIR%\\res\\emFileMan\\scripts\\cmd-util.js"
));
eval(incFile.ReadAll());
incFile.Close();

ErrorIfNoTargets();
ErrorIfTargetsNotFiles();

BatBegin("edit");
BatWriteCmdEchoed(["edit"].concat(Tgt));
BatWriteCheckError();
BatEnd();
