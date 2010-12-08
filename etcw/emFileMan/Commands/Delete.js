/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 7.0
# Interpreter = wscript
# Caption = Delete
# Descr =Remove one or more files and/or directories.
# Descr =Directories are removed recursively.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files and directories to be removed.
# ButtonBgColor = #E56
# ButtonFgColor = #000
# Hotkey = Delete
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
ErrorIfRootTargets();

ConfirmIfTargetsAccrossDirs();

var message="Are you sure to remove definitively";
for (var i=0; i<Tgt.length; i++) {
	if (IsDirectory(Tgt[i])) {
		message+=" and recursively";
		break;
	}
}
message+=":\n\n";
message+=GetTgtListing();
Confirm("Delete",message);

BatBegin("Delete");
for (var i=0; i<Tgt.length; i++) {
	if (IsDirectory(Tgt[i])) {
		BatWriteCmdEchoed(["rmdir","/S","/Q",Tgt[i]]);
	}
	else {
		BatWriteCmdEchoed(["del","/F","/Q",Tgt[i]]);
	}
	// rmdir and del are not always setting the error level on error.
	BatWriteLine("if exist " + QuoteArg(Tgt[i]) + " (");
	BatWriteSetErrored();
	BatWriteLine("goto L_END");
	BatWriteLine(")");
}
BatWriteLine(":L_END");
BatWriteSendUpdate();
BatEnd();
