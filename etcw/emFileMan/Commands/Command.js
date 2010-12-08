/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 8.0
# Interpreter = wscript
# DefaultFor = directory
# Caption = Command
# Descr =Start a command line interpreter where the current working directory
# Descr =is set to a given directory.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The directory to be used as the current working directory.
# Descr =          If a file is selected as the target, the parent directory
# Descr =          of that file is used.
# ButtonBgColor = #A9A
# ButtonFgColor = #000
# Hotkey = Ctrl+T
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

var dir=Tgt[0];
if (!IsDirectory(dir)) dir=GetParentPath(dir);

WshShell.CurrentDirectory=dir;

RestoreOrigPath();
WshShell.Run("cmd");
