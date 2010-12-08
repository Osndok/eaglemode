/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 0.0
# Interpreter = wscript
# DefaultFor = file
# Caption = Open or Run
# Descr =Open or run a file. This performs the Windows default
# Descr =action for a file.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The file to be opened or run.
# Descr =
# Descr =Hint: The current working directory is set to the parent
# Descr =directory of the target file.
# ButtonBgColor = #B9D
# ButtonFgColor = #000
# Hotkey = Ctrl+R
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

WshShell.CurrentDirectory=GetParentPath(Tgt[0]);

RestoreOrigPath();
WshShell.Run(CommandFromArgs([Tgt[0]]));
