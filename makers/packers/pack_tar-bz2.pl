#!/usr/bin/perl
#-------------------------------------------------------------------------------
# pack_tar-bz2.pl
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

# Don't do it on Windows.
if ($Config{'osname'} eq "MSWin32" || $Config{'osname'} eq "cygwin") {
	print(STDERR
		"Packing of source package refused, because this operating ".
		"system does not support UNIX file attributes.\n"
	);
	exit(1);
}

# Make sure to have the documentation up-to-date.
system('perl',Var('PRJ_DIR').'/doc/html/src/make-CppApiRef.pl')==0 || exit(1);
system('perl',Var('PRJ_DIR').'/doc/ps/src/make-ps.pl')==0 || exit(1);

# Have an empty temporary directory.
my $tmpDir=catfile(Var('TMP_DIR'),Var('NAME').'-tar-bz2-packing-'.$>);
if (-e $tmpDir) { RemoveTree($tmpDir); }
CreateDirPath($tmpDir);

# Have a temporary source directory.
my $srcDir=catfile($tmpDir,Var('NAME').'-'.Var('VERSION'));
CreateDirPath($srcDir);

# Copy package contents.
print("Creating package contents in $srcDir\n");
my $oldDir=getcwd();
chdir(Var('PRJ_DIR')) or die;
my @paths=readpipe "perl make.pl list listdirs=yes filter=-clean-private";
if (!@paths) {
	die("make.pl failed, stopped");
}
for (my $i=0; $i<@paths; $i++) {
	my $path=$paths[$i];
	$path =~ s/\x{a}|\x{d}//;
	my $tgtPath=catfile($srcDir,$path);
	if (-d($path)) {
		if (!mkpath($tgtPath,0,0755)) {
			die "failed to create directory \"$tgtPath\": $!";
		}
	}
	else {
		if (!copy($path,$tgtPath)) {
			die "failed to copy \"$path\" to \"$tgtPath\": $!";
		}
		if (!chmod(0644,$tgtPath)) {
			die "failed to chmod \"$tgtPath\": $!";
		}
	}
}
@paths=readpipe "perl make.pl list listdirs=no filter=-clean-private+exec";
if (!@paths) {
	die("make.pl failed, stopped");
}
for (my $i=0; $i<@paths; $i++) {
	my $path=$paths[$i];
	$path =~ s/\x{a}|\x{d}//;
	my $tgtPath=catfile($srcDir,$path);
	if (!chmod(0755,$tgtPath)) {
		die "failed to chmod \"$tgtPath\": $!";
	}
}
chdir($oldDir) or die;

# Pack the package.
CreateDirPath(Var('PKG_DIR'));
my $tbzFile=catfile(Var('PKG_DIR'),Var('NAME').'-'.Var('VERSION').'.tar.bz2');
if (-e $tbzFile) { RemoveTree($tbzFile); }
print("Creating package: $tbzFile\n");
my $oldDir2=getcwd();
chdir(dirname($srcDir)) or die;
system(
	"tar",
	"cfj",
	$tbzFile,
	basename($srcDir)
)==0 or die("tar failed, stopped");
chdir($oldDir2) or die;

# Remove temporary directory.
RemoveTree($tmpDir);
