#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 4.0
# Interpreter = perl
# DefaultFor = file
# Caption = NEdit
# Descr =Open plain text files in the text editor NEdit. Actually, the NEdit
# Descr =client program "nedit-nc" is called. (Originally, the client program
# Descr =was named "nc", but some distributions have renamed it to "nedit-nc"
# Descr =because "nc" is used for another program there.)
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

OpenTargetFilesWith('nedit-nc','-noask');
