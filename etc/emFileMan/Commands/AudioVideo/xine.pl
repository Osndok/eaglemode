#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = perl
# DefaultFor = .aac:.asf:.au:.avi:.flac:.fli:.it:.m2v:.mod:.mov:.mp3:.mp4:.mpeg:.mpg:.mpv:.ogg:.ra:.rm:.s3m:.stm:.wav:.wma:.wmv:.xm
# Caption = xine
# Descr =Open an audio or video file in xine.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The file to be opened.
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

OpenSingleTargetFileWith('xine');
