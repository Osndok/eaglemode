#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 6.0
# Interpreter = perl
# Caption = Rename
# Descr =Rename one or more files or directories. The new name is
# Descr =asked. When renaming multiple entries, you can change only
# Descr =the identical parts at the beginning and end of the names.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files and directories to be renamed.
# ButtonBgColor = #C85
# ButtonFgColor = #000
# Hotkey = Backspace
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoTargets();
	ErrorIfRootTargets();
	ConfirmIfTargetsAccrossDirs();

	my @tgt=GetTgt();

	if (@tgt==1) {
		my $oldPath=$tgt[0];
		my ($oldName,$dir)=fileparse($oldPath);
		my $newName=Edit(
			"Rename",
			"Please enter a new name for:\n\n$oldPath",
			$oldName
		);
		if ($oldName eq $newName) { exit(0); }
		if (-e catfile($dir,$newName)) {
			Error("A file or directory with that name already exists.");
		}
		SetFirstPassResult($newName);
		SecondPassInTerminal("Rename");
	}
	else {
		my ($n,$d)=fileparse($tgt[0]);
		my $p=$n;
		my $q=$n;
		my $pl=length($p);
		my $ql=length($q);
		my $ml=length($n);
		for (my $i=1; $i<@tgt; $i++) {
			($n,$d)=fileparse($tgt[$i]);
			my $nl=length($n);
			if ($ml>$nl) { $ml=$nl; }
			while ($pl>0 && ($nl<$pl || substr($n,0,$pl) ne $p)) {
				$pl--;
				$p=substr($p,0,$pl);
			}
			while ($ql>0 && ($nl<$ql || substr($n,$nl-$ql,$ql) ne $q)) {
				$ql--;
				$q=substr($q,1,$ql);
			}
		}
		while ($pl+$ql>$ml) { $ql--; $q=substr($q,1,$ql); }
		my $res=Edit(
			"Multi-Rename",
			"Please enter the pattern for the new names of multiple\n".
			"entries. The asterisk (\"*\") stands for the unequal part\n".
			"in the names - do not remove it.\n",
			$p.'*'.$q
		);
		my $i=index($res,'*');
		if ($i<0) { Error("You removed the asterisk from the pattern."); }
		if (index($res,'*',$i+1)>0) { Error("Too many asterisks in the pattern."); }
		my $rp=substr($res,0,$i);
		my $rq=substr($res,$i+1);
		for (my $i=0; $i<@tgt; $i++) {
			($n,$d)=fileparse($tgt[$i]);
			$n=$rp.substr($n,$pl,length($n)-$pl-$ql).$rq;
			if (-e catfile($d,$n)) {
				Error("A file or directory named \"$n\" already exists.");
			}
		}
		SetFirstPassResult($pl.'*'.$ql.'*'.$res);
		SecondPassInTerminal("Multi-Rename");
	}
}

my @tgt=GetTgt();

if (@tgt==1) {
	my $oldPath=$tgt[0];
	my ($oldName,$dir)=fileparse($oldPath);
	my $newName=GetFirstPassResult();
	my $newPath=catfile($dir,$newName);
	my $e=TermRunAndSync(
		"mv",
		($Config{'osname'} eq 'linux' or $Config{'osname'} eq 'cygwin') ? (
			"-v"
		)
		: (
		),
		"--",
		$oldPath,
		$newPath
	);
	if (-e $newPath) {
		SendSelectKS($newPath);
	}
	else {
		SendUpdate();
	}
	TermEnd($e);
}
else {
	my @fpr=split(/\*/,GetFirstPassResult().'*x');
	my $pl=$fpr[0];
	my $ql=$fpr[1];
	my $rp=$fpr[2];
	my $rq=$fpr[3];
	my @newTgt=();
	my $e=0;
	for (my $i=0; $i<@tgt; $i++) {
		my ($n,$d)=fileparse($tgt[$i]);
		$n=$rp.substr($n,$pl,length($n)-$pl-$ql).$rq;
		$n=catfile($d,$n);
		push(@newTgt,$n);
		my $e=TermRun(
			"mv",
			($Config{'osname'} eq 'linux' or $Config{'osname'} eq 'cygwin') ? (
				"-v"
			)
			: (
			),
			"--",
			$tgt[$i],
			$n
		);
		if ($e) { last; }
	}
	$e|=TermSync();
	if (@newTgt > 0) {
		SendSelectKS(@newTgt);
	}
	else {
		SendUpdate();
	}
	TermEnd($e);
}
