#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 1.0
# Interpreter = perl
# DefaultFor = .it:.mod:.mp3:.mtm:.ogg:.s3m:.stm:.wav:.xm
# Caption = XMMS
# Descr =Open audio or video files in XMMS. It is even possible to open
# Descr =some directories which are containing files to be played.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files or directories to be opened.
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

OpenTargetsWith('xmms');
