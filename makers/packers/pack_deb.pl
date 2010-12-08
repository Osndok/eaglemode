#!/usr/bin/perl
#-------------------------------------------------------------------------------
# pack_deb.pl
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
my $tgzFile=catfile(Var('PKG_DIR'),Var('NAME').'-'.Var('VERSION').'.tar.gz');
if (!-e $tgzFile) { system('perl','pack_tar-gz.pl')==0 || exit(1); }

# Have an empty temporary directory.
my $tmpDir=catfile(Var('TMP_DIR'),Var('NAME').'-deb-packing-'.$>);
if (-e $tmpDir) { RemoveTree($tmpDir); }
CreateDirPath($tmpDir);

# Copy and rename source package.
CopyFile(
	$tgzFile,
	catfile($tmpDir,Var('NAME').'_'.Var('VERSION').'.orig.tar.gz'),
);

# Unpack it
my $srcDir=catfile($tmpDir,Var('NAME').'-'.Var('VERSION'));
print("Unpacking source package to $srcDir.\n");
my $oldDir=getcwd();
chdir($tmpDir) or die;
system(
	'tar',
	'xfz',
	Var('NAME').'_'.Var('VERSION').'.orig.tar.gz'
)==0 || exit 1;
chdir($oldDir) or die;

# Create Debian files.
my $debian=catfile($srcDir,'debian');
CreateDirPath($debian);
CreateFile(
	catfile($debian,'README.Debian'),
	Var('TITLE').' installs to '.Var('INSTALL_DIR').'. Please refer to that directory'."\n".
	'in order to read the documentation and to start the program.'."\n"
);
CreateFile(
	catfile($debian,'changelog'),
	Var('NAME').' ('.Var('VERSION').'-'.Var('DEB_PACKAGE_VERSION').') unstable; urgency=low'."\n".
	'  * This is a generic debian port of '.Var('NAME').' '.Var('VERSION')."\n".
	' -- '.Var('PACKAGE_MAINTAINER').'  '.readpipe('date -R')
);
CreateFile(
	catfile($debian,'compat'),
	'6'."\n"
);
CreateFile(
	catfile($debian,'control'),
	'Source: '.Var('NAME')."\n".
	'Section: '.Var('DEB_SECTION')."\n".
	'Priority: optional'."\n".
	'Maintainer: '.Var('PACKAGE_MAINTAINER')."\n".
	'Build-Depends: '.Var('DEB_BUILD_DEPENDS')."\n".
	'Standards-Version: 3.8.0'."\n".
	'Homepage: '.Var('HOMEPAGE')."\n".
	"\n".
	'Package: '.Var('NAME')."\n".
	'Architecture: any'."\n".
	'Depends: '.Var('DEB_DEPENDS')."\n".
	'Recommends: '.Var('DEB_RECOMMENDS')."\n".
	'Description: '.Var('SHORT_DESCRIPTION')."\n".
	Indent(Var('LONG_DESCRIPTION'),' ')."\n"
);
CreateFile(
	catfile($debian,'copyright'),
	'Package Maintainer: '.Var('PACKAGE_MAINTAINER')."\n".
	"\n".
	'Software Homepage: '.Var('HOMEPAGE')."\n".
	'Software Author: '.Var('SOFTWARE_AUTHOR')."\n".
	'Software License: '.Var('LICENSE')."\n"
);
CreateFile(
	catfile($debian,'rules'),
	'#!/usr/bin/make -f'."\n".
	"\n".
	'# Uncomment this to turn on verbose mode.'."\n".
	'#export DH_VERBOSE=1'."\n".
	"\n".
	'configure: configure-stamp'."\n".
	'configure-stamp:'."\n".
	"\t".'dh_testdir'."\n".
	'	touch configure-stamp'."\n".
	"\n".
	'build: build-stamp'."\n".
	"\n".
	'build-stamp: configure-stamp '."\n".
	"\t".'dh_testdir'."\n".
	"\t".'perl make.pl build continue=no'."\n".
	"\t".'touch $@'."\n".
	"\n".
	'clean:'."\n".
	"\t".'dh_testdir'."\n".
	"\t".'dh_testroot'."\n".
	"\t".'rm -f build-stamp configure-stamp'."\n".
	"\t".'perl make.pl clean'."\n".
	"\t".'dh_clean '."\n".
	"\n".
	'install: build'."\n".
	"\t".'dh_testdir'."\n".
	"\t".'dh_testroot'."\n".
	"\t".'dh_clean -k '."\n".
	"\t".'dh_installdirs'."\n".
	"\t".'perl make.pl install "root=$(CURDIR)/debian/'.Var('NAME').'" "dir='.Var('INSTALL_DIR').'" menu=yes bin=yes'."\n".
	"\n".
	'binary-indep: build install'."\n".
	"\n".
	'binary-arch: build install'."\n".
	"\t".'dh_testdir'."\n".
	"\t".'dh_testroot'."\n".
	"\t".'dh_installchangelogs '."\n".
	"\t".'dh_installdocs'."\n".
	"\t".'dh_installexamples'."\n".
	"\t".'dh_installman'."\n".
	"\t".'dh_link'."\n".
	"\t".'dh_strip'."\n".
	"\t".'dh_compress'."\n".
	"\t".'dh_fixperms'."\n".
	"\t".'dh_installdeb'."\n".
	"\t".'export LD_LIBRARY_PATH="$(CURDIR)/debian/'.Var('NAME').Var('INSTALL_DIR').'/lib:$(LD_LIBRARY_PATH)" ; dh_shlibdeps'."\n".
	"\t".'dh_gencontrol'."\n".
	"\t".'dh_md5sums'."\n".
	"\t".'dh_builddeb'."\n".
	"\n".
	'binary: binary-indep binary-arch'."\n".
	'.PHONY: build clean binary-indep binary-arch binary install configure'."\n"
);
if (!chmod(0755,catfile($debian,'rules'))) {
	die "failed to chmod \"$debian/'rules'\": $!";
}

# Call debuild (requires the packages debian-builder and debhelper).
my $cmd="cd $srcDir ; debuild -us -uc";
print("Calling debuild with:\n $cmd\n");
system($cmd)==0 || exit(1);

# Copy results.
print("Copying resulting files to ".Var('PKG_DIR')."\n");
system(
	'cp -av '.$tmpDir.'/'.Var('NAME').'_'.Var('VERSION').'* '.Var('PKG_DIR')
)==0 || exit(1);

# Remove temporary directory.
RemoveTree($tmpDir);
