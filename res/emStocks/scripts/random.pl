#!/usr/bin/perl
#-------------------------------------------------------------------------------
# This stock API script does not really connect to any API. Instead it just
# produces pseudo random prices. This can be useful only for tests and
# demonstrations.
#-------------------------------------------------------------------------------

use strict;
use warnings;
use Time::Local;

if (@ARGV != 3) {
	die("Three arguments required: symbol, start date, and API key,");
}
my $symbol   =$ARGV[0];
my $startDate=$ARGV[1];
my $apiKey   =$ARGV[2];

$startDate =~ /^(\d+)-(\d+)-(\d+)$/ or die "Failed to parse start date";
my $t=timelocal(0,0,0,$3,$2-1,$1-1900);
my $p=rand(50)+25;
while ($t<time()) {
	my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime($t);
	if ($wday>0 && $wday<6) {
		$p+=$p*0.13*rand(1)*rand(1)*rand(1)*rand(1)*(rand(1)<0.49?-1:1);
		if ($p<0.1) { $p=0.1; }
		printf("%04d-%02d-%02d %.3f\n",$year+1900,$mon+1,$mday,$p);
	}
	$t+=24*60*60;
}
