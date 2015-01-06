#!/usr/bin/env python
#-------------------------------------------------------------------------------
# cpptohtml.py
#
# Copyright (C) 2010 Oliver Hamann.
#
# Homepage: http://eaglemode.sourceforge.net/
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 3 as published by the
# Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
# more details.
#
# You should have received a copy of the GNU General Public License version 3
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#-------------------------------------------------------------------------------

import sys
import re


#==================================== Topic ====================================

class Topic:
	def __init__(self,text="",files=[]):
		self.text=text
		self.files=files[:]


#==================================== File =====================================

class File:
	def __init__(self,path="",sections=[]):
		self.path=path
		self.htmlPath=path.replace("/","_").replace("\\","_").replace(".","_")+".html"
		self.sections=sections[:]


#=================================== Section ===================================

class Section:
	def __init__(self,text="",isSymbol=False,lineNumber=-1):
		self.text=text
		self.isSymbol=isSymbol
		self.lineNumber=lineNumber
		self.anchor=re.sub("[^a-zA-Z0-9]+","_",text)


#============================ CreateFilesHashtable =============================

def CreateFilesHashtable(topics):
	pairs=[]
	for t in topics:
		for f in t.files:
			pairs.append([f.path,f.htmlPath])
	return dict(pairs)


#=========================== CreateSymbolsHashtable ============================

def CreateSymbolsDictonary(topics):
	pairs=[]
	for t in topics:
		for f in t.files:
			for s in f.sections:
				if s.isSymbol:
					pairs.append([
						s.text,
						f.htmlPath+"#"+s.anchor
					])
	return dict(pairs)


#============================= ConvertTabsToSpaces =============================

def ConvertTabsToSpaces(txt,tabSize):
	i=0
	x=0
	while i<len(txt):
		if txt[i]=="\t":
			n=tabSize-x%tabSize
			txt=txt[:i]+" "*n+txt[i+1:]
			x+=n
			i+=n
		elif txt[i]=="\n":
			x=0
			i+=1
		else:
			x+=1
			i+=1
	return txt


#================================== QuoteHtml ==================================

def QuoteHtml(txt):
	return txt.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;")


#================================== WriteFile ==================================

def WriteFile(contents,path):
	sys.stdout.write("Writing "+path+"\n")
	sys.stdout.flush()
	f=open(path,'w')
	f.write(contents)
	f.close()


#============================== WriteFrameSetFile ==============================

def WriteFrameSetFile(path):
	WriteFile(
		"<HTML>\n"
		"<HEAD>\n"
		"\t<TITLE>"+QuoteHtml(MainTitle)+"</TITLE>\n"
		"\t<SCRIPT LANGUAGE=\"JavaScript\">\n"
		"\t\tfunction FrameSetLoaded()\n"
		"\t\t{\n"
		"\t\t\tc=self.location.href.match(\"[?&]content[=]([^?&]+)([&]|$)\");\n"
		"\t\t\tif (c) frames[\"contentFrame\"].location=c[1];\n"
		"\t\t}\n"
		"\t</SCRIPT>\n"
		"</HEAD>\n"
		"<FRAMESET COLS=\"25%,75%\" ONLOAD=\"FrameSetLoaded()\">\n"
		"\t<FRAME NAME=\"treeFrame\" SRC=\"tree.html\">\n"
		"\t<FRAME NAME=\"contentFrame\" SRC=\"index-noframes.html\">\n"
		"\t<NOFRAMES>This page requires a browser that can show frames.</NOFRAMES>\n"
		"</FRAMESET>\n"
		"</HTML>\n",
		path
	)


#================================ WriteTreeFile ================================

