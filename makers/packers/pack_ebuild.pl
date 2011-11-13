#!/usr/bin/perl
#-------------------------------------------------------------------------------
# pack_ebuild.pl
#
# Copyright (C) 2010-2011 Oliver Hamann.
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

CreateDirPath(Var('PKG_DIR'));

CreateFile(
	catfile(Var('PKG_DIR'),Var('NAME').'-'.Var('VERSION').'.ebuild'),
	'# Generic ebuild file for '.Var('TITLE').' '.Var('VERSION')."\n".
	'# $Header: $'."\n".
	"\n".
	'DESCRIPTION="'.Var('SHORT_DESCRIPTION').'"'."\n".
	'SRC_URI="'.Var('EBUILD_SRC_URI').'"'."\n".
	'HOMEPAGE="'.Var('HOMEPAGE').'"'."\n".
	'KEYWORDS="'.Var('EBUILD_KEYWORDS').'"'."\n".
	'SLOT="0"'."\n".
	'LICENSE="'.(
		(length(Var('LICENSE'))==5 && lc(substr(Var('LICENSE'),0,4)) eq "gplv") ?
		"GPL-".substr(Var('LICENSE'),4) : Var('LICENSE')
	).'"'."\n".
	'IUSE="'.Var('EBUILD_IUSE').'"'."\n".
	'DEPEND="'.Var('EBUILD_DEPEND').'"'."\n".
	'RDEPEND="'.Var('EBUILD_RDEPEND').'"'."\n".
	"\n".
	'src_compile() {'."\n".
	"\t".'perl make.pl build continue=no || die "build failed"'."\n".
	'}'."\n".
	"\n".
	'src_install() {'."\n".
	"\t".'perl make.pl install "root=${D}" "dir='.Var('INSTALL_DIR').'" menu=yes bin=yes \\'."\n".
	"\t".'|| die "install failed"'."\n".
	'}'."\n"
);
