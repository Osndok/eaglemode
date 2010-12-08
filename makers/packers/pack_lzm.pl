#!/usr/bin/perl
#-------------------------------------------------------------------------------
# pack_lzm.pl
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
use File::Basename;
BEGIN { require (dirname($0).'/common.pm'); }

# Dependencies
my $tbzFile=catfile(Var('PKG_DIR'),Var('NAME').'-'.Var('VERSION').'.tar.bz2');
if (!-e $tbzFile) { system('perl','pack_tar-bz2.pl')==0 || exit(1); }

# Have an empty temporary directory.
my $tmpDir=catfile(Var('TMP_DIR'),Var('NAME').'-lzm-packing-'.$>);
if (-e $tmpDir) { RemoveTree($tmpDir); }
CreateDirPath($tmpDir);

# Unpack source package into the temporary directory.
my $srcDir=catfile($tmpDir,Var('NAME').'-'.Var('VERSION'));
print("Unpacking the source package to $srcDir\n");
my $oldDir=getcwd();
chdir($tmpDir) or die;
system('tar','xfj',$tbzFile)==0 || exit 1;
chdir($oldDir) or die;

# Compile it
system(
	'perl',
	catfile($srcDir,'make.pl'),
	'build',
	'continue=no'
)==0 || exit 1;

# Have a temporary root directory.
my $lzmRoot=catfile($tmpDir,'lzm-root');
CreateDirPath($lzmRoot);

# Install it
system(
	'perl',
	catfile($srcDir,'make.pl'),
	'install',
	'root='.$lzmRoot,
	'dir='.Var('INSTALL_DIR'),
	'menu=yes',
	'bin=yes'
)==0 || exit 1;

# Remove temporary source directory.
RemoveTree($srcDir);

# Create LZM archive
my $lzmFile=catfile(Var('PKG_DIR'),Var('NAME').'-'.Var('VERSION').'.lzm');
print("Creating $lzmFile\n");
system(
	'dir2lzm',
	$lzmRoot,
	$lzmFile
)==0 || exit 1;

# Remove temporary directory.
RemoveTree($tmpDir);
