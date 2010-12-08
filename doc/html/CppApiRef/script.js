
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
	if (!document.URL.match(/[/\\]index-noframes[.]html/i)) {
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
