//------------------------------------------------------------------------------
// emArch.js
//
// Copyright (C) 2019-2021 Oliver Hamann.
//
// Homepage: http://eaglemode.sourceforge.net/
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3 as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
// more details.
//
// You should have received a copy of the GNU General Public License version 3
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

var FileSys=WScript.CreateObject("Scripting.FileSystemObject");
var WshShell=WScript.CreateObject("WScript.Shell");


//================================ Description =================================

var Description=
"This javascript program can pack and unpack archive files of various formats\n"+
"by running the appropriate system tools. It is used by emTmpConv for showing\n"+
"the contents of an archive, and it is used by some user commands of emFileMan.\n"+
"\n"+
"SUPPORTED FORMATS:\n"+
"\n"+
"Archive file name suffices    | Required system tools\n"+
"------------------------------+----------------------\n"+
".7z                           | 7za\n"+
".bz             (unpack only) | bzip2\n"+
".bz2                          | bzip2\n"+
".gz                           | gzip\n"+
".lzma                         | xz\n"+
".tar                          | tar\n"+
".tar.bz|.tbz    (unpack only) | tar, bzip2\n"+
".tar.bz2|.tbz2|.tgj           | tar, bzip2\n"+
".tar.gz|.tgz                  | tar, gzip\n"+
".tar.lzma|.tlz                | tar, xz\n"+
".tar.xz|.txz                  | tar, xz\n"+
".tar.Z|.taz     (unpack only) | tar, gzip\n"+
".xz                           | xz\n"+
".Z              (unpack only) | gzip\n"+
".zip|.jar                     | zip, unzip\n"+
"\n"+
"SECURITY / SPEED-LOSS:\n"+
"\n"+
"At least because of the heavy use through emTmpConv, this script program tries\n"+
"its best in avoiding the creation of unpacked files outside the current\n"+
"directory, even through nasty archives. Mostly, the archive listing is inspected\n"+
"and the unpacking is refused if the listing contains an absolute path or an\n"+
"up-going relative path.\n"+
"\n"+
"Unfortunately, that listing of an archive can cost a lot of extra time,\n"+
"especially with tar-based archives. In Addition, the test is mostly a little\n"+
"bit heuristic and may falsely refuse archives with harmless paths like\n"+
"\"bla /bla\" and \"bla ../bla\".\n"+
"\n"+
"USAGE:\n"+
"\n"+
"  cscript emArch.js pack|p [<options>] [--] <archive file> <source file|dir>...\n"+
"\n"+
"  cscript emArch.js unpack|u [<options>] [--] <archive file>\n"+
"\n"+
"With the first variant, the archive file is created from the source files and\n"+
"directories recursively (but some archive formats can pack only a single file).\n"+
"With the second variant, the archive file is read and the contained files and\n"+
"directories are unpacked into the current working directory.\n"+
"\n"+
"OPTIONS:\n"+
"\n"+
"  -f|--format <format>\n"+
"      Force archive file format. The default is to detect the format from the\n"+
"      archive file name suffix. The format can be specified by a sample file\n"+
"      name or path with correct suffix, or just by the suffix with our without\n"+
"      the leading dot.\n"+
"\n"+
"  -g|--trust\n"+
"      Go on and trust the archive or the unpack tool. This disables the test\n"+
"      for absolute or up-going paths when unpacking.\n"+
"\n"+
"  -h|--help\n"+
"      Print this help and exit.\n";


//=============================== Configuration ================================

// Whether to use pigz or 7za instead of gzip for gz archives.
// (At most one of these may be set to true.)
var Use_pigz=false;
var Use_7za_for_gz=false;

// Whether to use pbzip2, lbzip2 or 7za instead of bzip2 for bz2 archives.
// (At most one of these may be set to true.)
var Use_pbzip2=false;
var Use_lbzip2=false;
var Use_7za_for_bz2=false;

// Whether to use pixz, pxz or 7za instead of xz for xz archives.
// (At most one of these may be set to true.)
var Use_pixz=false;
var Use_pxz=false;
var Use_7za_for_xz=false;

// Whether to use 7za instead of zip/unzip for zip archives.
var Use_7za_for_zip=false;


//============================== General Helpers ===============================

function Error(text)
{
	WScript.StdErr.WriteLine("Error: "+text);
	WScript.Quit(1);
}


function ErrorBadArgs()
{
	Error("Bad arguments. Type: cscript emArch.js --help");
}


