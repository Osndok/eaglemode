#!/usr/bin/perl
#-------------------------------------------------------------------------------
# pack_rpm.pl
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

# Create spec file.
CreateFile(
	catfile(Var('PKG_DIR'),Var('NAME').'-'.Var('VERSION').'.spec'),
	'BuildRequires: '.Var('RPM_BUILD_REQUIRES')."\n".
	'Requires: '.Var('RPM_REQUIRES')."\n".
	''."\n".
	'Name: '.Var('NAME')."\n".
	'Version: '.Var('VERSION')."\n".
	'Release: '.Var('RPM_PACKAGE_VERSION')."\n".
	'Group: '.Var('RPM_GROUP')."\n".
	'URL: '.Var('HOMEPAGE')."\n".
	'Vendor: '.Var('PACKAGE_MAINTAINER')."\n".
	'License: '.Var('LICENSE')."\n".
	'Summary: '.Var('SHORT_DESCRIPTION')."\n".
	'Source: %{name}-%{version}.tar.bz2'."\n".
	'BuildRoot: %{_tmppath}/%{name}-%{version}-build'."\n".
	''."\n".
	'%description'."\n".
	Var('LONG_DESCRIPTION')."\n".
	''."\n".
	'%prep'."\n".
	'%setup -q'."\n".
	''."\n".
	'%build'."\n".
	'perl make.pl build continue=no'."\n".
	''."\n".
	'%install'."\n".
	'perl make.pl install "root=$RPM_BUILD_ROOT" "dir='.Var('INSTALL_DIR').'" menu=yes bin=yes || exit 1'."\n".
	'ORIGDIR="`pwd`"'."\n".
	'cd "$RPM_BUILD_ROOT" || exit 1'."\n".
	'find * -type d -printf "%%%%dir /%p\n" | grep %{name}  > "$ORIGDIR/files.lst"'."\n".
	'find * -type f -printf "/%p\n"         | grep %{name} >> "$ORIGDIR/files.lst"'."\n".
	'cd "$ORIGDIR" || exit 1'."\n".
	'echo "'.Var('TITLE').' installs to '.Var('INSTALL_DIR').'. Please refer to that directory" > README'."\n".
	'echo "in order to read the documentation and to start the program." >> README'."\n".
	'chmod 0644 README'."\n".
	''."\n".
	'%clean'."\n".
	'rm -rf "$RPM_BUILD_ROOT"'."\n".
	''."\n".
	'%files -f files.lst'."\n".
	'%defattr(-,root,root)'."\n".
	'%doc README'."\n"
);

# Have an empty temporary directory.
my $tmpDir=catfile(Var('TMP_DIR'),Var('NAME').'-rpm-packing-'.$>);
if (-e $tmpDir) { RemoveTree($tmpDir); }
CreateDirPath($tmpDir);

# Prepare RPM tree.
my $rpmDir=catfile($tmpDir,'rpm');
CreateDirPath(catfile($rpmDir,'BUILD'));
CreateDirPath(catfile($rpmDir,'RPMS'));
CreateDirPath(catfile($rpmDir,'SOURCES'));
CreateDirPath(catfile($rpmDir,'SPECS'));
CreateDirPath(catfile($rpmDir,'SRPMS'));
CopyFile(
	catfile(Var('PKG_DIR'),Var('NAME').'-'.Var('VERSION').'.spec'),
	catfile($rpmDir,'SPECS')
);
CopyFile(
	catfile(Var('PKG_DIR'),Var('NAME').'-'.Var('VERSION').'.tar.bz2'),
	catfile($rpmDir,'SOURCES')
);

# Prepare RPM configuration ($tmpDir acts as a faked home directory).
CreateFile(
	catfile($tmpDir,'.rpmmacros'),
	'%_topdir '.$rpmDir."\n"
);

# Call rpmbuild.
my $cmd=
	"export HOME=$tmpDir ; ".
	"cd $rpmDir/SPECS ; ".
	"rpmbuild --buildroot $tmpDir/buildroot -ba ".Var('NAME').'-'.Var('VERSION').'.spec'
;
print("Calling rpmbuild with:\n $cmd\n");
system($cmd)==0 || exit(1);

# Copy rpm files into output directory.
my @rpmFiles=readpipe("find $rpmDir/RPMS $rpmDir/SRPMS -type f -name \*.rpm");
if (!@rpmFiles || @rpmFiles<2) {
	print(STDERR "ERROR: could not find generated RPM files.\n");
	exit(1);
}
foreach my $f (@rpmFiles) {
	$f=substr($f,0,length($f)-1);
	CopyFile($f,Var('PKG_DIR'));
}

# Remove temporary directory.
RemoveTree($tmpDir);
