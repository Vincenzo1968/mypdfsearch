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


/*
http://www.unicode.org/
 
------------------------------------------------------------------------
Unicode escape sequences in literal string:
https://en.wikipedia.org/wiki/Escape_sequences_in_C
https://en.cppreference.com/w/c/language/escape
https://it.cppreference.com/w/cpp/language/escape
------------------------------------------------------------------------
 
ESEMPI DI CONVERSIONE CON ICONV:
https://superuser.com/questions/381056/iconv-generating-utf-16-with-bom
http://xahlee.info/linux/linux_convert_file_encoding.html

http://murty4all.blogspot.com/2016/12/conversion-between-utf-16-utf-8-encoded.html
https://stackoverflow.com/questions/17287713/using-iconv-to-convert-from-utf-16le-to-utf-8
https://www.linuxquestions.org/questions/solaris-opensolaris-20/converting-utf-16-files-to-another-encoding-such-as-utf-8-a-630588/
https://www.tecmint.com/convert-files-to-utf-8-encoding-in-linux/
*/

/*
Da PDF3000_2008 pag. 86

For text strings encoded in Unicode, the first two bytes shall be 254 followed by 255. These two bytes represent
the Unicode byte order marker, U+FEFF, indicating that the string is encoded in the UTF-16BE (big-endian)
encoding scheme specified in the Unicode standard.
NOTE 3:
This mechanism precludes beginning a string using PDFDocEncoding with the two characters thorn
ydieresis, which is unlikely to be a meaningful beginning of a word or phrase.
 
QUINDI : Se i primi due byte sono 254 e 255, la stringa è in formato UTF-16BE.
*/

// OCCHIO : PDF3000_2008: pagg. 250-251 <----- *****

#include "mydecode.h"
#include "myoctal.h"

int decodeIsDelimiterChar(unsigned char c)
{
	switch ( c )
	{
		case ' ':
		case '\r':
		case '\n':
		case '\f':
		case '\t':
		case '\0':
			return DECODE_DELIM_SPACECHAR;
			break;
		case '%':
		case '(':
		case ')':
		case '<':
		case '>':
		case '[':
		case ']':
		case '{':
		case '}':
		case '/':
			return DECODE_DELIM_SPECIALSYMBOL;
			break;			
		default:
			return DECODE_DELIM_FALSE;
			break;
	}
	
	return DECODE_DELIM_FALSE;
}

wchar_t* FromUnicodeToUtf8WideCharString(const char *inbuf, size_t InbufSize, const char *InbufCharset)
{
	size_t insize = 0;
	size_t nconv;
	
	size_t OutbufSize;
	
	iconv_t cd;
	
	char *OutbufCharset = "WCHAR_T";
	
	const char *inptr; 
	char *wrptr = NULL;
	
	wchar_t *pMyWideString = NULL;
			
	pMyWideString = calloc(InbufSize + 1, sizeof(wchar_t));
	if ( NULL == pMyWideString )
	{
		perror("ERRORE myConvertUnicodeStringsToUtf8 calloc\n");
		return NULL;
	}
	wrptr = (char*)pMyWideString;
	
	//cd = iconv_open ("UTF-8", charset);
	cd = iconv_open(OutbufCharset, InbufCharset);
	if ( (iconv_t) -1 == cd)
	{
		// Something went wrong.  
		if (EINVAL == errno)
			wprintf(L"ERRORE FromUnicodeToUtf8WideCharString: conversion from '%s' to '%s' not available", InbufCharset, OutbufCharset);
		else
			perror ("iconv_open");
					
		free(pMyWideString);
		pMyWideString = NULL;
		
		return NULL;
	}
	
	inptr = inbuf;
	iconv(cd, NULL, NULL, &wrptr, &OutbufSize);
	insize += InbufSize;
	// Do the conversion.  
	nconv = iconv(cd, (char**)&inptr, &insize, &wrptr, &OutbufSize);
	if ( (size_t) -1 == nconv)
	{
		if ( EINVAL == errno)
			memmove((void*)inbuf, (void*)inptr, insize);
	}
	
	// Terminate the output string.  
	if ( OutbufSize >= sizeof (wchar_t) )
		*((wchar_t*) wrptr) = L'\0';
			
	return pMyWideString;
}