function BasicQuoteArg(arg)
{
	var str,res,i,k,c;

	str=arg.toString();
	for (i=str.length-1; i>=0; i--) {
		c=str.charAt(i);
		if ((c<'0' || c>'9') && (c<'A' || c>'Z') && (c<'a' || c>'z') &&
		    c!='\\' && c!='/' && c!='.' && c!=':' && c!='-') break;
	}
	if (i<0 && str.length>0) {
		res=str;
	}
	else {
		res='"';
		k=0;
		for (i=0; i<str.length; i++) {
			c=str.charAt(i);
			if (c=='"') {
				while (k>0) { res+='\\'; k--; }
				res+='\\';
			}
			else if (c=='\\') k++;
			else k=0;
			res+=c;
		}
		while (k>0) { res+='\\'; k--; }
		res+='"';
	}
	return (res);
}


function QuotePercentSignsByEnv(arg)
{
	var str,i;

	str=arg;
	for (i=str.length-1; i>=0; i--) {
		if (str.charAt(i)=='%') {
			var env=WshShell.Environment("PROCESS");
			if (env("PERCENT_SIGN")!="%") env("PERCENT_SIGN")="%";
			str=str.substr(0,i)+'%PERCENT_SIGN'+str.substr(i);
		}
	}
	return (str);
}


function WshShellQuoteArg(arg)
{
	return QuotePercentSignsByEnv(BasicQuoteArg(arg));
}


function WshShellCmdFromArgs(argsArray)
{
	var cmd,i;

	cmd="";
	for (i=0; i<argsArray.length; i++) {
		if (i>0) cmd+=" ";
		cmd+=WshShellQuoteArg(argsArray[i]);
	}
	return (cmd);
}


function GetNameInPath(path)
{
	return (FileSys.GetFileName(path));
}


function IsRootDirectory(path)
{
	return (
		(path.length==2 && path.charAt(1)==':') ||
		(path.length==3 && path.charAt(1)==':' && path.charAt(2)=='\\')
	);
}


function IsDirectory(path)
{
	return (FileSys.FolderExists(path) || IsRootDirectory(path));
}


function IsRegularFile(path)
{
	return (FileSys.FileExists(path));
}


function IsExistingPath(path)
{
	return (IsRegularFile(path) || IsDirectory(path));
}


function TestEnding(path,ending)
{
	if (ending.length>0 && ending.charAt(0)=='.') ending=ending.substr(1);
	var i=path.length-ending.length;
	return (
		i>=2 &&
		path.charAt(i-1)=='.' &&
		path.substr(i).toLowerCase()==ending.toLowerCase()
	);
}


function HasAnyEnding(path)
{
	var name=GetNameInPath(path);
	var i=name.indexOf('.');
	return (i>=1 && i<name.length-1);
}


function IsDosFilename(name)
{
	var i=name.indexOf('.');
	var l=name.length;
	return (i>=0 && l<=12 && i>=l-4);
}


function ConcatArrays(array1,array2)
{
	return (array1.concat(array2));
}


function GetCmdCall(command)
{
	var q=QuotePercentSignsByEnv(command);
		// Yes, quote percent signs by env a second time! (one for cscript,
		// one for cmd. Quoting by %% would be for BAT)

	return WshShellCmdFromArgs(["cmd","/E:ON","/F:OFF","/V:OFF","/C"])+" "+q;
}


function Exec(inCmd, readStdErrFirst, command)
{
	// Command must not print too much to stderr, or to stdout if
	// readStdErrFirst==true. Otherwise deadlock possible!

	var fullCommand;
	if (inCmd) {
		fullCommand=GetCmdCall(command);
	}
	else {
		fullCommand=command;
	}

	try {
		var wse=WshShell.Exec(fullCommand);
		if (readStdErrFirst) {
			while (!wse.StdErr.AtEndOfStream) {
				WScript.StdErr.WriteLine(wse.StdErr.ReadLine());
			}
		}
		while (!wse.StdOut.AtEndOfStream) {
			WScript.StdOut.WriteLine(wse.StdOut.ReadLine());
		}
		while (!wse.StdErr.AtEndOfStream) {
			WScript.StdErr.WriteLine(wse.StdErr.ReadLine());
		}
		while (!wse.Status) {
			WScript.Sleep(100);
		}
		if (wse.ExitCode != 0) {
			WScript.Quit(wse.ExitCode);
		}
	}
	catch (e) {
		Error("Failed to run " + fullCommand + ": " + e.description);
	}
}


