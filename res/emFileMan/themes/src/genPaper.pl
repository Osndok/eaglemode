#!/usr/bin/perl

use strict;
use warnings;
use File::Basename;


sub generate_theme
	# Arguments: tallness, display name, display icon, file
{
	my $tallness = shift;
	my $DisplayName = shift;
	my $DisplayIcon = shift;
	my $fileName = shift;

	my $BackgroundColor = '{247 247 239}';
	my $SourceSelectionColor = '{200 245 200}';
	my $TargetSelectionColor = '{255 197 197}';
	my $NormalNameColor = '{18 18 16 255}';
	my $ExeNameColor = '{8 100 6 255}';
	my $DirNameColor = '{14 34 176 255}';
	my $FifoNameColor = '{170 20 70 255}';
	my $BlkNameColor = '{100 45 20 255}';
	my $ChrNameColor = '{65 92 15 255}';
	my $SockNameColor = '{140 20 140 255}';
	my $OtherNameColor = '{140 20 10 255}';
	my $PathColor = '{61 61 49 161}';
	my $SymLinkColor = '{80 151 162 255}';
	my $LabelColor = '{61 61 49 89}';
	my $InfoColor = '{61 61 49 117}';
	my $FileContentColor = $BackgroundColor;
	my $DirContentColor = $BackgroundColor;

	my $Height = $tallness;

	my $frame=(1.0+$tallness)*($tallness*3.0+1.0)*0.00525;

	my $BackgroundX = $frame/151.0*64.0;
	my $BackgroundY = $frame/151.0*64.0;
	my $BackgroundW = 1.0-$BackgroundX-$frame;
	my $BackgroundH = $tallness-$BackgroundY-$frame;
	my $BackgroundRX = 0.0;
	my $BackgroundRY = 0.0;

	my $OuterBorderX = $BackgroundX-$frame/151.0*64.0;
	my $OuterBorderY = $BackgroundY-$frame/151.0*63.0;
	my $OuterBorderW = $BackgroundX+$BackgroundW-$OuterBorderX+$frame/151.0*131.0;
	my $OuterBorderH = $BackgroundY+$BackgroundH-$OuterBorderY+$frame/151.0*151.0;
	my $OuterBorderImg = 'PaperBorder.tga';
	my $OuterBorderImgL = 337.0;
	my $OuterBorderImgT = 337.0;
	my $OuterBorderImgR = 391.0;
	my $OuterBorderImgB = 410.0;
	my $OuterBorderL = $frame/151.0*337.0;
	my $OuterBorderT = $frame/151.0*337.0;
	my $OuterBorderR = $frame/151.0*391.0;
	my $OuterBorderB = $frame/151.0*410.0;

	my $spacing=$frame*0.357;

	my $x1=$BackgroundX+$spacing;
	my $y1=$BackgroundY+$spacing;
	my $x2=$BackgroundX+$BackgroundW-$spacing;
	my $y2=$BackgroundY+$BackgroundH-$spacing;

	my $NameX=$x1;
	my $NameY=$y1;
	my $NameW=$x2-$x1;
	my $NameH=0.09*$NameW;
	my $NameAlignment = "left";

	my $PathY=$NameY+$NameH*1.03;
	my $PathH=$NameH*0.14;
	my $PathAlignment = "bottom-left";

	my $FileInnerBorderY = $PathY+$PathH+$spacing*0.1;
	my $FileInnerBorderH = $y2-$FileInnerBorderY;
	my $FileInnerBorderL = 0.0;
	my $FileInnerBorderT = 0.0;
	my $FileInnerBorderR = 0.0;
	my $FileInnerBorderB = 0.0;
	my $FileInnerBorderImg = "";
	my $FileInnerBorderImgL = 0.0;
	my $FileInnerBorderImgT = 0.0;
	my $FileInnerBorderImgR = 0.0;
	my $FileInnerBorderImgB = 0.0;
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
	my $InfoW=$FileInnerBorderX-$x1-$spacing;
	my $InfoH=$y2-$InfoY;
	my $InfoAlignment = "top-left";

	my $altContentFrame=$frame*0.143/$FileInnerBorderW;

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

	my $AltW=$AltH*$AltInnerBorderW/($AltInnerBorderY+$AltInnerBorderH);
	my $AltX=$x2-$AltW;
	my $PathW=$AltX-$PathX-$PathH*0.4;

	my $AltAltW=$AltAltH*$AltW/$AltH;
	my $AltAltX=1.0-$AltAltW;
	my $AltPathW=$AltAltX-$AltPathX-$AltPathH*0.4;

	my $MinContentVW=26.0/sqrt($Height);
	my $MinAltVW=15.0/sqrt($Height);

	my $px=1.0-$BackgroundW-$BackgroundX*2;
	my $py=$Height-$BackgroundH-$BackgroundY*2;
	my $DirPaddingL=0.0;
	my $DirPaddingT=0.0;
	my $DirPaddingR=0.0;
	my $DirPaddingB=0.0;
	my $LnkPaddingL=$px*0.15;
	my $LnkPaddingT=$py*0.15;
	my $LnkPaddingR=-$px*0.85;
	my $LnkPaddingB=-$py*0.85;

	my $fh;
	open($fh,">",$fileName);

	print($fh
		"#%rec:emFileManTheme%#\n".
		"\n".
		"DisplayName = \"$DisplayName\"\n".
		"DisplayIcon = \"$DisplayIcon\"\n".
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
		"MinAltVW = $MinAltVW\n".
		"DirPaddingL = $DirPaddingL\n".
		"DirPaddingT = $DirPaddingT\n".
		"DirPaddingR = $DirPaddingR\n".
		"DirPaddingB = $DirPaddingB\n".
		"LnkPaddingL = $LnkPaddingL\n".
		"LnkPaddingT = $LnkPaddingT\n".
		"LnkPaddingR = $LnkPaddingR\n".
		"LnkPaddingB = $LnkPaddingB\n"
	);

	close($fh);
}


my $ThemeName = basename($0);
$ThemeName =~ s/(^gen)|(.pl$)//g;

my $DisplayName = $ThemeName;

generate_theme(1.0/3.0 , "${DisplayName}", "theme_${ThemeName}.tga", "../${ThemeName}1.emFileManTheme");
generate_theme(9.0/16.0, "${DisplayName}", "theme_${ThemeName}.tga", "../${ThemeName}2.emFileManTheme");
generate_theme(3.0/4.0 , "${DisplayName}", "theme_${ThemeName}.tga", "../${ThemeName}3.emFileManTheme");
