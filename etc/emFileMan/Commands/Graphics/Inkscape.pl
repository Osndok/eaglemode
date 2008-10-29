#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 2.0
# Interpreter = perl
# DefaultFor = .svg:.svgz:.svg.gz:.ai:.wmf
# Caption = Inkscape
# Descr =Open vector graphic files in Inkscape.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files to be opened.
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

OpenTargetFilesWith('inkscape');
