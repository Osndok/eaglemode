#-------------------------------------------------------------------------------
# unicc_bor.pm
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
		push(@args,"brc32");
		push(@args,"-r");
		foreach my $s (@{GetIncSearchDirs()}) { push(@args,"-i$s"); }
		foreach my $s (@{GetDefines()}) { push(@args,"-d$s"); }
		push(@args,"-fo".GetObjFiles->[$index]);
		push(@args,GetSrcFiles->[$index]);
	}
	else {
		push(@args,"bcc32");
		push(@args,"-q");
		push(@args,"-O2");
		if (HaveDebug) {
			push(@args,"-y");
			push(@args,"-v");
		}
		if (HaveRtti) { push(@args,"-RT"); }
		else { push(@args,"-RT-"); }
		if (HaveExceptions) { push(@args,"-x"); }
		else { push(@args,"-x-"); }
		push(@args,"-j4");
		push(@args,"-w");
		push(@args,"-w-par");
		push(@args,"-w-aus");
		my $tgtType=GetTgtType;
		if ($tgtType eq 'cexe') { push(@args,"-tWC"); }
		elsif ($tgtType eq 'wexe') { push(@args,"-tW"); }
		elsif ($tgtType eq 'dynlib') { push(@args,"-tWD"); }
		push(@args,"-tWM"); # multi-threaded
		push(@args,"-tWR");
		foreach my $s (@{GetIncSearchDirs()}) { push(@args,"-I$s"); }
		foreach my $s (@{GetDefines()}) { push(@args,"-D$s"); }
		push(@args,"-o".GetObjFiles->[$index]);
		push(@args,"-c");
		push(@args,GetSrcFiles->[$index]); # Must be the last
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
			my @dump=`tdump -q -m $objFiles->[$i]`;
			if ($? != 0) { exit(1); }
			for (my $j=0; $j<@dump; $j++) {
				my $str=$dump[$j];
				if (index($str,"virtual(_TEXT)")<0) { next; }
				while (ord($str)<=32) { $str=substr($str,1); }
				if (index($str,"Name:")!=0) { next; }
				$str=substr($str,5);
				while (ord($str)<=32) { $str=substr($str,1); }
				my $k=ord($str);
				if ($k<48 or $k>57) { next; }
				for (;;) {
					$str=substr($str,1);
					$k=ord($str);
					if ($k<48 or $k>57) { last; }
				}
				if (index($str,":")!=0) { next; }
				$str=substr($str,1);
				while (ord($str)<=32) { $str=substr($str,1); }
				if (index($str,"'")!=0) { next; }
				$str=substr($str,1);
				$k=index($str,"'");
				if ($k<0) { next; }
				my $sym=substr($str,0,$k);
				$str=substr($str,$k+1);
				while (ord($str)<=32) { $str=substr($str,1); }
				if (index($str,"virtual(_TEXT)")!=0) { next; }
				my $sym2=$sym;
				if (substr($sym,0,1) eq "_") { $sym2=substr($sym,1); }
				if (exists $exports{$sym}) { next; }
				$exports{$sym}=1;
				$n++;
				print($dfh " $sym \@$n\n");
				if ($sym2 ne $sym) {
					$n++;
					print($dfh " $sym2=$sym \@$n\n");
				}
			}
		}
		close($dfh);
	}

	if ($type eq 'lib') {
		push(@args,"tlib");
		push(@args,"/P64");
		push(@args,GetTgtFile);
		foreach my $o (@{GetObjFiles()}) { push(@args,"+$o"); }
		if (!PrintAndRun(@args)) { return 0; }
	}
	else {
		push(@args,"ilink32");
		push(@args,"-q");
		push(@args,"-w");
		push(@args,"-x");
		push(@args,"-Gn");
		if ($type eq 'cexe') {
			push(@args,"-ap");
			push(@args,"-Tpe");
		}
		if ($type eq 'wexe') {
			push(@args,"-aa");
			push(@args,"-Tpe");
		}
		if ($type eq 'dynlib') {
			push(@args,"-Tpd");
		}
		push(@args,"-S:2048576"); #??? stack size
		if (HaveDebug) { push(@args,"-v"); }
		foreach my $s (@{GetLibSearchDirs()}) { push(@args,"-L$s"); }
		if ($type eq 'cexe')   { push(@args,"c0x32.obj"); }
		if ($type eq 'wexe')   { push(@args,"c0w32.obj"); }
		if ($type eq 'dynlib') { push(@args,"c0d32.obj"); }
		my $objFiles=GetObjFiles;
		my $srcTypes=GetSrcTypes;
		my @resFiles=();
		for (my $i=0; $i<=$#{$objFiles}; $i++) {
			if ($srcTypes->[$i] eq 'rc') { push(@resFiles,$objFiles->[$i]); }
			else { push(@args,$objFiles->[$i]); }
		}
		push(@args,",");
		push(@args,GetTgtFile);
		push(@args,",");
		push(@args,",");
		push(@args,"cw32mti.lib"); # or cw32i.lib if not multi-threaded
		push(@args,"import32.lib");
		foreach my $s (@{GetLinkNames()}) {
			if (
				$s eq "advapi32" or
				$s eq "comdlg32" or
				$s eq "gdi32" or
				$s eq "ole32" or
				$s eq "shell32" or
				$s eq "user32" or
				$s eq "winmm" or
				$s eq "wsock32"
				#??? to be continued...
			) {
				print(STDERR "WARNING: desperated hack: library '$s' ignored!\n");
				next;
			}
			push(@args,"$s.lib");
		}
		push(@args,",");
		if ($type eq 'dynlib') { push(@args,$defFile); }
		push(@args,",");
		push(@args,(@resFiles));
		if (!PrintAndRun(@args)) { return 0; }
		if ($type eq 'dynlib') {
			@args=();
			push(@args,"implib");
			push(@args,catfile(GetLibDir,GetTgtName.".lib"));
			push(@args,GetTgtFile);
			if (!PrintAndRun(@args)) { return 0; }
		}
		if (!HaveDebug) {
			my $tdsFile=catfile(GetBinDir,GetTgtName.".tds");
			if (-e $tdsFile) {
				print("Deleting $tdsFile\n");
				unlink($tdsFile);
			}
		}
	}

	return 1;
}


return 1;
