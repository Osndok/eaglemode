#!/usr/bin/perl
#-------------------------------------------------------------------------------
# pack_tar-gz.pl
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

# Create output file.
my $tgzFile=catfile(Var('PKG_DIR'),Var('NAME').'-'.Var('VERSION').'.tar.gz');
print("Creating $tgzFile from $tbzFile\n");
system("bzip2 -dc $tbzFile | gzip > $tgzFile")==0 || exit(1);
