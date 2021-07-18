#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = perl
# DefaultFor = .htm:.html:webp
# Caption = Firefox
# Descr =Open files and/or directories in Firefox.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files and directories to be opened.
# Icon = thirdparty/firefox.tga
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

OpenTargetsWith('firefox');
