/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 51.0
# Interpreter = wscript
# DefaultFor = .7z:.bz:.bz2:.gz:.jar:.lzma:.tar:.tar.bz:.tar.bz2:.tar.gz:.tar.lzma:.tar.xz:.tar.z:.taz:.tbz:.tbz2:.tgj:.tgz:.tlz:.txz:.xz:.z:.zip
# Caption = Unpack In Place
# Descr =Unpack an archive file into the directory where the archive file
# Descr =is in.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The archive file.
# Descr =
# Descr =Following archive file formats are supported, provided that the
# Descr =corresponding system tools are installed: 7z, bz, bz2, gz, jar,
# Descr =lzma, tar, tar.bz, tar.bz2, tar.gz, tar.lzma, tar.xz, tar.Z, taz,
# Descr =tbz, tbz2, tgj, tgz, tlz, txz, xz, Z, zip
# Icon = unpack_file_in_place.tga
# Hotkey = Shift+Ctrl+U
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

var tgtdir=GetParentPath(Tgt[0]);

var message=
	"Are you sure to unpack the archive into the same directory\n"+
	"while overwriting any existing target files?\n"+
	"\n"+
	"From:\n"+
	GetTgtListing()+
	"\n"+
	"Into:\n"+
	"  "+tgtdir
;
Confirm("Unpack In Place",message);

BatBegin("Unpack In Place");
BatWriteUnpack(Tgt[0],Tgt[0],tgtdir);
BatWriteSendUpdate();
BatEnd();
