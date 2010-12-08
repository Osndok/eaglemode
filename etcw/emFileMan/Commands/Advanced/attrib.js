/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 5.0
# Interpreter = wscript
# Caption = attrib
# Descr =Change the attributes of one or more files and/or directories.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files and directories whose attributes shall be
# Descr =          changed.
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

ConfirmIfTargetsAccrossDirs();

var oldAttr=-1;

for (var i=0; i<Tgt.length; i++) {
	var entry;
	if (IsDirectory(Tgt[i])) entry=FileSys.GetFolder(Tgt[i]);
	else entry=FileSys.GetFile(Tgt[i]);
	var attr=(entry.Attributes&(1|2|4|32));
	if (i==0) oldAttr=attr;
	else if (oldAttr!=attr) oldAttr=-1;
}

var oldAttrStr="";
if (oldAttr!=-1) {
	oldAttrStr+="[";
	if (oldAttr& 1) oldAttrStr+="r";
	if (oldAttr& 2) oldAttrStr+="h";
	if (oldAttr& 4) oldAttrStr+="s";
	if (oldAttr&32) oldAttrStr+="a";
	oldAttrStr+="]";
}

var attrStr=Edit(
	"attrib",
	"Please enter new attribute set (enclosed in []) or attribute\n"+
	"changes (with +/-).\n"+
	"\n"+
	"Examples:\n"+
	"  +a       Set the archive attribute.\n"+
	"  +s       Set the system attribute.\n"+
	"  +h+r     Set the hidden and read-only attributes.\n"+
	"  -r       Remove the read-only attribute.\n"+
	"  -a-s     Remove the archive and system attributes.\n"+
	"  -h+s     Remove the hidden attribute and set the system attribute.\n"+
	"  []       Remove all attributes.\n"+
	"  [ashr]   Set all attributes.\n"+
	"  [ar]     Like +a-s-h+r.",
	oldAttrStr
);

attrStr=attrStr.toLowerCase();

for (var i=0; i<attrStr.length; i++) {
	if (attrStr.charAt(i)==' ') {
		attrStr=attrStr.substr(0,i)+attrStr.substr(i+1);
		i--;
	}
}

var attrArgs=new Array;
if (
	attrStr.length>=2 && attrStr.charAt(0)=="[" &&
	attrStr.charAt(attrStr.length-1)==']'
) {
	var newAttr=0;
	for (var i=1; i<attrStr.length-1; i++) {
		var c=attrStr.charAt(i);
		if      (c=='r') newAttr|= 1;
		else if (c=='h') newAttr|= 2;
		else if (c=='s') newAttr|= 4;
		else if (c=='a') newAttr|=32;
		else Error("Unknown attribute: "+c);
	}
	attrArgs[0] = ((newAttr& 1) ? "+" : "-" ) + "r";
	attrArgs[1] = ((newAttr& 2) ? "+" : "-" ) + "h";
	attrArgs[2] = ((newAttr& 4) ? "+" : "-" ) + "s";
	attrArgs[3] = ((newAttr&32) ? "+" : "-" ) + "a";
}
else {
	if (attrStr.length==0) WScript.Quit(0);
	if ((attrStr.length&1)!=0) Error("Illegal input");
	for (var i=0; i<attrStr.length; i+=2) {
		var c1=attrStr.charAt(i);
		var c2=attrStr.charAt(i+1);
		if (
			(c1!="+" && c1!="-") ||
			(c2!="a" && c2!="s" && c2!="h" && c2!="r")
		) {
			Error("Illegal input");
		}
		attrArgs[i/2]=c1+c2;
	}
}

BatBegin("attrib");
for (var i=0; i<Tgt.length; i++) {
	BatWriteCmdEchoed(["attrib"].concat(attrArgs).concat(Tgt[i]));
	BatWriteCheckError();
}
BatWriteSendUpdate();
BatWriteExitByUser(); // Because attrib does not always return an error code on error.
BatEnd();
