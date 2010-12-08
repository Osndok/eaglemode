/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 51.0
# Interpreter = wscript
# Caption = fc
# Descr =Compare two files and show the difference.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: First file.
# Descr =
# Descr =  Target: Second file.
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

ErrorIfNotSingleSource();
ErrorIfNotSingleTarget();
ErrorIfSourcesNotFiles();
ErrorIfTargetsNotFiles();

BatBegin("fc");
BatWriteCmdEchoed(["fc",Src[0],Tgt[0]]);
BatWriteLine("if errorlevel 2 (");
BatWriteSetErrored();
BatWriteLine(")");
BatWriteExitByUser();
BatEnd();
