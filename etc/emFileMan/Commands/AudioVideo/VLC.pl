#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = perl
# DefaultFor = .aac:.asf:.au:.avi:.flac:.fli:.flv:.it:.m2t:.m2v:.mkv:.mod:.mov:.mp3:.mp4:.mpeg:.mpg:.mpv:.ogg:.ogv:.ra:.rm:.s3m:.stm:.ts:.wav:.vob:.wma:.wmv:.xm
# Caption = VLC media player
# Descr =Open audio or video files in the VLC media palyer.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files to be opened.
# Icon = thirdparty/vlc.tga
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

OpenTargetFilesWith('vlc');
