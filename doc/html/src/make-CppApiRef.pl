#!/usr/bin/perl
#-------------------------------------------------------------------------------
# make-CppApiRef.pl
#
# Copyright (C) 2010 Oliver Hamann.
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
use Cwd;
use File::Basename;
use File::Spec::Functions;
use File::stat;

# Set current directory to the root directory of the project.
chdir(dirname($0));
chdir('..');
chdir('..');
chdir('..');

# Have output directory
my $outDir='doc/html/CppApiRef';
if (!-d $outDir) { mkdir($outDir) or die; }

# Check if the output is up-to-date.
my $inDir='include/emCore';
my @moreInFiles=(
	'doc/html/src/CppApiRef-config.py',
	'doc/html/src/cpptohtml.py'
);
my $anOutFile=$outDir.'/index.html';
if (-e $anOutFile) {
	my $st1=stat($anOutFile);
	my $upToDate=1;
	my $dh;
	opendir($dh,$inDir);
	while (defined(my $nm=readdir($dh))) {
		if ($nm ne '.' && $nm ne '..') {
			my $st2=stat(catfile($inDir,$nm));
			if ($st1->mtime<$st2->mtime) {
				$upToDate=0;
				last;
			}
		}
	}
	closedir($dh);
	if ($upToDate) {
		foreach my $f (@moreInFiles) {
			my $st2=stat($f);
			if ($st1->mtime<$st2->mtime) {
				$upToDate=0;
				last;
			}
		}
	}
	if ($upToDate) { exit(0); }
}

# Do it.
system(
	'python',
	'doc/html/src/cpptohtml.py',
	'doc/html/src/CppApiRef-config.py',
	'include',
	$outDir
)==0 || exit(1);
