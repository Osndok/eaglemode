/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 2.0
# Interpreter = wscript
# Caption = WordPad
# Descr =Open a text file in WordPad.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The text file to be opened.
#[[END PROPERTIES]]
*/

var FileSys=WScript.CreateObject("Scripting.FileSystemObject");
var WshShell=WScript.CreateObject("WScript.Shell");
var incFile=FileSys.OpenTextFile(WshShell.ExpandEnvironmentStrings(
	"%EM_DIR%\\res\\emFileMan\\scripts\\cmd-util.js"
));
eval(incFile.ReadAll());
incFile.Close();

var wordpad=WshShell.ExpandEnvironmentStrings(
	"%ProgramFiles%\\Windows NT\\Accessories\\wordpad.exe"
);

OpenSingleTargetFileWith([wordpad]);
