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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <locale.h>
#include <wchar.h>

#include "myGenHashTable.h"
#include "vlEncoding.h"

int GenStringHashFuncNew(GenHashTable_t* p, const void* pKey, uint32_t keysize);
int GenStringCompareFuncNew(const void* pKey1, uint32_t keysize1, const void* pKey2, uint32_t keysize2);
int GenUint16HashFunc(GenHashTable_t* p, const void* pKey, uint32_t keysize);
int GenUint16CompareFunc(const void* pKey1, uint32_t keysize1, const void* pKey2, uint32_t keysize2);

int GenStringHashFuncNew(GenHashTable_t* p, const void* pKey, uint32_t keysize)
{
	unsigned n = 0;
	const uint8_t* s = (const uint8_t*)pKey;
	
	//UNUSED(keysize);
	
	uint32_t k = 0;
	
	for ( k = 0; k < keysize; k++, s++)
		n = 31 * n + *s;
		
	return n % p->genhtSize;
}

int GenStringCompareFuncNew(const void* pKey1, uint32_t keysize1, const void* pKey2, uint32_t keysize2)
{
	//UNUSED(keysize1);
	//UNUSED(keysize2);
	
	int8_t *pszKey1 = (int8_t*)pKey1;
	int8_t *pszKey2 = (int8_t*)pKey2;
	
	uint32_t x;
	uint32_t k;
	
	k = keysize1 <= keysize2 ? keysize1 : keysize2;
	
	//printf("\n\nGenStringCompareFuncNew: k = %u; keysize1 = %u, keysize2 = %u\n", k, keysize1, keysize2);
	for ( x = 0; x < k; x++ )
	{
	//	printf("\tpszKey1[%u] = %u; pszKey2[%u] = %u\n", x, pszKey1[x], x, pszKey2[x]); 
		if ( pszKey1[x] > pszKey2[x] )
			return 1;
		else if ( pszKey1[x] < pszKey2[x] )
			return -1;
	}
	//printf("\n\n");
	
	return 0;
	
	//return strncmp((char*)pKey1, (char*)pKey2, BLOCK_SIZE - 1);
}

int GenUint16HashFunc(GenHashTable_t* p, const void* pKey, uint32_t keysize)
{
	UNUSED(p);
	UNUSED(keysize);
	
	uint16_t *pret = (uint16_t*)pKey;
		
	//return (int)(*pret) % p->genhtSize;
	return (int)(*pret);
}

int GenUint16CompareFunc(const void* pKey1, uint32_t keysize1, const void* pKey2, uint32_t keysize2)
{
	UNUSED(keysize1);
	UNUSED(keysize2);
	
	uint16_t *a = (uint16_t*)pKey1;
	uint16_t *b = (uint16_t*)pKey2;
	
	if ( *a > *b )
		return 1;
	else if ( *a < *b )
		return -1;
	else
		return 0;		
}

// Use to print the data in binary format
void PrintInBinary64_Wide(uint64_t n)
{
	int64_t iPos;
	int64_t x = 1;
	
	for ( iPos = (INT_BITS_64 - 1 ); iPos >= 0; iPos-- )
	{
		(n & (x << iPos)) ? wprintf(L"1") : wprintf(L"0");	
	}	
}

void PrintInBinary64(uint64_t n)
{
	int64_t iPos;
	int64_t x = 1;
	
	for ( iPos = (INT_BITS_64 - 1 ); iPos >= 0; iPos-- )
	{
		(n & (x << iPos)) ? printf("1") : printf("0");	
	}	
}

void PrintInBinary32_Wide(uint32_t n)
{
	int32_t iPos;
	int32_t x = 1;
	
	for ( iPos = (INT_BITS_32 - 1 ); iPos >= 0; iPos-- )
	{
		(n & (x << iPos)) ? wprintf(L"1") : wprintf(L"0");	
	}	
}

void PrintInBinary32(uint32_t n)
{
	int32_t iPos;
	int32_t x = 1;
	
	for ( iPos = (INT_BITS_32 - 1 ); iPos >= 0; iPos-- )
	{
		(n & (x << iPos)) ? printf("1") : printf("0");	
	}	
}

void PrintInBinary16_Wide(uint16_t n)
{
	int16_t iPos;
	int16_t x = 1;
	
	for ( iPos = (INT_BITS_16 - 1 ); iPos >= 0; iPos-- )
	{
		(n & (x << iPos)) ? wprintf(L"1") : wprintf(L"0");	
	}	
}

void PrintInBinary16(uint16_t n)
{
	int16_t iPos;
	int16_t x = 1;
	
	for ( iPos = (INT_BITS_16 - 1 ); iPos >= 0; iPos-- )
	{
		(n & (x << iPos)) ? printf("1") : printf("0");	
	}	
}

void PrintInBinary8_Wide(uint8_t n)
{
	int8_t iPos;
	int8_t x = 1;
	
	for ( iPos = (INT_BITS_8 - 1 ); iPos >= 0; iPos-- )
	{
		(n & (x << iPos)) ? wprintf(L"1") : wprintf(L"0");	
	}	
}

void PrintInBinary8(uint8_t n)
{
	int8_t iPos;
	int8_t x = 1;
	
	for ( iPos = (INT_BITS_8 - 1 ); iPos >= 0; iPos-- )
	{
		(n & (x << iPos)) ? printf("1") : printf("0");	
	}	
}

// ****************************************************************** AsciiHex **************************************************************

unsigned char * asciiHexEncode(unsigned char * pStream, size_t len, size_t *pLenOut, uint32_t *pErrorCode)
{
	size_t k;
	size_t y;
	
	unsigned char * pOutput = NULL;
	
	char s[3];
	
	*pErrorCode = ASCIIHEX_ERROR_NONE;
			
	*pLenOut = len * 2 * sizeof(unsigned char) + sizeof(unsigned char);
	pOutput = (unsigned char *)malloc(*pLenOut);
	if ( NULL == pOutput )
	{
		*pErrorCode = ASCIIHEX_ERROR_MEMALLOC_FAILED;
		*pLenOut = 0;
		return NULL;
	}
	for ( k = 0; k < *pLenOut; k++ )
		pOutput[k] = '>';
				
	y = 0;
	for ( k = 0; k < len; k++ )
	{	
		sprintf(s, "%X", pStream[k]);
		if ( pStream[k] > 0xF )
		{
			memcpy(pOutput + y, s, 2);
			y += 2;
		}
		else
		{
			memcpy(pOutput + y, "0", 1);
			y++;			
			memcpy(pOutput + y, s, 1);
			y++;			
		}		
	}
			
	return pOutput;
}

