#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = perl
# DefaultFor = .avif:.bitmap:.bmp:.g3:.gbr:.gif:.gih:.gpb:.heic:.heif:.ico:.icon:.im1:.im24:.im32:.im8:.jpeg:.jpg:.pat:.pbm:.pcx:.pgm:.png:.pnm:.ppm:.psd:.ras:.rgb:.sgi:.tga:.tif:.tiff:.webp:.xbm:.xcf:.xcfbz2:.xcf.bz2:.xcfgz:.xcf.gz:.xjt:.xjtbz2:.xjtgz:.xpm:.xwd
# Caption = The GIMP
# Descr =Open bitmap files in The GIMP.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The bitmap files to be opened.
# Icon = thirdparty/gimp.tga
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

OpenTargetFilesWith('gimp');
