#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 4.0
# Interpreter = perl
# DefaultFor = .123:.cgm:.doc:.docx:.dot:.dxf:.emf:.eps:.hwp:.jtd:.jtt:.lwp:.met:.mml:.odb:.odf:.odg:.odm:.odp:.ods:.odt:.otg:.oth:.otp:.ots:.ott:.pct:.pot:.pps:.ppt:.pptx:.rtf:.sda:.sdc:.sdd:.sdp:.sdw:.sgf:.sgl:.sgv:.slk:.smf:.stc:.std:.sti:.stw:.svm:.sxc:.sxd:.sxg:.sxi:.sxm:.sxw:.vor:.wb2:.wk1:.wk2:.wpd:.wps:.xls:.xlsx:.xlt:.xlw
# Caption = LibreOffice
# Descr =Open files in LibreOffice.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The office files to be opened.
# Icon = thirdparty/libreoffice.tga
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

OpenTargetFilesWith('soffice');