unsigned char * asciiHexDecode(unsigned char * pStream, size_t len, size_t *pLenOut, uint32_t *pErrorCode)
{
	unsigned char cHexadecimal;
	
	size_t lenNew;
	
	size_t k;
	size_t y;
	size_t x;
	size_t z;
		
	unsigned char * pOutput = NULL;
	
	*pErrorCode = ASCIIHEX_ERROR_NONE;
	
	*pLenOut = 0;

	if ( pStream[len - 1] != '>' )
	{
		*pErrorCode = ASCIIHEX_ERROR_EOD_NOT_FOUND;
		*pLenOut = 0;
		return NULL;
	}

	lenNew = len - 1;
	if ( 0 != lenNew % 2 )
	{
		//bZeroFollowLastDigit = 1;
		lenNew++;
		//wprintf(L"\n\nEHI!!! sostituisco '%c' con '0'\n", pStream[lenNew]);
		pStream[lenNew - 1] = '0';
		//wprintf(L"\tECCO: pStream[%d] = '%c'\n\n", lenNew - 1, pStream[lenNew - 1]);
	}
		
	*pLenOut = lenNew / 2;
		
	pOutput = (unsigned char *)malloc(*pLenOut * sizeof(unsigned char));
	if ( NULL == pOutput )
	{
		*pErrorCode = ASCIIHEX_ERROR_MEMALLOC_FAILED;
		*pLenOut = 0;
		return NULL;
	}
	for ( k = 0 ; k < lenNew * 4 * sizeof(unsigned char); k++ )
		pOutput[k] = '\0';
		
	//wprintf(L"\n\nasciiHexDecode: len = %d; lenNew = %d; *pLenOut = %d\n\n", len, lenNew, *pLenOut);
	
	y = 0;
	z = 0;
	for ( x = 0; x < lenNew/2; x++ )
	{
		switch ( pStream[y] )
		{
			case '0':
				cHexadecimal = 0;
				break;
			case '1':
				cHexadecimal = 16; // 1 * 16;
				break;
			case '2':
				cHexadecimal = 32; // 2 * 16;
				break;
			case '3':
				cHexadecimal = 48; // 3 * 16;
				break;
			case '4':
				cHexadecimal = 64; // 4 * 16;
				break;
			case '5':
				cHexadecimal = 80; // 5 * 16;
				break;
			case '6':
				cHexadecimal = 96; // 6 * 16;
				break;
			case '7':
				cHexadecimal = 112; // 7 * 16;
				break;
			case '8':
				cHexadecimal = 128; // 8 * 16;
				break;
			case '9':
				cHexadecimal = 144; // 9 * 16;
				break;
			case 'A':
			case 'a':
				cHexadecimal = 160; // 10 * 16;
				break;
			case 'B':
			case 'b':
				cHexadecimal = 176; // 11 * 16;
				break;
			case 'C':
			case 'c':
				cHexadecimal = 192; // 12 * 16;
				break;
			case 'D':
			case 'd':
				cHexadecimal = 208 ;// 13 * 16;
				break;
			case 'E':
			case 'e':
				cHexadecimal = 224; // 14 * 16;
				break;
			case 'F':
			case 'f':
				cHexadecimal = 240; // 15 * 16;
				break;
			default:
				//wprintf(L"ERRORE asciiHexDecode 1: carattere non valido: pStream[%d] = '%c'; z = %d\n", y, pStream[y], z); 
				*pErrorCode = ASCIIHEX_ERROR_UNEXPECTED_CHAR;
				free(pOutput);
				pOutput = NULL;
				*pLenOut = 0;
				return NULL;
				break;
		}
						
		switch ( pStream[y + 1] )
		{
			case '0':
				//cHexadecimal += 0;
				break;
			case '1':
				cHexadecimal += 1; // 1 * 16;
				break;
			case '2':
				cHexadecimal += 2;
				break;
			case '3':
				cHexadecimal += 3;
				break;
			case '4':
				cHexadecimal += 4;
				break;
			case '5':
				cHexadecimal += 5;
				break;
			case '6':
				cHexadecimal += 6;
				break;
			case '7':
				cHexadecimal += 7;
				break;
			case '8':
				cHexadecimal += 8;
				break;
			case '9':
				cHexadecimal += 9;
				break;
			case 'A':
			case 'a':
				cHexadecimal += 10;
				break;
			case 'B':
			case 'b':
				cHexadecimal += 11;
				break;
			case 'C':
			case 'c':
				cHexadecimal += 12;
				break;
			case 'D':
			case 'd':
				cHexadecimal += 13;
				break;
			case 'E':
			case 'e':
				cHexadecimal += 14;
				break;
			case 'F':
			case 'f':
				cHexadecimal += 15;
				break;
			default:
				//wprintf(L"ERRORE asciiHexDecode 2: carattere non valido: pStream[%d] = '%c'; z = %d\n", y + 1, pStream[y + 1], z); 
				*pErrorCode = ASCIIHEX_ERROR_UNEXPECTED_CHAR;
				free(pOutput);
				pOutput = NULL;
				*pLenOut = 0;
				return NULL;
				break;
		}
						
		pOutput[z] = cHexadecimal;
		z++;						
							
		y += 2;
	}
	
	return pOutput;
}

// ****************************************************************** Ascii85 **************************************************************

unsigned char * ascii85Encode(unsigned char * pStream, size_t len, size_t *pLenOut, uint32_t *pErrorCode)
{
	size_t k;
	size_t y;
	
	uint32_t countBlocks = 0;
	
	unsigned char * pStreamNew = NULL;
	size_t lenNew;
	uint32_t numPaddingInput;
	
	unsigned char * pOutput = NULL;
	
	uint32_t num;
	uint32_t numTemp;
	
	uint32_t howmanyBlocks;
	uint32_t numPadding;
	
	unsigned char b1, b2, b3, b4;
	unsigned char c0, c1, c2, c3, c4;
	
	uint32_t nMod;
	
	*pErrorCode = ASCII85_ERROR_NONE;
	
	nMod = len % 4;
	if ( nMod > 0 )
		numPaddingInput = 4 - nMod;
	else
		numPaddingInput = 0;
		
	lenNew = len + numPaddingInput;
	
	pStreamNew = (unsigned char*)malloc(lenNew * sizeof(unsigned char));
	if ( NULL == pStreamNew )
	{
		*pErrorCode = ASCII85_ERROR_MEMALLOC_FAILED;
		*pLenOut = 0;
		return NULL;
	}
	for ( k = 0; k < len; k++ )
		pStreamNew[k] = pStream[k];
	for ( ; k < lenNew; k++ )
		pStreamNew[k] = '\0';
	
	
	//numPadding = len % 5;
	numPadding = lenNew % 5;
	
	howmanyBlocks = lenNew / 5;
	if ( numPadding != 0 )
		howmanyBlocks++;
		
	//wprintf(L"\nlen = %d; numPaddingInput = %d; lenNew = %d\n", len, numPaddingInput, lenNew);
		
	//wprintf(L"LEN INPUT = %d; lenNew = %d; %d/5 = %d; BLOCCHI NECESSARI = %d; numPadding = %d\n", len, lenNew, lenNew, lenNew/5, howmanyBlocks, numPadding);
	
	*pLenOut = howmanyBlocks * 5 * sizeof(unsigned char);
	pOutput = (unsigned char *)malloc(*pLenOut * 2);
	if ( NULL == pOutput )
	{
		*pErrorCode = ASCII85_ERROR_MEMALLOC_FAILED;
		free(pStreamNew);
		pStreamNew = NULL;
		*pLenOut = 0;
		return NULL;
	}
	for ( k = 0; k < *pLenOut; k++ )
		pOutput[k] = '\0';
		
	//wprintf(L"ALLOCATI %d BYTES PER pOutput\n", *pLenOut * 2);
		
	num = 0;
	y = 0;
	for ( k = 0; k < lenNew; k += 4 )
	{	
		b1 = pStreamNew[k];
		b2 = pStreamNew[k + 1];
		b3 = pStreamNew[k + 2];
		b4 = pStreamNew[k + 3];
			
		numTemp = b1;
		numTemp <<= 24;
		num |= numTemp;
	
		numTemp = b2;
		numTemp <<= 16;
		num |= numTemp;

		numTemp = b3;
		numTemp <<= 8;
		num |= numTemp;

		numTemp = b4;
		//numTemp <<= 0;
		num |= numTemp;
			
		//wprintf(L"\nnum = %d\n", num);
		
		c4 = pOutput[y + 4] = (num % 85) + 33;
		num /= 85;
		
		c3 = pOutput[y + 3] = (num % 85) + 33;
		num /= 85;
		
		c2 = pOutput[y + 2] = (num % 85) + 33;
		num /= 85;
		
		c1 = pOutput[y + 1] = (num % 85) + 33;
		num /= 85;
		
		c0 = pOutput[y] = (num % 85) + 33;
		num /= 85;
			
		//if ( c0 != 0 || c1 != 0 || c2 != 0 || c3 != 0 || c4 != 0 )
		if ( (c0 != 33 || c1 != 33 || c2 != 33 || c3 != 33 || c4 != 33) || ( numPaddingInput > 0 && k > (lenNew - 5)) )
		{
			y += 5;
			countBlocks++;
			
			//wprintf(L"\tBLOCCO[%u] = <%c%c%c%c%c> y = %d\n", countBlocks, c0, c1, c2, c3, c4, y);
		}
		else
		{
			pOutput[y] = 'z';
			y++;
			countBlocks++;
			
			//wprintf(L"\tBLOCCO[%u] = <%c> y = %d\n", countBlocks, pOutput[y - 1], y);
		}
		
					
		num = 0;
	}
	
	//wprintf(L"\n\ncountZ = %d\n\n", countZ);
	
	//*pLenOut = y - (lenNew - len);
	*pLenOut = y - numPaddingInput;
	//if ( countZ )
	//{
	//	*pLenOut -= (countZ * 3);
	//	*pLenOut += (countZ) + 1;
	//}
	
	pOutput[*pLenOut] = '~';
	(*pLenOut)++;
	pOutput[*pLenOut] = '>';
	(*pLenOut)++;	
	
	//uscita:
	
	free(pStreamNew);
	pStreamNew = NULL;
	
	return pOutput;
}

