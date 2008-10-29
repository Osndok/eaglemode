#!/usr/bin/perl
#-------------------------------------------------------------------------------
# MakeCopies.pl
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

use strict;
use warnings;
use File::Spec::Functions;
use File::Copy;
use File::stat;


if (@ARGV<2 || @ARGV&1!=0) {
	print(
		"Usage:\n".
		"  perl $0 <source> <target> [<source> <target> ...]\n"
	);
	exit(1);
}


sub MakeFileCopy
{
	my $src=shift;
	my $tgt=shift;

	if (!-e($src)) {
		die "No such file or directory: \"$src\", stopped";
	}

	my $srcstat=stat($src);

	if (-e($tgt)) {
		my $tgtstat=stat($tgt);
		if ($tgtstat->mtime>=$srcstat->mtime) {
			return;
		}
	}

	print("Copying: $src -> $tgt\n");

	if (!copy($src,$tgt)) {
		die "Failed to copy \"$src\" to \"$tgt\": $!, stopped";
	}

	if (!chmod($srcstat->mode,$tgt)) {
		die "Failed to chmod \"$tgt\": $!, stopped";
	}
}


sub MakeTreeCopy
{
	my $src=shift;
	my $tgt=shift;

	if (!-e($src)) {
		die "No such file or directory: \"$src\", stopped";
	}
	if (!-d($src)) {
		die "Not a directory: \"$src\", stopped";
	}

	if (!-e($tgt)) {
		print("Making directory: $tgt\n");
		if (!mkdir($tgt)) {
			print("$!\n");
			exit(1);
		}
	}

	my $dh;
	my @names=();
	opendir($dh,$src);
	while (defined(my $name=readdir($dh))) {
		if ($name ne '.' && $name ne '..') {
			push(@names,$name);
		}
	}
	closedir($dh);
	@names=sort(@names);

	for (my $i=0; $i<@names; $i++) {
		my $subsrc=catfile($src,$names[$i]);
		my $subtgt=catfile($tgt,$names[$i]);
		if (-d($subsrc)) {
			MakeTreeCopy($subsrc,$subtgt);
		}
		else {
			MakeFileCopy($subsrc,$subtgt);
		}
	}
}


for (my $i=0; $i<@ARGV; $i+=2) {
	my $src=$ARGV[$i];
	my $tgt=$ARGV[$i+1];
	if (-d($src)) {
		MakeTreeCopy($src,$tgt);
	}
	else {
		MakeFileCopy($src,$tgt);
	}
}
