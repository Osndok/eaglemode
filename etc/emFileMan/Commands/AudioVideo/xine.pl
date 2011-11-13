#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = perl
# DefaultFor = .aac:.asf:.au:.avi:.flac:.fli:.flv:.it:.m2t:.m2v:.mkv:.mod:.mov:.mp3:.mp4:.mpeg:.mpg:.mpv:.ogg:.ogv:.ra:.rm:.s3m:.stm:.wav:.vob:.wma:.wmv:.xm
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
