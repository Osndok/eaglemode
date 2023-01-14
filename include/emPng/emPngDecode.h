/*------------------------------------------------------------------------------
// emPngDecode.h
//
// Copyright (C) 2022 Oliver Hamann.
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
//----------------------------------------------------------------------------*/

#ifndef emPngDecode_h
#define emPngDecode_h

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


// These C functions are also used by the emBmp plugin for decoding PNG data in
// ICO files. It loads the emPng library dynamically and resolves the functions
// by name, if possible. It has copies of these function declarations - adapt
// that when changing anything here!

void * emPngStartDecoding(
	FILE * file, int * width, int * height, int * channelCount,
	int * passCount, char * infoBuf, size_t infoBufSize,
	char * errorBuf, size_t errorBufSize
);

int emPngContinueDecoding(
	void * instance, unsigned char * rowBuf, char * commentBuf,
	size_t commentBufSize, char * errorBuf, size_t errorBufSize
);

void emPngQuitDecoding(void * instance);


#ifdef __cplusplus
}
#endif

#endif
