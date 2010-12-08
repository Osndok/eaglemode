/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 50.0
# Interpreter = wscript
# Caption = Unpack
# Descr =Unpack an archive file into a directory.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The archive file.
# Descr =
# Descr =  Target: The directory in which the unpacked files and
# Descr =          directories shall be created.
# Descr =
# Descr =Following archive file formats are supported, provided that the
# Descr =corresponding system tools are installed: 7z, arc, arj, bz, bz2,
# Descr =gz, jar, lha, lzh, lzma, tar, tar.bz, tar.bz2, tar.gz, tar.lzma,
# Descr =tar.xz, tar.Z, taz, tbz, tbz2, tgj, tgz, tlz, txz, xz, Z, zip
# ButtonFgColor = #AAD
# Hotkey = Ctrl+U
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

var message=
	"Are you sure to unpack the archive while overwriting any existing\n"+
	"target files?\n"+
	"\n"+
	"From:\n"+
	GetSrcListing()+
	"\n"+
	"Into:\n"+
	GetTgtListing()
;
Confirm("Unpack",message);

BatBegin("Unpack");
BatWriteUnpack(Src[0],Src[0],Tgt[0]);
BatWriteSendUpdate();
BatEnd();