unsigned char * ascii85Decode(unsigned char * pStream, size_t len, size_t *pLenOut, uint32_t *pErrorCode)
{
	size_t k;
	size_t y;
	size_t x;
	size_t w;
	
	unsigned char * pStreamNew = NULL;
	size_t lenNew;
	uint32_t numPaddingInput;
	
	unsigned char * pOutput = NULL;
	
	uint32_t num;
	//uint32_t numTemp;
	
	//uint32_t howmanyBlocks;
	//uint32_t numPadding;
	
	unsigned char cBlock[5];
	
	unsigned char c1, c2, c3, c4;
	
	uint32_t nMod;

	uint32_t c1Mask = 4278190080;
	uint32_t c2Mask = 16711680;
	uint32_t c3Mask = 65280;
	uint32_t c4Mask = 255;
	
	*pLenOut = 0;
	
	*pErrorCode = ASCII85_ERROR_NONE;
	
	while ( ' ' == pStream[len - 1] || '\n' == pStream[len - 1] || 'r' == pStream[len - 1] || '\t' == pStream[len - 1] || '\f' == pStream[len - 1] )
	{
		len--;
	}
	
	if ( pStream[len - 1] == '>' && pStream[len - 2] == '~' )
	{
		len -= 2;
	}
	else
	{
		//wprintf(L"ERRORE ascii85Decode: ATTESI '~>' TROVATI '%c%c'\n\n",  pStream[len - 2],  pStream[len - 1]);
		*pErrorCode = ASCII85_ERROR_EOD_NOT_FOUND;
		return NULL;
	}
		
	
	nMod = len % 5;
	if ( nMod > 0 )
		numPaddingInput = 5 - nMod;
	else
		numPaddingInput = 0;
	

		
	lenNew = len + numPaddingInput;
	pStreamNew = (unsigned char*)malloc(lenNew * 5 * sizeof(unsigned char));
	if ( NULL == pStreamNew )
	{
		*pErrorCode = ASCII85_ERROR_MEMALLOC_FAILED;
		*pLenOut = 0;
		return NULL;
	}
		
	lenNew = len;
	w = 0;
	for ( k = 0; k < len; k++ )
	{
		switch ( pStream[k] )
		{
			case 'z':
				if ( w % 5 != 0 )
				{
					//wprintf(L"ERRORE 0 ascii85Decode: k = %d; w = %d; carattere non valido -> 'z'", k, w);
					*pErrorCode = ASCII85_ERROR_UNEXPECTED_CHAR;
					free(pOutput);
					pOutput = NULL;
					*pLenOut = 0;
					goto uscita;
				}
				
				pStreamNew[w] = '!';
				w++;
				pStreamNew[w] = '!';
				w++;
				pStreamNew[w] = '!';
				w++;
				pStreamNew[w] = '!';
				w++;
				pStreamNew[w] = '!';
				w++;
				
				lenNew += 4;
				
				break;
			case ' ':
			case '\n':
			case '\r':
			case '\t':
			case '\f':
				lenNew--;
				break;
			default:
				pStreamNew[w] = pStream[k];
				w++;
				break;
		}
	}
	len = lenNew;
		
	nMod = len % 5;
	if ( nMod > 0 )
		numPaddingInput = 5 - nMod;
	else
		numPaddingInput = 0;
		
	lenNew = len + numPaddingInput;

	for ( ; w < lenNew; w++ )
		pStreamNew[w] = 'u';
			
			
			
	//wprintf(L"\n\nascii85Decode pStreamNew -> ENCODED STREAM(len = %d)\n", lenNew);
	//for ( k = 0; k < lenNew; k++ )
	//{
	//	if ( '\0' != pStreamNew[k] )
	//		wprintf(L"%c", pStreamNew[k]);
	//	else
	//		wprintf(L"\nEHI!!! ->\\0\n");
	//}
	//wprintf(L"\nFINE ascii85Decode pStreamNew ENCODED STREAM.\n\n");
	
	//wprintf(L"\n\nlen = %d; numPaddingInput = %d; lenNew = %d\n\n", len, numPaddingInput, lenNew);
	//wprintf(L"\n\nlenNew * 4 * sizeof(unsigned char) = %d\n\n", lenNew * 4 * sizeof(unsigned char));
		
	pOutput = (unsigned char *)malloc(lenNew * 4 * sizeof(unsigned char));
	if ( NULL == pOutput )
	{
		*pErrorCode = ASCII85_ERROR_MEMALLOC_FAILED;
		*pLenOut = 0;
		return NULL;
	}
	for ( k = 0 ; k < lenNew * 4 * sizeof(unsigned char); k++ )
		pOutput[k] = '\0';
		
		
	x = 0;
	y = 0;
	for ( k = 0; k <= lenNew - 5; k += 5 )
	{
		if ( pStreamNew[k] < 33 || pStreamNew[k] > 117 )
		{			
			//if ( pStreamNew[k] != '\0' )
			//	wprintf(L"ERRORE 1 ascii85Decode: k = %d; carattere non valido -> %d -> '%c'", k, pStreamNew[k], pStreamNew[k]);
			//else
			//	wprintf(L"ERRORE 1 ascii85Decode: k = %d; carattere non valido -> 0 -> '\\0'", k);
				
			*pErrorCode = ASCII85_ERROR_UNEXPECTED_CHAR;
			free(pOutput);
			pOutput = NULL;
			*pLenOut = 0;
			goto uscita;
		}
		cBlock[y] = pStreamNew[k] - 33;
		y++;
			
		if ( pStreamNew[k] < 33 || pStreamNew[k] > 117 )
		{
			//if ( pStreamNew[k] != '\0' )
			//	wprintf(L"ERRORE 2 ascii85Decode: k = %d; carattere non valido -> %d -> '%c'", k, pStreamNew[k], pStreamNew[k]);
			//else
			//	wprintf(L"ERRORE 2 ascii85Decode: k = %d; carattere non valido -> 0 -> '\\0'", k);
			
			*pErrorCode = ASCII85_ERROR_UNEXPECTED_CHAR;
			free(pOutput);
			pOutput = NULL;
			*pLenOut = 0;
			goto uscita;
		}
		cBlock[y] = pStreamNew[k + 1] - 33;
		y++;

		if ( pStreamNew[k] < 33 || pStreamNew[k] > 117 )
		{
			//if ( pStreamNew[k] != '\0' )
			//	wprintf(L"ERRORE 3 ascii85Decode: k = %d; carattere non valido -> %d -> '%c'", k, pStreamNew[k], pStreamNew[k]);
			//else
			//	wprintf(L"ERRORE 3 ascii85Decode: k = %d; carattere non valido -> 0 -> '\\0'", k);
				
			*pErrorCode = ASCII85_ERROR_UNEXPECTED_CHAR;
			free(pOutput);
			pOutput = NULL;
			*pLenOut = 0;
			goto uscita;
		}
		cBlock[y] = pStreamNew[k + 2] - 33;
		y++;

		if ( pStreamNew[k] < 33 || pStreamNew[k] > 117 )
		{
			//if ( pStreamNew[k] != '\0' )
			//	wprintf(L"ERRORE 4 ascii85Decode: k = %d; carattere non valido -> %d -> '%c'", k, pStreamNew[k], pStreamNew[k]);
			//else
			//	wprintf(L"ERRORE 4 ascii85Decode: k = %d; carattere non valido -> 0 -> '\\0'", k);
				
			*pErrorCode = ASCII85_ERROR_UNEXPECTED_CHAR;
			free(pOutput);
			pOutput = NULL;
			*pLenOut = 0;
			goto uscita;
		}
		cBlock[y] = pStreamNew[k + 3] - 33;
		y++;

		if ( pStreamNew[k] < 33 || pStreamNew[k] > 117 )
		{
			//if ( pStreamNew[k] != '\0' )
			//	wprintf(L"ERRORE 5 ascii85Decode: k = %d; carattere non valido -> %d -> '%c'", k, pStreamNew[k], pStreamNew[k]);
			//else
			//	wprintf(L"ERRORE 5 ascii85Decode: k = %d; carattere non valido -> 0 -> '\\0'", k);
				
			*pErrorCode = ASCII85_ERROR_UNEXPECTED_CHAR;
			free(pOutput);
			pOutput = NULL;
			*pLenOut = 0;
			goto uscita;
		}
		cBlock[y] = pStreamNew[k + 4] - 33;
		//y++;
		
		num = cBlock[0] * 52200625 + cBlock[1] * 614125 + cBlock[2] * 7225 + cBlock[3] * 85 + cBlock[4];
		
		//wprintf(L"num = %u -> (%c)%u * 85^4 + (%c)%u * 85^3 + (%c)%u * 85^2 + (%c)%u * 85 + (%c)%u\n", num, cBlock[0] + 33, cBlock[0], cBlock[1] + 33, cBlock[1], cBlock[2] + 33, cBlock[2], cBlock[3] + 33, cBlock[3], cBlock[4] + 33, cBlock[4]);
		
		c1 = (unsigned char)((num & c1Mask) >> 24);
		c2 = (unsigned char)((num & c2Mask) >> 16);
		c3 = (unsigned char)((num & c3Mask) >> 8);
		c4 = (unsigned char)((num & c4Mask));
		
		pOutput[x] = c1;
		x++;
		pOutput[x] = c2;
		x++;
		pOutput[x] = c3;
		x++;
		pOutput[x] = c4;
		x++;
		
		//wprintf(L"\tc1(%c) = %u; c2(%c) = %u; c3(%c) = %u; c4(%c) = %u; x = %d; k = %d\n", c1, c1, c2, c2, c3, c3, c4, c4, x, k);
							
		y = 0;

		if ( k >= lenNew - 5 )
			break;
	}
		
	*pLenOut = x - numPaddingInput;
	//*pLenOut = x;
	
	uscita:
	
	if ( NULL != pStreamNew )
	{
		free(pStreamNew);
		pStreamNew = NULL;
	}
	
	return pOutput;
}