/*
wchar_t* FromPdfDocEncodingToUtf8WideCharString(BYTE *pszPdf, size_t mbslen)
{
	size_t k;	
		
	wchar_t c;
	
	wchar_t *wcs = NULL;       // Pointer to converted wide character string 
	wchar_t *wcs2 = NULL;
		
	wcs = calloc(mbslen + 1, sizeof(wchar_t));
	if (wcs == NULL)
	{
		perror("calloc");
		return NULL;
	}
	
	wcs2 = calloc(mbslen + 1, sizeof(wchar_t));
	if (wcs2 == NULL)
	{
		perror("calloc");
		free(wcs);
		wcs = NULL;
		return NULL;
	}
	
	if (mbstowcs(wcs, (char*)pszPdf, mbslen + 1) == (size_t) -1)
	{
		perror("mbstowcs");
		free(wcs);
		wcs = NULL;
		free(wcs2);
		wcs2 = NULL;
		return NULL;
	}
					
	for ( k = 0; k < mbslen; k++ )
	{
		c = wcs[k];
						
		switch ( c )
		{
			case 23:
				wcs2[k] = 9239;
				break;
			case 24:
				wcs2[k] = 728;
				break;
			case 25:
				wcs2[k] = 711;
				break;
			case 26:
				wcs2[k] = 94;
				break;
			case 27:
				wcs2[k] = 183;
				break;				
			case 28:
				wcs2[k] = 8221;
				break;				
			case 29:
				wcs2[k] = 731;
				break;
			case 30:
				wcs2[k] = 176;
				break;
			case 31:
				wcs2[k] = 126;
				break;
			case 128:
				wcs2[k] = 8226;
				break;
			case 129:
				wcs2[k] = 8224;
				break;
			case 130:
				wcs2[k] = 8225;
				break;				
			case 131:
				wcs2[k] = 8230;
				break;
			case 132:
				wcs2[k] = 8212;
				break;
			case 133:
				wcs2[k] = 8211;
				break;
			case 134:
				wcs2[k] = 402;
				break;				
			case 135:
				wcs2[k] = 8260;
				break;
			case 136:
				wcs2[k] = 8249;
				break;				
			case 137:
				wcs2[k] = 8250;
				break;
			case 138:
				wcs2[k] = 352;
				break;
			case 139:
				wcs2[k] = 8240;
				break;
			case 140:
				wcs2[k] = 8222;
				break;
			case 141:
				wcs2[k] = 8220;
				break;
			case 142:
				wcs2[k] = 8221;
				break;
			case 143:
				wcs2[k] = 8216;
				break;
			case 144:
				wcs2[k] = 8217;
				break;
			case 145:
				wcs2[k] = 8218;
				break;
			case 146:
				wcs2[k] = 8482;
				break;
			case 147:
				wcs2[k] = 64257;
				break;
			case 148:
				wcs2[k] = 64258;
				break;
			case 149:
				wcs2[k] = 322;
				break;
			case 150:
				wcs2[k] = 338;
				break;
			case 151:
				wcs2[k] = 352;
				break;
			case 152:
				wcs2[k] = 376;
				break;
			case 153:
				wcs2[k] = 381;
				break;
			case 154:
				wcs2[k] = 305;
				break;
			case 155:
				wcs2[k] = 322;
				break;
			case 156:
				wcs2[k] = 339;
				break;				
			case 157:
				wcs2[k] = 353;
				break;
			case 158:
				wcs2[k] = 382;
				break;
			case 160:
				wcs2[k] = 8364;
				break;
			case 181:
				wcs2[k] = 956;
				break;
			default:
				if ( c == 127 || c == 159 || c == 173 )
				{
					wprintf(L"ERRORE FromPdfEncodingToMultibyteString: la stringa contiene il codice indefinito -> %d\n", (int)c);
					free(wcs2);
					wcs2 = NULL;
					free(wcs);
					wcs = NULL;					
					return NULL;
				}
				wcs2[k] = c;
				break;
		}
	}
		
	wcs2[k] = L'\0';
		
	free(wcs);
	wcs = NULL;	
			
	return wcs2;
}
*/


