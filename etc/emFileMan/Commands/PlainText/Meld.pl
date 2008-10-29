#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 52.0
# Interpreter = perl
# Caption = Meld
# Descr =Compare two or three files or directories in Meld.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =Source and target selections together must be the two or three
# Descr =files or directories which shall be compared. Source is "left",
# Descr =target is "right". Within source or target you cannot control
# Descr =the order.
# ButtonBgColor = #BBB
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

my @src=GetSrc();
my @tgt=GetTgt();
my $cnt=@src+@tgt;
if ($cnt<2) { Error("Too few selections."); }
if ($cnt>3) { Error("Too many selections."); }
ExecOrError('meld',@src,@tgt);
