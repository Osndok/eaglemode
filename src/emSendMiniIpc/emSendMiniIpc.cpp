//------------------------------------------------------------------------------
// emSendMiniIpc.cpp
//
// Copyright (C) 2004-2010 Oliver Hamann.
//
// Homepage: http://eaglemode.sourceforge.net/
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3 as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
// more details.
//
// You should have received a copy of the GNU General Public License version 3
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <emCore/emMiniIpc.h>


#if defined(__GNUC__) && defined(_WIN32)
	int _CRT_glob=0;
#endif

int main(int argc, char * argv[])
{
	emInitLocale();

	if (argc<2) {
		fprintf(
			stderr,
			"Usage: %s <server name> [<argument> [<argument>...]]\n",
			argv[0]
		);
		return 1;
	}
	try {
		emMiniIpcClient::TrySend(argv[1],argc-2,argv+2);
	}
	catch (emString errorMessage) {
		fprintf(stderr,"%s\n",errorMessage.Get());
		return 1;
	}
	return 0;
}
