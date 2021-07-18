#!/usr/bin/perl
#-------------------------------------------------------------------------------
# PkgConfig.pl
#
# Copyright (C) 2021 Oliver Hamann.
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
		"  perl $0 <package> [<package>...]\n"
	);
	exit(1);
}

my $cmd="pkg-config --cflags --libs";
for (my $i=0; $i<@ARGV; $i++) {
	$cmd.=" ".$ARGV[$i];
}

my $str=readpipe($cmd);
if (!$str) {
	exit(1);
}

my @arr=split(/\s+/,$str);

for (my $i=0; $i<@arr; $i++) {
	if ($arr[$i] =~ '^-I(.+)$') {
		print("--inc-search-dir\n$1\n");
	}
	elsif ($arr[$i] =~ '^-L(.+)$') {
		print("--lib-search-dir\n$1\n");
	}
	elsif ($arr[$i] =~ '^-Wl,-(R,|R|rpath,|rpath=)([^,=].*)$') {
		print("--runtime-lib-search-dir\n$2\n");
	}
	elsif ($arr[$i] =~ '^-Wl,-(R|rpath)$' && $i+1<@arr && $arr[$i+1] =~ '^-Wl,(.+)$') {
		print("--runtime-lib-search-dir\n$1\n");
		$i++;
	}
	elsif ($arr[$i] =~ '^-l(.+)$') {
		print("--link\n$1\n");
	}
	elsif ($arr[$i] =~ '^-D(.+)$') {
		print("--def\n$1\n");
	}
}