// ****************************************************************** LZW **************************************************************

int IsLittleEndian()
{
	uint16_t w = 0x0001;
	uint8_t *b = (uint8_t*)&w;

	return (b[0] ? 1 : 0);
}

int InitEncodeParams(EncodeParams *p, unsigned char *pOut, size_t sizeOutputStream,  uint32_t BitsNum)
{
	if ( BitsNum < 9 )
		BitsNum = 9;
	else if ( BitsNum > 12 )
		BitsNum = 12;
		
	p->pOut = pOut;
	p->sizeOutputStream = sizeOutputStream;
	
	p->currBitsNum = BitsNum;
	p->offsetOutputStream = 0;
	p->BitsWritten = 0;
		
	return 1;
}

int InitDecodeParams(DecodeParams *p, unsigned char *pIn, size_t lenInput, uint32_t BitsNum)
{
	if ( BitsNum < 9 )
		BitsNum = 9;
	else if ( BitsNum > 12 )
		BitsNum = 12;
		
	p->pIn = pIn;
	p->lenInput = lenInput;
			
	p->currBitsNum = BitsNum;
	p->offsetInputStream = 0;
	p->BitsWritten = 0;
		
	return 1;
}

int WriteBits(uint16_t code, EncodeParams *p)
{
	int x, k;
	uint8_t *pCodeOut;
	
	int32_t bitsToCurrByte;
	int32_t bitsToNextByte;
		
	uint32_t offOut = p->offsetOutputStream;
	
	bitsToCurrByte = 8 - p->BitsWritten;
	bitsToNextByte = p->currBitsNum - bitsToCurrByte;
	
	if ( bitsToCurrByte <= 0 )
	{
		if ( bitsToCurrByte < 0 )
			offOut++;
		//printf("\n\nATTENZIONE: bitsToCurrByte = %d, bitsToNextByte = %d, p->BitsWritten = %u\n", bitsToCurrByte, bitsToNextByte, p->BitsWritten);
		bitsToCurrByte = 8;
		bitsToNextByte = p->currBitsNum - 8;
		p->BitsWritten = 0;
		//printf("\tCAMBIO IN : bitsToCurrByte = %d, bitsToNextByte = %d, p->BitsWritten = %u\n\n", bitsToCurrByte, bitsToNextByte, p->BitsWritten);
	}
			
	if ( p->offsetOutputStream >= p->sizeOutputStream )
	{
		uint8_t *pTemp = NULL;	
		p->sizeOutputStream *= 2;
								
		pTemp = (uint8_t*)realloc(p->pOut, p->sizeOutputStream);
		if ( NULL == pTemp )
		{
			free(p->pOut);
			p->pOut = NULL;
			return 0;
		}
		p->pOut = pTemp;
	}
		
	pCodeOut = (uint8_t*)(p->pOut + offOut);
	if ( 0 == p->offsetOutputStream )
		*pCodeOut = 0;
		
	k = p->currBitsNum - 1;
	x = 8 - p->BitsWritten - 1;
	
	//printf("WRITE BITS FOR CODE %u -> ", code);
	//PrintInBinary16(code);
	//printf("\n\tbitsToCurrByte = %d\n", bitsToCurrByte);
	//printf("\tbitsToNextByte = %d\n", bitsToNextByte);
	//printf("\tp->BitsWritten = %u\n", p->BitsWritten);
	
	while ( k >= 0 )
	{
		if ( BIT_CHECK(code, k) )
		{
			BIT_SET(*pCodeOut, x);
			
			//printf("p->pOut[%u][%d] = 1\n", offOut, x);
		}
		//else
		//{
		//	printf("p->pOut[%u][%d] = 0\n", offOut, x);
		//}
		
		x--;
		k--;
		
		if ( x < 0 )
		{
			x = 7;
						
			//if ( *pCodeOut > 0xF )
			//	printf("\tCODE OUT = %X -> ", *pCodeOut);
			//else
			//	printf("\tCODE OUT = 0%X -> ", *pCodeOut);
			//PrintInBinary16(*pCodeOut);
			//printf("\n");
					
			offOut++;
			pCodeOut = (uint8_t*)(p->pOut + offOut);
			*pCodeOut = 0;
		}
	}	
			
	if ( M_EOD == code )
	{
		//printf("FINAL CODE OUT -> %u -> ", *pCodeOut);
		//PrintInBinary16(*pCodeOut);
		//printf("\n");
		
		if ( 0 == *pCodeOut )
			offOut--;
		//if ( 0 == *pCodeOut )
		//	p->offsetOutputStream--;
	}

	p->offsetOutputStream = offOut;
	
	while ( bitsToNextByte > 8 )
	{
		bitsToNextByte -= 8;
	}
	p->BitsWritten = bitsToNextByte;	
		
	//printf("\n");
	
	return 1;	
}

