#-------------------------------------------------------------------------------
# unicc_wat.pm
#
# Copyright (C) 2006-2010 Oliver Hamann.
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
		push(@args,"wrc");
		push(@args,"-q");
		push(@args,"-r");
		push(@args,"-bt=nt");
		push(@args,"-i=".dirname(GetSrcFiles->[$index]));
		foreach my $s (@{GetIncSearchDirs()}) { push(@args,"-i=$s"); }
		foreach my $s (@{GetDefines()}) { push(@args,"-d$s"); }
		push(@args,GetSrcFiles->[$index]);
		push(@args,"-fo=".GetObjFiles->[$index]);
	}
	else {
		push(@args,"wcl386");
		push(@args,"-zq");
		push(@args,"-fr");
		push(@args,"-bt=nt");
		my $tgtType=GetTgtType;
		if ($tgtType eq 'cexe') { push(@args,"-bc"); }
		elsif ($tgtType eq 'wexe') { push(@args,"-bg"); }
		elsif ($tgtType eq 'dynlib') { push(@args,"-bd"); }
		push(@args,"-bm"); # multi-threaded
		push(@args,"-br"); # run-time DLLs
		push(@args,"-mf");
		push(@args,"-6r");
		push(@args,"-otexan");
		if (HaveDebug) { push(@args,"-d2"); }
		if ($isCpp) {
			if (HaveRtti) { push(@args,"-xr"); }
			if (HaveExceptions) { push(@args,"-xs"); }
		}
		push(@args,"-e4");
		push(@args,"-wx");
		push(@args,"-wcd=549");
		push(@args,"-wcd=594");
		push(@args,"-wcd=604");
		push(@args,"-wcd=656");
		push(@args,"-wcd=657");
		push(@args,"-wcd=726");
		push(@args,"-wcd=730");
		foreach my $s (@{GetIncSearchDirs()}) { push(@args,"-i=$s"); }
		foreach my $s (@{GetDefines()}) { push(@args,"-d$s"); }
		push(@args,"-c");
		push(@args,GetSrcFiles->[$index]);
		push(@args,"-fo=".GetObjFiles->[$index]);
	}

	return PrintAndRun(@args);
}


sub Link
{
	my $type=GetTgtType;
	my @args=();

	my $exportsFile;
	if ($type eq 'dynlib') {
		$exportsFile=catfile(GetObjDir,GetTgtName.".lk");
		print("Creating Exports-File: $exportsFile\n");
		my $dfh;
		open($dfh,">",$exportsFile);
		my %exports;
		my $n=0;
		my $objFiles=GetObjFiles;
		my $srcTypes=GetSrcTypes;
		for (my $i=0; $i<=$#{$objFiles}; $i++) {
			if ($srcTypes->[$i] eq 'rc') { next; }
			my @dump=`wdis /a $objFiles->[$i]`;
			if ($? != 0) { return 0; }
			for (my $j=0; $j<@dump; $j++) {
				my $str=$dump[$j];
				my $k=index($str,"PUBLIC");
				if ($k<0) { next; }
				while (ord($str)<=32) { $str=substr($str,1); }
				if (index($str,"PUBLIC")!=0) { next; }
				$str=substr($str,6);
				while (ord($str)<=32) { $str=substr($str,1); }
				my $str2;
				if (substr($str,0,1) eq "`") {
					$str=substr($str,1);
					$k=0;
					while ($k<length($str) and substr($str,$k,1) ne "`") { $k++; }
					$str=substr($str,0,$k);
					if (index($str,"(")<0) { next; }
					if (index($str,")")<0) { next; }
					$str2=$str;
				}
				else {
					while ($k<length($str) and ord(substr($str,$k,1))>32) { $k++; }
					$str=substr($str,0,$k);
					if (substr($str,length($str)-1,1) eq "_") { $str2=substr($str,0,length($str)-1); }
					else { $str2=$str; }
				}
				if (exists $exports{$str}) { next; }
				$exports{$str}=1;
				$n++;
				print($dfh "EXP '$str'.$n\n");
				if ($str2 ne $str) {
					$n++;
					print($dfh "EXP '$str2'.$n='$str'\n");
				}
			}
		}
		close($dfh);
	}

	if ($type eq 'lib') {
		push(@args,"wlib");
		push(@args,"-b");
		push(@args,"-c");
		push(@args,"-n");
		push(@args,"-q");
		push(@args,"-p512");
		push(@args,GetTgtFile);
		foreach my $o (@{GetObjFiles()}) { push(@args,"+$o"); }
	}
	else {
		push(@args,"wlink");
		push(@args,"OP"); push(@args,"Q");
		if (HaveDebug) { push(@args,("DEBUG", "all")); }
		push(@args,("OP", "ST=2048576")); #??? stack size
		foreach my $s (@{GetLibSearchDirs()}) { push(@args,("LIBP", $s)); }
		foreach my $s (@{GetLinkNames()}) { push(@args,("L", $s)); }
		my $objFiles=GetObjFiles;
		my $srcTypes=GetSrcTypes;
		for (my $i=0; $i<=$#{$objFiles}; $i++) {
			if ($srcTypes->[$i] eq 'rc') { push(@args,("OP", "resource=$objFiles->[$i]")); }
			else { push(@args,("F", $objFiles->[$i])); }
		}
		push(@args,("N", GetTgtFile));
		if ($type eq 'cexe') { push(@args,("SYS", "nt")); }
		elsif ($type eq "wexe") { push(@args,("SYS", "nt_win")); }
		else {
			push(@args,("SYS", "nt_dll"));
			push(@args,"\@$exportsFile");
			push(@args,("OP", "IMPLIB=".catfile(GetLibDir,GetTgtName.".lib")));
		}
	}

	return PrintAndRun(@args);
}


return 1;
