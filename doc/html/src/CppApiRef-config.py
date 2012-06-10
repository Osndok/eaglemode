
MainTitle="Eagle Mode - C++ API Reference"


MainIntro="""
<SCRIPT LANGUAGE="JavaScript">TopBar();</SCRIPT><BR>
<CENTER>
<FONT SIZE=4><B>Eagle Mode -</B></FONT><BR>
<FONT SIZE=7><B>C++ API Reference</B></FONT><P>
<FONT SIZE=2>Copyright &copy; 2010-2012 Oliver Hamann.
Homepage: <A CLASS=main HREF="http://eaglemode.sourceforge.net/">http://eaglemode.sourceforge.net/</A></FONT>
</CENTER>
<H1>Introduction</H1>
The Eagle Mode C++ API Reference is made of comments in the header files.
Therefore this document is an index with hyperlinks to HTML-converted header
files. It is structured by topics, files and sections, where a topic may contain
multiple files, and a file may contain multiple sections. Often, a section
stands for a symbol or class. Everything is sorted roughly from low-level to
high-level. If you see this page with HTML frames, then you can use the tree
links in the left frame for navigation. But in any case you can find the
complete index below.
<P>
<H1>Index</H1>
"""


SourceIntro="""
<SCRIPT LANGUAGE="JavaScript">TopBar();</SCRIPT>
"""


def AutoSectionsFile(path):
	sections=[]
	r=re.compile("^//====*[\s]+([^\s].*[^\s])[\s]+====*$")
	sr=re.compile("^(em|EM)[a-zA-Z0-9_@]*$")
	lineNum=0
	f=open(SourceBaseDir+"/"+path)
	for line in f:
		lineNum+=1
		m=r.match(line)
		if m==None: continue
		t=m.group(1)
		if t.lower()=="implementations": continue
		if t.lower()=="inline implementations": continue
		isSym=False
		if sr.match(t)!=None: isSym=True
		l=lineNum;
		if l>1: l-=1
		sections.append(Section(
			text=t,
			isSymbol=isSym,
			lineNumber=l
		))
	f.close()
	return File(path=path,sections=sections)


Topics=[
	Topic(text="Various Low-Level Helpers",
		files=[
			AutoSectionsFile("emCore/emStd1.h"),
			AutoSectionsFile("emCore/emStd2.h"),
			AutoSectionsFile("emCore/emTmpFile.h"),
			AutoSectionsFile("emCore/emInstallInfo.h"),
			AutoSectionsFile("emCore/emRes.h")
		]
	),
	Topic(text="Container Classes",
		files=[
			AutoSectionsFile("emCore/emString.h"),
			AutoSectionsFile("emCore/emArray.h"),
			AutoSectionsFile("emCore/emList.h"),
			AutoSectionsFile("emCore/emAvlTree.h")
		]
	),
	Topic(text="Smart Pointers",
		files=[
			AutoSectionsFile("emCore/emRef.h"),
			AutoSectionsFile("emCore/emCrossPtr.h")
		]
	),
	Topic(text="CPU Sharing",
		files=[
			AutoSectionsFile("emCore/emEngine.h"),
			AutoSectionsFile("emCore/emSignal.h"),
			AutoSectionsFile("emCore/emScheduler.h"),
			AutoSectionsFile("emCore/emTimer.h"),
			AutoSectionsFile("emCore/emPriSchedAgent.h"),
			AutoSectionsFile("emCore/emThread.h"),
			AutoSectionsFile("emCore/emProcess.h"),
			AutoSectionsFile("emCore/emMiniIpc.h")
		]
	),
	Topic(text="Graphics",
		files=[
			AutoSectionsFile("emCore/emColor.h"),
			AutoSectionsFile("emCore/emImage.h"),
			AutoSectionsFile("emCore/emPainter.h"),
			AutoSectionsFile("emCore/emATMatrix.h"),
			AutoSectionsFile("emCore/emClipRects.h")
		]
	),
	Topic(text="Recordable Data Structures",
		files=[
			AutoSectionsFile("emCore/emRec.h")
		]
	),
	Topic(text="Contexts and Models",
		files=[
			AutoSectionsFile("emCore/emContext.h"),
			AutoSectionsFile("emCore/emModel.h"),
			AutoSectionsFile("emCore/emVarModel.h"),
			AutoSectionsFile("emCore/emSigModel.h"),
			AutoSectionsFile("emCore/emVarSigModel.h")
		]
	),
	Topic(text="Basic GUI Classes",
		files=[
			AutoSectionsFile("emCore/emInput.h"),
			AutoSectionsFile("emCore/emCursor.h"),
			AutoSectionsFile("emCore/emClipboard.h"),
			AutoSectionsFile("emCore/emScreen.h"),
			AutoSectionsFile("emCore/emWindow.h"),
			AutoSectionsFile("emCore/emView.h"),
			AutoSectionsFile("emCore/emPanel.h"),
			AutoSectionsFile("emCore/emSubViewPanel.h"),
			AutoSectionsFile("emCore/emErrorPanel.h"),
			AutoSectionsFile("emCore/emGUIFramework.h"),
			AutoSectionsFile("emCore/emViewInputFilter.h")
		]
	),
	Topic(text="Toolkit Panels",
		files=[
			AutoSectionsFile("emCore/emToolkit.h")
		]
	),
	Topic(text="File Models & -Panels",
		files=[
			AutoSectionsFile("emCore/emFileModel.h"),
			AutoSectionsFile("emCore/emFilePanel.h"),
			AutoSectionsFile("emCore/emImageFile.h"),
			AutoSectionsFile("emCore/emRecFileModel.h"),
			AutoSectionsFile("emCore/emConfigModel.h"),
			AutoSectionsFile("emCore/emCoreConfig.h")
		]
	),
	Topic(text="File Panel Plugins",
		files=[
			AutoSectionsFile("emCore/emFpPlugin.h")
		]
	)
]


