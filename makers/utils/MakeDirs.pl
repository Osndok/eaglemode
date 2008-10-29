#!/usr/bin/perl
#-------------------------------------------------------------------------------
# MakeDirs.pl
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

if (@ARGV<1) {
	print(
		"Usage:\n".
		"  perl $0 <dir> [<dir>...]\n"
	);
	exit(1);
}

for (my $i=0; $i<@ARGV; $i++) {
	my $path=$ARGV[$i];
	if (!-e($path)) {
		print("Making directory: $path\n");
		if (!mkdir($path)) {
			print("$!\n");
			exit(1);
		}
	}
}
