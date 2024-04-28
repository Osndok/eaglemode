#!/usr/bin/perl
#-------------------------------------------------------------------------------
# This stock API script fetches the prices from alphavantage.co. A valid API key
# must be provided. The script limits the requests to five per minute and it
# uses the TIME_SERIES_DAILY function, so that a free non-premium API key should
# work.
#-------------------------------------------------------------------------------

use strict;
use warnings;
use Fcntl qw(:flock);
use File::Spec;
use HTTP::Request::Common;
use IO::Handle;
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


#-------------- Calculate number of days from start date to today --------------

$startDate =~ /^(\d+)-(\d+)-(\d+)$/ or die "Failed to parse start date";
my $days=int((time()-timelocal(0,0,0,$3,$2-1,$1-1900))/86400+1);


#-------------------- Limit to at most 5 requests a minute ---------------------
# This could be removed if you have a premium account.

{
	my $timeFile=File::Spec->tmpdir()."/last_time_alphavantage_co-$<";
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

my $url=
	'https://www.alphavantage.co/query'.
	'?function=TIME_SERIES_DAILY'.
	'&symbol='.$symbol.
	'&outputsize='.($days <= 100 ? 'compact' : 'full').
	'&datatype=csv'.
	'&apikey='.$apiKey
;

my $ua=LWP::UserAgent->new;
my $response=$ua->get($url);
if (!$response->is_success) { die($response->status_line); }


#------------------------------- Transform data --------------------------------

my $anyValidLineFound=0;

foreach my $line (split(/\n/,$response->content())) {
	if ($line =~ /^(\d+-\d+-\d+),([0-9.]+),([0-9.]+),([0-9.]+),([0-9.]+),/) {
		$anyValidLineFound=1;
		my $date=$1;
		my $open=$2;
		my $high=$3;
		my $low=$4;
		my $close=$5;
		print("$date $close\n");
	}
}

if (!$anyValidLineFound) {
	die(
		"Unexpected response from server:\n".
		substr($response->content(),0,700)."\n,"
	);
}
