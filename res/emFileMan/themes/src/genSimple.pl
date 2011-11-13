#!/usr/bin/perl

use strict;
use warnings;
use File::Basename;


sub generate_theme
	# Arguments: tallness, display name, file
{
	my $tallness = shift;
	my $DisplayName = shift;
	my $fileName = shift;

	my $BackgroundColor = '{238 238 234}';
	my $SourceSelectionColor = '{171 233 171}';
	my $TargetSelectionColor = '{255 171 176}';
	my $NormalNameColor = '{20 20 15}';
	my $ExeNameColor = '{10 82 5}';
	my $DirNameColor = '{35 40 155}';
	my $FifoNameColor = '{170 20 70}';
	my $BlkNameColor = '{100 45 20}';
	my $ChrNameColor = '{65  92 15}';
	my $SockNameColor = '{140 20 140}';
	my $OtherNameColor = '{140 20 10}';
	my $PathColor = '{103 103 95}';
	my $SymLinkColor = '{60 123 133}';
	my $LabelColor = $PathColor;
	my $InfoColor = $PathColor;
	my $FileContentColor = $BackgroundColor;
	my $DirContentColor = $BackgroundColor;

	my $Height = $tallness;

	my $f=($tallness/(1.0/3.0)+1.0)/2.0;

	my $MinAltVW=25.0/$f;
	my $MinContentVW=45.0/$f;

	my $BackgroundW = 0.99;
	my $BackgroundH = $BackgroundW*$tallness;
	my $BackgroundX = (1.0-$BackgroundW)*0.5;
	my $BackgroundY = ($tallness-$BackgroundH)*0.5;
	my $BackgroundRX = 0.035;
	my $BackgroundRY = $BackgroundRX;

	my $OuterBorderX = 0.0;
	my $OuterBorderY = 0.0;
	my $OuterBorderW = 1.0;
	my $OuterBorderH = $tallness;
	my $OuterBorderImg = '';
	my $OuterBorderImgL = 0.0;
	my $OuterBorderImgT = 0.0;
	my $OuterBorderImgR = 0.0;
	my $OuterBorderImgB = 0.0;
	my $OuterBorderL = 0.0;
	my $OuterBorderT = 0.0;
	my $OuterBorderR = 0.0;
	my $OuterBorderB = 0.0;

	my $frame=(1.0+$tallness)*0.0075*$f;#*4.0*24.0/71.0;

	my $spacing=$frame*0.5;

	my $x1=$frame+$spacing;
	my $y1=$frame+$spacing;
	my $x2=1.0-$frame-$spacing;
	my $y2=$tallness-$frame-$spacing;

	my $topSpace = 0.14;
	my $bottomSpace = 0.03;

	my $NameW=0.93;
	my $NameX=(1.0-$NameW)*0.5;
	my $NameH=0.1*$NameW;
	my $NameAlignment='center';

	my $PathX=$NameX;
	my $PathW=$NameW;
	my $PathH=$NameH*0.14;
	my $PathAlignment='center';

	my $InfoX=$PathX;
	my $InfoW=$PathW;
	my $InfoH=$PathH*0.7;
	my $InfoAlignment='center';

	my $InfoY=$topSpace-$InfoH*1.7;
	my $PathY=$InfoY-$PathH*1.12;
	my $NameY=$PathY-$NameH*1.03;

	my $FileContentY = $topSpace;
	my $FileContentH = $tallness-$bottomSpace-$topSpace;
	my $FileContentW = $FileContentH/$tallness;
	my $FileContentX = (1.0-$FileContentW)*0.5;
	my $FileInnerBorderImg = '';
	my $FileInnerBorderImgL = 0.0;
	my $FileInnerBorderImgT = 0.0;
	my $FileInnerBorderImgR = 0.0;
	my $FileInnerBorderImgB = 0.0;
	my $FileInnerBorderL = 0.0;
	my $FileInnerBorderT = 0.0;
	my $FileInnerBorderR = 0.0;
	my $FileInnerBorderB = 0.0;
	my $FileInnerBorderX = $FileContentX;
	my $FileInnerBorderY = $FileContentY;
	my $FileInnerBorderW = $FileContentW;
	my $FileInnerBorderH = $FileContentH;

	my $DirContentX = $FileContentX;
	my $DirContentY = $FileContentY;
	my $DirContentW = $FileContentW;
	my $DirContentH = $FileContentH;
	my $DirInnerBorderImg = $FileInnerBorderImg;
	my $DirInnerBorderImgL = $FileInnerBorderImgL;
	my $DirInnerBorderImgT = $FileInnerBorderImgT;
	my $DirInnerBorderImgR = $FileInnerBorderImgR;
	my $DirInnerBorderImgB = $FileInnerBorderImgB;
	my $DirInnerBorderL = $FileInnerBorderL;
	my $DirInnerBorderT = $FileInnerBorderT;
	my $DirInnerBorderR = $FileInnerBorderR;
	my $DirInnerBorderB = $FileInnerBorderB;
	my $DirInnerBorderX = $FileInnerBorderX;
	my $DirInnerBorderY = $FileInnerBorderY;
	my $DirInnerBorderW = $FileInnerBorderW;
	my $DirInnerBorderH = $FileInnerBorderH;

	my $altContentFrame=0.002/0.604*$f;

	my $AltLabelX = 0.0;
	my $AltLabelY = 0.0;
	my $AltLabelW = 1.0;
	my $AltLabelH = 2.0*$altContentFrame;
	my $AltLabelAlignment = "left";
	my $AltPathX = $AltLabelX;
	my $AltPathY = $AltLabelY+$AltLabelH;
	my $AltPathH = 5.0*$altContentFrame;
	my $AltPathAlignment = "bottom-left";
	my $AltAltY = $AltPathY;
	my $AltAltH = $AltPathH;
	my $AltInnerBorderX=0.0;
	my $AltInnerBorderY=$AltPathY+$AltPathH+$altContentFrame*0.16666665;
	my $AltInnerBorderW=1.0;
	my $AltInnerBorderL = 0.0;
	my $AltInnerBorderT = 0.0;
	my $AltInnerBorderR = 0.0;
	my $AltInnerBorderB = 0.0;
	my $AltInnerBorderImg = "";
	my $AltInnerBorderImgL = 0.0;
	my $AltInnerBorderImgT = 0.0;
	my $AltInnerBorderImgR = 0.0;
	my $AltInnerBorderImgB = 0.0;
	my $AltContentX=$AltInnerBorderX+$AltInnerBorderL;
	my $AltContentY=$AltInnerBorderY+$AltInnerBorderT;
	my $AltContentW=$AltInnerBorderW-$AltInnerBorderL-$AltInnerBorderR;
	my $AltContentH=$AltContentW*$tallness;
	my $AltInnerBorderH=$AltContentH+$AltInnerBorderT+$AltInnerBorderB;

	my $AltH=$AltAltH/$AltContentH*$DirContentH;
	my $AltW=$AltH*$AltInnerBorderW/($AltInnerBorderY+$AltInnerBorderH);
	my $AltX=$DirInnerBorderX+$DirInnerBorderW-$AltW;
	my $AltY=$DirInnerBorderY-$AltH-
	         ($AltInnerBorderY-$AltAltH-$AltAltY)/$AltAltH*$AltH;

	my $AltAltW=$AltAltH*$AltW/$AltH;
	my $AltAltX=1.0-$AltAltW;
	my $AltPathW=$AltAltX-$AltPathX-$AltPathH*0.4;

	my $fh;
	open($fh,">",$fileName);

	print($fh
		"#%rec:emFileManTheme%#\n".
		"\n".
		"DisplayName = \"$DisplayName\"\n".
		"BackgroundColor = $BackgroundColor\n".
		"SourceSelectionColor = $SourceSelectionColor\n".
		"TargetSelectionColor = $TargetSelectionColor\n".
		"NormalNameColor = $NormalNameColor\n".
		"ExeNameColor = $ExeNameColor\n".
		"DirNameColor = $DirNameColor\n".
		"FifoNameColor = $FifoNameColor\n".
		"BlkNameColor = $BlkNameColor\n".
		"ChrNameColor = $ChrNameColor\n".
		"SockNameColor = $SockNameColor\n".
		"OtherNameColor = $OtherNameColor\n".
		"PathColor = $PathColor\n".
		"SymLinkColor = $SymLinkColor\n".
		"LabelColor = $LabelColor\n".
		"InfoColor = $InfoColor\n".
		"FileContentColor = $FileContentColor\n".
		"DirContentColor = $DirContentColor\n".
		"Height = $Height\n".
		"BackgroundX = $BackgroundX\n".
		"BackgroundY = $BackgroundY\n".
		"BackgroundW = $BackgroundW\n".
		"BackgroundH = $BackgroundH\n".
		"BackgroundRX = $BackgroundRX\n".
		"BackgroundRY = $BackgroundRY\n".
		"OuterBorderX = $OuterBorderX\n".
		"OuterBorderY = $OuterBorderY\n".
		"OuterBorderW = $OuterBorderW\n".
		"OuterBorderH = $OuterBorderH\n".
		"OuterBorderL = $OuterBorderL\n".
		"OuterBorderT = $OuterBorderT\n".
		"OuterBorderR = $OuterBorderR\n".
		"OuterBorderB = $OuterBorderB\n".
		"OuterBorderImg = \"$OuterBorderImg\"\n".
		"OuterBorderImgL = $OuterBorderImgL\n".
		"OuterBorderImgT = $OuterBorderImgT\n".
		"OuterBorderImgR = $OuterBorderImgR\n".
		"OuterBorderImgB = $OuterBorderImgB\n".
		"NameX = $NameX\n".
		"NameY = $NameY\n".
		"NameW = $NameW\n".
		"NameH = $NameH\n".
		"NameAlignment = $NameAlignment\n".
		"PathX = $PathX\n".
		"PathY = $PathY\n".
		"PathW = $PathW\n".
		"PathH = $PathH\n".
		"PathAlignment = $PathAlignment\n".
		"InfoX = $InfoX\n".
		"InfoY = $InfoY\n".
		"InfoW = $InfoW\n".
		"InfoH = $InfoH\n".
		"InfoAlignment = $InfoAlignment\n".
		"FileInnerBorderX = $FileInnerBorderX\n".
		"FileInnerBorderY = $FileInnerBorderY\n".
		"FileInnerBorderW = $FileInnerBorderW\n".
		"FileInnerBorderH = $FileInnerBorderH\n".
		"FileInnerBorderL = $FileInnerBorderL\n".
		"FileInnerBorderT = $FileInnerBorderT\n".
		"FileInnerBorderR = $FileInnerBorderR\n".
		"FileInnerBorderB = $FileInnerBorderB\n".
		"FileInnerBorderImg = \"$FileInnerBorderImg\"\n".
		"FileInnerBorderImgL = $FileInnerBorderImgL\n".
		"FileInnerBorderImgT = $FileInnerBorderImgT\n".
		"FileInnerBorderImgR = $FileInnerBorderImgR\n".
		"FileInnerBorderImgB = $FileInnerBorderImgB\n".
		"FileContentX = $FileContentX\n".
		"FileContentY = $FileContentY\n".
		"FileContentW = $FileContentW\n".
		"FileContentH = $FileContentH\n".
		"DirInnerBorderX = $DirInnerBorderX\n".
		"DirInnerBorderY = $DirInnerBorderY\n".
		"DirInnerBorderW = $DirInnerBorderW\n".
		"DirInnerBorderH = $DirInnerBorderH\n".
		"DirInnerBorderL = $DirInnerBorderL\n".
		"DirInnerBorderT = $DirInnerBorderT\n".
		"DirInnerBorderR = $DirInnerBorderR\n".
		"DirInnerBorderB = $DirInnerBorderB\n".
		"DirInnerBorderImg = \"$DirInnerBorderImg\"\n".
		"DirInnerBorderImgL = $DirInnerBorderImgL\n".
		"DirInnerBorderImgT = $DirInnerBorderImgT\n".
		"DirInnerBorderImgR = $DirInnerBorderImgR\n".
		"DirInnerBorderImgB = $DirInnerBorderImgB\n".
		"DirContentX = $DirContentX\n".
		"DirContentY = $DirContentY\n".
		"DirContentW = $DirContentW\n".
		"DirContentH = $DirContentH\n".
		"AltX = $AltX\n".
		"AltY = $AltY\n".
		"AltW = $AltW\n".
		"AltH = $AltH\n".
		"AltLabelX = $AltLabelX\n".
		"AltLabelY = $AltLabelY\n".
		"AltLabelW = $AltLabelW\n".
		"AltLabelH = $AltLabelH\n".
		"AltLabelAlignment = $AltLabelAlignment\n".
		"AltPathX = $AltPathX\n".
		"AltPathY = $AltPathY\n".
		"AltPathW = $AltPathW\n".
		"AltPathH = $AltPathH\n".
		"AltPathAlignment = $AltPathAlignment\n".
		"AltAltX = $AltAltX\n".
		"AltAltY = $AltAltY\n".
		"AltAltW = $AltAltW\n".
		"AltAltH = $AltAltH\n".
		"AltInnerBorderX = $AltInnerBorderX\n".
		"AltInnerBorderY = $AltInnerBorderY\n".
		"AltInnerBorderW = $AltInnerBorderW\n".
		"AltInnerBorderH = $AltInnerBorderH\n".
		"AltInnerBorderL = $AltInnerBorderL\n".
		"AltInnerBorderT = $AltInnerBorderT\n".
		"AltInnerBorderR = $AltInnerBorderR\n".
		"AltInnerBorderB = $AltInnerBorderB\n".
		"AltInnerBorderImg = \"$AltInnerBorderImg\"\n".
		"AltInnerBorderImgL = $AltInnerBorderImgL\n".
		"AltInnerBorderImgT = $AltInnerBorderImgT\n".
		"AltInnerBorderImgR = $AltInnerBorderImgR\n".
		"AltInnerBorderImgB = $AltInnerBorderImgB\n".
		"AltContentX = $AltContentX\n".
		"AltContentY = $AltContentY\n".
		"AltContentW = $AltContentW\n".
		"AltContentH = $AltContentH\n".
		"MinContentVW = $MinContentVW\n".
		"MinAltVW = $MinAltVW\n"
	);

	close($fh);
}


my $ThemeName = basename($0);
$ThemeName =~ s/(^gen)|(.pl$)//g;

generate_theme(9.0/16.0 , "${ThemeName}" , "../${ThemeName}.emFileManTheme");
