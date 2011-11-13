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

	my $BackgroundColor = '{187 187 187}';
	my $SourceSelectionColor = '{174 220 174}';
	my $TargetSelectionColor = '{255 153 155}';
	my $NormalNameColor = '{0 0 0}';
	my $ExeNameColor = '{0 78 0}';
	my $DirNameColor = '{0 0 164}';
	my $FifoNameColor = '{204 0 102}';
	my $BlkNameColor = '{102 68 0}';
	my $ChrNameColor = '{85 102 0}';
	my $SockNameColor = '{187 0 187}';
	my $OtherNameColor = '{153 34 17}';
	my $PathColor = '{68 68 68}';
	my $SymLinkColor = '{255 255 255}';
	my $LabelColor = '{136 136 136}';
	my $InfoColor = '{102 102 102}';
	my $FileContentColor = $BackgroundColor;
	my $DirContentColor = $BackgroundColor;

	my $Height = $tallness;

	my $f=($tallness/(1.0/3.0)+1.0)/2.0;

	my $MinAltVW=25.0/$f;
	my $MinContentVW=45.0/$f;

	my $frame=(1.0+$tallness)*0.0075*$f;

	my $BackgroundX = 0.0;
	my $BackgroundY = 0.0;
	my $BackgroundW = 1-$frame*(32.0/96.0);
	my $BackgroundH = $tallness-$frame*(32.0/96.0);
	my $BackgroundRX = $frame;
	my $BackgroundRY = $frame;

	my $OuterBorderX = 0.0;
	my $OuterBorderY = 0.0;
	my $OuterBorderW = 1.0;
	my $OuterBorderH = $tallness;
	my $OuterBorderImg = 'MetalOuterBorder.tga';
	my $OuterBorderImgL = 124.0;
	my $OuterBorderImgT = 121.0;
	my $OuterBorderImgR = 129.0;
	my $OuterBorderImgB = 128.0;
	my $OuterBorderL = $frame*($OuterBorderImgL/96.0);
	my $OuterBorderT = $frame*($OuterBorderImgT/96.0);
	my $OuterBorderR = $frame*($OuterBorderImgR/96.0);
	my $OuterBorderB = $frame*($OuterBorderImgB/96.0);

	my $x1=$frame*1.2;
	my $y1=$frame*1.2;
	my $x2=1.0-$frame*1.35;
	my $y2=$tallness-$frame*1.35;

	my $NameX=$x1;
	my $NameY=$y1;
	my $NameW=$x2-$x1;
	my $NameH=0.09389430477167778*$NameW;
	my $NameAlignment = "left";

	my $contentFrame=$frame*0.2;

	my $PathY=$NameY+$NameH+$contentFrame;
	my $PathH=$contentFrame*5.0;
	my $PathAlignment = "bottom-left";

	my $FileInnerBorderY=$PathY+$PathH+$contentFrame*0.1666666666667;
	my $FileInnerBorderH=$y2-$FileInnerBorderY;
	my $FileInnerBorderL = $contentFrame;
	my $FileInnerBorderT = $contentFrame;
	my $FileInnerBorderR = $contentFrame;
	my $FileInnerBorderB = $contentFrame;
	my $FileInnerBorderImg = "MetalInnerBorder.tga";
	my $FileInnerBorderImgL = 64.0;
	my $FileInnerBorderImgT = 64.0;
	my $FileInnerBorderImgR = 64.0;
	my $FileInnerBorderImgB = 64.0;
	my $FileContentY=$FileInnerBorderY+$FileInnerBorderT;
	my $FileContentH=$FileInnerBorderH-$FileInnerBorderT-$FileInnerBorderB;
	my $FileContentW=$FileContentH/$tallness;
	my $FileInnerBorderW=$FileContentW+$FileInnerBorderL+$FileInnerBorderR;
	my $FileInnerBorderX=$x2-$FileInnerBorderW;
	my $FileContentX=$FileInnerBorderX+$FileInnerBorderL;

	my $DirInnerBorderX=$FileInnerBorderX;
	my $DirInnerBorderY=$FileInnerBorderY;
	my $DirInnerBorderW=$FileInnerBorderW;
	my $DirInnerBorderH=$FileInnerBorderH;
	my $DirInnerBorderL=$FileInnerBorderL;
	my $DirInnerBorderT=$FileInnerBorderT;
	my $DirInnerBorderR=$FileInnerBorderR;
	my $DirInnerBorderB=$FileInnerBorderB;
	my $DirInnerBorderImg=$FileInnerBorderImg;
	my $DirInnerBorderImgL=$FileInnerBorderImgL;
	my $DirInnerBorderImgT=$FileInnerBorderImgT;
	my $DirInnerBorderImgR=$FileInnerBorderImgR;
	my $DirInnerBorderImgB=$FileInnerBorderImgB;
	my $DirContentX=$FileContentX;
	my $DirContentY=$FileContentY;
	my $DirContentW=$FileContentW;
	my $DirContentH=$FileContentH;

	my $AltY=$PathY;
	my $AltH=$PathH;

	my $PathX=$FileInnerBorderX;

	my $InfoX=$x1;
	my $InfoY=$PathY;
	my $InfoW=$FileInnerBorderX-$x1-$contentFrame;
	my $InfoH=$y2-$InfoY;
	my $InfoAlignment = "top-left";

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
	my $AltInnerBorderL = $altContentFrame;
	my $AltInnerBorderT = $altContentFrame;
	my $AltInnerBorderR = $altContentFrame;
	my $AltInnerBorderB = $altContentFrame;
	my $AltInnerBorderImg = "MetalInnerBorder.tga";
	my $AltInnerBorderImgL = 64.0;
	my $AltInnerBorderImgT = 64.0;
	my $AltInnerBorderImgR = 64.0;
	my $AltInnerBorderImgB = 64.0;
	my $AltContentX=$AltInnerBorderX+$AltInnerBorderL;
	my $AltContentY=$AltInnerBorderY+$AltInnerBorderT;
	my $AltContentW=$AltInnerBorderW-$AltInnerBorderL-$AltInnerBorderR;
	my $AltContentH=$AltContentW*$tallness;
	my $AltInnerBorderH=$AltContentH+$AltInnerBorderT+$AltInnerBorderB;

	my $AltW=$AltH*$AltInnerBorderW/($AltInnerBorderY+$AltInnerBorderH);
	my $AltX=$x2-$AltW;
	my $PathW=$AltX-$PathX-$PathH*0.4;

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

generate_theme(1.0/3.0 , "${ThemeName} 3:1" , "../${ThemeName}1.emFileManTheme");
generate_theme(9.0/16.0 , "${ThemeName} 16:9" , "../${ThemeName}2.emFileManTheme");
generate_theme(3.0/4.0 , "${ThemeName} 4:3" , "../${ThemeName}3.emFileManTheme");
