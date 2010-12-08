/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 53.0
# Interpreter = wscript
# Caption = findstr
# Descr =Search with a text pattern in a file or directory
# Descr =recursively, and show the matching lines. The text
# Descr =pattern is asked. The search is case-sensitive.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: File or directory to be searched.
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

var pat=Edit(
	"findstr",
	"Please enter the text pattern for the search. It's\n"+
	"a regular expression (run 'findstr /?' for details).",
	""
);

BatBegin("findstr");
if (IsDirectory(Tgt[0])) {
	BatWriteCmdEchoed(['cd','/D',Tgt[0]]);
	BatWriteLine("if errorlevel 1 (");
	BatWriteExitWithError();
	BatWriteLine(")");
	BatWriteCmdEchoed(["findstr","/n","/r","/s",pat,"*.*"]);
}
else {
	BatWriteCmdEchoed(["findstr","/n","/r",pat,Tgt[0]]);
}
BatWriteLine("if errorlevel 1 (");
BatWriteLine("if errorlevel 2 (");
BatWriteSetErrored();
BatWriteLine(") else (");
BatWriteLine("echo No match found.");
BatWriteLine(")");
BatWriteLine(")");
BatWriteExitByUser();
BatEnd();
