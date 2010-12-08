/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = wscript
# Caption = Exchange
# Descr =Exchange the source for the target.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The source file or directory that shall get the path name of the
# Descr =          target.
# Descr =
# Descr =  Target: The target file or directory that shall get the path name of the
# Descr =          source.
# Descr =
# Descr =NOTE: At first, the target is moved to a temporary name in the form
# Descr ="<name>.ex-tmp-<number>". Then the source path is moved to the target
# Descr =path, and finally the temporary path is move to the source path. If one of
# Descr =the movings fails, it is tried to restore the original state. But that
# Descr =does not work in all cases. You may have to repair manually then.
# ButtonBgColor = #99B
# Hotkey = Ctrl+E
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
ErrorIfRootSources();
ErrorIfRootTargets();

var message=
	"Are you sure to exchange\n"+
	"\n"+
	GetSrcListing()+
	"\n"+
	"for\n\n"+
	GetTgtListing()
;
Confirm("Exchange",message);

var srcPath=Src[0];
var tgtPath=Tgt[0];

var tmpPath;
for (var i=0; ; i++) {
	tmpPath=tgtPath+".ex-tmp-"+i;
	if (!IsExistingPath(tmpPath)) break;
}

BatBegin("Exchange");
BatWriteCmdEchoed(["move","/Y",tgtPath,tmpPath]);
BatWriteCheckError();
BatWrite("if not %ANY_ERROR%==0 goto L_END\n");
BatWriteCmdEchoed(["move","/Y",srcPath,tgtPath]);
BatWriteCheckError();
BatWrite("if not %ANY_ERROR%==0 goto L_TMP2TGT\n");
BatWriteCmdEchoed(["move","/Y",tmpPath,srcPath]);
BatWriteCheckError();
BatWrite("if %ANY_ERROR%==0 goto L_END\n");
BatWriteLine("if exist " + QuoteArg(srcPath) + " goto L_END\n");
BatWriteCmdEchoed(["move","/Y",tgtPath,srcPath]);
BatWriteLine(":L_TMP2TGT");
BatWriteLine("if exist " + QuoteArg(tgtPath) + " goto L_END\n");
BatWriteCmdEchoed(["move","/Y",tmpPath,tgtPath]);
BatWriteLine(":L_END");
BatWriteLine("if %ANY_ERROR%==0 (");
BatWriteSendSelect(srcPath);
BatWriteLine(") else (");
BatWriteSendUpdate();
BatWriteLine(")");
BatEnd();
