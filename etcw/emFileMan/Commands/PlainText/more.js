/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 50.0
# Interpreter = wscript
# Caption = more
# Descr =Show a file with more.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: ignored.
# Descr =
# Descr =  Target: The file.
# ButtonBgColor = #BBB
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
ErrorIfTargetsNotFiles();

BatBegin("more");
BatWriteCmdEchoed(["more",Tgt[0]]);
BatWriteCheckError();
BatWriteExitByUser();
BatEnd();