int ReadBits(uint16_t *code, DecodeParams *p)
{
	int x, k;
	size_t y;
	
	uint8_t *pCodeIn;
	
	int32_t bitsToCurrByte;
	int32_t bitsToNextByte;
		
	uint16_t nShift;
	
	uint32_t offIn = p->offsetInputStream;
	
	*code = 0;
	
	bitsToCurrByte = 8 - p->BitsWritten;
	bitsToNextByte = p->currBitsNum - bitsToCurrByte;	
	
	if ( bitsToCurrByte <= 0 )
	{
		//printf("\nATTENZIONE: bitsToCurrByte = %d, bitsToNextByte = %d, p->BitsWritten = %u\n", bitsToCurrByte, bitsToNextByte, p->BitsWritten);
		if ( bitsToCurrByte < 0 )
			offIn++;
		//offIn++;
		bitsToCurrByte = 8;
		bitsToNextByte = p->currBitsNum - 8;
		p->BitsWritten = 0;	
		//printf("\tCAMBIO IN : bitsToCurrByte = %d, bitsToNextByte = %d, p->BitsWritten = %u\n", bitsToCurrByte, bitsToNextByte, p->BitsWritten);
	}
			
	nShift = 16 - p->currBitsNum;
	
	pCodeIn = (uint8_t*)(p->pIn + offIn);
	
	//printf("\nREAD BITS FOR CODE %u -> ", *pCodeIn);
	//PrintInBinary16(*pCodeIn);
	//printf("\n\tbitsToCurrByte = %d\n", bitsToCurrByte);
	//printf("\tbitsToNextByte = %d\n", bitsToNextByte);
	//printf("\tp->BitsWritten = %u\n", p->BitsWritten);
				
	k = bitsToCurrByte - 1;
	x = 15;
	for ( y = 0; y < p->currBitsNum; y++ )
	{
		if ( BIT_CHECK(*pCodeIn, k) )
		{
			//printf("*pCodeIn[%u][%d] = 1\n", offIn, k);
			BIT_SET(*code, x);
		}
		//else
		//{
		//	printf("*pCodeIn[%u][%d] = 0\n", offIn, k);
		//}
		
		x--;
		k--;
		
		if ( k < 0 )
		{
			k = 7;									
			offIn++;
			pCodeIn = (uint8_t*)(p->pIn + offIn);
		}
	}
	
	//if ( *code > 0xF )
	//	printf("\tCODE = %u(%X) -> ", *code, *code);
	//else
	//	printf("\tCODE = %u(0%X) -> ", *code, *code);
	//PrintInBinary16(*code);
	//printf("\n");
	//printf("\t*code = *code >> nShift(%d)\n", nShift);
	
	*code = *code >> nShift;	
	
	//if ( *code > 0xF )
	//	printf("\tCODE OUT = %u(%X) -> ", *code, *code);
	//else
	//	printf("\tCODE OUT = %u(0%X) -> ", *code, *code);
	//PrintInBinary16(*code);
	//printf("\n\n");

	p->offsetInputStream = offIn;
	
	while ( bitsToNextByte > 8 )
	{
		bitsToNextByte -= 8;
	}
	p->BitsWritten = bitsToNextByte;
		
	return 1;
}

