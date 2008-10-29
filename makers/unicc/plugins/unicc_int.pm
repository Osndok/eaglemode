#-------------------------------------------------------------------------------
# unicc_int.pm
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

package unicc_plugin;

use strict;
use warnings;
BEGIN { unicc->import(); }


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
		: $type eq 'dynlib' ? "lib$name.so"
		: $name
	;
}


sub MakePossibleLibFileNames
{
	my $name=$_[1];

	return [
		"lib$name.a",
		"lib$name.so"
	];
}


sub Compile
{
	my $index=$_[1];
	my $srcType=GetSrcTypes->[$index];
	my $isCpp=($srcType eq 'cpp');
	my @args=();

	if ($srcType eq 'rc') {
		print(STDERR "rc files not supported\n");
		return 0;
	}
	else {
		push(@args,"icc");
		if (HaveDebug) { push(@args,"-g"); }
		push(@args,"-O2");
		push(@args,"-strict-ansi");
		my $tgtType=GetTgtType;
		if ($tgtType eq 'dynlib' or $tgtType eq 'lib') { push(@args,"-fPIC"); }
		if (!HaveRtti and $isCpp) { push(@args,"-fno-rtti"); }
		if (HaveExceptions) { push(@args,"-fexceptions"); }
		else { push(@args,"-fno-exceptions"); }
		push(@args,"-Wbrief");
		push(@args,"-Wall");
		push(@args,"-wd383,593,810,869,981,1418,1511,1572");
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
		push(@args,"icc");
		if (HaveDebug) { push(@args,"-g"); }
		if ($type eq 'dynlib') { push(@args,"-shared"); }
		foreach my $s (@{GetLibSearchDirs()}) { push(@args,"-L$s"); }
		push(@args,(@{GetObjFiles()}));
		foreach my $s (@{GetLinkNames()}) { push(@args,"-l$s"); }
		if (HaveMath) { push(@args,"-lm"); }
		if (HaveCppLib) { push(@args,"-lstdc++"); }
		push(@args,"-o");
		push(@args,GetTgtFile);
	}

	return PrintAndRun(@args);
}


return 1;
