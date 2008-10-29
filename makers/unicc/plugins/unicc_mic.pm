#-------------------------------------------------------------------------------
# unicc_mic.pm
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
use File::Spec::Functions;
BEGIN { unicc->import(); }


sub MakeObjectFileName
{
	my $name=$_[1];
	my $type=$_[2];

	return $type eq 'rc' ? "$name.res" : "$name.obj";
}


sub MakeTargetFileName
{
	my $name=$_[1];
	my $type=$_[2];

	return
		$type eq 'lib' ? "$name.lib"
		: $type eq 'dynlib' ? "$name.dll"
		: "$name.exe"
	;
}


sub MakePossibleLibFileNames
{
	my $name=$_[1];

	return [ "$name.lib" ];
}


sub Compile
{
	my $index=$_[1];
	my $srcType=GetSrcTypes->[$index];
	my $isCpp=($srcType eq 'cpp');
	my @args=();

	if ($srcType eq 'rc') {
		push(@args,"rc");
		foreach my $s (@{GetIncSearchDirs()}) { push(@args,"/I$s"); }
		foreach my $s (@{GetDefines()}) { push(@args,"/D$s"); }
		push(@args,"/Fo".GetObjFiles->[$index]);
		push(@args,GetSrcFiles->[$index]);
	}
	else {
		push(@args,"cl");
		push(@args,"/nologo");
		if (HaveDebug) { push(@args,"/Zi"); }
		push(@args,"/O2");
		push(@args,"/GS-"); # Otherwise bufferoverflowu.lib may be required.
		if (HaveRtti) { push(@args,"/GR"); }
		if (HaveExceptions) { push(@args,"/EHsc"); }
		if (HaveDebug) { push(@args,"/MDd"); }
		else { push(@args,"/MD"); }
		push(@args,"/W4");
		push(@args,"/wd4100");
		push(@args,"/wd4244");
		push(@args,"/wd4245");
		push(@args,"/wd4267");
		push(@args,"/wd4290");
		push(@args,"/wd4310");
		push(@args,"/wd4345");
		push(@args,"/wd4355");
		push(@args,"/wd4511");
		push(@args,"/wd4512");
		push(@args,"/wd4610");
		foreach my $s (@{GetIncSearchDirs()}) { push(@args,"/I$s"); }
		foreach my $s (@{GetDefines()}) { push(@args,"/D$s"); }
		push(@args,"/c");
		push(@args,GetSrcFiles->[$index]);
		push(@args,"/Fo".GetObjFiles->[$index]);
	}

	return PrintAndRun(@args);
}


sub Link
{
	my $type=GetTgtType;
	my @args=();

	my $defFile;
	if ($type eq 'dynlib') {
		$defFile=catfile(GetObjDir,GetTgtName.".def");
		print("Creating DEF-File: $defFile\n");
		my $dfh;
		open($dfh,">",$defFile);
		print($dfh "LIBRARY ".GetTgtName."\n");
		print($dfh "EXPORTS\n");
		my %exports;
		my $n=0;
		my $objFiles=GetObjFiles;
		my $srcTypes=GetSrcTypes;
		for (my $i=0; $i<=$#{$objFiles}; $i++) {
			if ($srcTypes->[$i] eq 'rc') { next; }
			my @dump=`dumpbin /SYMBOLS $objFiles->[$i]`;
			if ($? != 0) { exit(1); }
			for (my $j=0; $j<@dump; $j++) {
				my $str=$dump[$j];
				my $k=index($str,"SECT");
				if ($k<0) { next; }
				$k=index($str,"()",$k+4);
				if ($k<0) { next; }
				$k=index($str,"External",$k+2);
				if ($k<0) { next; }
				$k=index($str,"|",$k+8);
				if ($k<0) { next; }
				$str=substr($str,$k+1);
				if (index($str,"scalar deleting destructor")>=0) { next; } #???
				if (index($str,"vector deleting destructor")>=0) { next; } #???
				while (ord($str)<=32) { $str=substr($str,1); }
				$k=0;
				while ($k<length($str) and ord(substr($str,$k,1))>32) { $k++; }
				$str=substr($str,0,$k);
				if (substr($str,0,1) eq "_") { $str=substr($str,1); }
				if (exists $exports{$str}) { next; }
				$exports{$str}=1;
				$n++;
				print($dfh " $str \@$n\n");
			}
		}
		close($dfh);
	}

	if ($type eq 'lib') {
		push(@args,"lib");
		push(@args,"/NOLOGO");
		push(@args,(@{GetObjFiles()}));
		push(@args,"/OUT:".GetTgtFile);
	}
	else {
		push(@args,"link");
		push(@args,"/NOLOGO");
		if (HaveDebug) { push(@args,"/DEBUG"); }
		if ($type eq 'cexe') { push(@args,"/SUBSYSTEM:CONSOLE"); }
		if ($type eq 'wexe') { push(@args,"/SUBSYSTEM:WINDOWS"); }
		if ($type eq 'dynlib') { push(@args,"/DLL"); }
		push(@args,"/INCREMENTAL:NO");
		push(@args,"/FIXED:NO"); #???
		foreach my $s (@{GetLibSearchDirs()}) { push(@args,"/LIBPATH:$s"); }
		foreach my $s (@{GetLinkNames()}) { push(@args,"$s.lib"); }
		my $objFiles=GetObjFiles;
		my $srcTypes=GetSrcTypes;
		my @resFiles=();
		for (my $i=0; $i<=$#{$objFiles}; $i++) {
			if ($srcTypes->[$i] eq 'rc') { push(@resFiles,$objFiles->[$i]); }
			else { push(@args,$objFiles->[$i]); }
		}
		push(@args,(@resFiles));
		push(@args,"/OUT:".GetTgtFile);
		if ($type eq 'dynlib') {
			push(@args,"/DEF:$defFile");
			push(@args,"/IMPLIB:".catfile(GetLibDir,GetTgtName.".lib"));
		}
	}

	return PrintAndRun(@args);
}


return 1;