unsigned char * lzwEncode(unsigned char * pStream, size_t len, size_t *pLenOut, uint32_t *pErrorCode)
{	
	unsigned char *pRet = NULL;
	
	EncodeParams myEncodeParams;
	
	GenHashTable_t myHT;
		
	size_t k;
	
	uint8_t strTemp[2];
	uint16_t kTemp;
	
	uint8_t byte;
	uint8_t *pBuffer = NULL;
	uint32_t offsetBuffer = 0;
	uint32_t sizeBuffer;
	
	uint16_t codeOut = M_NEW;
	
	uint16_t codeHT;
	uint16_t *pCodeHT = &codeHT;
	uint32_t dataSize;
	
	*pLenOut = 0;
	
	*pErrorCode = LZW_ERROR_NONE;
				
	sizeBuffer = len * 2;
	pRet = (unsigned char*)malloc(sizeBuffer);
	if ( NULL == pRet )
	{
		*pErrorCode = LZW_ERROR_MEMALLOC_FAILED;
		return NULL;
	}
	for ( k = 0; k < sizeBuffer; k++ )
		pRet[k] = 0;
	
	pBuffer = (unsigned char*)malloc(sizeBuffer);
	if ( NULL == pBuffer )
	{
		*pErrorCode = LZW_ERROR_MEMALLOC_FAILED;
		genhtFree(&myHT);
		free(pRet);
		pRet = NULL;
		return NULL;
	}
	for ( k = 0; k < sizeBuffer; k++ )
		pBuffer[k] = '\0';

	InitEncodeParams(&myEncodeParams, pRet, sizeBuffer, 9);
	
	if ( !genhtInit(&myHT, 0, GenStringHashFuncNew, GenStringCompareFuncNew) )
	{
		*pErrorCode = LZW_ERROR_TABLE_INIT_FAILED;
		free(pRet);
		pRet = NULL;
		return NULL;
	}
		
	for ( kTemp = 0; kTemp < 256; kTemp++ )
	{
		strTemp[0] = kTemp;
		strTemp[1] = '\0';
		
		if ( genhtInsert(&myHT, strTemp, 2, (void*)&kTemp, sizeof(uint16_t)) < 0 )
		{
			//printf("ERRORE lzwEncode 0: genhtInsert failed for string <%s>\n", strTemp);
			*pErrorCode = LZW_ERROR_TABLE_INSERT_FAILED;
			free(pRet);
			pRet = NULL;
			goto uscita;
		}
	}
	
	//printf("\nOUTPUT CLEAR CODE = %u\n", M_CLR);
		
	if ( !WriteBits(M_CLR, &myEncodeParams) )
	{
		//printf("ERRORE lzwEncode: WriteBits failed.\n");
		*pErrorCode = LZW_ERROR_WRITEBITS_FAILED;
		free(pRet);
		pRet = NULL;
		goto uscita;
	}
	
	offsetBuffer = 0;
	//pBuffer[0] = '\0';
	for ( k = 0; k < len; k++ )
	{
		byte = pStream[k];
		
		if ( offsetBuffer >= sizeBuffer )
		{
				uint8_t *pBufferTemp = NULL;	
				sizeBuffer *= 2;
				
				pBufferTemp = (uint8_t*)realloc(pBuffer, sizeBuffer);
				if ( NULL == pBufferTemp )
				{
					//printf("ERRORE lzwEncode : realloc failed.\n");
					*pErrorCode = LZW_ERROR_MEMALLOC_FAILED;
					free(pRet);
					pRet = NULL;
					goto uscita;
				}
				pBuffer = pBufferTemp;
				
				pBufferTemp = (uint8_t*)realloc(pRet, sizeBuffer);
				if ( NULL == pBufferTemp )
				{
					//printf("ERRORE lzwEncode : realloc failed.\n");
					*pErrorCode = LZW_ERROR_MEMALLOC_FAILED;
					free(pRet);
					pRet = NULL;
					goto uscita;
				}
				pRet = pBufferTemp;
		}
		
		pBuffer[offsetBuffer] = byte;		
		offsetBuffer++;
		pBuffer[offsetBuffer] = '\0';
		
		//printf("offsetBuffer = %u\n", offsetBuffer);
		//for ( uint32_t i = 0; i < offsetBuffer; i++ )
		//	printf("\tpBuffer[%u] = %u\n", i, byte);	
		//printf("CERCO LA STRINGA <");
		//for ( uint32_t i = 0; i < offsetBuffer; i++ )
		//{
		//	printf("%u ", pBuffer[i]);
		//}
		//printf(">(%s) NELLA HASHTABLE.\n", (char*)pBuffer);
			 
		if ( genhtFind(&myHT, pBuffer, offsetBuffer + 1, (void**)&pCodeHT, &dataSize) >= 0 )
		{
			//printf("\n\tTROVATA! concateno: BUFFER = ");
			//for ( uint32_t i = 0; i < offsetBuffer; i++ )
			//{
			//	printf("%u ", pBuffer[i]);
			//}
			//printf("(%s)\n", (char*)pBuffer);
		}
		else
		{
			//printf("\tSTRINGA NON TROVATA!!!\n");
			
			pBuffer[offsetBuffer - 1] = '\0';
			if ( genhtFind(&myHT, pBuffer, offsetBuffer, (void**)&pCodeHT, &dataSize) < 0 )
			{
				//printf("\nERRORE lzwEncode 1: stringa <%s> NON TROVATA NELLA HASHTABLE.\n", pBuffer);
				*pErrorCode = LZW_ERROR_STRING_NOT_FOUND_IN_TABLE;
				free(pRet);
				pRet = NULL;
				goto uscita;
			}

			//printf("\nOUTPUT CODE = %u FOR STRING ", codeHT);
			//for ( uint32_t i = 0; i < offsetBuffer - 1; i++ )
			//{
			//	printf("%u ", pBuffer[i]);
			//}
			//printf("(%s)\n", (char*)pBuffer);
			
			if ( !WriteBits(codeHT, &myEncodeParams) )
			{
				//printf("ERRORE lzwEncode: WriteBits failed.\n");
				*pErrorCode = LZW_ERROR_WRITEBITS_FAILED;
				free(pRet);
				pRet = NULL;
				goto uscita;
			}
						
			pBuffer[offsetBuffer - 1] = byte;
			pBuffer[offsetBuffer] = '\0';
			
			//printf("\nBUFFER 1 = ");
			//for ( uint32_t i = 0; i < offsetBuffer; i++ )
			//{
			//	printf("%u ", pBuffer[i]);
			//}
			//printf("(%s)\n", (char*)pBuffer);
						
			if ( genhtInsert(&myHT, pBuffer, offsetBuffer + 1, (void*)&codeOut, sizeof(uint16_t)) < 0 )
			//if ( genhtInsert(&myHT, pBuffer, offsetBuffer, (void*)&codeOut, sizeof(uint16_t)) < 0 )
			{
				//printf("\nERRORE lzwEncode 2: genhtInsert failed for string <%s>\n", pBuffer);
				*pErrorCode = LZW_ERROR_TABLE_INSERT_FAILED;
				free(pRet);
				pRet = NULL;
				goto uscita;
			}
			
			//printf("\nINSERITO CODE = %u NELLA HASHTABLE PER LA STRINGA ", codeOut);
			//for ( uint32_t i = 0; i < offsetBuffer; i++ )
			//{
			//	printf("%u ", pBuffer[i]);
			//}
			//printf("(%s)\n", (char*)pBuffer);
				
			codeOut++;
			switch ( codeOut )
			{
				case 512:
					//printf("\nSWITCH TO 10 BITS\n");
					myEncodeParams.currBitsNum = 10;
					break;
				case 1024:
					//printf("\nSWITCH TO 11 BITS\n");
					myEncodeParams.currBitsNum = 11;
					break;
				case 2048:
					//printf("\nSWITCH TO 12 BITS\n");
					myEncodeParams.currBitsNum = 12;
					break;
				default:
					break;						
			}
								
			offsetBuffer = 0;
			pBuffer[0] = byte;
			pBuffer[1] = '\0';
			offsetBuffer++;
			
			//printf("\nBUFFER 2 = ");
			//for ( uint32_t i = 0; i < offsetBuffer; i++ )
			//{
			//	printf("%u ", pBuffer[i]);
			//}
			//printf("(%s)\n", (char*)pBuffer);
				
			if ( codeOut == 4096 )
			{
				//printf("\nATTENZIONEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE: codeOut = %u\n", codeOut);
				//printf("\nOUTPUT CLEAR CODE = %u\n", M_CLR);
				
				if ( !WriteBits(M_CLR, &myEncodeParams) )
				{
					//printf("ERRORE lzwEncode: WriteBits failed.\n");
					*pErrorCode = LZW_ERROR_WRITEBITS_FAILED;
					free(pRet);
					pRet = NULL;
					goto uscita;
				}
				
				genhtFree(&myHT);
					
				if ( !genhtInit(&myHT, 0, GenStringHashFuncNew, GenStringCompareFuncNew) )
				{
					*pErrorCode = LZW_ERROR_TABLE_INIT_FAILED;
					free(pRet);
					pRet = NULL;
					return NULL;
				}
					
				for ( kTemp = 0; kTemp < 256; kTemp++ )
				{
					strTemp[0] = kTemp;
					strTemp[1] = '\0';
					
					if ( genhtInsert(&myHT, strTemp, 2, (void*)&kTemp, sizeof(uint16_t)) < 0 )
					{
						//printf("\nERRORE lzwEncode 0 bis: genhtInsert failed for string <%s>\n", strTemp);
						*pErrorCode = LZW_ERROR_TABLE_INSERT_FAILED;
						free(pRet);
						pRet = NULL;
						goto uscita;
					}					
				}
				
				//printf("\tSWITCH TO 9 BITS\n");
				myEncodeParams.currBitsNum = 9;
									
				codeOut = M_NEW;
			}
		}	
	}
	
	if ( genhtFind(&myHT, pBuffer, offsetBuffer + 1, (void**)&pCodeHT, &dataSize) < 0 )
	{
		//printf("\nOUTPUT CODE = %u FOR STRING ", codeOut);
		//for ( uint32_t i = 0; i < offsetBuffer; i++ )
		//{
		//	printf("%u ", pBuffer[i]);
		//}
		//printf("(%s) NON TROVATA offsetBuffer = %u\n", (char*)pBuffer, offsetBuffer);

		if ( !WriteBits(codeOut, &myEncodeParams) )
		{
			//printf("ERRORE lzwEncode: WriteBits failed.\n");
			*pErrorCode = LZW_ERROR_WRITEBITS_FAILED;
			free(pRet);
			pRet = NULL;
			goto uscita;
		}
	}
	else
	{		
		//printf("\nOUTPUT CODE = %u FOR STRING ", codeHT);
		//for ( uint32_t i = 0; i < offsetBuffer; i++ )
		//{
		//	printf("%u ", pBuffer[i]);
		//}
		//printf("(%s) TROVATA offsetBuffer = %u\n", (char*)pBuffer, offsetBuffer);
		
		if ( !WriteBits(codeHT, &myEncodeParams) )
		{
			//printf("ERRORE lzwEncode: WriteBits failed.\n");
			*pErrorCode = LZW_ERROR_WRITEBITS_FAILED;
			free(pRet);
			pRet = NULL;
			goto uscita;
		}
	}
	
	//printf("\nOUTPUT END OF DATA CODE = %u\n", M_EOD);
	
	if ( !WriteBits(M_EOD, &myEncodeParams) )
	{
		//printf("ERRORE lzwEncode: WriteBits failed.\n");
		*pErrorCode = LZW_ERROR_WRITEBITS_FAILED;
		free(pRet);
		pRet = NULL;
		goto uscita;
	}

	
	//*pLenOut = myEncodeParams.offsetOutputStream;
	*pLenOut = myEncodeParams.offsetOutputStream + 1;
	//pRet[*pLenOut - 1] = myEncodeParams.cLastCode;

	uscita:	
	
	if ( NULL != pBuffer )
	{
		free(pBuffer);
		pBuffer = NULL;
	}
	
	genhtFree(&myHT);
	
	return pRet;		
}

