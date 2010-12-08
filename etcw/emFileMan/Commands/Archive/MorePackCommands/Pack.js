/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 20.0
# Interpreter = wscript
# Caption = Pack
# Descr =Create an archive from files and directories.
# Descr =The name of the archive file is asked. The archive
# Descr =type is specified by the file name suffix.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The files and directories to be packed.
# Descr =
# Descr =  Target: The directory in which the archive file
# Descr =          shall be created.
# Hotkey = Shift+Meta+P
#[[END PROPERTIES]]
*/

var FileSys=WScript.CreateObject("Scripting.FileSystemObject");
var WshShell=WScript.CreateObject("WScript.Shell");
var incFile=FileSys.OpenTextFile(WshShell.ExpandEnvironmentStrings(
	"%EM_DIR%\\res\\emFileMan\\scripts\\cmd-util.js"
));
eval(incFile.ReadAll());
incFile.Close();

ErrorIfNoSources();
ErrorIfSourcesAccrossDirs();
ErrorIfNotSingleTarget();
ErrorIfTargetsNotDirs();

var srcDir=GetParentPath(Src[0]);

var srcNames=new Array;
for (var i=0; i<Src.length; i++) srcNames[i]=GetNameInPath(Src[i]);

var name="";
if (Src.length == 1) name=srcNames[0]+".";

name=Edit(
	"Pack",
	"Please enter a file name for the new archive in:\n"+
	"\n"+
	"  "+Tgt[0]+"\n"+
	"\n"+
	"The file name suffix specifies the archive type and must be one of:\n"+
	"  .7z\n"+
	"  .arc\n"+
	"  .arj\n"+
	"  .bz2\n"+
	"  .gz\n"+
	"  .lzh | .lha\n"+
	"  .lzma\n"+
	"  .tar\n"+
	"  .tar.bz2 | .tbz2 | .tgj\n"+
	"  .tar.gz | .tgz\n"+
	"  .tar.lzma | .tlz\n"+
	"  .tar.xz | .txz\n"+
	"  .xz\n"+
	"  .zip | .jar\n",
	name
);
CheckFilename(name);

var path=GetChildPath(Tgt[0],name);
if (IsExistingPath(path)) {
	Error("A file or directory of that name already exists.");
}

BatBegin("Pack");
BatWritePack(path,path,srcDir,srcNames);
BatWriteLine("if exist " + QuoteArg(path) + " (");
BatWriteSendSelect(path);
BatWriteLine(") else (");
BatWriteSetErrored();
BatWriteSendUpdate();
BatWriteLine(")");
BatEnd();