function Pack(format,archive,names)
{
	if (FileSys.FileExists(archive)) {
		try {
			FileSys.DeleteFile(archive,true);
		}
		catch (e) {
			Error("Failed to delete " + archive + ": " + e.description);
		}
	}

	var f='x.'+format;
	if (TestEnding(f,'7z')) {
		if (!HasAnyEnding(archive)) {
			Error("Archive file name has no suffix (e.g. \".7z\").");
			// Otherwise 7z automatically appends ".7z" (would
			// break the semantics of this script interface).
		}
		Exec(false,false,"7za a -- "+WshShellQuoteArg(archive)+" "+WshShellCmdFromArgs(names));
	}
	else if (TestEnding(f,'tar')) {
		Exec(true,true,"tar cvf - -- "+WshShellCmdFromArgs(names)+" > "+WshShellQuoteArg(archive));
	}
	else if (TestEnding(f,'tar.bz2') || TestEnding(f,'tbz2') ||
	         TestEnding(f,'tgj')) {
		Exec(
			true,true,"tar cvf - -- "+WshShellCmdFromArgs(names)+" | "+(
				Use_pbzip2 ? "pbzip2 -c" :
				Use_lbzip2 ? "lbzip2 -c" :
				Use_7za_for_bz2 ? "7za a -si -so .bz2" :
				"bzip2 -c"
			)+" > "+WshShellQuoteArg(archive)
		);
	}
	else if (TestEnding(f,'tar.gz') || TestEnding(f,'tgz')) {
		Exec(
			true,true,"tar cvf - -- "+WshShellCmdFromArgs(names)+" | "+(
				Use_pigz ? "pigz -c" :
				Use_7za_for_gz ? "7za a -si -so .gz" :
				"gzip -c"
			)+" > "+WshShellQuoteArg(archive)
		);
	}
	else if (TestEnding(f,'tar.lzma') || TestEnding(f,'tlz')) {
		Exec(true,true,"tar cvf - -- "+WshShellCmdFromArgs(names)+" | xz --stdout --format=lzma > "+WshShellQuoteArg(archive));
	}
	else if (TestEnding(f,'tar.xz') || TestEnding(f,'txz')) {
		Exec(
			true,true,"tar cvf - -- "+WshShellCmdFromArgs(names)+" | "+(
				Use_pixz ? "pixz" :
				Use_pxz ? "pxz --stdout" :
				Use_7za_for_xz ? "7za a -si -so .xz" :
				"xz --stdout"
			)+" > "+WshShellQuoteArg(archive)
		);
	}
	else if (TestEnding(f,'zip') || TestEnding(f,'jar')) {
		if (!HasAnyEnding(archive)) {
			Error("Archive file name has no suffix (e.g. \".zip\").");
			// Otherwise zip automatically appends ".zip" (would
			// break the semantics of this script interface).
		}
		if (Use_7za_for_zip) {
			Exec(false,false,"7za a -tzip -- "+WshShellQuoteArg(archive)+" "+WshShellCmdFromArgs(names));
		}
		else {
			Exec(false,false,"zip -r -9 "+WshShellQuoteArg(archive)+" -- "+WshShellCmdFromArgs(names));
		}
	}
	else if (TestEnding(f,'bz2')) {
		if (names.length>1) {
			Error("Cannot pack multiple files into bz2 archive.");
		}
		if (IsDirectory(names[0])) {
			Error("Cannot pack a directory into bz2 archive.");
		}
		Exec(
			true,true,(
				Use_pbzip2 ? "pbzip2 -c --" :
				Use_lbzip2 ? "lbzip2 -c --" :
				Use_7za_for_bz2 ? "7za a -so -- .bz2" :
				"bzip2 -c --"
			)+" "+WshShellQuoteArg(names[0])+" > "+WshShellQuoteArg(archive)
		);
	}
	else if (TestEnding(f,'gz')) {
		if (names.length>1) {
			Error("Cannot pack multiple files into gz archive.");
		}
		if (IsDirectory(names[0])) {
			Error("Cannot pack a directory into gz archive.");
		}
		Exec(
			true,true,(
				Use_pigz ? "pigz -c --" :
				Use_7za_for_gz ? "7za a -so -- .gz" :
				"gzip -c --"
			)+" "+WshShellQuoteArg(names[0])+" > "+WshShellQuoteArg(archive)
		);
	}
	else if (TestEnding(f,'lzma')) {
		if (names.length>1) {
			Error("Cannot pack multiple files into lzma archive.");
		}
		if (IsDirectory(names[0])) {
			Error("Cannot pack a directory into lzma archive.");
		}
		Exec(true,true,"xz --stdout --format=lzma -- "+WshShellQuoteArg(names[0])+" > "+WshShellQuoteArg(archive));
	}
	else if (TestEnding(f,'xz')) {
		if (names.length>1) {
			Error("Cannot pack multiple files into xz archive.");
		}
		if (IsDirectory(names[0])) {
			Error("Cannot pack a directory into xz archive.");
		}
		Exec(
			true,true,(
				Use_pixz ? "pixz -t <" :
				Use_pxz ? "pxz --stdout --" :
				Use_7za_for_xz ? "7za a -so -- .xz" :
				"xz --stdout --"
			)+" "+WshShellQuoteArg(names[0])+" > "+WshShellQuoteArg(archive)
		);
	}
	else {
		Error("Packing of "+format+" not supported");
	}
}


