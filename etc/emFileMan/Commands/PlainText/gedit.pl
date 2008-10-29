#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 1.0
# Interpreter = perl
# DefaultFor = file
# Caption = gedit
# Descr =Open plain text files in the GNOME text editor gedit.
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

OpenTargetFilesWith('gedit');
