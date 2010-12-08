#!/usr/bin/perl
#-------------------------------------------------------------------------------
# pack_tgz.pl
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

if ($>!=0) {
	print(STDERR "Slackware package creation must be run as root!\n");
	exit(1);
}

# Dependencies
my $tbzFile=catfile(Var('PKG_DIR'),Var('NAME').'-'.Var('VERSION').'.tar.bz2');
if (!-e $tbzFile) { system('perl','pack_tar-bz2.pl')==0 || exit(1); }

# Have an empty temporary directory.
my $tmpDir=catfile(Var('TMP_DIR'),Var('NAME').'-slackware-packing-'.$>);
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
my $pkgRoot=catfile($tmpDir,'pkg-root');
CreateDirPath($pkgRoot);


# Install it
system(
	'perl',
	catfile($srcDir,'make.pl'),
	'install',
	'root='.$pkgRoot,
	'dir='.Var('INSTALL_DIR'),
	'menu=yes',
	'bin=yes'
)==0 || exit 1;

# Remove temporary source directory.
RemoveTree($srcDir);

# Create install directory.
my $insDir=catfile($pkgRoot,'install');
CreateDirPath($insDir);

# Create slack-desc (exact 11 lines, max 70 text columns)
my $desc=
	Var('NAME').': '.Var('NAME').' ('.Var('SHORT_DESCRIPTION').')'."\n".
	Var('NAME').':'."\n".
	Indent(Wrap(Var('LONG_DESCRIPTION'),69),Var('NAME').': ')
;
my $pos=0;
for (my $line=1; $line<=11; $line++) {
	if ($pos<length($desc)) {
		$pos=index($desc,"\n",$pos);
		if ($pos>=0) { $pos++; }
		else { $desc.="\n"; $pos=length($desc); }
	}
	else {
		$desc.=Var('NAME').':'."\n";
		$pos=length($desc);
	}
}
$desc=substr($desc,0,$pos);
CreateFile(catfile($insDir,'slack-desc'),$desc);

# Create slackware package
my $machine=readpipe('gcc -dumpmachine');
$machine=substr($machine,0,length($machine)-1);
$machine=~s/-linux$//i;
$machine=~s/-slackware$//i;
my $pkgFile=catfile(
	Var('PKG_DIR'),
	Var('NAME').'-'.Var('VERSION').'-'.$machine.'-1.tgz'
);
print("Running makepkg for creating $pkgFile\n");
my $oldDir2=getcwd();
chdir($pkgRoot) or die;
system("makepkg --linkadd y --chown y \"$pkgFile\"")==0 || exit 1;
chdir($oldDir2) or die;

# Remove temporary directory.
RemoveTree($tmpDir);