def WriteTreeFile(topics,current,path):
	txt=(
		"<HTML>\n"+
		"<HEAD>\n"+
		"\t<META NAME=\"ROBOTS\" CONTENT=\"NOINDEX, NOFOLLOW\">\n"+
		"\t<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"styles.css\">\n"+
		"</HEAD>\n"+
		"<BODY>\n"+
		"<A CLASS=treeIndex HREF=\"index.html\" target=\"_top\">Index</A>\n"+
		"<P>\n"
	)
	for i in range(len(topics)):
		t=topics[i]
		if i!=current:
			txt+="<A CLASS=treeTopic HREF=\"tree"+str(i)+".html\">"+QuoteHtml(t.text)+"</A><BR>\n"
			continue
		txt+="<A CLASS=treeTopic HREF=\"tree.html\">"+QuoteHtml(t.text)+"</A><BR>\n"
		for h in t.files:
			txt+="<TT>&nbsp;&nbsp;</TT><A CLASS=treeFile HREF=\""+h.htmlPath+"\" target=\"contentFrame\">"+h.path+"</A>"+"<BR>\n"
			for s in h.sections:
				txt+="<TT>&nbsp;&nbsp;&nbsp;&nbsp;</TT><A CLASS=treeSection HREF=\""+h.htmlPath+"#"+s.anchor+"\" target=\"contentFrame\">"+QuoteHtml(s.text)+"</A>"+"<BR>\n"
	txt+=(
		"</BODY>\n"
		"</HTML>\n"
	)
	WriteFile(txt,path)


#============================ WriteMainContentFile =============================

def WriteMainContentFile(topics,path):
	txt=(
		"<HTML>\n"+
		"<HEAD>\n"
		"\t<TITLE>"+QuoteHtml(MainTitle)+"</TITLE>\n"+
		"\t<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"styles.css\">\n"+
		"\t<SCRIPT TYPE=\"text/javascript\" SRC=\"script.js\"></SCRIPT>\n"+
		"</HEAD>\n"+
		"<BODY>\n"+
		MainIntro
	)
	for t in topics:
		txt+="<H2>"+QuoteHtml(t.text)+"</H2>\n"
		txt+="<TABLE BORDER=1 CELLPADDING=3 CELLSPACING=1>\n"
		txt+="<TR><TD><I>File</I></TD><TD><I>Section/Symbol</I></TD></TR>\n"
		for h in t.files:
			txt+="<TR><TD VALIGN=TOP>\n"
			txt+="<A CLASS=main HREF=\""+h.htmlPath+"\">"+h.path+"</A>"+"<BR>\n"
			txt+="</TD><TD VALIGN=TOP>\n"
			for s in h.sections:
				txt+="<A CLASS=main HREF=\""+h.htmlPath+"#"+s.anchor+"\">"+QuoteHtml(s.text)+"</A>"+"<BR>\n"
			txt+="</TD></TR>\n"
		txt+="</TABLE>\n"
	txt+="</BODY>\n</HTML>\n"
	WriteFile(txt,path)


#============================== CppHtmlConverter ===============================

