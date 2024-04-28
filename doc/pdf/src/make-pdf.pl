#!/usr/bin/perl
#-------------------------------------------------------------------------------
# make-pdf.pl
#
# Copyright (C) 2024 Oliver Hamann.
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

# Directories where the images are. If any contained file is younger than a
# target file, then the target file is re-created.
my @ImageDirs = ( 'doc/html/images' );

# Array of files to be made. Each entry is a pair of a source file (html) and
# a target file (pdf).
my $BuildList = [
	[ 'doc/html/index.html', 'doc/pdf/index.pdf' ],
	[ 'doc/html/ChangeLog.html', 'doc/pdf/ChangeLog.pdf' ],
	[ 'doc/html/License.html', 'doc/pdf/License.pdf' ],
	[ 'doc/html/SystemRequirements.html', 'doc/pdf/SystemRequirements.pdf' ],
	[ 'doc/html/InstallAndStart.html', 'doc/pdf/InstallAndStart.pdf' ],
	[ 'doc/html/GeneralUserGuide.html', 'doc/pdf/GeneralUserGuide.pdf' ],
	[ 'doc/html/emFileManUserGuide.html', 'doc/pdf/emFileManUserGuide.pdf' ],
	[ 'doc/html/emFileManCustomization.html', 'doc/pdf/emFileManCustomization.pdf' ],
	[ 'doc/html/AdvancedConfiguration.html', 'doc/pdf/AdvancedConfiguration.pdf' ]
];

# Determine latest modification time of images.
my $MTimeImg=0;
for (my $i=0; $i<@ImageDirs; $i++) {
	my $dir=$ImageDirs[$i];
	my $dh;
	opendir($dh,$dir);
	while (defined(my $nm=readdir($dh))) {
		if ($nm ne '.' && $nm ne '..') {
			my $st=stat(catfile($dir,$nm));
			if ($MTimeImg<$st->mtime) { $MTimeImg=$st->mtime; }
		}
	}
	closedir($dh);
}

# Make the pdf files.
for (my $i=0; $i<=$#{$BuildList}; $i++) {

	my $src=$BuildList->[$i]->[0];
	my $tgt=$BuildList->[$i]->[1];

	if (-e($tgt)) {
		my $srcstat=stat($src);
		my $tgtstat=stat($tgt);
		if (
			$tgtstat->mtime>=$srcstat->mtime &&
			$tgtstat->mtime>=$MTimeImg
		) {
			next;
		}
	}

	my @args= (
		'htmldoc',
		'--webpage',
		'--header', '   ',
		'--footer', '  1',
		'--format', 'pdf',
		'--outfile', $tgt,
		'--quiet',
		$src
	);

	print("@args\n");

	if (system(@args) != 0) { exit(1); }
}
