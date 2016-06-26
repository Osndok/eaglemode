var FileSys=WScript.CreateObject("Scripting.FileSystemObject");
var WshShell=WScript.CreateObject("WScript.Shell");


//============================== Helper Functions ==============================

function QuoteRecString(str)
{
	for (var i=str.length-1; i>=0; i--) {
		var c=str.charAt(i);
		if (c=='\\' || c=='"') {
			str=str.substr(0,i)+'\\'+str.substr(i);
		}
	}
	return '"' + str + '"';
}

function QuotePanelName(str)
{
	for (var i=str.length-1; i>=0; i--) {
		var c=str.charAt(i);
		if (c=='\\' || c==':') {
			str=str.substr(0,i)+'\\'+str.substr(i);
		}
	}
	return str;
}


//================ Parse args and start writing the output file ================

if (WScript.Arguments.length != 1 || WScript.Arguments(0).substr(0,1) == "-") {
		WScript.Echo("ERROR: Illegal arguments.\nUsage: wscript <script> <output file>\n");
		WScript.Quit(1);
}
var fh=FileSys.CreateTextFile(WScript.Arguments(0),true);
fh.Write("#\%rec:emBookmarks\%#\n");


//=============================== Bookmark: Help ===============================

fh.Write(
"Bookmark: {\n" +
"	Name = \"Help\"\n" +
"	Description = \"This brings you to the documentation area.\"\n" +
"	Icon = \"help.tga\"\n" +
"	Hotkey = \"F1\"\n" +
"	LocationIdentity = \":\"\n" +
"	LocationRelX = -0.36326\n" +
"	LocationRelY = -0.37791\n" +
"	LocationRelA = 0.00621\n" +
"}\n"
);


//============================= Bookmark: Home Dir =============================

var loc="::FS";
var env=WshShell.Environment("PROCESS");
var homeDir=env("USERPROFILE");
if (homeDir.length<3 || homeDir.substr(1,1)!=":") {
	homeDir="C:\\Users\\"+env("USERNAME");
}
for (var i=0; i<homeDir.length; ) {
	var j=homeDir.indexOf("\\",i);
	if (j<i) j=homeDir.length;
	if (j>i) {
		var k=j-i;
		if (j==2 && homeDir.substr(1,1) == ":") k++;
		loc += "::" + QuotePanelName(homeDir.substr(i,k));
	}
	i=j+1;
}
fh.Write(
"Bookmark: {\n" +
"	Name = \"Home\"\n" +
"	Description = \"This brings you to your home directory.\"\n" +
"	Icon = \"home.tga\"\n" +
"	Hotkey = \"F6\"\n" +
"	LocationIdentity = " + QuoteRecString(loc) + "\n" +
"	VisitAtProgramStart = yes\n" +
"}\n"
);


//============================= Bookmark: Root Dir =============================

fh.Write(
"Bookmark: {\n" +
"	Name = \"Root\"\n" +
"	Description = \"This brings you to the root directory of the file system.\"\n" +
"	Icon = \"root.tga\"\n" +
"	Hotkey = \"F7\"\n" +
"	LocationIdentity = \"::FS:\"\n" +
"}\n"
);


//========================== Bookmark: Virtual Cosmos ==========================

fh.Write(
"Bookmark: {\n" +
"	Name = \"Virtual Cosmos\"\n" +
"	Icon = \"virtual_cosmos.tga\"\n" +
"	Hotkey = \"F8\"\n" +
"	LocationIdentity = \":\"\n" +
"}\n"
);


//============================= Bookmark: SilChess =============================

fh.Write(
"Bookmark: {\n" +
"	Name = \"Chess\"\n" +
"	Icon = \"silchess.tga\"\n" +
"	LocationIdentity = \"::Chess1:\"\n" +
"}\n"
);


//============================== Bookmark: Mines ===============================

fh.Write(
"Bookmark: {\n" +
"	Name = \"Mines\"\n" +
"	Icon = \"mines.tga\"\n" +
"	LocationIdentity = \"::Mines1:\"\n" +
"}\n"
);


//============================= Bookmark: Netwalk ==============================

fh.Write(
"Bookmark: {\n" +
"	Name = \"Netwalk\"\n" +
"	Icon = \"netwalk.tga\"\n" +
"	LocationIdentity = \"::Netwalk1:\"\n" +
"}\n"
);


//============================== Bookmark: Clock ===============================

fh.Write(
"Bookmark: {\n" +
"	Name = \"Clock\"\n" +
"	Icon = \"clock.tga\"\n" +
"	LocationIdentity = \"::Clock1:\"\n" +
"}\n"
);


//================================== The End ===================================

fh.Close();
