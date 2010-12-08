/*
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 9.0
# Interpreter = wscript
# Caption = Select Recursively
# Descr =Recursively select files and/or directories whose names match a
# Descr =pattern. The pattern is asked. The matching entries are listed
# Descr =in a terminal and selected as the target. The source selection
# Descr =is kept unchanged.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files and directories to be taken for the pattern
# Descr =          test. Directories are scanned recursively.
# ButtonFgColor = #C44
# Hotkey = Ctrl+S
#[[END PROPERTIES]]
*/

var FileSys=WScript.CreateObject("Scripting.FileSystemObject");
var WshShell=WScript.CreateObject("WScript.Shell");
var incFile=FileSys.OpenTextFile(WshShell.ExpandEnvironmentStrings(
	"%EM_DIR%\\res\\emFileMan\\scripts\\cmd-util.js"
));
eval(incFile.ReadAll());
incFile.Close();

if (IsFirstPass()) {

	ErrorIfNoTargets();

	var pattern=Edit(
		"Select Recursively",
		"Please enter a pattern for the names of the entries which shall be selected.\n"+
		"\n"+
		"For this pattern matching, directory names have a backslash ('\\') at the end.\n"+
		"\n"+
		"Special characters in the pattern are:\n"+
		"  *  Matches any character sequence, but not the backslash of a directory name.\n"+
		"  ?  Matches any single character, but not the backslash of a directory name.\n"+
		"  |  Or-operator for multiple patterns.\n"+
		"\n"+
		"Examples:\n"+
		"  *          Select all files.\n"+
		"  *\\         Select all directories.\n"+
		"  *|*\\       Select all files and directories.\n"+
		"  README     Select all files named \"README\".\n"+
		"  CVS\\       Select all directories named \"CVS\".\n"+
		"  *.txt      Select all files ending with \".txt\".\n"+
		"  *.cpp|*.h  Select all files ending with \".cpp\" or \".h\".\n"+
		"  a*\\|b*\\    Select all directories beginning with \"a\" or \"b\".",
		"*"
	);

	SetFirstPassResult(pattern+"X");
		// The X is added because cscript does not unquote a backslash
		// before a terminating double-quote (CommandFromArgs does not
		// work correctly for scripts).

	SecondPassInTerminal("Select Recursively", true);
}

var pattern=GetFirstPassResult();
pattern=pattern.substr(0,pattern.length-1);

var regEx='(^';
for (var i=0; i<pattern.length; i++) {
	var c=pattern.substr(i,1);
	if (c == '*') regEx+='[^\\\\]*';
	else if (c == '?') regEx+='[^\\\\]';
	else if (c == '|') regEx+='$)|(^';
	else if ('^|?/.()[]{}$*\\+'.indexOf(c)>=0) regEx+="\\"+c;
	else regEx+=c;
}
regEx+='$)';
regEx=new RegExp(regEx,"i");

var found=new Array;
var foundAnyHidden=false;

function SrDoPathName(pathname)
{
	var path=GetParentPath(pathname);
	var name=GetNameInPath(pathname);
	var foundAny=false;

	if (found.length>1000) { //???
		WScript.StdOut.Write("\nToo many entries found.\n");
		WScript.Quit(1);
	}

	if (IsDirectory(pathname)) {
		if (regEx.test(name+"\\")) {
			found.push(pathname);
			WScript.StdOut.Write(
				IsRootDirectory(pathname) ? pathname : (pathname+"\\\n")
			);
			foundAny=true;
		}
		var list=new Array;
		var fld=FileSys.GetFolder(pathname);
		for (var e=new Enumerator(fld.SubFolders); !e.atEnd(); e.moveNext()) {
			list.push(e.item().Name);
		}
		for (var e=new Enumerator(fld.Files); !e.atEnd(); e.moveNext()) {
			list.push(e.item().Name);
		}
		list.sort();
		for (var i=0; i<list.length; i++) {
			if (SrDoPathName(GetChildPath(pathname,list[i]))) {
				foundAny=true;
			}
		}
	}
	else {
		if (regEx.test(name)) {
			found.push(pathname);
			WScript.StdOut.Write(pathname+"\n");
			foundAny=true;
		}
	}

	if (foundAny && IsHiddenPath(pathname)) {
		foundAnyHidden=true;
	}

	return foundAny;
}

WScript.StdOut.Write("\nFound the following files and directories:\n\n");
for (var i=0; i<Tgt.length; i++) {
	SrDoPathName(Tgt[i]);
}

if (found.length>0) {
	WScript.StdOut.Write("\nSelecting the found entries as the target.\n");
}
else {
	WScript.StdOut.Write("\nNo matching entries found - clearing target selection.\n");
}
CheckEnvProblem(found);
SendSelectKS(found);

if (foundAnyHidden) {
	Warning(
		"Warning: There are hidden files or directories which\n"+
		"match the pattern and which have been selected."
	);
}
