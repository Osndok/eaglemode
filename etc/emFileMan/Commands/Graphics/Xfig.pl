#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = perl
# DefaultFor = .fig
# Caption = Xfig
# Descr =Open an xfig file in Xfig.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The xfig file to be opened.
#[[END PROPERTIES]]

#??? Do not add fig.gz to the DefaultFor list, because xfig would unpack
#??? such a file in place.

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

OpenSingleTargetFileWith('xfig');
