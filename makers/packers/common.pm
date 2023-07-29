#!/usr/bin/perl
#-------------------------------------------------------------------------------
# common.pm
#
# Copyright (C) 2010-2014,2017,2019-2021,2023 Oliver Hamann.
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
use Config;
use Cwd;
use File::Basename;
use File::Copy;
use File::Path;
use File::Spec;
use File::Spec::Functions;
use File::Temp;


#================================= Preparation =================================

chdir(dirname($0)) or die;


#================================ Configuration ================================

my %V;
sub Var { return $V{$_[0]}; }

$V{'PRJ_DIR'}=dirname(dirname(getcwd()));
$V{'TMP_DIR'}=File::Spec->tmpdir();
$V{'PKG_DIR'}=catfile($V{'PRJ_DIR'},'packages');
$V{'NAME'}='eaglemode';
$V{'TITLE'}='Eagle Mode';
$V{'VERSION'}=DetectVersion();
$V{'LICENSE'}='GPLv3';
$V{'HOMEPAGE'}='http://eaglemode.sourceforge.net/';

$V{'SHORT_DESCRIPTION'}="Zoomable user interface with plugin applications";

$V{'LONG_DESCRIPTION'}=
	"Eagle Mode is an advanced solution for a futuristic style of man-machine\n".
	"communication in which the user can visit almost everything simply by\n".
	"zooming in. It has a professional file manager, file viewers and players\n".
	"for most of the common file types, a chess game, a 3D mines game, a\n".
	"netwalk game, a multi-function clock and some fractal fun, all\n".
	"integrated in a virtual cosmos. Besides, that cosmos also provides a\n".
	"Linux kernel configurator in form of a kernel patch.";

$V{'SOFTWARE_AUTHOR'}='Oliver Hamann';

$V{'PACKAGE_MAINTAINER'}='Oliver Hamann <olha@users.sourceforge.net>';

$V{'INSTALL_DIR'}='/usr/lib/'.$V{'NAME'};

$V{'DEB_PACKAGE_VERSION'}='1';
$V{'DEB_SECTION'}='x11';
my $dist='';
if ($Config{'osname'} eq "linux") {
	$dist=readpipe('lsb_release --id');
	if (defined($dist)) { ($dist)=($dist=~/^.*[:][\s]*([^\s].*)$/); }
	if (!defined($dist)) { $dist=''; }
}
$V{'DEB_BUILD_DEPENDS'}=
	'debhelper (>= 5), g++ (>= 4), perl, libx11-dev, libjpeg-dev, '.
	'libpng-dev, libtiff5-dev, libwebp-dev, libvlc-dev, librsvg2-dev, '.
	'libpoppler-glib-dev, libgtk2.0-dev, libfreetype6-dev';
$V{'DEB_DEPENDS'}=
	'perl, xterm, ghostscript, libc6, libgcc1, libstdc++6, libx11-6, '.
	'libjpeg62-turbo | libjpeg62, libpng16-16, libvlc5, '.
	'vlc-plugin-base, librsvg2-2, libpoppler-glib8, libgtk2.0-0, '.
	'libfreetype6';
$V{'DEB_RECOMMENDS'}=
	'abiword, genisoimage, htmldoc, libwebp6, libtiff5, netpbm, transfig';

$V{'RPM_PACKAGE_VERSION'}='1';
$V{'RPM_GROUP'}='System/GUI/Other';
$V{'RPM_BUILD_REQUIRES'}=
	"gcc-c++ perl\n".
	"\%if 0\%{?suse_version}\n".
	"BuildRequires: libX11-devel vlc-devel libjpeg62-devel libpng16-devel libtiff-devel libwebp-devel librsvg2-devel gtk2-devel libpoppler-glib-devel freetype-devel\n".
	"\%else\n".
	"BuildRequires: libX11-devel vlc-devel libjpeg-devel libpng-devel libtiff-devel libwebp-devel librsvg2-devel gtk2-devel poppler-glib-devel freetype-devel\n".
	"\%endif\n";
$V{'RPM_REQUIRES'}=
	"perl xterm ghostscript\n".
	"\%ifarch x86_64\n".
	"Requires:".
	" libc.so.6()(64bit)".
	" libgcc_s.so.1()(64bit)".
	" libstdc++.so.6()(64bit)".
	" libX11.so.6()(64bit)".
	" libjpeg.so.62()(64bit)".
	" libpng16.so.16()(64bit)".
	" libtiff.so.5()(64bit)".
	" libwebp.so.7()(64bit)".
	" libvlc.so.5()(64bit)".
	" librsvg-2.so.2()(64bit)".
	" libpoppler-glib.so.8()(64bit)".
	" libgtk-x11-2.0.so.0()(64bit)".
	" libfreetype.so.6()(64bit)".
	"\n".
	"\%else\n".
	"Requires:".
	" libc.so.6".
	" libgcc_s.so.1".
	" libstdc++.so.6".
	" libX11.so.6".
	" libjpeg.so.62".
	" libpng16.so.16".
	" libtiff.so.5".
	" libwebp.so.7".
	" libvlc.so.5".
	" librsvg-2.so.2".
	" libpoppler-glib.so.8".
	" libgtk-x11-2.0.so.0".
	" libfreetype.so.6".
	"\n".
	"\%endif\n";

