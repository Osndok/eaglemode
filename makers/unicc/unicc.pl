#!/usr/bin/perl
#-------------------------------------------------------------------------------
# unicc.pl
#
# Copyright (C) 2006-2008 Oliver Hamann.
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

package unicc;

use strict;
use warnings;
use Config;
use File::Basename;
use File::Spec::Functions;
use File::stat;
use Exporter;

our @ISA=qw(Exporter);


#============================= Function exports ==============================

our @EXPORT=qw(
	&GetTgtType
	&GetTgtName
	&GetObjDir
	&GetBinDir
	&GetLibDir
	&GetLibSearchDirs
	&GetIncSearchDirs
	&GetLinkNames
	&HaveMath
	&HaveRtti
	&HaveExceptions
	&HaveDebug
	&GetDefines
	&GetSrcFiles
	&GetSrcTypes
	&HaveCppLib
	&GetObjFiles
	&GetTgtFile
	&PrintAndRun
);


#=============================== Parse arguments ===============================

my $Compiler="gnu";
my $Cpus=1;
my $Type="cexe";
my $Name="unnamed";
my $ObjDir=".";
my $BinDir=".";
my $LibDir=".";
my @LibSearchDirs=();
my @IncSearchDirs=();
my @Links=();
my $Math=0;
my $Rtti=0;
my $Exceptions=0;
my $Debug=0;
my @Defs=();
my @Srcs=();

for (my $i=0; $i<@ARGV; $i++) {
	if ($ARGV[$i] eq "--compiler" and $i+1<@ARGV) {
		$Compiler=$ARGV[++$i];
	}
	elsif ($ARGV[$i] eq "--cpus" and $i+1<@ARGV) {
		$Cpus=$ARGV[++$i];
	}
	elsif ($ARGV[$i] eq "--type" and $i+1<@ARGV) {
		$Type=$ARGV[++$i];
		if ($Type !~ /^(cexe|wexe|lib|dynlib)$/) {
			die("Illegal value for option --type, stopped");
		}
	}
	elsif ($ARGV[$i] eq "--name" and $i+1<@ARGV) {
		$Name=$ARGV[++$i];
	}
	elsif ($ARGV[$i] eq "--obj-dir" and $i+1<@ARGV) {
		$ObjDir=$ARGV[++$i];
	}
	elsif ($ARGV[$i] eq "--bin-dir" and $i+1<@ARGV) {
		$BinDir=$ARGV[++$i];
	}
	elsif ($ARGV[$i] eq "--lib-dir" and $i+1<@ARGV) {
		$LibDir=$ARGV[++$i];
		push(@LibSearchDirs,$LibDir);
	}
	elsif ($ARGV[$i] eq "--lib-search-dir" and $i+1<@ARGV) {
		push(@LibSearchDirs,$ARGV[++$i]);
	}
	elsif ($ARGV[$i] eq "--inc-search-dir" and $i+1<@ARGV) {
		push(@IncSearchDirs,$ARGV[++$i]);
	}
	elsif ($ARGV[$i] eq "--link" and $i+1<@ARGV) {
		push(@Links,$ARGV[++$i]);
	}
	elsif ($ARGV[$i] eq "--math") {
		$Math=1;
	}
	elsif ($ARGV[$i] eq "--rtti") {
		$Rtti=1;
	}
	elsif ($ARGV[$i] eq "--exceptions") {
		$Exceptions=1;
	}
	elsif ($ARGV[$i] eq "--debug") {
		$Debug=1;
	}
	elsif ($ARGV[$i] eq "--def" and $i+1<@ARGV) {
		push(@Defs,$ARGV[++$i]);
	}
	elsif ($ARGV[$i]=~/^--/) {
		die("Unknown option '$ARGV[$i]', stopped");
	}
	else {
		push(@Srcs,$ARGV[$i]);
	}
}

if (@Srcs <= 0) {
	die("No source files, stopped");
}


#================ On Windows, translate '/' to '\' in all paths ================