// https://iconoun.com/articles/collisions/
// https://iconoun.com/articles/collisions/Unicode_Presentation.pdf

// https://www.asciitable.it/
// http://www.fileformat.info/info/charset/UTF-8/list.htm
// https://graphemica.com/

wchar_t* FromPdfDocEncodingToUtf8WideCharString_NEW(const BYTE *pszPdf, size_t mbslen)
{
	size_t k;	
	size_t x;
	
	wchar_t c;
	BYTE myByte;
	
	BYTE *myByteString = NULL;
	
	wchar_t *wcs = NULL;       // Pointer to converted wide character string 
	wchar_t *wcs2 = NULL;
		
	wcs = calloc(mbslen + 1, sizeof(wchar_t));
	if (wcs == NULL)
	{
		perror("calloc");
		return NULL;
	}
	
	wcs2 = calloc(mbslen + 1, sizeof(wchar_t));
	if (wcs2 == NULL)
	{
		perror("calloc");
		free(wcs);
		wcs = NULL;
		return NULL;
	}
	
	myByteString = calloc(mbslen * 4 + 1, sizeof(BYTE));
	if (myByteString == NULL)
	{
		perror("calloc");
		free(wcs);
		wcs = NULL;
		free(wcs2);
		wcs2 = NULL;
		return NULL;
	}	
	
	x = 0;
	for ( k = 0; k < mbslen; k++ )
	{
		myByte = pszPdf[k];
		
		switch ( myByte )
		{
			case 128:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x82;
				myByteString[x++] = 0xAC;
				break;
			case 129:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0xA0;		
				break;			
			case 130:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0x9A;			
				break;			
			case 131:
				myByteString[x++] = 0xC6;
				myByteString[x++] = 0x92;			
				break;			
			case 132:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0x9E;			
				break;			
			case 133:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0xA6;			
				break;			
			case 134:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0xA0;			
				break;			
			case 135:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0xA1;			
				break;			
			case 136:
				myByteString[x++] = 0xCB;
				myByteString[x++] = 0x86;			
				break;			
			case 137:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0xB0;			
				break;			
			case 138:
				myByteString[x++] = 0xC5;
				myByteString[x++] = 0xA0;			
				break;			
			case 139:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0xB9;			
				break;			
			case 140:
				myByteString[x++] = 0xC5;
				myByteString[x++] = 0x92;			
				break;			
			//case 141:
			//	myByteString[x++] = 0xC4;
			//	myByteString[x++] = 0xB1;			
			//	break;			
			case 142:
				myByteString[x++] = 0xC5;
				myByteString[x++] = 0xBD;			
				break;			
			//case 143:
			//	myByteString[x++] = 0xC4;
			//	myByteString[x++] = 0xB1;			
			//	break;			
			//case 144:
			//	myByteString[x++] = 0xC4;
			//	myByteString[x++] = 0xB1;			
			//	break;			
			case 145:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0x98;			
				break;			
			case 146:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0x99;			
				break;			
			case 147:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0x9C;			
				break;			
			case 148:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0x9D;			
				break;			
			case 149:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0xA2;			
				break;			
			case 150:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0x93;			
				break;			
			case 151:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0x94;			
				break;			
			case 152:
				myByteString[x++] = 0xCB;
				myByteString[x++] = 0x9C;			
				break;			
			case 153:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x84;
				myByteString[x++] = 0xA2;			
				break;
			case 154:
				myByteString[x++] = 0xC4;
				myByteString[x++] = 0xB1;			
				break;
			case 155:
				myByteString[x++] = 0xE2;
				myByteString[x++] = 0x80;
				myByteString[x++] = 0xBA;
				break;
			case 156:
				myByteString[x++] = 0xC5;
				myByteString[x++] = 0x93;			
				break;
			//case 157:
			//	myByteString[x++] = 0xC4;
			//	myByteString[x++] = 0xB1;			
			//	break;
			case 158:
				myByteString[x++] = 0xC5;
				myByteString[x++] = 0xBE;
				break;
			case 159:
				myByteString[x++] = 0xC5;
				myByteString[x++] = 0xB8;
				break;
			case 160:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xA0;
				break;
			case 161:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xA1;
				break;
			case 162:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xA2;
				break;
			case 163:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xA3;
				break;
			case 164:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xA4;
				break;
			case 165:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xA5;
				break;
			case 166:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xA6;
				break;
			case 167:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xA7;
				break;
			case 168:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xA8;
				break;
			case 169:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xA9;
				break;
			case 170:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xAA;
				break;
			case 171:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xAB;
				break;
			case 172:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xAC;
				break;
			case 173:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xAD;
				break;
			case 174:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xAE;
				break;
			case 175:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xAF;
				break;
			case 176:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xB0;
				break;
			case 177:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xB1;
				break;
			case 178:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xB2;
				break;
			case 179:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xB3;
				break;
			case 180:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xB4;
				break;
			case 181:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xB5;
				break;
			case 182:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xB6;
				break;
			case 183:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xB7;
				break;
			case 184:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xB8;
				break;
			case 185:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xB9;
				break;
			case 186:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xbA;
				break;
			case 187:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xBB;
				break;
			case 188:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xBC;
				break;
			case 189:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xBD;
				break;
			case 190:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xBE;
				break;
			case 191:
				myByteString[x++] = 0xC2;
				myByteString[x++] = 0xBF;
				break;
			case 192:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x80;
				break;
			case 193:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x81;
				break;
			case 194:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x82;
				break;
			case 195:
				k++;
				myByte = pszPdf[k];
				if ( myByte == 161 )
				{
					myByteString[x++] = 0xC3;
					myByteString[x++] = 0xA1;
				}
				else
				{
					k--;
					myByteString[x++] = 0xC3;
					myByteString[x++] = 0x83;
				}
				
				// myByteString[x++] = 0xC3;
				// myByteString[x++] = 0x83;				
				break;
			case 196:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x84;
				break;
			case 197:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x85;
				break;
			case 198:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x86;
				break;
			case 199:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x87;
				break;
			case 200:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x88;
				break;
			case 201:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x89;
				break;
			case 202:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x8A;
				break;
			case 203:
				//myByteString[x++] = 0xC3;
				//myByteString[x++] = 0x8B;
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xAC;
				break;
			case 204:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x8C;			
				break;
			case 205:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x8D;			
				break;
			case 206:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x8E;			
				break;
			case 207:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x8F;			
				break;
			case 208:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x90;			
				break;
			case 209:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x91;			
				break;
			case 210:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x92;			
				break;
			case 211:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x93;			
				break;
			case 212:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x94;			
				break;
			case 213:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x95;			
				break;
			case 214:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x96;			
				break;
			case 215:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x97;			
				break;
			case 216:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x98;			
				break;
			case 217:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x99;			
				break;
			case 218:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x9A;			
				break;
			case 219:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x9B;			
				break;
			case 220:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x9C;
				break;
			case 221:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x9D;			
				break;
			case 222:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x9E;			
				break;
			case 223:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0x9F;			
				break;
			case 224:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xA0;
				break;
			case 225:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xA1;			
				break;
			case 226:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xA2;			
				break;
			case 227:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xA3;			
				break;
			case 228:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xA4;			
				break;
			case 229:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xA5;			
				break;
			case 230:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xA6;			
				break;
			case 231:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xA7;			
				break;
			case 232:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xA8;			
				break;
			case 233:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xA9;			
				break;
			case 234:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xAA;			
				break;
			case 235:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xAB;			
				break;
			case 236:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xAC;			
				break;
			case 237:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xAD;			
				break;
			case 238:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xAE;			
				break;
			case 239:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xAF;			
				break;
			case 240:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xB0;			
				break;
			case 241:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xB1;			
				break;
			case 242:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xB2;			
				break;
			case 243:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xB3;			
				break;
			case 244:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xB4;			
				break;
			case 245:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xB5;			
				break;
			case 246:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xB6;			
				break;
			case 247:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xB7;			
				break;
			case 248:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xB8;			
				break;
			case 249:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xB9;			
				break;
			case 250:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xBA;			
				break;
			case 251:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xBD;			
				break;
			case 252:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xBC;			
				break;
			case 253:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xBD;			
				break;
			case 254:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xBE;			
				break;
			case 255:
				myByteString[x++] = 0xC3;
				myByteString[x++] = 0xBF;
				break;								
			default:
				myByteString[x++] = myByte;
				break;
		}		
	}
	myByteString[x] = '\0';
		
	//if (mbstowcs(wcs, (char*)pszPdf, mbslen + 1) == (size_t) -1)
	if (mbstowcs(wcs, (char*)myByteString, x + 1) == (size_t) -1)
	{
		perror("mbstowcs");
		free(wcs);
		wcs = NULL;
		free(wcs2);
		wcs2 = NULL;
		return NULL;
	}
					
	for ( k = 0; k < mbslen; k++ )
	{
		c = wcs[k];
				
		switch ( c )
		{
			case 23:
				wcs2[k] = 9239;
				break;
			case 24:
				wcs2[k] = 728;
				break;
			case 25:
				wcs2[k] = 711;
				break;
			case 26:
				wcs2[k] = 94;
				break;
			case 27:
				wcs2[k] = 183;
				break;				
			case 28:
				wcs2[k] = 8221;
				break;				
			case 29:
				wcs2[k] = 731;
				break;
			case 30:
				wcs2[k] = 176;
				break;
			case 31:
				wcs2[k] = 126;
				break;
			case 128:
				wcs2[k] = 8226;
				break;
			case 129:
				wcs2[k] = 8224;
				break;
			case 130:
				wcs2[k] = 8225;
				break;				
			case 131:
				wcs2[k] = 8230;
				break;
			case 132:
				wcs2[k] = 8212;
				break;
			case 133:
				wcs2[k] = 8211;
				break;
			case 134:
				wcs2[k] = 402;
				break;				
			case 135:
				wcs2[k] = 8260;
				break;
			case 136:
				wcs2[k] = 8249;
				break;				
			case 137:
				wcs2[k] = 8250;
				break;
			case 138:
				wcs2[k] = 352;
				break;
			case 139:
				wcs2[k] = 8240;
				break;
			case 140:
				wcs2[k] = 8222;
				break;
			case 141:
				wcs2[k] = 8220;
				break;
			case 142:
				wcs2[k] = 8221;
				break;
			case 143:
				wcs2[k] = 8216;
				break;
			case 144:
				wcs2[k] = 8217;
				break;
			case 145:
				wcs2[k] = 8218;
				break;
			case 146:
				wcs2[k] = 8482;
				break;
			case 147:
				wcs2[k] = 64257;
				break;
			case 148:
				wcs2[k] = 64258;
				break;
			case 149:
				wcs2[k] = 322;
				break;
			case 150:
				wcs2[k] = 338;
				break;
			case 151:
				wcs2[k] = 352;
				break;
			case 152:
				wcs2[k] = 376;
				break;
			case 153:
				wcs2[k] = 381;
				break;
			case 154:
				wcs2[k] = 305;
				break;
			case 155:
				wcs2[k] = 322;
				break;
			case 156:
				wcs2[k] = 339;
				break;				
			case 157:
				wcs2[k] = 353;
				break;
			case 158:
				wcs2[k] = 382;
				break;
			case 160:
				wcs2[k] = 8364;
				break;
			case 181:
				wcs2[k] = 956;
				break;
			default:
				if ( c == 127 || c == 159 || c == 173 )
				{
					wprintf(L"ERRORE FromPdfEncodingToMultibyteString: la stringa contiene il codice indefinito -> %d\n", (int)c);
					free(wcs2);
					wcs2 = NULL;
					free(wcs);
					wcs = NULL;	
					free(myByteString);
					myByteString = NULL;
					return NULL;
				}
				wcs2[k] = c;
				break;
		}
	}
	
		
	wcs2[k] = L'\0';
		
	free(wcs);
	wcs = NULL;	
	
	free(myByteString);
	myByteString = NULL;
			
	return wcs2;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("myzlib: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

int myInflate(unsigned char **dest, unsigned long int *destLen, const unsigned char *source, unsigned long int sourceLen)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    
    unsigned int OffsetSource;
    unsigned int bytesToCopy;
    int bytesRemaining;
    
    unsigned int OffsetDest;
    
    unsigned long int destLenNew;
    
    unsigned char *destNew = NULL;
    
    destLenNew = *destLen;

    // allocate inflate state 
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    
    ret = inflateInit(&strm);
    if (ret != Z_OK)
    {
        return ret;
	}

	OffsetSource = 0;
	bytesToCopy = 0;
	bytesRemaining = sourceLen;	
    if ( CHUNK <= bytesRemaining )
    {
		bytesToCopy = CHUNK;
    }
    else
    {
		bytesToCopy = bytesRemaining;
	}
	
	OffsetDest = 0;
	
    // decompress until deflate stream ends or end of file 
    do {		      
		memcpy((void*)in, (const void*)(source + OffsetSource), (size_t)bytesToCopy);
		strm.avail_in = bytesToCopy;
		        
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
        
		OffsetSource += bytesToCopy;
		bytesRemaining -= bytesToCopy;				
		if ( CHUNK <= bytesRemaining )
		{
			bytesToCopy = CHUNK;
		}
		else
		{
			bytesToCopy = bytesRemaining;
		}        

        // run inflate() on input until output buffer not full 
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  // state not clobbered 
            switch (ret)
            {
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;     // and fall through 
					(void)inflateEnd(&strm);
					return ret;
					break;
				case Z_DATA_ERROR:
					ret = Z_DATA_ERROR;     // and fall through 
					(void)inflateEnd(&strm);
					return ret;
					break;				
				case Z_MEM_ERROR:
					ret = Z_DATA_ERROR;     // and fall through 
					(void)inflateEnd(&strm);
					return ret;
					break;
			}
            
            have = CHUNK - strm.avail_out;
            
            if ( have > (destLenNew - OffsetDest) )
            {
				destLenNew += have;
				destNew = (unsigned char*)realloc(*dest, destLenNew * sizeof(unsigned char) + sizeof(unsigned char));
				if ( NULL == destNew )
				{
					wprintf(L"ERRORE myInf. Impossibile reallocare %lu bytes per dest\n", destLenNew * sizeof(char) + sizeof(unsigned char));
					return Z_DATA_ERROR;
				}
				*dest = destNew;
			}
            
            memcpy(*dest + OffsetDest, out, have);
            OffsetDest += have;
            
            
        } while (strm.avail_out == 0);

        // done when inflate() says it's done 
    } while (ret != Z_STREAM_END);

    // clean up and return 
    (void)inflateEnd(&strm);
    
    *destLen = OffsetDest;
        
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