TabSize=8


JScript="""
function TopBar()
{
	document.write(
		'<TABLE BGCOLOR="#E0E0E0" BORDER=0 CELLPADDING=4 CELLSPACING=0 WIDTH="100%">',
		'<TR>',
		'<TD ALIGN=LEFT WIDTH="33%">'
	);
	if (top!=self) document.write(
		'<A CLASS=nav HREF="'+
		self.location.href+
		'" TARGET="_top">Hide Index Frame</A>'
	);
	else document.write(
		'<A CLASS=nav HREF="index.html?content='+
		self.location.href.match("(^|[/])([^/?]+)([?]|$)")[2]+
		'" TARGET="_top">Show Index Frame</A>'
	);
	document.write(
		'</TD><TD ALIGN=CENTER WIDTH="34%">'
	)
	if (!document.URL.match(/[/\\\\]index-noframes[.]html/i)) {
		document.write(
			'<A CLASS=nav HREF="index-noframes.html">Go Up</A>'
		)
	}
	else if (document.URL.match(/^https?:\/\/eaglemode[.]sourceforge[.]net\//i)) {
		document.write(
			'<A CLASS=nav HREF="../doc.html" TARGET="_top">Go Up</A>'
		)
	}
	else {
		document.write(
			'<A CLASS=nav HREF="../index.html" TARGET="_top">Go Up</A>'
		)
	}
	document.write(
		'</TD><TD ALIGN=RIGHT WIDTH="33%">'
	)
	if (document.URL.match(/^https?:\/\/eaglemode[.]sourceforge[.]net\//i)) {
		document.write(
			'<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR>',
			'<TD><FONT SIZE=1 COLOR="#505050">Hosted&nbsp;at&nbsp;</FONT></TD>',
			'<TD>',
			'<a href="http://sourceforge.net/projects/eaglemode"><img border=0 src="http://sflogo.sourceforge.net/sflogo.php?group_id=224126&amp;type=9" width="80" height="15" alt="Get Eagle Mode at SourceForge.net. Fast, secure and Free Open Source software downloads" /></a>',
			'</TD>',
			'</TR></TABLE>'
		);
	}
	else {
		document.write('&nbsp;');
	}
	document.write(
		'</TD></TR></TABLE>'
	);
}
"""


Styles="""
BODY {
	font-family:sans-serif;
	background:#FFFFFF;
	color:#000000;
}
A.main {
	color:#0000E0;
}
A.main:visited {
	color:#600080;
}
A.main:active {
	color:#F00000;
}
A.nav {
	font-weight:bold;
	color:#6060A0;
	text-decoration:none;
}
A.nav:hover {
	text-decoration:underline;
}
SPAN.d { /* default source code (operators, symbols and some more)*/
	color:#000000;
}
SPAN.p { /* preprocessor */
	color:#004070;
}
SPAN.c { /* comment */
	font-style:italic;
	color:#207040;
}
SPAN.t { /* type keyword */
	font-weight:bold;
	color:#902020;
}
SPAN.k { /* non-type keyword */
	font-weight:bold;
	color:#000000;
}
SPAN.b { /* braces */
	font-weight:bold;
	color:#000000;
}
SPAN.u { /* numeric constant */
	color:#209000;
}
SPAN.v { /* string constant  */
	color:#209000;
}
SPAN.w { /* char constant */
	color:#209000;
}
A.f { /* file link */
	font-weight:bold;
	color:#003080;
	text-decoration:none;
}
A.f:hover {
	text-decoration:underline;
}
A.l { /* symbol link */
	font-weight:bold;
	color:#0000C0;
	text-decoration:none;
}
A.l:hover {
	text-decoration:underline;
}
A.m { /* symbol link in a comment*/
	font-style:italic;
	font-weight:bold;
	color:#006070;
	text-decoration:none;
}
A.m:hover {
	text-decoration:underline;
}
A.treeIndex {
	font-size:166%;
	white-space:nowrap;
	font-weight:bold;
	color:#303030;
	text-decoration:none;
}
A.treeIndex:hover {
	text-decoration:underline;
}
A.treeTopic {
	white-space:nowrap;
	font-weight:bold;
	color:#005000;
	text-decoration:none;
}
A.treeTopic:hover {
	text-decoration:underline;
}
A.treeFile {
	white-space:nowrap;
	color:#003080;
	text-decoration:none;
}
A.treeFile:hover {
	text-decoration:underline;
}
A.treeSection {
	white-space:nowrap;
	font-weight:bold;
	color:#0000C0;
	text-decoration:none;
}
A.treeSection:hover {
	text-decoration:underline;
}
"""