unsigned char * lzwDecode(unsigned char * pStream, size_t len, size_t *pLenOut, uint32_t *pErrorCode)
{	
	uint32_t k;
	
	uint8_t strTemp[2];
	uint16_t kTemp;
	
	int indexHT;
		
	unsigned char *pRet = NULL;
	
	DecodeParams myDecodeParams;
	
	GenHashTable_t myHT;
	
	uint16_t code;
	uint16_t oldCode;
	
	uint16_t codeOut = M_NEW;
	
	uint8_t *pBuffer = NULL;
	uint32_t sizeBuffer;
	uint32_t offsetBuffer = 0;
	uint32_t offsetOutput = 0;
		
	uint8_t *pData = NULL;
	uint32_t dataSize;
	
	uint8_t *pData2 = NULL;
	uint32_t dataSize2;
	
	*pLenOut = 0;
	
	*pErrorCode = LZW_ERROR_NONE;
	
	sizeBuffer = len * 2;
				
	pRet = (unsigned char*)malloc(sizeBuffer);
	if ( NULL == pRet )
	{
		*pErrorCode = LZW_ERROR_MEMALLOC_FAILED;
		return NULL;
	}
	for ( k = 0; k < sizeBuffer; k++ )
		pRet[k] = 0;
	
	pBuffer = (unsigned char*)malloc(sizeBuffer);
	if ( NULL == pBuffer )
	{
		*pErrorCode = LZW_ERROR_MEMALLOC_FAILED;
		genhtFree(&myHT);
		free(pRet);
		pRet = NULL;
		return NULL;
	}
	for ( k = 0; k < sizeBuffer; k++ )
		pBuffer[k] = '\0';

	InitDecodeParams(&myDecodeParams, pStream, len, 9);
	
	if ( !genhtInit(&myHT, 0, GenUint16HashFunc, GenUint16CompareFunc) )
	{
		//printf("\nERRORE lzwDecode: genhtInit failed.\n");
		*pErrorCode = LZW_ERROR_TABLE_INIT_FAILED;
		free(pRet);
		pRet = NULL;
		return NULL;
	}
		
	if ( !ReadBits(&code, &myDecodeParams) )
	{
		//printf("ERRORE lzwDecode: ReadBits failed.\n");
		*pErrorCode = LZW_ERROR_READBITS_FAILED;
		free(pRet);
		pRet = NULL;
		goto uscita;
	}
	
	//printf("\n\nlzwDecode: FIRST CODE READ = %u\n\n", code);
	
	while ( code != M_EOD && myDecodeParams.offsetInputStream < len )
	{
		if ( code == M_CLR )
		{
			genhtFree(&myHT);
					
			if ( !genhtInit(&myHT, 0, GenUint16HashFunc, GenUint16CompareFunc) )
			{
				//printf("\nERRORE lzwDecode: genhtInit failed.\n");
				*pErrorCode = LZW_ERROR_TABLE_INIT_FAILED;
				free(pRet);
				pRet = NULL;
				return NULL;
			}
			
			for ( kTemp = 0; kTemp < 256; kTemp++ )
			{
				strTemp[0] = kTemp;
				strTemp[1] = '\0';
				
				if ( (indexHT = genhtInsert(&myHT, (void*)&kTemp, sizeof(uint16_t), strTemp, 2)) < 0 )
				{
					//printf("\nERRORE lzwDecode 0 bis: genhtInsert failed for string <%s>\n", strTemp);
					*pErrorCode = LZW_ERROR_TABLE_INSERT_FAILED;
					free(pRet);
					pRet = NULL;
					goto uscita;
				}				
			}
			
			//printf("\nSWITCH TO 9 BITS\n");
			myDecodeParams.currBitsNum = 9;
			
			codeOut = M_NEW;
			
			
			if ( !ReadBits(&code, &myDecodeParams) )
			{
				//printf("ERRORE lzwDecode: ReadBits failed.\n");
				*pErrorCode = LZW_ERROR_READBITS_FAILED;
				free(pRet);
				pRet = NULL;
				goto uscita;
			}
			
			if ( code == M_EOD )
				break;
				
			// WriteString(StringFromCode(Code));
			if ( NULL != pData )
			{
				free(pData);
				pData = NULL;
			}
			dataSize = 1;
			
			if ( genhtFind(&myHT, &code, sizeof(uint16_t), (void**)&pData, &dataSize) < 0 )
			{
				*pErrorCode = LZW_ERROR_UNEXPECTED_CODE;
				//free(pRet);
				//pRet = NULL;
				pRet[offsetOutput] = '\0';
				*pLenOut = offsetOutput;
				goto uscita;
			}
			
			if ( (dataSize - 1) >= (sizeBuffer - offsetOutput) )
			{
				uint8_t *pBufferTemp = NULL;	
				sizeBuffer *= 2;
				
				pBufferTemp = (uint8_t*)realloc(pBuffer, sizeBuffer);
				if ( NULL == pBufferTemp )
				{
					//printf("\nERRORE lzwDecode: realloc failed.\n");
					*pErrorCode = LZW_ERROR_MEMALLOC_FAILED;
					free(pRet);
					pRet = NULL;
					goto uscita;
				}
				pBuffer = pBufferTemp;
				
				pBufferTemp = (uint8_t*)realloc(pRet, sizeBuffer);
				if ( NULL == pBufferTemp )
				{
					//printf("\nERRORE lzwDecode: realloc failed.\n");
					*pErrorCode = LZW_ERROR_MEMALLOC_FAILED;
					free(pRet);
					pRet = NULL;
					goto uscita;
				}
				pRet = pBufferTemp;
			}
			
			//pData[dataSize - 1] = '\0';
			//printf("WRITE STRING 1 <");
			//for ( size_t i = 0; i < dataSize - 1; i++ )
			//{
			//	printf("%u ", pData[i]);				
			//}
			//printf(">(%s) FROM CODE %u\n", (char*)pData, code);
			
			memcpy(pRet + offsetOutput, pData, dataSize - 1);
			offsetOutput += (dataSize - 1);
							
			oldCode = code;
		}
		else
		{
			//if ( IsInTable(code) )
			if ( NULL != pData )
			{
				free(pData);
				pData = NULL;
			}
			dataSize = 1;
			if ( genhtFind(&myHT, &code, sizeof(uint16_t), (void**)&pData, &dataSize) >= 0 )
			{
				//WriteString(StringFromCode(Code));
				//if ( (dataSize - 1) >= (sizeBuffer - offsetOutput) )
				//if ( (dataSize - 1) >= (sizeBuffer - offsetBuffer) )
				if ( ((dataSize - 1) >= (sizeBuffer - offsetBuffer)) || ((dataSize - 1) >= (sizeBuffer - offsetOutput)) )
				{
					uint8_t *pBufferTemp = NULL;	
					sizeBuffer *= 2;
				
					pBufferTemp = (uint8_t*)realloc(pBuffer, sizeBuffer);
					if ( NULL == pBufferTemp )
					{
						//printf("\nERRORE lzwDecode: realloc failed.\n");
						*pErrorCode = LZW_ERROR_MEMALLOC_FAILED;
						free(pRet);
						pRet = NULL;
						goto uscita;
					}
					pBuffer = pBufferTemp;
				
					pBufferTemp = (uint8_t*)realloc(pRet, sizeBuffer);
					if ( NULL == pBufferTemp )
					{
						//printf("\nERRORE lzwDecode: realloc failed.\n");
						*pErrorCode = LZW_ERROR_MEMALLOC_FAILED;
						free(pRet);
						pRet = NULL;
						goto uscita;
					}
					pRet = pBufferTemp;
				}
				
				//pData[dataSize - 1] = '\0';
				//printf("WRITE STRING 2 <");
				//for ( size_t i = 0; i < dataSize - 1; i++ )
				//{
				//	printf("%u ", pData[i]);				
				//}
				//printf(">(%s) FROM CODE %u\n", (char*)pData, code);
				
				memcpy(pRet + offsetOutput, pData, dataSize - 1);
				offsetOutput += (dataSize - 1);
												
				//AddStringToTable(StringFromCode(OldCode) + FirstChar(StringFromCode(Code)));
				if ( NULL != pData )
				{
					free(pData);
					pData = NULL;
				}
				dataSize = 1;
				
				if ( !genhtFind(&myHT, &oldCode, sizeof(uint16_t), (void**)&pData, &dataSize) )
				{
					*pErrorCode = LZW_ERROR_UNEXPECTED_CODE;
					//free(pRet);
					//pRet = NULL;
					pRet[offsetOutput] = '\0';
					*pLenOut = offsetOutput;
					goto uscita;
				}
				
				memcpy(pBuffer + offsetBuffer, pData, dataSize - 1);
				offsetBuffer += (dataSize - 1);
				
				if ( NULL != pData2 )
				{
					free(pData2);
					pData2 = NULL;
				}
				dataSize2 = 1;
				
				if ( !genhtFind(&myHT, &code, sizeof(uint16_t), (void**)&pData2, &dataSize2) )
				{
					*pErrorCode = LZW_ERROR_UNEXPECTED_CODE;
					//free(pRet);
					//pRet = NULL;
					pRet[offsetOutput] = '\0';
					*pLenOut = offsetOutput;
					goto uscita;
				}
				
				pBuffer[offsetBuffer] = pData2[0];
				offsetBuffer++;
				
				//pBuffer[dataSize] = '\0';
				//printf("\nAddStringToTable <");
				//for ( size_t i = 0; i < dataSize; i++ )
				//{
				//	if ( pBuffer[i] != '\0' )
				//		printf("%u ", pBuffer[i]);
				//	else
				//		printf("\\0 ");
				//}
				//printf(">(%s) with code %u\n\n", (char*)pBuffer, codeOut);
				
				if ( !genhtInsert(&myHT, &codeOut, sizeof(uint16_t), pBuffer, dataSize + 1) )
				{
					//printf("\nERRORE lzwDecode 0 bis: genhtInsert failed for string <%s>\n", strTemp);
					*pErrorCode = LZW_ERROR_TABLE_INSERT_FAILED;
					free(pRet);
					pRet = NULL;
					goto uscita;
				}
				
				pBuffer[0] = '\0';
				offsetBuffer = 0;
				
				codeOut++;
				switch ( codeOut )
				{
					case 511:
						//printf("\nSWITCH TO 10 BITS\n");
						myDecodeParams.currBitsNum = 10;
						break;
					case 1023:
						//printf("\nSWITCH TO 11 BITS\n");
						myDecodeParams.currBitsNum = 11;
						break;
					case 2047:
						//printf("\nSWITCH TO 12 BITS\n");
						myDecodeParams.currBitsNum = 12;
						break;
					default:
						break;
				}
				
				oldCode = code;
			}
			else
			{				
				//OutString = StringFromCode(OldCode) + FirstChar(StringFromCode(OldCode));
				if ( NULL != pData )
				{
					free(pData);
					pData = NULL;
				}
				dataSize = 1;
				indexHT = genhtFind(&myHT, &oldCode, sizeof(uint16_t), (void**)&pData, &dataSize);
								
				if ( indexHT < 0 )
				{
					//printf("\n\nlzwDecode ERROR: genhtFing failed for oldCode = %u\n\n", oldCode);
					//for ( uint32_t u = 0; u < offsetOutput - 1; u++ )
					//{
					//	printf("%c", pRet[u]);
					//}
					
					*pErrorCode = LZW_ERROR_UNEXPECTED_CODE;
					//free(pRet);
					//pRet = NULL;
					pRet[offsetOutput] = '\0';
					*pLenOut = offsetOutput;
					goto uscita;
				}
				//else
				//{
				
				//if ( (dataSize - 1) >= (sizeBuffer - offsetOutput) )
				//if ( (dataSize - 1) >= (sizeBuffer - offsetBuffer) )
				if ( ((dataSize - 1) >= (sizeBuffer - offsetBuffer)) || ((dataSize - 1) >= (sizeBuffer - offsetOutput)) )
				{
					uint8_t *pBufferTemp = NULL;	
					sizeBuffer *= 2;
					
					//printf("REALLOC! NEW sizeBuffer = %u\n", sizeBuffer);
				
					pBufferTemp = (uint8_t*)realloc(pBuffer, sizeBuffer);
					if ( NULL == pBufferTemp )
					{
						//printf("\nERRORE lzwDecode: realloc failed.\n");
						free(pRet);
						pRet = NULL;
						goto uscita;
					}
					pBuffer = pBufferTemp;
				
					pBufferTemp = (uint8_t*)realloc(pRet, sizeBuffer);
					if ( NULL == pBufferTemp )
					{
						//printf("\nERRORE lzwDecode: realloc failed.\n");
						free(pRet);
						pRet = NULL;
						goto uscita;
					}
					pRet = pBufferTemp;
				}
				
				//printf("offsetBuffer = %u; datasize - 1 = %u; sizeBuffer = %u\n", offsetBuffer, dataSize - 1, sizeBuffer);
				//printf("\tpData = ");
				//for ( uint32_t i = 0; i < dataSize - 1; i++ )
				//{
				//	if ( pData[i] > 0xF )
				//		printf("<%X>", pData[i]);
				//	else
				//		printf("<0%X>", pData[i]);
				//}
				//printf("(%s)\n", (char*)pData);
				
				memcpy(pBuffer + offsetBuffer, pData, dataSize - 1);
				offsetBuffer += (dataSize - 1);
				
				//printf("\toffsetBuffer = %u; sizeBuffer = %u\n", offsetBuffer, sizeBuffer);
				//if ( pData[0] > 0xF )
				//	printf("\tpData[0] = <%X>(%s)\n", pData[0], (char*)pData);
				//else
				//	printf("\tpData[0] = <0%X>(%s)\n", pData[0], (char*)pData);
				
				pBuffer[offsetBuffer] = pData[0];
				offsetBuffer++;
				
				//WriteString(OutString);
				
				//printf("WRITE OUT STRING <");
				//for ( size_t i = 0; i < dataSize; i++ )
				//{
				//	if ( pBuffer[i] != '\0' )
				//		printf("%u ", pBuffer[i]);
				//	else
				//		printf("\\0 ");
				//}
				//printf(">(%s)\n", (char*)pBuffer);
				
				memcpy(pRet + offsetOutput, pBuffer, dataSize);
				offsetOutput += dataSize;
				
				//AddStringToTable(OutString);
				
				//pBuffer[dataSize] = '\0';
				//printf("\nAddStringToTable <");
				//for ( size_t i = 0; i < dataSize; i++ )
				//{
				//	if ( pBuffer[i] != '\0' )
				//		printf("%u ", pBuffer[i]);
				//	else
				//		printf("\\0 ");
				//}
				//printf(">(%s) with code %u\n\n", (char*)pBuffer, codeOut);
				
				if ( !genhtInsert(&myHT, &codeOut, sizeof(uint16_t), pBuffer, dataSize + 1) )
				{
					//printf("\nERRORE lzwDecode 0 bis: genhtInsert failed for string <%s>\n", strTemp);
					*pErrorCode = LZW_ERROR_TABLE_INSERT_FAILED;
					free(pRet);
					pRet = NULL;
					goto uscita;
				}
				
				pBuffer[0] = '\0';
				offsetBuffer = 0;
				
				codeOut++;
				switch ( codeOut )
				{
					case 511:
						//printf("\nSWITCH TO 10 BITS\n");
						myDecodeParams.currBitsNum = 10;
						break;
					case 1023:
						//printf("\nSWITCH TO 11 BITS\n");
						myDecodeParams.currBitsNum = 11;
						break;
					case 2047:
						//printf("\nSWITCH TO 12 BITS\n");
						myDecodeParams.currBitsNum = 12;
						break;
					default:
						break;
				}
				
				oldCode = code;
				//}			
			}
		}
		
		if ( !ReadBits(&code, &myDecodeParams) )
		{
			//printf("ERRORE lzwDecode: ReadBits failed.\n");
			*pErrorCode = LZW_ERROR_READBITS_FAILED;
			free(pRet);
			pRet = NULL;
			goto uscita;
		}
	}
	
	if ( code != M_EOD )
	{
		//printf("\nERRORE lzwDecode: code(%u) != M_EOD(%u).\n", code, M_EOD);
		*pErrorCode = LZW_ERROR_EOD_NOT_FOUND;
		free(pRet);
		pRet = NULL;
		return NULL;
	}
		
	*pLenOut = offsetOutput;
	
	uscita:
	
	if ( NULL != pBuffer )
	{
		free(pBuffer);
		pBuffer = NULL;
	}
	
	if ( NULL != pData )
	{
		free(pData);
		pData = NULL;
	}
	
	if ( NULL != pData2 )
	{
		free(pData2);
		pData2 = NULL;
	}
	
	genhtFree(&myHT);
	
	return pRet;		
}
