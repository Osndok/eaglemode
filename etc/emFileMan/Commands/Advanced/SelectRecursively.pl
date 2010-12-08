#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 9.0
# Interpreter = perl
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

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoTargets();

	my $pattern=Edit(
		"Select Recursively",
		"Please enter a pattern for the names of the entries which shall be selected.\n".
		"\n".
		"For this pattern matching, directory names have a slash (\"/\") at the end.\n".
		"\n".
		"Special characters in the pattern are:\n".
		"  *  Matches any character sequence, but not the slash of a directory name.\n".
		"  ?  Matches any single character, but not the slash of a directory name.\n".
		"  |  Or-operator for multiple patterns.\n".
		"\n".
		"Examples:\n".
		"  *          Select all files.\n".
		"  */         Select all directories.\n".
		"  *|*/       Select all files and directories.\n".
		"  README     Select all files named \"README\".\n".
		"  CVS/       Select all directories named \"CVS\".\n".
		"  *.txt      Select all files ending with \".txt\".\n".
		"  *.cpp|*.h  Select all files ending with \".cpp\" or \".h\".\n".
		"  a*/|b*/    Select all directories beginning with \"a\" or \"b\".",
		"*"
	);

	SetFirstPassResult($pattern);

	SecondPassInTerminal("Select Recursively");
}

my $pattern=GetFirstPassResult();
my $regEx='(^';
for (my $i=0; $i<length($pattern); $i++) {
	my $c=substr($pattern,$i,1);
	if ($c eq '*') { $regEx.='[^/]*'; }
	elsif ($c eq '?') { $regEx.='[^/]'; }
	elsif ($c eq '|') { $regEx.='$)|(^'; }
	elsif (index('^|?/.()[]{}$*\\+',$c)>=0) { $regEx.="\\$c"; }
	else { $regEx.=$c; }
}
$regEx.='$)';

my @found=();
my $foundAnyHidden=0;

sub SrDoPathName
{
	my $path=shift;
	my $name=shift;
	my $pathname=catfile($path,$name);
	my $foundAny=0;

	if (@found>10000) { #???
		print("\nToo many entries found.\n");
		TermEndByUser(1);
	}

	if (-d $pathname) {
		if ("$name/" =~ /$regEx/) {
			push(@found,$pathname);
			print("$pathname/\n");
			$foundAny=1;
		}
		my $dh;
		my @list=();
		if (!opendir($dh,$pathname)) {
			print("\nFailed to read directory $pathname: $!\n");
			TermEndByUser(1);
		}
		while (defined(my $n=readdir($dh))) {
			if ($n ne '.' && $n ne '..') {
				push(@list,$n);
			}
		}
		closedir($dh);
		@list=sort(@list);
		for (my $i=0; $i<@list; $i++) {
			if (SrDoPathName($pathname,$list[$i])) {
				$foundAny=1;
			}
		}
	}
	else {
		if ($name =~ /$regEx/) {
			push(@found,$pathname);
			print("$pathname\n");
			$foundAny=1;
		}
	}
	if (
		$foundAny &&
		substr($name,0,1) eq '.' # ??? UNIX specific
	) {
		$foundAnyHidden=1;
	}

	return $foundAny;
}

print("\nFound the following files and directories:\n\n");
my @tgt=GetTgt();
for (my $i=0; $i<@tgt; $i++) {
	my ($name,$path)=fileparse($tgt[$i]);
	SrDoPathName($path,$name);
}

if (@found>0) {
	print("\nSelecting the found entries as the target.\n");
}
else {
	print("\nNo matching entries found - clearing target selection.\n");
}
SendSelectKS(@found);

if ($foundAnyHidden) {
	Warning(
		"Warning: There are hidden files or directories which\n".
		"match the pattern and which have been selected."
	);
}

TermEndByUser(0);
