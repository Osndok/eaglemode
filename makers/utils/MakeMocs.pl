#!/usr/bin/perl
#-------------------------------------------------------------------------------
# MakeMocs.pl
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
use File::Basename;
use File::Spec::Functions;
use File::stat;

if (@ARGV<2) {
	print(
		"Usage:\n".
		"  perl $0 <source> [source...] <target dir>\n"
	);
	exit(1);
}

my $TargetSuffix='.moc';

my $tgtdir=$ARGV[$#ARGV];

for (my $i=0; $i+1<@ARGV; $i++) {
	my $src=$ARGV[$i];
	my ($f,$d,$s)=fileparse($src,qr/\.[^.]*/);
	if ($s !~ /^(\.cpp|\.cxx|\.cc|\.hpp|\.hxx|\.h)$/) {
		die("Source file '$src' has illegal suffix, stopped");
	}
	my $tgt=catfile($tgtdir,$f.$TargetSuffix);

	if (-e($tgt)) {
		my $srcstat=stat($src);
		my $tgtstat=stat($tgt);
		if ($tgtstat->mtime>=$srcstat->mtime) {
			next;
		}
	}

	my @args=(
		catfile($ENV{'QTDIR'},'bin','moc'),
		$src,
		'-o',
		$tgt
	);
	print("@args\n");
	if (system(@args)!=0) { exit(1); }
}