function CheckUnpackPaths(inCmd,command,ignRegEx,failRegEx,failRegEx2)
{
	// Command must not print too much to stderr. Otherwise deadlock
	// possible!

	WScript.StdOut.WriteLine("Scanning archive listing for dangerous paths...");

	var fullCommand;
	if (inCmd) {
		fullCommand=GetCmdCall(command);
	}
	else {
		fullCommand=command;
	}

	var badLine=null;
	var badLine2=null;
	try {
		var wse=WshShell.Exec(fullCommand);
		while (!wse.StdOut.AtEndOfStream) {
			var line=wse.StdOut.ReadLine();
			if (badLine==null && badLine2==null && !ignRegEx.test(line)) {
				if (failRegEx.test(line)) {
					badLine=line;
				}
				if (failRegEx2.test(line)) {
					badLine2=line;
				}
			}
		}
		while (!wse.StdErr.AtEndOfStream) {
			WScript.StdErr.WriteLine(wse.StdErr.ReadLine());
		}
		while (!wse.Status) {
			WScript.Sleep(100);
		}
		if (wse.ExitCode != 0) {
			WScript.Quit(wse.ExitCode);
		}
	}
	catch (e) {
		Error("Failed to run " + fullCommand + ": " + e.description);
	}

	if (badLine!=null) {
		Error("Archive looks like containing an absolute or up-going path:\n" + badLine);
	}
	if (badLine2!=null) {
		Error("Archive seems to contain a file name which is equal to a Windows device name:\n" + badLine2);
	}

	WScript.StdOut.WriteLine("okay, unpacking...");
}


function CheckUnpackPathsInPureList(inCmd,command,ignRegEx)
{
	CheckUnpackPaths(
		inCmd,command,ignRegEx,
		/(^\s*((((.*[\\/])|)\.\.)|)[\\/])|:/,
		/((^\s*)|[\\/])(aux|(com[0-9])|con|(lpt[0-9])|nul|prn)(\.|[\\/]|(\s*$))/i
	);
}


function CheckUnpackPathsInInfoList(inCmd,command,ignRegEx)
{
	CheckUnpackPaths(
		inCmd,command,ignRegEx,
		/(^|\s)((((((.*[\\/])|)\.\.)|)[\\/])|(.:))/,
		/(^|\s|[\\/])(aux|(com[0-9])|con|(lpt[0-9])|nul|prn)(\.|[\\/]|\s|$)/i
	);
}


