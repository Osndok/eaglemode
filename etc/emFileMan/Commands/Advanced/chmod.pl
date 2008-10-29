#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 5.0
# Interpreter = perl
# Caption = chmod
# Descr =Change the permissions of one or more files and/or directories.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files and directories whose permissions shall be
# Descr =          changed.
# Hotkey = Meta+O
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoTargets();

	ConfirmIfTargetsAccrossDirs();

	my @tgt=GetTgt();
	my $oldMode="";
	for (my $i=0; $i<@tgt; $i++) {
		my $m=sprintf("0%03o",(stat($tgt[$i]))[2] & 07777);
		if ($i==0) {
			$oldMode=$m;
		}
		elsif ($oldMode ne $m) {
			$oldMode="";
			last;
		}
	}

	my $mode=Edit(
		"chmod",
		"Please enter the mode argument for chmod on the target(s).\n".
		"\n".
		"Examples:\n".
		"  0755              Set permissions by an octal number.\n".
		"  a-w               Remove write permission for all.\n".
		"  u+x,g+x           Set exec permission for user and group.\n".
		"  u=rwx,g=rx,o=rx   Similar to 0755.",
		$oldMode
	);

	SetFirstPassResult($mode);

	SecondPassInTerminal("chmod");
}

my $e=TermRunAndSync(
	"chmod",
	($Config{'osname'} eq 'linux' or $Config{'osname'} eq 'cygwin') ? (
		"-v"
	)
	: (
	),
	"--",
	GetFirstPassResult(),
	GetTgt()
);

SendUpdate();

TermEnd($e);
