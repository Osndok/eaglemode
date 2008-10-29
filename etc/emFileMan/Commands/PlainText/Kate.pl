#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = perl
# DefaultFor = file
# Caption = Kate
# Descr =Open plain text files in the KDE text editor Kate.
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

OpenTargetFilesWith('kate');