function Unpack(format,archive,trust)
{
	// The tools must not ask questions, because stdin cannot be forwarded.
	// Therefore, for example, 7za x is called with -aoa (overwrite without
	// prompt).
	var f='x.'+format;
	if (TestEnding(f,'7z')) {
		if (!trust) {
			CheckUnpackPathsInInfoList(
				false,
				"7za l "+WshShellQuoteArg(archive),
				/^(Listing archive:|Path =)/
			);
		}
		Exec(false,false,"7za x -aoa "+WshShellQuoteArg(archive));
	}
	else if (TestEnding(f,'tar')) {
		if (!trust) {
			CheckUnpackPathsInPureList(
				true,
				"tar tf - < "+WshShellQuoteArg(archive),
				/^$/
			);
		}
		Exec(true,true,"tar xvf - < "+WshShellQuoteArg(archive));
	}
	else if (TestEnding(f,'tar.bz2') || TestEnding(f,'tbz2') ||
	         TestEnding(f,'tgj') || TestEnding(f,'tar.bz') ||
	         TestEnding(f,'tbz')) {
		if (!trust) {
			CheckUnpackPathsInPureList(
				true,(
					Use_pbzip2 ? "pbzip2 -d -c" :
					Use_lbzip2 ? "lbzip2 -d -c" :
					Use_7za_for_bz2 ? "7za x -tbzip2 -si -so" :
					"bzip2 -d -c"
				)+" < "+WshShellQuoteArg(archive)+" | tar tf -",
				/^$/
			);
		}
		Exec(
			true,true,(
				Use_pbzip2 ? "pbzip2 -d -c" :
				Use_lbzip2 ? "lbzip2 -d -c" :
				Use_7za_for_bz2 ? "7za x -tbzip2 -si -so" :
				"bzip2 -d -c"
			)+" < "+WshShellQuoteArg(archive)+" | tar xvf -"
		);
	}
	else if (TestEnding(f,'tar.gz') || TestEnding(f,'tgz') ||
	         TestEnding(f,'tar.z') || TestEnding(f,'taz')) {
		if (!trust) {
			CheckUnpackPathsInPureList(
				true,(
					Use_pigz ? "pigz -d -c" :
					Use_7za_for_gz ? "7za x -tgzip -si -so" :
					"gzip -d -c"
				)+" < "+WshShellQuoteArg(archive)+" | tar tf -",
				/^$/
			);
		}
		Exec(
			true,true,(
				Use_pigz ? "pigz -d -c" :
				Use_7za_for_gz ? "7za x -tgzip -si -so" :
				"gzip -d -c"
			)+" < "+WshShellQuoteArg(archive)+" | tar xvf -"
		);
	}
	else if (TestEnding(f,'tar.lzma') || TestEnding(f,'tlz')) {
		if (!trust) {
			CheckUnpackPathsInPureList(
				true,
				"xz --decompress --stdout < "+WshShellQuoteArg(archive)+" | tar tf -",
				/^$/
			);
		}
		Exec(true,true,"xz --decompress --stdout < "+WshShellQuoteArg(archive)+" | tar xvf -");
	}
	else if (TestEnding(f,'tar.xz') || TestEnding(f,'txz')) {
		if (!trust) {
			CheckUnpackPathsInPureList(
				true,(
					Use_pixz ? "pixz -d" :
					Use_7za_for_xz ? "7za x -txz -si -so" :
					"xz --decompress --stdout"
				)+" < "+WshShellQuoteArg(archive)+" | tar tf -",
				/^$/
			);
		}
		Exec(
			true,true,(
				Use_pixz ? "pixz -d" :
				Use_7za_for_xz ? "7za x -txz -si -so" :
				"xz --decompress --stdout"
			)+" < "+WshShellQuoteArg(archive)+" | tar xvf -"
		);
	}
	else if (TestEnding(f,'zip') || TestEnding(f,'jar')) {
		if (!trust) {
			if (Use_7za_for_zip) {
				CheckUnpackPathsInInfoList(
					false,
					"7za l -tzip -- "+WshShellQuoteArg(archive),
					/^(Listing archive:|Path =)/
				);
			}
			else {
				CheckUnpackPathsInInfoList(
					false,
					"unzip -l "+WshShellQuoteArg(archive),
					/^Archive:/
				);
			}
		}
		if (Use_7za_for_zip) {
			Exec(false,false,"7za x -aoa -tzip -- "+WshShellQuoteArg(archive));
		}
		else {
			Exec(false,false,"unzip -o "+WshShellQuoteArg(archive));
		}
	}
	else if (TestEnding(f,'bz2') || TestEnding(f,'bz')) {
		var n=GetNameInPath(archive);
		var e='';
		i=n.lastIndexOf('.');
		if (i>0) {
			e=n.substr(i);
			n=n.substr(0,i);
		}
		if (e.toLowerCase()=='.bz2') e='';
		else if (e.toLowerCase()=='.bz') e='';
		else if (e.toLowerCase()=='.tbz2') e='.tar';
		else if (e.toLowerCase()=='.tbz') e='.tar';
		else if (e.toLowerCase()=='.tgz')  e='.tar';
		else e=e+'.unpacked';
		n+=e;
		if (IsExistingPath(n)) {
			Error("File already exists: "+n);
		}
		Exec(
			true,true,(
				Use_pbzip2 ? "pbzip2 -d -c" :
				Use_lbzip2 ? "lbzip2 -d -c" :
				Use_7za_for_bz2 ? "7za x -tbzip2 -si -so" :
				"bzip2 -d -c"
			)+" < "+WshShellQuoteArg(archive)+" > "+WshShellQuoteArg(n)
		);
	}
	else if (TestEnding(f,'gz') || TestEnding(f,'z')) {
		var n=GetNameInPath(archive);
		var e='';
		i=n.lastIndexOf('.');
		if (i>0) {
			e=n.substr(i);
			n=n.substr(0,i);
		}
		if (e.toLowerCase()=='.gz') e='';
		else if (e.toLowerCase()=='.z') e='';
		else if (e.toLowerCase()=='.tgz') e='.tar';
		else if (e.toLowerCase()=='.taz') e='.tar';
		else e=e+'.unpacked';
		n+=e;
		if (IsExistingPath(n)) {
			Error("File already exists: "+n);
		}
		Exec(
			true,true,(
				Use_pigz ? "pigz -d -c" :
				Use_7za_for_gz ? "7za x -tgzip -si -so" :
				"gzip -d -c"
			)+" < "+WshShellQuoteArg(archive)+" > "+WshShellQuoteArg(n)
		);
	}
	else if (TestEnding(f,'lzma')) {
		var n=GetNameInPath(archive);
		var e='';
		i=n.lastIndexOf('.');
		if (i>0) {
			e=n.substr(i);
			n=n.substr(0,i);
		}
		if (e.toLowerCase()=='.lzma') e='';
		else if (e.toLowerCase()=='.tlz') e='.tar';
		else e=e+'.unpacked';
		n+=e;
		if (IsExistingPath(n)) {
			Error("File already exists: "+n);
		}
		Exec(true,true,"xz --decompress --stdout < "+WshShellQuoteArg(archive)+" > "+WshShellQuoteArg(n));
	}
	else if (TestEnding(f,'xz')) {
		var n=GetNameInPath(archive);
		var e='';
		i=n.lastIndexOf('.');
		if (i>0) {
			e=n.substr(i);
			n=n.substr(0,i);
		}
		if (e.toLowerCase()=='.xz') e='';
		else if (e.toLowerCase()=='.txz') e='.tar';
		else e=e+'.unpacked';
		n+=e;
		if (IsExistingPath(n)) {
			Error("File already exists: "+n);
		}
		Exec(
			true,true,(
				Use_pixz ? "pixz -d" :
				Use_7za_for_xz ? "7za x -txz -si -so" :
				"xz --decompress --stdout"
			)+" < "+WshShellQuoteArg(archive)+" > "+WshShellQuoteArg(n)
		);
	}
	else {
		Error("Unpacking of "+format+" not supported");
	}
}


