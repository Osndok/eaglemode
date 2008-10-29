#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 2.0
# Interpreter = perl
# Caption = Create ISO
# Descr =Create an ISO file and copy files into it. The name of the new ISO
# Descr =file is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The files and directories to be copied into the ISO file.
# Descr =          Directories are copied recursively.
# Descr =
# Descr =  Target: The directory in which the ISO file shall be created.
# Descr =
# Descr =This command uses genisoimage to create an ISO9660 image file,
# Descr =which can be burned to a CD or DVD. The Joliet and Rock Ridge
# Descr =extensions are included (options -J and -r).
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoSources();
	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotDirs();

	my @tgt=GetTgt();
	my $isoDir=$tgt[0];
	my $isoName;
	for (my $i=1; ; $i++) {
		$isoName="newimage$i.iso";
		if (!-e catfile($isoDir,$isoName)) {
			last;
		}
	}

	$isoName=Edit(
		"Create ISO",
		"Please enter a name for the new ISO file in:\n\n$isoDir",
		$isoName
	);

	if (-e catfile($isoDir,$isoName)) {
		Error("A file or directory with that name already exists.");
	}

	my $volumeName='OPTICAL_DISK';
	$volumeName=Edit(
		"Create ISO",
		"Finally, please enter a volume name (disk label).\n".
		"It will be stored in the ISO file.",
		$volumeName
	);

	SetFirstPassResult(length($isoName).':'.$isoName.$volumeName);

	SecondPassInTerminal("Create ISO");
}

my $fpr=GetFirstPassResult();
my $fprl1=index($fpr,':');
my $fprl2=substr($fpr,0,$fprl1);
my $isoName=substr($fpr,$fprl1+1,$fprl2);
my $volumeName=substr($fpr,$fprl1+1+$fprl2);

my @src=GetSrc();
my @tgt=GetTgt();
my $isoPath=catfile($tgt[0],$isoName);

my @args=();
push(@args,'genisoimage');
push(@args,'-J');
push(@args,'-r');
push(@args,('-V',$volumeName));
push(@args,('-o',$isoPath));

# Normally, genisoimage puts the contents of a given directory into the root of
# the image, instead of creating the directory and filling it with the contents.
# Here comes a workaround for that:
push(@args,'-graft-points');
sub QuoteForGraftPoints
{
	my $s=shift;
	for (my $i=0; $i<length($s); $i++) {
		my $c=substr($s,$i,1);
		if ($c eq '=' or $c eq '\\') {
			$s=substr($s,0,$i).'\\'.substr($s,$i);
			$i++;
		}
	}
	return $s;
}
for (my $i=0; $i<@src; $i++) {
	my $srcPath=$src[$i];
	if (-d $srcPath) {
		my ($srcName,$srcDir)=fileparse($srcPath);
		push(@args,QuoteForGraftPoints($srcName)."=".QuoteForGraftPoints($srcPath));
	}
	else {
		push(@args,QuoteForGraftPoints($srcPath));
	}
}

my $e=TermRunAndSync(@args);

if (-e $isoPath) {
	SendSelectKS($isoPath);
}
else {
	SendUpdate();
}

TermEnd($e);
