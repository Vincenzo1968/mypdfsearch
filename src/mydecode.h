/*
   Copyright (C) 2019 Vincenzo Lo Cicero

   Author: Vincenzo Lo Cicero.
   e-mail: vincenzo.locicero@libero.it
          
   This file is part of mypdfsearch.

   mypdfsearch is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   mypdfsearch is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with mypdfsearch.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MYDECODE__
#define __MYDECODE__

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include <stdint.h>
#include <iconv.h>
#include <locale.h>
#include <wchar.h>
#include "zlib.h"

#define BYTE uint8_t

#define CHUNK 16384
//#define CHUNK 1000

#define DECODE_DELIM_FALSE 0
#define DECODE_DELIM_SPACECHAR 1
#define DECODE_DELIM_SPECIALSYMBOL 2

#define STRING_PDFENCODING 1
#define STRING_UTF_16BE 2

#define MAX_STRING_LENTGTH_IN_CONTENT_STREAM 32767

typedef enum tagDecodeStates
{
	DS0,
	DS1,
	DS2,
	DS3,
	DS3_BIS,
	DS4,
	DS5,
	DS6,
	DS7
} DecodeStates;

void zerr(int ret);

int myInflate(unsigned char **dest, unsigned long int *destLen, const unsigned char *source, unsigned long int sourceLen);

int ZEXPORT myUncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong *sourceLen);

wchar_t* FromPdfDocEncodingToUtf8WideCharString(BYTE *pszPdf, size_t mbslen);
wchar_t* FromPdfDocEncodingToUtf8WideCharString_NEW(const BYTE *pszPdf, size_t mbslen);
wchar_t* FromUnicodeToUtf8WideCharString(const char *inbuf, size_t InbufSize, const char *InbufCharset);

#endif /* __MYDECODE__ */
