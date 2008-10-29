#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 5.0
# Interpreter = perl
# DefaultFor = .abw:.aw:.awt:.bzabw:.cwk:.dbk:.doc:.dot:.fo:.htm:.html:.hwp:.ics:.icsii:.kwd:.mif:.pdb:.pdf:.rtf:.wml:.wpd:.wri:.zabw
# Caption = AbiWord
# Descr =Open files in AbiWord.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files to be opened.
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

OpenTargetFilesWith('abiword');
