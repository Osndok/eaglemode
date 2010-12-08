#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = perl
# Caption = Exchange
# Descr =Exchange the source for the target.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The source file or directory that shall get the path name of the
# Descr =          target.
# Descr =
# Descr =  Target: The target file or directory that shall get the path name of the
# Descr =          source.
# Descr =
# Descr =NOTE: At first, the target is moved to a temporary name in the form
# Descr ="<name>.ex-tmp-<number>". Then the source path is moved to the target
# Descr =path, and finally the temporary path is move to the source path. If one of
# Descr =the movings fails, it is tried to restore the original state. But that
# Descr =does not work in all cases. You may have to repair manually then.
# ButtonBgColor = #99B
# Hotkey = Ctrl+E
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNotSingleSource();
	ErrorIfNotSingleTarget();
	ErrorIfRootSources();
	ErrorIfRootTargets();

	my $message=
		"Are you sure to exchange\n".
		"\n".
		GetSrcListing().
		"\n".
		"for\n\n".
		GetTgtListing()
	;
	Confirm("Exchange",$message);

	SecondPassInTerminal("Exchange");
}

my @srcs=GetSrc();
my @tgts=GetTgt();
my $src=$srcs[0];
my $tgt=$tgts[0];

my $tmp;
for (my $i=0; ; $i++) {
	$tmp=$tgt.".ex-tmp-$i";
	if (!-e $tmp) {
		last;
	}
}

sub Move
{
	return TermRun(
		"mv",
		($Config{'osname'} eq 'linux' or $Config{'osname'} eq 'cygwin') ? (
			"-vf"
		)
		: (
			"-f"
		),
		"--",
		$_[0],
		$_[1]
	);
}

my $e=Move($tgt,$tmp);
if ($e==0) {
	$e=Move($src,$tgt);
	if ($e==0) {
		$e=Move($tmp,$src);
		if ($e!=0) {
			if (!-e $src) {
				if (Move($tgt,$src)==0) {
					if (!-e $tgt) {
						Move($tmp,$tgt);
					}
				}
			}
		}
	}
	else {
		if (!-e $tgt) {
			Move($tmp,$tgt);
		}
	}
}

TermSync();

if ($e==0) {
	SendSelect($src);
}
else {
	SendUpdate();
}

TermEnd($e);
