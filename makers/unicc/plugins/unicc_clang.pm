#-------------------------------------------------------------------------------
# unicc_clang.pm
#
# Copyright (C) 2018,2020-2022,2024 Oliver Hamann.
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


my $IsWin;
my $IsCygwin;
my $IsWinOrCygwin;
my $IsDarwin;
my $IsFreeBSD;

{
	$IsWin = $Config{'osname'} eq 'MSWin32' ? 1 : 0;
	$IsCygwin = $Config{'osname'} eq 'cygwin' ? 1 : 0;
	$IsWinOrCygwin = $IsWin || $IsCygwin;
	$IsDarwin = $Config{'osname'} eq 'darwin' ? 1 : 0;
	$IsFreeBSD = $Config{'osname'} eq 'freebsd' ? 1 : 0;
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
		push(@args,"clang");
		if (HaveDebug) { push(@args,"-g"); }
		push(@args,"-O2");
		if ($isCpp) {
			push(@args,"-std=c++11");
		}
		if ($IsWinOrCygwin) { push(@args,"-mthreads"); }
		if (!$IsWinOrCygwin) {
			my $tgtType=GetTgtType;
			if ($tgtType eq 'dynlib' or $tgtType eq 'lib') { push(@args,"-fPIC"); }
		}
		if (!HaveRtti and $isCpp) { push(@args,"-fno-rtti"); }
		if (HaveExceptions) { push(@args,"-fexceptions"); }
		else { push(@args,"-fno-exceptions"); }
		push(@args,"-fmessage-length=0");
		push(@args,"-Weverything");
		push(@args,"-Wno-c++98-compat");
		push(@args,"-Wno-c++98-compat-pedantic");
		push(@args,"-Wno-cast-align");
		push(@args,"-Wno-cast-qual");
		push(@args,"-Wno-comma");
		push(@args,"-Wno-covered-switch-default");
		push(@args,"-Wno-disabled-macro-expansion");
		push(@args,"-Wno-documentation");
		push(@args,"-Wno-documentation-unknown-command");
		push(@args,"-Wno-exit-time-destructors");
		push(@args,"-Wno-float-equal");
		push(@args,"-Wno-global-constructors");
		push(@args,"-Wno-implicit-fallthrough");
		push(@args,"-Wno-implicit-int-float-conversion");
		push(@args,"-Wno-invalid-offsetof");
		push(@args,"-Wno-missing-noreturn");
		push(@args,"-Wno-missing-prototypes");
		push(@args,"-Wno-nested-anon-types");
		push(@args,"-Wno-old-style-cast");
		push(@args,"-Wno-padded");
		push(@args,"-Wno-reserved-id-macro");
		push(@args,"-Wno-shadow");
		push(@args,"-Wno-shorten-64-to-32");
		push(@args,"-Wno-sign-conversion");
		push(@args,"-Wno-strict-prototypes");
		push(@args,"-Wno-suggest-destructor-override");
		push(@args,"-Wno-suggest-override");
		push(@args,"-Wno-switch-enum");
		push(@args,"-Wno-unknown-warning-option");
		push(@args,"-Wno-unsafe-buffer-usage");
		push(@args,"-Wno-unused-but-set-variable");
		push(@args,"-Wno-unused-parameter");
		push(@args,"-Wno-zero-as-null-pointer-constant");
		foreach my $s (@{GetIncSearchDirs()}) { push(@args,"-I$s"); }
		if ($IsFreeBSD) { push(@args,"-I/usr/local/include"); }
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
		push(@args,"clang");
		if (HaveDebug) { push(@args,"-g"); }
		if ($type eq 'dynlib') {
			push(@args,$IsDarwin ? "-dynamiclib" : "-shared");
		}
		if ($IsWinOrCygwin) {
			push(@args,"-mthreads");
		}
		if ($IsWinOrCygwin and ($type eq 'cexe' or $type eq 'wexe')) {
			push(@args,"-Wl,--stack=8388608");
		}
		if ($IsWin and $type eq 'wexe') { push(@args,"-mwindows"); }
		foreach my $s (@{GetLibSearchDirs()}) { push(@args,"-L$s"); }
		if ($IsFreeBSD) { push(@args,"-L/usr/local/lib"); }
		foreach my $s (@{GetRuntimeLibSearchDirs()}) { push(@args,"-Wl,-R,$s"); }
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