if ($Config{'osname'} eq 'MSWin32') {
	$ObjDir=~tr/\//\\/;
	$BinDir=~tr/\//\\/;
	$LibDir=~tr/\//\\/;
	for (my $i=0; $i<@LibSearchDirs; $i++) { $LibSearchDirs[$i]=~tr/\//\\/; }
	for (my $i=0; $i<@IncSearchDirs; $i++) { $IncSearchDirs[$i]=~tr/\//\\/; }
	for (my $i=0; $i<@Srcs; $i++) { $Srcs[$i]=~tr/\//\\/; }
}


#======================= Determine types of source files =======================

my @SrcNames=();
my @SrcTypes=(); # Each element is either "c", "cpp" or "rc".
my $AnyCpp=0;

for (my $i=0; $i<@Srcs; $i++) {
	my ($n,$d,$t)=fileparse($Srcs[$i],qr/\.[^.]*/);
	if ($t eq '.c') {
		$t='c';
	}
	elsif ($t =~ /^(\.cpp|\.cxx|\.cc)$/) {
		$t='cpp';
		$AnyCpp=1;
	}
	elsif ($t eq '.rc') {
		$t='rc';
	}
	else {
		die("Source file '$Srcs[$i]' has illegal suffix, stopped");
	}
	push(@SrcNames,$n);
	push(@SrcTypes,$t);
}


#========================== Load the compiler module ===========================

my $Module="unicc_plugin";
my $ModuleFile=dirname($0)."/plugins/unicc_$Compiler.pm";

if (!-e $ModuleFile) {
	die("Illegal value for --compiler option. \"$ModuleFile\" does not exist. Stopped");
}
do($ModuleFile);
if ($@) { die("$@"); }


#========================= Calculate output file paths =========================

my @Objs=();
my $Tgt;

for (my $i=0; $i<@Srcs; $i++) {
	push(
		@Objs,
		catfile(
			$ObjDir,
			$Module->MakeObjectFileName($SrcNames[$i],$SrcTypes[$i])
		)
	);
}

$Tgt=catfile(
	($Type eq "cexe" or $Type eq "wexe") ? $BinDir : $LibDir,
	$Module->MakeTargetFileName($Name,$Type)
);


#=========== Private Helpers: GetDirectIncludes and GetSrcUpdateTime ===========

sub GetDirectIncludes
{
	my $srcFile=shift;

	my $fh;
	open($fh,$srcFile);
	my @r=();
	while (defined(my $s=readline($fh))) {
		if ($s !~ /^[ \t]*\#[ \t]*include[ \t]*(["<])(.*)$/) { next; }
		my $q=$1;
		$s=$2;
		if ($q eq '<') { $q='>'; }
		my $i=index($s,$q);
		if ($i<=0) { next; }
		$s=substr($s,0,$i);
		if ($q eq '"') {
			my ($f,$d)=fileparse($srcFile);
			my $p=catfile($d,$s);
			if (-f $p) { push(@r,$p); next; }
		}
		foreach my $d (@IncSearchDirs) {
			my $p=catfile($d,$s);
			if (-f $p) { push(@r,$p); last; }
		}
	}
	close($fh);
	return @r;
}


sub GetSrcUpdateTime
{
	my $srcFile=shift;
	my $cacheRef=shift;

	if (defined(my $t=$$cacheRef{$srcFile})) { return $t; }

	# Elements of @fArray and %fHash are references to arrays with:
	# [ <file>, <time>, <todo flag>, <includer ref>, <includer ref>, ... ]
	my $fsrc=[$srcFile,0,1];
	my @fArray=($fsrc);
	my %fHash=($srcFile => $fsrc);
	for (my $i=0; $i<@fArray; $i++) {
		my $f=$fArray[$i];
		my $ft=stat($f->[0])->mtime;
		foreach my $c (GetDirectIncludes($f->[0])) {
			if (defined(my $ct=$$cacheRef{$c})) {
				if ($ft<$ct) { $ft=$ct; }
			}
			elsif (defined(my $cf=$fHash{$c})) {
				push(@$cf,$f);
			}
			else {
				my $cf=[$c,0,1,$f];
				push(@fArray,$cf);
				$fHash{$c}=$cf;
			}
		}
		$f->[1]=$ft;
	}
	while (defined(my $f=pop(@fArray))) {
		$f->[2]=0;
		my $ft=$f->[1];
		for (my $i=3; $i<@$f; $i++) {
			my $fp=$f->[$i];
			if ($fp->[1]<$ft) {
				$fp->[1]=$ft;
				if ($fp->[2] == 0) {
					$fp->[2]=1;
					push(@fArray,$fp);
				}
			}
		}
	}
	foreach my $f (values(%fHash)) {
		$$cacheRef{$f->[0]}=$f->[1];
	}
	return $fsrc->[1];
}


#==================== Compile source files to object files =====================

if ($Cpus > 1) {
	# The modules "threads" and "threads::shared" could be unavailable.
	eval(
		'use threads;                                            '."\n".
		'use threads::shared;                                    '."\n".
		'                                                        '."\n".
		'my @ExtraThreads=();                                    '."\n".
		'my %SrcUpdateTimeCache : shared;                        '."\n".
		'my $NextObj : shared = 0;                               '."\n".
		'my $AnyError : shared = 0;                              '."\n".
		'                                                        '."\n".
		'sub ThreadFunc                                          '."\n".
		'{                                                       '."\n".
		' my ($i,$t);                                            '."\n".
		'                                                        '."\n".
		' for (;;) {                                             '."\n".
		'  { lock($NextObj); $i=$NextObj++; }                    '."\n".
		'  if ($i>=@Srcs or $AnyError) { return; }               '."\n".
		'  if (-f $Objs[$i]) {                                   '."\n".
		'   {                                                    '."\n".
		'    lock(%SrcUpdateTimeCache);                          '."\n".
		'    $t=GetSrcUpdateTime($Srcs[$i],\%SrcUpdateTimeCache);'."\n".
		'   }                                                    '."\n".
		'   if (stat($Objs[$i])->mtime>=$t) { next; }            '."\n".
		'  }                                                     '."\n".
		'  if (!$Module->Compile($i)) { $AnyError=1; return; }   '."\n".
		' }                                                      '."\n".
		'}                                                       '."\n".
		'                                                        '."\n".
		'for (my $i=1; $i<$Cpus && $i<@Srcs; $i++) {             '."\n".
		'  push(@ExtraThreads,threads->new(\&ThreadFunc));       '."\n".
		'}                                                       '."\n".
		'                                                        '."\n".
		'ThreadFunc();                                           '."\n".
		'                                                        '."\n".
		'foreach my $t (@ExtraThreads) { $t->join(); }           '."\n".
		'                                                        '."\n".
		'if ($AnyError) { exit(1); }                             '."\n"
	);
	if ($@) {
		print(STDERR "unicc: warning: $@ - retrying with just one cpu...\n");
		$Cpus=1;
	}
}

if ($Cpus <= 1) {
	my %srcUpdateTimeCache;
	for (my $i=0; $i<@Srcs; $i++) {
		if (
			!-f $Objs[$i] or
			stat($Objs[$i])->mtime<GetSrcUpdateTime($Srcs[$i],\%srcUpdateTimeCache)
		) {
			if (!$Module->Compile($i)) { exit(1); }
		}
	}
}


#=========================== Target file up-to-date? ===========================

if (-e $Tgt) {
	TEST_LIBS: {
		my $t=stat($Tgt)->mtime;
		foreach my $o (@Objs) {
			if (stat($o)->mtime>$t) { last TEST_LIBS; }
		}
		foreach my $l (@Links) {
			my $names=$Module->MakePossibleLibFileNames($l);
			foreach my $d (@LibSearchDirs) {
				foreach my $n (@{$names}) {
					my $f=catfile($d,$n);
					if (-e $f and stat($f)->mtime>$t) { last TEST_LIBS; }
				}
			}
		}
		exit(0);
	}
}


#============================= Produce target file =============================

if (-e $Tgt) { unlink($Tgt); }

if (!$Module->Link()) { exit(1); }


#==================== Implementations of exported functions ====================

sub GetTgtType { return $Type; }
sub GetTgtName { return $Name; }
sub GetObjDir { return $ObjDir; }
sub GetBinDir { return $BinDir; }
sub GetLibDir { return $LibDir; }
sub GetLibSearchDirs { return \@LibSearchDirs; }
sub GetIncSearchDirs { return \@IncSearchDirs; }
sub GetLinkNames { return \@Links; }
sub HaveMath { return $Math; }
sub HaveRtti { return $Rtti; }
sub HaveExceptions { return $Exceptions; }
sub HaveDebug { return $Debug; }
sub GetDefines { return \@Defs; }
sub GetSrcFiles { return \@Srcs; }
sub GetSrcTypes { return \@SrcTypes; }
sub HaveCppLib { return $AnyCpp; }
sub GetObjFiles { return \@Objs; }
sub GetTgtFile { return $Tgt; }

sub PrintAndRun
{
	print("@_\n");
	return system(@_)!=0 ? 0 : 1;
}
