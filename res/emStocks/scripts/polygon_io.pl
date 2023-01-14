#!/usr/bin/perl
#-------------------------------------------------------------------------------
# This stock API script fetches the prices from polygon.io. A valid API key must
# be provided. The script limits the requests to five per minute, so that a free
# basic API key should work.
#-------------------------------------------------------------------------------

use strict;
use warnings;
use Fcntl qw(:flock);
use File::Spec;
use HTTP::Request::Common;
use IO::Handle;
use JSON;
use LWP;
use Time::HiRes;
use Time::Local;


#------------------------------- Parse arguments -------------------------------

if (@ARGV != 3) {
	die("Three arguments required: symbol, start date, and API key,");
}
my $symbol   =$ARGV[0];
my $startDate=$ARGV[1];
my $apiKey   =$ARGV[2];

if ($symbol eq "") { die("Symbol is empty,"); }
if ($startDate eq "") { die("Start date is empty,"); }
if ($apiKey eq "") { die("API key is empty,"); }


#-------------------- Limit to at most 5 requests a minute ---------------------
# This could be removed if you have a premium account.

{
	my $timeFile=File::Spec->tmpdir()."/last_time_polygon_io-$<";
	my $fh;
	if (!open $fh, '+>>', $timeFile) { die("Failed to open $timeFile: $!,"); }
	if (!flock($fh, LOCK_EX)) { die("Failed to lock $timeFile: $!,"); }
	seek $fh, 0, 0;
	my $requestTime=readline($fh);
	my $currentTime=Time::HiRes::time();
	if (defined($requestTime)) {
		$requestTime+=12.1;
		if ($requestTime<$currentTime) { $requestTime=$currentTime; }
	}
	else {
		$requestTime=$currentTime;
	}
	seek $fh, 0, 0;
	truncate $fh, 0;
	print $fh $requestTime."\n";
	close $fh;
	Time::HiRes::usleep(($requestTime-$currentTime)*1000000);
}


#---------------------------- Perform HTTPS request ----------------------------

my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time());
my $endDate=sprintf("%04d-%02d-%02d",$year+1900,$mon+1,$mday);

my $url=
	'https://api.polygon.io/v2/aggs/ticker/'.$symbol.
	'/range/1/day/'.$startDate.'/'.$endDate.'?apiKey='.$apiKey
;

my $ua=LWP::UserAgent->new;
my $response=$ua->get($url);
if (!$response->is_success) { die($response->status_line); }


#--------------------- Helper for generating error message ---------------------

sub errAppend
{
	my $text=" in server response:\n";
	for (my $i=0; $i<9 && $i*80<length($response->content()); $i++) {
		$text.=substr($response->content(),$i*80,80)."\n";
	}
	return $text.",";
}


#------------------------------- Transform data --------------------------------

my $data=decode_json($response->content());

my $status=$data->{'status'};
if ($status ne 'OK' and $status ne 'DELAYED') {
	die("Bad status ".$status.errAppend());
}

my $results=$data->{'results'};
if (!defined($results)) { die("'results' missing".errAppend()); }
if (ref($results) ne 'ARRAY') { die("'results' not an array".errAppend()); }
if (@$results < 1) { die("'results' empty".errAppend());}

foreach my $element (@$results) {
	if (ref($element) ne "HASH") { die("Element not a hash".errAppend()); }
	my $c=$element->{'c'};
	if (!defined($c)) { die("'c' value missing".errAppend()); }
	if ($c !~ /^[.0-9]+$/) { die("'c' value invalid".errAppend()); }
	my $t=$element->{'t'};
	if (!defined($t)) { die("'t' value missing".errAppend()); }
	if ($t !~ /^[0-9]+$/) { die("'t' value invalid".errAppend()); }
	my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime($t/1000);
	my $date=sprintf("%04d-%02d-%02d",$year+1900,$mon+1,$mday);
	print("$date $c\n");
}
