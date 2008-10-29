#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 10.0
# Interpreter = perl
# Caption = Pack bz2
# Descr =Create a bz2 archive from a file.
# Descr =The name of the archive file is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The file to be packed.
# Descr =
# Descr =  Target: The directory in which the archive file
# Descr =          shall be created.
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

PackType('bz2');
