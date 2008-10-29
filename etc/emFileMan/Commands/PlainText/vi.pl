#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 5.0
# Interpreter = perl
# DefaultFor = file
# Caption = vi
# Descr =Open plain text files in the text editor vi.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The plain text files to be opened.
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

OpenTargetFilesWith('xterm','-e','vi');