class CppHtmlConverter:
	def __init__(
		self,filesDictonary,symbolsDictonary,sourceBaseDir,outputDir
	):
		self._filesDictonary=filesDictonary
		self._symbolsDictonary=symbolsDictonary
		self._sourceBaseDir=sourceBaseDir
		self._outputDir=outputDir
		self._reFile=re.compile("[<\"]([^>\"]*)[>\"]")
		self._reSymbol=re.compile("([a-zA-Z_][a-zA-Z_0-9]*)")
		self._reNumeric=re.compile(
			"("
			"((([0-9]+)|(0[xX][0-9a-fA-F]*))[uU]?[lL]*)|"
			"([0-9]*\.?[0-9]+([eE][\+-]?[0-9]+)?[fF]?)"
			")"
			"(?![0-9a-zA-Z\.])"
		)
		self._keywordDictionary=dict({
			'NULL'            :'u',
			'asm'             :'t',
			'auto'            :'t',
			'bool'            :'t',
			'char'            :'t',
			'class'           :'t',
			'const'           :'t',
			'double'          :'t',
			'enum'            :'t',
			'explicit'        :'t',
			'export'          :'t',
			'extern'          :'t',
			'float'           :'t',
			'friend'          :'t',
			'inline'          :'t',
			'int'             :'t',
			'long'            :'t',
			'mutable'         :'t',
			'operator'        :'t',
			'private'         :'t',
			'protected'       :'t',
			'public'          :'t',
			'register'        :'t',
			'short'           :'t',
			'signed'          :'t',
			'static'          :'t',
			'struct'          :'t',
			'template'        :'t',
			'typedef'         :'t',
			'typeid'          :'t',
			'typename'        :'t',
			'union'           :'t',
			'unsigned'        :'t',
			'virtual'         :'t',
			'void'            :'t',
			'volatile'        :'t',
			'wchar_t'         :'t',
			'break'           :'k',
			'case'            :'k',
			'catch'           :'k',
			'const_cast'      :'k',
			'continue'        :'k',
			'default'         :'k',
			'delete'          :'k',
			'do'              :'k',
			'dynamic_cast'    :'k',
			'else'            :'k',
			'false'           :'k',
			'for'             :'k',
			'goto'            :'k',
			'if'              :'k',
			'namespace'       :'k',
			'new'             :'k',
			'reinterpret_cast':'k',
			'return'          :'k',
			'sizeof'          :'k',
			'static_cast'     :'k',
			'switch'          :'k',
			'this'            :'k',
			'throw'           :'k',
			'true'            :'k',
			'try'             :'k',
			'using'           :'k',
			'while'           :'k'
		})

	def Convert(self,theFile):
		self._file=theFile
		self._srcFileHandle=open(
			self._sourceBaseDir+"/"+self._file.path
		)
		outPath=self._outputDir+"/"+self._file.htmlPath
		sys.stdout.write("Writing "+outPath+"\n")
		sys.stdout.flush()
		self._htmlFileHandle=open(outPath,'w')
		self._lineNumber=1
		self._sectionIndex=0
		self._spanClass="d"
		self._WriteHeader()
		self._WriteSourceFile()
		self._WriteSetSpanClass("d")
		self._WriteFooter()
		self._htmlFileHandle.close()

	def _WriteHeader(self):
		self._Write(
			"<HTML>\n"+
			"<HEAD>\n"+
			"\t<TITLE>"+self._file.path+"</TITLE>\n"+
			"\t<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"styles.css\">\n"+
			"\t<SCRIPT TYPE=\"text/javascript\" SRC=\"script.js\"></SCRIPT>\n"+
			"</HEAD>\n"+
			"<BODY>\n"+
			SourceIntro+
			"<PRE><TT><SPAN CLASS=d>"
		)

	def _WriteFooter(self):
		self._Write("</SPAN></TT></PRE>\n</BODY>\n</HTML>\n")

	def _WriteSourceFile(self):
		txt=self._srcFileHandle.read()
		txt=ConvertTabsToSpaces(txt,TabSize)
		i=0
		while i<len(txt):
			c=txt[i]
			if c.isspace():
				self._WriteSource(c)
				i+=1
				continue
			if c=="/":
				if txt[i+1]=="/":
					j=i+2
					while j<len(txt) and txt[j]!="\n": j+=1
					j+=1
					self._WriteComment(txt[i:j])
					i=j
					continue
				if txt[i+1]=="*":
					j=i+2
					while j<len(txt) and txt[j:j+2]!="*/": j+=1
					j+=2
					self._WriteComment(txt[i:j])
					i=j
					continue
			if c=="#":
				j=i-1
				while j>=0:
					if not txt[j].isspace():
						j=1
						break
					if txt[j]=="\n":
						j=-1
						break
					j-=1
				if j<0:
					self._WriteSetSpanClass("p")
					j=i+1
					while j<len(txt) and txt[j]!="\n": j+=1
					j+=1
					k=i+1
					while k<len(txt) and txt[k].isspace(): k+=1
					if k+7<=len(txt) and txt[k:k+7]=="include":
						self._WriteWithFileLinks(txt[i:j])
					else:
						self._WriteSource(txt[i:j])
					i=j
					continue
			if c=="\"":
				self._WriteSetSpanClass("v")
				j=i+1
				while j<len(txt):
					if txt[j]=="\\" and j+1<len(txt):
						j+=1
					elif txt[j]=="\"":
						j+=1
						break;
					j+=1
				self._WriteSource(txt[i:j])
				i=j
				continue
			if c=="'":
				self._WriteSetSpanClass("w")
				j=i+1
				while j<len(txt):
					if txt[j]=="\\" and j+1<len(txt):
						j+=1
					elif txt[j]=="'":
						j+=1
						break;
					j+=1
				self._WriteSource(txt[i:j])
				i=j
				continue
			m=self._reSymbol.match(txt,i)
			if m:
				j=m.end(1)
				s=m.group(1)
				l=self._symbolsDictonary.get(s)
				if l!=None:
					self._WriteSetSpanClass("d")
					self._Write("<A CLASS=l HREF=\""+l+"\">")
					self._WriteSource(s)
					self._Write("</A>")
				else:
					self._WriteSetSpanClass(
						self._keywordDictionary.get(s,'d')
					)
					self._WriteSource(s)
				i=j
				continue
			m=self._reNumeric.match(txt,i)
			if m:
				j=m.end(1)
				s=m.group(1)
				self._WriteSetSpanClass('u')
				self._WriteSource(s)
				i=j
				continue
			if c=="{" or c=="}":
				self._WriteSetSpanClass("b")
				self._WriteSource(c)
				i+=1
				continue
			self._WriteSetSpanClass("d")
			self._WriteSource(c)
			i+=1

	def _WriteSetSpanClass(self,spanClass):
		if self._spanClass!=spanClass:
			if self._spanClass!="d": self._Write("</SPAN>")
			if spanClass!="d": self._Write("<SPAN CLASS="+spanClass+">")
			self._spanClass=spanClass

	def _WriteComment(self,txt):
		self._WriteSetSpanClass("c")
		while len(txt)>0:
			m=self._reSymbol.search(txt)
			if m:
				s=txt[m.start(1):m.end(1)]
				l=self._symbolsDictonary.get(s)
				if l!=None:
					if m.start(1)>0: self._WriteSource(txt[0:m.start(1)])
					self._Write("<A CLASS=m HREF=\""+l+"\">")
					self._WriteSource(s)
					self._Write("</A>")
				else:
					self._WriteSource(txt[0:m.end(1)])
				txt=txt[m.end(1):]
			else:
				self._WriteSource(txt)
				break;

	def _WriteWithFileLinks(self,txt):
		while len(txt)>0:
			m=self._reFile.search(txt)
			if m:
				s=txt[m.start(1):m.end(1)]
				l=self._filesDictonary.get(s)
				if l!=None:
					if m.start(1)>0: self._WriteSource(txt[0:m.start(1)])
					self._Write("<A CLASS=f HREF=\""+l+"\">")
					self._WriteSource(s)
					self._Write("</A>")
				else:
					self._WriteSource(txt[0:m.end(1)])
				txt=txt[m.end(1):]
			else:
				self._WriteSource(txt)
				break;

	def _WriteSource(self,txt):
		txt=QuoteHtml(txt)
		while len(txt)>0:
			i=txt.find("\n")
			if i<0:
				self._Write(txt)
				break
			self._Write(txt[0:i+1])
			self._lineNumber+=1
			while (
				self._sectionIndex<len(self._file.sections) and
				self._lineNumber>=self._file.sections[self._sectionIndex].lineNumber
			):
				self._Write(
					"<A NAME=\""+
					self._file.sections[self._sectionIndex].anchor+
					"\"></A>"
				)
				self._sectionIndex+=1
			txt=txt[i+1:]

	def _Write(self,txt):
		self._htmlFileHandle.write(txt)


#================ Parse args, execute config, write HTML files =================

if len(sys.argv)!=4:
	sys.stderr.write(
		"Usage: "+sys.argv[0]+" <config file> <source base dir> <output dir>\n"
	)
	sys.exit(1)
ConfigFile=sys.argv[1]
SourceBaseDir=sys.argv[2]
OutputDir=sys.argv[3]

execfile(ConfigFile)

WriteFile(Styles,OutputDir+"/styles.css")
WriteFile(JScript,OutputDir+"/script.js")

converter=CppHtmlConverter(
	CreateFilesHashtable(Topics),
	CreateSymbolsDictonary(Topics),
	SourceBaseDir,
	OutputDir
)
for t in Topics:
	for f in t.files:
		converter.Convert(f)

WriteTreeFile(Topics,-1,OutputDir+"/tree.html")

for i in range(len(Topics)):
	WriteTreeFile(Topics,i,OutputDir+"/tree"+str(i)+".html")

WriteMainContentFile(Topics,OutputDir+"/index-noframes.html")

WriteFrameSetFile(OutputDir+"/index.html")