//================================ Main Program ================================

if (WScript.Arguments.Count() == 0) {
	WScript.StdOut.Write(Description);
	WScript.Quit(1);
}

var argIdx=0;
var cmd=WScript.Arguments(argIdx++);
if (cmd=='u' || cmd=='unpack') {
	cmd='unpack';
}
else if (cmd=='p' || cmd=='pack') {
	cmd='pack';
}
else if (cmd=='-h' || cmd=='--help') {
	WScript.StdOut.Write(Description);
	WScript.Quit(0);
}
else {
	ErrorBadArgs();
}

var trust=false;
var format='';

while (argIdx < WScript.Arguments.Count()) {
	var a=WScript.Arguments(argIdx);
	if (a=='-g' || a=='--trust') {
		argIdx++;
		trust=true;
	}
	else if ((a=='-f' || a=='--format') && argIdx+1 < WScript.Arguments.Count()) {
		argIdx++;
		format=WScript.Arguments(argIdx++);
	}
	else if (a=='-h' || a=='--help') {
		WScript.StdOut.Write(Description);
		WScript.Quit(0);
	}
	else if (a=='--') {
		argIdx++;
		break;
	}
	else if (a.substring(0,1)=='-') {
		Error("Illegal option: "+a);
	}
	else {
		break;
	}
}

if (argIdx >= WScript.Arguments.Count()) {
	ErrorBadArgs();
}
var archive=WScript.Arguments(argIdx++);

if (format=='') {
	format=archive;
}

if (cmd=='pack') {
	if (argIdx >= WScript.Arguments.Count()) {
		ErrorBadArgs();
	}
	var names=[];
	while (argIdx < WScript.Arguments.Count()) {
		names.push(WScript.Arguments(argIdx++));
	}
	Pack(format,archive,names);
}
else {
	if (argIdx < WScript.Arguments.Count()) {
		ErrorBadArgs();
	}
	Unpack(format,archive,trust);
}
