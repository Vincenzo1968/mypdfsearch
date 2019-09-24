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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/*
#include "mypdfsearch.h"
#include "scanner.h"
*/
#include "parser.h"
#include "mydecode.h"
#include "myGenHashTable.h"
#include "myInitPredefCMapHT.h"


#define HI_SURROGATE_START   0xD800
#define LO_SURROGATE_START   0xDC00

#define HI_SURROGATE_END     0xDBFF
#define LO_SURROGATE_END     0xDFFF

#define LEAD_OFFSET          (0xD800 - (0x10000 >> 10))
#define SURROGATE_OFFSET     (0x10000 - (0xD800 << 10) - 0xDC00)


void getSurrogateLeadTrail(wchar_t codepoint, uint16_t *pLead, uint16_t *pTrail)
{
	*pLead = LEAD_OFFSET + (codepoint >> 10);
	*pTrail = LO_SURROGATE_START + (codepoint & 0x3FF);
}

uint32_t getSurrogateCodePoint(uint16_t lead, uint16_t trail)
{
	uint32_t codepoint = (lead << 10) + trail + SURROGATE_OFFSET;
	
	return codepoint;
}

//#define UNUSED(x) (void)(x)

/*
program : decl_list code_block;
decl_list : {decl};
decl : type var_list ';'
		| T_KW_ARRAY T_ID '[' T_INT_LITERAL ']' {'[' T_INT_LITERAL ']'} T_KW_OF type ';'
		;
type : T_KW_INT | T_KW_REAL | T_KW_BOOL | T_KW_STRING;
var_list : T_ID ['=' expr] {',' T_ID['=' expr]};
code_block : T_BEGIN stmt_list T_END;
stmt_list : {stmt};
stmt : T_KW_PRINT expr ';'
		| T_KW_READ expr ';'
		| T_KW_WHILE '(' expr ')' stmt
		| T_KW_DO stmt T_KW_WHILE '(' expr ')'
		| T_KW_IF '(' expr ')' stmt [T_KW_ELSE stmt]
		| T_BEGIN stmt_list T_END
		| T_ID {'[' expr ']'} '=' expr ';'
		;
expr  : expr1 {('&' | '|') expr1};
expr1 : expr2 {relop expr2};
expr2 : expr3 {addop expr3};
expr3 : expr4 {mulop expr4};
expr4 : T_INT_LITERAL
		| T_REAL_LITERAL
		| T_STRING_LITERAL
		| T_KW_TRUE
		| T_KW_FALSE
		| '(' expr ')'
		| '-' expr
		| '!' expr
		| T_ID {'[' expr ']'}
		;
*/

/*
obj        : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ objbody T_KW_ENDOBJ;
objbody    : objitem {objitem};
objitem    : T_KW_TRUE
           | T_KW_FALSE
           | T_KW_NULL
           | T_KW_STREAM
           | T_KW_ENDSTREAM
           | T_INT_LITERAL 
           | T_REAL_LITERAL
           | T_STRING_LITERAL
           | T_STRING_HEXADECIMAL
           | T_NAME
           | array
           | dictionary
           ;
array      : T_QOPAREN {objitem} T_QCPAREN;
dictionary : T_DICT_BEGIN {T_NAME objitem [ T_INT_LITERAL T_KW_R ]} T_DICT_END;
dictionary : T_DICT_BEGIN {T_NAME objitem} T_DICT_END;
*/

// -----------------------------------------------------------------------------------------------------------

/*
pagetree          : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ pagetreebody T_KW_ENDOBJ;
pagetreebody      : T_DICT_BEGIN pagetreeitems T_DICT_END;
pagetreeitems     : {T_NAME pagetreeobj};
pagetreeobj       : T_NAME
                  | T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
                  | T_STRING_LITERAL
                  | T_STRING_HEXADECIMAL
                  | T_QOPAREN pagetreearrayobjs T_QCPAREN
                  | T_DICT_BEGIN pagetreedictobjs T_DICT_END
                  ; 
pagetreearrayobjs : {T_INT_LITERAL T_INT_LITERAL T_KW_R};
pagetreedictobjs  : {T_NAME pagetreeobj};
*/

// -----------------------------------------------------------------------------------------------------------

/*
contentxobj         : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ contentxobjbody T_KW_ENDOBJ;
contentxobjbody     : T_DICT_BEGIN xobjstreamdictitems T_DICT_END T_KW_STREAM T_STRING T_KW_ENDSTREAM;
xobjstreamdictitems : T_NAME xobjcontentkeyvalue {T_NAME xobjcontentkeyvalue};
xobjcontentkeyvalue : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
                    | T_NAME
                    | T_STRING_LITERAL
                    | T_STRING_HEXADECIMAL
                    | TSTRING
                    | xobjcontentkeyarray
                    | xobjcontentkeydict
                    ;
xobjcontentkeyarray : T_QOPAREN { T_INT_LITERAL | T_REAL_LITERAL} T_QCPAREN;
xobjcontentkeydict  : T_DICT_BEGIN T_NAME xobjcontentkeyvalue { T_NAME xobjcontentkeyvalue } T_DICT_END;
*/

// -----------------------------------------------------------------------------------------------------------

/*
contentfontobj         : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ contentfontobjbody T_KW_ENDOBJ;
contentfontobjbody     : T_DICT_BEGIN fontobjstreamdictitems T_DICT_END;
fontobjstreamdictitems : T_NAME fontobjcontentkeyvalue {T_NAME fontobjcontentkeyvalue};
fontobjcontentkeyvalue : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
                       | T_NAME
                       | fontobjcontentkeyarray
                       | fontobjcontentkeydict
                       ;
fontobjcontentkeyarray : T_QOPAREN { T_INT_LITERAL | T_REAL_LITERAL} T_QCPAREN;
fontobjcontentkeydict  : T_DICT_BEGIN { T_NAME fontobjcontentkeyvalue } T_DICT_END;
*/

/*
encodingobj                   : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ encodingobjbody T_KW_ENDOBJ;
encodingobjbody               : T_NAME | T_DICT_BEGIN encodingobjdictitems T_DICT_END;
encodingobjdictitems          : {T_NAME encodingobjdictitemskeyvalues};
encodingobjdictitemskeyvalues : T_NAME
                              | encodingobjarray
                              ;
encodingobjarray              : T_QOPAREN { T_INT_LITERAL T_NAME {T_NAME} } T_QCPAREN;
*/

// -----------------------------------------------------------------------------------------------------------

/*
contentobj              : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ contentobjbody T_KW_ENDOBJ;
contentobjbody          : T_DICT_BEGIN streamdictitems T_DICT_END T_KW_STREAM T_STRING T_KW_ENDSTREAM;
streamdictitems         : T_NAME contentkeyvalue {T_NAME contentkeyvalue};
contentkeyvalue         : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
                        | T_NAME
                        | T_STRING_LITERAL
                        | T_STRING_HEXADECIMAL
                        | TSTRING
                        | contentkeyarray
                        | contentkeydict
                        ;
contentkeyarray         : T_QOPAREN ({TNAME} | {T_KW_NULL | contentkeydict}) T_QCPAREN;
contentkeydict          : T_DICT_BEGIN T_NAME contentkeyvalueinternal { T_NAME contentkeyvalueinternal } T_DICT_END;
contentkeyvalueinternal : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
                        | T_NAME
                        | T_STRING_LITERAL
                        | T_STRING_HEXADECIMAL
                        | TSTRING
                        | T_KW_TRUE
                        | T_KW_FALSE
                        | T_REAL_LITERAL
                        | contentkeyarrayinternal
                        | contentkeydict
                        ;
*/

// -----------------------------------------------------------------------------------------------------------

int myOnScopeTraverse(const void* key, uint32_t keySize, void* data, uint32_t dataSize, uint32_t bContentAlreadyProcessed)
{
	int* pData = NULL;
	
	pData = (int*)data;
	
	if ( NULL != pData )
		wprintf(L"\tmyOnScopeTraverse: Key -> '%s' keysize = %d <-> Data -> ObjectRef = '%d 0 R' dataSize = %d\n", (char*)key, keySize, *pData, dataSize);
	else
		wprintf(L"\tmyOnScopeTraverse: Key -> '%s' keysize = %d\n", (char*)key, keySize);
		
	if ( !bContentAlreadyProcessed )
	{
		wprintf(L"\tmyOnScopeTraverse: Content da processare\n");
	}
	else
	{
		wprintf(L"\tmyOnScopeTraverse: Content GIÀ PROCESSATO\n");
	}		
				
	return 1;
}

// -----------------------------------------------------------------------------------------------------------

void PrintToken(Token *pToken, char cCarattereIniziale, char cCarattereFinale, int bPrintACapo)
{
	if ( cCarattereIniziale != '\0' )
		wprintf(L"%c", cCarattereIniziale);
	
	switch ( pToken->Type )
	{
		case T_NAME:
			wprintf(L"T_NAME = '%s'", pToken->Value.vString);
			break;
		case T_STRING:
			wprintf(L"T_STRING = '%s'", pToken->Value.vString);
			break;
		case T_STRING_LITERAL:
			wprintf(L"T_STRING_LITERAL = '%s'", pToken->Value.vString);
			break;
		case T_STRING_HEXADECIMAL:
			wprintf(L"T_STRING_HEXADECIMAL = '%s'", pToken->Value.vString);
			break;
		case T_INT_LITERAL:
			wprintf(L"T_INT_LITERAL = %d", pToken->Value.vInt);			
			break;
		case T_REAL_LITERAL:
			wprintf(L"T_REAL_LITERAL = %f", pToken->Value.vDouble);
			break;
		case T_KW_NULL:
			wprintf(L"T_KW_NULL");
			break;
		case T_KW_TRUE:
			wprintf(L"T_KW_TRUE");
			break;
		case T_KW_FALSE:
			wprintf(L"T_KW_FALSE");
			break;			
		case T_ERROR:
			wprintf(L"T_ERROR");
			break;
		case T_UNKNOWN:
			wprintf(L"T_UNKNOWN");
			break;
		case T_EOF:
			wprintf(L"T_EOF");
			break;
		case T_OPAREN:		// '('
			wprintf(L"T_OPAREN = '('");
			break;
		case T_CPAREN:		// ')'
			wprintf(L"T_CPAREN = ')'");
			break;
		case T_QOPAREN:		// '['
			wprintf(L"T_QOPAREN ");
			break;
		case T_QCPAREN:		// ']'
			wprintf(L"T_QCPAREN ");
			break;
		case T_DICT_BEGIN:   // "<<"
			wprintf(L"T_DICT_BEGIN = '<<'");
			break;
		case T_DICT_END:     // ">>"
			wprintf(L"T_DICT_END = '>>'");
			break;
		case T_KW_OBJ:
			wprintf(L"T_KW_OBJ");
			break;
		case T_KW_ENDOBJ:
			wprintf(L"T_KW_ENDOBJ");
			break;
		case T_KW_STREAM:
			wprintf(L"T_KW_STREAM");
			break;
		case T_KW_ENDSTREAM:
			wprintf(L"T_KW_ENDSTREAM");
			break;
		case T_KW_R:
			wprintf(L"T_KW_R");
			break;
		case T_CONTENT_LQUOTE:   // "<"
			wprintf(L"T_CONTENT_LQUOTE = '<'");
			break;
		case T_CONTENT_RQUOTE:   // ">"
			wprintf(L"T_CONTENT_RQUOTE = '>'");
			break;
		case T_CONTENT_Do_COMMAND:
			wprintf(L"T_CONTENT_Do_COMMAND = 'Do'");
			break;
		case T_CONTENT_KW_BT:
			wprintf(L"T_CONTENT_KW_BT = 'BT -> Begin Text'");
			break;
		case T_CONTENT_KW_ET:
			wprintf(L"T_CONTENT_KW_ET = 'ET -> End Text'");
			break;
		case T_CONTENT_OP_TD:
			wprintf(L"T_CONTENT_OP_TD = 'TD'");
			break;
		case T_CONTENT_OP_Td:
			wprintf(L"T_CONTENT_OP_Td = 'Td'");
			break;
		case T_CONTENT_OP_Tm:
			wprintf(L"T_CONTENT_OP_Tm = 'Tm'");
			break;
		case T_CONTENT_OP_TASTERISCO:
			wprintf(L"T_CONTENT_OP_TASTERISCO = 'T*'");
			break;
		case T_CONTENT_OP_TJ:
			wprintf(L"T_CONTENT_OP_TJ = 'TJ'");
			break;
		case T_CONTENT_OP_Tj:
			wprintf(L"T_CONTENT_OP_Tj = 'Tj'");
			break;
		case T_CONTENT_OP_SINGLEQUOTE:
			wprintf(L"T_CONTENT_OP_SINGLEQUOTE = '");
			break;
		case T_CONTENT_OP_DOUBLEQUOTE:
			wprintf(L"T_CONTENT_OP_DOUBLEQUOTE = '\"'");
			break;	
		case T_CONTENT_OP_Tf:
			wprintf(L"T_CONTENT_OP_Tf = 'Tf'");
			break;
		case T_CONTENT_OP_Tc:
			wprintf(L"T_CONTENT_OP_Tc = 'Tc'");
			break;	
		case T_CONTENT_OP_Tw:
			wprintf(L"T_CONTENT_OP_Tw = 'Tw'");
			break;	
		case T_CONTENT_OP_begincodespacerange:
			wprintf(L"T_CONTENT_OP_begincodespacerange = 'begincodespacerange'");
			break;
		case T_CONTENT_OP_endcodespacerange:
			wprintf(L"T_CONTENT_OP_endcodespacerange = 'endcodespacerange'");
			break;
		case T_CONTENT_OP_beginbfchar:
			wprintf(L"T_CONTENT_OP_beginbfchar = 'beginbfchar'");
			break;
		case T_CONTENT_OP_endbfchar:
			wprintf(L"T_CONTENT_OP_endbfchar = 'endbfchar'");
			break;
		case T_CONTENT_OP_beginbfrange:
			wprintf(L"T_CONTENT_OP_beginbfrange = 'beginbfrange'");
			break;
		case T_CONTENT_OP_endbfrange:
			wprintf(L"T_CONTENT_OP_endbfrange = 'endbfrange'");
			break;
		case T_CONTENT_OP_begincidchar:
			wprintf(L"T_CONTENT_OP_begincidchar = 'begincidchar'");
			break;
		case T_CONTENT_OP_endcidchar:
			wprintf(L"T_CONTENT_OP_endcidchar = 'endcidchar'");
			break;
		case T_CONTENT_OP_begincidrange:
			wprintf(L"T_CONTENT_OP_begincidrange = 'begincidrange'");
			break;
		case T_CONTENT_OP_endcidrange:
			wprintf(L"T_CONTENT_OP_endcidrange = 'endcidrange'");
			break;
		case T_CONTENT_OP_beginnotdefrange:
			wprintf(L"T_CONTENT_OP_beginnotdefrange = 'beginnotdefrange'");
			break;
		case T_CONTENT_OP_endnotdefrange:
			wprintf(L"T_CONTENT_OP_endnotdefrange = 'endnotdefrange'");
			break;
		case T_LEFT_CURLY_BRACKET:
			wprintf(L"T_LEFT_CURLY_BRACKET = '{'");
			break;
		case T_RIGHT_CURLY_BRACKET:
			wprintf(L"T_RIGHT_CURLY_BRACKET = '}'");
			break;	
		case T_VOID_STRING:
			wprintf(L"T_VOID_STRING");
			break;
		default:
			wprintf(L"TOKEN n° -> %d", pToken->Type);
			break;
	}
	
	if ( cCarattereFinale != '\0' )
		wprintf(L"%c", cCarattereFinale);	
	
	if ( bPrintACapo )
		wprintf(L"\n");
}

// --------------------------------------------------------------------------------------------------------------------------------

int myGenOnTraverseForFreeData(const void* pKey, uint32_t keysize, void* pData, uint32_t dataSize)
{
	MyPredefinedCMapDef *pmyData = (MyPredefinedCMapDef*)pData;
	
	UNUSED(pKey);
	UNUSED(keysize);
	UNUSED(dataSize);
		
	if ( NULL != pmyData->pszDecodedStream )
	{
		free(pmyData->pszDecodedStream);
		pmyData->pszDecodedStream = NULL;
	}
	
	return 1;
}

int myPrintLastBlock(Params *pParams)
{
	int retValue = 1;
	
	int bytereads = 0;
	unsigned char szTemp[BLOCK_SIZE];

	if ( fseek(pParams->fp, 0, SEEK_END) )
	{
		snprintf(pParams->szError, 8192, "Errore ReadTrailer: fseek 1\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//fwprintf(pParams->fpErrors, L"Errore ReadTrailer: fseek 1\n");
		return 0;
	}

	pParams->dimFile = ftell(pParams->fp);
	if ( pParams->dimFile <= 0 )
	{
		return 0;
	}
	
	wprintf(L"ReadTrailer: dimFile = %d\n", pParams->dimFile);
	
	if ( pParams->dimFile >= BLOCK_SIZE )
	{
		if ( fseek(pParams->fp, -BLOCK_SIZE, SEEK_CUR) )
		{
			snprintf(pParams->szError, 8192, "Errore ReadTrailer: fseek 1\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore ReadTrailer: fseek 1\n");
			return 0;
		}
	}
	else
	{
		if ( fseek(pParams->fp, 0, SEEK_SET) )
		{
			snprintf(pParams->szError, 8192, "Errore ReadTrailer: fseek 2\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore ReadTrailer: fseek 2\n");
			return 0;
		}		
	}
	
	bytereads = fread(szTemp, 1, BLOCK_SIZE, pParams->fp);
	
	wprintf(L"\n\nReadTrailer: bytes read for szTemp -> %d\n", bytereads);
	wprintf(L"LAST BLOCK OF FILE -> <");
	for ( int i = 0; i < bytereads; i++ )
	{
		if ( (szTemp[i] >= 32 || szTemp[i] == '\n') && (szTemp[i] != 127 && szTemp[i] != 129 && szTemp[i] != 141 && szTemp[i] != 143 && szTemp[i] != 144 && szTemp[i] != 157) )
		{
			wprintf(L"%c", szTemp[i]);
		}
		else
		{
			wprintf(L"@<%u>@", szTemp[i]);
		}
	}
	wprintf(L">\n\n");
	
	return retValue;
}

int getObjsOffsets(Params *pParams, char *pszFileName)
{
	int retValue = 1;
	FILE *fp = NULL;
	size_t bytesRead = 0;
	uint32_t curPos = 0;
	unsigned char myBlock[BLOCK_SIZE];
	unsigned char myPrevBlock[BLOCK_SIZE];
	
	//unsigned char myTempBlock[BLOCK_SIZE];
	//size_t tempBytesRead;
	//unsigned char cTemp;
	
	unsigned char c;
	unsigned char lexeme[128];
	
	unsigned char NumberLexeme[128];
	int idxNumberLexeme = 0;
	int lenNumberLexeme = 0;
	
	char key[256];
	uint32_t keyLength;
	GenHashTable_t myHT;
	PdfIndirectObject myHT_Data;
	uint32_t myHT_DataSize;
	int retKeyFind;
	
	int bUpdateStreamOffset = 1;
	
	uint32_t k = 0;
	
	uint32_t Number = 0;
	uint32_t Generation = 0;
	uint32_t Offset = 0;
	uint32_t ObjOffset = 0;
	
	uint32_t countObjs = 0;
	uint32_t maxObjNum = 0;
	uint32_t maxObjGenNum = 0;
	
	myHT_Data.Type = OBJ_TYPE_RESERVED;
	myHT_Data.Number = 0;
	myHT_Data.Generation = 0;
	myHT_Data.Offset = 0;
	myHT_Data.StreamOffset = 0;
	myHT_Data.StreamLength = 0;
	myHT_Data.numObjParent = -1;
	myHT_Data.genObjParent = 0;
		
	NumberLexeme[0] = '\0';
		
	int bStreamState = 0;
	
	PreParseStates state = S_PP0;
	
	fp = fopen(pszFileName, "rb");
	if ( fp == NULL )
	{
		snprintf(pParams->szError, 8192, "ERROR getObjsOffsets: fopen failed for file '%s'.\n", pszFileName);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERROR getObjsOffsets: fopen failed for file '%s'.\n", pszFileName);
		//fwprintf(pParams->fpErrors, L"ERROR getObjsOffsets: fopen failed for file '%s'.\n\n", pszFileName);
		retValue = 0;
		goto uscita;
	}
	
	if ( !genhtInit(&myHT, GENHT_SIZE, GenStringHashFunc, GenStringCompareFunc) )
	{
		snprintf(pParams->szError, 8192, "ERROR getObjsOffsets: genhtInit failed for file '%s'.\n", pszFileName);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERROR getObjsOffsets: genhtInit failed.\n");
		//fwprintf(pParams->fpErrors, L"ERROR getObjsOffsets: genhtInit failed.\n\n");
		retValue = 0;
		goto uscita;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_getObjsOffsets_FN)
	wprintf(L"\n");
	#endif
	
	for ( int i = 0; i < BLOCK_SIZE; i++ )
		myBlock[i] = myPrevBlock[i] = '\0';
		
	while ( (bytesRead = fread(myBlock, 1, BLOCK_SIZE, fp)) )
	{
		curPos = 0;
		
		while ( curPos < bytesRead )
		{
			c = myBlock[curPos];
			
			if ( k >= 128 )
			{
				k = 0;
				state = S_PP0;
			}
			
			switch ( state )
			{
				case S_PP0:
					k = 0;
					
					if ( !bStreamState && ('%' == c) )
					{
						curPos++;
						Offset++;
						if ( curPos >= bytesRead )
						{
							bytesRead = fread(myBlock, 1, BLOCK_SIZE, fp);
							if ( bytesRead <= 0 )
								goto calcoli;
							curPos = 0;
						}
						c = myBlock[curPos];
						while ( '\n' != c  && '\r' != c )
						{
							curPos++;
							Offset++;
							if ( curPos >= bytesRead )
							{
								bytesRead = fread(myBlock, 1, BLOCK_SIZE, fp);
								if ( bytesRead <= 0 )
									goto calcoli;
								curPos = 0;
							}
							c = myBlock[curPos];
						}
						if ( '\r' == c )
						{
							curPos++;
							Offset++;
							if ( curPos >= bytesRead )
							{
								bytesRead = fread(myBlock, 1, BLOCK_SIZE, fp);
								if ( bytesRead <= 0 )
									goto calcoli;
								curPos = 0;
							}
							c = myBlock[curPos];
							
							if ( c >= '1' && c <= '9' )
							{
								idxNumberLexeme = 0;
								lexeme[k++] = c;
								NumberLexeme[idxNumberLexeme++] = c;
								
								ObjOffset = Offset;
								state = S_PP1;
							}
						}
					}
					else if ( !bStreamState && (c >= '1' && c <= '9') )
					{
						idxNumberLexeme = 0;
						lexeme[k++] = c;
						NumberLexeme[idxNumberLexeme++] = c;
						
						ObjOffset = Offset;
						state = S_PP1;
					}
					else if ( !bStreamState && 's' == c )
					{
						state = S_PP7;
					}
					else if ( bStreamState && 'e' == c )
					{
						state = S_PP13;
					}
					break;
				case S_PP1:
					if ( c >= '0' && c <= '9' )
					{
						lexeme[k++] = c;
						NumberLexeme[idxNumberLexeme++] = c;
					}
					else if ( IsDelimiterChar(c) )
					{
						lexeme[k] = '\0';
						k = 0;
						
						NumberLexeme[idxNumberLexeme] = '\0';
						lenNumberLexeme = idxNumberLexeme;
						idxNumberLexeme = 0;
						
						Number = atoi((char*)lexeme);
						state = S_PP2;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP2:
					if ( c >= '0' && c <= '9' )
					{
						lexeme[k++] = c;
					}
					else if ( k > 0 && IsDelimiterChar(c) )
					{
						lexeme[k] = '\0';
						k = 0;
												
						Generation = atoi((char*)lexeme);
						state = S_PP3;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP3:
					if ( 'o' == c )
					{
						state = S_PP4;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP4:
					if ( 'b' == c )
					{
						state = S_PP5;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP5:
					if ( 'j' == c )
					{
						state = S_PP6;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP6:
					if ( IsDelimiterChar(c) )
					{
						int y;
						
						myHT_Data.Number = Number;
						myHT_Data.Generation = Generation;
						
						countObjs++;
						
						if ( maxObjNum < Number )
							maxObjNum = Number;
						if ( maxObjGenNum < Generation )
							maxObjGenNum = Generation;
							
						keyLength = 0;
						key[keyLength++] = 'b';
						key[keyLength++] = 'k';
						for ( y = 0; y < lenNumberLexeme; y++ )
							key[keyLength++] = NumberLexeme[y];
						key[keyLength++] = 'e';
						key[keyLength++] = 'k';
						key[keyLength++] = '\0';   // INCREMENTIAMO keyLength ANCHE QUI, PERCHÉ MEMORIZZIAMO ANCHE IL CARATTERE NULL, TERMINATORE DELLA STRINGA, SULLA HASHTABLE.
												
						retKeyFind = genhtFind(&myHT, key, keyLength, &myHT_Data, &myHT_DataSize);
						if ( retKeyFind >= 0 )
						{
							if ( myHT_Data.Generation < Generation )
							{
								myHT_Data.Generation = Generation;
								
								myHT_Data.Type         = OBJ_TYPE_UNKNOWN; // Per il momento
								myHT_Data.StreamOffset = 0;
								myHT_Data.StreamLength = 0;
								myHT_Data.numObjParent = -1;
								myHT_Data.genObjParent = 0;
							
								if ( genhtUpdateData(&myHT, key, keyLength, &myHT_Data, sizeof(myHT_Data)) < 0 )
								{
									snprintf(pParams->szError, 8192, "ERROR getObjsOffsets: OBJECT(Number: %d, Generation: %d) genhtUpdateData failed for key '%s'.\n", myHT_Data.Number, myHT_Data.Generation, key);
									myShowErrorMessage(pParams, pParams->szError, 1);
									//wprintf(L"ERROR getObjsOffsets: OBJECT(Number: %lu, Generation: %lu) genhtUpdateData failed for key '%d'.\n", key, myHT_Data.Number, myHT_Data.Generation);
									//fwprintf(pParams->fpErrors, L"ERROR getObjsOffsets: OBJECT(Number: %lu, Generation: %lu) genhtUpdateData failed for key '%d'.\n", key, myHT_Data.Number, myHT_Data.Generation);
									retValue = 0;
									goto uscita;
								}
								
								bUpdateStreamOffset = 1;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_getObjsOffsets_FN)
								wprintf(L"OBJECT(Number: %lu, Generation: %lu) FOUND AT OFFSET %lu; UPDATE IT INTO HASHTABLE(key -> '%s')\n", myHT_Data.Number, myHT_Data.Generation, myHT_Data.Offset, key);
								#endif
							}
							else
							{
								bUpdateStreamOffset = 0;
							}
						}
						else
						{
							myHT_Data.Offset = ObjOffset;
							
							myHT_Data.Type         = OBJ_TYPE_UNKNOWN; // Per il momento
							myHT_Data.StreamOffset = 0;
							myHT_Data.StreamLength = 0;
							myHT_Data.numObjParent = -1;
							myHT_Data.genObjParent = 0;
			
							if ( genhtInsert(&myHT, key, keyLength, &myHT_Data, sizeof(myHT_Data)) < 0 )
							{
								snprintf(pParams->szError, 8192, "ERROR getObjsOffsets: OBJECT(Number: %d, Generation: %d) genhtInsert failed for key '%s'.\n\n", myHT_Data.Number, myHT_Data.Generation, key);
								myShowErrorMessage(pParams, pParams->szError, 1);
								//wprintf(L"ERROR getObjsOffsets: OBJECT(Number: %lu, Generation: %lu) genhtInsert failed for key '%s'.\n", myHT_Data.Number, myHT_Data.Generation, key);
								//fwprintf(pParams->fpErrors, L"ERROR getObjsOffsets: OBJECT(Number: %lu, Generation: %lu) genhtInsert failed for key '%s'.\n\n", myHT_Data.Number, myHT_Data.Generation, key);
								retValue = 0;
								goto uscita;
							}
														
							bUpdateStreamOffset = 1;
							
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_getObjsOffsets_FN)
							wprintf(L"OBJECT(Number: %lu, Generation: %lu) FOUND AT OFFSET %lu; INSERT IT INTO HASHTABLE(key -> '%s')\n", myHT_Data.Number, myHT_Data.Generation, myHT_Data.Offset, key);
							#endif
						}
					}
					state = S_PP0;
					break;
				case S_PP7:
					if ( 't' == c )
					{
						state = S_PP8;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP8:
					if ( 'r' == c )
					{
						state = S_PP9;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP9:
					if ( 'e' == c )
					{
						state = S_PP10;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP10:
					if ( 'a' == c )
					{
						state = S_PP11;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP11:
					if ( 'm' == c )
					{
						state = S_PP12;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP12:
					if ( IsDelimiterChar(c) )
					{
						if ( !bStreamState ) // stream
						{
							myHT_Data.StreamOffset = Offset + 1;
							bStreamState = 1;
							
							if ( '\r' == c )
							{
								curPos++;
								Offset++;
								if ( curPos >= bytesRead )
								{
									bytesRead = fread(myBlock, 1, BLOCK_SIZE, fp);
									if ( bytesRead <= 0 )
										goto uscita;
									curPos = 0;
								}
								c = myBlock[curPos];
								
								if ( '\n' == c )
								{
									myHT_Data.StreamOffset++;
								}
							}
							/*
							else if ( '\n' == c )
							{
								curPos++;
								Offset++;
								if ( curPos >= bytesRead )
								{
									bytesRead = fread(myBlock, 1, BLOCK_SIZE, fp);
									if ( bytesRead <= 0 )
										goto uscita;
									curPos = 0;
								}
								c = myBlock[curPos];
								
								myHT_Data.StreamOffset++;
							}
							*/
						}
						else // endstream
						{
							int nTemp;
							
							myHT_Data.StreamLength = (Offset - myHT_Data.StreamOffset) - 9;
							bStreamState = 0;
														
							nTemp = curPos - (9 + 2);
							if ( nTemp >= 0 )
							{								
								char c1, c2;
								
								c1 = myBlock[nTemp];
								c2 = myBlock[nTemp + 1];
								if ( '\r' == c1 )
									myHT_Data.StreamLength -= 2;
								else if ( '\n' == c2 )
									myHT_Data.StreamLength--;									
							}
							else if ( -1 == nTemp )
							{
								char c1, c2;
								
								c1 = myPrevBlock[BLOCK_SIZE - 1];
								c2 = myBlock[0];
								if ( '\r' == c1 )
									myHT_Data.StreamLength -= 2;
								else if ( '\n' == c2 )
									myHT_Data.StreamLength--;
							}
							else
							{
								char c1, c2;
								
								c1 = myPrevBlock[BLOCK_SIZE + nTemp];
								nTemp++;
								c2 = myPrevBlock[BLOCK_SIZE + (nTemp + 1)];
								if ( '\r' == c1 )
									myHT_Data.StreamLength -= 2;
								else if ( '\n' == c2 )
									myHT_Data.StreamLength--;									
							}
														
							if ( bUpdateStreamOffset )
							{
								if ( genhtUpdateData(&myHT, key, keyLength, &myHT_Data, sizeof(myHT_Data)) < 0 )
								{
									snprintf(pParams->szError, 8192, "ERROR getObjsOffsets: OBJECT(Number: %d, Generation: %d) genhtUpdateData failed for key '%s'.\n", myHT_Data.Number, myHT_Data.Generation, key);
									myShowErrorMessage(pParams, pParams->szError, 1);
									//wprintf(L"ERROR getObjsOffsets: OBJECT(Number: %lu, Generation: %lu) genhtUpdateData failed for key '%d'.\n", myHT_Data.Number, myHT_Data.Generation, key);
									//fwprintf(pParams->fpErrors, L"ERROR getObjsOffsets: OBJECT(Number: %lu, Generation: %lu) genhtUpdateData failed for key '%d'.\n", myHT_Data.Number, myHT_Data.Generation, key);
									retValue = 0;
									goto uscita;
								}
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_getObjsOffsets_FN) || defined(MYDEBUG_PRINT_ON_getObjsOffsets2_FN)
								wprintf(L"\tENDSTREAM -> STREAM OF OBJECT[number %lu, generation %lu] FOUND AT OFFSET %lu; (stream length = %lu)\n", myHT_Data.Number, myHT_Data.Generation, myHT_Data.StreamOffset, myHT_Data.StreamLength);
								wprintf(L"\t\tKEY1[%s](length = %lu)\n",  key, keyLength);
								#endif
							}
						}						
					}
					state = S_PP0;
					break;
				case S_PP13:
					if ( 'n' == c )
					{
						state = S_PP14;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP14:
					if ( 'd' == c )
					{
						state = S_PP15;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP15:
					if ( 's' == c )
					{
						state = S_PP7;
					}
					else if ( 'o' == c )
					{
						state = S_PP16;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP16:
					if ( 'b' == c )
					{
						state = S_PP17;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP17:
					if ( 'j' == c )
					{
						state = S_PP18;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP18:
					if ( IsDelimiterChar(c) )
					{
						if ( bStreamState )
						{
							int nTemp;
							
							myHT_Data.StreamLength = (Offset - myHT_Data.StreamOffset) - 6;
							bStreamState = 0;
														
							nTemp = curPos - (6 + 2);
							if ( nTemp >= 0 )
							{
								char c1, c2;
								c1 = myBlock[nTemp];
								c2 = myBlock[nTemp + 1];
								if ( '\r' == c1 )
									myHT_Data.StreamLength -= 2;
								else if ( '\n' == c2 )
									myHT_Data.StreamLength--;
							}
							else if ( -1 == nTemp )
							{
								char c1, c2;
								c1 = myBlock[BLOCK_SIZE - 1];
								c2 = myBlock[0];
								if ( '\r' == c1 )
									myHT_Data.StreamLength -= 2;
								else if ( '\n' == c2 )
									myHT_Data.StreamLength--;
							}
							else
							{
								char c1, c2;
								c1 = myBlock[BLOCK_SIZE + nTemp];
								nTemp++;
								c2 = myBlock[BLOCK_SIZE + nTemp];
								if ( '\r' == c1 )
									myHT_Data.StreamLength -= 2;
								else if ( '\n' == c2 )
									myHT_Data.StreamLength--;
							}
							
							if ( bUpdateStreamOffset )
							{
								if ( genhtUpdateData(&myHT, key, keyLength, &myHT_Data, sizeof(myHT_Data)) < 0 )
								{
									snprintf(pParams->szError, 8192, "ERROR getObjsOffsets OBJECT(Number: %d, Generation: %d): genhtUpdateData failed for key '%s'.\n", myHT_Data.Number, myHT_Data.Generation, key);
									myShowErrorMessage(pParams, pParams->szError, 1);
									//wprintf(L"ERROR getObjsOffsets OBJECT(Number: %lu, Generation: %lu): genhtUpdateData failed for key '%d'.\n", myHT_Data.Number, myHT_Data.Generation, key);
									//fwprintf(pParams->fpErrors, L"ERROR getObjsOffsets OBJECT(Number: %lu, Generation: %lu): genhtUpdateData failed for key '%d'.\n", myHT_Data.Number, myHT_Data.Generation, key);
									retValue = 0;
									goto uscita;
								}
								
								snprintf(pParams->szError, 8192, "\nWARNING OBJECT(Number: %d, Generation: %d): STREAM NOT CLOSED BY 'endstream', but 'endobj' keyword.\n", myHT_Data.Number, myHT_Data.Generation);
								myShowErrorMessage(pParams, pParams->szError, 1);
								//fwprintf(pParams->fpErrors, L"\nWARNING OBJECT(Number: %lu, Generation: %lu): STREAM NOT CLOSED BY 'endstream', but 'endobj' keyword.\n", myHT_Data.Number, myHT_Data.Generation);
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_getObjsOffsets_FN) || defined(MYDEBUG_PRINT_ON_getObjsOffsets2_FN)
								wprintf(L"\nWARNING OBJECT(Number: %lu, Generation: %lu): STREAM NOT CLOSED BY 'endstream', but 'endobj' keyword.\n", myHT_Data.Number, myHT_Data.Generation);
								wprintf(L"\tENDOBJ -> STREAM OF OBJECT[number %lu, generation %lu] FOUND AT OFFSET %lu; (stream length = %lu)\n", myHT_Data.Number, myHT_Data.Generation, myHT_Data.StreamOffset, myHT_Data.StreamLength);
								wprintf(L"\t\tKEY1[%s](length = %lu)\n",  key, keyLength);
								#endif
							}
						}
					}
					state = S_PP0;
					break;
				default:
					break;
			}
			
			curPos++;
			Offset++;
		}
		
		memcpy(myPrevBlock, myBlock, bytesRead);	
	}
	
	calcoli:
		
	if ( maxObjNum > countObjs )
		pParams->nObjsTableSizeFromPrescanFile = maxObjNum + 1;
	else
		pParams->nObjsTableSizeFromPrescanFile = countObjs + 1;
		
	pParams->myObjsTable = (PdfObjsTableItem **)malloc(sizeof(PdfObjsTableItem*) * pParams->nObjsTableSizeFromPrescanFile);
	if ( !(pParams->myObjsTable) )
	{
		snprintf(pParams->szError, 8192, "ERROR genhtUpdateData: memoria insufficiente per la tabella degli oggetti.\n\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERROR genhtUpdateData: memoria insufficiente per la tabella degli oggetti.\n\n");
		//fwprintf(pParams->fpErrors, L"ERROR genhtUpdateData: memoria insufficiente per la tabella degli oggetti.\n\n");
		retValue = 0;
		goto uscita;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_getObjsOffsets_FN)
	wprintf(L"\n\n***** START OBJECTS TABLE *****\n");
	#endif
	
	for ( k = 0; k < pParams->nObjsTableSizeFromPrescanFile; k++ )
	{
		pParams->myObjsTable[k] = (PdfObjsTableItem *)malloc(sizeof(PdfObjsTableItem));
		if ( !(pParams->myObjsTable) )
		{
			snprintf(pParams->szError, 8192, "ERROR genhtUpdateData: memoria insufficiente per allocare l'oggetto numero %d sulla tabella degli oggetti.\n\n", k);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERROR genhtUpdateData: memoria insufficiente per allocare l'oggetto numero %d sulla tabella degli oggetti.\n\n", k);
			//fwprintf(pParams->fpErrors, L"ERROR genhtUpdateData: memoria insufficiente per allocare l'oggetto numero %d sulla tabella degli oggetti.\n\n", k);
			retValue = 0;
			goto uscita;
		}
		
		sprintf(key, "bk%uek", k);
		retKeyFind = genhtFind(&myHT, key, strnlen(key, 256), &myHT_Data, &myHT_DataSize);
		if ( retKeyFind >= 0 )
		{
			pParams->myObjsTable[k]->Obj.Type         = OBJ_TYPE_UNKNOWN; // Per il momento
			pParams->myObjsTable[k]->Obj.Number       = myHT_Data.Number;
			pParams->myObjsTable[k]->Obj.Generation   = myHT_Data.Generation;
			pParams->myObjsTable[k]->Obj.Offset       = myHT_Data.Offset;
			pParams->myObjsTable[k]->Obj.StreamOffset = myHT_Data.StreamOffset;
			pParams->myObjsTable[k]->Obj.StreamLength = myHT_Data.StreamLength;
			pParams->myObjsTable[k]->Obj.pszDecodedStream = NULL;
			pParams->myObjsTable[k]->Obj.numObjParent = -1;
			pParams->myObjsTable[k]->Obj.genObjParent = 0;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_getObjsOffsets_FN)
			wprintf(L"\tINSERT(1) INTO OBJSTABLE[%d]: objNum = %lu, objGen = %lu, objOffset = %lu, objStreamOffset = %lu, objStreamLength = %lu\n",
					k,
			        pParams->myObjsTable[k]->Obj.Number,
			        pParams->myObjsTable[k]->Obj.Generation,
			        pParams->myObjsTable[k]->Obj.Offset,
			        pParams->myObjsTable[k]->Obj.StreamOffset,
			        pParams->myObjsTable[k]->Obj.StreamLength);
			#endif
		}
		else
		{
			pParams->myObjsTable[k]->Obj.Type         = OBJ_TYPE_UNKNOWN;
			pParams->myObjsTable[k]->Obj.Number       = 0;
			pParams->myObjsTable[k]->Obj.Generation   = 0;
			pParams->myObjsTable[k]->Obj.Offset       = 0;
			pParams->myObjsTable[k]->Obj.StreamOffset = 0;
			pParams->myObjsTable[k]->Obj.StreamLength = 0;
			pParams->myObjsTable[k]->Obj.pszDecodedStream = NULL;
			pParams->myObjsTable[k]->Obj.numObjParent = -1;
			pParams->myObjsTable[k]->Obj.genObjParent = 0;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_getObjsOffsets_FN)
			wprintf(L"\tINSERT(2) INTO OBJSTABLE[%d]: objNum = %lu, objGen = %lu, objOffset = %lu, objStreamOffset = %lu, objStreamLength = %lu\n",
					k,
			        pParams->myObjsTable[k]->Obj.Number,
			        pParams->myObjsTable[k]->Obj.Generation,
			        pParams->myObjsTable[k]->Obj.Offset,
			        pParams->myObjsTable[k]->Obj.StreamOffset,
			        pParams->myObjsTable[k]->Obj.StreamLength);
			#endif
			        
		}
		myobjreflist_Init(&(pParams->myObjsTable[k]->myXObjRefList));
		myobjreflist_Init(&(pParams->myObjsTable[k]->myFontsRefList));
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_getObjsOffsets_FN)
	wprintf(L"\n***** END OBJECTS TABLE *****\n");	
	wprintf(L"\n\tFOUND %lu OBJECTS.\n\t\tMAXIMUM OBJ NUMBER IS: %lu.\n\t\tMAXIMUM OBJ GENERATION NUMBER IS: %lu\n", countObjs, maxObjNum, maxObjGenNum);
	wprintf(L"\n\n");
	#endif	
		
uscita:

	if ( NULL != fp )
	{
		fclose(fp);
		fp = NULL;
	}
	
	genhtFree(&myHT);
	
	return retValue;
}

int Parse(Params *pParams, FilesList* myFilesList)
{
	int retValue = 1;
	FilesList* n;
	uint32_t x;
	int len;
	
	#if defined(_WIN64) || defined(_WIN32)
	int lung = 0;
	#endif
			
	int retReadTrailer;
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = 0;
	
	pParams->fpErrors = NULL;
				
	pParams->myBlock = NULL;
	pParams->fp = NULL;
	
	pParams->myBlockToUnicode = NULL;
	
	pParams->lexeme = NULL;
		
	pParams->myBlockLengthObj = NULL;
	pParams->fpLengthObjRef = NULL;	
	
	pParams->myToken.Type = T_NAME;
	pParams->myToken.Value.vString = NULL;
	
	pParams->myTokenLengthObj.Type = T_NAME;
	pParams->myTokenLengthObj.Value.vString = NULL;
		
	pParams->isEncrypted = 0;
	
	pParams->nXRefStreamObjNum = 0;
	pParams->offsetXRefObjStream = 0;
	
	pParams->myObjsTable = NULL;
	pParams->nObjsTableSizeFromPrescanFile = 0;
	pParams->pPagesArray = NULL;
	
	pParams->bUpdateNumBytesReadFromCurrentStream = 1;
		
	pParams->bStreamState = 0;
	pParams->bStringIsDecoded = 0;
	pParams->bStreamType = STREAM_TYPE_GENERIC;
	
	pParams->pUtf8String = NULL;
	
	pParams->pwszCurrentWord = NULL;
	pParams->pwszPreviousWord = NULL;
	
	pParams->nDictionaryType = DICTIONARY_TYPE_GENERIC;
	
	pParams->bReadingStringsFromDecodedStream = 0;
	
	pParams->paCustomizedFont_CharSet = NULL;
	
	pParams->nCurrentFontCodeSpacesNum = 0;
	pParams->pCodeSpaceRangeArray = NULL;
	
	pParams->myPdfTrailer.pIndexArray = NULL;
	pParams->myPdfTrailer.indexArraySize = 0;
	
	pParams->nCurrentFontSubtype = FONT_SUBTYPE_Unknown;
		
	pParams->currentObjStm.pszDecodedStream = NULL;
	pParams->currentObjStm.nDecodedStreamSize = 0;
	pParams->currentObjStm.N = 0;
	pParams->currentObjStm.First = 0;
	pParams->currentObjStm.Extend = 0;
	
	//pParams->pCurrentEncodingArray = &(pParams->aUtf8CharSet[0]);
	pParams->pCurrentEncodingArray = &(pParams->aSTD_CharSet[0]);
	//pParams->pCurrentEncodingArray = &(pParams->aWIN_CharSet[0]);
	//pParams->pCurrentEncodingArray = &(pParams->aMAC_CharSet[0]);
	//pParams->pCurrentEncodingArray = &(pParams->aPDF_CharSet[0]);
	
	InitializeUnicodeArray(pParams);
	InitializeCharSetArrays(pParams);
	InitializeCharSetHashTable(pParams);	
	
	//if ( !genhtInit(&(pParams->myCMapHT), GENHT_SIZE, GenStringHashFunc, GenStringCompareFunc) )
	//{
	//	wprintf(L"\nERROR Parse: genhtInit FAILED.\n");
	//	fwprintf(pParams->fpErrors, L"\nERROR Parse: genhtInit FAILED.\n");
	//	retValue = 0;
	//	goto uscita;
	//}
	
	if ( !InitCMapHT(pParams) )
	{
		snprintf(pParams->szError, 8192, "ERROR Parse: InitCMapHT FAILED\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERROR Parse: InitCMapHT FAILED\n");
		//fwprintf(pParams->fpErrors, L"ERROR Parse: InitCMapHT FAILED\n");
		retValue = 0;
		goto uscita;
	}
	
	tstInit(&(pParams->myTST));
		
	myobjreflist_Init(&(pParams->myXObjRefList));
	myobjreflist_Init(&(pParams->myFontsRefList));
	
	pParams->lexeme = (char *)malloc( sizeof(char) * MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(char) );
	if ( !(pParams->lexeme) )
	{
		snprintf(pParams->szError, 8192, "ERRORE Parse 1: Memoria insufficiente.\n\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE Parse 1: Memoria insufficiente.\n\n");
		//fwprintf(pParams->fpErrors, L"ERRORE Parse 1: Memoria insufficiente.\n\n");
		retValue = 0;
		goto uscita;
	}
	for ( x = 0; x < MAX_STRING_LENTGTH_IN_CONTENT_STREAM; x++ )
		pParams->lexeme[x] = '\0';
		
		
	pParams->lexemeTemp = (char *)malloc( sizeof(char) * MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(char) );
	if ( !(pParams->lexemeTemp) )
	{
		snprintf(pParams->szError, 8192, "ERRORE Parse 1 bis: Memoria insufficiente.\n\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE Parse 1: Memoria insufficiente.\n\n");
		//fwprintf(pParams->fpErrors, L"ERRORE Parse 1 bis: Memoria insufficiente.\n\n");
		retValue = 0;
		goto uscita;
	}
	for ( x = 0; x < MAX_STRING_LENTGTH_IN_CONTENT_STREAM; x++ )
		pParams->lexemeTemp[x] = '\0';
			
	pParams->pUtf8String = (wchar_t*)malloc( sizeof(wchar_t) * MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(wchar_t) );
	if ( !(pParams->pUtf8String) )
	{
		snprintf(pParams->szError, 8192, "ERRORE Parse 2: Memoria insufficiente.\n\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE Parse 2: Memoria insufficiente.\n\n");
		//fwprintf(pParams->fpErrors, L"ERRORE Parse 2: Memoria insufficiente.\n\n");
		retValue = 0;
		goto uscita;
	}
	for ( x = 0; x < MAX_STRING_LENTGTH_IN_CONTENT_STREAM; x++ )
		pParams->pUtf8String[x] = L'\0';
	
	pParams->myBlock = (unsigned char *)malloc(sizeof(unsigned char) * BLOCK_SIZE);
	if ( !(pParams->myBlock) )
	{
		snprintf(pParams->szError, 8192, "ERRORE Parse 3: Memoria insufficiente.\n\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE Parse 3: Memoria insufficiente.\n\n");
		//fwprintf(pParams->fpErrors, L"ERRORE Parse 3: Memoria insufficiente.\n\n");
		retValue = 0;
		goto uscita;
	}	
	
	pParams->myBlockLengthObj = (unsigned char *)malloc(sizeof(unsigned char) * BLOCK_SIZE);
	if ( !(pParams->myBlockLengthObj) )
	{
		snprintf(pParams->szError, 8192, "ERRORE Parse 4: Memoria insufficiente.\n\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE Parse 4: Memoria insufficiente.\n\n");
		//fwprintf(pParams->fpErrors, L"ERRORE Parse 4: Memoria insufficiente.\n\n");
		retValue = 0;
		goto uscita;
	}	
		

	n = myFilesList;
	while( n != NULL )
	{
		pParams->isEncrypted = 0;
		
		pParams->pReadNextChar = ReadNextChar;
			
		pParams->nCountPagesFromPdf = 0;
		if ( NULL != pParams->pPagesArray )
		{
			free(pParams->pPagesArray);
			pParams->pPagesArray = NULL;
		}
					
		#if !defined(_WIN64) && !defined(_WIN32)
		len = strnlen(n->myPathName, PATH_MAX);
		strncpy(pParams->szFileName, n->myPathName, len + 1);
		strncat(pParams->szFileName, "/", len + 1);
		len = strnlen(n->myFileName, PATH_MAX);
		strncat(pParams->szFileName, n->myFileName, len + 1);
		#else	
		len = strnlen(n->myFileName, PATH_MAX);
		strncpy(pParams->szFileName, n->myFileName, len + 1);
		#endif
				
		if ( pParams->fp != NULL )
		{
			fclose(pParams->fp);
			pParams->fp = NULL;			
		}
						
		pParams->fp = fopen(pParams->szFileName, "rb");
		if ( pParams->fp == NULL )
		{
			snprintf(pParams->szError, 8192, "ERRORE Parse 5: nell'apertura del file '%s'.\n\n", pParams->szFileName);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE Parse 5: nell'apertura del file '%s'.\n", pParams->szFileName);
			//fwprintf(pParams->fpErrors, L"ERRORE Parse 5: nell'apertura del file '%s'.\n\n", pParams->szFileName);
										
			retValue = 0;
			goto successivo;
		}
								
		if ( pParams->szOutputFile[0] != '\0' )
		{
			#if !defined(_WIN64) && !defined(_WIN32)
			fwprintf(pParams->fpOutput, L"File: '%s'\n", pParams->szFileName);
			#else
			fwprintf(pParams->fpOutput, L"FILE = <");
			lung = strnlen(pParams->szFileName, PATH_MAX);
			for ( int idx = 0; idx < lung; idx++ )
			{
				fwprintf(pParams->fpOutput, L"%c", pParams->szFileName[idx]);
			}
			fwprintf(pParams->fpOutput, L">\n");			
			#endif
		}
		else
		{
			#if !defined(_WIN64) && !defined(_WIN32)
			wprintf(L"File: '%s'\n", pParams->szFileName);
			#else
			lung = strnlen(pParams->szFileName, PATH_MAX);
			wprintf(L"FILE = <");
			for ( int idx = 0; idx < lung; idx++ )
			{
				wprintf(L"%c", pParams->szFileName[idx]);
			}
			wprintf(L">\n");
			#endif
		}
		
				
		// ****************************************** PRE PARSING FILE ***************************************************************
		getObjsOffsets(pParams, pParams->szFileName);		
		// ***************************************************************************************************************************
		
		if ( !ReadHeader(pParams) )
		{
			snprintf(pParams->szError, 8192, "ERRORE Parse 5 tris nella lettura del file header '%s'.\n\n", pParams->szFileName);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE Parse 5 tris nella lettura del file header '%s'.\n\n", pParams->szFileName);
			//fwprintf(pParams->fpErrors, L"ERRORE Parse 5 tris nella lettura del file header '%s'.\n\n", pParams->szFileName);
			retValue = 0;
			goto successivo;
		}
				
		pParams->myPdfTrailer.Size = 0;
		pParams->myPdfTrailer.Prev = 0;
		pParams->myPdfTrailer.Root.Number = 0;
		pParams->myPdfTrailer.Root.Generation = 0;
		
		if ( NULL != pParams->myPdfTrailer.pIndexArray )
		{
			for ( int i = 0; i < pParams->myPdfTrailer.indexArraySize; i++ )
			{
				if ( NULL != pParams->myPdfTrailer.pIndexArray[i] )
				{
					free(pParams->myPdfTrailer.pIndexArray[i]);
					pParams->myPdfTrailer.pIndexArray[i] = NULL;
				}
			}
			free(pParams->myPdfTrailer.pIndexArray);
			pParams->myPdfTrailer.pIndexArray = NULL;
		}
		pParams->myPdfTrailer.indexArraySize = 0;
		
		pParams->trailerW1 = -1;
		pParams->trailerW2 = -1;
		pParams->trailerW3 = -1;
	
		pParams->isEncrypted = 0;
	
		mynumstacklist_Init( &(pParams->myNumStack) );
				
		retReadTrailer = ReadTrailer(pParams);
		
		if ( !retReadTrailer )
		{
			if ( pParams->isEncrypted )
			{
				snprintf(pParams->szError, 8192, "Encrypted file '%s'.\n", pParams->szFileName);
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"Encrypted file '%s'.\n", pParams->szFileName);
				//fwprintf(pParams->fpErrors, L"Encrypted file '%s'.\n", pParams->szFileName);
			}
			else
			{
				snprintf(pParams->szError, 8192, "ERRORE Parse 6 nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"ERRORE Parse 6 nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
				//fwprintf(pParams->fpErrors, L"ERRORE Parse 6 nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
			}
			retValue = 0;
			mynumstacklist_Free( &(pParams->myNumStack) );			
		}
		else if ( 2 == retReadTrailer )
		{
			MyIntQueueList_t myObjsQueue;
			int myObjNum;
			
			myintqueuelist_Init(&myObjsQueue);
			
			for ( uint32_t k = 0; k < pParams->nObjsTableSizeFromPrescanFile; k++ )
			{
				if ( pParams->myObjsTable[k]->Obj.Offset > 0 )
				{
					if ( !CheckObjectType(pParams, k) )
					{
						snprintf(pParams->szError, 8192, "ERRORE Parse 6(CheckObjectType) nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
						myShowErrorMessage(pParams, pParams->szError, 1);
						//wprintf(L"ERRORE Parse 6 (CheckObjectType) nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
						//fwprintf(pParams->fpErrors, L"ERRORE Parse 6(CheckObjectType) nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
						PrintThisObject(pParams, k, 0, 0, NULL);
						PrintThisObject(pParams, k, 0, 0, pParams->fpErrors);
						retValue = 0;
						goto successivo;
					}
					pParams->myObjsTable[k]->Obj.Type = pParams->nCotObjType;
					
					if ( OBJ_TYPE_STREAM == pParams->nCotObjType )
					{
						myintqueuelist_Enqueue(&myObjsQueue, (int)k);
					}
				}
				
				myobjreflist_Init(&(pParams->myObjsTable[k]->myXObjRefList));
				myobjreflist_Init(&(pParams->myObjsTable[k]->myFontsRefList));
			}
			
			while ( myintqueuelist_Dequeue(&myObjsQueue, &myObjNum) )
			{
				if ( !ParseStmObj(pParams, myObjNum) )
				{
					snprintf(pParams->szError, 8192, "ERRORE Parse 6(ParseStmObj) nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
					myShowErrorMessage(pParams, pParams->szError, 1);
					//wprintf(L"ERRORE Parse 6 (ParseStmObj) nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
					//fwprintf(pParams->fpErrors, L"ERRORE Parse 6(ParseStmObj) nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
					PrintThisObject(pParams, myObjNum, 0, 0, NULL);
					PrintThisObject(pParams, myObjNum, 0, 0, pParams->fpErrors);
					myintqueuelist_Free(&myObjsQueue);
					
					if ( NULL != pParams->currentObjStm.pszDecodedStream )
					{
						free(pParams->currentObjStm.pszDecodedStream);
						pParams->currentObjStm.pszDecodedStream = NULL;
					}
					pParams->currentObjStm.nDecodedStreamSize = 0;
				
					retValue = 0;
					goto successivo;
				}
				
				if ( NULL != pParams->currentObjStm.pszDecodedStream )
				{
					free(pParams->currentObjStm.pszDecodedStream);
					pParams->currentObjStm.pszDecodedStream = NULL;
				}
				pParams->currentObjStm.nDecodedStreamSize = 0;
			}
			
			myintqueuelist_Free(&myObjsQueue);
			
			if ( !ParseTrailerXRefStreamObject(pParams) )
			{
				snprintf(pParams->szError, 8192, "ERRORE Parse 6(ParseTrailerXRefStreamObject) nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"ERRORE Parse 6 (ParseTrailerXRefStreamObject) nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
				//fwprintf(pParams->fpErrors, L"ERRORE Parse 6(ParseTrailerXRefStreamObject) nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
				retValue = 0;
				goto successivo;
			}
		}
			
		/*			
		if ( pParams->myPdfTrailer.Size != (int)pParams->nObjsTableSizeFromPrescanFile )
		{
			//wprintf(L"\nERRORE Parse 6 tris: pParams->myPdfTrailer.Size = %d -> pParams->nObjsTableSizeFromPrescanFile = %lu\n", pParams->myPdfTrailer.Size, pParams->nObjsTableSizeFromPrescanFile);
			fwprintf(pParams->fpErrors, L"\nERRORE Parse 6 tris: pParams->myPdfTrailer.Size = %d -> pParams->nObjsTableSizeFromPrescanFile = %lu\n", pParams->myPdfTrailer.Size, pParams->nObjsTableSizeFromPrescanFile);
			//pParams->myPdfTrailer.Size = (int)pParams->nObjsTableSizeFromPrescanFile;
			retValue = 0;
			goto successivo;
		}
		*/
		
		mynumstacklist_Free( &(pParams->myNumStack) );
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)		
		wprintf(L"ReadTrailer OK!!!\n");					
		wprintf(L"Trailer Size: %d\n", pParams->myPdfTrailer.Size);
		wprintf(L"nObjsTableSizeFromPrescanFile: %lu\n", pParams->nObjsTableSizeFromPrescanFile);
		wprintf(L"Trailer Root: %d %d\n", pParams->myPdfTrailer.Root.Number, pParams->myPdfTrailer.Root.Generation);
		wprintf(L"Trailer Prev: %d\n\n", pParams->myPdfTrailer.Prev);
		#endif
				
		pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
		pParams->blockCurPos = 0;
		
		pParams->nCurrentFontSubtype = FONT_SUBTYPE_Type1;
						
		if ( !ParseObject(pParams, pParams->myPdfTrailer.Root.Number) )
		{
			retValue = 0;
			goto successivo;
		}
				
successivo:

//**********************************************************************************************************************************
		if ( pParams->myObjsTable != NULL )
		{
			//for ( x = 0; x < pParams->myPdfTrailer.Size; x++ )
			for ( x = 0; x < pParams->nObjsTableSizeFromPrescanFile; x++ )
			{
				if ( pParams->myObjsTable[x] != NULL )
				{
					if ( NULL != pParams->myObjsTable[x]->Obj.pszDecodedStream )
					{
						free(pParams->myObjsTable[x]->Obj.pszDecodedStream);
						pParams->myObjsTable[x]->Obj.pszDecodedStream = NULL;
					}
					myobjreflist_Free(&(pParams->myObjsTable[x]->myXObjRefList));
					myobjreflist_Free(&(pParams->myObjsTable[x]->myFontsRefList));
					free(pParams->myObjsTable[x]);
					pParams->myObjsTable[x] = NULL;
				}
			}
		
			free(pParams->myObjsTable);
			pParams->myObjsTable = NULL;
		}
		
		if ( NULL != pParams->pPagesArray )
		{
			free(pParams->pPagesArray);
			pParams->pPagesArray = NULL;
		}
			
		if ( NULL != pParams->myTST.pRoot )
		{
			tstFreeRecursive(&(pParams->myTST), pParams->myTST.pRoot);
		}
		
		if ( NULL != pParams->pwszCurrentWord )
		{
			free(pParams->pwszCurrentWord);
			pParams->pwszCurrentWord = NULL;
		}
	
		if ( NULL != pParams->pwszPreviousWord )
		{
			free(pParams->pwszPreviousWord);
			pParams->pwszPreviousWord = NULL;
		}		
	
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}
		
		if ( NULL != pParams->currentObjStm.pszDecodedStream )
		{
			free(pParams->currentObjStm.pszDecodedStream);
			pParams->currentObjStm.pszDecodedStream = NULL;
		}
		pParams->currentObjStm.nDecodedStreamSize = 0;
		pParams->currentObjStm.N = 0;
		pParams->currentObjStm.First = 0;
		pParams->currentObjStm.Extend = 0;
	
//**********************************************************************************************************************************
		
		n = n->next;
	}	

uscita:

	if ( pParams->lexeme != NULL )
	{
		free(pParams->lexeme);
		pParams->lexeme = NULL;
	}
	
	if ( pParams->lexemeTemp != NULL )
	{
		free(pParams->lexemeTemp);
		pParams->lexemeTemp = NULL;
	}	
	
	if ( pParams->pUtf8String != NULL )
	{
		free(pParams->pUtf8String);
		pParams->pUtf8String = NULL;
	}	

	if ( pParams->myBlock != NULL )
	{
		free(pParams->myBlock);
		pParams->myBlock = NULL;
	}

	if ( pParams->myBlockLengthObj != NULL )
	{
		free(pParams->myBlockLengthObj);
		pParams->myBlockLengthObj = NULL;
	}
	
	if ( pParams->fp != NULL )
		fclose(pParams->fp);	
			
	if ( pParams->myObjsTable != NULL )
	{
		//for ( x = 0; x < pParams->myPdfTrailer.Size; x++ )
		for ( x = 0; x < pParams->nObjsTableSizeFromPrescanFile; x++ )
		{
			if ( pParams->myObjsTable[x] != NULL )
			{
				if ( NULL != pParams->myObjsTable[x]->Obj.pszDecodedStream )
				{
					free(pParams->myObjsTable[x]->Obj.pszDecodedStream);
					pParams->myObjsTable[x]->Obj.pszDecodedStream = NULL;
				}
				myobjreflist_Free(&(pParams->myObjsTable[x]->myXObjRefList));
				myobjreflist_Free(&(pParams->myObjsTable[x]->myFontsRefList));
				free(pParams->myObjsTable[x]);
				pParams->myObjsTable[x] = NULL;
			}
		}
		
		free(pParams->myObjsTable);
		pParams->myObjsTable = NULL;
	}
		
	if ( NULL != pParams->pPagesArray )
	{
		free(pParams->pPagesArray);
		pParams->pPagesArray = NULL;
	}
		
	htFree(&(pParams->myCharSetHashTable));
	
	if ( NULL != pParams->myTST.pRoot )
	{
		tstFreeRecursive(&(pParams->myTST), pParams->myTST.pRoot);
	}
		
	if ( NULL != pParams->pwszCurrentWord )
	{
		free(pParams->pwszCurrentWord);
		pParams->pwszCurrentWord = NULL;
	}
	
	if ( NULL != pParams->pwszPreviousWord )
	{
		free(pParams->pwszPreviousWord);
		pParams->pwszPreviousWord = NULL;
	}		
	
	if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
	{
		if ( NULL != pParams->myToken.Value.vString )
		{
			free(pParams->myToken.Value.vString);
			pParams->myToken.Value.vString = NULL;
		}
	}
	
	if ( NULL != pParams->paCustomizedFont_CharSet )
	{
		free(pParams->paCustomizedFont_CharSet);
		pParams->paCustomizedFont_CharSet = NULL;
	}	
	
	if ( NULL != pParams->pArrayUnicode )
	{
		free(pParams->pArrayUnicode);
		pParams->pArrayUnicode = NULL;
	}
	
	if ( NULL != pParams->fpOutput )
	{
		fclose(pParams->fpOutput);
		pParams->fpOutput = NULL;
	}
	
	if ( NULL != pParams->fpErrors )
	{
		fclose(pParams->fpErrors);
		pParams->fpErrors = NULL;
	}	
	
	if ( NULL != pParams->myPdfTrailer.pIndexArray )
	{
		for ( int i = 0; i < pParams->myPdfTrailer.indexArraySize; i++ )
		{
			if ( NULL != pParams->myPdfTrailer.pIndexArray[i] )
			{
				free(pParams->myPdfTrailer.pIndexArray[i]);
				pParams->myPdfTrailer.pIndexArray[i] = NULL;
			}
		}
		free(pParams->myPdfTrailer.pIndexArray);
		pParams->myPdfTrailer.pIndexArray = NULL;
	}
	
	if ( NULL != pParams->currentObjStm.pszDecodedStream )
	{
		free(pParams->currentObjStm.pszDecodedStream);
		pParams->currentObjStm.pszDecodedStream = NULL;
	}
		
	genhtTraverse(&(pParams->myCMapHT), myGenOnTraverseForFreeData);
	genhtFree( &(pParams->myCMapHT) );
	
	return retValue;
}

int match(Params *pParams, TokenTypeEnum ExpectedToken, char *pszFunctionName)
{
	int retValue = 1;
	char szError[1024];
	
	if ( pParams->myToken.Type == ExpectedToken )
	{
		GetNextToken(pParams);
	}
	else
	{
		retValue = 0;
		
		if ( NULL != pszFunctionName )
		{
			snprintf(szError, 1024, "\nERROR(OBJ num = %d, gen = %d): FUNZIONE match(richiamata dalla funzione '%s' -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pszFunctionName, ExpectedToken, pParams->myToken.Type);
			myShowErrorMessage(pParams, szError, 1);
			//fwprintf(pParams->fpErrors, L"\nERROR(OBJ num = %lu, gen = %lu): FUNZIONE match(richiamata dalla funzione '%s' -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pszFunctionName, ExpectedToken, pParams->myToken.Type);
			//wprintf(L"\nERROR(OBJ num = %lu, gen = %lu): FUNZIONE match(richiamata dalla funzione '%s' -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pszFunctionName, ExpectedToken, pParams->myToken.Type);
		}
		else
		{
			snprintf(szError, 1024, "\nERROR(OBJ num = %d, gen = %d): FUNZIONE match -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, ExpectedToken, pParams->myToken.Type);
			myShowErrorMessage(pParams, szError, 1);
			//fwprintf(pParams->fpErrors, L"\nERROR(OBJ num = %lu, gen = %lu): FUNZIONE match -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, ExpectedToken, pParams->myToken.Type);
			//wprintf(L"\nERROR(OBJ num = %lu, gen = %lu): FUNZIONE match -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, ExpectedToken, pParams->myToken.Type);			
		}
		
		PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
	}
	
#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_MATCH)

	if ( 0 == retValue )
		wprintf(L"Token atteso : ");

	wprintf(L"MATCH ");
	switch ( ExpectedToken )
	{
		case T_ERROR:
			wprintf(L"T_ERROR");
			break;
		case T_UNKNOWN:
			wprintf(L"T_UNKNOWN");
			break;
		case  T_EOF:
			wprintf(L"T_EOF");
			break;
		case T_STRING:
			wprintf(L"T_STRING");
			break;
		case T_STRING_LITERAL:
			wprintf(L"T_STRING_LITERAL");
			break;
		case T_STRING_HEXADECIMAL:
			wprintf(L"T_STRING_HEXADECIMAL");
			break;
		case T_NAME:
			wprintf(L"T_NAME");
			break;
		case T_INT_LITERAL:
			wprintf(L"T_INT_LITERAL");
			break;
		case T_REAL_LITERAL:
			wprintf(L"T_REAL_LITERAL");
			break;
		case T_OPAREN:		// '('
			wprintf(L"T_OPAREN");
			break;
		case T_CPAREN:		// ')'
			wprintf(L"T_CPAREN");
			break;
		case T_QOPAREN:		// '['
			wprintf(L"T_QOPAREN");
			break;
		case T_QCPAREN:		// ']'
			wprintf(L"T_QCPAREN");
			break;
		case T_DICT_BEGIN:   // "<<"
			wprintf(L"T_DICT_BEGIN");
			break;
		case T_DICT_END:     // ">>"
			wprintf(L"T_DICT_END");
			break;
		case T_KW_NULL:
			wprintf(L"T_KW_NULL");
			break;
		case T_KW_OBJ:
			wprintf(L"T_KW_OBJ");
			break;
		case T_KW_ENDOBJ:
			wprintf(L"T_KW_ENDOBJ");
			break;
		case T_KW_STREAM:
			wprintf(L"T_KW_STREAM");
			break;
		case T_KW_ENDSTREAM:
			wprintf(L"T_KW_ENDSTREAM");
			break;
		case T_KW_FALSE:
			wprintf(L"T_KW_FALSE");
			break;
		case T_KW_TRUE:
			wprintf(L"T_KW_TRUE");
			break;
		case T_KW_R:
			wprintf(L"T_KW_R");
			break;
		default:
			break;
	}
	
	if ( 0 == retValue )
		wprintf(L" <> Token trovato : ");
	
	PrintToken(&(pParams->myToken), '\n', '\n', 1);

#endif

	return retValue;
}

int matchLengthObj(Params *pParams, TokenTypeEnum ExpectedToken, char *pszFunctionName)
{
	int retValue = 1;
	
	if ( pParams->myTokenLengthObj.Type == ExpectedToken )
	{
		GetNextTokenLengthObj(pParams);
	}
	else
	{
		retValue = 0;
		
		if ( NULL != pszFunctionName )
		{
			snprintf(pParams->szError, 8192, "\nERROR(OBJ num = %d, gen = %d): FUNZIONE matchLengthObj(richiamata dalla funzione '%s' -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingLengthObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingLengthObj]->Obj.Generation, pszFunctionName, ExpectedToken, pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//fwprintf(pParams->fpErrors, L"\nERROR(OBJ num = %lu, gen = %lu): FUNZIONE matchLengthObj(richiamata dalla funzione '%s' -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingLengthObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingLengthObj]->Obj.Generation, pszFunctionName, ExpectedToken, pParams->myToken.Type);
			//wprintf(L"\nERROR(OBJ num = %lu, gen = %lu): FUNZIONE matchLengthObj(richiamata dalla funzione '%s' -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingLengthObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingLengthObj]->Obj.Generation, pszFunctionName, ExpectedToken, pParams->myToken.Type);
		}
		else
		{
			snprintf(pParams->szError, 8192, "\nERROR(OBJ num = %d, gen = %d): FUNZIONE matchLengthObj -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingLengthObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingLengthObj]->Obj.Generation, ExpectedToken, pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//fwprintf(pParams->fpErrors, L"\nERROR(OBJ num = %lu, gen = %lu): FUNZIONE matchLengthObj -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingLengthObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingLengthObj]->Obj.Generation, ExpectedToken, pParams->myToken.Type);
			//wprintf(L"\nERROR(OBJ num = %lu, gen = %lu): FUNZIONE matchLengthObj -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingLengthObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingLengthObj]->Obj.Generation, ExpectedToken, pParams->myToken.Type);			
		}
	}	
		
#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_MATCH)
	
	if ( 0 == retValue )
		wprintf(L"Token atteso : ");
	
	switch ( ExpectedToken )
	{
		case T_ERROR:
			wprintf(L"T_ERROR");
			break;
		case T_UNKNOWN:
			wprintf(L"T_UNKNOWN");
			break;
		case  T_EOF:
			wprintf(L"T_EOF");
			break;
		case T_STRING:
			wprintf(L"T_STRING");
			break;
		case T_STRING_LITERAL:
			wprintf(L"T_STRING_LITERAL");
			break;
		case T_STRING_HEXADECIMAL:
			wprintf(L"T_STRING_HEXADECIMAL");
			break;
		case T_NAME:
			wprintf(L"T_NAME");
			break;
		case T_INT_LITERAL:
			wprintf(L"T_INT_LITERAL");
			break;
		case T_REAL_LITERAL:
			wprintf(L"T_REAL_LITERAL");
			break;
		case T_OPAREN:		// '('
			wprintf(L"T_OPAREN");
			break;
		case T_CPAREN:		// ')'
			wprintf(L"T_CPAREN");
			break;
		case T_QOPAREN:		// '['
			wprintf(L"T_QOPAREN");
			break;
		case T_QCPAREN:		// ']'
			wprintf(L"T_QCPAREN");
			break;
		case T_DICT_BEGIN:   // "<<"
			wprintf(L"T_DICT_BEGIN");
			break;
		case T_DICT_END:     // ">>"
			wprintf(L"T_DICT_END");
			break;
		case T_KW_NULL:
			wprintf(L"T_KW_NULL");
			break;
		case T_KW_OBJ:
			wprintf(L"T_KW_OBJ");
			break;
		case T_KW_ENDOBJ:
			wprintf(L"T_KW_ENDOBJ");
			break;
		case T_KW_STREAM:
			wprintf(L"T_KW_STREAM");
			break;
		case T_KW_ENDSTREAM:
			wprintf(L"T_KW_ENDSTREAM");
			break;
		case T_KW_FALSE:
			wprintf(L"T_KW_FALSE");
			break;
		case T_KW_TRUE:
			wprintf(L"T_KW_TRUE");
			break;
		case T_KW_R:
			wprintf(L"T_KW_R");
			break;
		default:
			break;
	}
	
	if ( 0 == retValue )
		wprintf(L" <> Token trovato : ");
	
	PrintToken(&(pParams->myToken), '\0', ' ', 1);
	
#endif
	
	return retValue;
}

int myOnTraverse(const void* key, uint32_t keySize, void* data, uint32_t dataSize, uint32_t bContentAlreadyProcessed)
{
	UNUSED(keySize);
	UNUSED(dataSize);
	
	int *pIntData = (int*)data;
	
	if ( NULL != data )
		wprintf(L"\t\tKey -> '%s' -> %d 0 R\n", (char*)key, *pIntData);
	else
		wprintf(L"\t\tKey -> '%s' -> NULL DATA\n", (char*)key);
		
	if ( bContentAlreadyProcessed )
		wprintf(L"\t\tCONTENT GIA' PROCESSATO\n\n");
	else
		wprintf(L"\t\tCONTENT IN ATTESA DI ESSERE PROCESSATO\n\n");
				
	return 1;
}

int PushXObjDecodedContent(Params *pParams, int nPageNumber, int nXObjNumber)
{
	int retValue = 1;
	
	unsigned long int k;
	
	unsigned long int DecodedStreamSize = 0;
			
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent) && !defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent_HEXCODECHAR)
	UNUSED(k);
	#endif	
		
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	UNUSED(nPageNumber);
	UNUSED(nXObjNumber);
	#endif
		
	if ( pParams->CurrentContent.bExternalFile )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)		
		wprintf(L"PushXObjDecodedContent: lo stream e' riferito a un file esterno\n");
		#endif
		
		retValue = 0;
		
		goto uscita;
	}
			
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	wprintf(L"***** INIZIO PushXObjDecodedContent PAGE SCOPE TRAVERSE *****\n\n");
	scopeTraverse(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef), myOnTraverse, 0);
	wprintf(L"***** FINE   PushXObjDecodedContent PAGE SCOPE TRAVERSE *****\n");
	#endif
	
	if ( pParams->CurrentContent.LengthFromPdf > 0 )
	{				
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)		
		wprintf(L"\nSTAMPO LO STREAM(Pag. %d) n° %d *********************:\n", nPageNumber, nXObjNumber);
		#endif
				
		// PUSH
		pParams->nStreamsStackTop++;
	
		pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = NULL;
		
		pParams->myStreamsStack[pParams->nStreamsStackTop].bStreamState = 1;
		pParams->myStreamsStack[pParams->nStreamsStackTop].bStringIsDecoded = 1;
		pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos = 0;
	
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"PushXObjDecodedContent -> AFTER STACK PUSH: pParams->nStreamsStackTop = %d\n", pParams->nStreamsStackTop);
		wprintf(L"STREAM -> Length = %lu", pParams->CurrentContent.LengthFromPdf);
		wprintf(L"\n");
		#endif
		
		pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = getDecodedStream(pParams, &DecodedStreamSize, NULL);
		if ( NULL ==  pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream )
		{
			retValue = 0;
			goto uscita;
		}

		pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize = DecodedStreamSize;
		pParams->myStreamsStack[pParams->nStreamsStackTop].blockLen = pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize;

		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent)
		wprintf(L"\nPushXObjDecodedContent(XOBJ %d) -> INIZIO STREAM DECODIFICATO IN PARTENZA:\n", nXObjNumber);
		for ( k = 0; k < DecodedStreamSize; k++ )
		{
			if ( pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k] == '\0' )
				wprintf(L"\\0");
			else
				wprintf(L"%c", pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k]);
		}
		wprintf(L"PushXObjDecodedContent(XOBJ %d) -> FINE STREAM DECODIFICATO IN PARTENZA.\n\n", nXObjNumber);
		#endif			
				
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent_HEXCODECHAR)				
		wprintf(L"\nPushXObjDecodedContent(XOBJ %d) -> INIZIO STREAM DECODIFICATO IN PARTENZA(HEXCODECHAR):\n", nXObjNumber);
		for ( k = 0; k < DecodedStreamSize; k++ )
		{
			if ( pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k] == '\0' )
				wprintf(L"<00>");
			else
				wprintf(L"<%X>", pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k]);
		}
		wprintf(L"PushXObjDecodedContent(XOBJ %d) -> FINE STREAM DECODIFICATO IN PARTENZA(HEXCODECHAR).\n\n", nXObjNumber);
		#endif	

		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)		
		wprintf(L"FINE STREAM(Pag. %d) n° %d:     *********************.\n", nPageNumber, nXObjNumber);
		#endif
	}
	
uscita:
	
	return retValue;
}

int InsertWordIntoTst(Params *pParams)
{
	int k;
	
	pParams->pwszCurrentWord[pParams->idxCurrentWordChar] = L'\0';
	if ( pParams->idxCurrentWordChar > 0 )
	{
		if ( !(pParams->bStateSillab) )
		{
			pParams->myTST.pRoot = tstInsertRecursive(pParams->myTST.pRoot, pParams->pwszCurrentWord, NULL, 0, NULL);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_TST)
			//wprintf(L"INSERT KEY(%ls) INTO TERNARY SEARCH TREE\n", pParams->pwszCurrentWord);
			wprintf(L"(%ls)\n", pParams->pwszCurrentWord);
			#endif
								
			pParams->idxCurrentWordChar = pParams->idxPreviousWordChar = 0;
			while ( L'\0' != pParams->pwszCurrentWord[pParams->idxCurrentWordChar] )
			{
				pParams->pwszPreviousWord[pParams->idxPreviousWordChar++] = pParams->pwszCurrentWord[pParams->idxCurrentWordChar++];
			}
			pParams->pwszPreviousWord[pParams->idxPreviousWordChar] = L'\0';

			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS)
			// ************************************************************************************************************
			wprintf(L"\nECCOMI QUA: INSERISCO (%ls) NEL TERNARY SEARCH TREE.\n", pParams->pwszCurrentWord);
			wprintf(L"\tPREVIOUS WORD = (%ls)\n\n", pParams->pwszPreviousWord);
			// ************************************************************************************************************
			#endif
								
					
			pParams->idxCurrentWordChar = 0;
			//pParams->idxPreviousWordChar = 0; // QUI NON VA BENE.
		}
		else
		{
			k = 0;
			while ( k < pParams->idxCurrentWordChar )
			{
				pParams->pwszPreviousWord[pParams->idxPreviousWordChar + k] = pParams->pwszCurrentWord[k];
				k++;
			}
			pParams->pwszPreviousWord[pParams->idxPreviousWordChar + k] = L'\0';
			
			pParams->myTST.pRoot = tstInsertRecursive(pParams->myTST.pRoot, pParams->pwszPreviousWord, NULL, 0, NULL);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_TST)
			//wprintf(L"INSERT KEY(%ls) INTO TERNARY SEARCH TREE\n", pParams->pwszPreviousWord);
			wprintf(L"(%ls)\n", pParams->pwszPreviousWord);
			#endif
			
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS)
			// ************************************************************************************************************
			wprintf(L"\nEQQUE QUA: INSERISCO (%ls) NEL TERNARY SEARCH TREE.\n", pParams->pwszCurrentWord);
			wprintf(L"\tPREVIOUS WORD = (%ls)\n\n", pParams->pwszPreviousWord);
			// ************************************************************************************************************
			#endif
			
																
			pParams->idxCurrentWordChar = 0;
			pParams->idxPreviousWordChar = 0;	
			
			pParams->bStateSillab = 0;		
		}
	}
	
	return 1;	
}

int ManageDecodedContent(Params *pParams, int nPageNumber)
{
	int retValue = 1;
	
	char szName[512];
	char szPrevFontResName[512];
	
	unsigned long int nBlockSize = BLOCK_SIZE;
	unsigned long int x;
	
	TokenTypeEnum PrevType = T_UNKNOWN;
	
	unsigned char *pszString = NULL;
	wchar_t *pWideCharString = NULL;
	size_t len;	
	
	wchar_t c;
		
	uint32_t bContentAlreadyProcessed;
	int nTemp;
	uint32_t nDataSize;
	int nRes;
		
	int bArrayState = 0;
		
	int bLastNumberIsReal = 0;
	int iLastNumber = 0;
	double dLastNumber = 0.0;
	
	double dFontSize = 12.0;
	
	pParams->bReadingStringsFromDecodedStream = 1;
		
	pszString = (unsigned char *)malloc(MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(unsigned char));
	if ( NULL == pszString )
	{
		snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: impossibile allocare la memoria per pszString.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pszString.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pszString.\n");
		retValue = 0;
		goto uscita;
	}
		
	pParams->pwszCurrentWord = (wchar_t*)malloc(MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(wchar_t));
	if ( NULL == pParams->pwszCurrentWord )
	{
		snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: impossibile allocare la memoria per pParams->pwszCurrentWord.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pwszCurrentWord.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pParams->pwszCurrentWord.\n");
		retValue = 0;
		goto uscita;
	}	
	
	pParams->pwszPreviousWord = (wchar_t*)malloc(MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(wchar_t));
	if ( NULL == pParams->pwszPreviousWord )
	{
		snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: impossibile allocare la memoria per pParams->pwszPreviousWord.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pwszPreviousWord.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pParams->pwszPreviousWord.\n");
		retValue = 0;
		goto uscita;
	}	
	
	pParams->idxCurrentWordChar = 0;
	pParams->pwszCurrentWord[0] = '\0';
	pParams->idxPreviousWordChar = 0;
	pParams->pwszPreviousWord[0] = '\0';
	
	pParams->bStateSillab = 0;
	
	szPrevFontResName[0] = '\0';
	
	// ----------------------------------- STACK -----------------------------------------------		
	while ( pParams->nStreamsStackTop >= 0 )
	{
		qui_dopo_push:
		
		if ( pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize > nBlockSize )
		{
			for ( x = 2; nBlockSize <= pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize ; x++ )
				nBlockSize += BLOCK_SIZE;
		
			if ( NULL != pParams->myBlock )
				free(pParams->myBlock);
			
			pParams->myBlock = (unsigned char *)malloc(sizeof(unsigned char) * nBlockSize);
			if ( !(pParams->myBlock) )
			{
				snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: Memoria insufficiente.\n\n");
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"ERRORE ManageDecodedContent: Memoria insufficiente.\n\n");
				//fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: Memoria insufficiente.\n\n");
				retValue = 0;
				goto uscita;
			}
		}
		
		// PEEK STACK
		pParams->bStreamState = pParams->myStreamsStack[pParams->nStreamsStackTop].bStreamState;
		pParams->bStringIsDecoded = pParams->myStreamsStack[pParams->nStreamsStackTop].bStringIsDecoded;
		pParams->blockCurPos = pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos;		
		pParams->blockLen = pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize;		
		memcpy(pParams->myBlock, pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream, pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize);
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"INIZIO STACK PEEK pParams->nStreamsStackTop = %d\n", pParams->nStreamsStackTop);
		wprintf(L"pParams->blockCurPos = %d, pParams->blockLen = %d\n", pParams->blockCurPos, pParams->blockLen);
		wprintf(L"FINE   STACK PEEK pParams->nStreamsStackTop = %d\n\n", pParams->nStreamsStackTop);
		
		#endif				
	
		pParams->myToken.Type = T_UNKNOWN;
						
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"Page %d -> MY BLOCK(len = %d, size = %lu)\n", nPageNumber, pParams->blockLen, nBlockSize);
		#endif
			
		szName[0] = '\0';
		//szPrevFontResName[0] = '\0';
	
		PrevType = pParams->myToken.Type;
				
		GetNextToken(pParams);
		
		while ( T_VOID_STRING == pParams->myToken.Type ) 
		{
			GetNextToken(pParams);
			if ( T_EOF == pParams->myToken.Type )
			{
				retValue = 0;
				goto uscita;
			}
		}		
			
		while ( T_EOF != pParams->myToken.Type )
		{					
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS)
			PrintToken(&(pParams->myToken), '\t', '\0', 1);
			#endif
			
			if ( T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type )
			{
				len = strnlen(pParams->myToken.Value.vString, MAX_STRING_LENTGTH_IN_CONTENT_STREAM);
				memcpy(pszString, (unsigned char*)pParams->myToken.Value.vString, len + 1);
											
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings)
				//wprintf(L"\tSTRINGA: (%s) <-> UTF-8: (%ls)\n", pszString, (wchar_t*)(pParams->pUtf8String));
				wprintf(L"STRING -> <%ls>", (wchar_t*)(pParams->pUtf8String));
				#endif
				
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS)
				// ****************************************************************************************************************************
				wprintf(L"UNICODE STRING -> <%ls> <- *************************************************************\n", (wchar_t*)(pParams->pUtf8String));
				// ****************************************************************************************************************************
				#endif
				
								
				if ( pParams->szFilePdf[0] == '\0' )
				{
					// SPLIT WORDS INIZIO
					c = (wchar_t)pParams->pUtf8String[0];
					x = 1;
					while ( L'\0' != c )
					{
						c = towlower(c);
					
						if ( c >= L'a' && c <= L'z' )
						{
							pParams->pwszCurrentWord[pParams->idxCurrentWordChar++] = c;
							//wprintf(L"EQQUE QUA -> pParams->pwszCurrentWord[%d] = '%c'\n", pParams->idxCurrentWordChar - 1, pParams->pwszCurrentWord[pParams->idxCurrentWordChar - 1]);
						}
						else if ( L'-' == c )
						{				
							pParams->pwszPreviousWord[pParams->idxPreviousWordChar] = L'\0';
							pParams->bStateSillab = 1;
						}					
						else
						{
							switch ( c )
							{
								case 0x00E1:
								case 0x0103:
								case 0x00E4:
								case 0x00E6:
								case 0x01FD:
								case 0x00E0:
								case 0x03B1:
								case 0x03AC:
								case 0x0101:
								case 0x0105:
								case 0x00E5:
								case 0x01FB:
								case 0x00E3:
								case 0x03B2:
								case 0x0107:
								case 0x010D:
								case 0x00E7:
								case 0x0109:
								case 0x010B:
								case 0x03C7:
								case 0x010F:
								case 0x0111:
								case 0x03B4:
								case 0x0131:
								case 0x0065:
								case 0x00E9:
								case 0x0115:
								case 0x011B:
								case 0x00EA:
								case 0x00EB:
								case 0x0117:
								case 0x00E8:
								case 0x0113:
								case 0x014B:
								case 0x0119:
								case 0x03B5:
								case 0x03AD:
								case 0x03B7:
								case 0x03AE:
								case 0x00F0:
								case 0x0192:
								case 0x03B3:
								case 0x011F:
								case 0x01E7:
								case 0x011D:
								case 0x0121:
								case 0x00DF:
								case 0x0127:
								case 0x0125:
								case 0x00ED:
								case 0x012D:
								case 0x00EE:
								case 0x00EF:
								case 0x00EC:
								case 0x0133:
								case 0x012B:
								case 0x012F:
								case 0x03B9:
								case 0x03CA:
								case 0x0390:
								case 0x03AF:
								case 0x0129:
								case 0x0135:
								case 0x03BA:
								case 0x0138:
								case 0x013A:
								case 0x03BB:
								case 0x013E:
								case 0x0140:
								case 0x017F:
								case 0x0142:
								case 0x0144:
								case 0x0149:
								case 0x0148:
								case 0x00F1:
								case 0x03BD:
								case 0x00F3:
								case 0x014F:
								case 0x00F4:
								case 0x00F6:
								case 0x0153:
								case 0x00F2:
								case 0x01A1:
								case 0x0151:
								case 0x014D:
								case 0x03C9:
								case 0x03CE:
								case 0x03BF:
								case 0x03CC:
								case 0x00F8:
								case 0x01FF:
								case 0x00F5:
								case 0x03C6:
								case 0x03D5:
								case 0x03C0:
								case 0x03C8:
								case 0x0071:
								case 0x0155:
								case 0x0159:
								case 0x03C1:
								case 0x015B:
								case 0x0161:
								case 0x015F:
								case 0x015D:
								case 0x03C3:
								case 0x03C2:
								case 0x0074:
								case 0x03C4:
								case 0x0167:
								case 0x0165:
								case 0x00FE:
								case 0x00FA:
								case 0x016D:
								case 0x00FB:
								case 0x00FC:
								case 0x00F9:
								case 0x01B0:
								case 0x0171:
								case 0x016B:
								case 0x0173:
								case 0x03C5:
								case 0x03CB:
								case 0x03B0:
								case 0x03CD:
								case 0x016F:
								case 0x0169:
								case 0x1E83:
								case 0x0175:
								case 0x1E85:
								case 0x2118:
								case 0x1E81:
								case 0x03BE:
								case 0x00FD:
								case 0x0177:
								case 0x00FF:
								case 0x1EF3:
								case 0x017A:
								case 0x017E:
								case 0x017C:
								case 0x03B6:
									pParams->pwszCurrentWord[pParams->idxCurrentWordChar++] = c;
									goto letterastrana;
									break;
								default:
									break;
							}						
																																
							InsertWordIntoTst(pParams);
						}
						
						letterastrana:
						c = (wchar_t)pParams->pUtf8String[x++];
						
						// ATTENZIONE!!! IL CODICE COMMENTATO SEGUENTE, È UN ROGNOSISSIMO BUG! NON DECOMMENTARE! NON CANCELLARE IL CODICE COMMENTATO: A FUTURA MEMORIA!!!
						//if ( L'\0' == c )
						//{
						//	InsertWordIntoTst(pParams);
						//	wprintf(L"INSERITA WORD DOPO LETTERA STRANA. CIAO\n");
						//}
					}					
					// SPLIT WORDS FINE
				}
				else
				{
					if ( pParams->szOutputFile[0] != '\0' )
						fwprintf(pParams->fpOutput, L"%ls", (wchar_t*)(pParams->pUtf8String));
					else
						wprintf(L"%ls", (wchar_t*)(pParams->pUtf8String));
				}
			}						
			else if ( T_INT_LITERAL == pParams->myToken.Type )
			{
				bLastNumberIsReal = 0;
				iLastNumber = pParams->myToken.Value.vInt;
				
				if ( bArrayState )
				{
					//wprintf(L"FONT SIZE = %d <> iLastNumber = %d\n", (int)dFontSize, iLastNumber);
					//if ( iLastNumber < 0 )
					if ( iLastNumber < -((int)dFontSize) )
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings)
						//wprintf(L" <SPAZIO INTEGER %d> ", iLastNumber);
						wprintf(L" ");
						#endif
						if ( pParams->szFilePdf[0] == '\0' )
						{							
							InsertWordIntoTst(pParams);
						}
						else
						{
							if ( pParams->szOutputFile[0] != '\0' )
								fwprintf(pParams->fpOutput, L" ");
							else
								wprintf(L" ");
						}
					}
				}
			}
			else if ( T_REAL_LITERAL == pParams->myToken.Type )
			{
				bLastNumberIsReal = 1;
				dLastNumber = pParams->myToken.Value.vDouble;
				
				if ( bArrayState )
				{
					//wprintf(L"FONT SIZE = %f <> dLastNumber = %d\n", dFontSize, dLastNumber);
					//if ( dLastNumber < 0.0 )
					if ( dLastNumber < -dFontSize )
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings)
						wprintf(L" ");
						#endif
						
						if ( pParams->szFilePdf[0] == '\0' )
						{
							InsertWordIntoTst(pParams);
						}
						else
						{
							if ( pParams->szOutputFile[0] != '\0' )
								fwprintf(pParams->fpOutput, L" ");
							else
								wprintf(L" ");							
						}
					}
				}
			}
			else if ( T_NAME == pParams->myToken.Type )
			{
				strncpy(szName, pParams->myToken.Value.vString, 512 - 1);
			}
			else if ( T_QOPAREN == pParams->myToken.Type )
			{
				bArrayState = 1;
			}
			else if ( T_QCPAREN == pParams->myToken.Type )
			{
				bArrayState = 0;
			}
			else if (
						(T_CONTENT_OP_Td == pParams->myToken.Type) ||
						(T_CONTENT_OP_TD == pParams->myToken.Type) ||
						(T_CONTENT_OP_Tm == pParams->myToken.Type) ||
						(T_CONTENT_OP_TASTERISCO == pParams->myToken.Type) ||
						(T_CONTENT_OP_SINGLEQUOTE == pParams->myToken.Type) ||
						(T_CONTENT_OP_DOUBLEQUOTE == pParams->myToken.Type)
					)
			{							
				/*
				All'interno degli array, un numero negativo dopo la stringa segna la fine della parola, come fosse uno spazio o a capo:
				[(lina ha)-160.92255 (v)13.91769 (olut)3.90831 (o separ)17.80328 (ar)17.86009 (e)-160.85439 (quant)3.89695 (o pos)3.89695 (sibile)-160.91119 (il de)]TJ 
				*/

				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings)
				if ( pParams->szFilePdf[0] == '\0' )
				{
					wprintf(L"\n");
				}
				#endif
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS)
				wprintf(L"\n\tVADO A CAPO ");
				switch ( pParams->myToken.Type )
				{
					case T_CONTENT_OP_Tm:
						wprintf(L"CON L'OPERATORE Tm\n\n");
						break;					
					case T_CONTENT_OP_Td:
						wprintf(L"CON L'OPERATORE Td\n\n");
						break;
					case T_CONTENT_OP_TD:
						wprintf(L"CON L'OPERATORE TD\n\n");
						break;
					case T_CONTENT_OP_TASTERISCO:
						wprintf(L"CON L'OPERATORE T*\n\n");
						break;
					case T_CONTENT_OP_SINGLEQUOTE:
						wprintf(L"CON L'OPERATORE '\n\n");
						break;
					case T_CONTENT_OP_DOUBLEQUOTE:
						wprintf(L"CON L'OPERATORE \"\n\n");
						break;
					default:
						break;
				}
				#endif		
								
				if ( pParams->szFilePdf[0] == '\0' )
				{
					InsertWordIntoTst(pParams);
				}
				else
				{
					if ( pParams->szOutputFile[0] != '\0' )
						fwprintf(pParams->fpOutput, L"\n");
					else
						wprintf(L"\n");
				}
			}
			else if ( T_CONTENT_Do_COMMAND == pParams->myToken.Type )
			{
				if ( '\0' != szName[0] )
				{
					len = strnlen(szName, 128);
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowResourceSelected)
					wprintf(L"TROVATO 'Do' command: vado a prendere la Resource %s\n", szName);
					#endif
									
					nRes = scopeFind(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef), szName, len + sizeof(char), (void*)&nTemp, &nDataSize, &bContentAlreadyProcessed, 1);
					if ( nRes >= 0 ) // TROVATO
					{
						if ( !bContentAlreadyProcessed )
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowResourceSelected)
							wprintf(L"\tVado a fare il parsing dell'oggetto %d 0 R e torno subito.\n", nTemp);
							#endif
					
							bContentAlreadyProcessed = 1;
							if ( !scopeUpdateValue(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef), szName, len + sizeof(char), (void*)&nTemp, nDataSize, bContentAlreadyProcessed, 1, 1) )
							{
								snprintf(pParams->szError, 8192, "\nERRORE ManageDecodedContent scopeUpdateValue 1 : impossibile aggiornare bContentAlreadyProcessed\n"); 
								myShowErrorMessage(pParams, pParams->szError, 1);
								//wprintf(L"\nERRORE ManageDecodedContent scopeUpdateValue 1 : impossibile aggiornare bContentAlreadyProcessed\n"); 
								retValue = 0;
								goto uscita;
							}
							
							// **********************************************************************
							scopePush(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef));
							scopePush(&(pParams->pPagesArray[nPageNumber].myScopeHT_FontsRef));
							
							pParams->bStreamState = 0;
							pParams->bStringIsDecoded = 0;
							pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos = pParams->blockCurPos;	
							
							mydictionaryqueuelist_Init(&(pParams->CurrentContent.decodeParms), 1, 1);
							mystringqueuelist_Init(&(pParams->CurrentContent.queueFilters));
										
							if ( !ParseStreamXObject(pParams, nTemp) )
							{
								snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent ParseStreamXObject.\n");
								myShowErrorMessage(pParams, pParams->szError, 1);
								//wprintf(L"ERRORE ManageDecodedContent ParseStreamXObject.\n");
								//fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent ParseStreamXObject.\n");
								
								snprintf(pParams->szError, 8192, "\n***** ECCO LO SCHIFO:\n");
								myShowErrorMessage(pParams, pParams->szError, 1);
								//wprintf(L"\n***** ECCO LO SCHIFO:\n");
								//fwprintf(pParams->fpErrors, L"\n***** ECCO LO SCHIFO:\n");
								PrintThisObject(pParams, nTemp, 0, 0, pParams->fpErrors);
								
								snprintf(pParams->szError, 8192, "\n***** FINE DELLO SCHIFO:\n");
								myShowErrorMessage(pParams, pParams->szError, 1);
								//wprintf(L"\n***** FINE DELLO SCHIFO\n"); 
								//fwprintf(pParams->fpErrors, L"\n***** FINE DELLO SCHIFO\n"); 
								
								retValue = 0;
								goto uscita;
							}							
							
							if ( !(pParams->bXObjIsImage) )
							{
								PushXObjDecodedContent(pParams, nPageNumber, nTemp);
							}
							else
							{								
								pParams->bStreamState = pParams->myStreamsStack[pParams->nStreamsStackTop].bStreamState;
								pParams->bStringIsDecoded = pParams->myStreamsStack[pParams->nStreamsStackTop].bStringIsDecoded;
								pParams->blockCurPos = pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos;
							}
							
							mystringqueuelist_Free(&(pParams->CurrentContent.queueFilters));
							mydictionaryqueuelist_Free(&(pParams->CurrentContent.decodeParms));
							pParams->myDataDecodeParams.numFilter = 0;
							
							goto qui_dopo_push;
							// **********************************************************************
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowResourceSelected)
						else
						{
							wprintf(L"\tOggetto %d 0 R già processato.\n", nTemp);
						}
						#endif
					}
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowResourceSelected)
					else
					{
						wprintf(L"\tRISORSA XOBJ '%s' NON TROVATA!!!.\n", szName);
					}
					#endif	
					
					//strncpy(szPrevXObjResName, szName, len);
					szName[0] = '\0';				
				}
				else
				{
					snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: trovato comando 'Do' ma non e' preceduto dal nome della Resource!\n");
					myShowErrorMessage(pParams, pParams->szError, 1);
					//wprintf(L"ERRORE ManageDecodedContent: trovato comando 'Do' ma non e' preceduto dal nome della Resource!\n");
					//fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: trovato comando 'Do' ma non e' preceduto dal nome della Resource!\n");
					retValue = 0;
					goto uscita;
				}
				
				//strncpy(szPrevName, szName, len);
				//szName[0] = '\0';
			}
			else if ( T_CONTENT_OP_Tf == pParams->myToken.Type )
			{
				if ( '\0' != szName[0] )
				{
					len = strnlen(szName, 128);
					
					if ( bLastNumberIsReal )
						dFontSize = dLastNumber;
					else
						dFontSize = (double)iLastNumber;
					
					if ( strncmp(szName, szPrevFontResName, 512 - 1) != 0 )
					{
						//len = strnlen(szName, 128);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected)
						//wprintf(L"\nEHI 1: szName = '%s' -> szPrevFontResName = '%s'\n", szName, szPrevFontResName);
						wprintf(L"\nTROVATO 'Tf FONT SELECTOR' command: vado a prendere la Resource %s\n", szName);
						#endif
									
						nRes = scopeFind(&(pParams->pPagesArray[nPageNumber].myScopeHT_FontsRef), szName, len + sizeof(char), (void*)&nTemp, &nDataSize, &bContentAlreadyProcessed, 0);
						if ( nRes >= 0 ) // TROVATO
						{
							//if ( !bContentAlreadyProcessed )
							//{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected)
								wprintf(L"\tVado a fare il parsing dell'oggetto FONT %d 0 R e torno subito.\n", nTemp);
								#endif
					
								bContentAlreadyProcessed = 1;
								if ( !scopeUpdateValue(&(pParams->pPagesArray[nPageNumber].myScopeHT_FontsRef), szName, len + sizeof(char), (void*)&nTemp, nDataSize, bContentAlreadyProcessed, 1, 0) )
								{
									snprintf(pParams->szError, 8192, "\nERRORE ManageDecodedContent scopeUpdateValue 2 : impossibile aggiornare bContentAlreadyProcessed\n"); 
									myShowErrorMessage(pParams, pParams->szError, 1);
									//wprintf(L"\nERRORE ManageDecodedContent scopeUpdateValue 2 : impossibile aggiornare bContentAlreadyProcessed\n"); 
									retValue = 0;
									goto uscita;
								}
							
								// **********************************************************************
								//scopePush(&(pParams->pPagesArray[nPageNumber].myScopeHT_FontsRef));
							
								pParams->bStreamState = 0;
								pParams->bStringIsDecoded = 0;
								pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos = pParams->blockCurPos;	
																	
								if ( !ParseFontObject(pParams, nTemp) )
								{
									snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent ParseFontObject.\n"); 
									myShowErrorMessage(pParams, pParams->szError, 1);
									//fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent ParseFontObject.\n"); 
								
									snprintf(pParams->szError, 8192, "\n***** ECCO L'OGGETTO ERRATO:\n");
									myShowErrorMessage(pParams, pParams->szError, 1);
									//fwprintf(pParams->fpErrors, L"\n***** ECCO L'OGGETTO ERRATO:\n");
								
									PrintThisObject(pParams, nTemp, 0, 0, pParams->fpErrors);
								
									snprintf(pParams->szError, 8192, "\n***** FINE OGGETTO ERRATO:\n");
									myShowErrorMessage(pParams, pParams->szError, 1);
									//fwprintf(pParams->fpErrors, L"\n***** FINE OGGETTO ERRATO\n");
									retValue = 0;
									goto uscita;
								}
														
								pParams->bStreamState = pParams->myStreamsStack[pParams->nStreamsStackTop].bStreamState;
								pParams->bStringIsDecoded = pParams->myStreamsStack[pParams->nStreamsStackTop].bStringIsDecoded;
								pParams->blockCurPos = pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos;
								
								strncpy(szPrevFontResName, szName, len);
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected)
								wprintf(L"\nEHI 2: szName = '%s' -> szPrevFontResName = '%s'\n", szName, szPrevFontResName);
								#endif
								//szName[0] = '\0';
							
								goto qui_dopo_push;
								// **********************************************************************
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected)
						else
						{
							wprintf(L"\tRISORSA FONT '%s' NON TROVATA!!!.\n", szName);
						}
						#endif
					}
					
					//strncpy(szPrevFontResName, szName, len);
					//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected)
					//wprintf(L"\nEHI 3: szName = '%s' -> szPrevFontResName = '%s'\n", szName, szPrevFontResName);
					//#endif
					//szName[0] = '\0';
				}
				else
				{
					snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: trovato comando 'Do' ma non e' preceduto dal nome della Resource!\n");
					myShowErrorMessage(pParams, pParams->szError, 1);
					//wprintf(L"ERRORE ManageDecodedContent: trovato comando 'Do' ma non e' preceduto dal nome della Resource!\n");
					//fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: trovato comando 'Do' ma non e' preceduto dal nome della Resource!\n");
					retValue = 0;
					goto uscita;
				}
				
				//strncpy(szPrevFontResName, szName, len);
				//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected)
				//wprintf(L"\nEHI 2: szName = '%s' -> szPrevFontResName = '%s'\n", szName, szPrevFontResName);
				//#endif
				//szName[0] = '\0';
			}
			else if ( T_CONTENT_OP_BI == pParams->myToken.Type )
			{
				// IGNORIAMO L'OPERATORE BI(Begin Image) FINO ALLA FINE DELLO STREAM
				goto libera;
			}			
							
			GetNextToken(pParams);
						
			while ( T_VOID_STRING == pParams->myToken.Type )
			{
				GetNextToken(pParams);
				if ( T_EOF == pParams->myToken.Type )
				{
					retValue = 0;
					goto uscita;
				}
			}			
		}
		
		libera:
				
		// POP
		if ( NULL != pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream )
			free(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream);
		pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = NULL;
		pParams->nStreamsStackTop--;
		
		scopePop(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef));
		scopePop(&(pParams->pPagesArray[nPageNumber].myScopeHT_FontsRef));
		
		mystringqueuelist_Free(&(pParams->CurrentContent.queueFilters));
		mydictionaryqueuelist_Free(&(pParams->CurrentContent.decodeParms));
		pParams->myDataDecodeParams.numFilter = 0;
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"AFTER STACK POP: pParams->nStreamsStackTop = %d\n", pParams->nStreamsStackTop);
		#endif		
		
	
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"\n");
		#endif
	}
	
uscita:

	pParams->bReadingStringsFromDecodedStream = 0;

	if ( NULL != pszString )
	{
		free(pszString);
		pszString = NULL;
	}
	
	if ( NULL != pWideCharString )
	{
		free(pWideCharString);
		pWideCharString = NULL;
	}
	
	if ( NULL != pParams->pwszCurrentWord )
	{
		free(pParams->pwszCurrentWord);
		pParams->pwszCurrentWord = NULL;
	}
	
	if ( NULL != pParams->pwszPreviousWord )
	{
		free(pParams->pwszPreviousWord);
		pParams->pwszPreviousWord = NULL;
	}	
		
	while ( pParams->nStreamsStackTop >= 0 )
	{
		if ( NULL != pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream )
			free(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream);			
		pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = NULL;
		
		pParams->nStreamsStackTop--;
	}
	
	if ( NULL != pParams->pCodeSpaceRangeArray )
	{
		free(pParams->pCodeSpaceRangeArray);
		pParams->pCodeSpaceRangeArray = NULL;
	}
	
	mystringqueuelist_Free(&(pParams->CurrentContent.queueFilters));
	mydictionaryqueuelist_Free(&(pParams->CurrentContent.decodeParms));
	pParams->myDataDecodeParams.numFilter = 0;

	pParams->bStreamState = 0;
	pParams->bStringIsDecoded = 0;
	pParams->myToken.Type = PrevType;
					
	return retValue;
}

int ManageContent(Params *pParams, int nPageNumber)
{
	int retValue = 1;	
		
	MyContent_t myContent;
	MyContent_t *pContent;
	MyContentQueueItem_t* pContentItem;
	unsigned long int totalLengthFromPdf = 0;
	
	unsigned long int k;
	
	unsigned long int DecodedStreamSize = 0;
	
	unsigned long int offsetEncodedStream = 0;
	
	unsigned char *pszDecodedStream = NULL;
	unsigned long int offsetDecodedStream = 0;
	
	unsigned char *pszDecodedStreamNew = NULL;
	
	unsigned long int bytesAllocatedForDecodedStreamOnStack = 0;
	
	int nTemp;
	
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent) && !defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent_HEXCODECHAR)
	UNUSED(k);
	#endif
	
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
	UNUSED(nPageNumber);
	#endif	
			
	for ( nTemp = 0; nTemp < STREAMS_STACK_SIZE; nTemp++ )
		pParams->myStreamsStack[nTemp].pszDecodedStream = NULL;	
	
	pContentItem = pParams->pPagesArray[nPageNumber].queueContens.head;
	totalLengthFromPdf = 0;
	while ( NULL != pContentItem )
	{
		totalLengthFromPdf += pContentItem->myContent.LengthFromPdf;
		pContentItem = pContentItem->next;
	}
	
	if ( totalLengthFromPdf <= 0 )
		goto uscita;
				
	// PUSH BEGIN
	pParams->nStreamsStackTop = 0;
		
	DecodedStreamSize = ( totalLengthFromPdf * sizeof(unsigned char) ) * 55 + sizeof(unsigned char);
	
	if ( DecodedStreamSize > 409600000 )
		DecodedStreamSize = 409600000;
		
	if ( DecodedStreamSize < totalLengthFromPdf )
		DecodedStreamSize = totalLengthFromPdf + (4096 * 89);
	
	pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = (unsigned char *)malloc( DecodedStreamSize );
	if ( NULL == pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream )
	{
		snprintf(pParams->szError, 8192, "ERRORE ManageContent: impossibile allocare la memoria per pszDecodedStream sullo stack %d.\n", pParams->nStreamsStackTop);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE ManageContent: impossibile allocare la memoria per pszDecodedStream sullo stack %d.\n", pParams->nStreamsStackTop);
		//fwprintf(pParams->fpErrors, L"ERRORE ManageContent: impossibile allocare la memoria per pszDecodedStream sullo stack %d.\n", pParams->nStreamsStackTop);
		pParams->nStreamsStackTop--;
		retValue = 0;
		goto uscita;		
	}	
	for ( unsigned long int x = 0; x < DecodedStreamSize; x++ )
		pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[x] = '\0';
		
	bytesAllocatedForDecodedStreamOnStack = DecodedStreamSize;
	
	
		
	myContent.bExternalFile = 0;
	myContent.LengthFromPdf = 0;
	myContent.Offset = 0;
	mydictionaryqueuelist_Init(&(myContent.decodeParms), 1, 1);
	mystringqueuelist_Init(&(myContent.queueFilters));	

	pContent = &myContent;
		
	if ( pContent->bExternalFile )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)		
		wprintf(L"\nSTAMPO LO STREAM(Pag. %d) *********************:\n", nPageNumber);
		wprintf(L"Lo stream e' riferito a un file esterno\n");
		wprintf(L"FINE STREAM(Pag. %d)     *********************.\n", nPageNumber);
		#endif
		
		goto uscita;
	}
					
	offsetEncodedStream = 0;
	while ( mycontentqueuelist_Dequeue(&(pParams->pPagesArray[nPageNumber].queueContens), &myContent) )
	{									
		offsetEncodedStream += pContent->LengthFromPdf;
								
		if ( NULL != pszDecodedStream )
		{
			free(pszDecodedStream);
			pszDecodedStream = NULL;
		}
		
		pszDecodedStream = getDecodedStream(pParams, &DecodedStreamSize, pContent);
		if ( NULL ==  pszDecodedStream )
		{
			retValue = 0;
			goto uscita;
		}
					
		if ( DecodedStreamSize > (bytesAllocatedForDecodedStreamOnStack - offsetDecodedStream) )
		{
			bytesAllocatedForDecodedStreamOnStack = DecodedStreamSize * 3;
						
			pszDecodedStreamNew = (unsigned char*)realloc(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream, bytesAllocatedForDecodedStreamOnStack);
			if ( NULL == pszDecodedStreamNew )
			{
				snprintf(pParams->szError, 8192, "ERRORE ManageContent: impossibile reallocare %lu byte per leggere lo stream\n", bytesAllocatedForDecodedStreamOnStack);
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"ERRORE ManageContent: impossibile reallocare %lu byte per leggere lo stream\n", bytesAllocatedForDecodedStreamOnStack);				      
				//fwprintf(pParams->fpErrors, L"ERRORE ManageContent: impossibile reallocare %lu byte per leggere lo stream\n", bytesAllocatedForDecodedStreamOnStack);
						      								      
				retValue = 0;
				goto uscita;
			}
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
			else
			{
				wprintf(L"\nManageContent Y (PAGE %d) -> REALLOCATI CORRETTAMENTE %lu BYTES\n", nPageNumber, bytesAllocatedForDecodedStreamOnStack);
			}
			#endif

			pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = pszDecodedStreamNew;						
		}
																
		memcpy(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream + offsetDecodedStream, pszDecodedStream, DecodedStreamSize);
		offsetDecodedStream += DecodedStreamSize;
	}
	
	//pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize = DecodedStreamSize;
	//pParams->myStreamsStack[pParams->nStreamsStackTop].blockLen = DecodedStreamSize;
	pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize = offsetDecodedStream;
	pParams->myStreamsStack[pParams->nStreamsStackTop].blockLen = offsetDecodedStream;
	pParams->myStreamsStack[pParams->nStreamsStackTop].bStreamState = 1;
	pParams->myStreamsStack[pParams->nStreamsStackTop].bStringIsDecoded = 1;
	pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos = 0;	
	// PUSH END	
		
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	wprintf(L"STREAM -> Length = %lu", totalLengthFromPdf);
	wprintf(L"\n");
	#endif	

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent)
	wprintf(L"\n\nManageContent -> INIZIO STREAM DECODIFICATO DOPO myInflate: pParams->nStreamsStackTop = %d; blockLen = %d\n", pParams->nStreamsStackTop, pParams->myStreamsStack[pParams->nStreamsStackTop].blockLen);
	for ( k = 0; k < offsetDecodedStream; k++ )
	{
		if ( pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k] == '\0' )
			wprintf(L"\\0");
		else
			wprintf(L"%c", pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k]);
	}
	wprintf(L"ManageContent -> FINE STREAM DECODIFICATO DOPO myInflate. pParams->nStreamsStackTop = %d; blockLen = %d\n", pParams->nStreamsStackTop, pParams->myStreamsStack[pParams->nStreamsStackTop].blockLen);
	#endif
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent_HEXCODECHAR)
	wprintf(L"\n\nManageContent -> INIZIO STREAM DECODIFICATO DOPO myInflate(HEXCODECHAR):\n");
	for ( k = 0; k < offsetDecodedStream; k++ )
	{
		if ( pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k] == '\0' )
			wprintf(L"<00>");
		else
			wprintf(L"<%X>", pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k]);
	}
	wprintf(L"ManageContent -> FINE STREAM DECODIFICATO DOPO myInflate(HEXCODECHAR).\n\n");
	#endif
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	wprintf(L"FINE STREAM(Pag. %d)     *********************.\n", nPageNumber);
	#endif	
		
	retValue = ManageDecodedContent(pParams, nPageNumber);
			
uscita:

	if ( NULL != pszDecodedStream )
	{
		free(pszDecodedStream);
		pszDecodedStream = NULL;
	}
	
	while ( pParams->nStreamsStackTop >= 0 )
	{
		if ( NULL != pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream )
			free(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream);
		pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = NULL;
		
		pParams->nStreamsStackTop--;
	}
			
	return retValue;
}

int LoadFirstBlock(Params *pParams, int objNum, const char *pszFunctionName)
{
	int retValue = 1;
	
	unsigned long int nBlockSize = BLOCK_SIZE;
	unsigned long int x;
	
	unsigned char c;
	
	char szTemp1[1024];
	size_t lenTemp1;
	
	char szTemp2[1024];
	size_t lenTemp2;
	
	uint32_t newStreamLength;
	uint32_t blockOffset;
	
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
		
	if ( OBJ_TYPE_IN_USE == pParams->myObjsTable[objNum]->Obj.Type )
	{
		pParams->pReadNextChar = ReadNextChar;
		
		if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Obj.Offset - 1, SEEK_SET) )
		{
			snprintf(pParams->szError, 8192, "Errore LoadFirstBlock(%s) fseek\n", pszFunctionName);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore LoadFirstBlock(%s) fseek\n", pszFunctionName);
			//fwprintf(pParams->fpErrors, L"Errore LoadFirstBlock(%s) fseek\n", pszFunctionName);
			retValue = 0;
			goto uscita;
		}		
	
		pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
		if ( pParams->blockLen < 3 )
		{
			snprintf(pParams->szError, 8192, "Errore LoadFirstBlock(%s): Offset oggetto n. %d errato -> %d\n", pszFunctionName, objNum, pParams->myObjsTable[objNum]->Obj.Offset);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore LoadFirstBlock(%s): Offset oggetto n. %d errato -> %d\n", pszFunctionName, objNum, pParams->myObjsTable[objNum]->Obj.Offset);
			//fwprintf(pParams->fpErrors, L"Errore LoadFirstBlock(%s): Offset oggetto n. %d errato -> %d\n", pszFunctionName, objNum, pParams->myObjsTable[objNum]->Obj.Offset);
			retValue = 0;
			goto uscita;
		}
		
		pParams->blockCurPos = 0;	
	
		c = pParams->myBlock[pParams->blockCurPos++];
		if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
		{
			snprintf(pParams->szError, 8192, "Errore LoadFirstBlock(%s): oggetto n. %d; atteso spazio, trovato '%c'\n", pszFunctionName, objNum, c);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore LoadFirstBlock(%s): oggetto n. %d; atteso spazio, trovato '%c'\n", pszFunctionName, objNum, c);
			//fwprintf(pParams->fpErrors, L"Errore LoadFirstBlock(%s): oggetto n. %d; atteso spazio, trovato '%c'\n", pszFunctionName, objNum, c);
			retValue = 0;
			goto uscita;		
		}
	}
	else if ( OBJ_TYPE_STREAM == pParams->myObjsTable[objNum]->Obj.Type )
	{	
		pParams->pReadNextChar = ReadNextCharFromStmObjStream;
		
		newStreamLength = 0;
		
		snprintf(szTemp1, 1024, "%d 0 obj\n", pParams->myObjsTable[objNum]->Obj.Number);
		lenTemp1 = strnlen(szTemp1, 1024);
		
		newStreamLength += lenTemp1;
		
		newStreamLength += pParams->myObjsTable[objNum]->Obj.StreamLength;
		
		snprintf(szTemp2, 1024, "\nendobj\n");
		lenTemp2 = strnlen(szTemp2, 1024);
		
		newStreamLength += lenTemp2;
			
		if ( newStreamLength > nBlockSize )
		{
			for ( x = 2; nBlockSize <= pParams->myObjsTable[objNum]->Obj.StreamLength; x++ )
				nBlockSize += BLOCK_SIZE;
	
			if ( NULL != pParams->myBlock )
				free(pParams->myBlock);
			
			pParams->myBlock = (unsigned char *)malloc(sizeof(unsigned char) * nBlockSize);
			if ( !(pParams->myBlock) )
			{
				snprintf(pParams->szError, 8192, "ERRORE LoadFirstBlock(%s): malloc failed for pParams->myBlock.\n\n", pszFunctionName);
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"ERROR LoadFirstBlock(%s): malloc failed for pParams->myBlock.\n\n", pszFunctionName);
				//fwprintf(pParams->fpErrors, L"ERRORE LoadFirstBlock(%s): malloc failed for pParams->myBlock.\n\n", pszFunctionName);
				retValue = 0;
				goto uscita;
			}
		}
				
		//wprintf(L"\n\nLoadFirstBlock STREAM: nBlockSize = %lu -> pParams->myObjsTable[%d]->Obj.StreamLength = %lu\n", nBlockSize, objNum, pParams->myObjsTable[objNum]->Obj.StreamLength);
		//for ( uint32_t i = 0; i < pParams->myObjsTable[objNum]->Obj.StreamLength; i++ )
		//{
		//	wprintf(L"%c", pParams->myObjsTable[objNum]->Obj.pszDecodedStream[i]);
		//}
		//wprintf(L"\nLoadFirstBlock FINE STREAM.\n\n");
		
		blockOffset = 0;
		
		memcpy(pParams->myBlock, szTemp1, lenTemp1);
		blockOffset += lenTemp1;
		
		memcpy(pParams->myBlock + blockOffset, pParams->myObjsTable[objNum]->Obj.pszDecodedStream, pParams->myObjsTable[objNum]->Obj.StreamLength);
		blockOffset += pParams->myObjsTable[objNum]->Obj.StreamLength;
		
		memcpy(pParams->myBlock + blockOffset, szTemp2, lenTemp2);
		blockOffset += lenTemp2;
		
		pParams->blockLen = blockOffset;
		
		pParams->blockCurPos = 0;
		
		//wprintf(L"\n\nLoadFirstBlock STREAM(obj num = %d): nBlockSize = %lu -> newStreamLength = %lu; blockOffset = %lu; pParams->blockLen = %d\n", objNum, nBlockSize, newStreamLength, blockOffset, pParams->blockLen);
		//for ( int i = 0; i < pParams->blockLen; i++ )
		//{
		//	wprintf(L"%c", pParams->myBlock[i]);
		//}
		//wprintf(L"\nLoadFirstBlock FINE STREAM.\n\n");
	}
	else
	{
		pParams->pReadNextChar = ReadNextChar;
		
		snprintf(pParams->szError, 8192, "ERRORE LoadFirstBlock(%s): INVALID OBJ TYPE.\n\n", pszFunctionName);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERROR LoadFirstBlock(%s): INVALID OBJ TYPE.\n\n", pszFunctionName);
		//fwprintf(pParams->fpErrors, L"ERRORE LoadFirstBlock(%s): INVALID OBJ TYPE.\n\n", pszFunctionName);
		retValue = 0;
		goto uscita;
	}
	
uscita:

	return retValue;
}

int ParseObject(Params *pParams, int objNum)
{
	int retValue = 1;
	int nInt;
	int numObjQueueContent;
	int x;
	
	int nFromPage;
	int nToPage;
	
	int idxWord;
	
	uint32_t res;
	
	int count;
	
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_TST)	
	UNUSED(count);
	#endif		
					
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseObject: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseObject: objNum non valido -> %d\n", objNum);
		//fwprintf(pParams->fpErrors, L"Errore ParseObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->bPrePageTreeExit = 0;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->nObjToParse = objNum;
	pParams->nCurrentObjNum = 0;
	
	pParams->nCountPageFound = 0;
	
	pParams->pReadNextChar = ReadNextChar;
	
	if ( !LoadFirstBlock(pParams, objNum, "ParseObject") )
	{
		retValue = 0;
		goto uscita;
	}
	
	mynumstacklist_Init( &(pParams->myNumStack) );
	pParams->ObjPageTreeRoot.Number = 0;
	pParams->ObjPageTreeRoot.Generation = 0;
	
	myintqueuelist_Init( &(pParams->myPagesQueue) );
	
	GetNextToken(pParams);
		
	pParams->nDictionaryType = DICTIONARY_TYPE_GENERIC;
	
	if ( !obj(pParams) )
	{
		retValue = 0;
		goto uscita;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)	
	wprintf(L"PDF VERSION FROM CATALOG = %s\n", pParams->szPdfVersionFromCatalog);
	wprintf(L"\nECCO IL PAGE TREE ROOT: Number -> %d, Generation -> %d\n", pParams->ObjPageTreeRoot.Number, pParams->ObjPageTreeRoot.Generation);
	#endif
		
	mynumstacklist_Free( &(pParams->myNumStack) );
	
	myobjreflist_Free(&(pParams->myXObjRefList));
	myobjreflist_Free(&(pParams->myFontsRefList));
	
	pParams->eCurrentObjType = OBJ_TYPE_GENERIC;
	
	pParams->nDictionaryType = DICTIONARY_TYPE_GENERIC;
	
	if ( !ParseNextObject(pParams, pParams->ObjPageTreeRoot.Number) )
	{
		snprintf(pParams->szError, 8192, "ERRORE!!! ParseNextObject\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE!!! ParseNextObject\n");
		//fwprintf(pParams->fpErrors, L"ERRORE!!! ParseNextObject\n");
		retValue = 0;
		goto uscita;
	}
	
	pParams->myObjsTable[pParams->nCurrentObjNum]->Obj.numObjParent = pParams->nCurrentPageParent;
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)	
	wprintf(L"ROOT PAGES PARSING -> pParams->myObjsTable[%d]->numObjParent = %d\n", pParams->nCurrentObjNum, pParams->myObjsTable[pParams->nCurrentObjNum]->Obj.genObjParent);
	#endif
	
	if ( -1 == pParams->nCurrentPageResources  )   // La pagina eredita Resources da uno dei suoi parenti.
	{		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)		
		wprintf(L"Il nodo ROOT(obj num: %d) non ha riferimenti a Resources.\n", pParams->nCurrentObjNum);
		#endif
	}
	else if ( 0 == pParams->nCurrentPageResources  ) // La pagina non ha riferimenti a Resources.
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)		
		wprintf(L"Il nodo ROOT(obj num: %d) non ha riferimenti a Resources\n", pParams->nCurrentObjNum);
		#endif
	}
	else if ( pParams->nCurrentPageResources > 0 )   // Un intero > 0 che indica il riferimento al numero dell'oggetto Resources.
	{
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentPageResources) )
		{
			snprintf(pParams->szError, 8192, "ERRORE ParseObject ROOT -> ParseDictionaryObject\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE ParseObject ROOT -> ParseDictionaryObject\n");
			//fwprintf(pParams->fpErrors, L"ERRORE ParseObject ROOT -> ParseDictionaryObject\n");
		}		
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)		
		wprintf(L"Il nodo ROOT(obj num: %d) ha un oggetto Resources indiretto.\n", pParams->nCurrentObjNum);
		wprintf(L"PAGETREE PAGES(ROOT NODE %d) PARSING -> pParams->nCurrentPageResources = %d 0 R\n", pParams->nCurrentObjNum, pParams->nCurrentPageResources);
		#endif
		
		while ( myobjreflist_Dequeue(&(pParams->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)			
			wprintf(L"\tKey = '%s' -> XObjectRef = %d 0 R\n", pParams->szTemp, pParams->nTemp);
			#endif
			myobjreflist_Enqueue( &(pParams->myObjsTable[1]->myXObjRefList), pParams->szTemp, pParams->nTemp );
		}

		while ( myobjreflist_Dequeue(&(pParams->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)			
			wprintf(L"\tKey = '%s' -> FontsObjectRef = %d 0 R\n", pParams->szTemp, pParams->nTemp);
			#endif
			myobjreflist_Enqueue( &(pParams->myObjsTable[1]->myFontsRefList), pParams->szTemp, pParams->nTemp );
		}
	}
	
	if ( pParams->bCurrentPageHasDirectResources  )   // La pagina ha un oggetto Resources diretto.
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)		
		wprintf(L"Il nodo ROOT(obj num: %d) ha un oggetto Resources diretto.\n", pParams->nCurrentObjNum);
		#endif
		while ( myobjreflist_Dequeue(&(pParams->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)			
			wprintf(L"\tKey = '%s' -> XObjectRef = %d 0 R\n", pParams->szTemp, pParams->nTemp);
			#endif
			myobjreflist_Enqueue( &(pParams->myObjsTable[1]->myXObjRefList), pParams->szTemp, pParams->nTemp );
		}
		
		while ( myobjreflist_Dequeue(&(pParams->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)			
			wprintf(L"\tKey = '%s' -> FontsObjectRef = %d 0 R\n", pParams->szTemp, pParams->nTemp);
			#endif
			myobjreflist_Enqueue( &(pParams->myObjsTable[1]->myFontsRefList), pParams->szTemp, pParams->nTemp );
		}		
	}
				
	myobjreflist_Free(&(pParams->myXObjRefList));
	myobjreflist_Free(&(pParams->myFontsRefList));	
					
	if ( pParams->nCountPagesFromPdf > 0 )
	{	
		pParams->pPagesArray = (Page*)malloc(sizeof(Page) * (pParams->nCountPagesFromPdf + 1));
		if ( NULL == pParams->pPagesArray )
		{
			snprintf(pParams->szError, 8192, "ERRORE!!! ParseObject. Impossibile allocare la memoria per l'array delle pagine per %d pagine\n", pParams->nCountPagesFromPdf);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE!!! ParseObject. Impossibile allocare la memoria per l'array delle pagine per %d pagine\n", pParams->nCountPagesFromPdf);
			retValue = 0;
			goto uscita;			
		}
		
		pParams->pPagesArray[0].numObjNumber = 0;
		pParams->pPagesArray[0].numObjContent = 0;
		
		myintqueuelist_Init(&(pParams->pPagesArray[0].queueContentsObjRefs));
		mycontentqueuelist_Init(&(pParams->pPagesArray[0].queueContens));
				
		for ( nInt = 1; nInt <= pParams->nCountPagesFromPdf; nInt++ )
		{
			pParams->pPagesArray[nInt].numObjNumber = 0;
			pParams->pPagesArray[nInt].numObjContent = 0;
			myintqueuelist_Init(&(pParams->pPagesArray[nInt].queueContentsObjRefs));
			mycontentqueuelist_Init(&(pParams->pPagesArray[nInt].queueContens));
			scopeInit(&(pParams->pPagesArray[nInt].myScopeHT_XObjRef));
			scopeInit(&(pParams->pPagesArray[nInt].myScopeHT_FontsRef));
		}
		
		while ( myintqueuelist_Dequeue(&(pParams->myPagesQueue), &nInt) )
		{			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)			
			wprintf(L"\n\nDEQUEUE Ecco ---> '%d'\n", nInt);
			#endif
	
			myobjreflist_Free(&(pParams->myXObjRefList));
			myobjreflist_Free(&(pParams->myFontsRefList));
						
			pParams->pReadNextChar = ReadNextChar;
			
			pParams->eCurrentObjType = OBJ_TYPE_GENERIC;
			if ( !ParseNextObject(pParams, nInt) )
			{
				snprintf(pParams->szError, 8192, "ERRORE!!! ParseNextObject 2\n");
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"ERRORE!!! ParseNextObject 2\n");
				//fwprintf(pParams->fpErrors, L"ERRORE!!! ParseNextObject 2\n");
				retValue = 0;
				goto uscita;
			}
			
			pParams->nCurrentPageNum = pParams->nCountPageFound;
			
			if ( OBJ_TYPE_PAGE == pParams->eCurrentObjType )
			{
				pParams->pPagesArray[pParams->nCountPageFound].numObjParent = pParams->nCurrentPageParent;
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)		
				wprintf(L"PAGETREE PAGE PARSING -> pParams->pPagesArray[%d].numObjParent    = %d\n", pParams->nCountPageFound, pParams->pPagesArray[pParams->nCountPageFound].numObjParent);				
				#endif
			}
			else if ( OBJ_TYPE_PAGES == pParams->eCurrentObjType )
			{
				pParams->myObjsTable[pParams->nCurrentObjNum]->Obj.numObjParent = pParams->nCurrentPageParent;
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
				wprintf(L"PAGETREE PAGES PARSING -> (obj num: %d) pParams->nCurrentPageParent = %d\n", pParams->nCurrentObjNum, pParams->nCurrentPageParent);
				#endif
			}
	
			if ( !(pParams->bCurrentPageHasDirectResources) && (-1 == pParams->nCurrentPageResources)  )   // La pagina(o il nodo interno) eredita Resources da uno dei suoi parenti.
			{
				if ( OBJ_TYPE_PAGE == pParams->eCurrentObjType )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)					
					wprintf(L"La pagina %d eredita Resources da uno dei suoi parenti.\n", pParams->nCountPageFound);
					#endif
					x = pParams->pPagesArray[pParams->nCountPageFound].numObjParent;
					while ( x > 0 )
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)						
						wprintf(L"\tParent -> %d\n", x);
						#endif
						
						while ( myobjreflist_Dequeue(&(pParams->myObjsTable[x]->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
						{
							scopeInsert(&(pParams->pPagesArray[pParams->nCountPageFound].myScopeHT_XObjRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096) + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
							
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
							if ( pParams->myObjsTable[x]->myXObjRefList.count <= 0 )
								scopeTraverse(&(pParams->pPagesArray[pParams->nCountPageFound].myScopeHT_XObjRef), myOnScopeTraverse, 0);
							#endif							
						}
						
						while ( myobjreflist_Dequeue(&(pParams->myObjsTable[x]->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
						{
							scopeInsert(&(pParams->pPagesArray[pParams->nCountPageFound].myScopeHT_FontsRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096) + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
							
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
							if ( pParams->myObjsTable[x]->myFontsRefList.count <= 0 )
								scopeTraverse(&(pParams->pPagesArray[pParams->nCountPageFound].myScopeHT_FontsRef), myOnScopeTraverse, 0);
							#endif							
						}						
						
						x = pParams->myObjsTable[x]->Obj.numObjParent;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
						wprintf(L"\n");
						#endif
					}
				}
				else if ( OBJ_TYPE_PAGES == pParams->eCurrentObjType )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)					
					wprintf(L"Il nodo interno(obj num: %d) eredita Resources da uno dei suoi parenti.\n", pParams->nCurrentObjNum);
					#endif
				}		
			}
			else if ( !(pParams->bCurrentPageHasDirectResources) && (0 == pParams->nCurrentPageResources)  ) // La pagina(o il nodo interno) non ha riferimenti a Resources.
			{
				if ( !pParams->bCurrentPageHasDirectResources && OBJ_TYPE_PAGE == pParams->eCurrentObjType )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)					
					wprintf(L"La pagina %d non ha riferimenti a Resources.\n", pParams->nCountPageFound);
					#endif
				}
				else if ( !pParams->bCurrentPageHasDirectResources && OBJ_TYPE_PAGES == pParams->eCurrentObjType )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
					wprintf(L"Il nodo interno(obj num: %d) non ha riferimenti a Resources\n", pParams->nCurrentObjNum);
					#endif
				}		
			}
			else if ( !(pParams->bCurrentPageHasDirectResources) && pParams->nCurrentPageResources > 0 )   // Un intero > 0 indica il riferimento al numero dell'oggetto Resources.
			{
				if ( !ParseDictionaryObject(pParams, pParams->nCurrentPageResources) )
				{
					snprintf(pParams->szError, 8192, "ERRORE ParseObject -> ParseDictionaryObject\n");
					myShowErrorMessage(pParams, pParams->szError, 1);
					//wprintf(L"ERRORE ParseObject -> ParseDictionaryObject\n");
					//fwprintf(pParams->fpErrors, L"ERRORE ParseObject -> ParseDictionaryObject\n");
				}		
		
				if ( OBJ_TYPE_PAGE == pParams->eCurrentObjType )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)					
					wprintf(L"La pagina %d ha un oggetto Resources indiretto.\n", pParams->nCountPageFound);
					#endif
					
					while ( myobjreflist_Dequeue(&(pParams->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
					{
						scopeInsert(&(pParams->pPagesArray[pParams->nCountPageFound].myScopeHT_XObjRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096)  + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
					}
					
					while ( myobjreflist_Dequeue(&(pParams->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
					{
						scopeInsert(&(pParams->pPagesArray[pParams->nCountPageFound].myScopeHT_FontsRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096)  + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
					}					
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
					scopeTraverse(&(pParams->pPagesArray[pParams->nCountPageFound].myScopeHT_XObjRef), myOnScopeTraverse, 0);
					scopeTraverse(&(pParams->pPagesArray[pParams->nCountPageFound].myScopeHT_FontsRef), myOnScopeTraverse, 0);
					#endif					
				}
				else if ( OBJ_TYPE_PAGES == pParams->eCurrentObjType )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)					
					wprintf(L"Il nodo interno(obj num: %d) ha un oggetto Resources indiretto.\n", pParams->nCurrentObjNum);
					wprintf(L"PAGETREE PAGES(INTERNAL NODE %d) PARSING -> pParams->nCurrentPageResources = %d 0 R\n", pParams->nCurrentObjNum, pParams->nCurrentPageResources);
					#endif
					
					while ( myobjreflist_Dequeue(&(pParams->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)						
						wprintf(L"\tKey = '%s' -> XObjectRef = %d 0 R\n", pParams->szTemp, pParams->nTemp);
						#endif
						myobjreflist_Enqueue( &(pParams->myObjsTable[pParams->nCurrentObjNum]->myXObjRefList), pParams->szTemp, pParams->nTemp );
					}
					
					while ( myobjreflist_Dequeue(&(pParams->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)						
						wprintf(L"\tKey = '%s' -> FontsObjectRef = %d 0 R\n", pParams->szTemp, pParams->nTemp);
						#endif
						myobjreflist_Enqueue( &(pParams->myObjsTable[pParams->nCurrentObjNum]->myFontsRefList), pParams->szTemp, pParams->nTemp );
					}
				}		
			}
	
			if ( pParams->bCurrentPageHasDirectResources  )   // La pagina(o il nodo interno) ha un oggetto Resources diretto.
			{
				if ( OBJ_TYPE_PAGE == pParams->eCurrentObjType )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)					
					wprintf(L"La pagina %d ha un oggetto Resources diretto.\n", pParams->nCountPageFound);
					#endif
					
					while ( myobjreflist_Dequeue(&(pParams->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
					{
						scopeInsert(&(pParams->pPagesArray[pParams->nCountPageFound].myScopeHT_XObjRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096) + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
					}
					
					while ( myobjreflist_Dequeue(&(pParams->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
					{
						scopeInsert(&(pParams->pPagesArray[pParams->nCountPageFound].myScopeHT_FontsRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096) + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
					}	
									
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
					scopeTraverse(&(pParams->pPagesArray[pParams->nCountPageFound].myScopeHT_XObjRef), myOnScopeTraverse, 0);
					scopeTraverse(&(pParams->pPagesArray[pParams->nCountPageFound].myScopeHT_FontsRef), myOnScopeTraverse, 0);
					#endif
				}
				else if ( OBJ_TYPE_PAGES == pParams->eCurrentObjType )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)					
					wprintf(L"Il nodo interno(obj num: %d) ha un oggetto Resources diretto.\n", pParams->nCurrentObjNum);
					#endif
					while ( myobjreflist_Dequeue(&(pParams->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
						wprintf(L"\tKey = '%s' -> XObjectRef = %d 0 R\n", pParams->szTemp, pParams->nTemp);
						#endif
						myobjreflist_Enqueue( &(pParams->myObjsTable[pParams->nCurrentObjNum]->myXObjRefList), pParams->szTemp, pParams->nTemp );
					}
					
					while ( myobjreflist_Dequeue(&(pParams->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
						wprintf(L"\tKey = '%s' -> FontsObjectRef = %d 0 R\n", pParams->szTemp, pParams->nTemp);
						#endif
						myobjreflist_Enqueue( &(pParams->myObjsTable[pParams->nCurrentObjNum]->myFontsRefList), pParams->szTemp, pParams->nTemp );
					}					
				}		
			}

			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)			
			wprintf(L"\n");	
			#endif

			myobjreflist_Free(&(pParams->myXObjRefList));
			myobjreflist_Free(&(pParams->myFontsRefList));
		}
	}
								
	if ( pParams->fromPage <= 0 )
		nFromPage = 1;
	else
		nFromPage = pParams->fromPage;
				
	if ( pParams->toPage <= 0 )
		nToPage = pParams->nCountPageFound;
	else
		nToPage = pParams->toPage;
		
	if ( nToPage > pParams->nCountPageFound )
		nToPage = pParams->nCountPageFound;
						
	for ( nInt = nFromPage; nInt <= nToPage; nInt++ )
	{		
		pParams->nCurrentPageNum = nInt;
							
		//pParams->pCurrentEncodingArray = &(pParams->aUtf8CharSet[0]);
		pParams->pCurrentEncodingArray = &(pParams->aSTD_CharSet[0]);
		//pParams->pCurrentEncodingArray = &(pParams->aWIN_CharSet[0]);
		//pParams->pCurrentEncodingArray = &(pParams->aMAC_CharSet[0]);
		//pParams->pCurrentEncodingArray = &(pParams->aPDF_CharSet[0]);			
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)		
		wprintf(L"PAGINA %d -> Obj Number = %d\n", nInt, pParams->pPagesArray[nInt].numObjNumber);
		#endif
		
		if ( pParams->pPagesArray[nInt].numObjContent <= 0 && pParams->pPagesArray[nInt].queueContentsObjRefs.count <= 0 )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)			
			wprintf(L"\t****************************\n");
			wprintf(L"\tBLANK PAGE\n");
			wprintf(L"\t****************************\n\n");
			#endif
			continue;
		}
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
		wprintf(L"\tContents -> { ");
		#endif
		if ( pParams->pPagesArray[nInt].numObjContent > 0 )
		{
			if ( !ParseStreamObject(pParams, pParams->pPagesArray[nInt].numObjContent) )
			{
				snprintf(pParams->szError, 8192, "ERRORE ParseStreamObject streamobj number %d\n", pParams->pPagesArray[nInt].numObjContent);
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"ERRORE ParseStreamObject streamobj number %d\n", pParams->pPagesArray[nInt].numObjContent);
				//fwprintf(pParams->fpErrors, L"ERRORE ParseStreamObject streamobj number %d\n", pParams->pPagesArray[nInt].numObjContent);
				
				snprintf(pParams->szError, 8192, "ECCO L'OGGETTO X:\n");
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"ECCO L'OGGETTO X:\n");
				//fwprintf(pParams->fpErrors, L"ECCO L'OGGETTO X:\n");
				
				PrintThisObject(pParams, pParams->pPagesArray[nInt].numObjContent, 0, 0, pParams->fpErrors);
				
				snprintf(pParams->szError, 8192, "FINE OGGETTO X:\n");
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"FINE OGGETTO X:\n");
				//fwprintf(pParams->fpErrors, L"FINE OGGETTO X:\n");
				
				retValue = 0;
				goto uscita;
			}
		}
		else
		{
			while ( myintqueuelist_Dequeue(&(pParams->pPagesArray[nInt].queueContentsObjRefs), &numObjQueueContent) )
			{				
				if ( !ParseStreamObject(pParams, numObjQueueContent) )
				{
					snprintf(pParams->szError, 8192, "ERRORE ParseStreamObject streamobj number %d\n", numObjQueueContent);
					myShowErrorMessage(pParams, pParams->szError, 1);
					//wprintf(L"ERRORE ParseStreamObject streamobj number %d\n", numObjQueueContent);
					//fwprintf(pParams->fpErrors, L"ERRORE ParseStreamObject streamobj number %d\n", numObjQueueContent);
					
					snprintf(pParams->szError, 8192, "ECCO L'OGGETTO Y:\n");
					myShowErrorMessage(pParams, pParams->szError, 1);
					//wprintf(L"ECCO L'OGGETTO Y:\n");
					//fwprintf(pParams->fpErrors, L"ECCO L'OGGETTO Y:\n");
					
					PrintThisObject(pParams, numObjQueueContent, 0, 0, pParams->fpErrors);
					
					snprintf(pParams->szError, 8192, "FINE OGGETTO Y:\n");
					myShowErrorMessage(pParams, pParams->szError, 1);
					//wprintf(L"FINE OGGETTO Y:\n");
					//fwprintf(pParams->fpErrors, L"FINE OGGETTO Y:\n");
									
					retValue = 0;
					goto uscita;
				}
			}			
		}

		tstInit(&(pParams->myTST));
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintPageNum)
		wprintf(L"\nPAGINA %d ------------------------------------------------------------------------------------------------------------------\n", nInt);
		#endif		
		
		if ( pParams->szFilePdf[0] != '\0' )
		{
			if ( pParams->szOutputFile[0] != '\0' )
				fwprintf(pParams->fpOutput, L"\n\nTEXT EXTRACTED FROM PAGE %d:\n\n", nInt);
			else
				wprintf(L"\n\nTEXT EXTRACTED FROM PAGE %d:\n\n", nInt);
		}
				
		ManageContent(pParams, nInt);
				
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)		
		wprintf(L"}\n\n");
		#endif
		
		myintqueuelist_Free(&(pParams->pPagesArray[nInt].queueContentsObjRefs));
		mycontentqueuelist_Free(&(pParams->pPagesArray[nInt].queueContens));
		scopeFree(&(pParams->pPagesArray[nInt].myScopeHT_XObjRef));
		scopeFree(&(pParams->pPagesArray[nInt].myScopeHT_FontsRef));
	
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_TST)
			wprintf(L"\nTERNARY SEARCH TREE (Page %d) -> TRAVERSE START\n", nInt);
			count = tstTraverseRecursive(pParams->myTST.pRoot, OnTraverseTST, 0);
			//count = tstTraverseDescRecursive(pParams->myTST.pRoot, OnTraverseTST, 0);
			wprintf(L"\nTERNARY SEARCH TREE (Page %d) -> TRAVERSE END\n", nInt);
			wprintf(L"TERNARY SEARCH TREE (Page %d) -> TRAVERSE COUNT = %u\n", nInt, count);
		#endif
		
		for ( idxWord = 0; idxWord < pParams->countWordsToSearch; idxWord++ )
		{
			res = tstSearchRecursive(pParams->myTST.pRoot, pParams->pWordsToSearchArray[idxWord], NULL, NULL);						
			if ( res )
			{				
				if ( pParams->szOutputFile[0] != '\0' )
				{
					fwprintf(pParams->fpOutput, L"\tKey '%ls' found on page %d\n", pParams->pWordsToSearchArray[idxWord], nInt);
				}
				else
				{
					wprintf(L"\tKey '%ls' found on page %d\n", pParams->pWordsToSearchArray[idxWord], nInt);
				}
			}
		}
		
		if ( NULL != pParams->myTST.pRoot )
			tstFreeRecursive(&(pParams->myTST), pParams->myTST.pRoot);			
	}
	
uscita:

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	//for ( nInt = 1; nInt <= pParams->nCountPageFound; nInt++ )
	for ( nInt = 1; nInt <= pParams->nCountPagesFromPdf; nInt++ )
	{
		myintqueuelist_Free(&(pParams->pPagesArray[nInt].queueContentsObjRefs));
		mycontentqueuelist_Free(&(pParams->pPagesArray[nInt].queueContens));
		scopeFree(&(pParams->pPagesArray[nInt].myScopeHT_XObjRef));
		scopeFree(&(pParams->pPagesArray[nInt].myScopeHT_FontsRef));
	}
	
	for ( nInt = 0; nInt < pParams->myPdfTrailer.Size; nInt++ )
	{
		myobjreflist_Free(&(pParams->myObjsTable[nInt]->myXObjRefList));
		myobjreflist_Free(&(pParams->myObjsTable[nInt]->myFontsRefList));
		
		if ( NULL != pParams->myObjsTable[nInt]->Obj.pszDecodedStream )
		{
			free(pParams->myObjsTable[nInt]->Obj.pszDecodedStream);
			pParams->myObjsTable[nInt]->Obj.pszDecodedStream = NULL;
		}
		
		free(pParams->myObjsTable[nInt]);
		pParams->myObjsTable[nInt] = NULL;
	}	
	
	myintqueuelist_Free(&(pParams->myPagesQueue));
	
	myobjreflist_Free(&(pParams->myXObjRefList));
	myobjreflist_Free(&(pParams->myFontsRefList));
	
	if ( NULL != pParams->myTST.pRoot )
		tstFreeRecursive(&(pParams->myTST), pParams->myTST.pRoot);

	if ( 0 == retValue )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}		
	}
		
	return retValue;
}

int OnTraverseTST(const wchar_t* key, void* data, uint32_t dataSize)
{			
	if ( NULL != data )
	{
		#if defined(_WIN64) || defined(_WIN32)
		wchar_t *pWideCharString = NULL;
		wprintf(L"KEY = '%ls' <-> DATA = '%ls' dataSize = %d\n", (wchar_t*)key, (wchar_t*)pWideCharString, dataSize);
		#else
		wprintf(L"KEY = '%ls' <->  DATA = '%s' dataSize = %d\n", (wchar_t*)key, (char*)data, dataSize);
		#endif
	}
	else
	{
		#if defined(_WIN64) || defined(_WIN32)
		wprintf(L"KEY = '%ls' <-> DATA = NULL\n", (wchar_t*)key);	
		#else
		wprintf(L"KEY = '%ls' <-> DATA ) NULL\n", (wchar_t*)key);
		#endif
	}
				
	return 1;
}

int PrintThisObject(Params *pParams, int objNum, int bDecodeStream, int nPageNumber, FILE* fpOutput)
{
	int retValue = 1;
	unsigned char c;
	int k;
	int j;
	int y;
	
	uint32_t uStreamOffset;
	uint32_t uStreamLength;
	int bStreamState;
	int bStreamAlreadyHandled;
	
	unsigned char *pszDecodedStream = NULL;
	unsigned long DecodedStreamSize = 0; 
	
	UNUSED(nPageNumber);
			
	unsigned char szTemp[21];
	unsigned char szTempStream[21];
			
	//if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	if ( objNum < 1 || (uint32_t)objNum >= pParams->nObjsTableSizeFromPrescanFile )
	{
		retValue = 0;
		if ( NULL == fpOutput )
		{
			wprintf(L"Errore PrintThisObject: objNum non valido -> %d; \n", objNum);
		}
		else
		{
			snprintf(pParams->szError, 8192, "Errore PrintThisObject: objNum non valido -> %d\n", objNum);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//fwprintf(fpOutput, L"Errore PrintThisObject: objNum non valido -> %d\n", objNum);
		}
		goto uscita;
	}
	
	pParams->nObjToParse = objNum;
		
	if ( !LoadFirstBlock(pParams, objNum, "PrintThisObject") )
	{
		retValue = 0;
		goto uscita;
	}
		
	pParams->blockCurPos = 0;
		
	uStreamOffset = pParams->myObjsTable[objNum]->Obj.StreamOffset;
	uStreamLength = pParams->myObjsTable[objNum]->Obj.StreamLength;
		
	if ( NULL == fpOutput )
	{
		wprintf(L"\n***** INIZIO OBJ(%d) = \n", objNum);
		wprintf(L"\tOffset = %lu, StreamOffset = %lu, StreamLength = %lu\n", pParams->myObjsTable[objNum]->Obj.Offset, uStreamOffset, uStreamLength);
	}
	else
	{
		fwprintf(fpOutput, L"\n***** INIZIO OBJ(%d) = \n", objNum);
		fwprintf(fpOutput, L"\tOffset = %lu, StreamOffset = %lu, StreamLength = %lu\n", pParams->myObjsTable[objNum]->Obj.Offset, uStreamOffset, uStreamLength);
	}
	
	k = 0;
	szTempStream[0] = '\0';
	y = 0;
	szTemp[0] = '\0';
	j = 0;
	bStreamState = 0;
	bStreamAlreadyHandled = 0;
	while ( pParams->blockLen > 0 )
	{
		if ( OBJ_TYPE_STREAM == pParams->myObjsTable[objNum]->Obj.Type && k >= pParams->blockLen )
			goto uscita;
		
		c = pParams->myBlock[k];
			
		if ( y > 0 && !bStreamAlreadyHandled )
		{
			if ( y < 6 )
			{
				szTempStream[y++] = c;
			}
			else
			{				
				if ( 's' == szTempStream[0] && 't' == szTempStream[1] && 'r' == szTempStream[2] && 'e' == szTempStream[3] && 'a' == szTempStream[4] && 'm' == szTempStream[5] )
				{
					bStreamState = 1;
					y = 0;
					szTempStream[0] = '\0';
					
					k++;
					if ( k >= pParams->blockLen )
					{
						pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
						k = 0;
					}
					continue;
				}
				else
				{
					y = 0;
					szTempStream[0] = '\0';
				}				
			}
		}
		
		if ( j > 0 )
		{
			if ( j < 6 )
			{
				szTemp[j++] = c;				
			}
			else
			{				
				if ( 'e' == szTemp[0] && 'n' == szTemp[1] && 'd' == szTemp[2] && 'o' == szTemp[3] && 'b' == szTemp[4] && 'j' == szTemp[5] )
				{
					break;
				}			
				else
				{
					j = 0;
					szTemp[0] = '\0';
				}
			}
		}
		
		if ( c == '\0' )
		{
			if ( NULL == fpOutput )
				wprintf(L"\\0");
			else
				fwprintf(fpOutput, L"\\0");
		}
		else if ( bStreamState && (c != '\n' && c != '\r') )
		{
			if ( fseek(pParams->fp, uStreamOffset + uStreamLength, SEEK_SET) != 0 )
			{
				snprintf(pParams->szError, 8192, "ERRORE PrintThisObject: fseek\n");
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"ERRORE PrintThisObject: fseek\n");
				//fwprintf(pParams->fpErrors, L"ERRORE PrintThisObject: fseek\n");
				retValue = 0;
				goto uscita;		
			}
	
			pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
			if ( pParams->blockLen == 0 )
			{
				retValue = 0;
				goto uscita;		
			}
			k = 0;
			pParams->blockCurPos = 0;
			
			bStreamState = 0;
			
			bStreamAlreadyHandled = 1;
			
			continue;
		}
		else if ( 's' == c )
		{
			y = 0;
			szTempStream[y++] = 's';
						
			if ( NULL == fpOutput )
				wprintf(L"s");
			else
				fwprintf(fpOutput, L"s");
		}
		else if ( 'e' == c )
		{
			j = 0;
			szTemp[j++] = 'e';
			if ( NULL == fpOutput )
				wprintf(L"e");
			else
				fwprintf(fpOutput, L"e");
		}
		else
		{
			if ( NULL == fpOutput )
				wprintf(L"%c", c);
			else
				fwprintf(fpOutput, L"%c", c);
		}		
		
		k++;
		if ( k >= pParams->blockLen )
		{
			pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
			k = 0;
		}
	}
			
	if ( bDecodeStream )
	{		
		if ( NULL == fpOutput )
			wprintf(L"\nSTAMPO LO STREAM *********************:\n");
		else
			fwprintf(fpOutput, L"\nSTAMPO LO STREAM *********************:\n");
		
		if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Obj.Offset, SEEK_SET) != 0 )
		{
			snprintf(pParams->szError, 8192, "ERRORE PrintThisObject: fseek\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE PrintThisObject: fseek\n");
			//fwprintf(pParams->fpErrors, L"ERRORE PrintThisObject: fseek\n");
			retValue = 0;
			goto uscita;		
		}
		
		pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
		pParams->blockCurPos = 0;
		
		pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Obj.Offset;
		
		mynumstacklist_Init( &(pParams->myNumStack) );
		
		GetNextToken(pParams);
	
		pParams->bStreamStateToUnicode = 0;
		if ( !contentobj(pParams) )
		{
			if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
			{
				if ( NULL != pParams->myToken.Value.vString )
				{
					free(pParams->myToken.Value.vString);
					pParams->myToken.Value.vString = NULL;
				}
			}
			snprintf(pParams->szError, 8192, "\n\nERRORE PrintThisObject -> contentobj: objNum = %d; pParams->myObjsTable[%d]->Obj.Offset = %d\n\n", objNum, objNum, pParams->myObjsTable[objNum]->Obj.Offset);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"\n\nERRORE PrintThisObject -> contentobj: objNum = %d; pParams->myObjsTable[%d]->Obj.Offset = %lu\n\n", objNum, objNum, pParams->myObjsTable[objNum]->Obj.Offset);
			retValue = 0;
			goto uscita;
		}
		
		uStreamOffset = pParams->myObjsTable[objNum]->Obj.StreamOffset;
		uStreamLength = pParams->myObjsTable[objNum]->Obj.StreamLength;
			
		mynumstacklist_Free( &(pParams->myNumStack) );
		
		pszDecodedStream = getDecodedStream(pParams, &DecodedStreamSize, NULL);
		if ( NULL == pszDecodedStream )
		{
			snprintf(pParams->szError, 8192, "ERRORE PrintThisObject: getDecodedStream\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE PrintThisObject: getDecodedStream\n");
			//fwprintf(pParams->fpErrors, L"ERRORE PrintThisObject: getDecodedStream\n");
			retValue = 0;
			goto uscita;
		}

			
			
		if ( NULL == fpOutput )
			wprintf(L"\nINIZIO STREAM DECODIFICATO DOPO myInflate:\n");
		else
			fwprintf(fpOutput, L"\nINIZIO STREAM DECODIFICATO DOPO myInflate:\n");
			
		for ( unsigned long int k = 0; k < DecodedStreamSize; k++ )
		{
			if ( pszDecodedStream[k] == '\0' )
			{
				if ( NULL == fpOutput )
					wprintf(L"<00>");
				else
					fwprintf(fpOutput, L"<00>");
			}
			else if( pszDecodedStream[k] < 32 || pszDecodedStream[k] >= 127 )
			{
				if ( '\n' == pszDecodedStream[k] || '\r' == pszDecodedStream[k] )
				{
					if ( NULL == fpOutput )
						wprintf(L"%c", pszDecodedStream[k]);
					else
						fwprintf(fpOutput, L"%c", pszDecodedStream[k]);
				}
				else
				{
					if ( NULL == fpOutput )
						wprintf(L"<%2X>", pszDecodedStream[k]);
					else
						fwprintf(fpOutput, L"<%2X>", pszDecodedStream[k]);
				}
			}
			else
			{
				if ( NULL == fpOutput )
					wprintf(L"%c", pszDecodedStream[k]);
				else
					fwprintf(fpOutput, L"%c", pszDecodedStream[k]);
			}
		}
		
		if ( NULL == fpOutput )
			wprintf(L"\nFINE STREAM DECODIFICATO DOPO myInflate.\n\n");
		else
			fwprintf(fpOutput, L"\nFINE STREAM DECODIFICATO DOPO myInflate.\n\n");
			
			
			
		wprintf(L"\nFINE DELLO STREAM *********************:\n");
	}
			
	if ( NULL == fpOutput )
		wprintf(L"\n***** FINE OBJ(%d)\n\n", objNum);
	else
		fwprintf(fpOutput, L"\n***** FINE OBJ(%d)\n\n", objNum);
	
uscita:

	if ( NULL != pszDecodedStream )
	{
		free(pszDecodedStream);
		pszDecodedStream = NULL;
	}
	
	return retValue;
}

int ParseNextObject(Params *pParams, int objNum)
{
	int retValue = 1;
	
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseNextObject: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseNextObject: objNum non valido -> %d\n", objNum);
		//fwprintf(pParams->fpErrors, L"Errore ParseNextObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->nObjToParse = objNum;
	
	pParams->pReadNextChar = ReadNextChar;
	
	if ( !LoadFirstBlock(pParams, objNum, "ParseNextObject") )
	{
		retValue = 0;
		goto uscita;
	}
			
	mynumstacklist_Init( &(pParams->myNumStack) );
			
	GetNextToken(pParams);
				
	if ( !prepagetree(pParams) )
	{
		retValue = 0;
						
		goto uscita;
	}
		
	pParams->bPrePageTreeExit = 1;
		
	mynumstacklist_Free( &(pParams->myNumStack) );
	
	// --------------------------------------------------------------------------------------------------------------------------------------
			
	if ( !LoadFirstBlock(pParams, objNum, "ParseNextObject") )
	{
		retValue = 0;
		goto uscita;
	}
		
	GetNextToken(pParams);
	
	if ( !pagetree(pParams) )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}		
		retValue = 0;
				
		goto uscita;
	}
					
uscita:
	pParams->bPrePageTreeExit = 0;

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	return retValue;
}

unsigned char * getDecodedStream(Params *pParams, unsigned long int *pDecodedStreamSize, MyContent_t *pContent)
{	
	unsigned char *pszEncodedStream = NULL;
	unsigned long int EncodedStreamSize = 0;
	
	unsigned char *pszDecodedStream = NULL;	
	
	size_t bytesRead = 0;
		
	if ( NULL == pContent )
		pContent = &(pParams->CurrentContent);
		
	if ( 0 == pContent->Offset )
	{
		snprintf(pParams->szError, 8192, "ERRORE getDecodedStream: pContent->Offset = 0\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE getDecodedStream: pContent->Offset = 0\n");
		//fwprintf(pParams->fpErrors, L"ERRORE getDecodedStream: pContent->Offset = 0\n");
		goto uscita;
	}
	
	EncodedStreamSize = pContent->LengthFromPdf * sizeof(unsigned char) + sizeof(unsigned char);
	
	pszEncodedStream = (unsigned char*)malloc(EncodedStreamSize);
	if ( NULL == pszEncodedStream )
	{
		snprintf(pParams->szError, 8192, "ERRORE getDecodedStream: impossibile allocare %lu byte per leggere lo stream\n", EncodedStreamSize);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE getDecodedStream: impossibile allocare %lu byte per leggere lo stream\n", EncodedStreamSize);
		//fwprintf(pParams->fpErrors, L"ERRORE getDecodedStream: impossibile allocare %lu byte per leggere lo stream\n", EncodedStreamSize);
		goto uscita;
	}

	fseek(pParams->fp, pContent->Offset, SEEK_SET);
						
	bytesRead = fread(pszEncodedStream, 1, pContent->LengthFromPdf, pParams->fp);
	if ( bytesRead < pContent->LengthFromPdf )
	{
		snprintf(pParams->szError, 8192, "\nERRORE getDecodedStream: fread ha letto %lu byte quando il pdf ne specifica, invece, %lu\n", bytesRead, pContent->LengthFromPdf);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"\nERRORE getDecodedStream: fread ha letto %lu byte quando il pdf ne specifica, invece, %lu\n", bytesRead, pContent->LengthFromPdf);
		//fwprintf(pParams->fpErrors, L"\nERRORE getDecodedStream: fread ha letto %lu byte quando il pdf ne specifica, invece, %lu\n", bytesRead, pContent->LengthFromPdf);
		goto uscita;
	}
	
	pszDecodedStream = DecodeStream(pszEncodedStream, EncodedStreamSize, pContent, pParams->fpErrors, pszDecodedStream, pDecodedStreamSize);
	if ( NULL ==  pszDecodedStream )
	{
		snprintf(pParams->szError, 8192, "\nERRORE getDecodedStream: DecodeStream failed.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"\nERRORE getDecodedStream: DecodeStream failed.\n");
		//fwprintf(pParams->fpErrors, L"\nERRORE getDecodedStream: DecodeStream failed.\n");
		goto uscita;
	}
	
uscita:

	if ( NULL != pszEncodedStream )
	{
		free(pszEncodedStream);
		pszEncodedStream = NULL;
	}
	
	return pszDecodedStream;
}

int CheckObjectType(Params *pParams, int objNum)
{	
	int retValue = 1;
			
	//if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	if ( objNum < 1 || (uint32_t)objNum >= pParams->nObjsTableSizeFromPrescanFile )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore CheckObjectType: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//fwprintf(pParams->fpErrors, L"Errore CheckObjectType: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
		
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Obj.Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Obj.Offset, SEEK_SET) )
	{
		snprintf(pParams->szError, 8192, "Errore CheckObjectType fseek\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore CheckObjectType fseek\n");
		//fwprintf(pParams->fpErrors, L"Errore CheckObjectType fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
			
	pParams->blockCurPos = 0;
		
	mynumstacklist_Init( &(pParams->myNumStack) );
		
	GetNextToken(pParams);
	
	if ( !cot(pParams) )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}		
		retValue = 0;
		goto uscita;
	}
				
uscita:

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	return retValue;
}

int ParseStmObj(Params *pParams, int objNum)
{	
	int retValue = 1;
			
	//if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	if ( objNum < 1 || (uint32_t)objNum >= pParams->nObjsTableSizeFromPrescanFile )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseStmObj: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//fwprintf(pParams->fpErrors, L"Errore ParseStmObj: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	//if ( OBJ_TYPE_STREAM != pParams->myObjsTable[objNum]->Obj.Type )
	//{
	//	fwprintf(pParams->fpErrors, L"Errore ParseStmObj: l'oggetto %d non è un oggetto Object Stream.\n", objNum);
	//	retValue = 0;
	//	goto uscita;
	//}
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Obj.Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Obj.Offset, SEEK_SET) )
	{
		snprintf(pParams->szError, 8192, "Errore ParseStmObj fseek\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseStmObj fseek\n");
		//fwprintf(pParams->fpErrors, L"Errore ParseStmObj fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
			
	pParams->blockCurPos = 0;
		
	mynumstacklist_Init( &(pParams->myNumStack) );
		
	GetNextToken(pParams);
	
	if ( !stmobj(pParams) )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}		
		retValue = 0;
		goto uscita;
	}
			
uscita:

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	return retValue;
}

int ParseTrailerXRefStreamObject(Params *pParams)
{	
	int retValue = 1;
	
	unsigned char *pszDecodedStream = NULL;	
	unsigned long int DecodedStreamSize = 0;
			
	int nObjNum;
	int nNumberOfEntries;
	int nSubSectionNum;
	int nCountEntries;
	
	unsigned long int y;
	unsigned long int x;
	
	uint32_t myDecimalValue1 = 0x00000000;
	uint32_t myDecimalValue2 = 0x00000000;
	uint32_t myDecimalValue3 = 0x00000000;
		
	pParams->myPdfTrailer.Prev = 0;
	pParams->myPdfTrailer.Root.Number = 0;
	pParams->myPdfTrailer.Root.Generation = 0;
	pParams->myPdfTrailer.Size = 0;	
			
	pParams->nNumBytesReadFromCurrentStream = pParams->offsetXRefObjStream;
	
	pParams->nObjToParse = pParams->nXRefStreamObjNum;
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)pParams->nXRefStreamObjNum;
		
	if ( fseek(pParams->fp, pParams->offsetXRefObjStream, SEEK_SET) )
	{
		snprintf(pParams->szError, 8192, "Errore ParseTrailerXRefStreamObject fseek\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseTrailerXRefStreamObject fseek\n");
		//fwprintf(pParams->fpErrors, L"Errore ParseTrailerXRefStreamObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	pParams->blockCurPos = 0;
	
	if ( pParams->blockLen < 3 )
	{
		snprintf(pParams->szError, 8192, "Errore ParseTrailerXRefStreamObject: Offset oggetto n. %d errato -> %d\n", pParams->nXRefStreamObjNum, pParams->offsetXRefObjStream);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseTrailerXRefStreamObject: Offset oggetto n. %d errato -> %d\n", pParams->nXRefStreamObjNum, pParams->offsetXRefObjStream);
		//fwprintf(pParams->fpErrors, L"Errore ParseTrailerXRefStreamObject: Offset oggetto n. %d errato -> %d\n", pParams->nXRefStreamObjNum, pParams->offsetXRefObjStream);
		retValue = 0;
		goto uscita;
	}
				
	myintqueuelist_Init( &(pParams->queueTrailerIndex) );
	myintqueuelist_Init( &(pParams->queueTrailerW) );

	pParams->myPdfTrailer.pIndexArray = NULL;
	pParams->myPdfTrailer.indexArraySize = 0;
	pParams->trailerW1 = -1;
	pParams->trailerW2 = -1;
	pParams->trailerW3 = -1;
		
	GetNextToken(pParams);
	
	pParams->bIsLastTrailer = 1;
	
	if ( !xrefstream_obj(pParams) )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}
		snprintf(pParams->szError, 8192, "Errore ParseTrailerXRefStreamObject: xrefstream_obj failed\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseTrailerXRefStreamObject: xrefstream_obj failed\n");
		//fwprintf(pParams->fpErrors, L"Errore ParseTrailerXRefStreamObject: xrefstream_obj failed\n");
		retValue = 0;
		goto uscita;
	}
	
	// ************************************************************************************************************************************************************
	
	pszDecodedStream = getDecodedStream(pParams, &DecodedStreamSize, NULL);
	if ( NULL ==  pszDecodedStream )
	{
		snprintf(pParams->szError, 8192, "Errore ParseTrailerXRefStreamObject: getDecodedStream failed\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseTrailerXRefStreamObject: getDecodedStream failed\n");
		//fwprintf(pParams->fpErrors, L"Errore ParseTrailerXRefStreamObject: getDecodedStream failed\n");
		retValue = 0;
		goto uscita;
	}
	
	nCountEntries = 0;
	nSubSectionNum = 0;
	nObjNum = pParams->myPdfTrailer.pIndexArray[nSubSectionNum]->FirstObjNumber;
	nNumberOfEntries = pParams->myPdfTrailer.pIndexArray[nSubSectionNum]->NumberOfEntries;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
	wprintf(L"\nParseTrailerXRefStreamObject: INIZIO STREAM DECODIFICATO:\n");
	wprintf(L"SubSection %d -> NumberOfEntries = %d, FirstObjNumber = %d\n", nSubSectionNum, nNumberOfEntries, nObjNum);
	#endif
	
	for ( y = 0; y < DecodedStreamSize; y++ )
	{	
		myDecimalValue1 = getDecimalValue(pParams->nThisMachineEndianness, &(pszDecodedStream[y]), pParams->trailerW1);
		
		for ( int z = 0; z < pParams->trailerW1; z++ )
		{
			if ( y >= DecodedStreamSize )
				break;
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
			if ( pszDecodedStream[y] <= 0xF )
			{
				wprintf(L"0");
				wprintf(L"%X", pszDecodedStream[y]);
			}
			else
			{
				wprintf(L"%X", pszDecodedStream[y]);
			}
			#endif
			y++;
		}
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
		wprintf(L" ");
		#endif
		
		myDecimalValue2 = getDecimalValue(pParams->nThisMachineEndianness, &(pszDecodedStream[y]), pParams->trailerW2);
		
		for ( int z = 0; z < pParams->trailerW2; z++ )
		{
			if ( y >= DecodedStreamSize )
				break;
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
			if ( pszDecodedStream[y] <= 0xF )
			{
				wprintf(L"0");
				wprintf(L"%X", pszDecodedStream[y]);
			}
			else
			{
				wprintf(L"%X", pszDecodedStream[y]);
			}
			#endif
			y++;
		}			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
		wprintf(L" ");
		#endif
		
		myDecimalValue3 = getDecimalValue(pParams->nThisMachineEndianness, &(pszDecodedStream[y]), pParams->trailerW3);
		
		if ( OBJ_TYPE_UNKNOWN == pParams->myObjsTable[nObjNum]->Obj.Type )
		{
			if ( 1 == myDecimalValue1 )
				pParams->myObjsTable[nObjNum]->Obj.Type = OBJ_TYPE_IN_USE;
			else if ( 2 == myDecimalValue1 )
				pParams->myObjsTable[nObjNum]->Obj.Type = OBJ_TYPE_STREAM;
			else if ( 0 == myDecimalValue1 )
				pParams->myObjsTable[nObjNum]->Obj.Type = OBJ_TYPE_FREE;
			else
				pParams->myObjsTable[nObjNum]->Obj.Type = OBJ_TYPE_RESERVED;
			pParams->myObjsTable[nObjNum]->Obj.Number = nObjNum;
			pParams->myObjsTable[nObjNum]->Obj.Generation = myDecimalValue3;
			if ( (OBJ_TYPE_IN_USE == pParams->myObjsTable[nObjNum]->Obj.Type) && (myDecimalValue2 != pParams->myObjsTable[nObjNum]->Obj.Offset) )
			{
				//fwprintf(pParams->fpErrors, L"\nWARNING: ParseTrailerXRefStream -> myDecimalValue2 = %lu differs from pParams->myObjsTable[%d]->Obj.Offset = %lu\n", myDecimalValue2, nObjNum, pParams->myObjsTable[nObjNum]->Obj.Offset);
				;
			}
			else
			{
				pParams->myObjsTable[nObjNum]->Obj.Offset = myDecimalValue2;
			}
			pParams->myObjsTable[nObjNum]->Obj.numObjParent = -1;
			myobjreflist_Init(&(pParams->myObjsTable[nObjNum]->myXObjRefList));
			myobjreflist_Init(&(pParams->myObjsTable[nObjNum]->myFontsRefList));
		}
		
		x = y;	
		for ( int z = 0; z < pParams->trailerW3; z++ )
		{
			if ( x >= DecodedStreamSize )
				break;
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
			if ( pszDecodedStream[x] <= 0xF )
			{
				wprintf(L"0");
				wprintf(L"%X", pszDecodedStream[x]);
			}
			else
			{
				wprintf(L"%X", pszDecodedStream[x]);
			}
			#endif
			//y++;   // NO PERCHÉ L'ULTIMO INCREMENTO LO EFFETTUA IL CICLO FOR; ALTRIMENTI y VIENE INCREMENTATA DI pParams->trailerW3 UNITÀ IN PIÙ E SBALLA TUTTO.
			x++;
		}
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
		wprintf(L"   (ObjNum = %5d) -> valori decimali -> ", nObjNum);
		if ( nObjNum < 10 )
			wprintf(L" ");
		wprintf(L"{[%10lu] [%10lu] [%10lu]}\n", myDecimalValue1, myDecimalValue2, myDecimalValue3);
		#endif
		
		nCountEntries++;
		if ( nCountEntries >= nNumberOfEntries )
		{
			nSubSectionNum++;
			nObjNum = pParams->myPdfTrailer.pIndexArray[nSubSectionNum]->FirstObjNumber;
			nNumberOfEntries = pParams->myPdfTrailer.pIndexArray[nSubSectionNum]->NumberOfEntries;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
			wprintf(L"SubSection %d -> NumberOfEntries = %d, FirstObjNumber = %d\n", nSubSectionNum, nNumberOfEntries, nObjNum);
			#endif
		}
		else
		{
			nObjNum++;
		}
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
	wprintf(L"ParseTrailerXRefStreamObject: FINE DECODED STREAM(DecodedStremSizeTemp = %lu).\n\n", DecodedStreamSize);
	#endif

	// ************************************************************************************************************************************************************
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
	wprintf(L"\n\npParams->myPdfTrailer.Prev = %d\n\n", pParams->myPdfTrailer.Prev);
	#endif
	
	pParams->bIsLastTrailer = 0;
	while ( pParams->myPdfTrailer.Prev > 0 )
	{		
		if ( NULL != pszDecodedStream )
		{
			free(pszDecodedStream);
			pszDecodedStream = NULL;
		}
		
		myintqueuelist_Free( &(pParams->queueTrailerIndex) );
		myintqueuelist_Free( &(pParams->queueTrailerW) );
			
		mystringqueuelist_Free(&(pParams->CurrentContent.queueFilters));	
		mydictionaryqueuelist_Free(&(pParams->CurrentContent.decodeParms));
		
		if ( NULL != pParams->myPdfTrailer.pIndexArray )
		{
			for ( int i = 0; i < pParams->myPdfTrailer.indexArraySize; i++ )
			{
				if ( NULL != pParams->myPdfTrailer.pIndexArray[i] )
				{
					free(pParams->myPdfTrailer.pIndexArray[i]);
					pParams->myPdfTrailer.pIndexArray[i] = NULL;
				}
			}
			free(pParams->myPdfTrailer.pIndexArray);
			pParams->myPdfTrailer.pIndexArray = NULL;
			pParams->myPdfTrailer.indexArraySize = 0;
		}
		pParams->trailerW1 = -1;
		pParams->trailerW2 = -1;
		pParams->trailerW3 = -1;
				
		pParams->nNumBytesReadFromCurrentStream = pParams->myPdfTrailer.Prev;
			
		if ( fseek(pParams->fp, pParams->myPdfTrailer.Prev, SEEK_SET) )
		{
			snprintf(pParams->szError, 8192, "Errore ParseTrailerXRefStreamObject fseek\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore ParseTrailerXRefStreamObject fseek\n");
			//fwprintf(pParams->fpErrors, L"Errore ParseTrailerXRefStreamObject fseek\n");
			retValue = 0;
			goto uscita;
		}		
	
		pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
		pParams->blockCurPos = 0;
					
		GetNextToken(pParams);

		if ( !xrefstream_obj(pParams) )
		{
			if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
			{
				if ( NULL != pParams->myToken.Value.vString )
				{
					free(pParams->myToken.Value.vString);
					pParams->myToken.Value.vString = NULL;
				}
			}
			snprintf(pParams->szError, 8192, "Errore ParseTrailerXRefStreamObject: xrefstream_obj failed\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore ParseTrailerXRefStreamObject: xrefstream_obj failed\n");
			//fwprintf(pParams->fpErrors, L"Errore ParseTrailerXRefStreamObject: xrefstream_obj failed\n");
			retValue = 0;
			goto uscita;
		}
		
		// ************************************************************************************************************************************************************
		
		pszDecodedStream = getDecodedStream(pParams, &DecodedStreamSize, NULL);
		if ( NULL ==  pszDecodedStream )
		{
			snprintf(pParams->szError, 8192, "Errore ParseTrailerXRefStreamObject: getDecodedStream failed\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore ParseTrailerXRefStreamObject: getDecodedStream failed\n");
			//fwprintf(pParams->fpErrors, L"Errore ParseTrailerXRefStreamObject: getDecodedStream failed\n");
			retValue = 0;
			goto uscita;
		}
						
		nCountEntries = 0;
		nSubSectionNum = 0;
		nObjNum = pParams->myPdfTrailer.pIndexArray[nSubSectionNum]->FirstObjNumber;
		nNumberOfEntries = pParams->myPdfTrailer.pIndexArray[nSubSectionNum]->NumberOfEntries;
	
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
		wprintf(L"\nParseTrailerXRefStreamObject: INIZIO STREAM DECODIFICATO:\n");	
		wprintf(L"SubSection %d -> NumberOfEntries = %d, FirstObjNumber = %d\n", nSubSectionNum, nNumberOfEntries, nObjNum);
		#endif
	
		for ( y = 0; y < DecodedStreamSize; y++ )
		{			
			myDecimalValue1 = getDecimalValue(pParams->nThisMachineEndianness, &(pszDecodedStream[y]), pParams->trailerW1);
		
			for ( int z = 0; z < pParams->trailerW1; z++ )
			{
				if ( y >= DecodedStreamSize )
					break;
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
				if ( pszDecodedStream[y] <= 0xF )
				{
					wprintf(L"0");
					wprintf(L"%X", pszDecodedStream[y]);
				}
				else
				{
					wprintf(L"%X", pszDecodedStream[y]);
				}
				#endif
				y++;
			}
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
			wprintf(L" ");
			#endif
			
			myDecimalValue2 = getDecimalValue(pParams->nThisMachineEndianness, &(pszDecodedStream[y]), pParams->trailerW2);
		
			for ( int z = 0; z < pParams->trailerW2; z++ )
			{
				if ( y >= DecodedStreamSize )
					break;
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
				if ( pszDecodedStream[y] <= 0xF )
				{
					wprintf(L"0");
					wprintf(L"%X", pszDecodedStream[y]);
				}
				else
				{
					wprintf(L"%X", pszDecodedStream[y]);
				}
				#endif
				y++;
			}
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
			wprintf(L" ");
			#endif
		
			myDecimalValue3 = getDecimalValue(pParams->nThisMachineEndianness, &(pszDecodedStream[y]), pParams->trailerW3);
						
			if ( OBJ_TYPE_UNKNOWN == pParams->myObjsTable[nObjNum]->Obj.Type )
			{				
				if ( 1 == myDecimalValue1 )
					pParams->myObjsTable[nObjNum]->Obj.Type = OBJ_TYPE_IN_USE;
				else if ( 2 == myDecimalValue1 )
					pParams->myObjsTable[nObjNum]->Obj.Type = OBJ_TYPE_STREAM;
				else if ( 0 == myDecimalValue1 )
					pParams->myObjsTable[nObjNum]->Obj.Type = OBJ_TYPE_FREE;
				else
					pParams->myObjsTable[nObjNum]->Obj.Type = OBJ_TYPE_RESERVED;
				pParams->myObjsTable[nObjNum]->Obj.Number = nObjNum;
				pParams->myObjsTable[nObjNum]->Obj.Generation = myDecimalValue3;
				if ( (OBJ_TYPE_IN_USE == pParams->myObjsTable[nObjNum]->Obj.Type) && (myDecimalValue2 != pParams->myObjsTable[nObjNum]->Obj.Offset) )
				{
					//fwprintf(pParams->fpErrors, L"\nWARNING: ParseTrailerXRefStream -> myDecimalValue2 = %lu differs from pParams->myObjsTable[%d]->Obj.Offset = %lu\n", myDecimalValue2, nObjNum, pParams->myObjsTable[nObjNum]->Obj.Offset);
					;
				}
				else
				{
					pParams->myObjsTable[nObjNum]->Obj.Offset = myDecimalValue2;
				}
				pParams->myObjsTable[nObjNum]->Obj.numObjParent = -1;
				myobjreflist_Init(&(pParams->myObjsTable[nObjNum]->myXObjRefList));
				myobjreflist_Init(&(pParams->myObjsTable[nObjNum]->myFontsRefList));
			}
					
			x = y;	
			for ( int z = 0; z < pParams->trailerW3; z++ )
			{
				if ( x >= DecodedStreamSize )
					break;
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
				if ( pszDecodedStream[x] <= 0xF )
				{
					wprintf(L"0");
					wprintf(L"%X", pszDecodedStream[x]);
				}
				else
				{
					wprintf(L"%X", pszDecodedStream[x]);
				}
				#endif
				//y++;   // NO PERCHÉ L'ULTIMO INCREMENTO LO EFFETTUA IL CICLO FOR; ALTRIMENTI y VIENE INCREMENTATA DI pParams->trailerW3 UNITÀ IN PIÙ E SBALLA TUTTO.
				x++;
			}
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
			wprintf(L"   (ObjNum = %5d) -> valori decimali -> ", nObjNum);
			wprintf(L"{[%10lu] [%10lu] [%10lu]}\n", myDecimalValue1, myDecimalValue2, myDecimalValue3);
			#endif
		
			nCountEntries++;
			if ( nCountEntries > nNumberOfEntries )
			{
				nSubSectionNum++;
				nObjNum = pParams->myPdfTrailer.pIndexArray[nSubSectionNum]->FirstObjNumber;
				nNumberOfEntries = pParams->myPdfTrailer.pIndexArray[nSubSectionNum]->NumberOfEntries;
			
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
				wprintf(L"SubSection %d -> NumberOfEntries = %d, FirstObjNumber = %d\n", nSubSectionNum, nNumberOfEntries, nObjNum);
				#endif
			}
			else
			{
				nObjNum++;
			}
		}
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
		wprintf(L"ParseTrailerXRefStreamObject: FINE DECODED STREAM(DecodedStremSizeTemp = %lu).\n\n", DecodedStreamSize);
		#endif
	}
			
uscita:
		
	if ( NULL != pszDecodedStream )
	{
		free(pszDecodedStream);
		pszDecodedStream = NULL;
	}
	
	myintqueuelist_Free( &(pParams->queueTrailerIndex) );
	myintqueuelist_Free( &(pParams->queueTrailerW) );
	
	mystringqueuelist_Free(&(pParams->CurrentContent.queueFilters));	
	mydictionaryqueuelist_Free(&(pParams->CurrentContent.decodeParms));
	
	if ( NULL != pParams->myPdfTrailer.pIndexArray )
	{
		for ( int i = 0; i < pParams->myPdfTrailer.indexArraySize; i++ )
		{
			if ( NULL != pParams->myPdfTrailer.pIndexArray[i] )
			{
				free(pParams->myPdfTrailer.pIndexArray[i]);
				pParams->myPdfTrailer.pIndexArray[i] = NULL;
			}
		}
		free(pParams->myPdfTrailer.pIndexArray);
		pParams->myPdfTrailer.pIndexArray = NULL;
	}
	pParams->myPdfTrailer.indexArraySize = 0;
			
	return retValue;
}

int ParseStreamObject(Params *pParams, int objNum)
{	
	int retValue = 1;
	unsigned char c;
		
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseStreamObject: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseStreamObject: objNum non valido -> %d\n", objNum);
		//fwprintf(pParams->fpErrors, L"Errore ParseStreamObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Obj.Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Obj.Offset - 1, SEEK_SET) )
	{
		snprintf(pParams->szError, 8192, "Errore ParseStreamObject: fseek\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseStreamObject fseek\n");
		//fwprintf(pParams->fpErrors, L"Errore ParseStreamObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
	if ( pParams->blockLen < 3 )
	{
		snprintf(pParams->szError, 8192, "Errore ParseStreamObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Obj.Offset);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseStreamObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Obj.Offset);
		//fwprintf(pParams->fpErrors, L"Errore ParseStreamObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Obj.Offset);
		retValue = 0;
		goto uscita;
	}
		
	pParams->blockCurPos = 0;
	
	c = pParams->myBlock[pParams->blockCurPos++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		snprintf(pParams->szError, 8192, "Errore ParseStreamObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseStreamObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		//fwprintf(pParams->fpErrors, L"Errore ParseStreamObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		retValue = 0;
		goto uscita;		
	}
	
	mynumstacklist_Init( &(pParams->myNumStack) );
		
	GetNextToken(pParams);
	
	pParams->bStreamStateToUnicode = 0;
	if ( !contentobj(pParams) )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}		
		retValue = 0;
		goto uscita;
	}
			
uscita:

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	return retValue;
}

int ParseStreamXObject(Params *pParams, int objNum)
{	
	int retValue = 1;
	unsigned char c;
		
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseStreamXObject: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseStreamXObject: objNum non valido -> %d\n", objNum);
		//fwprintf(pParams->fpErrors, L"Errore ParseStreamXObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Obj.Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Obj.Offset - 1, SEEK_SET) )
	{
		snprintf(pParams->szError, 8192, "Errore ParseStreamXObject fseek\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseStreamXObject fseek\n");
		//fwprintf(pParams->fpErrors, L"Errore ParseStreamXObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
	if ( pParams->blockLen < 3 )
	{
		snprintf(pParams->szError, 8192, "Errore ParseStreamXObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Obj.Offset);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseStreamXObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Obj.Offset);
		//fwprintf(pParams->fpErrors, L"Errore ParseStreamXObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Obj.Offset);
		retValue = 0;
		goto uscita;
	}
		
	pParams->blockCurPos = 0;
	
	c = pParams->myBlock[pParams->blockCurPos++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		snprintf(pParams->szError, 8192, "Errore ParseStreamXObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseStreamXObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		//fwprintf(pParams->fpErrors, L"Errore ParseStreamXObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		retValue = 0;
		goto uscita;		
	}
	
	mynumstacklist_Init( &(pParams->myNumStack) );
		
	GetNextToken(pParams);
	
	if ( !contentxobj(pParams) )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}		
		retValue = 0;
		goto uscita;
	}
			
uscita:

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	return retValue;
}

int ParseCMapStream(Params *pParams, int objNum, unsigned char *pszDecodedStream, unsigned long int DecodedStreamSize)
{
	int retValue = 1;
	
	uint32_t myValue;
	
	int base;
	int len;
	int i;
	char c;
	
	int bCodeSpaceRangeState;
	uint32_t codeSpaceRange1 = -1;
	uint32_t codeSpaceRange2 = -1;
	int nCountCodeSpaceRangeValues = 0;
	
	int bCidRangeState;
	uint32_t cidRange1 = -1;
	uint32_t cidRange2 = -1;
	uint32_t cidRange3 = -1;
	int nCountCidRangeValues = 0;
	
	int bCidCharState;
	uint32_t cidChar1 = -1;
	uint32_t cidChar2 = -1;	
	int nCountCidCharValues;
	
	int bNotdefRangeState;
	uint32_t notdefRange1 = -1;
	uint32_t notdefRange2 = -1;
	uint32_t notdefRange3 = -1;
	int nCountNotDefRangeValues = 0;
	
	int bNotdefCharState;
	uint32_t notdefChar1 = -1;
	uint32_t notdefChar2 = -1;
	int nCountNotDefCharValues = 0;
	
	int bBfCharState;
	uint32_t bfChar1 = 0;
	uint32_t bfChar2 = 0;
	int nCountBfCharValues = 0;
	
	int bBfRangeState;
	uint32_t bfRange1 = 0;
	uint32_t bfRange2 = 0;
	uint32_t bfRange3 = 0;
	int nCountBfRangeValues = 0;
	
	int lastInteger = 0;
	int idxCodeSpace = 0;
	
	int nRes;
	size_t tnameLen;
	uint32_t nDataSize;
	uint32_t bContentAlreadyProcessed;	
		
	uint32_t codepoint;
	uint16_t lead;
	uint16_t trail;
				
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->myBlockToUnicode = pszDecodedStream;
	pParams->blockLenToUnicode = DecodedStreamSize;
	pParams->blockCurPosToUnicode = 0;
	
	pParams->bStringIsDecoded = 1;
	
	pParams->pReadNextChar = ReadNextChar;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
	wprintf(L"\n\nParseCMapStream(OBJ %d) -> INIZIO STREAM DECODIFICATO DOPO myInflate:\n", objNum);
	for ( unsigned long int k = 0; k < DecodedStreamSize; k++ )
	{
		if ( pszDecodedStream[k] == '\0' )
			wprintf(L"\\0");
		else
			wprintf(L"%c", pszDecodedStream[k]);
	}
	wprintf(L"\nParseCMapStream(OBJ %d) -> FINE STREAM DECODIFICATO DOPO myInflate.\n\n", objNum);	
	#endif
		
	bCodeSpaceRangeState = 0;
	bCidRangeState = 0;
	bCidCharState = 0;
	bNotdefRangeState = 0;
	bNotdefCharState = 0;
	
	bBfCharState = 0;
	bBfRangeState = 0;
	
	codepoint = lead = trail = 0;	
	
	pParams->bHasCodeSpaceOneByte = pParams->bHasCodeSpaceTwoByte = 0;
	
	GetNextTokenFromToUnicodeStream(pParams);				
	while ( T_EOF != pParams->myToken.Type )
	{	
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
		PrintToken(&(pParams->myToken), '\t', '\0', 1);
		#endif
		
		switch ( pParams->myToken.Type )
		{
			case T_INT_LITERAL:
				lastInteger = pParams->myToken.Value.vInt;
				break;
			case T_CONTENT_OP_begincodespacerange:
				if ( NULL != pParams->pCodeSpaceRangeArray )
				{
					free(pParams->pCodeSpaceRangeArray);
					pParams->pCodeSpaceRangeArray = NULL;
				}
				pParams->pCodeSpaceRangeArray = (CodeSpaceRange_t*)malloc(sizeof(CodeSpaceRange_t) * lastInteger);
				if ( NULL == pParams->pCodeSpaceRangeArray )
				{
					snprintf(pParams->szError, 8192, "ParseCMapStream: impossibile allocare la memoria per pParams->pCodeSpaceRangeArray\n");
					myShowErrorMessage(pParams, pParams->szError, 1);
					//fwprintf(pParams->fpErrors, L"ParseCMapStream: impossibile allocare la memoria per pParams->pCodeSpaceRangeArray\n");
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
					wprintf(L"ParseCMapStream: impossibile allocare la memoria per pParams->pCodeSpaceRangeArray\n");
					#endif
					retValue = 0;
					goto uscita;
				}
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
				else
				{
					wprintf(L"\tParseCMapStream 1: ALLOCATI CORRETTAMENTE %lu BYTE per pParams->pCodeSpaceRangeArray. lastInteger = %d\n", sizeof(CodeSpaceRange_t) * lastInteger, lastInteger);
				}
				#endif				
				for ( int j = 0; j < lastInteger; j++ )
				{
					pParams->pCodeSpaceRangeArray[idxCodeSpace].nFrom = 0;
					pParams->pCodeSpaceRangeArray[idxCodeSpace].nTo = 0;
				}
				pParams->nCurrentFontCodeSpacesNum = lastInteger;
				idxCodeSpace = 0;
								
				bCodeSpaceRangeState = 1;
				codeSpaceRange1 = 0;
				codeSpaceRange2 = 0;
				nCountCodeSpaceRangeValues = 0;
				break;				
			case T_CONTENT_OP_endcodespacerange:
				bCodeSpaceRangeState = 0;
				codeSpaceRange1 = 0;
				codeSpaceRange2 = 0;
				nCountCodeSpaceRangeValues = 0;
				break;
			case T_CONTENT_OP_begincidrange:				
				bCidRangeState = 1;
				cidRange1 = 0;
				cidRange2 = 0;
				cidRange3 = 0;
				nCountCidRangeValues = 0;
				break;
			case T_CONTENT_OP_endcidrange:
				bCidRangeState = 0;
				cidRange1 = 0;
				cidRange2 = 0;
				cidRange3 = 0;
				nCountCidRangeValues = 0;
				break;
			case T_CONTENT_OP_begincidchar:				
				bCidCharState = 1;
				cidChar1 = 0;
				cidChar2 = 0;
				nCountCidCharValues = 0;
				break;
			case T_CONTENT_OP_endcidchar:
				bCidCharState = 0;
				cidChar1 = 0;
				cidChar2 = 0;
				nCountCidCharValues = 0;
				break;				
			case T_CONTENT_OP_beginnotdefrange:				
				bNotdefRangeState = 1;
				notdefRange1 = 0;
				notdefRange2 = 0;
				notdefRange3 = 0;
				nCountNotDefRangeValues = 0;
				break;
			case T_CONTENT_OP_endnotdefrange:
				bNotdefRangeState = 0;
				notdefRange1 = 0;
				notdefRange2 = 0;
				notdefRange3 = 0;
				nCountNotDefRangeValues = 0;
				break;	
			case T_CONTENT_OP_beginnotdefchar:				
				bNotdefCharState = 1;
				notdefChar1 = 0;
				notdefChar2 = 0;
				nCountNotDefCharValues = 0;
				break;
			case T_CONTENT_OP_endnotdefchar:
				bNotdefCharState = 0;
				notdefChar1 = 0;
				notdefChar2 = 0;
				nCountNotDefCharValues = 0;
				break;
			case T_CONTENT_OP_beginbfchar:				
				bBfCharState = 1;
				bfChar1 = 0;
				bfChar2 = 0;
				nCountBfCharValues = 0;
				codepoint = lead = trail = 0;
				break;
			case T_CONTENT_OP_endbfchar:
				bBfCharState = 0;
				bfChar1 = 0;
				bfChar2 = 0;
				nCountBfCharValues = 0;	
				codepoint = lead = trail = 0;
				break;
			case T_CONTENT_OP_beginbfrange:				
				bBfRangeState = 1;
				bfRange1 = 0;
				bfRange2 = 0;
				bfRange3 = 0;
				nCountBfRangeValues = 0;
				codepoint = lead = trail = 0;
				break;
			case T_CONTENT_OP_endbfrange:
				bBfRangeState = 0;
				bfRange1 = 0;
				bfRange2 = 0;
				bfRange3 = 0;
				nCountBfRangeValues = 0;
				codepoint = lead = trail = 0;
				break;
			case T_STRING_HEXADECIMAL:
				myValue = 0;
				base = 1;
				len = strnlen(pParams->myToken.Value.vString, 1024);
				for ( i = len - 1; i >= 0; i-- ) 
				{
					if ( pParams->myToken.Value.vString[i] >= '0' && pParams->myToken.Value.vString[i] <= '9' ) 
					{
						myValue += (pParams->myToken.Value.vString[i] - 48) * base;
						base = base * 16; 
					} 
					else
					{
						c = toupper(pParams->myToken.Value.vString[i]);
						if ( c >= 'A' && c <= 'F' ) 
						{ 
							myValue += (c - 55) * base; 
							base = base * 16;
						}
					}
				}
				
				lead = 0;
				trail = 0;
				codepoint = 0;

				if ( 8 == len )
				{					
					c = pParams->myToken.Value.vString[0];
					if ( toupper(c) == 'D' )
					{
						c = pParams->myToken.Value.vString[4];
						if ( toupper(c) == 'D' )
						{
							for ( i = 0; i < 4; i++ )
							{
								if ( pParams->myToken.Value.vString[i] >= '0' && pParams->myToken.Value.vString[i] <= '9' ) 
								{
									lead += (pParams->myToken.Value.vString[i] - 48) * base;
									base = base * 16; 
								} 
								else
								{
									c = toupper(pParams->myToken.Value.vString[i]);
									if ( c >= 'A' && c <= 'F' ) 
									{ 
										lead += (c - 55) * base; 
										base = base * 16;
									}
								}								
							}
							
							for ( i = 4; i < 8; i++ )
							{
								if ( pParams->myToken.Value.vString[i] >= '0' && pParams->myToken.Value.vString[i] <= '9' ) 
								{
									trail += (pParams->myToken.Value.vString[i] - 48) * base;
									base = base * 16; 
								} 
								else
								{
									c = toupper(pParams->myToken.Value.vString[i]);
									if ( c >= 'A' && c <= 'F' ) 
									{ 
										trail += (c - 55) * base;
										base = base * 16;
									}
								}								
							}
								
							codepoint = getSurrogateCodePoint(lead, trail);
						}
					}
				}
				
				//if ( codepoint > 0 )
				//	myValue = codepoint;
					
				if ( bBfRangeState && nCountBfRangeValues >= 2 )
				{											
					if ( codepoint > 0 )
						myValue = codepoint;
					
					//nCountBfRangeValues = 0;
				}			
				
				if ( bCodeSpaceRangeState )
				{						
					if ( nCountCodeSpaceRangeValues < 1 )
					{
						codeSpaceRange1 = myValue;
						nCountCodeSpaceRangeValues++;
					}
					else
					{
						codeSpaceRange2 = myValue;
						//nCountCodeSpaceRangeValues++;
						
						if ( codeSpaceRange2 > 0xFFFF )
						{
							snprintf(pParams->szError, 8192, "ParseCMapStream: VALORI ARRAY UNICODE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> codeSpaceRange1 = %u, codeSpaceRange2 = %u\n", codeSpaceRange1, codeSpaceRange2);
							myShowErrorMessage(pParams, pParams->szError, 1);
							//fwprintf(pParams->fpErrors, L"ParseCMapStream: VALORI ARRAY UNICODE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> codeSpaceRange1 = %lu, codeSpaceRange2 = %lu\n", codeSpaceRange1, codeSpaceRange2);
							
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ParseCMapStream: VALORI ARRAY UNICODE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> codeSpaceRange1 = %u, codeSpaceRange2 = %u\n", codeSpaceRange1, codeSpaceRange2);
							#endif
							
							pParams->bHasCodeSpaceOneByte = pParams->bHasCodeSpaceTwoByte = 0;
							
							//retValue = 0;
							//goto uscita;
							
							//codeSpaceRange1 = 0;
							//codeSpaceRange2 = 0;
							//nCountCodeSpaceRangeValues = 0;
							//continue;
						
						}
						else if ( codeSpaceRange2 <= 0xFF )
						{
							pParams->bHasCodeSpaceOneByte = 1;
							pParams->pCodeSpaceRangeArray[idxCodeSpace].nNumBytes = 1;
						}
						else if ( codeSpaceRange2 <= 0xFFFF )
						{
							pParams->bHasCodeSpaceTwoByte = 1;
							pParams->pCodeSpaceRangeArray[idxCodeSpace].nNumBytes = 2;
						}						
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
						else
						{
							wprintf(L"ParseCMapStream: OK!!! ARRAY UNICODE CODE SPACE RANGE -> codeSpaceRange1 = %u, codeSpaceRange2 = %u\n", codeSpaceRange1, codeSpaceRange2);	
						}	
						#endif	
						
						pParams->pCodeSpaceRangeArray[idxCodeSpace].nFrom = codeSpaceRange1;
						pParams->pCodeSpaceRangeArray[idxCodeSpace].nTo = codeSpaceRange2;
						idxCodeSpace++;

						codeSpaceRange1 = 0;
						codeSpaceRange2 = 0;
						nCountCodeSpaceRangeValues = 0;
					}
				}				
				else if ( bCidRangeState )
				{
					if ( nCountCidRangeValues < 1 )
					{
						cidRange1 = myValue;
						nCountCidRangeValues++;
					}
					else if ( nCountCidRangeValues < 2 )
					{
						cidRange2 = myValue;
						
						GetNextTokenFromToUnicodeStream(pParams);
						
						if ( T_INT_LITERAL == pParams->myToken.Type )
						{
							cidRange3 = pParams->myToken.Value.vInt;
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ERRORE ParseCMapStream 1: atteso T_INT_LITERAL\n");
							#endif
							snprintf(pParams->szError, 8192, "ERRORE ParseCMapStream 1: atteso T_INT_LITERAL\n");
							myShowErrorMessage(pParams, pParams->szError, 1);
							//fwprintf(pParams->fpErrors, L"ERRORE ParseCMapStream 1: atteso T_INT_LITERAL\n");
							retValue = 0;
							goto uscita;							
						}

						if ( cidRange1 <= 0xFFFF && cidRange2 <= 0xFFFF )
						{
							for ( uint32_t i = cidRange1; i <= cidRange2; i++ )
							{
								pParams->paCustomizedFont_CharSet[i] = cidRange3;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
								wprintf(L"ParseCMapStream: OK!!! ARRAY UNICODE -> pParams->paCustomizedFont_CharSet[%lu] = %u\n", i, cidRange3);
								#endif								
								
								cidRange3++;
							}														
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
						else
						{
							wprintf(L"ParseCMapStream: VALORI CIDRANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> cidRange1 = %u, cidRange2 = %u, cidRange3 = %u\n", cidRange1, cidRange2, cidRange3);
						}
						#endif
						
						cidRange1 = 0;
						cidRange2 = 0;
						cidRange3 = 0;
						nCountCidRangeValues = 0;
					}
				}
				else if ( bCidCharState )
				{
					if ( nCountCidCharValues < 1 )
					{
						cidChar1 = myValue;
						nCountCidCharValues++;
						
						GetNextTokenFromToUnicodeStream(pParams);
						
						if ( T_INT_LITERAL == pParams->myToken.Type )
						{
							cidChar2 = pParams->myToken.Value.vInt;
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ERRORE ParseCMapStream 2: atteso T_INT_LITERAL\n");
							#endif
							snprintf(pParams->szError, 8192, "ERRORE ParseCMapStream 2: atteso T_INT_LITERAL\n");
							myShowErrorMessage(pParams, pParams->szError, 1);
							//fwprintf(pParams->fpErrors, L"ERRORE ParseCMapStream 2: atteso T_INT_LITERAL\n");
							retValue = 0;
							goto uscita;							
						}
						
						if ( cidChar1 <= 0xFFFF )
						{
							pParams->paCustomizedFont_CharSet[cidChar1] = cidChar2;
							
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ParseCMapStream: OK!!! ARRAY UNICODE -> pParams->paCustomizedFont_CharSet[%lu] = %lu\n", cidChar1, cidChar2);
							#endif							
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
						else
						{
							wprintf(L"ParseCMapStream: VALORI CIDCHAR > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> cidChar1 = %lu, cidChar2 = %lu\n", cidChar1, cidChar2);
						}	
						#endif					
						
						cidChar1 = 0;
						cidChar2 = 0;
						nCountCidCharValues = 0;
					}
				}				
				else if ( bNotdefRangeState )
				{
					if ( nCountNotDefRangeValues < 1 )
					{
						notdefRange1 = myValue;
						nCountNotDefRangeValues++;
					}
					else if ( nCountNotDefRangeValues < 2 )
					{
						notdefRange2 = myValue;
						//nCountNotDefRangeValues++;
						
						GetNextTokenFromToUnicodeStream(pParams);
						
						if ( T_INT_LITERAL == pParams->myToken.Type )
						{
							notdefRange3 = pParams->myToken.Value.vInt;
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ERRORE ParseCMapStream 3: atteso T_INT_LITERAL\n");
							#endif
							snprintf(pParams->szError, 8192, "ERRORE ParseCMapStream 3: atteso T_INT_LITERAL\n");
							myShowErrorMessage(pParams, pParams->szError, 1);
							//fwprintf(pParams->fpErrors, L"ERRORE ParseCMapStream 3: atteso T_INT_LITERAL\n");
							retValue = 0;
							goto uscita;							
						}

						if ( notdefRange1 <= 0xFFFF && notdefRange2 <= 0xFFFF )
						{
							for ( uint32_t i = notdefRange1; i <= notdefRange2; i++ )
							{
								pParams->paCustomizedFont_CharSet[i] = notdefRange3;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
								wprintf(L"ParseCMapStream: OK!!! NOTDEFRANGE -> pParams->paCustomizedFont_CharSet[%lu] = %lu\n", i, notdefRange3);
								#endif								
								
								notdefRange3++;
							}
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
						else
						{
							wprintf(L"ParseCMapStream: VALORI NOTDEFRANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> notdefRange1 = %lu, notdefRange2 = %lu, notdefRange3 = %lu\n", notdefRange1, notdefRange2, notdefRange3);
						}
						#endif
						
						notdefRange1 = 0;
						notdefRange2 = 0;
						notdefRange3 = 0;
						nCountNotDefRangeValues = 0;
					}
				}
				else if ( bNotdefCharState )
				{
					if ( nCountNotDefCharValues < 1 )
					{
						notdefChar1 = myValue;
						nCountNotDefCharValues++;
						
						GetNextTokenFromToUnicodeStream(pParams);
						
						if ( T_INT_LITERAL == pParams->myToken.Type )
						{
							notdefChar2 = pParams->myToken.Value.vInt;
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ERRORE ParseCMapStream 4: atteso T_INT_LITERAL\n");
							#endif
							snprintf(pParams->szError, 8192, "ERRORE ParseCMapStream 4: atteso T_INT_LITERAL\n");
							myShowErrorMessage(pParams, pParams->szError, 1);
							//fwprintf(pParams->fpErrors, L"ERRORE ParseCMapStream 4: atteso T_INT_LITERAL\n");
							retValue = 0;
							goto uscita;							
						}
																		
						if ( notdefChar1 <= 0xFFFF )
						{
							pParams->paCustomizedFont_CharSet[cidChar1] = notdefChar2;
							
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ParseCMapStream: OK!!! NOTDEFCHAR -> pParams->paCustomizedFont_CharSet[%lu] = %lu\n", notdefChar1, notdefChar2);
							#endif							
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
						else
						{
							wprintf(L"ParseCMapStream: VALORI NOTDEFCHAR > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> notdefChar1 = %lu, notdefChar2 = %lu\n", notdefChar1, notdefChar2);
						}	
						#endif					
						
						notdefChar1 = 0;
						notdefChar2 = 0;
						nCountNotDefCharValues = 0;
					}
				}
				else if ( bBfCharState )
				{
					if ( nCountBfCharValues < 1 )
					{
						bfChar1 = myValue;
						nCountBfCharValues++;
						
						GetNextTokenFromToUnicodeStream(pParams);
						
						if ( T_STRING_HEXADECIMAL == pParams->myToken.Type )
						{
							myValue = 0;
							base = 1;
							len = strnlen(pParams->myToken.Value.vString, 1024);
							for ( i = len - 1; i >= 0; i-- ) 
							{
								if ( pParams->myToken.Value.vString[i] >= '0' && pParams->myToken.Value.vString[i] <= '9' ) 
								{
									myValue += (pParams->myToken.Value.vString[i] - 48) * base;
									base = base * 16; 
								} 
								else
								{
									c = toupper(pParams->myToken.Value.vString[i]);
									if ( c >= 'A' && c <= 'F' ) 
									{ 
										myValue += (c - 55) * base; 
										base = base * 16;
									}
								}
							}
							
							if ( codepoint > 0 )
								myValue = codepoint;
								
							//lead = 0;
							//trail = 0;
							//codepoint = 0;
														
							bfChar2 = myValue;
						
							if ( bfChar1 <= 0xFFFF )
							{
								pParams->paCustomizedFont_CharSet[bfChar1] = bfChar2;
							
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
								wprintf(L"ParseCMapStream: OK!!! BFCHAR -> pParams->paCustomizedFont_CharSet[%lu] = %lu\n", bfChar1, bfChar2);
								#endif
							}
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							else
							{
								wprintf(L"ParseCMapStream: VALORI BFCHAR > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> bfChar1 = %lu, bfChar2 = %lu\n", bfChar1, bfChar2);
							}
							#endif
						
							bfChar1 = 0;
							bfChar2 = 0;
							nCountBfCharValues = 0;
						}
						else if ( T_NAME == pParams->myToken.Type )
						{
							tnameLen = strnlen(pParams->myToken.Value.vString, 4096);
							nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.Value.vString, tnameLen + sizeof(char), (void*)&bfChar2, &nDataSize, &bContentAlreadyProcessed);
							if ( nRes >= 0 ) // TROVATO
							{				
								pParams->paCustomizedFont_CharSet[bfChar1] = bfChar2;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
								wprintf(L"ParseCMapStream: OK!!! BFCHAR -> pParams->paCustomizedFont_CharSet[%lu] = %d -> (%s)\n", bfChar1, bfChar2, pParams->myToken.Value.vString);
								#endif																
							}
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
							else
							{
								wprintf(L"ParseCMapStream: BFCHAR -> KEY(%s) NOT FOUND\n", pParams->myToken.Value.vString);
							}
							#endif							
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ERRORE ParseCMapStream: atteso T_STRING_HEXADECIMAL o T_NAME\n");
							#endif
							snprintf(pParams->szError, 8192, "ERRORE ParseCMapStream: atteso T_STRING_HEXADECIMAL o T_NAME\n");
							myShowErrorMessage(pParams, pParams->szError, 1);
							//fwprintf(pParams->fpErrors, L"ERRORE ParseCMapStream: atteso T_STRING_HEXADECIMAL o T_NAME\n");
							retValue = 0;
							goto uscita;
						}						
					}
				}
				else if ( bBfRangeState )
				{
					if ( nCountBfRangeValues < 1 )
					{
						bfRange1 = myValue;
						nCountBfRangeValues++;
					}
					else if ( nCountBfRangeValues < 2 )
					{
						bfRange2 = myValue;
						nCountBfRangeValues++;
					}					
					else
					{
						bfRange3 = myValue;
						//nCountBfRangeValues++;
						
						if ( bfRange1 <= 0xFFFF && bfRange2 <= 0xFFFF )
						{
							for ( uint32_t i = bfRange1; i <= bfRange2; i++ )
							{
								pParams->paCustomizedFont_CharSet[i] = bfRange3;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
								wprintf(L"ParseCMapStream: OK!!! BFRANGE -> pParams->paCustomizedFont_CharSet[%lu] = %lu\n", i, bfRange3);
								#endif								
								
								bfRange3++;
							}
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
						else
						{
							wprintf(L"ParseCMapStream: VALORI BFRANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> bfRange1 = %lu, bfRange2 = %lu, bfRange3 = %lu\n", bfRange1, bfRange2, bfRange3);
						}
						#endif
						
						bfRange1 = 0;
						bfRange2 = 0;
						bfRange3 = 0;
						nCountBfRangeValues = 0;
					}
				}
				
				break;				
			case T_QOPAREN:
				if ( bCidRangeState )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
					wprintf(L"ParseCMapStream: VALORI CIDFRANGE ALL'INTERNO DI UN ARRAY PER IL MOMENTO NON SUPPORTATI.\n");
					#endif
					
					cidRange1 = 0;
					cidRange2 = 0;
					cidRange3 = 0;
					bCidRangeState = 0;
				}
				break;
			case T_ERROR:
			case T_VOID_STRING:
				retValue = 0;
				goto uscita;			
				break;
			default:
				break;
		}
				
		GetNextTokenFromToUnicodeStream(pParams);
	}		
	
uscita:	
	pParams->bStringIsDecoded = 1;
	return retValue;
}

int ParseToUnicodeStream(Params *pParams, int objNum, unsigned char *pszDecodedStream, unsigned long int DecodedStreamSize)
{
	int retValue = 1;
	
	int nRes;
	size_t tnameLen;
	uint32_t nDataSize;
	uint32_t bContentAlreadyProcessed;	
	
	uint32_t myValue;
	int base;
	int len;
	int i;
	char c;
	
	uint32_t codepoint;
	uint16_t lead;
	uint16_t trail;
	
	int bCodeSpaceRangeState;
	uint32_t codeSpaceRange1 = -1;
	uint32_t codeSpaceRange2 = -1;
	int nCountCodeSpaceRangeValues = 0;
	
	int bBfCharState;
	uint32_t bfChar1 = 0;
	uint32_t bfChar2 = 0;
	int nCountBfCharValues = 0;
	
	int bBfRangeState;
	uint32_t bfRange1 = 0;
	uint32_t bfRange2 = 0;
	uint32_t bfRange3 = 0;
	int nCountBfRangeValues = 0;
	
	int lastInteger = 0;
	int idxCodeSpace = 0;
				
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->myBlockToUnicode = pszDecodedStream;
	pParams->blockLenToUnicode = DecodedStreamSize;
	pParams->blockCurPosToUnicode = 0;
	
	pParams->bStringIsDecoded = 1;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
	wprintf(L"\n\nParseToUnicodeStream(OBJ %d) -> INIZIO STREAM DECODIFICATO DOPO myInflate:\n", objNum);
	for ( unsigned long int k = 0; k < DecodedStreamSize; k++ )
	{
		if ( pszDecodedStream[k] == '\0' )
			wprintf(L"\\0");
		else
			wprintf(L"%c", pszDecodedStream[k]);
	}
	wprintf(L"\nParseToUnicodeStream(OBJ %d) -> FINE STREAM DECODIFICATO DOPO myInflate.\n\n", objNum);	
	#endif
	
	pParams->pReadNextChar = ReadNextChar;
	
	bCodeSpaceRangeState = 0;
	bBfCharState = 0;
	bBfRangeState = 0;
	
	codepoint = lead = trail = 0;
		
	pParams->bHasCodeSpaceOneByte = pParams->bHasCodeSpaceTwoByte = 0;
	
	GetNextTokenFromToUnicodeStream(pParams);				
	while ( T_EOF != pParams->myToken.Type )
	{	
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
		PrintToken(&(pParams->myToken), '\t', '\0', 1);
		#endif
		
		switch ( pParams->myToken.Type )
		{
			case T_INT_LITERAL:
				lastInteger = pParams->myToken.Value.vInt;
				break;
			case T_CONTENT_OP_begincodespacerange:
				if ( NULL != pParams->pCodeSpaceRangeArray )
				{
					free(pParams->pCodeSpaceRangeArray);
					pParams->pCodeSpaceRangeArray = NULL;
				}
				pParams->pCodeSpaceRangeArray = (CodeSpaceRange_t*)malloc(sizeof(CodeSpaceRange_t) * lastInteger);
				if ( NULL == pParams->pCodeSpaceRangeArray )
				{
					snprintf(pParams->szError, 8192, "ParseToUnicodeStream: impossibile allocare la memoria per pParams->pCodeSpaceRangeArray\n");
					myShowErrorMessage(pParams, pParams->szError, 1);
					//fwprintf(pParams->fpErrors, L"ParseToUnicodeStream: impossibile allocare la memoria per pParams->pCodeSpaceRangeArray\n");
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
					wprintf(L"ParseToUnicodeStream: impossibile allocare la memoria per pParams->pCodeSpaceRangeArray\n");
					#endif
					retValue = 0;
					goto uscita;
				}
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
				else
				{
					wprintf(L"\tParseToUnicodeStream 1: ALLOCATI CORRETTAMENTE %lu BYTE per pParams->pCodeSpaceRangeArray. lastInteger = %d\n", sizeof(CodeSpaceRange_t) * lastInteger, lastInteger);
				}
				#endif
				for ( int j = 0; j < lastInteger; j++ )
				{
					pParams->pCodeSpaceRangeArray[idxCodeSpace].nFrom = 0;
					pParams->pCodeSpaceRangeArray[idxCodeSpace].nTo = 0;
				}
				pParams->nCurrentFontCodeSpacesNum = lastInteger;
				idxCodeSpace = 0;
				
				bCodeSpaceRangeState = 1;
				codeSpaceRange1 = 0;
				codeSpaceRange2 = 0;
				nCountCodeSpaceRangeValues = 0;
				break;
			case T_CONTENT_OP_endcodespacerange:
				bCodeSpaceRangeState = 0;
				codeSpaceRange1 = 0;
				codeSpaceRange2 = 0;
				nCountCodeSpaceRangeValues = 0;
				break;
			case T_CONTENT_OP_beginbfchar:				
				bBfCharState = 1;
				bfChar1 = 0;
				bfChar2 = 0;
				nCountBfCharValues = 0;
				codepoint = lead = trail = 0;
				break;
			case T_CONTENT_OP_endbfchar:
				bBfCharState = 0;
				bfChar1 = 0;
				bfChar2 = 0;			
				nCountBfCharValues = 0;	
				codepoint = lead = trail = 0;
				break;
			case T_CONTENT_OP_beginbfrange:				
				bBfRangeState = 1;
				bfRange1 = 0;
				bfRange2 = 0;
				bfRange3 = 0;
				nCountBfRangeValues = 0;
				codepoint = lead = trail = 0;
				break;
			case T_CONTENT_OP_endbfrange:
				bBfRangeState = 0;
				bfRange1 = 0;
				bfRange2 = 0;
				bfRange3 = 0;
				nCountBfRangeValues = 0;
				codepoint = lead = trail = 0;
				break;
			case T_STRING_HEXADECIMAL:
				myValue = 0;
				base = 1;
				len = strnlen(pParams->myToken.Value.vString, 1024);
				for ( i = len - 1; i >= 0; i-- ) 
				{
					if ( pParams->myToken.Value.vString[i] >= '0' && pParams->myToken.Value.vString[i] <= '9' ) 
					{
						myValue += (pParams->myToken.Value.vString[i] - 48) * base;
						base = base * 16; 
					} 
					else
					{
						c = toupper(pParams->myToken.Value.vString[i]);
						if ( c >= 'A' && c <= 'F' ) 
						{ 
							myValue += (c - 55) * base; 
							base = base * 16;
						}
					}
				}
				
				if ( bBfRangeState && nCountBfRangeValues >= 2 )
				{
					lead = 0;
					trail = 0;
					codepoint = 0;
							
					if ( 8 == len )
					{					
						c = pParams->myToken.Value.vString[0];
						if ( toupper(c) == 'D' )
						{
							c = pParams->myToken.Value.vString[4];
							if ( toupper(c) == 'D' )
							{
								for ( i = 0; i < 4; i++ )
								{
									if ( pParams->myToken.Value.vString[i] >= '0' && pParams->myToken.Value.vString[i] <= '9' ) 
									{
										lead += (pParams->myToken.Value.vString[i] - 48) * base;
										base = base * 16; 
									} 
									else
									{
										c = toupper(pParams->myToken.Value.vString[i]);
										if ( c >= 'A' && c <= 'F' ) 
										{ 
											lead += (c - 55) * base; 
											base = base * 16;
										}
									}								
								}
							
								for ( i = 4; i < 8; i++ )
								{
									if ( pParams->myToken.Value.vString[i] >= '0' && pParams->myToken.Value.vString[i] <= '9' ) 
									{
										trail += (pParams->myToken.Value.vString[i] - 48) * base;
										base = base * 16; 
									} 
									else
									{
										c = toupper(pParams->myToken.Value.vString[i]);
										if ( c >= 'A' && c <= 'F' ) 
										{ 
											trail += (c - 55) * base;
											base = base * 16;
										}
									}								
								}
								
								codepoint = getSurrogateCodePoint(lead, trail);
							}
						}
					}
				
					if ( codepoint > 0 )
						myValue = codepoint;
						
					lead = 0;
					trail = 0;
					codepoint = 0;						
				}
								
				if ( bCodeSpaceRangeState )
				{
					if ( nCountCodeSpaceRangeValues < 1 )
					{
						codeSpaceRange1 = myValue;
						nCountCodeSpaceRangeValues++;
					}
					else
					{
						codeSpaceRange2 = myValue;
						
						if ( codeSpaceRange2 > 0xFFFF )
						{
							snprintf(pParams->szError, 8192, "ParseToUnicodeStream: VALORI CODESPACERANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> codeSpaceRange1 = %u, codeSpaceRange2 = %u\n", codeSpaceRange1, codeSpaceRange2);
							myShowErrorMessage(pParams, pParams->szError, 1);
							//fwprintf(pParams->fpErrors, L"ParseToUnicodeStream: VALORI CODESPACERANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> codeSpaceRange1 = %lu, codeSpaceRange2 = %lu\n", codeSpaceRange1, codeSpaceRange2);
							
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
							wprintf(L"ParseToUnicodeStream: VALORI CODESPACERANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> codeSpaceRange1 = %u, codeSpaceRange2 = %u\n", codeSpaceRange1, codeSpaceRange2);
							#endif
							
							pParams->bHasCodeSpaceOneByte = pParams->bHasCodeSpaceTwoByte = 0;
							
							//retValue = 0;
							//goto uscita;
						}
						else if ( codeSpaceRange2 <= 0xFF )
						{
							pParams->bHasCodeSpaceOneByte = 1;
							pParams->pCodeSpaceRangeArray[idxCodeSpace].nNumBytes = 1;
						}
						else if ( codeSpaceRange2 <= 0xFFFF )
						{
							pParams->bHasCodeSpaceTwoByte = 1;
							pParams->pCodeSpaceRangeArray[idxCodeSpace].nNumBytes = 2;
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
						else
						{
							wprintf(L"ParseToUnicodeStream: OK!!! CODESPACERANGE -> codeSpaceRange1 = %u, codeSpaceRange2 = %u\n", codeSpaceRange1, codeSpaceRange2);	
						}	
						#endif			
						
						pParams->pCodeSpaceRangeArray[idxCodeSpace].nFrom = codeSpaceRange1;
						pParams->pCodeSpaceRangeArray[idxCodeSpace].nTo = codeSpaceRange2;
						idxCodeSpace++;		

						codeSpaceRange1 = 0;
						codeSpaceRange2 = 0;
						nCountCodeSpaceRangeValues = 0;
					}
				}
				else if ( bBfCharState )
				{
					if ( nCountBfCharValues < 1 )
					{
						bfChar1 = myValue;
						nCountBfCharValues++;
						
						GetNextTokenFromToUnicodeStream(pParams);
						
						if ( T_STRING_HEXADECIMAL == pParams->myToken.Type )
						{
							myValue = 0;
							base = 1;
							len = strnlen(pParams->myToken.Value.vString, 1024);
							for ( i = len - 1; i >= 0; i-- ) 
							{
								if ( pParams->myToken.Value.vString[i] >= '0' && pParams->myToken.Value.vString[i] <= '9' ) 
								{
									myValue += (pParams->myToken.Value.vString[i] - 48) * base;
									base = base * 16; 
								} 
								else
								{
									c = toupper(pParams->myToken.Value.vString[i]);
									if ( c >= 'A' && c <= 'F' ) 
									{ 
										myValue += (c - 55) * base; 
										base = base * 16;
									}
								}
							}
							
							lead = 0;
							trail = 0;
							codepoint = 0;
							
							if ( 8 == len )
							{					
								c = pParams->myToken.Value.vString[0];
								if ( toupper(c) == 'D' )
								{
									c = pParams->myToken.Value.vString[4];
									if ( toupper(c) == 'D' )
									{
										for ( i = 0; i < 4; i++ )
										{
											if ( pParams->myToken.Value.vString[i] >= '0' && pParams->myToken.Value.vString[i] <= '9' ) 
											{
												lead += (pParams->myToken.Value.vString[i] - 48) * base;
												base = base * 16; 
											} 
											else
											{
												c = toupper(pParams->myToken.Value.vString[i]);
												if ( c >= 'A' && c <= 'F' ) 
												{ 
													lead += (c - 55) * base; 
													base = base * 16;
												}
											}								
										}
							
										for ( i = 4; i < 8; i++ )
										{
											if ( pParams->myToken.Value.vString[i] >= '0' && pParams->myToken.Value.vString[i] <= '9' ) 
											{
												trail += (pParams->myToken.Value.vString[i] - 48) * base;
												base = base * 16; 
											} 
											else
											{
												c = toupper(pParams->myToken.Value.vString[i]);
												if ( c >= 'A' && c <= 'F' ) 
												{ 
													trail += (c - 55) * base;
													base = base * 16;
												}
											}								
										}
										
										codepoint = getSurrogateCodePoint(lead, trail);							
									}
								}
							}
				
							if ( codepoint > 0 )
								myValue = codepoint;
								
							lead = 0;
							trail = 0;
							codepoint = 0;								
														
							bfChar2 = myValue;
						
							if ( bfChar1 <= 0xFFFF )
							{
								pParams->paCustomizedFont_CharSet[bfChar1] = bfChar2;
							
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
								wprintf(L"ParseToUnicodeStream: OK!!! BFCHAR -> pParams->paCustomizedFont_CharSet[%lu] = %u\n", bfChar1, bfChar2);
								#endif
							}
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
							else
							{
								wprintf(L"ParseToUnicodeStream: VALORI BFCHAR > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> bfChar1 = %u, bfChar2 = %u\n", bfChar1, bfChar2);
							}
							#endif
						
							bfChar1 = 0;
							bfChar2 = 0;
							nCountBfCharValues = 0;
						}
						else if ( T_NAME == pParams->myToken.Type )
						{
							tnameLen = strnlen(pParams->myToken.Value.vString, 4096);
							nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.Value.vString, tnameLen + sizeof(char), (void*)&bfChar2, &nDataSize, &bContentAlreadyProcessed);
							if ( nRes >= 0 ) // TROVATO
							{				
								pParams->paCustomizedFont_CharSet[bfChar1] = bfChar2;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
								wprintf(L"ParseToUnicodeStream: OK!!! BFCHAR -> pParams->paCustomizedFont_CharSet[%lu] = %u -> (%s)\n", bfChar1, bfChar2, pParams->myToken.Value.vString);
								#endif																
							}
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
							else
							{
								wprintf(L"ParseToUnicodeStream: BFCHAR -> KEY(%s) NOT FOUND\n", pParams->myToken.Value.vString);
							}
							#endif							
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
							wprintf(L"ERRORE ParseToUnicodeStream: atteso T_STRING_HEXADECIMAL o T_NAME\n");
							#endif
							snprintf(pParams->szError, 8192, "ERRORE ParseToUnicodeStream: atteso T_STRING_HEXADECIMAL o T_NAME\n");
							myShowErrorMessage(pParams, pParams->szError, 1);
							//fwprintf(pParams->fpErrors, L"ERRORE ParseToUnicodeStream: atteso T_STRING_HEXADECIMAL o T_NAME\n");
							retValue = 0;
							goto uscita;
						}						
					}
				}
				else if ( bBfRangeState )
				{
					if ( nCountBfRangeValues < 1 )
					{
						bfRange1 = myValue;
						nCountBfRangeValues++;
					}
					else if ( nCountBfRangeValues < 2 )
					{
						bfRange2 = myValue;
						nCountBfRangeValues++;
					}					
					else
					{
						bfRange3 = myValue;
						//nCountBfRangeValues++;
						
						if ( bfRange1 <= 0xFFFF && bfRange2 <= 0xFFFF )
						{
							for ( uint32_t i = bfRange1; i <= bfRange2; i++ )
							{
								pParams->paCustomizedFont_CharSet[i] = bfRange3;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
								wprintf(L"ParseToUnicodeStream: OK!!! BFRANGE -> pParams->paCustomizedFont_CharSet[%lu] = %u\n", i, bfRange3);
								#endif								
								
								bfRange3++;
							}
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
						else
						{
							wprintf(L"ParseToUnicodeStream: VALORI BFRANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> bfRange1 = %u, bfRange2 = %u, bfRange3 = %lu\n", bfRange1, bfRange2, bfRange3);
						}
						#endif
						
						bfRange1 = 0;
						bfRange2 = 0;
						bfRange3 = 0;
						nCountBfRangeValues = 0;
					}
				}								
				break;
			case T_QOPAREN:
				if ( bBfRangeState )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
					wprintf(L"ParseToUnicodeStream: VALORI BFRANGE ALL'INTERNO DI UN ARRAY PER IL MOMENTO NON SUPPORTATI.\n");
					#endif
					
					bfRange1 = -1;
					bfRange2 = -1;
					bfRange3 = -1;
					bBfRangeState = 0;
				}
				break;
			case T_ERROR:
			case T_VOID_STRING:
				retValue = 0;
				goto uscita;			
				break;
			default:
				break;
		}
				
		GetNextTokenFromToUnicodeStream(pParams);
	}		
	
uscita:	
	pParams->bStringIsDecoded = 1;
	return retValue;
}

int ParseCMapObject(Params *pParams, int objNum)
{	
	int retValue = 1;
	
	unsigned char *pszDecodedStream = NULL;
	unsigned long int DecodedStreamSize = 0;
				
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseCMapObject: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseCMapObject: objNum non valido -> %d\n", objNum);
		//fwprintf(pParams->fpErrors, L"Errore ParseCMapObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Obj.Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( !LoadFirstBlock(pParams, objNum, "ParseCMapObject") )
	{
		retValue = 0;
		goto uscita;
	}
	
	mynumstacklist_Init( &(pParams->myNumStack) );	
		
	GetNextToken(pParams);
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
	wprintf(L"\n**************************************************** INIZIO STREAM TO UNICODE ******************************************************\n");
	#endif
	
	pParams->bStreamStateToUnicode = 1;
	if ( !contentobj(pParams) )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}		
		retValue = 0;
		goto uscita;
	}
			
	// ************************************************************************************************************************************************************
	
	//wprintf(L"\npParams->CurrentContent.queueFilters.count = %d\n", pParams->CurrentContent.queueFilters.count);
	//wprintf(L"\tpParams->CurrentContent.decodeParms.count = %d\n", pParams->CurrentContent.decodeParms.count);
		
	pszDecodedStream = getDecodedStream(pParams, &DecodedStreamSize, NULL);
	if ( NULL ==  pszDecodedStream )
	{
		retValue = 0;
		goto uscita;
	}
	
	mystringqueuelist_Free(&(pParams->CurrentContent.queueFilters));
	mydictionaryqueuelist_Free(&(pParams->CurrentContent.decodeParms));
	pParams->myDataDecodeParams.numFilter = 0;
				
	if ( STREAM_TYPE_TOUNICODE == pParams->bStreamType )
	{
		if ( !ParseToUnicodeStream(pParams, objNum, pszDecodedStream, DecodedStreamSize) )
		{
			retValue = 0;
			goto uscita;
		}
	}
	else // STREAM_TYPE_CMAP
	{ 
		if ( !ParseCMapStream(pParams, objNum, pszDecodedStream, DecodedStreamSize) )
		{
			retValue = 0;
			goto uscita;
		}		
	}
	// ************************************************************************************************************************************************************
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
	wprintf(L"\n**************************************************** FINE STREAM TO UNICODE ********************************************************\n");
	#endif
			
uscita:
	
	if ( NULL != pszDecodedStream )
	{
		free(pszDecodedStream);
		pszDecodedStream = NULL;
	}
	
	mystringqueuelist_Free(&(pParams->CurrentContent.queueFilters));
	mydictionaryqueuelist_Free(&(pParams->CurrentContent.decodeParms));
	pParams->myDataDecodeParams.numFilter = 0;	

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	return retValue;
}

int ParseFontObject(Params *pParams, int objNum)
{	
	int retValue = 1;
			
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseFontObject: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseFontObject: objNum non valido -> %d\n", objNum);
		//fwprintf(pParams->fpErrors, L"Errore ParseFontObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Obj.Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( !LoadFirstBlock(pParams, objNum, "ParseFontObject") )
	{
		retValue = 0;
		goto uscita;
	}	
	
	mynumstacklist_Init( &(pParams->myNumStack) );
	//pParams->nToUnicodeStreamObjRef = 0;
		
	GetNextToken(pParams);
	
	if ( !contentfontobj(pParams) )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}		
		retValue = 0;
		goto uscita;
	}
			
uscita:

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	return retValue;
}

int ParseEncodingObject(Params *pParams, int objNum)
{	
	int retValue = 1;
			
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseEncodingObject: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseEncodingObject: objNum non valido -> %d\n", objNum);
		//fwprintf(pParams->fpErrors, L"Errore ParseEncodingObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Obj.Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( !LoadFirstBlock(pParams, objNum, "ParseEncodingObject") )
	{
		retValue = 0;
		goto uscita;
	}
	
	mynumstacklist_Init( &(pParams->myNumStack) );
	
	GetNextToken(pParams);
	
	if ( !encodingobj(pParams) )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}		
		retValue = 0;
		goto uscita;
	}
			
uscita:

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	return retValue;
}

int ParseDictionaryObject(Params *pParams, int objNum)
{	
	int retValue = 1;
		
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseDictionaryObject: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseDictionaryObject: objNum non valido -> %d\n", objNum);
		//fwprintf(pParams->fpErrors, L"Errore ParseDictionaryObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->blockLen = 0;
	pParams->blockCurPos = 0;
		
	pParams->nObjToParse = objNum;
		
	if ( !LoadFirstBlock(pParams, objNum, "ParseEncodingObject") )
	{
		retValue = 0;
		goto uscita;
	}
	
	mynumstacklist_Init( &(pParams->myNumStack) );
	
	GetNextToken(pParams);

	//wprintf(L"\nParseDictionaryObject: INIZIO DIZIONARIO(objNum = %d).\n", objNum);
	if ( !resourcesdictionary(pParams) )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}
		snprintf(pParams->szError, 8192, "\nERRORE ParseDictionaryObject resourcesdictionary -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
		myShowErrorMessage(pParams, pParams->szError, 1);	
		//wprintf(L"\nERRORE ParseDictionaryObject resourcesdictionary -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
		//fwprintf(pParams->fpErrors, L"\nERRORE ParseDictionaryObject resourcesdictionary -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
		
		PrintThisObject(pParams, pParams->nObjToParse, 0, pParams->nCurrentPageNum, pParams->fpErrors);
		
		retValue = 0;
		goto uscita;
	}
	//wprintf(L"\nParseDictionaryObject: FINE DIZIONARIO(objNum = %d).\n", objNum);
		
uscita:

	mynumstacklist_Free( &(pParams->myNumStack) );
		
	return retValue;
}

int ParseLengthObject(Params *pParams, int objNum)
{
	int retValue = 1;
	unsigned char c;
	
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseLengthObject: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseLengthObject: objNum non valido -> %d\n", objNum);
		//fwprintf(pParams->fpErrors, L"Errore ParseLengthObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = objNum;;
	//pParams->nCurrentParsingObj = (uint32_t)objNum;
		
	pParams->blockLenLengthObj = 0;
	pParams->blockCurPosLengthObj = 0;
	
	pParams->fpLengthObjRef = NULL;
	pParams->fpLengthObjRef = fopen(pParams->szFileName, "rb");
	if ( pParams->fpLengthObjRef == NULL )
	{
		snprintf(pParams->szError, 8192, "Errore nell'apertura del file '%s'.\n\n", pParams->szFileName);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore nell'apertura del file '%s'.\n\n", pParams->szFileName);
		//fwprintf(pParams->fpErrors, L"Errore nell'apertura del file '%s'.\n\n", pParams->szFileName);
		retValue = 0;
		goto uscita;
	}	
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fpLengthObjRef, pParams->myObjsTable[objNum]->Obj.Offset - 1, SEEK_SET) )
	{
		snprintf(pParams->szError, 8192, "Errore ParseLengthObject fseek\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseLengthObject fseek\n");
		//fwprintf(pParams->fpErrors, L"Errore ParseLengthObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLenLengthObj = fread(pParams->myBlockLengthObj, 1, BLOCK_SIZE, pParams->fpLengthObjRef);
	
	if ( pParams->blockLenLengthObj < 3 )
	{
		snprintf(pParams->szError, 8192, "Errore ParseLengthObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Obj.Offset);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseLengthObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Obj.Offset);
		//fwprintf(pParams->fpErrors, L"Errore ParseLengthObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Obj.Offset);
		retValue = 0;
		goto uscita;
	}
			
	pParams->blockCurPosLengthObj = 0;
	
	c = pParams->myBlockLengthObj[pParams->blockCurPosLengthObj++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}
		snprintf(pParams->szError, 8192, "Errore ParseLengthObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		myShowErrorMessage(pParams, pParams->szError, 1);		
		//wprintf(L"Errore ParseLengthObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		//fwprintf(pParams->fpErrors, L"Errore ParseLengthObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		retValue = 0;
		goto uscita;		
	}
	
	mynumstacklist_Init( &(pParams->myNumStack) );
	

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseLengthObject_FN)
	PrintToken(&(pParams->myTokenLengthObj), ' ', ' ', 1);
	#endif	
	
	GetNextTokenLengthObj(pParams);
		
	if ( !lengthobj(pParams) )
	{
		snprintf(pParams->szError, 8192, "\nERRORE ParseLengthObject lengthobj -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"\nERRORE ParseLengthObject lengthobj -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
		//fwprintf(pParams->fpErrors, L"\nERRORE ParseLengthObject lengthobj -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
		retValue = 0;
		goto uscita;
	}
		
uscita:

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	if ( NULL != pParams->fpLengthObjRef )
	{
		fclose(pParams->fpLengthObjRef);
		pParams->fpLengthObjRef = NULL;
	}
	
	pParams->blockLenLengthObj = 0;
	pParams->blockCurPosLengthObj = 0;
		
	return retValue;
}

int ParseIntegerObject(Params *pParams, int objNum)
{
	int retValue = 1;
	unsigned char c;
	
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseLengthObject: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseLengthObject: objNum non valido -> %d\n", objNum);
		//fwprintf(pParams->fpErrors, L"Errore ParseLengthObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = objNum;
	//pParams->nCurrentParsingObj = (uint32_t)objNum;
		
	pParams->blockLenLengthObj = 0;
	pParams->blockCurPosLengthObj = 0;
	
	pParams->fpLengthObjRef = NULL;
	pParams->fpLengthObjRef = fopen(pParams->szFileName, "rb");
	if ( pParams->fpLengthObjRef == NULL )
	{
		snprintf(pParams->szError, 8192, "Errore nell'apertura del file '%s'.\n\n", pParams->szFileName);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore nell'apertura del file '%s'.\n\n", pParams->szFileName);
		//fwprintf(pParams->fpErrors, L"Errore nell'apertura del file '%s'.\n\n", pParams->szFileName);
		retValue = 0;
		goto uscita;
	}	
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fpLengthObjRef, pParams->myObjsTable[objNum]->Obj.Offset - 1, SEEK_SET) )
	{
		snprintf(pParams->szError, 8192, "Errore ParseIntegerObject fseek\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseIntegerObject fseek\n");
		//fwprintf(pParams->fpErrors, L"Errore ParseIntegerObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLenLengthObj = fread(pParams->myBlockLengthObj, 1, BLOCK_SIZE, pParams->fpLengthObjRef);
	
	if ( pParams->blockLenLengthObj < 3 )
	{
		snprintf(pParams->szError, 8192, "Errore ParseIntegerObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Obj.Offset);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"Errore ParseIntegerObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Obj.Offset);
		//fwprintf(pParams->fpErrors, L"Errore ParseIntegerObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Obj.Offset);
		retValue = 0;
		goto uscita;
	}
			
	pParams->blockCurPosLengthObj = 0;
	
	c = pParams->myBlockLengthObj[pParams->blockCurPosLengthObj++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
		{
			if ( NULL != pParams->myToken.Value.vString )
			{
				free(pParams->myToken.Value.vString);
				pParams->myToken.Value.vString = NULL;
			}
		}
		snprintf(pParams->szError, 8192, "Errore ParseIntegerObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		myShowErrorMessage(pParams, pParams->szError, 1);	
		//wprintf(L"Errore ParseIntegerObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		//fwprintf(pParams->fpErrors, L"Errore ParseIntegerObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		retValue = 0;
		goto uscita;		
	}
	
	mynumstacklist_Init( &(pParams->myNumStack) );
	

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseLengthObject_FN)
	PrintToken(&(pParams->myTokenLengthObj), ' ', ' ', 1);
	#endif	
	
	GetNextTokenLengthObj(pParams);
		
	if ( !integer_obj(pParams) )
	{
		snprintf(pParams->szError, 8192, "\nERRORE ParseIntegerObject lengthobj -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"\nERRORE ParseIntegerObject lengthobj -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
		//fwprintf(pParams->fpErrors, L"\nERRORE ParseIntegerObject lengthobj -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
		retValue = 0;
		goto uscita;
	}
		
uscita:

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	if ( NULL != pParams->fpLengthObjRef )
	{
		fclose(pParams->fpLengthObjRef);
		pParams->fpLengthObjRef = NULL;
	}
	
	pParams->blockLenLengthObj = 0;
	pParams->blockCurPosLengthObj = 0;
		
	return retValue;
}


// **********************************************************************************************************************

// obj        : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ objbody T_KW_ENDOBJ;
int obj(Params *pParams)
{		
	pParams->szPdfVersionFromCatalog[0] = '\0';
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE obj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//fwprintf(pParams->fpErrors, L"ERRORE obj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.Value.vInt;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	pParams->nCurrentObjNum = pParams->myToken.Value.vInt;
	
	if ( pParams->nObjToParse != pParams->nCurrentObjNum )
	{
		snprintf(pParams->szError, 8192, "ERRORE parsing obj: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE parsing obj: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		//fwprintf(pParams->fpErrors, L"ERRORE parsing obj: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		return 0;
	}
	
	GetNextToken(pParams);
			
	//if ( !match(pParams, T_INT_LITERAL, "obj") )
	//	return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
			
	if ( !match(pParams, T_INT_LITERAL, "obj") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_OBJ, "obj") )
		return 0;
		
	if ( !objbody(pParams) )
		return 0;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_ENDOBJ, "obj") )
		return 0;
		
	return 1;
}

// objbody    : objitem {objitem};
int objbody(Params *pParams)
{			
	if ( !objitem(pParams) )
		return 0;
		
	while (pParams->myToken.Type == T_NAME ||
			pParams->myToken.Type == T_STRING_LITERAL ||
			pParams->myToken.Type == T_STRING_HEXADECIMAL ||
			pParams->myToken.Type == T_REAL_LITERAL ||
			pParams->myToken.Type == T_INT_LITERAL ||
			pParams->myToken.Type == T_STRING ||
			pParams->myToken.Type == T_KW_NULL ||
			pParams->myToken.Type == T_KW_FALSE ||
			pParams->myToken.Type == T_KW_TRUE ||
			pParams->myToken.Type == T_KW_STREAM ||
			pParams->myToken.Type == T_KW_ENDSTREAM ||
			pParams->myToken.Type == T_DICT_BEGIN ||
			pParams->myToken.Type == T_QOPAREN)
			{				
				if ( !objitem(pParams) )
					return 0;					
			}
			
	return 1;
}

/*
objitem    : T_KW_TRUE
           | T_KW_FALSE
           | T_KW_NULL
           | T_KW_STREAM
           | T_KW_ENDSTREAM
           | T_INT_LITERAL
           | T_REAL_LITERAL
           | T_STRING_LITERAL
           | T_STRING_HEXADECIMAL
           | T_NAME
           | T_STRING
           | array
           | dictionary
           ;
*/
int objitem(Params *pParams)
{	
	switch( pParams->myToken.Type )
	{
		case T_NAME:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
			GetNextToken(pParams);
			break;
		case T_STRING:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			break;			
		case T_STRING_HEXADECIMAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			break;
		case T_STRING_LITERAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			break;
		case T_REAL_LITERAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			break;
		case T_INT_LITERAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			break;
		case T_KW_NULL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			break;
		case T_KW_FALSE:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			break;
		case T_KW_TRUE:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			break;
		case T_KW_STREAM:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			break;
		case T_KW_ENDSTREAM:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			break;			
		case T_DICT_BEGIN:
			if ( !dictionary(pParams) )
				return 0;
			break;
		case T_QOPAREN:
			if ( !array(pParams) )
				return 0;
			break;
		default:
			snprintf(pParams->szError, 8192, "Errore parsing objitem: pParams->myToken.Type = %d\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore parsing objitem: pParams->myToken.Type = %d\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"Errore parsing objitem: pParams->myToken.Type = %d\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
			break;
	}
	
	return 1;
}

// array      : T_QOPAREN {objitem} T_QCPAREN;
int array(Params *pParams)
{		
	if ( !match(pParams, T_QOPAREN, "array") )
		return 0;
	
	while (pParams->myToken.Type == T_NAME ||
			pParams->myToken.Type == T_STRING_LITERAL ||
			pParams->myToken.Type == T_STRING_HEXADECIMAL ||
			pParams->myToken.Type == T_REAL_LITERAL ||
			pParams->myToken.Type == T_INT_LITERAL ||
			pParams->myToken.Type == T_STRING ||
			pParams->myToken.Type == T_KW_NULL ||
			pParams->myToken.Type == T_KW_FALSE ||
			pParams->myToken.Type == T_KW_TRUE ||
			pParams->myToken.Type == T_KW_STREAM ||
			pParams->myToken.Type == T_KW_ENDSTREAM ||
			pParams->myToken.Type == T_DICT_BEGIN ||
			pParams->myToken.Type == T_QOPAREN)
			{
				if ( !objitem(pParams) )
					return 0;				
			}	

	if ( !match(pParams, T_QCPAREN, "array") )
		return 0;
		
	return 1;
}

// dictionary : T_DICT_BEGIN {T_NAME objitem [T_INT_LITERAL T_KW_R]} T_DICT_END;
int dictionary(Params *pParams)
{
	int bNameIsPages = 0;
	int bNameIsVersion = 0;
	double dNum;
		
	pParams->nScope = 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_BEGIN, "dictionary") )
		return 0;
		
	pParams->nScope++;
		
	while (pParams->myToken.Type == T_NAME)
	{				
		if ( strncmp(pParams->myToken.Value.vString, "Pages", 1024) == 0 )
		{
			bNameIsPages = 1;			
		}
		else if ( strncmp(pParams->myToken.Value.vString, "Version", 1024) == 0 )
		{
			bNameIsVersion = 1;
		}	
				
		if ( !objitem(pParams) )
			return 0;
				
		if ( pParams->myToken.Type == T_DICT_BEGIN )
		{
			pParams->nScope++;
			while ( pParams->nScope > 0 )
			{
				GetNextToken(pParams);
				
				if ( pParams->myToken.Type == T_DICT_END )
					pParams->nScope--;
				else if ( pParams->myToken.Type == T_DICT_BEGIN )
					pParams->nScope++;
				else if ( pParams->myToken.Type == T_ERROR || pParams->myToken.Type == T_EOF || pParams->myToken.Type == T_UNKNOWN )
					return 0;
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
				
			GetNextToken(pParams);
		}
		pParams->nScope = 0;
		
		if ( pParams->myToken.Type == T_QOPAREN )
		{
			pParams->nScope++;
			while ( pParams->nScope > 0 )
			{
				GetNextToken(pParams);
				
				if ( pParams->myToken.Type == T_QCPAREN )
					pParams->nScope--;
				else if ( pParams->myToken.Type == T_QOPAREN )
					pParams->nScope++;
				else if ( pParams->myToken.Type == T_ERROR || pParams->myToken.Type == T_EOF || pParams->myToken.Type == T_UNKNOWN )
					return 0;
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
				
			GetNextToken(pParams);
		}
		pParams->nScope = 0;		
						
		if ( pParams->myToken.Type == T_INT_LITERAL )
		{
			if ( bNameIsPages )
			{
				mynumstacklist_Push(&(pParams->myNumStack), pParams->myToken.Value.vInt);
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			
			GetNextToken(pParams);
			
			if ( bNameIsPages && pParams->myToken.Type == T_INT_LITERAL )
			{
				mynumstacklist_Push(&(pParams->myNumStack), pParams->myToken.Value.vInt);			
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
	
			if ( !match(pParams, T_INT_LITERAL, "dictionary") )
			{
				return 0;
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			
			if ( !match(pParams, T_KW_R, "dictionary") )
			{
				return 0;
			}
			
			mynumstacklist_Pop(&(pParams->myNumStack), &dNum);
			pParams->ObjPageTreeRoot.Generation = (int)dNum;
		
			mynumstacklist_Pop(&(pParams->myNumStack), &dNum);
			pParams->ObjPageTreeRoot.Number = (int)dNum;
		}
		
		if ( pParams->myToken.Type == T_NAME && bNameIsVersion )
		{
			strncpy(pParams->szPdfVersionFromCatalog, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
				
			GetNextToken(pParams);
		}	
		
		if ( pParams->myToken.Type == T_STRING_LITERAL || pParams->myToken.Type == T_STRING_HEXADECIMAL )
		{			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
				
			GetNextToken(pParams);
		}
				
		bNameIsPages = 0;
		bNameIsVersion = 0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_END, "dictionary") )
	{
		return 0;	
	}
	
	return 1;
}

// ************************************************************************************************************************

//prepagetree          : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ pagetreebody T_KW_ENDOBJ;
int prepagetree(Params *pParams)
{	
	int bPreviousTokenIsDictBegin;
	
	int bPreviousTokenIsName;
	char szPreviousName[1024];
	
	char szLastName[1024];
	
		
	// nCurrentPageResources            -> 0 se la pagina non ha riferimenti a Resources;
	//                                     -1 se la pagina eredita Resources da uno dei suoi parenti;
	//                                     altrimenti un intero > 0 che indica il riferimento al numero dell'oggetto Resources.
	
	// bCurrentPageHasDirectResources   -> 1 Se risorsa diretta; 0 altrimenti.
	
	pParams->bIsInXObjState = 0;
		
	pParams->bCurrentPageHasDirectResources = 0;
	pParams->nCurrentPageResources = -1;
	
	pParams->nCountPageAlreadyDone = 0;	
	
	//if ( pParams->bPrePageTreeExit )
	//{
	//	return 1;
	//}
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "AHIA! ERRORE prepagetree: atteso T_INT_LITERAL; trovato token n° %d.\n", pParams->myToken.Type);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE prepagetree: atteso T_INT_LITERAL; trovato token n° %d.\n", pParams->myToken.Type);
		//fwprintf(pParams->fpErrors, L"AHIA! ERRORE prepagetree: atteso T_INT_LITERAL; trovato token n° %d.\n", pParams->myToken.Type);
		PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.Value.vInt;
					
	pParams->nCurrentObjNum = pParams->myToken.Value.vInt;
			
	if ( pParams->nObjToParse != pParams->nCurrentObjNum )
	{
		snprintf(pParams->szError, 8192, "ERRORE parsing prepagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE parsing prepagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		//fwprintf(pParams->fpErrors, L"ERRORE parsing prepagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
		return 0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PREPAGETREEOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	GetNextToken(pParams);
					
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PREPAGETREEOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_INT_LITERAL, "prepagetree") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PREPAGETREEOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_KW_OBJ, "prepagetree") )
		return 0;
				

	bPreviousTokenIsDictBegin = 0;
	bPreviousTokenIsName = 0;
	szPreviousName[0] = '\0';
	szLastName[0] = '\0';
	do
	{		
		if ( pParams->myToken.Type == T_ERROR || pParams->myToken.Type == T_EOF )
		{
			snprintf(pParams->szError, 8192, "Errore parsing prepagetree: token non valido: %d\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore parsing prepagetree: token non valido: %d\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"Errore parsing prepagetree: token non valido: %d\n", pParams->myToken.Type);
			PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
			PrintToken(&(pParams->myToken), '\0', ' ', 1);
				
			return 0;
		}
		
		if ( T_NAME == pParams->myToken.Type )
		{
			bPreviousTokenIsName = 1;
			strncpy(szPreviousName, pParams->myToken.Value.vString, 1023);
			strncpy(szLastName, pParams->myToken.Value.vString, 1023);
		}
		else if ( T_DICT_BEGIN == pParams->myToken.Type )
		{
			bPreviousTokenIsDictBegin = 1;
			bPreviousTokenIsName = 0;
			szPreviousName[0] = '\0';			
		}
		else if ( T_DICT_END == pParams->myToken.Type )
		{			
			if ( bPreviousTokenIsDictBegin && strncmp(szLastName, "Resources", 1024) == 0 )
			{
				pParams->nCurrentPageResources = 0;
			}
			bPreviousTokenIsDictBegin = 0;
			bPreviousTokenIsName = 0;
			szPreviousName[0] = '\0';			
		}		
		else
		{
			bPreviousTokenIsDictBegin = 0;
			bPreviousTokenIsName = 0;
			szPreviousName[0] = '\0';
		}
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PREPAGETREEOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
					
		GetNextToken(pParams);
		
		if ( (bPreviousTokenIsName) && (T_NAME == pParams->myToken.Type) && (strncmp(szPreviousName, "Type", 4096) == 0) )
		{
			if ( strncmp(pParams->myToken.Value.vString, "Pages", 4096) == 0 )
			{
				pParams->eCurrentObjType = OBJ_TYPE_PAGES;
			}
			else if ( strncmp(pParams->myToken.Value.vString, "Page", 4096) == 0 )
			{
				pParams->eCurrentObjType = OBJ_TYPE_PAGE;
					
				if ( !pParams->nCountPageAlreadyDone )
				{
					pParams->nCountPageFound++;
					pParams->nCountPageAlreadyDone = 1;						
				}					
					
				//pParams->pPagesArray[pParams->nCountPageFound].numObjNumber = pParams->nCurrentObjNum;
				pParams->pPagesArray[pParams->nCountPageFound].numObjNumber = pParams->nCurrentParsingObj;
				pParams->pPagesArray[pParams->nCountPageFound].numObjContent = 0;
					
				mycontentqueuelist_Init(&(pParams->pPagesArray[pParams->nCountPageFound].queueContens));
				myintqueuelist_Init(&(pParams->pPagesArray[pParams->nCountPageFound].queueContentsObjRefs));
			}
		}
		else if ( T_NAME == pParams->myToken.Type )
		{
			strncpy(szLastName, pParams->myToken.Value.vString, 1023);
		}	
		else if ( T_DICT_BEGIN == pParams->myToken.Type )
		{
			bPreviousTokenIsDictBegin = 1;
			bPreviousTokenIsName = 0;
			szPreviousName[0] = '\0';			
		}
		else if ( T_DICT_END == pParams->myToken.Type )
		{			
			if ( bPreviousTokenIsDictBegin && strncmp(szLastName, "Resources", 1024) == 0 )
			{
				pParams->nCurrentPageResources = 0;
			}		
			
			bPreviousTokenIsDictBegin = 0;
			bPreviousTokenIsName = 0;
			szPreviousName[0] = '\0';						
		}			
		else
		{
			bPreviousTokenIsDictBegin = 0;
			bPreviousTokenIsName = 0;
			szPreviousName[0] = '\0';
		}			
	} while ( pParams->myToken.Type != T_KW_ENDOBJ );
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PREPAGETREEOBJ)
	wprintf(L"\n");
	#endif
											
	return 1;
}	


//*************************************************************************************************************************

//pagetree          : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ pagetreebody T_KW_ENDOBJ;
int pagetree(Params *pParams)
{
	//pParams->nCountPageAlreadyDone = 0;
	//pParams->nCountPagesFromPdf = 0;
	//pParams->nCurrentPageResources = -1;
	//pParams->bCurrentPageHasDirectResources = 0;


	// nCurrentPageParent            -> 0 se nodo radice; altrimenti intero > 0 indica il nodo genitore della pagina corrente
	
	// nCurrentPageResources         -> 0 se la pagina non ha riferimenti a Resources;
	//                                  -1 se la pagina eredita Resources da uno dei suoi parenti;
	//                                  altrimenti un intero > 0 che indica il riferimento al numero dell'oggetto Resources.
	
	// bCurrentPageDirectResources   -> 1 Se risorsa diretta; 0 altrimenti.
			
	pParams->bIsInXObjState = 0;
	
	pParams->nCurrentPageParent = 0;
	
		
	pParams->szCurrKeyName[0] = '\0';
	pParams->szCurrResourcesKeyName[0] = '\0';
	
	pParams->nCurrentXObjRef = 0;
	pParams->nCurrentFontsRef = 0;
		
	pParams->bXObjectKeys = 0;
	pParams->bFontsKeys = 0;	
	
	pParams->nDictionaryType = DICTIONARY_TYPE_GENERIC;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
	wprintf(L"\n");
	#endif	
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE pagetree: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE pagetree: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE pagetree: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.Value.vInt;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	pParams->nCurrentObjNum = pParams->myToken.Value.vInt;
		
	if ( pParams->nObjToParse != pParams->nCurrentObjNum )
	{
		snprintf(pParams->szError, 8192, "ERRORE parsing pagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE parsing pagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		//fwprintf(pParams->fpErrors, L"ERRORE parsing pagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		return 0;
	}
	
	GetNextToken(pParams);			
					
	//if ( !match(pParams, T_INT_LITERAL, "pagetree") )
	//	return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif		
		
	if ( !match(pParams, T_INT_LITERAL, "pagetree") )
		return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif		

	if ( !match(pParams, T_KW_OBJ, "pagetree") )
		return 0;
				
	if ( !pagetreebody(pParams) )
		return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif		
			
	if ( !match(pParams, T_KW_ENDOBJ, "pagetree") )
		return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
	wprintf(L"\n\n");
	#endif	
	
	
	pParams->nCurrentPageNum = pParams->nCountPageFound;
	
	//if ( pParams->nCountPageFound > pParams->nCountPagesFromPdf )
	//{
	//	wprintf(L"ERRORE parsing pagetree: numero indice pagina corrente %d maggiore del numero totale di pagine attesi -> %d\n", pParams->nCountPageFound, pParams->nCountPagesFromPdf);
	//	return 0;
	//}	
	
	if ( pParams->nCurrentPageResources > 0 )
	{
		pParams->nDictionaryType = DICTIONARY_TYPE_RESOURCES;
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentPageResources) )
		{
			return 0;
		}
	}
		
	if ( pParams->nCurrentXObjRef > 0 )
	{
		pParams->nDictionaryType = DICTIONARY_TYPE_XOBJ;
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentXObjRef) )
			return 0;
	}
	
	if ( pParams->nCurrentFontsRef > 0 )
	{
		pParams->nDictionaryType = DICTIONARY_TYPE_FONT;
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentFontsRef) )
			return 0;		
	}	
	
	pParams->nCurrentXObjRef = 0;
	pParams->nCurrentFontsRef = 0;
						
	return 1;
}

// pagetreebody      : T_DICT_BEGIN pagetreeitems T_DICT_END;
int pagetreebody(Params *pParams)
{
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_BEGIN, "pagetreebody") )
		return 0;
		
	if ( !pagetreeitems(pParams) )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
			
	if ( !match(pParams, T_DICT_END, "pagetreebody") )
		return 0;
	
	return 1;
}

// pagetreeitems     : {T_NAME pagetreeobj};
int pagetreeitems(Params *pParams)
{		
	while ( pParams->myToken.Type == T_NAME )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.Value.vString, 4096 - 1);
		
		if ( !pParams->nCountPageAlreadyDone && strncmp(pParams->szCurrKeyName, "Contents", 4096 - 1) == 0 )
		{
			pParams->nCountPageFound++;
			pParams->nCountPageAlreadyDone = 1;
			
			myintqueuelist_Init(&(pParams->pPagesArray[pParams->nCountPageFound].queueContentsObjRefs));
			mycontentqueuelist_Init(&(pParams->pPagesArray[pParams->nCountPageFound].queueContens));	
		}
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
		
		GetNextToken(pParams);
		
		if ( !pagetreeobj(pParams) )
			return 0;
	}
	
	return 1;
}

/*
pagetreeobj : T_NAME
            | T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
            | T_STRING_LITERAL
            | T_STRING_HEXADECIMAL
            | T_KW_TRUE
            | T_KW_FALSE
            | T_QOPAREN pagetreearrayobjs T_QCPAREN
            | T_DICT_BEGIN pagetreedictobjs T_DICT_END
            ; 
*/
int pagetreeobj(Params *pParams)
{
	double dNum;
	int iNum;
	//int iGen;
	
	switch ( pParams->myToken.Type )
	{
		case T_NAME:							
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			break;
		case T_INT_LITERAL:
			mynumstacklist_Push(&(pParams->myNumStack), pParams->myToken.Value.vInt);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
	
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL )
			{
				mynumstacklist_Push(&(pParams->myNumStack), pParams->myToken.Value.vInt);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif				
				
				GetNextToken(pParams);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
								
				if ( !match(pParams, T_KW_R, "pagetreeobj") )
					return 0;
				
				mynumstacklist_Pop(&(pParams->myNumStack), &dNum);
				//iGen = (int)dNum;
				mynumstacklist_Pop(&(pParams->myNumStack), &dNum);
				iNum = (int)dNum;
				
				//wprintf(L"pagetreeobj OBJECT REFERENCE -> %d %d R\n", iNum, iGen);
				
				if ( strncmp(pParams->szCurrKeyName, "Contents", 4096) == 0 )
				{
					pParams->pPagesArray[pParams->nCountPageFound].numObjContent = iNum;
				}
				else if ( strncmp(pParams->szCurrKeyName, "Parent", 4096) == 0  )
				{
					pParams->nCurrentPageParent = iNum;
				}							
				else if ( strncmp(pParams->szCurrKeyName, "Resources", 4096) == 0  )
				{				
					if ( !(pParams->bCurrentPageHasDirectResources) )
					{						
						pParams->nCurrentPageResources = iNum;
						pParams->bXObjectKeys = 0;
						pParams->bFontsKeys = 0;
					}
					else
					{
						if ( pParams->bXObjectKeys )
						{
							if ( strncmp(pParams->szCurrResourcesKeyName, "XObject", 4096) != 0  )
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
								wprintf(L"ECCO, pagetreeobj -> metto in coda(pParams->myXObjRefList) l'XObjRef(Key = '%s') %d della pagina %d\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCountPageFound);
								#endif						
								myobjreflist_Enqueue(&(pParams->myXObjRefList), pParams->szCurrResourcesKeyName, iNum);
							}
							else
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
								wprintf(L"ECCO, pagetreeobj -> NON metto in coda pParams->nCurrentXObjRef %d della pagina %d\n", iNum, pParams->nCountPageFound);
								#endif									
								pParams->nCurrentXObjRef = iNum;
							}							
						}
						else
						{
							if ( strncmp(pParams->szCurrResourcesKeyName, "XObject", 4096) == 0  )
								pParams->nCurrentXObjRef = iNum;
						}
					
						if ( pParams->bFontsKeys )
						{
							if ( strncmp(pParams->szCurrResourcesKeyName, "Font", 4096) != 0  )
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
								wprintf(L"ECCO, pagetreeobj -> metto in coda(pParams->myFontsRefList) il FontsRef(Key = '%s') %d della pagina %d\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCountPageFound);
								#endif						
								myobjreflist_Enqueue(&(pParams->myFontsRefList), pParams->szCurrResourcesKeyName, iNum);
							}
							else
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
								wprintf(L"ECCO, pagetreeobj -> NON metto in coda pParams->nCurrentFontsRef %d della pagina %d\n", iNum, pParams->nCountPageFound);
								#endif	
								pParams->nCurrentFontsRef = iNum;
							}
						}
						else
						{
							//if ( strncmp(pParams->szCurrResourcesKeyName, "Font", 4096) == 0  )
							if ( strncmp(pParams->szCurrKeyName, "Font", 4096) == 0  )
							{
								pParams->nCurrentFontsRef = iNum;
							}
						}
					}
				}
			}
			else
			{
				mynumstacklist_Pop(&(pParams->myNumStack), &dNum);
				iNum = (int)dNum;				
				if ( pParams->nCountPagesFromPdf == 0 && (strncmp(pParams->szCurrKeyName, "Count", 4096) == 0) )
				{
					pParams->nCountPagesFromPdf = iNum;
				}
				//if ( (strncmp(pParams->szCurrKeyName, "Count", 4096) == 0) )
				//{
				//	pParams->nCountPagesFromPdf = iNum;
				//}				
			}
			break;
		case T_STRING_LITERAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
			GetNextToken(pParams);
			break;
		case T_STRING_HEXADECIMAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
			GetNextToken(pParams);
			break;
		case T_KW_TRUE:
		case T_KW_FALSE:
			// IGNORIAMO
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			GetNextToken(pParams);
			break;			
		case T_QOPAREN:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif	
			
			pParams->countArrayScope = 1;	
			
			GetNextToken(pParams);
			
			if ( !pagetreearrayobjs(pParams) )
				return 0;
				
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif				
			
			if ( !match(pParams, T_QCPAREN, "pagetreeobj") )
				return 0;
				
			pParams->countArrayScope = 0;
			break;
		case T_DICT_BEGIN:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif	
				
			GetNextToken(pParams);
			
			if ( !pagetreedictobjs(pParams) )
				return 0;
				
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif				
			
			if ( !match(pParams, T_DICT_END, "pagetreeobj") )
				return 0;
				
			pParams->bXObjectKeys = 0;
			pParams->bFontsKeys = 0;
			break;
		default:
			snprintf(pParams->szError, 8192, "Errore parsing pagetreeobj: token non valido: %d\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore parsing pagetreeobj: token non valido: %d\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"Errore parsing pagetreeobj: token non valido: %d\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), '\0', ' ', 1);
			return 0;
			break;
	}
	
	return 1;
}

// pagetreearrayobjs : {T_INT_LITERAL T_INT_LITERAL T_KW_R};
int pagetreearrayobjs(Params *pParams)
{
	//int count = 1;
	int nInt = 0;
	
	if ( strncmp(pParams->szCurrKeyName, "Kids", 4096) == 0 )
	{			
		while ( pParams->myToken.Type == T_INT_LITERAL )
		{
			nInt = pParams->myToken.Value.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			
			GetNextToken(pParams);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
						
			if ( !match(pParams, T_INT_LITERAL, "pagetreearrayobjs") )
				return 0;

			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			if ( !match(pParams, T_KW_R, "pagetreearrayobjs") )
				return 0;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
			wprintf(L"ECCO, Metto in coda il nodo pagina %d\n", nInt);
			#endif
				
			myintqueuelist_Enqueue(&(pParams->myPagesQueue), nInt);
		}	
	}
	else if ( strncmp(pParams->szCurrKeyName, "Contents", 4096) == 0 )
	{			
		while ( pParams->myToken.Type == T_INT_LITERAL )
		{
			nInt = pParams->myToken.Value.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			
			GetNextToken(pParams);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
						
			if ( !match(pParams, T_INT_LITERAL, "pagetreearrayobjs") )
				return 0;

			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			if ( !match(pParams, T_KW_R, "pagetreearrayobjs") )
			{
				return 0;
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
			wprintf(L"ECCO, Metto in coda il contenuto Obj(%d) della pagina %d\n", nInt, pParams->nCountPageFound);
			#endif
				
			myintqueuelist_Enqueue(&(pParams->pPagesArray[pParams->nCountPageFound].queueContentsObjRefs), nInt);
		}
	}	
	else
	{
		do
		{
			if ( pParams->myToken.Type == T_QCPAREN )
				pParams->countArrayScope--;
			else if ( pParams->myToken.Type == T_QOPAREN )
				pParams->countArrayScope++;
													
			if ( pParams->myToken.Type == T_ERROR || pParams->myToken.Type == T_EOF )
			{
				snprintf(pParams->szError, 8192, "Errore parsing pagetreearrayobjs: token non valido: %d\n", pParams->myToken.Type);
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"Errore parsing pagetreearrayobjs: token non valido: %d\n", pParams->myToken.Type);
				//fwprintf(pParams->fpErrors, L"Errore parsing pagetreearrayobjs: token non valido: %d\n", pParams->myToken.Type);
				PrintToken(&(pParams->myToken), '\0', ' ', 1);
	
				return 0;
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif	
			
			GetNextToken(pParams);
						
		} while ( pParams->myToken.Type != T_QCPAREN || pParams->countArrayScope > 1 );
	}
	
	return 1;
}

// pagetreedictobjs  : {T_NAME pagetreeobj};
int pagetreedictobjs(Params *pParams)
{	
	while ( T_NAME == pParams->myToken.Type )
	{
		strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.Value.vString, 4096 - 1);
		
		if ( strncmp(pParams->szCurrKeyName, "Resources", 4096) == 0 )
		{	
			pParams->bCurrentPageHasDirectResources = 1;
						
			if ( strncmp(pParams->szCurrResourcesKeyName, "XObject", 4096) == 0  )
			{
				pParams->bXObjectKeys = 1;
				pParams->bFontsKeys = 0;
			}
			
			if ( strncmp(pParams->szCurrResourcesKeyName, "Font", 4096) == 0  )
			{
				pParams->bFontsKeys = 1;
				pParams->bXObjectKeys = 0;
			}			
			
			//strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.Value.vString, 4096 - 1);
		}
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif

		GetNextToken(pParams);
				
		if ( !pagetreeobj(pParams) )
			return 0;
	}
			
	return 1;
}


// ************************************************************************************************************************

//contentobj      : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ contentobjbody T_KW_ENDOBJ;
int contentobj(Params *pParams)
{	
	pParams->nCountDecodeParams = pParams->nCountFilters = 0;
	
	pParams->nCurrentUseCMapRef = 0;
	pParams->szUseCMap[0] = '\0';	
	
	
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE contentobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE contentobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.Value.vInt;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	GetNextToken(pParams);
	
	
				
	//if ( !match(pParams, T_INT_LITERAL, "contentobj") )
	//	return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_INT_LITERAL, "contentobj") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_KW_OBJ, "contentobj") )
		return 0;
		
	if ( !contentobjbody(pParams) )
		return 0;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_ENDOBJ, "contentobj") )
		return 0;
		
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	wprintf(L"\n\n");
	#endif		
			
	if ( !(pParams->bStreamLengthIsPresent) )
	{
		snprintf(pParams->szError, 8192, "ERRORE contentobj: la chiave 'Length' e' assente.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentobj: la chiave 'Length' e' assente.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE contentobj: la chiave 'Length' e' assente.\n");
		return 0;
	}
	
	if ( NULL != pParams->myDataDecodeParams.pszKey )
	{
		free(pParams->myDataDecodeParams.pszKey);
		pParams->myDataDecodeParams.pszKey = NULL;
	}
	
	return 1;
}

// contentobjbody  : T_DICT_BEGIN streamdictitems T_DICT_END T_KW_STREAM T_STRING T_KW_ENDSTREAM;
int contentobjbody(Params *pParams)
{
	int retValue = 1;
	
	//long int pos1;
	//long int pos2;
	
	pParams->CurrentContent.bExternalFile = 0;
	pParams->CurrentContent.Offset = 0;
	//pParams->CurrentContent.Length = 0;
	pParams->CurrentContent.LengthFromPdf = 0;
	pParams->bStreamLengthIsPresent = 0;
	
	pParams->myDataDecodeParams.numFilter = 0;
	pParams->myDataDecodeParams.pszKey = NULL;
	
	mystringqueuelist_Init(&(pParams->CurrentContent.queueFilters));
	
	mydictionaryqueuelist_Init(&(pParams->CurrentContent.decodeParms), 1, 1);
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_BEGIN, "contentobjbody") )
	{
		retValue = 0;
		goto uscita;
	}
				
	if ( !streamdictitems(pParams) )
	{
		retValue = 0;
		goto uscita;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_DICT_END, "contentobjbody") )
	{
		retValue = 0;
		goto uscita;
	}
		
	if ( pParams->myToken.Type != T_KW_STREAM )
	{
		snprintf(pParams->szError, 8192, "ERRORE contentobjbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentobjbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		//fwprintf(pParams->fpErrors, L"ERRORE contentobjbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		retValue = 0;
		goto uscita;
	}
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	pParams->CurrentContent.Offset = pParams->nNumBytesReadFromCurrentStream;
	pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamOffset = pParams->CurrentContent.Offset;
	//pParams->CurrentContent.Offset = pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamOffset;
	/*
	if ( pParams->CurrentContent.LengthFromPdf != pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength )
	{
		fwprintf(pParams->fpErrors, L"WARNING contentobjbody: pParams->CurrentContent.LengthFromPdf = %lu differs from pParams->myObjsTable[%d]->Obj.StreamLength = %lu\n",  pParams->CurrentContent.LengthFromPdf, pParams->nCurrentParsingObj, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength);
		//PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
		//pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength = pParams->CurrentContent.LengthFromPdf;
	}
	*/
	//pParams->CurrentContent.LengthFromPdf = pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength;
			
	//pos1 = ftell(pParams->fp);	
	//GetNextToken(pParams);
	if ( fseek(pParams->fp, pParams->CurrentContent.Offset + pParams->CurrentContent.LengthFromPdf, SEEK_SET) != 0 )
	{
		snprintf(pParams->szError, 8192, "ERRORE contentobjbody: fseek\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentobjbody: fseek\n");
		//fwprintf(pParams->fpErrors, L"ERRORE contentobjbody: fseek\n");
		retValue = 0;
		goto uscita;		
	}
	//pos2 = ftell(pParams->fp);
	//wprintf(L"\n\npos1 = %lu <> pos2 = %lu <> Length = %lu <> pos2 - pos1 = %lu\n\n", pos1, pos2, pParams->CurrentContent.LengthFromPdf, pos2 - pos1);
	
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	if ( pParams->blockLen == 0 )
	{
		pParams->myToken.Type = T_EOF;
		retValue = 0;
		goto uscita;		
	}
	pParams->blockCurPos = 0;
		
	GetNextToken(pParams);
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif		
	if ( !match(pParams, T_KW_ENDSTREAM, "contentobjbody") )
	{
		retValue = 0;
		goto uscita;
	}
	
	if ( pParams->nCountDecodeParams > 0 )
	{
		if ( pParams->nCountDecodeParams != pParams->nCountFilters )
		{			
			snprintf(pParams->szError, 8192, "ERRORE parsing contentobjbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE parsing contentobjbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
			//fwprintf(pParams->fpErrors, L"ERRORE parsing contentobjbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
			retValue = 0;
			goto uscita;
		}
	}	
	
	if ( !(pParams->bStreamStateToUnicode) )
	{
		mycontentqueuelist_Enqueue(&(pParams->pPagesArray[pParams->nCurrentPageNum].queueContens), &(pParams->CurrentContent));
	}
	
uscita:
	
	if ( !(pParams->bStreamStateToUnicode) )
	{
		mystringqueuelist_Free(&(pParams->CurrentContent.queueFilters));	
		mydictionaryqueuelist_Free(&(pParams->CurrentContent.decodeParms));
		pParams->myDataDecodeParams.numFilter = 0;
	}

	return retValue;
}

// streamdictitems : T_NAME contentkeyvalue {T_NAME contentkeyvalue};
int streamdictitems(Params *pParams)
{
	if ( pParams->myToken.Type !=  T_NAME )
	{
		snprintf(pParams->szError, 8192, "ERRORE streamdictitems: Atteso token T_NAME, trovato TOKEN n° %d\n", pParams->myToken.Type);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE streamdictitems: Atteso token T_NAME, trovato TOKEN n° %d\n", pParams->myToken.Type);
		//fwprintf(pParams->fpErrors, L"ERRORE streamdictitems: Atteso token T_NAME, trovato TOKEN n° %d\n", pParams->myToken.Type);
		return 0;
	}
	strncpy(pParams->szCurrKeyName, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
	
	if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
		pParams->bStreamLengthIsPresent = 1;	
		
	if ( strncmp(pParams->szCurrKeyName, "F", 1024) == 0 )
		pParams->CurrentContent.bExternalFile = 1;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif		
		
	GetNextToken(pParams);
	
	if ( !contentkeyvalue(pParams) )
		return 0;
		
	while ( pParams->myToken.Type ==  T_NAME )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
		
		if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
			pParams->bStreamLengthIsPresent = 1;		
						
		if ( strncmp(pParams->szCurrKeyName, "F", 1024) == 0 )
			pParams->CurrentContent.bExternalFile = 1;	
			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
			
		GetNextToken(pParams);
		
		if ( !contentkeyvalue(pParams) )
			return 0;		
	}
	
	return 1;
}

/*
contentkeyvalue : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
                | T_NAME
                | T_STRING_LITERAL
                | T_STRING_HEXADECIMAL
                | TSTRING
                | contentkeyarray
                | contentkeydict
                ;
*/
int contentkeyvalue(Params *pParams)
{
	int n1 = -1;
	int n2 = -1;
	//int len;
	
	switch ( pParams->myToken.Type )
	{
		case T_INT_LITERAL:
			n1 = pParams->myToken.Value.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.Value.vInt;
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				GetNextToken(pParams);
				
				if ( !match(pParams, T_KW_R, "contentkeyvalue") )
					return 0;
			}
			if ( n2 < 0 )
			{
				if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
				{
					pParams->CurrentContent.LengthFromPdf = n1;
				}
			}
			else
			{
				if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
				{
					pParams->CurrentContent.LengthFromPdf = 0;
					
					if ( !ParseLengthObject(pParams, n1) )
					{
						snprintf(pParams->szError, 8192, "ERRORE contentkeyvalue streamobj number %d\n", n1);
						myShowErrorMessage(pParams, pParams->szError, 1);
						//wprintf(L"ERRORE contentkeyvalue streamobj number %d\n", n1);
						//fwprintf(pParams->fpErrors, L"ERRORE contentkeyvalue streamobj number %d\n", n1);
						return 0;
					}
					
					pParams->CurrentContent.LengthFromPdf = pParams->nCurrentStreamLenghtFromObjNum;
				}				
				else if ( strncmp(pParams->szCurrKeyName, "UseCMap", 1024) == 0 ) // UseCMap REF
				{
					//strncpy(pParams->szUseCMap, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
					pParams->nCurrentUseCMapRef = n1;
				}				
			}
			break;
		case T_NAME:
			if ( (strncmp(pParams->szCurrKeyName, "Filter", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
			{
				if ( (strncmp(pParams->myToken.Value.vString, "FlateDecode", 1024) != 0) && pParams->bStreamStateToUnicode )
				{
					snprintf(pParams->szError, 8192, "ERRORE contentkeyvalue(bStreamStateToUnicode): Flitri diversi da FlateDecode non supportati in questa versione del programma.\n");
					myShowErrorMessage(pParams, pParams->szError, 1);
					//wprintf(L"ERRORE contentkeyvalue(bStreamStateToUnicode): Flitri diversi da FlateDecode non supportati in questa versione del programma.\n");
					//fwprintf(pParams->fpErrors, L"ERRORE contentkeyvalue(bStreamStateToUnicode): Flitri diversi da FlateDecode non supportati in questa versione del programma.\n");
					return 0;					
				}
				
				//UseCMap PREDEFINED
				if ( strncmp(pParams->szCurrKeyName, "UseCMap", 1024) == 0 )
				{
					strncpy(pParams->szUseCMap, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
				}
				
				mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.Value.vString);
				if ( (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
					pParams->CurrentContent.bExternalFile = 1;
				pParams->nCountFilters++;
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			break;
		case T_STRING_LITERAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			/*
			if ( (strncmp(pParams->szCurrKeyName, "F", 1024) == 0) )
			{
				len = strnlen(pParams->myToken.Value.vString, 4096);
				strncpy(pParams->CurrentContent.szFileSpecifications, pParams->myToken.Value.vString, len + 1);
			}
			*/
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING_HEXADECIMAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			/*
			if ( (strncmp(pParams->szCurrKeyName, "F", 1024) == 0) )
			{
				len = strnlen(pParams->myToken.Value.vString, 4096);
				strncpy(pParams->CurrentContent.szFileSpecifications, pParams->myToken.Value.vString, len + 1);
			}
			*/
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING: // File Specifications: vedi su PDF3000_2008 a pag. 99
			/*
			if ( (strncmp(pParams->szCurrKeyName, "F", 1024) == 0) )
			{
				len = strnlen(pParams->myToken.Value.vString, 4096);
				strncpy(pParams->CurrentContent.szFileSpecifications, pParams->myToken.Value.vString, len + 1);
			}
			*/
			// IGNORIAMO
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			GetNextToken(pParams);
			break;
		case T_QOPAREN:
			if ( !contentkeyarray(pParams) )
				return 0;
			break;
		case T_DICT_BEGIN:
			if ( !contentkeydict(pParams) )
				return 0;
			if ( (strncmp(pParams->szCurrKeyName, "DecodeParms", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FDecodeParms", 1024) == 0) )
				pParams->nCountDecodeParams++;				
			break;
		default:
			snprintf(pParams->szError, 8192, "ERRORE contentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE contentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE contentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
			break;
	}
	
	return 1;
}

// contentkeyarray : T_QOPAREN ({TNAME} | {T_KW_NULL | contentkeydict}) T_QCPAREN;
int contentkeyarray(Params *pParams)
{
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_QOPAREN, "contentkeyarray") )
		return 0;
			
	switch ( pParams->myToken.Type )
	{
		case T_NAME:
			while ( pParams->myToken.Type == T_NAME )
			{				
				if ( (strncmp(pParams->szCurrKeyName, "Filter", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
				{
					mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.Value.vString);
					if ( (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
						pParams->CurrentContent.bExternalFile = 1;
					pParams->nCountFilters++;
				}
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				GetNextToken(pParams);
			}
			break;
		case T_KW_NULL:
		case T_DICT_BEGIN:		
			while ( pParams->myToken.Type == T_KW_NULL || pParams->myToken.Type == T_DICT_BEGIN )
			{
				if ( pParams->myToken.Type == T_DICT_BEGIN )
				{
					if ( !contentkeydict(pParams) )
						return 0;
					pParams->myDataDecodeParams.numFilter++;
					
					pParams->nCountDecodeParams++;
				}
				
				if (pParams->myToken.Type == T_KW_NULL) 
				{
					pParams->myDataDecodeParams.tok.Type = T_KW_NULL;
					
					if ( NULL != pParams->myDataDecodeParams.pszKey )
					{
						free(pParams->myDataDecodeParams.pszKey);
						pParams->myDataDecodeParams.pszKey = NULL;
					}
					mydictionaryqueuelist_Enqueue(&(pParams->CurrentContent.decodeParms), &(pParams->myDataDecodeParams));
										
					pParams->myDataDecodeParams.numFilter++;
					
					pParams->nCountDecodeParams++;
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
					PrintToken(&(pParams->myToken), ' ', ' ', 1);
					#endif
					
					GetNextToken(pParams);
				}
			}
			break;
		default:
			snprintf(pParams->szError, 8192, "ERRORE contentkeyarray: Atteso uno di questi token: T_NAME, T_KW_NULL, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE contentkeyarray: Atteso uno di questi token: T_NAME, T_KW_NULL, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE contentkeyarray: Atteso uno di questi token: T_NAME, T_KW_NULL, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
			break;
	}
		
	if ( !match(pParams, T_QCPAREN, "contentkeyarray") )
	{
		return 0;
	}
	
	return 1;
}

// contentkeydict  : T_DICT_BEGIN T_NAME contentkeyvalueinternal { T_NAME contentkeyvalueinternal } T_DICT_END;
int contentkeydict(Params *pParams)
{
	int len = 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_BEGIN, "contentkeydict") )
		return 0;
	
	if ( T_NAME != pParams->myToken.Type )
		return 0;		
		
	if ( NULL != pParams->myDataDecodeParams.pszKey )
	{
		free(pParams->myDataDecodeParams.pszKey);
		pParams->myDataDecodeParams.pszKey = NULL;
	}
	
	len = strnlen(pParams->myToken.Value.vString, 1024);	
	pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));	
	if ( NULL == pParams->myDataDecodeParams.pszKey )
	{
		snprintf(pParams->szError, 8192, "ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		//fwprintf(pParams->fpErrors, L"ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		return 0;
	}
	strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.Value.vString, len + sizeof(char));
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	

	GetNextToken(pParams);
	
	if ( !contentkeyvalueinternal(pParams) )
		return 0;
		
	while ( pParams->myToken.Type == T_NAME )
	{		
		if ( NULL != pParams->myDataDecodeParams.pszKey )
		{
			free(pParams->myDataDecodeParams.pszKey);
			pParams->myDataDecodeParams.pszKey = NULL;
		}
		len = strnlen(pParams->myToken.Value.vString, 1024);	
		pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));
		if ( NULL == pParams->myDataDecodeParams.pszKey )
		{
			snprintf(pParams->szError, 8192, "ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			//fwprintf(pParams->fpErrors, L"ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			return 0;
		}
		strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.Value.vString, len + sizeof(char));
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);			
				
		if ( !contentkeyvalueinternal(pParams) )
		{
			return 0;
		}		
	}
	
	if ( !match(pParams, T_DICT_END, "contentkeydict") )
	{
		return 0;
	}	
	
	return 1;	
}

/*
contentkeyvalueinternal : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
                        | T_NAME
                        | T_STRING_LITERAL
                        | T_STRING_HEXADECIMAL
                        | TSTRING
                        | T_KW_TRUE
                        | T_KW_FALSE
                        | T_REAL_LITERAL
                        | contentkeyarrayinternal
                        | contentkeydict
                        ;
*/
int contentkeyvalueinternal(Params *pParams)
{
	int n1 = -1;
	int n2 = -1;
	//int len;
	
	switch ( pParams->myToken.Type )
	{
		case T_INT_LITERAL:
			n1 = pParams->myToken.Value.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			
			GetNextToken(pParams);		
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.Value.vInt;
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif				
				
				GetNextToken(pParams);
				
				if ( !match(pParams, T_KW_R, "contentkeyvalueinternal") )
					return 0;
			}
			if ( n2 < 0 )
			{
				if ( (strncmp(pParams->szCurrKeyName, "DecodeParms", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FDecodeParms", 1024) == 0) )
				{
					pParams->myDataDecodeParams.tok.Type = T_INT_LITERAL;
					pParams->myDataDecodeParams.tok.Value.vInt = n1;
					mydictionaryqueuelist_Enqueue(&(pParams->CurrentContent.decodeParms), &(pParams->myDataDecodeParams));								
				}
			}
			break;
		case T_NAME:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
				
			GetNextToken(pParams);
			break;
		case T_STRING_LITERAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING_HEXADECIMAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_KW_TRUE:        // File Specifications: vedi su PDF3000_2008 a pag. 99
		case T_KW_FALSE:       // File Specifications: vedi su PDF3000_2008 a pag. 99
		case T_REAL_LITERAL:   // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			// IGNORIAMO
			GetNextToken(pParams);			
			break;
		case T_QOPAREN:
			if ( !contentkeyarrayinternal(pParams) )
				return 0;
			break;
		case T_DICT_BEGIN:
			if ( !contentkeydict(pParams) )
				return 0;
			break;
		default:
			snprintf(pParams->szError, 8192, "ERRORE contentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE contentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE contentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
			break;
	}
	
	return 1;
}

// contentkeyarrayinternal : T_QOPAREN T_STRING_HEXADECIMAL T_STRING_HEXADECIMAL T_QCPAREN
int contentkeyarrayinternal(Params *pParams)
{	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_QOPAREN, "contentkeyarrayinternal") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_STRING_HEXADECIMAL, "contentkeyarrayinternal") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_STRING_HEXADECIMAL, "contentkeyarrayinternal") )
		return 0;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_QCPAREN, "contentkeyarrayinternal") )
		return 0;
		
	return 1;
}

// ************************************************************************************************************************

// lengthobj : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ T_INT_LITERAL T_KW_ENDOBJ;
int lengthobj(Params *pParams)
{
	if ( !matchLengthObj(pParams, T_INT_LITERAL, "lengthobj") )
		return 0;
		
	if ( !matchLengthObj(pParams, T_INT_LITERAL, "lengthobj") )
		return 0;
	
	if ( !matchLengthObj(pParams, T_KW_OBJ, "lengthobj") )
		return 0;
		
	if ( pParams->myTokenLengthObj.Type != T_INT_LITERAL )
	{
		snprintf(pParams->szError, 8192, "\nERRORE lengthobj -> errore di sintassi: Atteso token T_INT_LITERAL, trovato token n° %d\n", pParams->myTokenLengthObj.Type);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"\nERRORE lengthobj -> errore di sintassi: Atteso token T_INT_LITERAL, trovato token n° %d\n", pParams->myTokenLengthObj.Type);
		//fwprintf(pParams->fpErrors, L"\nERRORE lengthobj -> errore di sintassi: Atteso token T_INT_LITERAL, trovato token n° %d\n", pParams->myTokenLengthObj.Type);
		PrintToken(&(pParams->myTokenLengthObj), '\0', ' ', 1);
		return 0;
	}
			
	pParams->nCurrentStreamLenghtFromObjNum = pParams->myTokenLengthObj.Value.vInt;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_LENGTHOBJ)	
	PrintToken(&(pParams->myTokenLengthObj), ' ', ' ', 1);
	#endif	
	
	GetNextTokenLengthObj(pParams);
		
	if ( !matchLengthObj(pParams, T_KW_ENDOBJ, "lengthobj") )
		return 0;
	
	return 1;
}

// ************************************************************************************************************************

// integer_obj : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ T_INT_LITERAL T_KW_ENDOBJ;
int integer_obj(Params *pParams)
{
	if ( !matchLengthObj(pParams, T_INT_LITERAL, "integer_obj") )
		return 0;
		
	if ( !matchLengthObj(pParams, T_INT_LITERAL, "integer_obj") )
		return 0;
	
	if ( !matchLengthObj(pParams, T_KW_OBJ, "integer_obj") )
		return 0;
		
	if ( pParams->myTokenLengthObj.Type != T_INT_LITERAL )
	{
		snprintf(pParams->szError, 8192, "\nERRORE integer_obj -> errore di sintassi: Atteso token T_INT_LITERAL, trovato token n° %d\n", pParams->myTokenLengthObj.Type);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"\nERRORE integer_obj -> errore di sintassi: Atteso token T_INT_LITERAL, trovato token n° %d\n", pParams->myTokenLengthObj.Type);
		//fwprintf(pParams->fpErrors, L"\nERRORE integer_obj -> errore di sintassi: Atteso token T_INT_LITERAL, trovato token n° %d\n", pParams->myTokenLengthObj.Type);
		PrintToken(&(pParams->myTokenLengthObj), '\0', ' ', 1);
		return 0;
	}
			
	pParams->nCurrentTrailerIntegerNum = pParams->myTokenLengthObj.Value.vInt;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_INTEGEROBJ)	
	PrintToken(&(pParams->myTokenLengthObj), ' ', ' ', 1);
	#endif	
	
	GetNextTokenLengthObj(pParams);
		
	if ( !matchLengthObj(pParams, T_KW_ENDOBJ, "integer_obj") )
		return 0;
	
	return 1;
}

// ************************************************************************************************************************

// resourcesdictionary      : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ T_DICT_BEGIN resourcesdictionarybody T_DICT_END T_KW_ENDOBJ;
int resourcesdictionary(Params *pParams)
{	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE resourcesdictionary: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE resourcesdictionary: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE resourcesdictionary: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.Value.vInt;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	GetNextToken(pParams);
				
	//if ( !match(pParams, T_INT_LITERAL, "resourcesdictionary") )
	//	return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	wprintf(L"resourcesdictionary -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_INT_LITERAL, "resourcesdictionary") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	wprintf(L"resourcesdictionary -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_KW_OBJ, "resourcesdictionary") )
		return 0;	
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	wprintf(L"resourcesdictionary -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_DICT_BEGIN, "resourcesdictionary") )
		return 0;		
		
		
	if ( !resourcesdictionarybody(pParams) )
		return 0;
		
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	wprintf(L"resourcesdictionary -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
			
	if ( !match(pParams, T_DICT_END, "resourcesdictionary") )
		return 0;		
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	wprintf(L"resourcesdictionary -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_ENDOBJ, "resourcesdictionary") )
		return 0;
		
	return 1;
}

// resourcesdictionarybody  : { T_NAME resourcesdictionaryitems }
int resourcesdictionarybody(Params *pParams)
{
	// nCurrentPageResources         -> 0 se la pagina non ha riferimenti a Resources;
	//                                  -1 se la pagina eredita Resources da uno dei suoi parenti;
	//                                  altrimenti un intero > 0 che indica il riferimento al numero dell'oggetto Resources.	
	//if ( !(pParams->bIsInXObjState) )
	//	pParams->nCurrentPageResources = 0;
	
	pParams->bXObjectKeys = 0;
	pParams->bFontsKeys = 0;
		
	while ( pParams->myToken.Type == T_NAME )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.Value.vString, 4096 - 1);
		
		
		if ( DICTIONARY_TYPE_XOBJ == pParams->nDictionaryType )
		{
			strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.Value.vString, 4096 - 1);
			
			pParams->bXObjectKeys = 1;
			pParams->bFontsKeys = 0;
		}
		else if ( DICTIONARY_TYPE_FONT == pParams->nDictionaryType )
		{
			strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.Value.vString, 4096 - 1);
			
			pParams->bXObjectKeys = 0;
			pParams->bFontsKeys = 1;
		}		
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"resourcesdictionarybody -> ");
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
		
		GetNextToken(pParams);
		
		if ( !resourcesdictionaryitems(pParams) )
			return 0;
	}
	
	return 1;
}

// resourcesdictionaryitems : T_NAME
//                          | T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
//                          | T_STRING_LITERAL
//                          | T_STRING_HEXADECIMAL
//                          | T_QOPAREN resourcesarrayobjs T_QCPAREN
//                          | T_DICT_BEGIN resourcesdictobjs T_DICT_END
//                          ; 
int resourcesdictionaryitems(Params *pParams)
{
	double dNum;
	int iNum;
	//int iGen;
	
	switch ( pParams->myToken.Type )
	{
		case T_NAME:			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
			wprintf(L"resourcesdictionaryitems -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			break;
		case T_INT_LITERAL:
			mynumstacklist_Push(&(pParams->myNumStack), pParams->myToken.Value.vInt);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
			wprintf(L"resourcesdictionaryitems -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
	
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL )
			{
				mynumstacklist_Push(&(pParams->myNumStack), pParams->myToken.Value.vInt);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
				wprintf(L"resourcesdictionaryitems -> ");
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif				
				
				GetNextToken(pParams);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
				wprintf(L"resourcesdictionaryitems -> ");
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
								
				if ( !match(pParams, T_KW_R, "resourcesdictionaryitems") )
					return 0;
				
				mynumstacklist_Pop(&(pParams->myNumStack), &dNum);
				//iGen = (int)dNum;
				mynumstacklist_Pop(&(pParams->myNumStack), &dNum);
				iNum = (int)dNum;
				
				if ( DICTIONARY_TYPE_RESOURCES == pParams->nDictionaryType && pParams->nCurrentPageResources < 0 )
				{						
					pParams->nCurrentPageResources = iNum;
					pParams->bXObjectKeys = 0;
					pParams->bFontsKeys = 0;
				}
				
				if ( !(pParams->bIsInXObjState) )
				{					
					if ( pParams->bXObjectKeys )
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
						//wprintf(L"ECCO, metto in coda(pParams->myXObjRefList) l'XObjRef -> %s %d 0 R\n", pParams->szCurrResourcesKeyName, iNum);
						wprintf(L"ECCO, resourcesdictionaryitems -> metto in coda(pParams->myXObjRefList) l'XObjRef(Key = '%s') %d della pagina %d\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentPageNum);
						#endif						
						myobjreflist_Enqueue(&(pParams->myXObjRefList), pParams->szCurrResourcesKeyName, iNum);					
					}
					else
					{
						if ( strncmp(pParams->szCurrKeyName, "XObject", 4096) == 0  )
							pParams->nCurrentXObjRef = iNum;
					}					
					
					if ( pParams->bFontsKeys )
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
						//wprintf(L"ECCO, metto in coda(pParams->myFontsRefList) il FontsRef -> %s %d 0 R\n", pParams->szCurrResourcesKeyName, iNum);
						wprintf(L"ECCO, resourcesdictionaryitems -> metto in coda(pParams->myFontsRefList) il FontsRef(Key = '%s') %d della pagina %d\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentPageNum);
						#endif						
						myobjreflist_Enqueue(&(pParams->myFontsRefList), pParams->szCurrResourcesKeyName, iNum);					
					}
					else
					{
						if ( strncmp(pParams->szCurrKeyName, "Font", 4096) == 0  )
						{
							pParams->nCurrentFontsRef = iNum;
						}
					}					
				}
				else
				{
					if ( pParams->bXObjectKeys )
					{
						Scope PageScope;
						int nRes;
						size_t len;
						uint32_t nDataSize;
						uint32_t bContentAlreadyProcessed;						
						int n1;
						
						PageScope = pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef;
						len = strnlen(pParams->szCurrResourcesKeyName, 128);
						nRes = scopeFind(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, &nDataSize, &bContentAlreadyProcessed, 1);
						//nRes = scopeFind(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef), pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, &nDataSize, &bContentAlreadyProcessed, 1);
						if ( nRes < 0 ) // NON TROVATO
						{
							bContentAlreadyProcessed = 0;
							nRes = scopeInsert(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&(iNum), sizeof(iNum), bContentAlreadyProcessed);
							//nRes = scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef), pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&(iNum), sizeof(iNum), bContentAlreadyProcessed);
							if ( nRes )
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
								wprintf(L"resourcesdictionaryitems -> INSERITO XOBJ Key = '%s' -> %d 0 R NELLO SCOPE\n", pParams->szCurrResourcesKeyName, iNum);
								#endif
							}
						}
					}
					else
					{
						if ( strncmp(pParams->szCurrKeyName, "XObject", 4096) == 0  )
							pParams->nCurrentXObjRef = iNum;
					}					
					
					if ( pParams->bFontsKeys )
					{
						Scope PageScope;
						int nRes;
						size_t len;
						uint32_t nDataSize;
						uint32_t bContentAlreadyProcessed;
						int n1;
						
						PageScope = pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef;
						len = strnlen(pParams->szCurrResourcesKeyName, 128);
						nRes = scopeFind(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, &nDataSize, &bContentAlreadyProcessed, 1);
						//nRes = scopeFind(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef), pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, &nDataSize, &bContentAlreadyProcessed, 1);
						if ( nRes < 0 ) // NON TROVATO
						{
							bContentAlreadyProcessed = 0;
							nRes = scopeInsert(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&(iNum), sizeof(iNum), bContentAlreadyProcessed);
							//nRes = scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef), pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&(iNum), sizeof(iNum), bContentAlreadyProcessed);
							if ( nRes )
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
								wprintf(L"resourcesdictionaryitems -> INSERITO FONTS Key = '%s' -> %d 0 R NELLO SCOPE\n", pParams->szCurrResourcesKeyName, iNum);
								#endif
							}
						}
					}
					else
					{
						if ( strncmp(pParams->szCurrKeyName, "Font", 4096) == 0  )
						{
							pParams->nCurrentFontsRef = iNum;
						}
					}
				}					
			}
			else
			{
				mynumstacklist_Pop(&(pParams->myNumStack), &dNum);
			}
			break;
		case T_STRING_LITERAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
			wprintf(L"resourcesdictionaryitems -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
			GetNextToken(pParams);
			break;
		case T_STRING_HEXADECIMAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
			wprintf(L"resourcesdictionaryitems -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
			GetNextToken(pParams);
			break;
		case T_QOPAREN:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
			wprintf(L"resourcesdictionaryitems -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif	
			
			pParams->countArrayScope = 1;	
			
			GetNextToken(pParams);
			
			if ( !resourcesarrayobjs(pParams) )
				return 0;
				
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
			wprintf(L"resourcesdictionaryitems -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif				
			
			if ( !match(pParams, T_QCPAREN, "resourcesdictionaryitems") )
				return 0;
				
			pParams->countArrayScope = 0;
			break;
		case T_DICT_BEGIN:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
			wprintf(L"resourcesdictionaryitems -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif	
				
			GetNextToken(pParams);
			
			if ( !resourcesdictobjs(pParams) )
				return 0;
				
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
			wprintf(L"resourcesdictionaryitems -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif				
			
			if ( !match(pParams, T_DICT_END, "resourcesdictionaryitems") )
				return 0;
				
			pParams->bXObjectKeys = 0;
			pParams->bFontsKeys = 0;
			break;
		default:
			snprintf(pParams->szError, 8192, "Errore parsing resourcesdictionaryitems: token non valido: %d\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore parsing resourcesdictionaryitems: token non valido: %d\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), '\0', ' ', 1);
			return 0;
			break;
	}
		
	return 1;
}

// resourcesarrayobjs : {T_INT_LITERAL T_INT_LITERAL T_KW_R};
int resourcesarrayobjs(Params *pParams)
{
	do
	{
		if ( pParams->myToken.Type == T_QCPAREN )
			pParams->countArrayScope--;
		else if ( pParams->myToken.Type == T_QOPAREN )
			pParams->countArrayScope++;
												
		if ( pParams->myToken.Type == T_ERROR || pParams->myToken.Type == T_EOF )
		{
			snprintf(pParams->szError, 8192, "Errore parsing resourcesarrayobjs: token non valido: %d\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"Errore parsing resourcesarrayobjs: token non valido: %d\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"Errore parsing resourcesarrayobjs: token non valido: %d\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), '\0', ' ', 1);
			return 0;
		}
			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"resourcesarrayobjs -> ");
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif	
			
		GetNextToken(pParams);
						
	} while ( pParams->myToken.Type != T_QCPAREN || pParams->countArrayScope > 1 );
	
	return 1;
}

// resourcesdictobjs  : {T_NAME resourcesdictionaryitems};
int resourcesdictobjs(Params *pParams)
{
	if ( DICTIONARY_TYPE_RESOURCES == pParams->nDictionaryType )
	{		
		if ( strncmp(pParams->szCurrKeyName, "XObject", 4096) == 0 )
		{
			pParams->bXObjectKeys = 1;
			pParams->bFontsKeys = 0;
		}
		else if ( strncmp(pParams->szCurrKeyName, "Font", 4096) == 0 )	
		{
			pParams->bXObjectKeys = 0;
			pParams->bFontsKeys = 1;
		}
	}
	
	while ( T_NAME == pParams->myToken.Type )
	{		
		strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.Value.vString, 4096 - 1);
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"resourcesdictobjs -> ");
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif

		GetNextToken(pParams);
				
		if ( !resourcesdictionaryitems(pParams) )
			return 0;
	}
			
	return 1;
}

// ***********************************************************************************************************************

//contentxobj      : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ contentxobjbody T_KW_ENDOBJ;
int contentxobj(Params *pParams)
{	
	pParams->bIsInXObjState = 1;
	pParams->bIsInFontObjState = 1;
	
	pParams->nCountDecodeParams = pParams->nCountFilters = 0;
	
	pParams->bXObjIsIndirect = 1;
	pParams->bInXObj = 0;
	pParams->nXObjRef = 0;
	pParams->bXObjIsImage = 0;
	
	pParams->bFontObjIsIndirect = 1;
	pParams->bInFontObj = 0;
	pParams->nFontObjRef = 0;
		
	pParams->szCurrResourcesKeyName[0] = '\0';
	
	
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE contentxobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentxobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE contentxobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.Value.vInt;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	wprintf(L"contentxobj -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	GetNextToken(pParams);
	
	
	
	//if ( !match(pParams, T_INT_LITERAL, "contentxobj") )
	//	return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	wprintf(L"contentxobj -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
			
	if ( !match(pParams, T_INT_LITERAL, "contentxobj") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	wprintf(L"contentxobj -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	if ( !match(pParams, T_KW_OBJ, "contentxobj") )
		return 0;
		
	if ( !contentxobjbody(pParams) )
		return 0;

	if ( pParams->bXObjIsImage )
	{
		mystringqueuelist_Free(&(pParams->CurrentContent.queueFilters));
		mydictionaryqueuelist_Free(&(pParams->CurrentContent.decodeParms));
		pParams->myDataDecodeParams.numFilter = 0;
		
		goto uscita;
	}

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	wprintf(L"contentxobj -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_ENDOBJ, "contentxobj") )
		return 0;
			
	if ( !(pParams->bStreamLengthIsPresent) )
	{
		snprintf(pParams->szError, 8192, "ERRORE contentxobj: la chiave 'Length' è assente.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentxobj: la chiave 'Length' e' assente.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE contentxobj: la chiave 'Length' è assente.\n");
		return 0;
	}
	
	if ( pParams->nXObjRef )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"contentxobj -> VADO A PRENDERE GLI XOBJREFS DA %d 0 R\n", pParams->nXObjRef);
		#endif
		
		pParams->nDictionaryType = DICTIONARY_TYPE_XOBJ;
		
		ParseDictionaryObject(pParams, pParams->nXObjRef);
	}
	
uscita:
	
	pParams->bInXObj = 0;	
	pParams->bIsInXObjState = 0;
	
	pParams->bInFontObj = 0;
	pParams->bIsInFontObjState = 0;
	
	pParams->szCurrResourcesKeyName[0] = '\0';
	
	if ( NULL != pParams->myDataDecodeParams.pszKey )
	{
		free(pParams->myDataDecodeParams.pszKey);
		pParams->myDataDecodeParams.pszKey = NULL;
	}	
	
	return 1;
}

// contentxobjbody  : T_DICT_BEGIN xobjstreamdictitems T_DICT_END T_KW_STREAM T_STRING T_KW_ENDSTREAM;
int contentxobjbody(Params *pParams)
{
	int retValue = 1;
	
	pParams->CurrentContent.bExternalFile = 0;
	pParams->CurrentContent.Offset = 0;
	//pParams->CurrentContent.Length = 0;
	pParams->CurrentContent.LengthFromPdf = 0;
	pParams->bStreamLengthIsPresent = 0;
	
	pParams->myDataDecodeParams.numFilter = 0;
	pParams->myDataDecodeParams.pszKey = NULL;
	
	mystringqueuelist_Init(&(pParams->CurrentContent.queueFilters));
	
	mydictionaryqueuelist_Init(&(pParams->CurrentContent.decodeParms), 1, 1);
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	wprintf(L"contentxobjbody -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	
	if ( !match(pParams, T_DICT_BEGIN, "contentxobjbody") )
	{
		retValue = 0;
		goto uscita;
	}
				
	if ( !xobjstreamdictitems(pParams) )
	{
		retValue = 0;
		goto uscita;
	}
	
	if ( pParams->bXObjIsImage )
		goto uscita;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	wprintf(L"contentxobjbody -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
			
	if ( !match(pParams, T_DICT_END, "contentxobjbody") )
	{
		retValue = 0;
		goto uscita;
	}
				
	if ( pParams->myToken.Type != T_KW_STREAM )
	{
		snprintf(pParams->szError, 8192, "ERRORE contentxobjbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentxobjbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		//fwprintf(pParams->fpErrors, L"ERRORE contentxobjbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		retValue = 0;
		goto uscita;
	}
	
	pParams->CurrentContent.Offset = pParams->nNumBytesReadFromCurrentStream;	
	pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamOffset = pParams->CurrentContent.Offset;
	//pParams->CurrentContent.Offset = pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamOffset;
	/*
	if ( pParams->CurrentContent.LengthFromPdf != pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength )
	{
		fwprintf(pParams->fpErrors, L"WARNING contentxobjbody: pParams->CurrentContent.LengthFromPdf = %lu differs from pParams->myObjsTable[%d]->Obj.StreamLength = %lu\n",  pParams->CurrentContent.LengthFromPdf, pParams->nCurrentParsingObj, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength);
		//PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
		//pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength = pParams->CurrentContent.LengthFromPdf;
	}
	*/
	//pParams->CurrentContent.LengthFromPdf = pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength;
	
	
	
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	wprintf(L"contentxobjbody -> ");	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	//GetNextToken(pParams);
	if ( fseek(pParams->fp, pParams->CurrentContent.Offset + pParams->CurrentContent.LengthFromPdf, SEEK_SET) != 0 )
	{
		snprintf(pParams->szError, 8192, "ERRORE contentxobjbody: fseek\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentxobjbody: fseek\n");
		//fwprintf(pParams->fpErrors, L"ERRORE contentxobjbody: fseek\n");
		retValue = 0;
		goto uscita;		
	}
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);	
	if ( pParams->blockLen == 0 )
	{
		pParams->myToken.Type = T_EOF;
		retValue = 0;
		goto uscita;		
	}
	pParams->blockCurPos = 0;
			
	GetNextToken(pParams);
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	wprintf(L"contentxobjbody -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	
	if ( !match(pParams, T_KW_ENDSTREAM, "contentxobjbody") )
	{
		retValue = 0;
		goto uscita;
	}
		
	if ( pParams->nCountDecodeParams > 0 )
	{
		if ( pParams->nCountDecodeParams != pParams->nCountFilters )
		{
			snprintf(pParams->szError, 8192, "ERRORE parsing contentxobjbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE parsing contentxobjbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
			//fwprintf(pParams->fpErrors, L"ERRORE parsing contentxobjbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
			retValue = 0;
			goto uscita;
		}
	}	
				
uscita:
	
	return retValue;
}

// xobjstreamdictitems : {T_NAME xobjcontentkeyvalue};
int xobjstreamdictitems(Params *pParams)
{		
	while ( T_NAME == pParams->myToken.Type )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
		
		if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
			pParams->bStreamLengthIsPresent = 1;		
						
		if ( strncmp(pParams->szCurrKeyName, "F", 1024) == 0 )
			pParams->CurrentContent.bExternalFile = 1;	
			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
		wprintf(L"xobjstreamdictitems -> ");
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
			
		GetNextToken(pParams);
		
		if ( pParams->bXObjIsImage )
			return 1;
		
		if ( !xobjcontentkeyvalue(pParams) )
			return 0;		
	}
	
	return 1;
}

/*
xobjcontentkeyvalue : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
                    | T_NAME
                    | T_STRING_LITERAL
                    | T_STRING_HEXADECIMAL
                    | T_KW_TRUE
                    | T_KW_FALSE
                    | TSTRING
                    | xobjcontentkeyarray
                    | xobjcontentkeydict
                    ;
*/
int xobjcontentkeyvalue(Params *pParams)
{
	int n1 = -1;
	int n2 = -1;
	//int len;
		
	switch ( pParams->myToken.Type )
	{
		case T_INT_LITERAL:
			n1 = pParams->myToken.Value.vInt;
						
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			wprintf(L"xobjcontentkeyvalue -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.Value.vInt;
							
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
				wprintf(L"xobjcontentkeyvalue -> ");
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				GetNextToken(pParams);
				
				if ( !match(pParams, T_KW_R, "xobjcontentkeyvalue") )
					return 0;
			}
			if ( n2 < 0 )
			{
				if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
				{
					pParams->CurrentContent.LengthFromPdf = n1;
				}
			}
			else
			{
				if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
				{
					pParams->CurrentContent.LengthFromPdf = 0;
					
					if ( !ParseLengthObject(pParams, n1) )
					{
						snprintf(pParams->szError, 8192, "ERRORE xobjcontentkeyvalue streamobj number %d\n", n1);
						myShowErrorMessage(pParams, pParams->szError, 1);
						//wprintf(L"ERRORE xobjcontentkeyvalue streamobj number %d\n", n1);
						//fwprintf(pParams->fpErrors, L"ERRORE xobjcontentkeyvalue streamobj number %d\n", n1);
						return 0;
					}
					
					pParams->CurrentContent.LengthFromPdf = pParams->nCurrentStreamLenghtFromObjNum;					
				}
				
				if ( pParams->bInXObj )
				{
					if ( 0 == pParams->bXObjIsIndirect )
					{
						Scope PageScope;
						int nRes;
						size_t len;
						uint32_t nDataSize;
						uint32_t bContentAlreadyProcessed;						
						
						PageScope = pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef;
						len = strnlen(pParams->szCurrResourcesKeyName, 128);
						//nRes = scopeFind(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef), pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, &nDataSize, &bContentAlreadyProcessed, 1);
						nRes = scopeFind(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, &nDataSize, &bContentAlreadyProcessed, 1);
						if ( nRes < 0 ) // NON TROVATO
						{
							bContentAlreadyProcessed = 0;
							nRes = scopeInsert(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, sizeof(n1), bContentAlreadyProcessed);
							//nRes = scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef), pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, sizeof(n1), bContentAlreadyProcessed);
							if ( nRes )
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
								wprintf(L"xobjcontentkeyvalue -> INSERITO XOBJ Key = '%s' -> %d %d R NELLO SCOPE\n", pParams->szCurrResourcesKeyName, n1, n2);
								#endif
							}
						}
					}
					else
					{
						pParams->bInXObj = 0;						
						pParams->nXObjRef = n1;
					}
				}
				
				if ( pParams->bInFontObj )
				{
					if ( 0 == pParams->bFontObjIsIndirect )
					{
						Scope PageScope;
						int nRes;
						size_t len;
						uint32_t nDataSize;
						uint32_t bContentAlreadyProcessed;						
						
						PageScope = pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef;
						len = strnlen(pParams->szCurrResourcesKeyName, 128);
						nRes = scopeFind(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, &nDataSize, &bContentAlreadyProcessed, 1);
						//nRes = scopeFind(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef), pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, &nDataSize, &bContentAlreadyProcessed, 1);
						if ( nRes < 0 ) // NON TROVATO
						{
							bContentAlreadyProcessed = 0;
							nRes = scopeInsert(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, sizeof(n1), bContentAlreadyProcessed);
							//nRes = scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef), pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, sizeof(n1), bContentAlreadyProcessed);
							if ( nRes )
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
								wprintf(L"xobjcontentkeyvalue -> INSERITO FONT Key = '%s' -> %d %d R NELLO SCOPE\n", pParams->szCurrResourcesKeyName, n1, n2);
								#endif
							}
						}
					}
					else
					{
						pParams->bInFontObj = 0;						
						pParams->nFontObjRef = n1;
					}
				}				
			}
			break;
		case T_NAME:
			if ( (strncmp(pParams->szCurrKeyName, "Filter", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
			{
				mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.Value.vString);
				if ( (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
					pParams->CurrentContent.bExternalFile = 1;
				pParams->nCountFilters++;
			}
			else if ( strncmp(pParams->szCurrKeyName, "Subtype", 1024) == 0 )
			{
				//if ( strncmp(pParams->myToken.Value.vString, "Image", 128) == 0 )
				if ( 'I' == pParams->myToken.Value.vString[0] )
				{
					pParams->bXObjIsImage = 1;
					return 1;
				}
				
				pParams->bXObjIsImage = 0;
			}	
									
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			wprintf(L"xobjcontentkeyvalue -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			break;
		case T_STRING_LITERAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			wprintf(L"xobjcontentkeyvalue -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING_HEXADECIMAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)	
			wprintf(L"xobjcontentkeyvalue -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);			
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_KW_TRUE:
		case T_KW_FALSE:
			// IGNORIAMO
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			wprintf(L"xobjcontentkeyvalue -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			GetNextToken(pParams);
			break;		
		case T_STRING: // File Specifications: vedi su PDF3000_2008 a pag. 99
			// IGNORIAMO
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			wprintf(L"xobjcontentkeyvalue -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			GetNextToken(pParams);
			break;
		case T_QOPAREN:
			if ( !xobjcontentkeyarray(pParams) )
				return 0;
			//GetNextToken(pParams);
			break;
		case T_DICT_BEGIN:
			pParams->bXObjIsIndirect = 0;
			pParams->bFontObjIsIndirect = 0;
			if ( !xobjcontentkeydict(pParams) )
				return 0;
			pParams->bXObjIsIndirect = 1;
			pParams->bFontObjIsIndirect = 1;
			if ( (strncmp(pParams->szCurrKeyName, "DecodeParms", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FDecodeParms", 1024) == 0) )
				pParams->nCountDecodeParams++;
			//GetNextToken(pParams);
			break;
		default:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			wprintf(L"ERRORE xobjcontentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			#endif
			snprintf(pParams->szError, 8192, "ERRORE xobjcontentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//fwprintf(pParams->fpErrors, L"ERRORE xobjcontentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
			break;
	}
	
	return 1;
}

// xobjcontentkeyarray : T_QOPAREN xobjcontentkeyvalue T_QCPAREN;
int xobjcontentkeyarray(Params *pParams)
{
	int countOpen = 0;
	//int countClose = 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	wprintf(L"xobjcontentkeyarray -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
		
	if ( !match(pParams, T_QOPAREN, "xobycontentkeyarray") )
		return 0;
		
	countOpen = 1;
		
	//if ( !xobjcontentkeyvalue(pParams) )
	//	return 0;
		
	ricomincia:
	while ( T_QCPAREN != pParams->myToken.Type )
	{			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
		wprintf(L"xobjcontentkeyarray -> ");	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);
		
		if ( T_QOPAREN == pParams->myToken.Type )
			countOpen++;
		else if ( T_QCPAREN == pParams->myToken.Type )
			countOpen--;
	}
	
	if ( countOpen > 0 )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
		wprintf(L"xobjcontentkeyarray -> ");
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);
				
		goto ricomincia;
	}

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	wprintf(L"xobjcontentkeyarray -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	
	if ( !match(pParams, T_QCPAREN, "xobjcontentkeyarray") )
		return 0;
		
	return 1;
}

// xobjcontentkeydict  : T_DICT_BEGIN { T_NAME xobjcontentkeyvalue } T_DICT_END;
int xobjcontentkeydict(Params *pParams)
{
	int len = 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)	
	wprintf(L"xobjcontentkeydict -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_DICT_BEGIN, "xobjcontentkeydict") )
	{
		return 0;
	}	
		
	while ( pParams->myToken.Type == T_NAME )
	{	
		
		if ( strncmp(pParams->szCurrKeyName, "Resources", 1024) == 0 )
		{
			if ( strncmp(pParams->myToken.Value.vString, "XObject", 1024) == 0)
				pParams->bInXObj = 1;
			if ( strncmp(pParams->myToken.Value.vString, "Font", 1024) == 0)
				pParams->bInFontObj = 1;
		}
		else
		{
			pParams->bInXObj = 0;
			pParams->bInFontObj = 0;
			pParams->szCurrResourcesKeyName[0] = '\0';
		}
					
		if ( pParams->bInXObj || pParams->bInFontObj )	
		{
			strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
		}		
		
		if ( NULL != pParams->myDataDecodeParams.pszKey )
		{
			free(pParams->myDataDecodeParams.pszKey);
			pParams->myDataDecodeParams.pszKey = NULL;
		}
		len = strnlen(pParams->myToken.Value.vString, 1024);	
		pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));
		if ( NULL == pParams->myDataDecodeParams.pszKey )
		{
			snprintf(pParams->szError, 8192, "ERRORE xobjcontentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE xobjcontentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			//fwprintf(pParams->fpErrors, L"ERRORE xobjcontentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			return 0;
		}
		strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.Value.vString, len + sizeof(char));
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)	
		wprintf(L"xobjcontentkeydict -> ");
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);	
								
		if ( !xobjcontentkeyvalue(pParams) )
		{
			return 0;
		}		
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	wprintf(L"xobjcontentkeydict -> ");	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_DICT_END, "xobjcontentkeydict") )
	{
		return 0;
	}
	
	pParams->bInXObj = 0;
	pParams->bInFontObj = 0;
	pParams->szCurrResourcesKeyName[0] = '\0';
	
	return 1;	
}

// ************************************************************************************************************************

// contentfontobj         : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ contentfontobjbody T_KW_ENDOBJ;
int contentfontobj(Params *pParams)
{
	int k;
	char szFontType[128];
	
	pParams->bEncodigArrayAlreadyInit = 0;
	pParams->bCurrentFontHasDirectEncodingArray = 0;
	pParams->nCurrentEncodingObj = 0;
	pParams->nToUnicodeStreamObjRef = 0;
	pParams->szTemp[0] = '\0';
	
	pParams->nCurrentUseCMapRef = 0;
	pParams->szUseCMap[0] = '\0';
	
	for ( k = 0; k < 256; k++ )
		pParams->paCustomizedFont_CharSet[k] = 0;
	
	
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE contentfontobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentfontobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE contentfontobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.Value.vInt;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	GetNextToken(pParams);
			
	//if ( !match(pParams, T_INT_LITERAL, "contentfontobj") )
	//	return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	if ( !match(pParams, T_INT_LITERAL, "contentfontobj") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_KW_OBJ, "contentfontobj") )
		return 0;
	
	if ( !contentfontobjbody(pParams) )
		return 0;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_ENDOBJ, "contentfontobj") )
		return 0;
	
	pParams->bHasCodeSpaceTwoByte = 0;
	pParams->bHasCodeSpaceOneByte = 1;
	
	switch ( pParams->nCurrentFontSubtype )
	{
		case FONT_SUBTYPE_Type0:
			strncpy(szFontType, "Type0", 6);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings_HEXADECIMAL)
			wprintf(L"\n\n***** FONT Type0 ***** <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			#endif
			break;
		case FONT_SUBTYPE_Type1:
			strncpy(szFontType, "Type1", 6);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings_HEXADECIMAL)
			wprintf(L"\n\n***** FONT Type1 ***** <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			#endif
			break;
		case FONT_SUBTYPE_MMType1:
			strncpy(szFontType, "MMType1", 8);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings_HEXADECIMAL)
			wprintf(L"\n\n***** FONT MMType1 ***** <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			#endif
			break;
		case FONT_SUBTYPE_Type3:
			strncpy(szFontType, "Type3", 6);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings_HEXADECIMAL)
			wprintf(L"\n\n***** FONT Type3 *****\n <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			#endif
			break;
		case FONT_SUBTYPE_TrueType:
			strncpy(szFontType, "TrueType", 9);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings_HEXADECIMAL)
			wprintf(L"\n\n***** FONT TrueType ***** <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			#endif
			break;
		case FONT_SUBTYPE_CIDFontType0:
			strncpy(szFontType, "CIDFontType0", 13);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings_HEXADECIMAL)
			wprintf(L"\n\n***** FONT CIDFontType0 ***** <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			#endif
			break;
		case FONT_SUBTYPE_CIDFontType2:
			strncpy(szFontType, "CIDFontType2", 13);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings_HEXADECIMAL)
			wprintf(L"\n\n***** FONT CIDFontType2 ***** <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			#endif
			break;
		default:
			szFontType[0] = '\0';
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings_HEXADECIMAL)
			wprintf(L"\n\n***** EHI!!! UNKNOWN FONT ***** <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			#endif
			break;
	}

	if ( FONT_SUBTYPE_Type0 == pParams->nCurrentFontSubtype && pParams->nToUnicodeStreamObjRef <= 0 )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		wprintf(L"\t***** 1 pParams->nToUnicodeStreamObjRef = %d, pParams->nCurrentUseCMapRef = %d *****\n", pParams->nToUnicodeStreamObjRef, pParams->nCurrentUseCMapRef);
		#endif
				
		if ( pParams->nCurrentEncodingObj > 0 )
		{
			for ( int i = 0; i < pParams->dimCustomizedFont_CharSet; i++ )
				pParams->paCustomizedFont_CharSet[i] = pParams->pArrayUnicode[i];
				
			for ( int j = pParams->dimCustomizedFont_CharSet; j < 0xFFFF; j++ )
				pParams->paCustomizedFont_CharSet[j] = 0xFFFD;				
				
			pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
							
			if ( pParams->nCurrentUseCMapRef > 0 )
			{
				pParams->bEncodigArrayAlreadyInit = 0;
				
				pParams->bStreamType = STREAM_TYPE_CMAP;
				if ( !ParseCMapObject(pParams, pParams->nCurrentUseCMapRef) )
					return 0;
					
				pParams->bEncodigArrayAlreadyInit = 1;			
			}
			else if ( pParams->szUseCMap[0] != '\0' )   // QUI GESTIONE CMAP PREDEFINITO
			{							
				// Identity-H Identity-V
				if ( (strncmp(pParams->szUseCMap, "Identity-H", strnlen(pParams->szUseCMap, 4096) + 1) == 0) || (strncmp(pParams->szUseCMap, "Identity-V", strnlen(pParams->szUseCMap, 4096) + 1) == 0) )
				{
					pParams->pCurrentEncodingArray = &(pParams->pArrayUnicode[0]);
					
					pParams->bHasCodeSpaceOneByte = 0;
					pParams->bHasCodeSpaceTwoByte = 1;
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					wprintf(L"\t***** USECMAP PREDEFINITO 0 -> '%s'. OK! *****\n\n", pParams->szUseCMap);
					#endif
					return 1;
				}				
				else
				{
					MyPredefinedCMapDef myData1;
					MyPredefinedCMapDef myData2;
					uint32_t myData1Size;
					uint32_t myData2Size;
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					wprintf(L"\t***** USECMAP PREDEFINITO 1 -> '%s' *****\n\n", pParams->szUseCMap);
					#endif
					
					pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
					
					myData1.szUseCMap[0] = '\0';
					myData1.pszDecodedStream = NULL;
					myData1.DecodedStreamSize = 0;
					
					myData2.szUseCMap[0] = '\0';
					myData2.pszDecodedStream = NULL;
					myData2.DecodedStreamSize = 0;
					
					if ( genhtFind(&(pParams->myCMapHT), pParams->szUseCMap, sizeof(pParams->szUseCMap), &myData1, &myData1Size ) >= 0 )
					{
						pParams->bEncodigArrayAlreadyInit = 0;
						pParams->bStreamType = STREAM_TYPE_CMAP;
						
						if ( '\0' != myData1.szUseCMap[0] )
						{
							if ( genhtFind(&(pParams->myCMapHT), myData1.szUseCMap, sizeof(myData1.szUseCMap), &myData2, &myData2Size ) >= 0 )
							{
								if ( !ParseCMapStream(pParams, 1000000, myData2.pszDecodedStream, myData2.DecodedStreamSize ) )
									return 0;
								pParams->bEncodigArrayAlreadyInit = 1;
							}
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
							else
							{
								wprintf(L"\t***** PREDEFINED CMAP 2 -> '%s', NOT FOUND!!! *****\n\n", myData1.szUseCMap);
							}
							#endif	
						}
						
						if ( !ParseCMapStream(pParams, 1000000, myData1.pszDecodedStream, myData1.DecodedStreamSize ) )
							return 0;
						pParams->bEncodigArrayAlreadyInit = 1;
					}
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					else
					{
						wprintf(L"\t***** PREDEFINED CMAP 1 -> '%s', NOT FOUND!!! *****\n\n", pParams->szUseCMap);
					}
					#endif				
				}
			}
		
			pParams->bStreamType = STREAM_TYPE_CMAP;
			if ( !ParseCMapObject(pParams, pParams->nCurrentEncodingObj) )
				return 0;
			
			pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
		}
		else   // QUI GESTIONE CMAP PREDEFINITO
		{
			pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
			
			// Identity-H Identity-V
			if ( (strncmp(pParams->szTemp, "Identity-H", strnlen(pParams->szTemp, 4096) + 1) == 0) || (strncmp(pParams->szTemp, "Identity-V", strnlen(pParams->szTemp, 4096) + 1) == 0) )
			{
				pParams->pCurrentEncodingArray = &(pParams->pArrayUnicode[0]);
				
				pParams->bHasCodeSpaceOneByte = 0;
				pParams->bHasCodeSpaceTwoByte = 1;

				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\t***** USECMAP USECMAP PREDEFINITO -> '%s'. OK! *****\n\n", pParams->szTemp);
				#endif
				return 1;
			}			
			else
			{
				MyPredefinedCMapDef myData1;
				MyPredefinedCMapDef myData2;
				uint32_t myData1Size;
				uint32_t myData2Size;

				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\t***** USECMAP PREDEFINITO 2 -> '%s' *****\n\n", pParams->szTemp);
				#endif
				
				pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
					
				myData1.szUseCMap[0] = '\0';
				myData1.pszDecodedStream = NULL;
				myData1.DecodedStreamSize = 0;
					
				myData2.szUseCMap[0] = '\0';
				myData2.pszDecodedStream = NULL;
				myData2.DecodedStreamSize = 0;
					
				if ( genhtFind(&(pParams->myCMapHT), pParams->szTemp, sizeof(pParams->szTemp), &myData1, &myData1Size ) >= 0 )
				{
					pParams->bEncodigArrayAlreadyInit = 0;
					pParams->bStreamType = STREAM_TYPE_CMAP;
						
					if ( '\0' != myData1.szUseCMap[0] )
					{
						if ( genhtFind(&(pParams->myCMapHT), myData1.szUseCMap, sizeof(myData1.szUseCMap), &myData2, &myData2Size ) >= 0 )
						{
							if ( !ParseCMapStream(pParams, 1000000, myData2.pszDecodedStream, myData2.DecodedStreamSize ) )
								return 0;
							pParams->bEncodigArrayAlreadyInit = 1;
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
						else
						{
							wprintf(L"\t***** PREDEFINED CMAP 2 -> '%s', NOT FOUND!!! *****\n\n", myData1.szUseCMap);
						}
						#endif	
					}
						
					if ( !ParseCMapStream(pParams, 1000000, myData1.pszDecodedStream, myData1.DecodedStreamSize ) )
						return 0;
					pParams->bEncodigArrayAlreadyInit = 1;						
				}
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				else
				{
					wprintf(L"\t***** PREDEFINED CMAP 1 -> '%s', NOT FOUND!!! *****\n\n", pParams->szTemp);
				}
				#endif
			}
		}
		
		return 1;
	}
	
	if ( pParams->nToUnicodeStreamObjRef > 0 )
	{
		for ( int i = 0; i < pParams->dimCustomizedFont_CharSet; i++ )
			pParams->paCustomizedFont_CharSet[i] = pParams->pArrayUnicode[i];
			
		for ( int j = pParams->dimCustomizedFont_CharSet; j < 0xFFFF; j++ )
			pParams->paCustomizedFont_CharSet[j] = 0xFFFD;
			
		pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		wprintf(L"\t***** 2 pParams->nToUnicodeStreamObjRef = %d, pParams->nCurrentUseCMapRef = %d *****\n", pParams->nToUnicodeStreamObjRef, pParams->nCurrentUseCMapRef);
		#endif		
		
		if ( pParams->nCurrentUseCMapRef > 0 )
		{
			pParams->bEncodigArrayAlreadyInit = 0;
			
			pParams->bStreamType = STREAM_TYPE_TOUNICODE;
			if ( !ParseCMapObject(pParams, pParams->nCurrentUseCMapRef) )
				return 0;
			
			pParams->bEncodigArrayAlreadyInit = 1;			
		}
		else if ( pParams->szUseCMap[0] != '\0' )   // QUI GESTIONE USECMAP TOUNICODE PREDEFINITO
		{
			// Identity-H Identity-V
			if ( (strncmp(pParams->szUseCMap, "Identity-H", strnlen(pParams->szUseCMap, 4096) + 1) == 0) ||
			     (strncmp(pParams->szUseCMap, "Identity-V", strnlen(pParams->szUseCMap, 4096) + 1) == 0)
			   )
			{
				pParams->pCurrentEncodingArray = &(pParams->pArrayUnicode[0]);
				
				pParams->bHasCodeSpaceOneByte = 0;
				pParams->bHasCodeSpaceTwoByte = 1;
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\t***** USECMAP TOUNICODE PREDEFINITO -> '%s'. OK! *****\n\n", pParams->szUseCMap);
				#endif
				return 1;
			}
			else
			{
				MyPredefinedCMapDef myData1;
				MyPredefinedCMapDef myData2;
				uint32_t myData1Size;
				uint32_t myData2Size;
					
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\t***** USECMAP TOUNICODE PREDEFINITO -> '%s' *****\n\n", pParams->szUseCMap);
				#endif
				
				pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
					
				myData1.szUseCMap[0] = '\0';
				myData1.pszDecodedStream = NULL;
				myData1.DecodedStreamSize = 0;
					
				myData2.szUseCMap[0] = '\0';
				myData2.pszDecodedStream = NULL;
				myData2.DecodedStreamSize = 0;
					
				if ( genhtFind(&(pParams->myCMapHT), pParams->szUseCMap, sizeof(pParams->szUseCMap), &myData1, &myData1Size ) >= 0 )
				{
					pParams->bEncodigArrayAlreadyInit = 0;
					pParams->bStreamType = STREAM_TYPE_CMAP;
						
					if ( '\0' != myData1.szUseCMap[0] )
					{
						if ( genhtFind(&(pParams->myCMapHT), myData1.szUseCMap, sizeof(myData1.szUseCMap), &myData2, &myData2Size ) >= 0 )
						{
							if ( !ParseCMapStream(pParams, 1000000, myData2.pszDecodedStream, myData2.DecodedStreamSize ) )
								return 0;
							pParams->bEncodigArrayAlreadyInit = 1;
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
						else
						{
							wprintf(L"\t***** PREDEFINED TOUNICODE CMAP 2 -> '%s', NOT FOUND!!! *****\n\n", myData1.szUseCMap);
						}
						#endif	
					}
						
					if ( !ParseCMapStream(pParams, 1000000, myData1.pszDecodedStream, myData1.DecodedStreamSize ) )
						return 0;
					pParams->bEncodigArrayAlreadyInit = 1;
				}
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				else
				{
					wprintf(L"\t***** PREDEFINED TOUNICODE CMAP 1 -> '%s', NOT FOUND!!! *****\n\n", pParams->szUseCMap);
				}
				#endif
			}
		}		
		
		pParams->bStreamType = STREAM_TYPE_TOUNICODE;
		if ( !ParseCMapObject(pParams, pParams->nToUnicodeStreamObjRef) )
			return 0;
			
		pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
	}	
	else if ( pParams->nCurrentEncodingObj > 0 )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		wprintf(L"\t***** pParams->nCurrentEncodingObj = %d *****\n\n", pParams->nCurrentEncodingObj);
		#endif
				
		if ( !ParseEncodingObject(pParams, pParams->nCurrentEncodingObj) )
			return 0;
	}
	else if ( pParams->bCurrentFontHasDirectEncodingArray )
	{
		int k;
		// MacRomanEncoding, MacExpertEncoding, or WinAnsiEncoding
		if ( strncmp(pParams->szTemp, "WinAnsiEncoding", strnlen(pParams->szTemp, 4096) + 1) == 0 )
		{
			for ( k = 0; k < 256; k++ )
			{
				if ( pParams->paCustomizedFont_CharSet[k] == 0 )
				{
					pParams->paCustomizedFont_CharSet[k] = pParams->aWIN_CharSet[k];
				}
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\t***** FONT 'WinAnsiEncoding'. OK! *****\n\n");
			#endif			
		}
		else if ( strncmp(pParams->szTemp, "MacRomanEncoding", strnlen(pParams->szTemp, 4096) + 1) == 0 )
		{
			for ( k = 0; k < 256; k++ )
			{
				if ( pParams->paCustomizedFont_CharSet[k] == 0 )
					pParams->paCustomizedFont_CharSet[k] = pParams->aMAC_CharSet[k];
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\t***** FONT 'MacRomanEncoding'. OK! *****\n\n");
			#endif			
		}	
		else if ( strncmp(pParams->szTemp, "MacExpertEncoding", strnlen(pParams->szTemp, 4096) + 1) == 0 )
		{
			for ( k = 0; k < 256; k++ )
			{
				if ( pParams->paCustomizedFont_CharSet[k] == 0 )
					pParams->paCustomizedFont_CharSet[k] = pParams->aMACEXP_CharSet[k];
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\t***** FONT 'MacExpertEncoding'. OK! *****\n\n");
			#endif			
		}
		else
		{
			for ( k = 0; k < 256; k++ )
			{
				if ( pParams->paCustomizedFont_CharSet[k] == 0 )
					pParams->paCustomizedFont_CharSet[k] = pParams->aSTD_CharSet[k];
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\t***** FONT 'PDF STANDARD'. OK! *****\n\n");
			#endif			
		}

		pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
	}
	else
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		wprintf(L"\tcontentfontobj ENCODING -> '%s'\n", pParams->szTemp);
		#endif
		
		// MacRomanEncoding, MacExpertEncoding, or WinAnsiEncoding
		if ( strncmp(pParams->szTemp, "WinAnsiEncoding", strnlen("WinAnsiEncoding", 4096) + 1) == 0 )
		{
			pParams->pCurrentEncodingArray = &(pParams->aWIN_CharSet[0]);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\t***** FONT 'WinAnsiEncoding'. OK! *****\n\n");
			#endif			
		}
		else if ( strncmp(pParams->szTemp, "MacRomanEncoding", strnlen("MacRomanEncoding", 4096) + 1) == 0 )
		{
			pParams->pCurrentEncodingArray = &(pParams->aMAC_CharSet[0]);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\t***** FONT 'MacRomanEncoding'. OK! *****\n\n");
			#endif			
		}
		else if ( strncmp(pParams->szTemp, "MacExpertEncoding", strnlen("MacExpertEncoding", 4096) + 1) == 0 )
		{
			pParams->pCurrentEncodingArray = &(pParams->aMACEXP_CharSet[0]);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\t***** FONT 'MacExpertEncoding'. OK! *****\n\n");
			#endif			
		}
		else
		{
			pParams->pCurrentEncodingArray = &(pParams->aSTD_CharSet[0]);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\t***** FONT 'PDF STANDARD'. OK! *****\n\n");
			#endif
		}
	}
			
	return 1;
}

// contentfontobjbody     : T_DICT_BEGIN fontobjstreamdictitems T_DICT_END;
int contentfontobjbody(Params *pParams)
{
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_BEGIN, "contentfontobjbody") )
		return 0;
	
	if ( !fontobjstreamdictitems(pParams) )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_END, "contentfontobjbody") )
		return 0;
	
	return 1;
}

// fontobjstreamdictitems : {T_NAME fontobjcontentkeyvalue};
int fontobjstreamdictitems(Params *pParams)
{
	while ( pParams->myToken.Type == T_NAME )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
				
		GetNextToken(pParams);			
				
		if ( !fontobjcontentkeyvalue(pParams) )
			return 0;
	}
	
	return 1;
}

//fontobjcontentkeyvalue : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
//                       | T_NAME
//                       | fontobjcontentkeyarray
//                       | fontobjcontentkeydict
//                       ;
int fontobjcontentkeyvalue(Params *pParams)
{
	int n1 = -1;
	int n2 = -1;
		
	switch ( pParams->myToken.Type )
	{
		case T_INT_LITERAL:
			n1 = pParams->myToken.Value.vInt;
						
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.Value.vInt;
							
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				GetNextToken(pParams);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif				
				
				if ( !match(pParams, T_KW_R, "fontobjcontentkeyvalue") )
					return 0;
			}
			
			if ( n2 >= 0 )
			{
				if ( strncmp(pParams->szCurrKeyName, "Encoding", 1024) == 0 )
				{
					pParams->nCurrentEncodingObj = n1;
					pParams->szTemp[0] = '0';			
				}
				else if ( strncmp(pParams->szCurrKeyName, "ToUnicode", 1024) == 0 )
				{
					pParams->nToUnicodeStreamObjRef = n1;
					pParams->szTemp[0] = '0';			
				}				
			}
			
			break;
		case T_NAME:
			if ( strncmp(pParams->szCurrKeyName, "Encoding", 1024) == 0 )
			{
				// MacRomanEncoding, MacExpertEncoding, or WinAnsiEncoding
				strncpy(pParams->szTemp, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
				pParams->nCurrentEncodingObj = 0;
			}			
			else if ( strncmp(pParams->szCurrKeyName, "Subtype", 1024) == 0 )
			{
				if ( strncmp(pParams->myToken.Value.vString, "Type1", strnlen(pParams->myToken.Value.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_Type1;
				else if ( strncmp(pParams->myToken.Value.vString, "Type0", strnlen(pParams->myToken.Value.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_Type0;
				else if ( strncmp(pParams->myToken.Value.vString, "Type3", strnlen(pParams->myToken.Value.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_Type3;
				else if ( strncmp(pParams->myToken.Value.vString, "MMType1", strnlen(pParams->myToken.Value.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_MMType1;
				else if ( strncmp(pParams->myToken.Value.vString, "TrueType", strnlen(pParams->myToken.Value.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_TrueType;
				else if ( strncmp(pParams->myToken.Value.vString, "CIDFontType0", strnlen(pParams->myToken.Value.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_CIDFontType0;
				else if ( strncmp(pParams->myToken.Value.vString, "CIDFontType2", strnlen(pParams->myToken.Value.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_CIDFontType2;					
			}

			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			break;
		case T_QOPAREN:
			if ( !fontobjcontentkeyarray(pParams) )
				return 0;
			break;
		case T_DICT_BEGIN:
			if ( strncmp(pParams->szCurrKeyName, "Encoding", 1024) == 0 )
			{
				pParams->nCurrentEncodingObj = -1;
			}			
			if ( !fontobjcontentkeydict(pParams) )
				return 0;
			break;
		default:
			snprintf(pParams->szError, 8192, "ERRORE fontobjcontentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE fontobjcontentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE fontobjcontentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;			
			break;
	}
	
	return 1;
}

//encodingobjarray              : T_QOPAREN { T_INT_LITERAL T_NAME {T_NAME} } T_QCPAREN;
int fontdirectencodingobjarray(Params *pParams)
{	
	int keyValue;
	int nRes;
	uint32_t nData;
	uint32_t nDataSize;
	uint32_t bContentAlreadyProcessed;
	size_t len;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
			
	if ( !match(pParams, T_QOPAREN, "fontdirectencodingobjarray") )
	{
		return 0;
	}
		
	while ( pParams->myToken.Type == T_INT_LITERAL )
	{	
		keyValue = pParams->myToken.Value.vInt;
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);
		
		if ( pParams->myToken.Type != T_NAME )
		{
			snprintf(pParams->szError, 8192, "ERRORE fontdirectencodingobjarray: Atteso T_NAME, trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE fontdirectencodingobjarray: Atteso T_NAME, trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE fontdirectencodingobjarray: Atteso T_NAME, trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;			
		}
		
		len = strnlen(pParams->myToken.Value.vString, 4096);
		nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.Value.vString, len + sizeof(char), (void*)&nData, &nDataSize, &bContentAlreadyProcessed);
		if ( nRes >= 0 ) // TROVATO
		{				
			pParams->paCustomizedFont_CharSet[keyValue] = nData;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\tfontdirectencodingobjarray KEY -> '%s' -> Value = %d -> char = '%lc'\n", pParams->myToken.Value.vString, keyValue, (wchar_t)pParams->paCustomizedFont_CharSet[keyValue]);
			#endif			
		}
		else
		{
			//wprintf(L"encodingobjarray -> WARNING: KEY '%s' NON TROVATA\n", pParams->myToken.Value.vString);
			pParams->paCustomizedFont_CharSet[keyValue] = L' ';
		}
		
		GetNextToken(pParams);
		
		while ( pParams->myToken.Type == T_NAME )
		{
			keyValue++;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif				
			
			len = strnlen(pParams->myToken.Value.vString, 4096);
			nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.Value.vString, len + sizeof(char), (void*)&nData, &nDataSize, &bContentAlreadyProcessed);
			if ( nRes >= 0 ) // TROVATO
			{							
				pParams->paCustomizedFont_CharSet[keyValue] = nData;
								
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\tfontdirectencodingobjarray KEY -> '%s' -> Value = %d -> char = '%lc'\n", pParams->myToken.Value.vString, keyValue, (wchar_t)pParams->paCustomizedFont_CharSet[keyValue]);
				#endif			
			}
			else
			{
				pParams->paCustomizedFont_CharSet[keyValue] = L' ';
			}
			
			GetNextToken(pParams);
		}
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
			
	if ( !match(pParams, T_QCPAREN, "fontdirectencodingobjarray") )
	{
		return 0;
	}	
	
	return 1;
}

// fontobjcontentkeyarray : T_QOPAREN { T_INT_LITERAL [T_INT_LITERAL T_KW_R ] | T_REAL_LITERAL} T_QCPAREN;
int fontobjcontentkeyarray(Params *pParams)
{
	int n1 = -1;
	int n2 = -1;
	
	
	if ( strncmp(pParams->szCurrKeyName, "Encoding", 1024) == 0 )
	{
		pParams->bCurrentFontHasDirectEncodingArray = 1;
		return fontdirectencodingobjarray(pParams);
	}	
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_QOPAREN, "fontobjcontentkeyarray") )
	{
		return 0;
	}
		
	while ( pParams->myToken.Type == T_INT_LITERAL || pParams->myToken.Type == T_REAL_LITERAL )
	{	
		if ( T_INT_LITERAL == pParams->myToken.Type )
		{
			n1 = pParams->myToken.Value.vInt;
		}
		else
		{
			n1 = n2 = -1;
		}
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
		
		GetNextToken(pParams);
		
		if ( n1 > 0 && pParams->myToken.Type == T_INT_LITERAL )
		{
			n2 = pParams->myToken.Value.vInt;
							
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
				
			GetNextToken(pParams);
				
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif				
				
			if ( n2 >= 0 )
			{
				if ( pParams->myToken.Type == T_KW_R )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					PrintToken(&(pParams->myToken), ' ', ' ', 1);
					#endif
					
					GetNextToken(pParams);
				}
			}
		}
		else
		{
			n2 = -1;
		}		
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_QCPAREN, "fontobjcontentkeyarray") )
	{
		return 0;
	}	
	
	return 1;
}


//fontobjcontentkeydict  : T_DICT_BEGIN { T_NAME fontobjcontentkeyvalue } T_DICT_END;
int fontobjcontentkeydict(Params *pParams)
{
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	// Inutile matchare qui. Già matchato in fontobjcontentkeyvalue
	//if ( !match(pParams, T_DICT_BEGIN, "fontobjcontentkeydict") )
	//{
	//	return 0;
	//}	
	GetNextToken(pParams);	
		
	while ( pParams->myToken.Type == T_NAME )
	{		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
					
		GetNextToken(pParams);							
		
		if ( !fontobjcontentkeyvalue(pParams) )
			return 0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	
	if ( !match(pParams, T_DICT_END, "fontobjcontentkeydict") )
	{
		return 0;
	}	
	
	return 1;
}

// ------------------------------------------------------------------------------------------------------------------------

//encodingobj                   : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ encodingobjbody T_KW_ENDOBJ;
int encodingobj(Params *pParams)
{
	int k;
	
	pParams->nCurrentEncodingObj = 0;
	pParams->szTemp[0] = '\0';
	
	for ( k = 0; k < 256; k++ )
		pParams->paCustomizedFont_CharSet[k] = 0;	
	
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE encodingobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE encodingobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE encodingobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.Value.vInt;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	GetNextToken(pParams);
		
	//if ( !match(pParams, T_INT_LITERAL, "encodingobj") )
	//	return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	
	if ( !match(pParams, T_INT_LITERAL, "encodingobj") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_KW_OBJ, "encodingobj") )
		return 0;
	
	if ( !encodingobjbody(pParams) )
		return 0;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_ENDOBJ, "encodingobj") )
		return 0;
		
	// MacRomanEncoding, MacExpertEncoding, or WinAnsiEncoding
	if ( strncmp(pParams->szTemp, "WinAnsiEncoding", strnlen(pParams->szTemp, 4096) + 1) == 0 )
	{
		for ( k = 0; k < 256; k++ )
		{
			if ( pParams->paCustomizedFont_CharSet[k] == 0 )
			{
				pParams->paCustomizedFont_CharSet[k] = pParams->aWIN_CharSet[k];
			}
		}
	}
	else if ( strncmp(pParams->szTemp, "MacRomanEncoding", strnlen(pParams->szTemp, 4096) + 1) == 0 )
	{
		for ( k = 0; k < 256; k++ )
		{
			if ( pParams->paCustomizedFont_CharSet[k] == 0 )
				pParams->paCustomizedFont_CharSet[k] = pParams->aMAC_CharSet[k];
		}
	}	
	else if ( strncmp(pParams->szTemp, "MacExpertEncoding", strnlen(pParams->szTemp, 4096) + 1) == 0 )
	{
		for ( k = 0; k < 256; k++ )
		{
			if ( pParams->paCustomizedFont_CharSet[k] == 0 )
				pParams->paCustomizedFont_CharSet[k] = pParams->aMACEXP_CharSet[k];
		}
	}
	else
	{
		for ( k = 0; k < 256; k++ )
		{
			if ( pParams->paCustomizedFont_CharSet[k] == 0 )
				pParams->paCustomizedFont_CharSet[k] = pParams->aSTD_CharSet[k];
		}		
	}

	pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
	
	return 1;
}

//encodingobjbody               : T_NAME | T_DICT_BEGIN encodingobjdictitems T_DICT_END;
int encodingobjbody(Params *pParams)
{			
	if ( pParams->myToken.Type == T_NAME )
	{		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
			
		// MacRomanEncoding, MacExpertEncoding, or WinAnsiEncoding
		if ( strncmp(pParams->szTemp, "WinAnsiEncoding", strnlen(pParams->myToken.Value.vString, 4096) + 1) == 0 )
		{
			pParams->pCurrentEncodingArray = &(pParams->aWIN_CharSet[0]);
		}
		else if ( strncmp(pParams->szTemp, "MacRomanEncoding", strnlen(pParams->myToken.Value.vString, 4096) + 1) == 0 )
		{
			pParams->pCurrentEncodingArray = &(pParams->aMAC_CharSet[0]);
		}
		else if ( strncmp(pParams->szTemp, "MacExpertEncoding", strnlen(pParams->myToken.Value.vString, 4096) + 1) == 0 )
		{
			pParams->pCurrentEncodingArray = &(pParams->aMACEXP_CharSet[0]);
		}
		else
		{
			pParams->pCurrentEncodingArray = &(pParams->aSTD_CharSet[0]);
		}
		
		GetNextToken(pParams);
	}
	else if ( pParams->myToken.Type == T_DICT_BEGIN )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif	
				
		GetNextToken(pParams);
		
		pParams->szTemp[0] = '\0';
		if ( !encodingobjdictitems(pParams) )
			return 0;
			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
	
		if ( !match(pParams, T_DICT_END, "encodingobjbody") )
		{
			return 0;
		}				
	}
	else
	{
		snprintf(pParams->szError, 8192, "ERRORE encodingobjbody: Atteso uno di questi token: T_NAME, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE encodingobjbody: Atteso uno di questi token: T_NAME, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
		//fwprintf(pParams->fpErrors, L"ERRORE encodingobjbody: Atteso uno di questi token: T_NAME, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		return 0;		
	}
		
	return 1;
}

//encodingobjdictitems          : {T_NAME encodingobjdictitemskeyvalues};
int encodingobjdictitems(Params *pParams)
{
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	while ( pParams->myToken.Type == T_NAME )
	{		
		strncpy(pParams->szCurrKeyName, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
		wprintf(L"\tencodingobjdictitems -> pParams->szCurrKeyName = <%s>\n", pParams->szCurrKeyName);
		#endif
		
		GetNextToken(pParams);
		
		if ( !encodingobjdictitemskeyvalues(pParams) )
			return 0;
	}
		
	return 1;
}

//encodingobjdictitemskeyvalues : T_NAME 
//                              | encodingobjarray
//                              ;
int encodingobjdictitemskeyvalues(Params *pParams)
{	
	if ( pParams->myToken.Type == T_NAME )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
		
		if ( strncmp(pParams->szCurrKeyName, "BaseEncoding", 1024) == 0 )
		{
			strncpy(pParams->szTemp, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
		}
		
		GetNextToken(pParams);
	}
	else
	{		
		if ( !encodingobjarray(pParams) )
			return 0;
	}
		
	return 1;
}

//encodingobjarray              : T_QOPAREN { T_INT_LITERAL T_NAME {T_NAME} } T_QCPAREN;
int encodingobjarray(Params *pParams)
{	
	int keyValue;
	int nRes;
	uint32_t nData;
	uint32_t nDataSize;
	uint32_t bContentAlreadyProcessed;
	size_t len;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
			
	if ( !match(pParams, T_QOPAREN, "encodingobjarray") )
	{
		return 0;
	}
		
	while ( pParams->myToken.Type == T_INT_LITERAL )
	{	
		keyValue = pParams->myToken.Value.vInt;
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);
		
		if ( pParams->myToken.Type != T_NAME )
		{
			snprintf(pParams->szError, 8192, "ERRORE encodingobjarray: Atteso T_NAME, trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE encodingobjarray: Atteso T_NAME, trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE encodingobjarray: Atteso T_NAME, trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;			
		}
		
		len = strnlen(pParams->myToken.Value.vString, 4096);
		nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.Value.vString, len + sizeof(char), (void*)&nData, &nDataSize, &bContentAlreadyProcessed);
		if ( nRes >= 0 ) // TROVATO
		{				
			pParams->paCustomizedFont_CharSet[keyValue] = nData;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
			wprintf(L"\tencodingobjarray KEY -> '%s' -> Value = %d -> char = '%lc'\n", pParams->myToken.Value.vString, keyValue, (wchar_t)pParams->paCustomizedFont_CharSet[keyValue]);
			#endif			
		}
		else
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
			wprintf(L"encodingobjarray -> WARNING: KEY '%s' NON TROVATA\n", pParams->myToken.Value.vString);
			#endif
			pParams->paCustomizedFont_CharSet[keyValue] = L' ';
		}
		
		GetNextToken(pParams);
		
		while ( pParams->myToken.Type == T_NAME )
		{
			keyValue++;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif				
			
			len = strnlen(pParams->myToken.Value.vString, 4096);
			nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.Value.vString, len + sizeof(char), (void*)&nData, &nDataSize, &bContentAlreadyProcessed);
			if ( nRes >= 0 ) // TROVATO
			{							
				pParams->paCustomizedFont_CharSet[keyValue] = nData;
								
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
				wprintf(L"\tencodingobjarray KEY -> '%s' -> Value = %d -> char = '%lc'\n", pParams->myToken.Value.vString, keyValue, (wchar_t)pParams->paCustomizedFont_CharSet[keyValue]);
				#endif			
			}
			else
			{
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
				wprintf(L"encodingobjarray -> WARNING: KEY(%d) '%s' NON TROVATA\n", keyValue, pParams->myToken.Value.vString);
				#endif
				pParams->paCustomizedFont_CharSet[keyValue] = L' ';
			}
			
			GetNextToken(pParams);
		}
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
			
	if ( !match(pParams, T_QCPAREN, "encodingobjarray") )
	{
		return 0;
	}	
	
	return 1;
}

// ************************************************************************************************************************

//xrefstream_obj      : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ xrefstream_objbody T_KW_ENDOBJ;
int xrefstream_obj(Params *pParams)
{		
	pParams->nCountDecodeParams = pParams->nCountFilters = 0;
	
	pParams->myPdfTrailer.Prev = 0;
	//pParams->myPdfTrailer.Root.Number = 0;
	//pParams->myPdfTrailer.Root.Generation = 0;
	//pParams->myPdfTrailer.Size = 0;
			
	pParams->trailerW1 = -1;
	pParams->trailerW2 = -1;
	pParams->trailerW3 = -1;
	
	//myintqueuelist_Init( &(pParams->queueTrailerIndex) );
	//myintqueuelist_Init( &(pParams->queueTrailerW) );
	
	
	
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE xrefstream_obj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE xrefstream_obj: la chiave 'Length' e' assente.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_obj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.Value.vInt;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	GetNextToken(pParams);
	
	
				
	//if ( !match(pParams, T_INT_LITERAL, "xrefstream_obj") )
	//	return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_INT_LITERAL, "xrefstream_obj") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_KW_OBJ, "xrefstream_obj") )
		return 0;
		
	if ( !xrefstream_objbody(pParams) )
		return 0;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_ENDOBJ, "xrefstream_obj") )
		return 0;
		
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	wprintf(L"\n\n");
	#endif		
			
	if ( !(pParams->bStreamLengthIsPresent) )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
		wprintf(L"ERRORE xrefstream_obj: la chiave 'Lenght' e' assente.\n");
		#endif
		
		snprintf(pParams->szError, 8192, "ERRORE xrefstream_obj: la chiave 'Length' e' assente.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_obj: la chiave 'Length' e' assente.\n");
		return 0;
	}
	
	if ( NULL != pParams->myDataDecodeParams.pszKey )
	{
		free(pParams->myDataDecodeParams.pszKey);
		pParams->myDataDecodeParams.pszKey = NULL;
	}
	
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
	wprintf(L"\n\nOK xrefstream_obj, COMINCIO A IMPOSTARE I DATI...\n\n");
	#endif
		
	
	if ( pParams->queueTrailerW.count != 3 )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
		wprintf(L"ERRORE xrefstream_obj: la chiave 'W' e' assente.\n");
		#endif
		
		snprintf(pParams->szError, 8192, "ERRORE xrefstream_obj: la chiave 'W' e' assente.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_obj: la chiave 'W' e' assente.\n");
		return 0;
	}
	myintqueuelist_Dequeue(&(pParams->queueTrailerW), &(pParams->trailerW3) );
	myintqueuelist_Dequeue(&(pParams->queueTrailerW), &(pParams->trailerW2) );
	myintqueuelist_Dequeue(&(pParams->queueTrailerW), &(pParams->trailerW1) );	
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
	wprintf(L"\txrefstream_obj: pParams->trailerW1 = %d, pParams->trailerW2 = %d, pParams->trailerW3 = %d\n", pParams->trailerW1, pParams->trailerW2, pParams->trailerW3);
	wprintf(L"\txrefstream_obj: pParams->queueTrailerIndex.count = %d\n", pParams->queueTrailerIndex.count);
	#endif
	
	if ( pParams->queueTrailerIndex.count <= 0 )
	{
		pParams->myPdfTrailer.indexArraySize = 1;
		if ( NULL != pParams->myPdfTrailer.pIndexArray )
		{
			free(pParams->myPdfTrailer.pIndexArray);
			pParams->myPdfTrailer.pIndexArray = NULL;			
		}
		
		pParams->myPdfTrailer.pIndexArray = (TrailerIndex**)malloc(sizeof(TrailerIndex*));
		if ( NULL == pParams->myPdfTrailer.pIndexArray )
		{
			snprintf(pParams->szError, 8192, "ERRORE refstream_obj: impossibile allocare la memoria per pParams->myPdfTrailer.pIndexArray.\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE refstream_obj: impossibile allocare la memoria per pParams->myPdfTrailer.pIndexArray.\n");
			//fwprintf(pParams->fpErrors, L"ERRORE refstream_obj: impossibile allocare la memoria per pParams->myPdfTrailer.pIndexArray.\n");
			return 0;
		}
		
		pParams->myPdfTrailer.pIndexArray[0] = (TrailerIndex*)malloc(sizeof(TrailerIndex));
		if ( NULL == pParams->myPdfTrailer.pIndexArray[0] )
		{
			snprintf(pParams->szError, 8192, "ERRORE refstream_obj: impossibile allocare la memoria per pParams->myPdfTrailer.pIndexArray[0].\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE refstream_obj: impossibile allocare la memoria per pParams->myPdfTrailer.pIndexArray[0].\n");
			//fwprintf(pParams->fpErrors, L"ERRORE refstream_obj: impossibile allocare la memoria per pParams->myPdfTrailer.pIndexArray[0].\n");
			return 0;
		}
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
		else
		{
			wprintf(L"\tCORRETTAMENTE ALLOCATI %lu BYTE PER pParams->myPdfTrailer.pIndexArray[0]\n", sizeof(TrailerIndex));
		}
		#endif
			
		pParams->myPdfTrailer.pIndexArray[0]->FirstObjNumber = 0;
		pParams->myPdfTrailer.pIndexArray[0]->NumberOfEntries = pParams->myPdfTrailer.Size;
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
		wprintf(L"\txrefstream_obj: pParams->myPdfTrailer.pIndexArray[0]->FirstObjNumber = %d, pParams->myPdfTrailer.pIndexArray[0]->NumberOfEntries = %d\n", pParams->myPdfTrailer.pIndexArray[0]->FirstObjNumber, pParams->myPdfTrailer.pIndexArray[0]->NumberOfEntries);
		#endif		
	}
	else
	{
		int num1;
		int num2;
		int myIdx;
		
		if ( pParams->queueTrailerIndex.count % 2 != 0 )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
			wprintf(L"ERRORE refstream_obj: pParams->myPdfTrailer.pIndexArray deve contenere un numero pari di coppie di interi.\n");
			#endif
			
			snprintf(pParams->szError, 8192, "ERRORE refstream_obj: pParams->myPdfTrailer.pIndexArray deve contenere un numero pari di coppie di interi.\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			//fwprintf(pParams->fpErrors, L"ERRORE refstream_obj: pParams->myPdfTrailer.pIndexArray deve contenere un numero pari di coppie di interi.\n");
			return 0;
		}
		
		pParams->myPdfTrailer.indexArraySize = pParams->queueTrailerIndex.count / 2;
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
		wprintf(L"\txrefstream_obj: pParams->myPdfTrailer.indexArraySize(2) = %d\n", pParams->myPdfTrailer.indexArraySize);
		#endif
		
		if ( NULL != pParams->myPdfTrailer.pIndexArray )
		{
			free(pParams->myPdfTrailer.pIndexArray);
			pParams->myPdfTrailer.pIndexArray = NULL;
		}		
		
		pParams->myPdfTrailer.pIndexArray = (TrailerIndex**)malloc(sizeof(TrailerIndex*) * pParams->myPdfTrailer.indexArraySize);
		if ( NULL == pParams->myPdfTrailer.pIndexArray )
		{
			snprintf(pParams->szError, 8192, "ERRORE refstream_obj 1: impossibile allocare la memoria per pParams->myPdfTrailer.pIndexArray.\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE refstream_obj 1: impossibile allocare la memoria per pParams->myPdfTrailer.pIndexArray.\n");
			//fwprintf(pParams->fpErrors, L"ERRORE refstream_obj 2: impossibile allocare la memoria per pParams->myPdfTrailer.pIndexArray.\n");
			return 0;
		}
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
		else
		{
			wprintf(L"\tCORRETTAMENTE ALLOCATI %lu BYTE PER pParams->myPdfTrailer.pIndexArray\n", sizeof(TrailerIndex*) * pParams->myPdfTrailer.indexArraySize);
		}
		#endif
		
		myIdx = 0;
		while ( myintqueuelist_Dequeue( &(pParams->queueTrailerIndex), &num2) )
		{
			myintqueuelist_Dequeue( &(pParams->queueTrailerIndex), &num1);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
			wprintf(L"\txrefstream_obj: myIdx = %d -> num1 = %d, num2 = %d\n", myIdx, num1, num2);
			#endif
			
			pParams->myPdfTrailer.pIndexArray[myIdx] = (TrailerIndex*)malloc(sizeof(TrailerIndex));
			if ( NULL == pParams->myPdfTrailer.pIndexArray[myIdx] )
			{
				snprintf(pParams->szError, 8192, "ERRORE refstream_obj 2: impossibile allocare la memoria per pParams->myPdfTrailer.pIndexArray.\n");
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"ERRORE refstream_obj 2: impossibile allocare la memoria per pParams->myPdfTrailer.pIndexArray.\n");
				//fwprintf(pParams->fpErrors, L"ERRORE refstream_obj 2: impossibile allocare la memoria per pParams->myPdfTrailer.pIndexArray.\n");
				return 0;
			}
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
			else
			{
				wprintf(L"\tCORRETTAMENTE ALLOCATI %lu BYTE PER pParams->myPdfTrailer.pIndexArray[%d]\n", sizeof(TrailerIndex), myIdx);
			}
			#endif
			
			pParams->myPdfTrailer.pIndexArray[myIdx]->FirstObjNumber = num1;
			pParams->myPdfTrailer.pIndexArray[myIdx]->NumberOfEntries = num2;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
			wprintf(L"\txrefstream_obj: pParams->myPdfTrailer.pIndexArray[%d]->FirstObjNumber = %d, pParams->myPdfTrailer.pIndexArray[%d]->NumberOfEntries = %d\n", myIdx, pParams->myPdfTrailer.pIndexArray[myIdx]->FirstObjNumber, myIdx, pParams->myPdfTrailer.pIndexArray[myIdx]->NumberOfEntries);
			#endif
			
			myIdx++;
		}
	}
	
	myintqueuelist_Free( &(pParams->queueTrailerIndex) );
	myintqueuelist_Free( &(pParams->queueTrailerW) );
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
	wprintf(L"\n\nOK xrefstream_obj, OK, HO CORRETTAMENTE IMPOSTATO TUTTI I DATI:\n");
	wprintf(L"\tpParams->myPdfTrailer.Prev = %d\n", pParams->myPdfTrailer.Prev);
	wprintf(L"\tpParams->myPdfTrailer.Root.Number = %d\n", pParams->myPdfTrailer.Root.Number);
	wprintf(L"\tpParams->myPdfTrailer.Root.Generation = %d\n", pParams->myPdfTrailer.Root.Generation);
	wprintf(L"\tpParams->myPdfTrailer.Size = %d\n\n", pParams->myPdfTrailer.Size);
	#endif
	
	return 1;
}

// xrefstream_objbody  : T_DICT_BEGIN xrefstream_streamdictitems T_DICT_END T_KW_STREAM T_STRING T_KW_ENDSTREAM;
int xrefstream_objbody(Params *pParams)
{
	int retValue = 1;
	
	//long int pos1;
	//long int pos2;
	
	pParams->CurrentContent.bExternalFile = 0;
	pParams->CurrentContent.Offset = 0;
	//pParams->CurrentContent.Length = 0;
	pParams->CurrentContent.LengthFromPdf = 0;
	pParams->bStreamLengthIsPresent = 0;
	
	pParams->myDataDecodeParams.numFilter = 0;
	pParams->myDataDecodeParams.pszKey = NULL;
	
	mystringqueuelist_Init(&(pParams->CurrentContent.queueFilters));
	
	mydictionaryqueuelist_Init(&(pParams->CurrentContent.decodeParms), 1, 1);
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_BEGIN, "xrefstream_objbody") )
	{
		retValue = 0;
		goto uscita;
	}
				
	if ( !xrefstream_streamdictitems(pParams) )
	{
		retValue = 0;
		goto uscita;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_DICT_END, "xrefstream_objbody") )
	{
		retValue = 0;
		goto uscita;
	}
					
	if ( pParams->myToken.Type != T_KW_STREAM )
	{
		snprintf(pParams->szError, 8192, "ERRORE xrefstream_objbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE xrefstream_objbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_objbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		retValue = 0;
		goto uscita;
	}
	
	pParams->CurrentContent.Offset = pParams->nNumBytesReadFromCurrentStream;	
	pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamOffset = pParams->CurrentContent.Offset;
	/*
	if ( pParams->CurrentContent.LengthFromPdf != pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength )
	{
		fwprintf(pParams->fpErrors, L"WARNING xrefstream_objbody: pParams->CurrentContent.LengthFromPdf = %lu differs from pParams->myObjsTable[%d]->Obj.StreamLength = %lu\n",  pParams->CurrentContent.LengthFromPdf, pParams->nCurrentParsingObj, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength);
		//PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
		//pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength = pParams->CurrentContent.LengthFromPdf;
	}
	*/
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	//pos1 = ftell(pParams->fp);	
	//GetNextToken(pParams);
	if ( fseek(pParams->fp, pParams->CurrentContent.Offset + pParams->CurrentContent.LengthFromPdf, SEEK_SET) != 0 )
	{
		snprintf(pParams->szError, 8192, "ERRORE xrefstream_objbody: fseek\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE xrefstream_objbody: fseek\n");
		//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_objbody: fseek\n");
		retValue = 0;
		goto uscita;		
	}
	//pos2 = ftell(pParams->fp);
	//wprintf(L"\n\npos1 = %lu <> pos2 = %lu <> Length = %lu <> pos2 - pos1 = %lu\n\n", pos1, pos2, pParams->CurrentContent.LengthFromPdf, pos2 - pos1);
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);	
	if ( pParams->blockLen == 0 )
	{
		pParams->myToken.Type = T_EOF;
		retValue = 0;
		goto uscita;		
	}
	pParams->blockCurPos = 0;
			
	GetNextToken(pParams);
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
			
	if ( !match(pParams, T_KW_ENDSTREAM, "xrefstream_objbody") )
	{
		retValue = 0;
		goto uscita;
	}
		
	if ( pParams->nCountDecodeParams > 0 )
	{
		if ( pParams->nCountDecodeParams != pParams->nCountFilters )
		{
			snprintf(pParams->szError, 8192, "ERRORE parsing xrefstream_objbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE parsing xrefstream_objbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
			//fwprintf(pParams->fpErrors, L"ERRORE parsing xrefstream_objbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
			retValue = 0;
			goto uscita;
		}
	}	
		
uscita:

	return retValue;
}

// xrefstream_streamdictitems : {T_NAME xrefstream_keyvalue};
int xrefstream_streamdictitems(Params *pParams)
{		
	while ( pParams->myToken.Type ==  T_NAME )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
		
		if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
			pParams->bStreamLengthIsPresent = 1;		
						
		if ( strncmp(pParams->szCurrKeyName, "F", 1024) == 0 )
			pParams->CurrentContent.bExternalFile = 1;	
			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
			
		GetNextToken(pParams);
		
		if ( !xrefstream_keyvalue(pParams) )
			return 0;		
	}
	
	return 1;
}

/*
xrefstream_keyvalue : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
                | T_NAME
                | T_STRING_LITERAL
                | T_STRING_HEXADECIMAL
                | TSTRING
                | xrefstream_keyarray
                | xrefstream_keydict
                ;
*/
int xrefstream_keyvalue(Params *pParams)
{
	int n1 = -1;
	int n2 = -1;
	//int len;
	
	//pParams->myPdfTrailer.Prev = 0;
	//pParams->myPdfTrailer.Root = 0;
	//pParams->myPdfTrailer.Size = 0;
	
	switch ( pParams->myToken.Type )
	{
		case T_INT_LITERAL:
			n1 = pParams->myToken.Value.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.Value.vInt;
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				GetNextToken(pParams);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif	
				
				if ( !match(pParams, T_KW_R, "xrefstream_keyvalue") )
					return 0;
			}
			if ( n2 < 0 )
			{
				if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
				{
					pParams->CurrentContent.LengthFromPdf = n1;
				}
				else if ( strncmp(pParams->szCurrKeyName, "Prev", 1024) == 0 )
				{
					pParams->myPdfTrailer.Prev = n1;
				}
				else if ( strncmp(pParams->szCurrKeyName, "Size", 1024) == 0 )
				{
					if ( pParams->bIsLastTrailer )
					{	
						pParams->myPdfTrailer.Size = n1;						
					}
				}
			}
			else
			{
				if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
				{
					pParams->CurrentContent.LengthFromPdf = 0;
					
					if ( !ParseLengthObject(pParams, n1) )
					{
						snprintf(pParams->szError, 8192, "ERRORE xrefstream_keyvalue: ParseLengthObject: streamobj number %d\n", n1);
						myShowErrorMessage(pParams, pParams->szError, 1);
						//wprintf(L"ERRORE xrefstream_keyvalue streamobj number %d\n", n1);
						//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_keyvalue: ParseLengthObject: streamobj number %d\n", n1);
						return 0;
					}
					
					pParams->CurrentContent.LengthFromPdf = pParams->nCurrentStreamLenghtFromObjNum;
				}				
				else if ( strncmp(pParams->szCurrKeyName, "Root", 1024) == 0 ) 
				{
					pParams->myPdfTrailer.Root.Number = n1;
					pParams->myPdfTrailer.Root.Generation = n2;
				}
			}
			break;
		case T_NAME:
			if ( (strncmp(pParams->szCurrKeyName, "Filter", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
			{
				if ( strncmp(pParams->myToken.Value.vString, "FlateDecode", 1024) != 0 )
				{
					snprintf(pParams->szError, 8192, "ERRORE xrefstream_keyvalue(bStreamStateToUnicode): Flitri diversi da FlateDecode non supportati in questa versione del programma.\n");
					myShowErrorMessage(pParams, pParams->szError, 1);
					//wprintf(L"ERRORE xrefstream_keyvalue(bStreamStateToUnicode): Flitri diversi da FlateDecode non supportati in questa versione del programma.\n");
					//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_keyvalue(bStreamStateToUnicode): Flitri diversi da FlateDecode non supportati in questa versione del programma.\n");
					return 0;					
				}
				
				mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.Value.vString);
				if ( (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
					pParams->CurrentContent.bExternalFile = 1;
				pParams->nCountFilters++;
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			break;
		case T_STRING_LITERAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING_HEXADECIMAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_KW_TRUE:        // File Specifications: vedi su PDF3000_2008 a pag. 99
		case T_KW_FALSE:       // File Specifications: vedi su PDF3000_2008 a pag. 99
		case T_REAL_LITERAL:   // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			// IGNORIAMO
			GetNextToken(pParams);			
			break;
		case T_QOPAREN:
			if ( !xrefstream_keyarray(pParams) )
				return 0;
			break;
		case T_DICT_BEGIN:
			if ( !xrefstream_keydict(pParams) )
				return 0;
			if ( (strncmp(pParams->szCurrKeyName, "DecodeParms", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FDecodeParms", 1024) == 0) )
				pParams->nCountDecodeParams++;				
			break;
		default:
			snprintf(pParams->szError, 8192, "ERRORE xrefstream_keyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE xrefstream_keyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_keyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
			break;
	}
	
	return 1;
}

// xrefstream_keyarray : T_QOPAREN ( {T_INT_LITERAL} | T_STRING_HEXADECIMAL T_STRING_HEXADECIMAL |  {TNAME} | {T_KW_NULL | xrefstream_keydict}) T_QCPAREN;
int xrefstream_keyarray(Params *pParams)
{
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_QOPAREN, "xrefstream_keyarray") )
		return 0;
			
	switch ( pParams->myToken.Type )
	{
		case T_INT_LITERAL:
			while ( T_INT_LITERAL == pParams->myToken.Type )
			{				
				if ( strncmp(pParams->szCurrKeyName, "Index", 1024) == 0 )
				{
					myintqueuelist_Enqueue(	&(pParams->queueTrailerIndex), pParams->myToken.Value.vInt);
				}
				else if ( strncmp(pParams->szCurrKeyName, "W", 1024) == 0 )
				{
					myintqueuelist_Enqueue(	&(pParams->queueTrailerW), pParams->myToken.Value.vInt);
				}
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
				wprintf(L"xrefstream_keyarray -> ");
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				GetNextToken(pParams);
			}
			break;
		case T_STRING_HEXADECIMAL:
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				if ( !match(pParams, T_STRING_HEXADECIMAL, "xrefstream_keyarray") )
					return 0;
					
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				if ( !match(pParams, T_STRING_HEXADECIMAL, "xrefstream_keyarray") )
					return 0;
			break;
		case T_NAME:
			while ( pParams->myToken.Type == T_NAME )
			{				
				if ( (strncmp(pParams->szCurrKeyName, "Filter", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
				{
					mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.Value.vString);
					if ( (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
						pParams->CurrentContent.bExternalFile = 1;
					pParams->nCountFilters++;
				}
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				GetNextToken(pParams);
			}
			break;
		case T_KW_NULL:
		case T_DICT_BEGIN:		
			while ( pParams->myToken.Type == T_KW_NULL || pParams->myToken.Type == T_DICT_BEGIN )
			{
				if ( pParams->myToken.Type == T_DICT_BEGIN )
				{
					if ( !xrefstream_keydict(pParams) )
						return 0;
					pParams->myDataDecodeParams.numFilter++;
					
					pParams->nCountDecodeParams++;
				}
				
				if (pParams->myToken.Type == T_KW_NULL) 
				{
					pParams->myDataDecodeParams.tok.Type = T_KW_NULL;
					
					if ( NULL != pParams->myDataDecodeParams.pszKey )
					{
						free(pParams->myDataDecodeParams.pszKey);
						pParams->myDataDecodeParams.pszKey = NULL;
					}
					mydictionaryqueuelist_Enqueue(&(pParams->CurrentContent.decodeParms), &(pParams->myDataDecodeParams));
										
					pParams->myDataDecodeParams.numFilter++;
					
					pParams->nCountDecodeParams++;
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
					PrintToken(&(pParams->myToken), ' ', ' ', 1);
					#endif
					
					GetNextToken(pParams);
				}
			}
			break;
		default:
			snprintf(pParams->szError, 8192, "ERRORE xrefstream_keyarray: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_KW_NULL, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE xrefstream_keyarray: Atteso uno di questi token: T_NAME, T_KW_NULL, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_keyarray: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_KW_NULL, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
			break;
	}
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_QCPAREN, "xrefstream_keyarray") )
	{
		return 0;
	}
	
	return 1;
}

// xrefstream_keydict  : T_DICT_BEGIN T_NAME xrefstream_keyvalueinternal { T_NAME xrefstream_keyvalueinternal } T_DICT_END;
int xrefstream_keydict(Params *pParams)
{
	int len = 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_BEGIN, "xrefstream_keydict") )
		return 0;
	
	if ( T_NAME != pParams->myToken.Type )
		return 0;		
		
	if ( NULL != pParams->myDataDecodeParams.pszKey )
	{
		free(pParams->myDataDecodeParams.pszKey);
		pParams->myDataDecodeParams.pszKey = NULL;
	}
	
	len = strnlen(pParams->myToken.Value.vString, 1024);	
	pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));	
	if ( NULL == pParams->myDataDecodeParams.pszKey )
	{
		snprintf(pParams->szError, 8192, "ERRORE xrefstream_keydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE xrefstream_keydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_keydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		return 0;
	}
	strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.Value.vString, len + sizeof(char));
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	

	GetNextToken(pParams);
	
	if ( !xrefstream_keyvalueinternal(pParams) )
		return 0;
		
	while ( pParams->myToken.Type == T_NAME )
	{		
		if ( NULL != pParams->myDataDecodeParams.pszKey )
		{
			free(pParams->myDataDecodeParams.pszKey);
			pParams->myDataDecodeParams.pszKey = NULL;
		}
		len = strnlen(pParams->myToken.Value.vString, 1024);	
		pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));
		if ( NULL == pParams->myDataDecodeParams.pszKey )
		{
			snprintf(pParams->szError, 8192, "ERRORE xrefstream_keydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE xrefstream_keydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_keydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			return 0;
		}
		strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.Value.vString, len + sizeof(char));
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);			
				
		if ( !xrefstream_keyvalueinternal(pParams) )
		{
			return 0;
		}		
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_DICT_END, "xrefstream_keydict") )
	{
		return 0;
	}	
	
	return 1;	
}

/*
xrefstream_keyvalueinternal : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
                        | T_NAME
                        | T_STRING_LITERAL
                        | T_STRING_HEXADECIMAL
                        | TSTRING
                        | T_KW_TRUE
                        | T_KW_FALSE
                        | T_REAL_LITERAL
                        | xrefstream_keyarrayinternal
                        | xrefstream_keydict
                        ;
*/
int xrefstream_keyvalueinternal(Params *pParams)
{
	int n1 = -1;
	int n2 = -1;
	//int len;
	
	switch ( pParams->myToken.Type )
	{
		case T_INT_LITERAL:
			n1 = pParams->myToken.Value.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			
			GetNextToken(pParams);		
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.Value.vInt;
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif				
				
				GetNextToken(pParams);
				
				if ( !match(pParams, T_KW_R, "xrefstream_keyvalueinternal") )
					return 0;
			}
			if ( n2 < 0 )
			{
				if ( (strncmp(pParams->szCurrKeyName, "DecodeParms", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FDecodeParms", 1024) == 0) )
				{
					pParams->myDataDecodeParams.tok.Type = T_INT_LITERAL;
					pParams->myDataDecodeParams.tok.Value.vInt = n1;
					mydictionaryqueuelist_Enqueue(&(pParams->CurrentContent.decodeParms), &(pParams->myDataDecodeParams));								
				}
			}
			break;
		case T_NAME:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
				
			GetNextToken(pParams);
			break;
		case T_STRING_LITERAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING_HEXADECIMAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_KW_TRUE:        // File Specifications: vedi su PDF3000_2008 a pag. 99
		case T_KW_FALSE:       // File Specifications: vedi su PDF3000_2008 a pag. 99
		case T_REAL_LITERAL:   // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			// IGNORIAMO
			GetNextToken(pParams);			
			break;
		case T_QOPAREN:
			if ( !xrefstream_keyarrayinternal(pParams) )
				return 0;
			break;
		case T_DICT_BEGIN:
			if ( !xrefstream_keydict(pParams) )
				return 0;
			break;
		default:
			snprintf(pParams->szError, 8192, "ERRORE xrefstream_keyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE xrefstream_keyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_keyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
			break;
	}
	
	return 1;
}

// xrefstream_keyarrayinternal : T_QOPAREN T_STRING_HEXADECIMAL T_STRING_HEXADECIMAL T_QCPAREN
int xrefstream_keyarrayinternal(Params *pParams)
{	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_QOPAREN, "xrefstream_keyarrayinternal") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_STRING_HEXADECIMAL, "xrefstream_keyarrayinternal") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_STRING_HEXADECIMAL, "xrefstream_keyarrayinternal") )
		return 0;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_QCPAREN, "xrefstream_keyarrayinternal") )
		return 0;
		
	return 1;
}

// ************************************************************************************************************************

int getObjStmDataFromDecodedStream(Params *pParams)
{
	int retValue = 1;
	
	uint32_t objNum;
	uint32_t objOffset;
	uint32_t objOffsetPrev;
	int k;
	char lexeme[1024];
	int lenLexeme;
	uint32_t idx;
	
	unsigned char cPrev;
	
	PdfIndirectObject *pArray = NULL;
		
	lenLexeme = 0;
	objNum = 0;
	objOffset = 0;
	objOffsetPrev = 0;
	idx = 0;
	
	pArray = (PdfIndirectObject*)malloc(sizeof(PdfIndirectObject) * pParams->currentObjStm.N);
	if ( NULL == pArray )
	{
		snprintf(pParams->szError, 8192, "\n\nERRORE getObjStmDataFromDecodedStream(num = %u, gen = %u): malloc failed\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"\n\nERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): malloc failed\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		//fwprintf(pParams->fpErrors, L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): malloc failed.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		retValue = 0;
		goto uscita;
	}
	
	cPrev = pParams->currentObjStm.pszDecodedStream[0];
	if ( cPrev < '0' || cPrev > '9' )
	{
		snprintf(pParams->szError, 8192, "ERRORE getObjStmDataFromDecodedStream(num = %u, gen = %u): atteso T_INT_LITERAL; trovato carattere(hex) %X.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, cPrev);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): atteso T_INT_LITERAL; trovato carattere(hex) %X.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, cPrev);
		//fwprintf(pParams->fpErrors, L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): atteso T_INT_LITERAL; trovato carattere(hex) %X.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, cPrev);
		PrintThisObject(pParams, pParams->nCurrentParsingObj, 1, 0, pParams->fpErrors);
		retValue = 0;
		goto uscita;
	}
	lexeme[lenLexeme++] = cPrev;
	
	for ( k = 1; k < pParams->currentObjStm.First; k++ )
	{
		if ( pParams->currentObjStm.pszDecodedStream[k] >= '0' && pParams->currentObjStm.pszDecodedStream[k] <= '9' )
		{
			lexeme[lenLexeme++] = pParams->currentObjStm.pszDecodedStream[k];
			cPrev = pParams->currentObjStm.pszDecodedStream[k];
		}
		else if ( pParams->currentObjStm.pszDecodedStream[k] == ' ' )
		{
			if ( ' ' == cPrev )
			{
				snprintf(pParams->szError, 8192, "ERRORE getObjStmDataFromDecodedStream(num = %u, gen = %u): atteso T_INT_LITERAL; trovato SPAZIO.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): atteso T_INT_LITERAL; trovato SPAZIO.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
				//fwprintf(pParams->fpErrors, L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): atteso T_INT_LITERAL; trovato SPAZIO.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
				PrintThisObject(pParams, pParams->nCurrentParsingObj, 1, 0, pParams->fpErrors);
				retValue = 0;
				goto uscita;	
			}
			
			cPrev = pParams->currentObjStm.pszDecodedStream[k];
			
			lexeme[lenLexeme] = '\0';
			if ( 0 == objNum )
			{
				objNum = atoi(lexeme);
				
				if ( objNum < 1 || objNum >= pParams->nObjsTableSizeFromPrescanFile )
				{
					snprintf(pParams->szError, 8192, "Errore getObjStmDataFromDecodedStream: objNum non valido -> %d; pParams->nObjsTableSizeFromPrescanFile = %u\n", objNum, pParams->nObjsTableSizeFromPrescanFile);
					myShowErrorMessage(pParams, pParams->szError, 1);
					//wprintf(L"Errore getObjStmDataFromDecodedStream: objNum non valido -> %d; pParams->nObjsTableSizeFromPrescanFile = %lu\n", objNum, pParams->nObjsTableSizeFromPrescanFile);
					//fwprintf(pParams->fpErrors, L"Errore getObjStmDataFromDecodedStream: objNum non valido -> %d; pParams->nObjsTableSizeFromPrescanFile = %lu\n", objNum, pParams->nObjsTableSizeFromPrescanFile);
					retValue = 0;
					goto uscita;
				}
			}
			else
			{
				objOffset = atoi(lexeme);
				
				pParams->myObjsTable[objNum]->Obj.Type = OBJ_TYPE_STREAM;
				pParams->myObjsTable[objNum]->Obj.Number = objNum;
				pParams->myObjsTable[objNum]->Obj.Generation = idx;   
				pParams->myObjsTable[objNum]->Obj.Offset = pParams->nCurrentParsingObj; // The object number of the object stream in which this object is stored. (The generation number of the object stream shall be implicitly 0.)
				pParams->myObjsTable[objNum]->Obj.StreamOffset = objOffset;
				pParams->myObjsTable[objNum]->Obj.StreamLength = objOffset - objOffsetPrev;
				pParams->myObjsTable[objNum]->Obj.numObjParent = -1;
				pParams->myObjsTable[objNum]->Obj.genObjParent = 0;
								
				if ( (int)idx < pParams->currentObjStm.N )
				{
					pArray[idx].Type = OBJ_TYPE_STREAM;
					pArray[idx].Number = objNum;
					pArray[idx].Generation = idx;   // The object number of the object stream in which this object is stored. (The generation number of the object stream shall be implicitly 0.)
					pArray[idx].Offset = pParams->nCurrentParsingObj;
					pArray[idx].StreamOffset = objOffset;
					//pArray[idx].StreamLength = objOffset - objOffsetPrev;
					pArray[idx].StreamLength = 0;
					pArray[idx].numObjParent = -1;
					pArray[idx].genObjParent = 0;
				}
				else
				{
					snprintf(pParams->szError, 8192, "ERRORE getObjStmDataFromDecodedStream(num = %u, gen = %u): N = %d; idx = %u\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->currentObjStm.N, idx);
					myShowErrorMessage(pParams, pParams->szError, 1);
					//wprintf(L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): bjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->currentObjStm.pszDecodedStream[k]);
					//fwprintf(pParams->fpErrors, L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): N = %d; idx = %lu\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->currentObjStm.N, idx);
					retValue = 0;
					goto uscita;
				}
				
				idx++;
				
				objOffsetPrev = objOffset;
				
				objNum = 0;
				objOffset = 0;
			}
			lenLexeme = 0;
		}
		else
		{
			snprintf(pParams->szError, 8192, "ERRORE getObjStmDataFromDecodedStream(num = %u, gen = %u): atteso T_INT_LITERAL o SPAZIO; trovato carattere(hex) %X.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->currentObjStm.pszDecodedStream[k]);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): atteso T_INT_LITERAL o SPAZIO; trovato carattere(hex) %X.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->currentObjStm.pszDecodedStream[k]);
			//fwprintf(pParams->fpErrors, L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): atteso T_INT_LITERAL o SPAZIO; trovato carattere(hex) %X.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->currentObjStm.pszDecodedStream[k]);
			PrintThisObject(pParams, pParams->nCurrentParsingObj, 1, 0, pParams->fpErrors);
			retValue = 0;
			goto uscita;
		}
	}
	
	if ( objNum != 0 || objOffset != 0 )
	{
		snprintf(pParams->szError, 8192, "ERRORE getObjStmDataFromDecodedStream(num = %u, gen = %u): odd numer of integer in ObjStm Decoded Stream.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): odd numer of integer in ObjStm Decoded Stream.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		//fwprintf(pParams->fpErrors, L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): odd numer of integer in ObjStm Decoded Stream.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		PrintThisObject(pParams, pParams->nCurrentParsingObj, 1, 0, pParams->fpErrors);
		retValue = 0;
		goto uscita;
	}
	
	if ( (int)idx != pParams->currentObjStm.N )
	{
		snprintf(pParams->szError, 8192, "ERRORE getObjStmDataFromDecodedStream(num = %u, gen = %u): NUMERO OGGETTI NON CORRISPONDENTE -> N = %d; idx = %u\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->currentObjStm.N, idx);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): NUMERO OGGETTI NON CORRISPONDENTE -> N = %d; idx = %lu\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->currentObjStm.N, idx);
		//fwprintf(pParams->fpErrors, L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): NUMERO OGGETTI NON CORRISPONDENTE -> N = %d; idx = %lu\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->currentObjStm.N, idx);
		retValue = 0;
		goto uscita;
	}

	if ( 1 == pParams->currentObjStm.N )
	{
		pParams->myObjsTable[pArray[0].Number]->Obj.StreamLength = pParams->currentObjStm.nDecodedStreamSize - pParams->currentObjStm.First;
	}		
	else if ( pParams->currentObjStm.N > 1 )
	{
		int i;
		for ( i = 0; i < pParams->currentObjStm.N - 1; i++ )
		{
			pParams->myObjsTable[pArray[i].Number]->Obj.StreamLength = pParams->myObjsTable[pArray[i + 1].Number]->Obj.StreamOffset - pParams->myObjsTable[pArray[i].Number]->Obj.StreamOffset;
		}
		
		pParams->myObjsTable[pArray[i].Number]->Obj.StreamLength = pParams->currentObjStm.nDecodedStreamSize - pParams->myObjsTable[pArray[i].Number]->Obj.StreamOffset - k;
	}
			
	
	
	//wprintf(L"\n\n%d OBJECTS CONTAINED IN OBJSTM(%lu):\n", pParams->currentObjStm.N, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number);
	for ( uint32_t i = 0; i < idx; i++ )
	{
		//wprintf(L"\n\tOBJ(%lu %lu): pArray[%lu]->StreamOffset = %lu <> pArray[%lu]->StreamLength = %lu\n", pParams->myObjsTable[pArray[i].Number]->Obj.Number, pParams->myObjsTable[pArray[i].Number]->Obj.Generation, i, pParams->myObjsTable[pArray[i].Number]->Obj.StreamOffset, i, pParams->myObjsTable[pArray[i].Number]->Obj.StreamLength);
		
		if ( NULL == pParams->myObjsTable[pArray[i].Number]->Obj.pszDecodedStream )
		{
			//if ( NULL != pParams->myObjsTable[pArray[i].Number]->Obj.pszDecodedStream )
			//{
			//	free(pParams->myObjsTable[pArray[i].Number]->Obj.pszDecodedStream);
			//	pParams->myObjsTable[pArray[i].Number]->Obj.pszDecodedStream = NULL;
			//}
			pParams->myObjsTable[pArray[i].Number]->Obj.pszDecodedStream = (unsigned char*)malloc(sizeof(unsigned char) * pParams->myObjsTable[pArray[i].Number]->Obj.StreamLength);
			if ( NULL == pParams->myObjsTable[pArray[i].Number]->Obj.pszDecodedStream )
			{
				snprintf(pParams->szError, 8192, "ERRORE getObjStmDataFromDecodedStream(num = %u, gen = %u): malloc 2 failed.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
				myShowErrorMessage(pParams, pParams->szError, 1);
				//wprintf(L"\n\nERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): malloc 2 failed\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
				//fwprintf(pParams->fpErrors, L"ERRORE getObjStmDataFromDecodedStream(num = %lu, gen = %lu): malloc 2 failed.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
				retValue = 0;
				goto uscita;
			}		
			memcpy(pParams->myObjsTable[pArray[i].Number]->Obj.pszDecodedStream, pParams->currentObjStm.pszDecodedStream + pParams->myObjsTable[pArray[i].Number]->Obj.StreamOffset + k, pParams->myObjsTable[pArray[i].Number]->Obj.StreamLength);
			
			//wprintf(L"\nStmObj -> ALLOC pParams->myObjsTable[%lu]->Obj.pszDecodedStream\n", pArray[i].Number);
		}
		
		//PrintThisObject(pParams, pParams->myObjsTable[pArray[i].Number]->Obj.Number, 0, 0, NULL);
	}
	//wprintf(L"\nFine pArray\n\n");



uscita:

	free(pParams->currentObjStm.pszDecodedStream);
	pParams->currentObjStm.pszDecodedStream = NULL;
	pParams->currentObjStm.nDecodedStreamSize = 0;
	
	if ( NULL != pArray )
	{
		free(pArray);
		pArray = NULL;
	}

	return retValue;
}

//stmobj      : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ stmobjbody T_KW_ENDOBJ;
int stmobj(Params *pParams)
{	
	pParams->nCountDecodeParams = pParams->nCountFilters = 0;
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE stmobj(num = %u, gen = %u): atteso T_INT_LITERAL; trovato token n° %d.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE stmobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE stmobj(num = %lu, gen = %lu): atteso T_INT_LITERAL; trovato token n° %d.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.Value.vInt;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	GetNextToken(pParams);
	
	//if ( !match(pParams, T_INT_LITERAL, "stmobj") )
	//	return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_INT_LITERAL, "stmobj") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_KW_OBJ, "stmobj") )
		return 0;
		
	if ( !stmobjbody(pParams) )
		return 0;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_ENDOBJ, "stmobj") )
		return 0;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	wprintf(L"\n\n");
	#endif		
			
	if ( !(pParams->bStreamLengthIsPresent) )
	{
		snprintf(pParams->szError, 8192, "ERROR stmobj(num = %u, gne = %u): 'Length' key not found.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE stmobj: la chiave 'Length' e' assente.\n");
		//fwprintf(pParams->fpErrors, L"ERROR stmobj(num = %lu, gne = %lu): 'Length' key not found.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
		return 0;
	}
	
	if ( 0 == pParams->currentObjStm.N )
	{
		snprintf(pParams->szError, 8192, "ERROR parsing stmobj(num = %u, gen = %u): 'N' key not found.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//fwprintf(pParams->fpErrors, L"ERROR parsing stmobj(num = %d, gen = %d): 'N' key not found.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
		return 0;
	}
	
	if ( 0 == pParams->currentObjStm.First )
	{
		snprintf(pParams->szError, 8192, "ERROR parsing stmobj(num = %u, gen = %u): 'First' key not found.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//fwprintf(pParams->fpErrors, L"ERROR parsing stmobj(num = %d, gen = %d): 'First' key not found.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
		return 0;
	}
	
	if ( NULL != pParams->myDataDecodeParams.pszKey )
	{
		free(pParams->myDataDecodeParams.pszKey);
		pParams->myDataDecodeParams.pszKey = NULL;
	}
	
	return 1;
}

// stmobjbody  : T_DICT_BEGIN stmobjstreamdictitems T_DICT_END T_KW_STREAM T_STRING T_KW_ENDSTREAM;
int stmobjbody(Params *pParams)
{
	int retValue = 1;
	
	pParams->CurrentContent.bExternalFile = 0;
	pParams->CurrentContent.Offset = 0;
	//pParams->CurrentContent.Length = 0;
	pParams->CurrentContent.LengthFromPdf = 0;
	pParams->bStreamLengthIsPresent = 0;
	
	pParams->myDataDecodeParams.numFilter = 0;
	pParams->myDataDecodeParams.pszKey = NULL;
	
	pParams->currentObjStm.N = 0;
	pParams->currentObjStm.First = 0;
	pParams->currentObjStm.Extend = 0;
	if ( NULL != pParams->currentObjStm.pszDecodedStream )
	{
		free(pParams->currentObjStm.pszDecodedStream);
		pParams->currentObjStm.pszDecodedStream = NULL;
	}
	pParams->currentObjStm.nDecodedStreamSize = 0;
	
	mystringqueuelist_Init(&(pParams->CurrentContent.queueFilters));
	
	mydictionaryqueuelist_Init(&(pParams->CurrentContent.decodeParms), 1, 1);
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_BEGIN, "stmobjbody") )
	{
		retValue = 0;
		goto uscita;
	}
				
	if ( !stmobjstreamdictitems(pParams) )
	{
		retValue = 0;
		goto uscita;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_DICT_END, "stmobjbody") )
	{
		retValue = 0;
		goto uscita;
	}
					
	if ( pParams->myToken.Type != T_KW_STREAM )
	{
		snprintf(pParams->szError, 8192, "ERRORE stmobjbody(OBJ num = %u, gen = %u): Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE stmobjbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		//fwprintf(pParams->fpErrors, L"ERRORE stmobjbody(OBJ num = %lu, gen = %lu): Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		retValue = 0;
		goto uscita;
	}
	
	pParams->CurrentContent.Offset = pParams->nNumBytesReadFromCurrentStream;	
	pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamOffset = pParams->CurrentContent.Offset;
	//pParams->CurrentContent.Offset = pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamOffset;
	/*
	if ( pParams->CurrentContent.LengthFromPdf != pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength )
	{
		fwprintf(pParams->fpErrors, L"WARNING stmobjbody(OBJ num = %lu, gen = %lu): pParams->CurrentContent.LengthFromPdf = %lu differs from pParams->myObjsTable[%d]->Obj.StreamLength = %lu\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->CurrentContent.LengthFromPdf, pParams->nCurrentParsingObj, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength);
		//PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
		//pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength = pParams->CurrentContent.LengthFromPdf;
	}
	*/
	//pParams->CurrentContent.LengthFromPdf = pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.StreamLength;
	
	
	
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( fseek(pParams->fp, pParams->CurrentContent.Offset + pParams->CurrentContent.LengthFromPdf, SEEK_SET) != 0 )
	{
		snprintf(pParams->szError, 8192, "ERRORE stmobjbody(OBJ num = %u, gen = %u): fseek\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		myShowErrorMessage(pParams, pParams->szError, 1);
		wprintf(L"ERRORE stmobjbody(OBJ num = %lu, gen = %lu): fseek\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		//fwprintf(pParams->fpErrors, L"ERRORE stmobjbody(OBJ num = %lu, gen = %lu): fseek\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		retValue = 0;
		goto uscita;		
	}
		
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);	
	if ( pParams->blockLen == 0 )
	{
		pParams->myToken.Type = T_EOF;
		retValue = 0;
		goto uscita;		
	}
	pParams->blockCurPos = 0;
	
	GetNextToken(pParams);
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
			
	if ( !match(pParams, T_KW_ENDSTREAM, "stmobjbody") )
	{
		retValue = 0;
		goto uscita;
	}
		
	if ( pParams->nCountDecodeParams > 0 )
	{
		if ( pParams->nCountDecodeParams != pParams->nCountFilters )
		{
			snprintf(pParams->szError, 8192, "ERRORE parsing stmobjbody(OBJ num = %u, gen = %u): il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->nCountDecodeParams, pParams->nCountFilters);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE parsing stmobjbody(OBJ num = %lu, gen = %lu): il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->nCountDecodeParams, pParams->nCountFilters);
			//fwprintf(pParams->fpErrors, L"ERRORE parsing stmobjbody(OBJ num = %d, gen = %d): il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->nCountDecodeParams, pParams->nCountFilters);
			retValue = 0;
			goto uscita;
		}
	}
	
	pParams->currentObjStm.pszDecodedStream = getDecodedStream(pParams, &(pParams->currentObjStm.nDecodedStreamSize), &(pParams->CurrentContent));
	if ( NULL == pParams->currentObjStm.pszDecodedStream )
	{
		snprintf(pParams->szError, 8192, "ERRORE parsing stmobjbody(OBJ num = %u, gen = %u): getDecodedStream failed.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE parsing stmobjbody(OBJ num = %d, gen = %d): getDecodedStream failed.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		//fwprintf(pParams->fpErrors, L"ERRORE parsing stmobjbody(OBJ num = %d, gen = %d): getDecodedStream failed.\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation);
		retValue = 0;
		goto uscita;
	}
	
	retValue = getObjStmDataFromDecodedStream(pParams);
	
uscita:
	
	return retValue;
}

// stmobjstreamdictitems : {T_NAME stmobjkeyvalue};
int stmobjstreamdictitems(Params *pParams)
{		
	while ( pParams->myToken.Type ==  T_NAME )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
		
		if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
			pParams->bStreamLengthIsPresent = 1;		
						
		if ( strncmp(pParams->szCurrKeyName, "F", 1024) == 0 )
			pParams->CurrentContent.bExternalFile = 1;	
			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
			
		GetNextToken(pParams);
		
		if ( !stmobjkeyvalue(pParams) )
			return 0;		
	}
	
	return 1;
}

/*
stmobjkeyvalue : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
                | T_NAME
                | T_STRING_LITERAL
                | T_STRING_HEXADECIMAL
                | TSTRING
                | stmkeyarray
                | stmkeydict
                ;
*/
int stmobjkeyvalue(Params *pParams)
{
	int n1 = -1;
	int n2 = -1;
	//int len;
	
	switch ( pParams->myToken.Type )
	{
		case T_INT_LITERAL:
			n1 = pParams->myToken.Value.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.Value.vInt;
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				GetNextToken(pParams);
				
				if ( !match(pParams, T_KW_R, "stmobjkeyvalue") )
					return 0;
			}
			if ( n2 < 0 )
			{
				if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
				{
					pParams->CurrentContent.LengthFromPdf = n1;
				}
				else if ( strncmp(pParams->szCurrKeyName, "N", 1024) == 0 )
				{
					pParams->currentObjStm.N = n1;
				}
				else if ( strncmp(pParams->szCurrKeyName, "First", 1024) == 0 )
				{
					pParams->currentObjStm.First = n1;
				}
			}
			else
			{
				if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
				{
					pParams->CurrentContent.LengthFromPdf = 0;
					
					if ( !ParseLengthObject(pParams, n1) )
					{
						snprintf(pParams->szError, 8192, "ERRORE stmobjkeyvalue(OBJ num = %u, gen = %u) streamobj number %u\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, n1);
						myShowErrorMessage(pParams, pParams->szError, 1);
						//wprintf(L"ERRORE stmobjkeyvalue(OBJ num = %lu, gen = %lu) streamobj number %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, n1);
						//fwprintf(pParams->fpErrors, L"ERRORE stmobjkeyvalue(OBJ num = %lu, gen = %lu) streamobj number %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, n1);
						return 0;
					}
					
					pParams->CurrentContent.LengthFromPdf = pParams->nCurrentStreamLenghtFromObjNum;
				}
				else if ( strncmp(pParams->szCurrKeyName, "N", 1024) == 0 )
				{					
					if ( !ParseLengthObject(pParams, n1) )
					{
						snprintf(pParams->szError, 8192, "ERRORE stmobjkeyvalue(OBJ num = %u, gen = %u) streamobj number %u\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, n1);
						myShowErrorMessage(pParams, pParams->szError, 1);
						//wprintf(L"ERRORE stmobjkeyvalue(OBJ num = %lu, gen = %lu) streamobj number %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, n1);
						//fwprintf(pParams->fpErrors, L"ERRORE stmobjkeyvalue(OBJ num = %lu, gen = %lu) streamobj number %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, n1);
						return 0;
					}
					
					pParams->currentObjStm.N = pParams->nCurrentStreamLenghtFromObjNum;
				}
				else if ( strncmp(pParams->szCurrKeyName, "First", 1024) == 0 )
				{					
					if ( !ParseLengthObject(pParams, n1) )
					{
						snprintf(pParams->szError, 8192, "ERRORE stmobjkeyvalue(OBJ num = %u, gen = %u) streamobj number %u\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, n1);
						myShowErrorMessage(pParams, pParams->szError, 1);
						//wprintf(L"ERRORE stmobjkeyvalue(OBJ num = %lu, gen = %lu) streamobj number %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, n1);
						//fwprintf(pParams->fpErrors, L"ERRORE stmobjkeyvalue(OBJ num = %lu, gen = %lu) streamobj number %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, n1);
						return 0;
					}
					
					pParams->currentObjStm.First = pParams->nCurrentStreamLenghtFromObjNum;
				}
				else if ( strncmp(pParams->szCurrKeyName, "Extends", 1024) == 0 )
				{
					pParams->currentObjStm.Extend = n1;
				}		
			}
			break;
		case T_NAME:
			if ( (strncmp(pParams->szCurrKeyName, "Filter", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
			{
				if ( (strncmp(pParams->myToken.Value.vString, "FlateDecode", 1024) == 0) )
				{
					mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.Value.vString);
					if ( (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
						pParams->CurrentContent.bExternalFile = 1;
					pParams->nCountFilters++;
				}
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			break;
		case T_STRING_LITERAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING_HEXADECIMAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING: // File Specifications: vedi su PDF3000_2008 a pag. 99
			// IGNORIAMO
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			GetNextToken(pParams);
			break;
		case T_QOPAREN:
			if ( !stmobjkeyarray(pParams) )
				return 0;
			break;
		case T_DICT_BEGIN:
			if ( !stmobjkeyarray(pParams) )
				return 0;
			if ( (strncmp(pParams->szCurrKeyName, "DecodeParms", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FDecodeParms", 1024) == 0) )
				pParams->nCountDecodeParams++;				
			break;
		default:
			snprintf(pParams->szError, 8192, "ERRORE stmobjkeyvalue(OBJ num = %u, gen = %u): Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE stmobjkeyvalue(OBJ num = %lu, gen = %lu): Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE stmobjkeyvalue(OBJ num = %d, gen = %d): Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
			break;
	}
	
	return 1;
}

// stmobjkeyarray : T_QOPAREN ( {T_INT_LITERAL} | T_STRING_HEXADECIMAL T_STRING_HEXADECIMAL |  {TNAME} | {T_KW_NULL | xrefstream_keydict}) T_QCPAREN;
int stmobjkeyarray(Params *pParams)
{
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_QOPAREN, "stmobjkeyarray") )
		return 0;
			
	switch ( pParams->myToken.Type )
	{
		case T_INT_LITERAL:
			while ( T_INT_LITERAL == pParams->myToken.Type )
			{								
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
				wprintf(L"stmobjkeyarray -> ");
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				GetNextToken(pParams);
			}
			break;
		case T_STRING_HEXADECIMAL:
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				if ( !match(pParams, T_STRING_HEXADECIMAL, "stmobjkeyarray") )
					return 0;
					
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				if ( !match(pParams, T_STRING_HEXADECIMAL, "stmobjkeyarray") )
					return 0;
			break;
		case T_NAME:
			while ( pParams->myToken.Type == T_NAME )
			{				
				if ( (strncmp(pParams->szCurrKeyName, "Filter", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
				{
					mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.Value.vString);
					if ( (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
						pParams->CurrentContent.bExternalFile = 1;
					pParams->nCountFilters++;
				}
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				GetNextToken(pParams);
			}
			break;
		case T_KW_NULL:
		case T_DICT_BEGIN:		
			while ( pParams->myToken.Type == T_KW_NULL || pParams->myToken.Type == T_DICT_BEGIN )
			{
				if ( pParams->myToken.Type == T_DICT_BEGIN )
				{
					if ( !stmobjkeydict(pParams) )
						return 0;
					pParams->myDataDecodeParams.numFilter++;
					
					pParams->nCountDecodeParams++;
				}
				
				if (pParams->myToken.Type == T_KW_NULL) 
				{
					pParams->myDataDecodeParams.tok.Type = T_KW_NULL;
					
					if ( NULL != pParams->myDataDecodeParams.pszKey )
					{
						free(pParams->myDataDecodeParams.pszKey);
						pParams->myDataDecodeParams.pszKey = NULL;
					}
					mydictionaryqueuelist_Enqueue(&(pParams->CurrentContent.decodeParms), &(pParams->myDataDecodeParams));
										
					pParams->myDataDecodeParams.numFilter++;
					
					pParams->nCountDecodeParams++;
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
					PrintToken(&(pParams->myToken), ' ', ' ', 1);
					#endif
					
					GetNextToken(pParams);
				}
			}
			break;
		default:
			snprintf(pParams->szError, 8192, "ERRORE stmobjkeyvalue(OBJ num = %u, gen = %u): Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE stmobjkeyarray(OBJ num = %lu, gen = %lu): Atteso uno di questi token: T_NAME, T_KW_NULL, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE stmobjkeyarray(OBJ num = %lu, gen = %lu): Atteso uno di questi token: T_NAME, T_KW_NULL, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
			break;
	}
		
	if ( !match(pParams, T_QCPAREN, "stmobjkeyarray") )
	{
		return 0;
	}
	
	return 1;
}

// stmobjkeydict  : T_DICT_BEGIN T_NAME stmobjkeyvalueinternal { T_NAME stmobjkeyvalueinternal } T_DICT_END;
int stmobjkeydict(Params *pParams)
{
	int len = 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_BEGIN, "stmobjkeydict") )
		return 0;
	
	if ( T_NAME != pParams->myToken.Type )
		return 0;		
		
	if ( NULL != pParams->myDataDecodeParams.pszKey )
	{
		free(pParams->myDataDecodeParams.pszKey);
		pParams->myDataDecodeParams.pszKey = NULL;
	}
	
	len = strnlen(pParams->myToken.Value.vString, 1024);	
	pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));	
	if ( NULL == pParams->myDataDecodeParams.pszKey )
	{
		snprintf(pParams->szError, 8192, "ERRORE stmobjkeydict(OBJ num = %u, gen = %u): impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, sizeof(char) * len + sizeof(char));
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE stmobjkeydict(OBJ num = %lu, gen = %lu): impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, sizeof(char) * len + sizeof(char));
		//fwprintf(pParams->fpErrors, L"ERRORE stmobjkeydict(OBJ num = %lu, gen = %lu): impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, sizeof(char) * len + sizeof(char));
		return 0;
	}
	strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.Value.vString, len + sizeof(char));
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	

	GetNextToken(pParams);
	
	if ( !stmobjkeyvalueinternal(pParams) )
		return 0;
		
	while ( pParams->myToken.Type == T_NAME )
	{		
		if ( NULL != pParams->myDataDecodeParams.pszKey )
		{
			free(pParams->myDataDecodeParams.pszKey);
			pParams->myDataDecodeParams.pszKey = NULL;
		}
		len = strnlen(pParams->myToken.Value.vString, 1024);	
		pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));
		if ( NULL == pParams->myDataDecodeParams.pszKey )
		{
			snprintf(pParams->szError, 8192, "ERRORE stmobjkeydict(OBJ num = %u, gen = %u): impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, sizeof(char) * len + sizeof(char));
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE stmobjkeydict(OBJ num = %lu, gen = %lu): impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, sizeof(char) * len + sizeof(char));
			//fwprintf(pParams->fpErrors, L"ERRORE stmobjkeydict(OBJ num = %lu, gen = %lu): impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, sizeof(char) * len + sizeof(char));
			return 0;
		}
		strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.Value.vString, len + sizeof(char));
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);			
				
		if ( !stmobjkeyvalueinternal(pParams) )
		{
			return 0;
		}		
	}
	
	if ( !match(pParams, T_DICT_END, "stmobjkeydict") )
	{
		return 0;
	}	
	
	return 1;	
}

/*
stmobjkeyvalueinternal : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
                        | T_NAME
                        | T_STRING_LITERAL
                        | T_STRING_HEXADECIMAL
                        | TSTRING
                        | T_KW_TRUE
                        | T_KW_FALSE
                        | T_REAL_LITERAL
                        | stmobjkeyarrayinternal
                        | stmobjkeydict
                        ;
*/
int stmobjkeyvalueinternal(Params *pParams)
{
	int n1 = -1;
	int n2 = -1;
	//int len;
	
	switch ( pParams->myToken.Type )
	{
		case T_INT_LITERAL:
			n1 = pParams->myToken.Value.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			
			GetNextToken(pParams);		
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.Value.vInt;
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif				
				
				GetNextToken(pParams);
				
				if ( !match(pParams, T_KW_R, "stmobjkeyvalueinternal") )
					return 0;
			}
			if ( n2 < 0 )
			{
				if ( (strncmp(pParams->szCurrKeyName, "DecodeParms", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FDecodeParms", 1024) == 0) )
				{
					pParams->myDataDecodeParams.tok.Type = T_INT_LITERAL;
					pParams->myDataDecodeParams.tok.Value.vInt = n1;
					mydictionaryqueuelist_Enqueue(&(pParams->CurrentContent.decodeParms), &(pParams->myDataDecodeParams));
				}
			}
			break;
		case T_NAME:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
				
			GetNextToken(pParams);
			break;
		case T_STRING_LITERAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING_HEXADECIMAL: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_STRING: // File Specifications: vedi su PDF3000_2008 a pag. 99
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_KW_TRUE:        // File Specifications: vedi su PDF3000_2008 a pag. 99
		case T_KW_FALSE:       // File Specifications: vedi su PDF3000_2008 a pag. 99
		case T_REAL_LITERAL:   // File Specifications: vedi su PDF3000_2008 a pag. 99
#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
#endif
			// IGNORIAMO
			GetNextToken(pParams);			
			break;
		case T_QOPAREN:
			if ( !stmobjkeyarrayinternal(pParams) )
				return 0;
			break;
		case T_DICT_BEGIN:
			if ( !stmobjkeydict(pParams) )
				return 0;
			break;
		default:
			snprintf(pParams->szError, 8192, "ERRORE stmobjkeyvalue(OBJ num = %u, gen = %u): Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE stmobjkeyvalue(OBJ num = %lu, gen = %lu): Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE stmobjkeyvalue(OBJ num = %lu, gen = %lu): Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
			break;
	}
	
	return 1;
}

// stmobjkeyarrayinternal : T_QOPAREN T_STRING_HEXADECIMAL T_STRING_HEXADECIMAL T_QCPAREN
int stmobjkeyarrayinternal(Params *pParams)
{	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_QOPAREN, "stmobjkeyarrayinternal") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_STRING_HEXADECIMAL, "stmobjkeyarrayinternal") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_STRING_HEXADECIMAL, "stmobjkeyarrayinternal") )
		return 0;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_QCPAREN, "stmobjkeyarrayinternal") )
		return 0;
		
	return 1;
}

// ******************************************************************************************************************************************

// cot : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ T_DICT_BEGIN cot_dictbody T_DICT_END
int cot(Params *pParams)
{
	int retValue = 1;
	
	pParams->nCotDictLevel = pParams->nCotArrayLevel = 0;
	pParams->nCotObjType = OBJ_TYPE_IN_USE;
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE cot: atteso T_INT_LITERAL; trovato token n° %d.\n", pParams->myToken.Type);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE cot: atteso T_INT_LITERAL; trovato token n° %d.\n", pParams->myToken.Type);
		//fwprintf(pParams->fpErrors, L"ERRORE cot: atteso T_INT_LITERAL; trovato token n° %d.\n", pParams->myToken.Type);
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		retValue = 0;
		goto uscita;
	}
	pParams->nCurrentParsingObj = pParams->myToken.Value.vInt;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	GetNextToken(pParams);
				
	//if ( !match(pParams, T_INT_LITERAL, "cot") )
	//	return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	if ( !match(pParams, T_INT_LITERAL, "cot") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	if ( !match(pParams, T_KW_OBJ, "cot") )
		return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	if ( !match(pParams, T_DICT_BEGIN, "cot") )
		return 1;   // OBJ_TYPE_IN_USE
	
	
	
	retValue = cot_dictbody(pParams);
	if ( !retValue )
		goto uscita;
	if ( retValue > 1 )
		goto uscita;
	
	
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	if ( !match(pParams, T_DICT_END, "cot") )
		return 0;
		
uscita:

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
	wprintf(L"\n\n");
	#endif		
			
	return retValue;
}

// cot_dictbody : {T_NAME cot_dictbodyitems}
int cot_dictbody(Params *pParams)
{
	int retValue = 1;
		
	while ( T_NAME == pParams->myToken.Type )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
						
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		GetNextToken(pParams);
		
		retValue = cot_dictbodyitems(pParams);
		if ( 2 == retValue || 0 == retValue )
			return retValue;
	}
	
	return retValue;
}

// cot_dictbodyitems : T_NAME 
//                   | T_STRING_LITERAL
//                   | T_STRING_HEXADECIMAL
//                   | T_REAL_LITERAL
//                   | T_INT_LITERAL [T_INT_LITERAL T_KW_R]
//                   | T_STRING
//                   | | T_KW_R
//                   | T_KW_NULL
//                   | T_KW_FALSE
//                   | T_KW_TRUE
//                   | T_DICT_BEGIN cot_dictbodyitems T_DICT_END
//                   | T_QOPAREN cot_dictbodyitems T_QCPAREN
//                   ;
int cot_dictbodyitems(Params *pParams)
{
	int retValue = 1;
				
	switch ( pParams->myToken.Type )
	{
		case T_NAME:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif

			if ( strncmp(pParams->szCurrKeyName, "Type", 1024) == 0 )
			{
				if ( strncmp(pParams->myToken.Value.vString, "ObjStm", 1024) == 0 )
				{
					pParams->nCotObjType = OBJ_TYPE_STREAM;
					GetNextToken(pParams);
					return 2;
				}
				else
				{
					GetNextToken(pParams);
					return 1;
				}
			}
			
			GetNextToken(pParams);
			break;
		case T_INT_LITERAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			
			if ( T_INT_LITERAL == pParams->myToken.Type )
			{
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				GetNextToken(pParams);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				if ( !match(pParams, T_KW_R, "cot_dictbodyitems") )
					return 0;
			}
			break;
        case T_STRING_LITERAL:
        case T_STRING_HEXADECIMAL:
        case T_REAL_LITERAL:
        case T_STRING:
        case T_KW_R:
        case T_KW_NULL:
        case T_KW_FALSE:
        case T_KW_TRUE:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			break;
        case T_DICT_BEGIN:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			
			pParams->nCotDictLevel++;
			while ( 1 )
			{					
				if ( T_DICT_BEGIN == pParams->myToken.Type )
					pParams->nCotDictLevel++;
				else if ( T_DICT_END == pParams->myToken.Type )
					pParams->nCotDictLevel--;
				else if ( T_EOF == pParams->myToken.Type || T_ERROR == pParams->myToken.Type )
					break;

				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				GetNextToken(pParams);	
				
				if ( 0 == pParams->nCotDictLevel )
					break;
			}			
			break;
        case T_QOPAREN:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			
			pParams->nCotArrayLevel++;
			while ( 1 )
			{					
				if ( T_QOPAREN == pParams->myToken.Type )
					pParams->nCotArrayLevel++;
				else if ( T_QCPAREN == pParams->myToken.Type )
					pParams->nCotArrayLevel--;
				else if ( T_EOF == pParams->myToken.Type || T_ERROR == pParams->myToken.Type )
					break;

				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				GetNextToken(pParams);	
				
				if ( 0 == pParams->nCotArrayLevel )
					break;
			}
			break;
		default:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_COT)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			GetNextToken(pParams);
			break;
	}
	
	return retValue;
}

// ******************************************************************************************************************************************

int pdf(Params *pParams)
{
	int retValue = 1;
	
	retValue = ReadTrailer(pParams);
	
	return retValue;
}
