#-------------------------------------------------------------------------------
# unicc_sun.pm
#
# Copyright (C) 2007-2008 Oliver Hamann.
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
use File::Basename;
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
		if ($isCpp) { push(@args,"CC"); }
		else        { push(@args,"cc"); }
		if (HaveDebug) { push(@args,"-g"); }
		push(@args,"-mt");
		push(@args,"-fast");
		if (GetTgtType eq 'dynlib') { push(@args,"-G"); }
		if ($isCpp) {
			if (!HaveRtti) { push(@args,'-features=no%rtti'); }
			if (!HaveExceptions) { push(@args,'-features=no%except'); }
		}
		push(@args,'+w');
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
		push(@args,"CC");
		push(@args,"-xar");
		push(@args,"-o");
		push(@args,GetTgtFile);
		push(@args,(@{GetObjFiles()}));
	}
	else {
		push(@args,"CC"); #??? or cc if no C++ files?
		if (HaveDebug) { push(@args,"-g"); }
		push(@args,"-mt");
		if ($type eq 'dynlib') { push(@args,"-G"); }
		foreach my $s (@{GetLibSearchDirs()}) { push(@args,"-L$s"); }
		push(@args,(@{GetObjFiles()}));
		foreach my $s (@{GetLinkNames()}) { push(@args,"-l$s"); }
		push(@args,"-o");
		push(@args,GetTgtFile);
		if ($type eq 'dynlib') {
			push(@args,"-norunpath");
			push(@args,"-h");
			push(@args,basename(GetTgtFile));
		}
		else {
			push(@args,"-norunpath"); #???
		}
	}

	return PrintAndRun(@args);
}


return 1;
