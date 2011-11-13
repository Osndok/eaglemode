#-------------------------------------------------------------------------------
# unicc_gnu.pm
#
# Copyright (C) 2006-2011 Oliver Hamann.
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

package unicc_plugin;

use strict;
use warnings;
use Config;
use File::Basename;
use File::Spec::Functions;
BEGIN { unicc->import(); }


my $GccVersion;
my $IsWin;
my $IsCygwin;
my $IsWinOrCygwin;
my $IsDarwin;

{
	my $s=`gcc -dumpversion`; # Examples: "4.3", "2.95.3"
	if ($? != 0) { exit(1); }
	for (;; $s=substr($s,1)) {
		if (length($s)==0) { die("Cannot determine GCC version, stopped"); }
		if (ord($s)>=48 and ord($s)<=57) { last; }
	}
	my $l;
	for ($l=1; $l<length($s); $l++) {
		if (ord(substr($s,$l))<48 or ord(substr($s,$l))>57) { last; }
	}
	if (substr($s,$l,1) eq "." and ord(substr($s,$l+1))>=48 and
	    ord(substr($s,$l+1))<=57) {
		for ($l=$l+2; $l<length($s); $l++) {
			if (ord(substr($s,$l))<48 or ord(substr($s,$l))>57) { last; }
		}
	}
	$GccVersion=substr($s,0,$l);
	$IsWin = $Config{'osname'} eq 'MSWin32' ? 1 : 0;
	$IsCygwin = $Config{'osname'} eq 'cygwin' ? 1 : 0;
	$IsWinOrCygwin = $IsWin || $IsCygwin;
	$IsDarwin = $Config{'osname'} eq 'darwin' ? 1 : 0;
}


sub MakeObjectFileName
{
	my $name=$_[1];
	my $type=$_[2];

	return $type eq 'rc' ? "$name.res" : "$name.o";
}


sub MakeTargetFileName
{
	my $name=$_[1];
	my $type=$_[2];

	return
		$type eq 'lib' ? "lib$name.a"
		: $type eq 'dynlib' ? (
			$IsWinOrCygwin ? "$name.dll" :
				$IsDarwin ? "lib$name.dylib" : "lib$name.so"
		) : $IsWinOrCygwin ? "$name.exe" : $name
	;
}


sub MakePossibleLibFileNames
{
	my $name=$_[1];

	return [
		"lib$name.a",
		$IsWinOrCygwin ? ("$name.lib", "lib$name.dll.a") :
			$IsDarwin ? "lib$name.dylib" : "lib$name.so"
	];
}


sub Compile
{
	my $index=$_[1];
	my $srcType=GetSrcTypes->[$index];
	my $isCpp=($srcType eq 'cpp');
	my @args=();

	if ($srcType eq 'rc') {
		push(@args,"windres");
		push(@args,("-O","coff"));
		push(@args,("-I",dirname(GetSrcFiles->[$index])));
		foreach my $s (@{GetIncSearchDirs()}) { push(@args,("-I",$s)); }
		foreach my $s (@{GetDefines()}) { push(@args,("-D",$s)); }
		push(@args,("-i",GetSrcFiles->[$index]));
		push(@args,("-o",GetObjFiles->[$index]));
	}
	else {
		push(@args,"gcc");
		if (HaveDebug) { push(@args,"-g"); }
		push(@args,"-O2");
		if ($IsWinOrCygwin) { push(@args,"-mthreads"); }
		if (!$IsWinOrCygwin) {
			my $tgtType=GetTgtType;
			if ($tgtType eq 'dynlib' or $tgtType eq 'lib') { push(@args,"-fPIC"); }
		}
		if (!HaveRtti and $isCpp) { push(@args,"-fno-rtti"); }
		if (HaveExceptions) { push(@args,"-fexceptions"); }
		else { push(@args,"-fno-exceptions"); }
		if ($GccVersion>=3.0) { push(@args,"-fmessage-length=0"); }
		push(@args,"-Wall");
		if ($GccVersion<4.3) { #??? GCC 4.3.0 does not inline several destructors.
			push(@args,"-Winline");
		}
		push(@args,"-Wpointer-arith");
		push(@args,"-Wredundant-decls");
		push(@args,"-Wsign-compare");
		push(@args,"-Wwrite-strings");
		if ($isCpp) {
			push(@args,"-Wsign-promo");
			push(@args,"-Wsynth");
			if ($GccVersion>=4.0) { push(@args,"-Wno-invalid-offsetof"); }
		}
		foreach my $s (@{GetIncSearchDirs()}) { push(@args,"-I$s"); }
		foreach my $s (@{GetDefines()}) { push(@args,"-D$s"); }
		push(@args,"-c");
		push(@args,GetSrcFiles->[$index]);
		push(@args,"-o");
		push(@args,GetObjFiles->[$index]);
	}

	return PrintAndRun(@args);
}


sub Link
{
	my $type=GetTgtType;
	my @args=();

	if ($type eq 'lib') {
		push(@args,"ar");
		push(@args,"rcs");
		push(@args,GetTgtFile);
		push(@args,(@{GetObjFiles()}));
	}
	else {
		push(@args,"gcc");
		if (HaveDebug) { push(@args,"-g"); }
		if ($type eq 'dynlib') {
			push(@args,$IsDarwin ? "-dynamiclib" : "-shared");
		}
		if ($IsWinOrCygwin) {
			push(@args,"-mthreads");
			push(@args,"-shared-libgcc");
		}
		if ($IsWin and $type eq 'wexe') { push(@args,"-mwindows"); }
		foreach my $s (@{GetLibSearchDirs()}) { push(@args,"-L$s"); }
		push(@args,(@{GetObjFiles()}));
		foreach my $s (@{GetLinkNames()}) { push(@args,"-l$s"); }
		if ($IsCygwin && -e "/lib/libcygipc.a") { push(@args,"-lcygipc"); }
		if (HaveMath) { push(@args,"-lm"); }
		if (HaveCppLib) { push(@args,"-lstdc++"); }
		push(@args,"-o");
		push(@args,GetTgtFile);
		if ($IsWinOrCygwin and $type eq "dynlib") {
			push(@args,"-Xlinker");
			push(@args,"--out-implib");
			push(@args,"-Xlinker");
			push(@args,catfile(GetLibDir,"lib".GetTgtName.".dll.a"));
		}
	}

	return PrintAndRun(@args);
}


return 1;