$V{'EBUILD_SRC_URI'}='http://prdownloads.sourceforge.net/${PN}/${P}.tar.bz2';
$V{'EBUILD_KEYWORDS'}='~x86';
$V{'EBUILD_IUSE'}='';
$V{'EBUILD_DEPEND'}=
	"dev-lang/perl\n\tx11-libs/libX11\n\tmedia-libs/jpeg\n\t".
	"media-libs/libpng\n\tmedia-libs/tiff\n\tmedia-libs/webp\n\tmedia-video/vlc\n\t".
	"gnome-base/librsvg\n\tapp-text/poppler[cairo]\n\tmedia-libs/freetype";
$V{'EBUILD_RDEPEND'}="\${DEPEND}\n\tx11-terms/xterm\n\tapp-text/ghostscript-gpl";

$V{'WIN_NSIS_DIR'}='C:\\Program Files (x86)\\NSIS';
$V{'WIN_THIRDPARTY_DIR'}=catfile($V{'PRJ_DIR'},'thirdparty');


#============================== Helper functions ===============================

sub DetectVersion
{
	my $incFile=catfile(Var('PRJ_DIR'),'include','emCore','emStd1.h');

	my $maj;
	my $min;
	my $mic;
	my $postfix;

	my $fh;
	open($fh,"<",$incFile) or die "$incFile: $!";
	while (my $ln=readline($fh)) {
		if (!defined($maj) && $ln =~ /^#\s*define\s*EM_MAJOR_VERSION\s*([0-9]*)\s*$/) {
			$maj=$1;
		}
		if (!defined($min) && $ln =~ /^#\s*define\s*EM_MINOR_VERSION\s*([0-9]*)\s*$/) {
			$min=$1;
		}
		if (!defined($mic) && $ln =~ /^#\s*define\s*EM_MICRO_VERSION\s*([0-9]*)\s*$/) {
			$mic=$1;
		}
		if (!defined($postfix) && $ln =~ /^#\s*define\s*EM_VERSION_POSTFIX\s*"(.*)"\s*$/) {
			$postfix=$1;
		}
		if (defined($maj) && defined($min) && defined($mic) && defined($postfix)) {
			last;
		}
	}
	close($fh);

	if (!defined($maj) || !defined($min) || !defined($mic) || !defined($postfix)) {
		print(STDERR "Failed to extract version numbers from $incFile\n");
		exit(1);
	}

	return "$maj.$min.$mic$postfix";
}


sub CreateDirPath
{
	my $path=shift;

	if (!-d $path) {
		print("Creating directory: $path\n");
		if (!mkpath($path,0,0755)) {
			print(STDERR "failed to create directory \"$path\": $!\n");
			exit(1);
		}
	}
}


sub CopyFile
{
	my $src=shift;
	my $tgt=shift;

	print("Copying $src to $tgt\n");
	if (!copy($src,$tgt)) {
		print(STDERR "failed to copy $src to $tgt: $!\n");
		exit(1);
	}
}


sub CopyTree
{
	my $src=shift;
	my $tgt=shift;

	if (-d $src) {
		if (-d $tgt) {
			$tgt=catfile($tgt,basename($src));
		}
		CreateDirPath($tgt);
		my $dh;
		if (!opendir($dh,$src)) {
			print(STDERR "failed to read directory $src: $!\n");
			exit(1);
		}
		while (defined(my $name=readdir($dh))) {
			if ($name ne '.' && $name ne '..') {
				CopyTree(catfile($src,$name),$tgt);
			}
		}
		closedir($dh);
	}
	else {
		CopyFile($src,$tgt);
	}
}


sub CreateFile
{
	my $path=shift;
	my $contents=shift;
	print("Creating file: $path\n");
	my $fh;
	if (!open($fh,">",$path)) {
		print(STDERR "failed to create $path: $!\n");
		exit(1);
	}
	print $fh $contents;
	close($fh);
}


sub RemoveTree
{
	my $path=shift;

	if (-e $path) {
		print("Removing: $path\n");
		if (!rmtree($path,0,0)) {
			print(STDERR "failed to remove $path: $!\n");
			exit(1);
		}
	}
}


sub Indent
{
	my $text=shift;
	my $indent=shift;
	for (my $i=0;;) {
		$text=substr($text,0,$i).$indent.substr($text,$i);
		$i=index($text,"\n",$i)+1;
		if ($i<=0) { last; }
	}
	return $text;
}


sub Wrap
{
	my $text=shift;
	my $cols=shift;

	$text=~tr/\n/ /;
	for (my $pos=0; $pos+$cols<length($text); ) {
		my $i=rindex(substr($text,$pos,$cols+1),' ');
		if ($i>=0) {
			$pos+=$i+1;
			$text=substr($text,0,$pos-1)."\n".substr($text,$pos);
		}
		else {
			$pos+=$cols+1;
			$text=substr($text,0,$pos-1)."\n".substr($text,$pos-1);
		}
	}
	return $text;
}
