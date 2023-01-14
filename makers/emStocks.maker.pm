package emStocks;

use strict;
use warnings;

sub GetDependencies
{
	return ('emCore');
}

sub IsEssential
{
	return 0;
}

sub GetFileHandlingRules
{
	return ();
}

sub GetExtraBuildOptions
{
	return ();
}

sub Build
{
	shift;
	my %options=@_;

	system(
		@{$options{'unicc_call'}},
		"--math",
		"--rtti",
		"--exceptions",
		"--bin-dir"       , "bin",
		"--lib-dir"       , "lib",
		"--obj-dir"       , "obj",
		"--inc-search-dir", "include",
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emStocks",
		"src/emStocks/emStocksConfig.cpp",
		"src/emStocks/emStocksControlPanel.cpp",
		"src/emStocks/emStocksFetchPricesDialog.cpp",
		"src/emStocks/emStocksFileModel.cpp",
		"src/emStocks/emStocksFilePanel.cpp",
		"src/emStocks/emStocksFpPlugin.cpp",
		"src/emStocks/emStocksItemChart.cpp",
		"src/emStocks/emStocksItemPanel.cpp",
		"src/emStocks/emStocksListBox.cpp",
		"src/emStocks/emStocksPricesFetcher.cpp",
		"src/emStocks/emStocksRec.cpp"
	)==0 or return 0;

	return 1;
}
