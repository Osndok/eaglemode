/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 1.0
# Interpreter = wscript
# Caption = Notepad
# Descr =Open a plain text file in the Notepad text editor.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The plain text file to be opened.
#[[END PROPERTIES]]
*/

var FileSys=WScript.CreateObject("Scripting.FileSystemObject");
var WshShell=WScript.CreateObject("WScript.Shell");
var incFile=FileSys.OpenTextFile(WshShell.ExpandEnvironmentStrings(
	"%EM_DIR%\\res\\emFileMan\\scripts\\cmd-util.js"
));
eval(incFile.ReadAll());
incFile.Close();

OpenSingleTargetFileWith(['notepad']);
