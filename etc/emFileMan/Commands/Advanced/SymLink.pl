#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 4.0
# Interpreter = perl
# Caption = Sym Link
# Descr =Create symbolic links to files and/or directories. The symbolic
# Descr =links get the same names as the originals.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The files and directories to which the symbolic links
# Descr =          shall point.
# Descr =
# Descr =  Target: The directory in which the symbolic links shall be
# Descr =          created.
# ButtonFgColor = #FFF
# Hotkey = Ctrl+L
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoSources();
	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotDirs();

	ConfirmIfSourcesAccrossDirs();

	my $message=
		"Are you sure to create symbolic link(s):\n".
		"\n".
		"To:\n".
		GetSrcListing().
		"\n".
		"In:\n".
		GetTgtListing()
	;
	Confirm("Sym Link",$message);

	SecondPassInTerminal("Sym Link");
}

my @src=GetSrc();
my @tgt=GetTgt();

my $e=TermRunAndSync(
	"ln",
	($Config{'osname'} eq 'linux' or $Config{'osname'} eq 'cygwin') ? (
		"-vs"
	)
	: (
		"-s"
	),
	"--",
	@src,
	$tgt[0]
);

my @newTgt=();
for (my $i=0; $i<@src; $i++) {
	my $f=fileparse($src[$i]);
	my $p=catfile($tgt[0],$f);
	if (-e $p) {
		push(@newTgt,$p);
	}
}
if (@newTgt > 0) {
	SendSelectKS(@newTgt);
}
else {
	SendUpdate();
}

TermEnd($e);
