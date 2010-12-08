/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 52.0
# Interpreter = wscript
# Caption = Unpack New Dir
# Descr =Create a new subdirectory and unpack an archive file into it. The
# Descr =name is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The archive file.
# Descr =
# Descr =  Target: The directory in which the new subdirectory shall be
# Descr =          created.
# Descr =
# Descr =Following archive file formats are supported, provided that the
# Descr =corresponding system tools are installed: 7z, arc, arj, bz, bz2,
# Descr =gz, jar, lha, lzh, lzma, tar, tar.bz, tar.bz2, tar.gz, tar.lzma,
# Descr =tar.xz, tar.Z, taz, tbz, tbz2, tgj, tgz, tlz, txz, xz, Z, zip
# ButtonFgColor = #AAD
# Hotkey = Meta+U
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
ErrorIfTargetsNotDirs();

var name=GetNameInPath(Src[0]);
var i=name.lastIndexOf('.');
if (i<=0) {
	name+='.unpacked';
}
else {
	name=name.substr(0,i);
	i=name.lastIndexOf('.');
	if (i>0 && name.substr(i).toLowerCase() == '.tar') {
		name=name.substr(0,i);
	}
}

name=Edit(
	"Unpack New Dir",
	"Please enter a name for a new subdirectory in\n"+
	"\n"+
	"  "+Tgt[0]+"\n"+
	"\n"+
	"in order to unpack the archive file\n"+
	"\n"+
	"  "+Src[0]+"\n"+
	"\n"+
	"into that new subdirectory.",
	name
);
CheckFilename(name);

var path=GetChildPath(Tgt[0],name);

if (IsExistingPath(path)) {
	Error("A file or directory of that name already exists");
}

BatBegin("Unpack New Dir");
BatWriteCmdEchoed(["mkdir",path]);
BatWriteLine("if errorlevel 1 (");
BatWriteSendUpdate();
BatWriteExitWithError();
BatWriteLine(")");
BatWriteUnpack(Src[0],Src[0],path);
BatWriteSendUpdate();
BatEnd();
