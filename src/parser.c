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

int Parse(Params *pParams, FilesList* myFilesList, int bPrintObjsAndExit)
{
	int retValue = 1;
	FilesList* n;
	int x;
	int len;
			
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
	pParams->myObjsTable = NULL;
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
	
	
	//pParams->pCurrentEncodingArray = &(pParams->aUtf8CharSet[0]);
	pParams->pCurrentEncodingArray = &(pParams->aSTD_CharSet[0]);
	//pParams->pCurrentEncodingArray = &(pParams->aWIN_CharSet[0]);
	//pParams->pCurrentEncodingArray = &(pParams->aMAC_CharSet[0]);
	//pParams->pCurrentEncodingArray = &(pParams->aPDF_CharSet[0]);
	
	InitializeUnicodeArray(pParams);
	InitializeCharSetArrays(pParams);
	InitializeCharSetHashTable(pParams);	
		
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_PARSE_FN) && !defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
	UNUSED(bPrintObjsAndExit);
	#endif	
	
	tstInit(&(pParams->myTST));
		
	myobjreflist_Init(&(pParams->myXObjRefList));
	myobjreflist_Init(&(pParams->myFontsRefList));
	
	pParams->lexeme = (char *)malloc( sizeof(char) * MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(char) );
	if ( !(pParams->lexeme) )
	{
		//wprintf(L"ERRORE Parse 1: Memoria insufficiente.\n\n");
		fwprintf(pParams->fpErrors, L"ERRORE Parse 1: Memoria insufficiente.\n\n");
		retValue = 0;
		goto uscita;
	}
	for ( x = 0; x < MAX_STRING_LENTGTH_IN_CONTENT_STREAM; x++ )
		pParams->lexeme[x] = '\0';
			
	pParams->pUtf8String = (wchar_t*)malloc( sizeof(wchar_t) * MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(wchar_t) );
	if ( !(pParams->pUtf8String) )
	{
		//wprintf(L"ERRORE Parse 2: Memoria insufficiente.\n\n");
		fwprintf(pParams->fpErrors, L"ERRORE Parse 2: Memoria insufficiente.\n\n");
		retValue = 0;
		goto uscita;
	}
	for ( x = 0; x < MAX_STRING_LENTGTH_IN_CONTENT_STREAM; x++ )
		pParams->pUtf8String[x] = L'\0';
	
	pParams->myBlock = (unsigned char *)malloc(sizeof(unsigned char) * BLOCK_SIZE);
	if ( !(pParams->myBlock) )
	{
		//wprintf(L"ERRORE Parse 3: Memoria insufficiente.\n\n");
		fwprintf(pParams->fpErrors, L"ERRORE Parse 3: Memoria insufficiente.\n\n");
		retValue = 0;
		goto uscita;
	}	
	
	pParams->myBlockLengthObj = (unsigned char *)malloc(sizeof(unsigned char) * BLOCK_SIZE);
	if ( !(pParams->myBlockLengthObj) )
	{
		//wprintf(L"ERRORE Parse 4: Memoria insufficiente.\n\n");
		fwprintf(pParams->fpErrors, L"ERRORE Parse 4: Memoria insufficiente.\n\n");
		retValue = 0;
		goto uscita;
	}	
		

	n = myFilesList;
	while( n != NULL )
	{
		pParams->isEncrypted = 0;
			
		pParams->nCountPagesFromPdf = 0;
		if ( NULL != pParams->pPagesArray )
		{
			free(pParams->pPagesArray);
			pParams->pPagesArray = NULL;
		}
					
		/*
		int lung = strnlen(n->myPathName, MAX_LEN_STR);
		wprintf(L"COPIO IL FILE = <");
		for ( int idx = 0; idx < lung; idx++ )
		{
			wprintf(L"%c", n->myPathName[idx]);
		}
		wprintf(L">\n\n");
		*/
			
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
			wprintf(L"ERRORE Parse 5: nell'apertura del file '%s'.\n", pParams->szFileName);
			fwprintf(pParams->fpErrors, L"ERRORE Parse 5: nell'apertura del file '%s'.\n\n", pParams->szFileName);
			
			int lung = strnlen(pParams->szFileName, MAX_LEN_STR);
			wprintf(L"FILE SCHIFOSO = <");
			for ( int idx = 0; idx < lung; idx++ )
			{
				wprintf(L"%c", pParams->szFileName[idx]);
			}
			wprintf(L">\n\n");
							
			retValue = 0;
			goto successivo;
		}
		
		//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FN)	
		if ( pParams->szOutputFile[0] != '\0' )
		{
			#if !defined(_WIN64) && !defined(_WIN32)
			fwprintf(pParams->fpOutput, L"File: '%s'\n", pParams->szFileName);
			#else
			int lung = strnlen(pParams->szFileName, MAX_LEN_STR);
			fwprintf(pParams->fpOutput, L"FILE = <");
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
			int lung = strnlen(pParams->szFileName, MAX_LEN_STR);
			wprintf(L"FILE = <");
			for ( int idx = 0; idx < lung; idx++ )
			{
				wprintf(L"%c", pParams->szFileName[idx]);
			}
			wprintf(L">\n");
			#endif
		}
		//#endif		

		if ( !ReadHeader(pParams) )
		{
			//wprintf(L"ERRORE Parse 5 bis nella lettura del file header '%s'.\n\n", pParams->szFileName);
			fwprintf(pParams->fpErrors, L"ERRORE Parse 5 bis nella lettura del file header '%s'.\n\n", pParams->szFileName);
			retValue = 0;
			goto successivo;
		}
		
		pParams->myPdfTrailer.Size = 0;
		pParams->myPdfTrailer.Prev = 0;
		pParams->myPdfTrailer.Root.Number = 0;
		pParams->myPdfTrailer.Root.Generation = 0;
	
		pParams->isEncrypted = 0;
	
		mynumstacklist_Init( &(pParams->myNumStack) );		
		
		if ( !ReadTrailer(pParams) )
		{
			if ( pParams->isEncrypted )
				//wprintf(L"Encrypted file '%s'.\n", pParams->szFileName);
				fwprintf(pParams->fpErrors, L"Encrypted file '%s'.\n", pParams->szFileName);
			else
				//wprintf(L"ERRORE Parse 6 nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
				fwprintf(pParams->fpErrors, L"ERRORE Parse 6 nella lettura del trailer del file '%s'.\n\n", pParams->szFileName);
			retValue = 0;
			mynumstacklist_Free( &(pParams->myNumStack) );
			goto successivo;
		}
		
		mynumstacklist_Free( &(pParams->myNumStack) );
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)		
		wprintf(L"ReadTrailer OK!!!\n");
			
		for ( x = 0; x < pParams->myPdfTrailer.Size; x++)
		{
			if ( pParams->myObjsTable[x] != NULL )
			{
				if ( !bPrintObjsAndExit )
					wprintf(L"Obj[%d] = <%d %d> Offset: %d\n", x, pParams->myObjsTable[x]->Obj.Number, pParams->myObjsTable[x]->Obj.Generation, pParams->myObjsTable[x]->Offset);
				if ( bPrintObjsAndExit && x > 0 )
					PrintThisObject(pParams, x, 0, 0, pParams->fpOutput);
			}
		}
		if ( bPrintObjsAndExit )
			goto uscita;
					
		wprintf(L"Trailer Size: %d\n", pParams->myPdfTrailer.Size);
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
			for ( x = 0; x < pParams->myPdfTrailer.Size; x++ )
			{
				if ( pParams->myObjsTable[x] != NULL )
				{
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
	
		//if ( NULL != pParams->paCustomizedFont_CharSet )
		//{
		//	free(pParams->paCustomizedFont_CharSet);
		//	pParams->paCustomizedFont_CharSet = NULL;
		//}
//**********************************************************************************************************************************
		
		n = n->next;
	}	

uscita:

	if ( pParams->lexeme != NULL )
	{
		free(pParams->lexeme);
		pParams->lexeme = NULL;
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
		for ( x = 0; x < pParams->myPdfTrailer.Size; x++ )
		{
			if ( pParams->myObjsTable[x] != NULL )
			{
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
	
	return retValue;
}

int match(Params *pParams, TokenTypeEnum ExpectedToken, char *pszFunctionName)
{
	int retValue = 1;
	
	if ( pParams->myToken.Type == ExpectedToken )
	{
		GetNextToken(pParams);
	}
	else
	{
		retValue = 0;
		
		if ( NULL != pszFunctionName )
			fwprintf(pParams->fpErrors, L"\nFUNZIONE match(richiamata dalla funzione '%s' -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pszFunctionName, ExpectedToken, pParams->myToken.Type);
			//wprintf(L"\nFUNZIONE match(richiamata dalla funzione '%s' -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pszFunctionName, ExpectedToken, pParams->myToken.Type);
		else
			fwprintf(pParams->fpErrors, L"\nFUNZIONE match -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", ExpectedToken, pParams->myToken.Type);			
			//wprintf(L"\nFUNZIONE match -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", ExpectedToken, pParams->myToken.Type);			
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
			fwprintf(pParams->fpErrors, L"\nFUNZIONE matchLengthObj(richiamata dalla funzione '%s' -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pszFunctionName, ExpectedToken, pParams->myToken.Type);
			//wprintf(L"\nFUNZIONE matchLengthObj(richiamata dalla funzione '%s' -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", pszFunctionName, ExpectedToken, pParams->myToken.Type);
		else
			fwprintf(pParams->fpErrors, L"\nFUNZIONE matchLengthObj -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", ExpectedToken, pParams->myToken.Type);
			//wprintf(L"\nFUNZIONE matchLengthObj -> errore di sintassi: Atteso token n° %d, trovato token n° %d\n", ExpectedToken, pParams->myToken.Type);
	}	
		
#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_MATCH)
	
	if ( 0 == retValue )
		//wprintf(L"Token atteso : ");
	
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
	
	int nNumFilter;
	
	int ret = 0;
	
	unsigned long int k;
	
	unsigned long int DecodedStreamSize = 0;
	
	unsigned char szTemp[4096];
	unsigned char *pszEncodedStream = NULL;
	
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
		
	szTemp[0] = '\0';
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	wprintf(L"***** INIZIO PushXObjDecodedContent PAGE SCOPE TRAVERSE *****\n\n");
	scopeTraverse(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef), myOnTraverse, 0);
	wprintf(L"***** FINE   PushXObjDecodedContent PAGE SCOPE TRAVERSE *****\n");
	#endif
	
	if ( pParams->CurrentContent.LengthFromPdf > 0 )
	{
		pszEncodedStream = (unsigned char*)malloc( pParams->CurrentContent.LengthFromPdf + sizeof(unsigned char) );
		if ( NULL == pszEncodedStream )
		{
			wprintf(L"ERRORE PushXObjDecodedContent: impossibile allocare %lu byte per leggere lo stream\n", pParams->CurrentContent.LengthFromPdf);
			fwprintf(pParams->fpErrors, L"ERRORE PushXObjDecodedContent: impossibile allocare %lu byte per leggere lo stream\n", pParams->CurrentContent.LengthFromPdf);
			retValue = 0;
			goto uscita;
		}
				
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)		
		wprintf(L"\nSTAMPO LO STREAM(Pag. %d) n° %d *********************:\n", nPageNumber, nXObjNumber);
		#endif
		
		DecodedStreamSize = pParams->CurrentContent.LengthFromPdf * 3 + sizeof(unsigned char);
		
		// PUSH
		pParams->nStreamsStackTop++;
	
		pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = (unsigned char *)malloc( DecodedStreamSize );
		if ( NULL == pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream )
		{
			wprintf(L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pszDecodedStream sullo stack %d.\n", pParams->nStreamsStackTop);
			fwprintf(pParams->fpErrors, L"ERRORE PushXObjDecodedContent: impossibile allocare %lu byte per leggere lo stream\n", pParams->CurrentContent.LengthFromPdf);
			pParams->nStreamsStackTop--;
			retValue = 0;
			goto uscita;		
		}	
		
		pParams->myStreamsStack[pParams->nStreamsStackTop].bStreamState = 1;
		pParams->myStreamsStack[pParams->nStreamsStackTop].bStringIsDecoded = 1;
		pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos = 0;
	
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"PushXObjDecodedContent -> AFTER STACK PUSH: pParams->nStreamsStackTop = %d\n", pParams->nStreamsStackTop);
		#endif		
				
		fseek(pParams->fp, pParams->CurrentContent.Offset, SEEK_SET);
		fread(pszEncodedStream, 1, pParams->CurrentContent.LengthFromPdf, pParams->fp);
		
		pszEncodedStream[pParams->CurrentContent.LengthFromPdf] = '\0';
					
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"STREAM -> Length = %lu", pParams->CurrentContent.LengthFromPdf);
		wprintf(L"\n");
		#endif
					
		//retPeek = 0;
		//bBreak = 0;
		nNumFilter = 0;

		if ( pParams->CurrentContent.queueFilters.count <= 0 )
		{	
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
			wprintf(L"Filters -> NONE, NESSUNO, NIENTE, NADA\n");
			#endif
					
			DecodedStreamSize = pParams->CurrentContent.LengthFromPdf;
			
			pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize = DecodedStreamSize;
			pParams->myStreamsStack[pParams->nStreamsStackTop].blockLen = pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize;			
			
			memcpy(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream, pszEncodedStream, DecodedStreamSize);
			
			//wprintf(L"\n####################################### HO APPENA PIAZZATO NELLO STACK QUESTA STRINGA -> pszEncodedStream:\n");
			//wprintf(L"%s", pszEncodedStream);
			//wprintf(L"\n####################################### FINE PIAZZAMENTO NELLO STACK DELLA STRINGA -> pszEncodedStream.\n\n");
			
			pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[pParams->CurrentContent.LengthFromPdf] = '\0';
			
			free(pszEncodedStream);
			pszEncodedStream = NULL;
				
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
			
			
			goto uscita;
		}
		
// -----------------------------------------------------------------------------------------------------------------------------------------------
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)		
		wprintf(L"Filters -> [ ");
		#endif
				
		while ( mystringqueuelist_Dequeue(&(pParams->CurrentContent.queueFilters), (char*)szTemp) )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
			wprintf(L"%s ", szTemp);
			#endif
											
			if (pParams->CurrentContent.decodeParms.count <= 0)
			{
				if ( strncmp((char*)szTemp, "FlateDecode", 4096) == 0 )
				{
					ret = myInflate(&(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream), &DecodedStreamSize, pszEncodedStream, pParams->CurrentContent.LengthFromPdf);
					if ( Z_OK != ret )
					{
						zerr(ret, pParams->fpErrors);
						free(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream);
						pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = NULL;
						pParams->nStreamsStackTop--;
						retValue = 0;
						goto uscita;
					}
					pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[DecodedStreamSize] = '\0';
					
					pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize = DecodedStreamSize;
					pParams->myStreamsStack[pParams->nStreamsStackTop].blockLen = pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize;					
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent)
					wprintf(L"\n\nPushXObjDecodedContent(XOBJ %d) -> INIZIO STREAM DECODIFICATO DOPO myInflate:\n", nXObjNumber);
					for ( k = 0; k < DecodedStreamSize; k++ )
					{
						if ( pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k] == '\0' )
							wprintf(L"\\0");
						else
							wprintf(L"%c", pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k]);
					}
					wprintf(L"\nPushXObjDecodedContent(XOBJ %d) -> FINE STREAM DECODIFICATO DOPO myInflate.\n", nXObjNumber);
					#endif
					
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent_HEXCODECHAR)
					wprintf(L"\n\nPushXObjDecodedContent(XOBJ %d) -> INIZIO STREAM DECODIFICATO DOPO myInflate(HEXCODECHAR):\n", nXObjNumber);
					for ( k = 0; k < DecodedStreamSize; k++ )
					{
						if ( pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k] == '\0' )
							wprintf(L"<00>");
						else
							wprintf(L"<%X>", pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k]);
					}
					wprintf(L"\nPushXObjDecodedContent(XOBJ %d) -> FINE STREAM DECODIFICATO DOPO myInflate(HEXCODECHAR).\n", nXObjNumber);
					#endif
					
					//fwrite("\xEF\xBB\xBF", 3, 1, fpTemp);
					//fwrite(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream, DecodedStreamSize, 1, fpTemp);
				}				
			}
				
			nNumFilter++;
	
			//bBreak = 0;
		}
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)		
		wprintf(L"]\n");
		wprintf(L"FINE STREAM(Pag. %d) n° %d:     *********************.\n", nPageNumber, nXObjNumber);
		#endif
	}
	
uscita:

	//if ( NULL != fpTemp )
	//{
	//	fclose(fpTemp);
	//	fpTemp = NULL;
	//}

	if ( NULL != pszEncodedStream )
	{
		free(pszEncodedStream);
		pszEncodedStream = NULL;
	}
	
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
	
	//int bTcNegState = 0;
	//int bTwNegState = 0;	
	//int bTcState = 0;
	
	//int bPrevNumberIsReal = 0;
	//int iPrevNumber = 0;
	//double dPrevNumber = 0.0;	
	
	int bLastNumberIsReal = 0;
	int iLastNumber = 0;
	double dLastNumber = 0.0;
	
	double dFontSize = 12.0;
	
	pParams->bReadingStringsFromDecodedStream = 1;
	
	//size_t k;	
	
	//#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	//UNUSED(nPageNumber);
	//#endif	
	
	pszString = (unsigned char *)malloc(MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(unsigned char));
	if ( NULL == pszString )
	{
		wprintf(L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pszString.\n");
		fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pszString.\n");
		retValue = 0;
		goto uscita;
	}
		
	pParams->pwszCurrentWord = (wchar_t*)malloc(MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(wchar_t));
	if ( NULL == pParams->pwszCurrentWord )
	{
		wprintf(L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pwszCurrentWord.\n");
		fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pszString.\n");
		retValue = 0;
		goto uscita;
	}	
	
	pParams->pwszPreviousWord = (wchar_t*)malloc(MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(wchar_t));
	if ( NULL == pParams->pwszPreviousWord )
	{
		wprintf(L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pwszPreviousWord.\n");
		fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pszString.\n");
		retValue = 0;
		goto uscita;
	}	
	
	pParams->idxCurrentWordChar = 0;
	pParams->pwszCurrentWord[0] = '\0';
	pParams->idxPreviousWordChar = 0;
	pParams->pwszPreviousWord[0] = '\0';
	
	pParams->bStateSillab = 0;
	
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
				wprintf(L"ERRORE ManageDecodedContent: Memoria insufficiente.\n\n");
				fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: Memoria insufficiente.\n\n");
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
		//pParams->myBlock[pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize] = '\0';
		//wprintf(L"Page %d -> MY BLOCK(len = %d, size = %lu) ->\n@>%s<@\n", nPageNumber, pParams->blockLen, nBlockSize, pParams->myBlock);
		wprintf(L"Page %d -> MY BLOCK(len = %d, size = %lu)\n", nPageNumber, pParams->blockLen, nBlockSize);
		#endif
			
		szName[0] = '\0';
	
		PrevType = pParams->myToken.Type;
				
		GetNextToken(pParams);
		
		//if ( T_ERROR == pParams->myToken.Type )
		//{
		//	wprintf(L"\nERRORE GETNEXTTOKEN X -> INIZIO STREAM CORRENTE\n**************************************************************************\n");
		//	wprintf(L"%s", pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream);
		//	wprintf(L"\nERRORE GETNEXTTOKEN X -> FINE   STREAM CORRENTE\n**************************************************************************\n\n");
		//	retValue = 0;
		//	goto uscita;
		//}
		
		//PrintToken(&(pParams->myToken), ' ', ' ', 1);
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
						
							//if ( !bTcNegState && !bTwNegState )
							//	InsertWordIntoTst(pParams);
																										
							InsertWordIntoTst(pParams);
							//wprintf(L"INSERITA WORD GENERICA. CIAO\n");
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
				//bPrevNumberIsReal = 0;

				//iPrevNumber = iLastNumber;
				iLastNumber = pParams->myToken.Value.vInt;
				//dLastNumber = 0.0;
				
				if ( bArrayState )
				{
					//wprintf(L"FONT SIZE = %d <> iLastNumber = %d\n", (int)dFontSize, iLastNumber);
					//if ( iLastNumber < 0 )
					if ( iLastNumber < -((int)dFontSize) )
					{
						//wprintf(L"ECCOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO -> FONT SIZE = %d <> iLastNumber = %d\n", (int)dFontSize, iLastNumber);
						
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings)
						//wprintf(L" <SPAZIO INTEGER %d> ", iLastNumber);
						wprintf(L" ");
						#endif
						if ( pParams->szFilePdf[0] == '\0' )
						{
							//wprintf(L"QUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA -> FONT SIZE = %d <> iLastNumber = %d\n", (int)dFontSize, iLastNumber);
							
							InsertWordIntoTst(pParams);
							//wprintf(L"INSERITA WORD IN ARRAY STATE CASE INTEGER. CIAO\n");
						}
						else
						{
							if ( pParams->szOutputFile[0] != '\0' )
								fwprintf(pParams->fpOutput, L" ");
							else
								wprintf(L" ");
							//wprintf(L" ");
						}
					}
				}
			}
			else if ( T_REAL_LITERAL == pParams->myToken.Type )
			{
				bLastNumberIsReal = 1;
				//bPrevNumberIsReal = 1;
				
				//dPrevNumber = pParams->myToken.Value.vDouble;
				dLastNumber = pParams->myToken.Value.vDouble;
				//iLastNumber = 0;
				
				if ( bArrayState )
				{
					//wprintf(L"FONT SIZE = %f <> dLastNumber = %d\n", dFontSize, dLastNumber);
					//if ( dLastNumber < 0.0 )
					if ( dLastNumber < -dFontSize )
					{
						//wprintf(L"ECCOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO -> FONT SIZE = %f <> dLastNumber = %f\n", dFontSize, dLastNumber);
						
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings)
						//wprintf(L" <SPAZIO REAL %f> ", dLastNumber);
						wprintf(L" ");
						#endif
						
						if ( pParams->szFilePdf[0] == '\0' )
						{
							//wprintf(L"QUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA -> FONT SIZE = %f <> iLastNumber = %f\n", dFontSize, dLastNumber);
							
							InsertWordIntoTst(pParams);
							//wprintf(L"INSERITA WORD IN ARRAY STATE CASE REAL. CIAO\n");
						}
						else
						{
							if ( pParams->szOutputFile[0] != '\0' )
								fwprintf(pParams->fpOutput, L" ");
							else
								wprintf(L" ");							
							//wprintf(L" ");						
						}
					}
				}
			}
			/*
			else if ( T_CONTENT_OP_Tc == pParams->myToken.Type )
			{
				bTcState = 1;
				
						
				//if ( bLastNumberIsReal )
				//{
				//	if ( dLastNumber < 0.0 )
				//		bTcNegState = 1;
				//	else
				//		bTcNegState = 0;
				//}
				//else 
				//{
				//	if ( iLastNumber < 0 )
				//		bTcNegState = 1;
				//	else
				//		bTcNegState = 0;
				//}
								
			}
			*/			
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
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
					wprintf(L"TROVATO 'Do' command: vado a prendere la Resource %s\n", szName);
					#endif
									
					nRes = scopeFind(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef), szName, len + sizeof(char), (void*)&nTemp, &nDataSize, &bContentAlreadyProcessed, 1);
					if ( nRes >= 0 ) // TROVATO
					{
						if ( !bContentAlreadyProcessed )
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
							wprintf(L"\tVado a fare il parsing dell'oggetto %d 0 R e torno subito.\n", nTemp);
							#endif
					
							bContentAlreadyProcessed = 1;
							if ( !scopeUpdateValue(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef), szName, len + sizeof(char), (void*)&nTemp, nDataSize, bContentAlreadyProcessed, 1, 1) )
							{
								wprintf(L"ERRORE ManageDecodedContent scopeUpdateValue : impossibile aggiornare bContentAlreadyProcessed\n"); 
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
								//wprintf(L"ERRORE ManageDecodedContent ParseStreamXObject.\n");
								fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent ParseStreamXObject.\n");
								//wprintf(L"\n***** ECCO LO SCHIFO:\n");
								fwprintf(pParams->fpErrors, L"\n***** ECCO LO SCHIFO:\n");
								PrintThisObject(pParams, nTemp, 0, 0, pParams->fpErrors);
								//wprintf(L"\n***** FINE DELLO SCHIFO\n"); 
								fwprintf(pParams->fpErrors, L"\n***** FINE DELLO SCHIFO\n"); 
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
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
						else
						{
							wprintf(L"\tOggetto %d 0 R già processato.\n", nTemp);
						}
						#endif
					}
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
					else
					{
						wprintf(L"\tRISORSA XOBJ '%s' NON TROVATA!!!.\n", szName);
					}
					#endif					
				}
				else
				{
					//wprintf(L"ERRORE ManageDecodedContent: trovato comando 'Do' ma non e' preceduto dal nome della Resource!\n");
					fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: trovato comando 'Do' ma non e' preceduto dal nome della Resource!\n");
					retValue = 0;
					goto uscita;
				}
				szName[0] = '\0';
			}
			else if ( T_CONTENT_OP_Tf == pParams->myToken.Type )
			{
				if ( '\0' != szName[0] )
				{
					if ( bLastNumberIsReal )
						dFontSize = dLastNumber;
					else
						dFontSize = (double)iLastNumber;
					
					len = strnlen(szName, 128);
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
					wprintf(L"TROVATO 'Tf FONT SELECTOR' command: vado a prendere la Resource %s\n", szName);
					#endif
									
					//nRes = scopeFind(&(pParams->pPagesArray[nPageNumber].myScopeHT_FontsRef), szName, len + sizeof(char), (void*)&nTemp, &nDataSize, &bContentAlreadyProcessed, 1);
					nRes = scopeFind(&(pParams->pPagesArray[nPageNumber].myScopeHT_FontsRef), szName, len + sizeof(char), (void*)&nTemp, &nDataSize, &bContentAlreadyProcessed, 0);
					if ( nRes >= 0 ) // TROVATO
					{
						//if ( !bContentAlreadyProcessed )
						//{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
							wprintf(L"\tVado a fare il parsing dell'oggetto FONT %d 0 R e torno subito.\n", nTemp);
							#endif
					
							bContentAlreadyProcessed = 1;
							//if ( !scopeUpdateValue(&(pParams->pPagesArray[nPageNumber].myScopeHT_FontsRef), szName, len + sizeof(char), (void*)&nTemp, nDataSize, bContentAlreadyProcessed, 1, 1) )
							if ( !scopeUpdateValue(&(pParams->pPagesArray[nPageNumber].myScopeHT_FontsRef), szName, len + sizeof(char), (void*)&nTemp, nDataSize, bContentAlreadyProcessed, 1, 0) )
							{
								wprintf(L"ERRORE ManageDecodedContent scopeUpdateValue : impossibile aggiornare bContentAlreadyProcessed\n"); 
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
								fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent ParseFontObject.\n"); 
								//wprintf(L"\n***** ECCO L'OGGETTO ERRATO:\n");
								fwprintf(pParams->fpErrors, L"\n***** ECCO L'OGGETTO ERRATO:\n");
								
								PrintThisObject(pParams, nTemp, 0, 0, pParams->fpErrors);
								
								//PrintThisObject(pParams, 71, 0, 0, pParams->fpErrors);
								//PrintThisObject(pParams, 72, 0, 0, pParams->fpErrors);
								//PrintThisObject(pParams, 73, 0, 0, pParams->fpErrors);
								//PrintThisObject(pParams, 74, 0, 0, pParams->fpErrors);								
								//PrintThisObject(pParams, 75, 0, 0, pParams->fpErrors);
								
																
								//wprintf(L"\n***** FINE OGGETTO ERRATO\n");
								fwprintf(pParams->fpErrors, L"\n***** FINE OGGETTO ERRATO\n");
								retValue = 0;
								goto uscita;
							}
														
							pParams->bStreamState = pParams->myStreamsStack[pParams->nStreamsStackTop].bStreamState;
							pParams->bStringIsDecoded = pParams->myStreamsStack[pParams->nStreamsStackTop].bStringIsDecoded;
							pParams->blockCurPos = pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos;
							
							goto qui_dopo_push;
							// **********************************************************************
						//}
						//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
						//else
						//{
						//	wprintf(L"\tOggetto FONT %d 0 R già processato.\n", nTemp);
						//}
						//#endif
					}
					else
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
						wprintf(L"\tRISORSA FONT '%s' NON TROVATA!!!.\n", szName);
						#endif						
					}
				}
				else
				{
					//wprintf(L"ERRORE ManageDecodedContent: trovato comando 'Do' ma non e' preceduto dal nome della Resource!\n");
					fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: trovato comando 'Do' ma non e' preceduto dal nome della Resource!\n");
					retValue = 0;
					goto uscita;
				}
				szName[0] = '\0';
			}
			else if ( T_CONTENT_OP_BI == pParams->myToken.Type )
			{
				// IGNORIAMO L'OPERATORE BI(Begin Image) FINO ALLA FINE DELLO STREAM
				goto libera;
			}			
							
			GetNextToken(pParams);
			
			//if ( T_ERROR == pParams->myToken.Type )
			//{
			//	wprintf(L"\nERRORE GETNEXTTOKEN Y -> INIZIO STREAM CORRENTE\n**************************************************************************\n");
			//	wprintf(L"%s", pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream);
			//	wprintf(L"\nERRORE GETNEXTTOKEN Y -> FINE   STREAM CORRENTE\n**************************************************************************\n\n");
			
			//	retValue = 0;
			//	goto uscita;
			//}
			
			//PrintToken(&(pParams->myToken), ' ', ' ', 1);
			
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
	}   // FINE while ( pParams->nStreamsStackTop >= 0 )
	
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
	
	//scopeFree(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef));
	//scopeFree(&(pParams->pPagesArray[nPageNumber].myScopeHT_FontsRef));
	
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
	
	int ret = 0;
	
	MyContent_t myContent;
	MyContent_t *pContent;
	MyContentQueueItem_t* pContentItem;
	unsigned long int totalLengthFromPdf = 0;
	
	unsigned long int k;
	
	unsigned long int DecodedStreamSize = 0;
	
	unsigned char szTemp[4096];
	
	unsigned char *pszDecodedStreamNew = NULL;
	
	unsigned char *pszEncodedStream = NULL;
	unsigned long int offsetEncodedStream = 0;
	
	unsigned char *pszDecodedStream = NULL;
	unsigned long int offsetDecodedStream = 0;
	
	unsigned long int bytesAllocatedForDecodedStreamOnStack = 0;
	
	int nTemp;
	
	size_t bytesRead;
		
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent) && !defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent_HEXCODECHAR)
	UNUSED(k);
	#endif
	
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
	UNUSED(nPageNumber);
	#endif	
	
	szTemp[0] = '\0';
	
	for ( nTemp = 0; nTemp < STREAMS_STACK_SIZE; nTemp++ )
		pParams->myStreamsStack[nTemp].pszDecodedStream = NULL;	
	
	pContentItem = pParams->pPagesArray[nPageNumber].queueContens.head;
	totalLengthFromPdf = 0;
	while ( NULL != pContentItem )
	{
		totalLengthFromPdf += pContentItem->myContent.LengthFromPdf;
		pContentItem = pContentItem->next;
		//countContents++;
	}
	
	if ( totalLengthFromPdf <= 0 )
		goto uscita;
				
	// PUSH BEGIN
	pParams->nStreamsStackTop = 0;
	
	//wprintf(L"totalLengthFromPdf = %lu BYTE\n", totalLengthFromPdf);
	
	DecodedStreamSize = ( totalLengthFromPdf * sizeof(unsigned char) ) * 55 + sizeof(unsigned char);
	
	if ( DecodedStreamSize > 409600000 )
		DecodedStreamSize = 409600000;
		
	if ( DecodedStreamSize < totalLengthFromPdf )
		DecodedStreamSize = totalLengthFromPdf + (4096 * 89);
	
	//wprintf(L"DecodedStreamSize = %lu BYTE -> ( totalLengthFromPdf * sizeof(unsigned char) ) * 55 + sizeof(unsigned char)\n", DecodedStreamSize);
		
	pszEncodedStream = (unsigned char*)malloc( totalLengthFromPdf * sizeof(unsigned char) + sizeof(unsigned char) );
	if ( NULL == pszEncodedStream )
	{
		wprintf(L"ERRORE ManageContent: impossibile allocare %lu byte per leggere lo stream\n", totalLengthFromPdf * sizeof(unsigned char) + sizeof(unsigned char));
		fwprintf(pParams->fpErrors, L"ERRORE ManageContent: impossibile allocare %lu byte per leggere lo stream\n", totalLengthFromPdf * sizeof(unsigned char) + sizeof(unsigned char));
		retValue = 0;
		goto uscita;
	}
	//wprintf(L"ALLOCATI %lu BYTE PER pszEncodedStream\n", totalLengthFromPdf * sizeof(unsigned char) + sizeof(unsigned char));
	
	pszDecodedStream = (unsigned char *)malloc( DecodedStreamSize );
	if ( NULL == pszDecodedStream )
	{
		wprintf(L"ERRORE ManageContent: impossibile allocare la memoria per pszDecodedStream sullo stack %d.\n", pParams->nStreamsStackTop);
		fwprintf(pParams->fpErrors, L"ERRORE ManageContent: impossibile allocare la memoria per pszDecodedStream sullo stack %d.\n", pParams->nStreamsStackTop);
		pParams->nStreamsStackTop--;
		retValue = 0;
		goto uscita;		
	}
	//wprintf(L"ALLOCATI %lu BYTE PER pszDecodedStream\n", DecodedStreamSize);
		
	pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = (unsigned char *)malloc( DecodedStreamSize );
	if ( NULL == pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream )
	{
		wprintf(L"ERRORE ManageContent: impossibile allocare la memoria per pszDecodedStream sullo stack %d.\n", pParams->nStreamsStackTop);
		fwprintf(pParams->fpErrors, L"ERRORE ManageContent: impossibile allocare la memoria per pszDecodedStream sullo stack %d.\n", pParams->nStreamsStackTop);
		pParams->nStreamsStackTop--;
		retValue = 0;
		goto uscita;		
	}	
	//wprintf(L"ALLOCATI %lu BYTE PER pParams->myStreamsStack[%d].pszDecodedStream\n", DecodedStreamSize, pParams->nStreamsStackTop);
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
		szTemp[0] = '\0';
		while ( mystringqueuelist_Dequeue(&(pContent->queueFilters), (char*)szTemp) )
		{
			if (pContent->decodeParms.count <= 0)
			{
				if ( strncmp((char*)szTemp, "FlateDecode", 4096) == 0 )
				{						
					fseek(pParams->fp, pContent->Offset, SEEK_SET);
						
					bytesRead = fread(pszEncodedStream, 1, pContent->LengthFromPdf, pParams->fp);
					if ( bytesRead < pContent->LengthFromPdf )
					{
						wprintf(L"\nERRORE ManageContent: fread ha letto %lu byte quando il pdf ne specifica, invece, %lu\n", bytesRead, pContent->LengthFromPdf);
						fwprintf(pParams->fpErrors, L"\nERRORE ManageContent: fread ha letto %lu byte quando il pdf ne specifica, invece, %lu\n", bytesRead, pContent->LengthFromPdf);
						retValue = 0;
						goto uscita;
					}
						
					offsetEncodedStream += pContent->LengthFromPdf;
					
					pszEncodedStream[pContent->LengthFromPdf] = '\0';						
		
					pszDecodedStream[0] = '\0';
					
					DecodedStreamSize = ( totalLengthFromPdf * sizeof(unsigned char) ) * 5 + sizeof(unsigned char);	
								
					ret = myInflate(&(pszDecodedStream), &DecodedStreamSize, pszEncodedStream, pContent->LengthFromPdf);
					if ( Z_OK != ret )
					{
						zerr(ret, pParams->fpErrors);
						if ( NULL != pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream )
						{
							free(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream);
							pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = NULL;
						}
						pParams->nStreamsStackTop--;
						retValue = 0;
						goto uscita;
					}
					pszDecodedStream[DecodedStreamSize] = '\0';
					
					
					//wprintf(L"COPIO %lu (<- DecodedStreamSize) BYTES SU pParams->myStreamsStack[%d].pszDecodedStream, ALL'OFFSET %lu (<- offsetDecodedStream)\n\n",
					//		DecodedStreamSize,
					//		pParams->nStreamsStackTop,
					//		offsetDecodedStream);
					
					
					if ( DecodedStreamSize > (bytesAllocatedForDecodedStreamOnStack - offsetDecodedStream) )
					{
						bytesAllocatedForDecodedStreamOnStack = DecodedStreamSize * 3;
						
						pszDecodedStreamNew = (unsigned char*)realloc(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream, bytesAllocatedForDecodedStreamOnStack);
						if ( NULL == pszDecodedStreamNew )
						{
							wprintf(L"ERRORE ManageContent: impossibile reallocare %lu byte per leggere lo stream\n", bytesAllocatedForDecodedStreamOnStack);
								      
							fwprintf(pParams->fpErrors, L"ERRORE ManageContent: impossibile reallocare %lu byte per leggere lo stream\n", bytesAllocatedForDecodedStreamOnStack);
							      								      
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
				else
				{
					fwprintf(pParams->fpErrors, L"ERRORE ManageContent: filtro '%s' non supportato in questa versione del programma.\n", szTemp);
					retValue = 0;
					goto uscita;
				}
			}
			else
			{
				fwprintf(pParams->fpErrors, L"ERRORE ManageContent: filtro FlateDecode con parametri non supportato in questa versione del programma.\n");
				retValue = 0;
				goto uscita;				
			}
		}
	}
	pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[offsetDecodedStream] = '\0';
	
	pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize = DecodedStreamSize;
	pParams->myStreamsStack[pParams->nStreamsStackTop].blockLen = DecodedStreamSize;
	pParams->myStreamsStack[pParams->nStreamsStackTop].bStreamState = 1;
	pParams->myStreamsStack[pParams->nStreamsStackTop].bStringIsDecoded = 1;
	pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos = 0;	
	// PUSH END	
		
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	wprintf(L"STREAM -> Length = %lu", totalLengthFromPdf);
	wprintf(L"\n");
	#endif	

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent)
	wprintf(L"\n\nManageContent -> INIZIO STREAM DECODIFICATO DOPO myInflate:\n");
	for ( k = 0; k < offsetDecodedStream; k++ )
	{
		if ( pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k] == '\0' )
			wprintf(L"\\0");
		else
			wprintf(L"%c", pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k]);
	}
	wprintf(L"ManageContent -> FINE STREAM DECODIFICATO DOPO myInflate.\n\n");
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

	//#define MYDEBUG_PRINT_ON_GetNextToken_FN 1
	ManageDecodedContent(pParams, nPageNumber);
	//#undef MYDEBUG_PRINT_ON_GetNextToken_FN
			
uscita:

	if ( NULL != pszEncodedStream )
	{
		free(pszEncodedStream);
		pszEncodedStream = NULL;
	}

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

int ManageContent_OLD(Params *pParams, int nPageNumber)
{
	int retValue = 1;
	
	int nNumFilter;
	
	//int bBreak;
	//int retPeek;
	
	int ret = 0;
	
	MyContent_t myContent;
	MyContent_t *pContent;
	MyContentQueueItem_t* pContentItem;
	unsigned long int totalLengthFromPdf = 0;
	
	unsigned long int k;
	
	unsigned long int DecodedStreamSize = 0;
	unsigned long int ProgressiveDecodedStreamSize = 0;
	
	unsigned char szTemp[4096];
	unsigned char *pszEncodedStream = NULL;
	unsigned char *pszDecodedStream = NULL;
	unsigned char *pszDecodedStreamNew = NULL;
	
	unsigned long int offsetDecodedStream = 0;
		
	int nTemp;
		
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent)	
	UNUSED(k);
	#endif
	
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
	UNUSED(nPageNumber);
	#endif	
	
	szTemp[0] = '\0';
	
	for ( nTemp = 0; nTemp < STREAMS_STACK_SIZE; nTemp++ )
		pParams->myStreamsStack[nTemp].pszDecodedStream = NULL;	
		
			
	pContentItem = pParams->pPagesArray[nPageNumber].queueContens.head;
	totalLengthFromPdf = 0;
	while ( NULL != pContentItem )
	{
		totalLengthFromPdf += pContentItem->myContent.LengthFromPdf;
		pContentItem = pContentItem->next;
	}
	
					
	if ( totalLengthFromPdf > 0 )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)		
		wprintf(L"\nSTAMPO LO STREAM(Pag. %d) *********************:\n", nPageNumber);
		#endif
		
		pszEncodedStream = (unsigned char*)malloc( totalLengthFromPdf + sizeof(unsigned char) );
		if ( NULL == pszEncodedStream )
		{
			wprintf(L"ERRORE ManageContent: impossibile allocare %lu byte per leggere lo stream\n", totalLengthFromPdf);
			retValue = 0;
			goto uscita;
		}
		
		//DecodedStreamSize = totalLengthFromPdf * sizeof(unsigned char) * 3 + sizeof(unsigned char);
		DecodedStreamSize = totalLengthFromPdf * sizeof(unsigned char) * 8 + sizeof(unsigned char);
		//DecodedStreamSize = totalLengthFromPdf * sizeof(unsigned char) * 21 + sizeof(unsigned char);
		//DecodedStreamSize = totalLengthFromPdf * sizeof(unsigned char) * 34 + sizeof(unsigned char);
				
		// PUSH
		pParams->nStreamsStackTop = 0;
		pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize = DecodedStreamSize;
	
		pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = (unsigned char *)malloc( DecodedStreamSize );
		if ( NULL == pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream )
		{
			wprintf(L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pszDecodedStream sullo stack %d.\n", pParams->nStreamsStackTop);
			fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pszDecodedStream sullo stack %d.\n", pParams->nStreamsStackTop);
			pParams->nStreamsStackTop--;
			retValue = 0;
			goto uscita;		
		}
		
		//wprintf(L"\nALLOCATI %lu BYTES PER pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream\n", DecodedStreamSize);
						
		pParams->myStreamsStack[pParams->nStreamsStackTop].bStreamState = 1;
		pParams->myStreamsStack[pParams->nStreamsStackTop].bStringIsDecoded = 1;
		//pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize = DecodedStreamSize;
		//pParams->myStreamsStack[pParams->nStreamsStackTop].blockLen = pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize;
		pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos = 0;
					
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"ManageContent -> AFTER STACK PUSH: pParams->nStreamsStackTop = %d\n", pParams->nStreamsStackTop);
		#endif		
						
		offsetDecodedStream = 0;
		
		myContent.bExternalFile = 0;
		myContent.LengthFromPdf = 0;
		myContent.Offset = 0;
		mydictionaryqueuelist_Init(&(myContent.decodeParms), 1, 1);
		mystringqueuelist_Init(&(myContent.queueFilters));
				
		// CICLO:
		myCycle:
		
		if ( !mycontentqueuelist_Dequeue(&(pParams->pPagesArray[nPageNumber].queueContens), &myContent) )
			goto gestiscodecodedcontent;
			
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
				
		fseek(pParams->fp, pContent->Offset, SEEK_SET);
		fread(pszEncodedStream, 1, pContent->LengthFromPdf, pParams->fp);
		
		pszEncodedStream[pContent->LengthFromPdf] = '\0';
									
		//retPeek = 0;
		//bBreak = 0;
		nNumFilter = 0;
		
		if ( pContent->queueFilters.count <= 0 )
		{	
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)		
			wprintf(L"Filters -> NONE, NESSUNO, NIENTE, NADA\n");
			#endif
								
			DecodedStreamSize = pContent->LengthFromPdf;
			
			ProgressiveDecodedStreamSize += DecodedStreamSize;			
			
			if ( DecodedStreamSize > (pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize - offsetDecodedStream) )
			{
				pszDecodedStreamNew = ( unsigned char*)realloc(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream,
				                       (pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize + DecodedStreamSize) * 8 + sizeof(unsigned char) );
				                       
				if ( NULL == pszDecodedStreamNew )
				{
					wprintf(L"ERRORE ManageContent: impossibile reallocare %lu byte per leggere lo stream\n",
					       (pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize + DecodedStreamSize) * 8 + sizeof(unsigned char)
					      );
					      
					fwprintf(pParams->fpErrors, L"ERRORE ManageContent: impossibile reallocare %lu byte per leggere lo stream\n",
					       (pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize + DecodedStreamSize) * 8 + sizeof(unsigned char)
					      );					      
					      
					retValue = 0;
					goto uscita;
				}
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
				else
				{
					wprintf(L"\nManageContent X (PAGE %d) -> REALLOCATI CORRETTAMENTE %lu BYTES\n", nPageNumber,
					       (pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize + DecodedStreamSize) * 8 + sizeof(unsigned char)
					      );
				}
				#endif
				pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = pszDecodedStreamNew;
			}
			
			memcpy(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream + offsetDecodedStream, pszEncodedStream, DecodedStreamSize);
			offsetDecodedStream += DecodedStreamSize;
			
			pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[pParams->CurrentContent.LengthFromPdf] = '\0';
			
			free(pszEncodedStream);
			pszEncodedStream = NULL;	
			
			pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize = offsetDecodedStream;
			pParams->myStreamsStack[pParams->nStreamsStackTop].blockLen = pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize;
																			
			goto myCycle;
		}
		else
		{		
			// -----------------------------------------------------------------------------------------------------------------------------------------------		
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)		
			wprintf(L"Filters -> [ ");
			#endif
				
			while ( mystringqueuelist_Dequeue(&(pContent->queueFilters), (char*)szTemp) )
			{
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
				wprintf(L"%s ", szTemp);
				#endif
											
				if (pContent->decodeParms.count <= 0)
				{
					if ( strncmp((char*)szTemp, "FlateDecode", 4096) == 0 )
					{
						DecodedStreamSize = totalLengthFromPdf * 3 + sizeof(unsigned char);
						
						if ( NULL != pszDecodedStream )
						{
							free(pszDecodedStream);
							pszDecodedStream = NULL;
						}						
						pszDecodedStream = (unsigned char*)malloc( DecodedStreamSize + sizeof(unsigned char) );
						if ( NULL == pszDecodedStream )
						{
							wprintf(L"ERRORE ManageContent: impossibile allocare %lu byte per leggere lo stream\n", totalLengthFromPdf);
							fwprintf(pParams->fpErrors, L"ERRORE ManageContent: impossibile allocare %lu byte per leggere lo stream\n", totalLengthFromPdf);
							
							retValue = 0;
							goto uscita;
						}												
																		
						ret = myInflate(&pszDecodedStream, &DecodedStreamSize, pszEncodedStream, pContent->LengthFromPdf);
						if ( Z_OK != ret )
						{
							zerr(ret, pParams->fpErrors);
							free(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream);
							pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = NULL;
							pParams->nStreamsStackTop--;
							retValue = 0;
							goto uscita;
						}
						pszDecodedStream[DecodedStreamSize] = '\0';												
						
						ProgressiveDecodedStreamSize += DecodedStreamSize;
																	
						//wprintf(L"\n\n***** INIZIO STREAM DECODIFICATO X Pag. %d\n", nPageNumber);
						//wprintf(L"%s", pszDecodedStream);
						//wprintf(L"***** FINE STREAM DECODIFICATO X Pag. %d\n\n", nPageNumber);

						if ( DecodedStreamSize > (pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize - offsetDecodedStream) )
						{
							pszDecodedStreamNew = (unsigned char*)realloc(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream,
							                       (pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize + DecodedStreamSize) * 8 + sizeof(unsigned char) );
							if ( NULL == pszDecodedStreamNew )
							{
								wprintf(L"ERRORE ManageContent: impossibile reallocare %lu byte per leggere lo stream\n",
								       (pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize + DecodedStreamSize) * 8 + sizeof(unsigned char)
								      );
								      
								fwprintf(pParams->fpErrors, L"ERRORE ManageContent: impossibile reallocare %lu byte per leggere lo stream\n",
								       (pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize + DecodedStreamSize) * 8 + sizeof(unsigned char)
								      );
								      								      
								retValue = 0;
								goto uscita;
							}
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
							else
							{
								wprintf(L"\nManageContent Y (PAGE %d) -> REALLOCATI CORRETTAMENTE %lu BYTES\n", nPageNumber,
								       (pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize + DecodedStreamSize) * 8 + sizeof(unsigned char)
								      );
							}
							#endif
							pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = pszDecodedStreamNew;
						} 
						
						//wprintf(L"\noffsetDecodedStream = %lu, DecodedStreamSize = %lu, pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize = %lu\n", offsetDecodedStream, DecodedStreamSize, pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize);
						//wprintf(L"PAGE %d: STO PER COPIARE %lu BYTES All'OFFSET %lu DI UNA STRINGA LUNGA %lu BYTES\n\n", nPageNumber, DecodedStreamSize, offsetDecodedStream, pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize);
						//wprintf(L"COPIO %lu BYTES ALL'OFFSET %lu ALL'INDIRIZZO %p\n", DecodedStreamSize, offsetDecodedStream, (pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream + offsetDecodedStream));
						//wprintf(L"COPIO %lu BYTES ALL'OFFSET %lu\n", DecodedStreamSize, offsetDecodedStream);
						
						memcpy(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream + offsetDecodedStream, pszDecodedStream, DecodedStreamSize);
						offsetDecodedStream += DecodedStreamSize;
					
						pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize = offsetDecodedStream;
						pParams->myStreamsStack[pParams->nStreamsStackTop].blockLen = pParams->myStreamsStack[pParams->nStreamsStackTop].DecodedStreamSize;					
					}				
				}
				else
				{
					/*	
					myDataTemp.pszKey = NULL;
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)				
					wprintf(L"(DecodeParms -> { ");
					#endif
				
					while ( mydictionaryqueuelist_Dequeue(&(pContent->decodeParms), &myDataTemp) )
					{										
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)					
						wprintf(L"\n\nNumFilter = %d\n", nNumFilter);
						wprintf(L"myDataTemp.numFilter = %d\n\n", myDataTemp.numFilter);
						#endif
						switch ( myDataTemp.tok.Type )
						{
							case T_INT_LITERAL:
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)						
								wprintf(L"Key(%s) -> Value = ", myDataTemp.pszKey);
								#endif
								free(myDataTemp.pszKey);
								myDataTemp.pszKey = NULL;
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)							
								wprintf(L"%d", myDataTemp.tok.Value.vInt);
								#endif							
								break;
							case T_REAL_LITERAL:
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)						
								wprintf(L"Key(%s) -> Value = ", myDataTemp.pszKey);
								#endif
								free(myDataTemp.pszKey);
								myDataTemp.pszKey = NULL;
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)							
								wprintf(L"%f", myDataTemp.tok.Value.vDouble);
								#endif
								break;
							case T_KW_NULL:
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)						
								wprintf(L"KEYWORD NULL");
								#endif
								break;
							case T_STRING_LITERAL:
							case T_STRING_HEXADECIMAL:
							case T_STRING:
							case T_NAME:
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)						
								wprintf(L"Key(%s) -> Value = ", myDataTemp.pszKey);
								#endif
								free(myDataTemp.pszKey);
								myDataTemp.pszKey = NULL;
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)							
								wprintf(L"'%s'", myDataTemp.tok.Value.vString);
								#endif
								free(myDataTemp.tok.Value.vString);
								myDataTemp.tok.Value.vString = NULL;
								break;
							default:
								break;
						}
					
						retPeek = mydictionaryqueuelist_Peek(&(pContent->decodeParms), &myDataTemp);
						if ( myDataTemp.numFilter != nNumFilter )
						{
							bBreak = 1;
							break;
						}										
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)					
						wprintf(L" ");
						#endif
					}
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
					wprintf(L"}) ");
					#endif
				
					if ( retPeek )
					{
						switch ( myDataTemp.tok.Type )
						{
							case T_INT_LITERAL:
								free(myDataTemp.pszKey);
								myDataTemp.pszKey = NULL;
								break;
							case T_REAL_LITERAL:
								free(myDataTemp.pszKey);
								myDataTemp.pszKey = NULL;					
								break;
							//case T_KW_NULL:
							//	break;
							case T_STRING_LITERAL:
							case T_STRING_HEXADECIMAL:
							case T_STRING:
							case T_NAME:
								free(myDataTemp.pszKey);
								myDataTemp.pszKey = NULL;					
								free(myDataTemp.tok.Value.vString);
								myDataTemp.tok.Value.vString = NULL;
								break;
							default:
								break;
						}					
					}
				
					if ( !bBreak )
						mydictionaryqueuelist_Free(&(pContent->decodeParms));
					*/
				}
				
				nNumFilter++;
	
				//bBreak = 0;
			}
									
			//wprintf(L"\n\n***** INIZIO CONTENT STREAM DECODIFICATO Y (offsetDecodedStream = %lu) Pag. %d\n", offsetDecodedStream, nPageNumber);
			//wprintf(L"%s", pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream);
			//wprintf(L"\n***** FINE   CONTENT STREAM DECODIFICATO Y Pag. %d\n\n", nPageNumber);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
			wprintf(L"]\n");
			#endif				
		
			// -----------------------------------------------------------------------------------------------------------------------------------------------		
		}		
		
		mydictionaryqueuelist_Free(&(myContent.decodeParms));
		mystringqueuelist_Free(&(myContent.queueFilters));		
		
		goto myCycle;
	}	
	
gestiscodecodedcontent:

	//wprintf(L"METTO IL CARATTERE NULLO ALL'OFFSET %ld\n\n", (pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream + ProgressiveDecodedStreamSize) - pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream);
	pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[offsetDecodedStream] = '\0';
	
	//wprintf(L"\n\n***** INIZIO STREAM DECODIFICATO Y Pag. %d\n", nPageNumber);
	//wprintf(L"%s", pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream);
	//wprintf(L"***** FINE STREAM DECODIFICATO Y Pag. %d\n\n", nPageNumber);	
	

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
	wprintf(L"STREAM -> Length = %lu", totalLengthFromPdf);
	wprintf(L"\n");
	#endif	

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent)
	wprintf(L"\nManageContent -> INIZIO STREAM DECODIFICATO DOPO myInflate:\n");
	for ( k = 0; k < DecodedStreamSize; k++ )
	{
		if ( pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k] == '\0' )
			wprintf(L"\\0");
		else
			wprintf(L"%c", pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream[k]);
	}
	wprintf(L"\nManageContent -> FINE STREAM DECODIFICATO DOPO myInflate:\n\n");
	#endif
	
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)		
	wprintf(L"FINE STREAM(Pag. %d)     *********************.\n", nPageNumber);
	#endif	
		
uscita:

	if ( NULL != pszEncodedStream )
	{
		free(pszEncodedStream);
		pszEncodedStream = NULL;
	}
	
	if ( NULL != pszDecodedStream )
	{
		free(pszDecodedStream);
		pszDecodedStream = NULL;
	}
	
	//#define MYDEBUG_PRINT_ON_GetNextToken_FN
	if ( retValue )
		ManageDecodedContent(pParams, nPageNumber);
	//#undef MYDEBUG_PRINT_ON_GetNextToken_FN
	
	while ( pParams->nStreamsStackTop >= 0 )
	{
		if ( NULL != pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream )
			free(pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream);			
		pParams->myStreamsStack[pParams->nStreamsStackTop].pszDecodedStream = NULL;
		
		pParams->nStreamsStackTop--;
	}
			
	return retValue;
}

int ParseObject(Params *pParams, int objNum)
{
	int retValue = 1;
	unsigned char c;
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
		//wprintf(L"Errore ParseObject: objNum non valido -> %d\n", objNum);
		fwprintf(pParams->fpErrors, L"Errore ParseObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->nObjToParse = objNum;
	pParams->nCurrentObjNum = 0;
	
	pParams->nCountPageFound = 0;	
				
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Offset - 1, SEEK_SET) )
	{
		wprintf(L"Errore ParseObject fseek\n");
		fwprintf(pParams->fpErrors, L"Errore ParseObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
	if ( pParams->blockLen < 3 )
	{
		//wprintf(L"Errore ParseObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		fwprintf(pParams->fpErrors, L"Errore ParseObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		retValue = 0;
		goto uscita;
	}
		
	pParams->blockCurPos = 0;	
	
	c = pParams->myBlock[pParams->blockCurPos++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		//wprintf(L"Errore ParseObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		fwprintf(pParams->fpErrors, L"Errore ParseObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
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
		//wprintf(L"ERRORE!!! ParseNextObject\n");
		fwprintf(pParams->fpErrors, L"ERRORE!!! ParseNextObject\n");
		retValue = 0;
		goto uscita;
	}
	
	pParams->myObjsTable[pParams->nCurrentObjNum]->numObjParent = pParams->nCurrentPageParent;	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)	
	wprintf(L"ROOT PAGES PARSING -> pParams->myObjsTable[%d]->numObjParent = %d\n", pParams->nCurrentObjNum, pParams->myObjsTable[pParams->nCurrentObjNum]->numObjParent);
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
			wprintf(L"ERRORE ParseObject ROOT -> ParseDictionaryObject\n");
			fwprintf(pParams->fpErrors, L"ERRORE ParseObject ROOT -> ParseDictionaryObject\n");
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
			
	//if ( NULL == pParams->fpOutput )
	//	wprintf(L"\n");	
	
	myobjreflist_Free(&(pParams->myXObjRefList));
	myobjreflist_Free(&(pParams->myFontsRefList));	
					
	if ( pParams->nCountPagesFromPdf > 0 )
	{	
		pParams->pPagesArray = (Page*)malloc(sizeof(Page) * (pParams->nCountPagesFromPdf + 1));
		if ( NULL == pParams->pPagesArray )
		{
			wprintf(L"ERRORE!!! ParseNextObject. Impossibile allocare la memoria per l'array delle pagine per %d pagine\n", pParams->nCountPagesFromPdf);
			retValue = 0;
			goto uscita;			
		}
		
		pParams->pPagesArray[0].numObjNumber = 0;
		pParams->pPagesArray[0].numObjContent = 0;
		
		myintqueuelist_Init(&(pParams->pPagesArray[0].queueContentsObjRefs));
		mycontentqueuelist_Init(&(pParams->pPagesArray[0].queueContens));
		
		//scopeInit(&(pParams->pPagesArray[0].myScopeHT_XObjRef));
		//scopeInit(&(pParams->pPagesArray[0].myScopeHT_FontsRef));
		
		for ( nInt = 1; nInt <= pParams->nCountPagesFromPdf; nInt++ )
		{
			pParams->pPagesArray[nInt].numObjNumber = 0;
			pParams->pPagesArray[nInt].numObjContent = 0;
			myintqueuelist_Init(&(pParams->pPagesArray[nInt].queueContentsObjRefs));
			mycontentqueuelist_Init(&(pParams->pPagesArray[nInt].queueContens));
			scopeInit(&(pParams->pPagesArray[nInt].myScopeHT_XObjRef));
			scopeInit(&(pParams->pPagesArray[nInt].myScopeHT_FontsRef));
		}
		
		//pParams->nCountPagesFromPdf = 0;
		while ( myintqueuelist_Dequeue(&(pParams->myPagesQueue), &nInt) )
		{			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)			
			wprintf(L"\n\nDEQUEUE Ecco ---> '%d'\n", nInt);
			#endif
	
			myobjreflist_Free(&(pParams->myXObjRefList));
			myobjreflist_Free(&(pParams->myFontsRefList));
						
			pParams->eCurrentObjType = OBJ_TYPE_GENERIC;
			if ( !ParseNextObject(pParams, nInt) )
			{
				//wprintf(L"ERRORE!!! ParseNextObject 2\n");
				fwprintf(pParams->fpErrors, L"ERRORE!!! ParseNextObject 2\n");
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
				pParams->myObjsTable[pParams->nCurrentObjNum]->numObjParent = pParams->nCurrentPageParent;
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
						
						x = pParams->myObjsTable[x]->numObjParent;
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
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
					//wprintf(L"ERRORE ParseObject -> ParseDictionaryObject\n");
					fwprintf(pParams->fpErrors, L"ERRORE ParseObject -> ParseDictionaryObject\n");
					#endif
				}		
		
				if ( OBJ_TYPE_PAGE == pParams->eCurrentObjType )
				{
					//pParams->nCountPagesFromPdf++;
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
					//pParams->nCountPagesFromPdf++;
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
		
		
		//PrintThisObject(pParams, 2672, 0, nInt, NULL);
		//PrintThisObject(pParams, 2656, 0, nInt, NULL);	
		//PrintThisObject(pParams, 9, 0, 0, NULL);	
		
		
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
				//wprintf(L"ERRORE ParseStreamObject streamobj number %d\n", pParams->pPagesArray[nInt].numObjContent);
				fwprintf(pParams->fpErrors, L"ERRORE ParseStreamObject streamobj number %d\n", pParams->pPagesArray[nInt].numObjContent);
				
				//wprintf(L"ECCO L'OGGETTO X:\n");
				fwprintf(pParams->fpErrors, L"ECCO L'OGGETTO X:\n");
				
				PrintThisObject(pParams, pParams->pPagesArray[nInt].numObjContent, 0, 0, NULL);
				
				//wprintf(L"FINE OGGETTO X:\n");
				fwprintf(pParams->fpErrors, L"FINE OGGETTO X:\n");
				
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
					//wprintf(L"ERRORE ParseStreamObject streamobj number %d\n", numObjQueueContent);
					fwprintf(pParams->fpErrors, L"ERRORE ParseStreamObject streamobj number %d\n", numObjQueueContent);
					
					//wprintf(L"ECCO L'OGGETTO Y:\n");
					fwprintf(pParams->fpErrors, L"ECCO L'OGGETTO Y:\n");
					
					PrintThisObject(pParams, numObjQueueContent, 0, 0, NULL);
					
					//wprintf(L"FINE OGGETTO Y:\n");
					fwprintf(pParams->fpErrors, L"FINE OGGETTO Y:\n");
									
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
	
		/*
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_TST)
			wprintf(L"\nTERNARY SEARCH TREE (Page %d) -> TRAVERSE START\n", nInt);
			count = tstTraverseRecursive(pParams->myTST.pRoot, OnTraverseTST, 0);
			//count = tstTraverseDescRecursive(pParams->myTST.pRoot, OnTraverseTST, 0);
			wprintf(L"\nTERNARY SEARCH TREE (Page %d) -> TRAVERSE END\n", nInt);
			wprintf(L"TERNARY SEARCH TREE (Page %d) -> TRAVERSE COUNT = %u\n", nInt, count);
		#endif
		*/
		
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
			/*
			else
			{
				if ( pParams->szOutputFile[0] != '\0' )
				{
					fwprintf(pParams->fpOutput, L"\tKey '%ls' NOT FOUND on page %d\n", pParams->pWordsToSearchArray[idxWord], nInt);
				}
				else
				{
					wprintf(L"\tKey '%ls' NOT FOUND on page %d\n", pParams->pWordsToSearchArray[idxWord], nInt);
				}			
			}
			*/
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
		//wprintf(L"'%ls' <-> '%s' dataSize = %d\n", (wchar_t*)key, (char*)data, dataSize);
		//pWideCharString = (wchar_t*)AsciiToUtf8WideCharString((char*)data, dataSize);
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
	
	//unsigned char *pszEncodedStream = NULL;
	//unsigned long EncodesStreamSize 
	//unsigned char *pszDecodedStream = NULL;
	
	MyContent_t myContent;
	int nTemp;
	
	unsigned char szTemp[21];
		
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		if ( NULL == fpOutput )
			wprintf(L"Errore PrintThisObject: objNum non valido -> %d\n", objNum);
		else
			fwprintf(fpOutput, L"Errore PrintThisObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Offset - 1, SEEK_SET) )
	{
		if ( NULL == fpOutput )
			wprintf(L"Errore PrintThisObject fseek\n");
		else
			fwprintf(fpOutput, L"Errore PrintThisObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
	if ( pParams->blockLen < 3 )
	{
		if ( NULL == fpOutput )
			wprintf(L"Errore PrintThisObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		else
			fwprintf(fpOutput, L"Errore PrintThisObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		retValue = 0;
		goto uscita;
	}
		
	pParams->blockCurPos = 0;
	
	c = pParams->myBlock[pParams->blockCurPos++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		if ( NULL == fpOutput )
			wprintf(L"Errore PrintThisObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		else
			fwprintf(fpOutput, L"Errore PrintThisObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		retValue = 0;
		goto uscita;		
	}
	
	if ( NULL == fpOutput )
		wprintf(L"\n\n***** INIZIO OBJ(%d) = \n", objNum);
	else
		fwprintf(fpOutput, L"\n\n***** INIZIO OBJ(%d) = \n", objNum);
	
	k = 1;
	szTemp[0] = '\0';
	j = 0;
	while ( pParams->blockLen > 0 )
	{
		c = pParams->myBlock[k];
				
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
		else if ( 'e' == c )
		{
			j  = 0;
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
	//wprintf(L"\n\n--->%s<---\n\n", (char*)pParams->myBlock);
	
	
	if ( bDecodeStream )
	{
		pParams->nCurrentPageNum = nPageNumber;
		nTemp = 1;
		if ( !ParseStreamObject(pParams, objNum) )
		{
			if ( NULL == fpOutput )
				wprintf(L"Errore PrintThisObject ParseStreamObject: oggetto n. %d;\n", objNum);
			else
				fwprintf(fpOutput, L"Errore PrintThisObject ParseStreamObject: oggetto n. %d;\n", objNum);
			retValue = 0;
			goto uscita;			
		}
		
		//wprintf(L"\nSTAMPO LO STREAM *********************:\n");		
		myContent.bExternalFile = 0;
		myContent.LengthFromPdf = 0;
		myContent.Offset = 0;
			
		mystringqueuelist_Init(&(myContent.queueFilters));
		mydictionaryqueuelist_Init(&(myContent.decodeParms), 1, 1);
		
		//while ( mycontentqueuelist_Dequeue(&(pParams->pPagesArray[nPageNumber].queueContens), &myContent) )
		if ( mycontentqueuelist_Dequeue(&(pParams->pPagesArray[nPageNumber].queueContens), &myContent) )
		{			
			ManageContent(pParams, nPageNumber);
			
			mydictionaryqueuelist_Free(&(myContent.decodeParms));
			mystringqueuelist_Free(&(myContent.queueFilters));
			nTemp++;
		}
		else
		{
			if ( NULL == fpOutput )
				wprintf(L"PrintThisObject coda degli stream per la pagina n° %d vuota!\n", nPageNumber);
			else
				fwprintf(fpOutput, L"PrintThisObject coda degli stream per la pagina n° %d vuota!\n", nPageNumber);
		}
		//wprintf(L"FINE STREAM:     *********************.\n");
	}
			
	if ( NULL == fpOutput )
		wprintf(L"\n*****FINE OBJ(%d)\n\n", objNum);
	else
		fwprintf(pParams->fpErrors, L"\n*****FINE OBJ(%d)\n\n", objNum);
	
uscita:
	
	return retValue;
}

int ParseNextObject(Params *pParams, int objNum)
{
	int retValue = 1;
	unsigned char c;
	
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		//wprintf(L"Errore ParseNextObject: objNum non valido -> %d\n", objNum);
		fwprintf(pParams->fpErrors, L"Errore ParseNextObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Offset - 1, SEEK_SET) )
	{
		wprintf(L"Errore ParseNextObject fseek\n");
		fwprintf(pParams->fpErrors, L"Errore ParseNextObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
	if ( pParams->blockLen < 3 )
	{
		//wprintf(L"Errore ParseNextObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		fwprintf(pParams->fpErrors, L"Errore ParseNextObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		retValue = 0;
		goto uscita;
	}
		
	pParams->blockCurPos = 0;
	
	c = pParams->myBlock[pParams->blockCurPos++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		//wprintf(L"Errore ParseNextObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		fwprintf(pParams->fpErrors, L"Errore ParseNextObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
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
	
	// --------------------------------------------------------------------------------------------------------------------------------------
	
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Offset, SEEK_SET) )
	{
		wprintf(L"Errore ParseNextObject fseek 2\n");
		fwprintf(pParams->fpErrors, L"Errore ParseNextObject fseek 2\n");
		retValue = 0;
		goto uscita;
	}	
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
		
	pParams->blockCurPos = 0;
		
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

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	return retValue;
}

int ParseStreamObject(Params *pParams, int objNum)
{	
	int retValue = 1;
	unsigned char c;
		
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		//wprintf(L"Errore ParseStreamObject: objNum non valido -> %d\n", objNum);
		fwprintf(pParams->fpErrors, L"Errore ParseStreamObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Offset - 1, SEEK_SET) )
	{
		wprintf(L"Errore ParseStreamObject fseek\n");
		fwprintf(pParams->fpErrors, L"Errore ParseStreamObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
	if ( pParams->blockLen < 3 )
	{
		//wprintf(L"Errore ParseStreamObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		fwprintf(pParams->fpErrors, L"Errore ParseStreamObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		retValue = 0;
		goto uscita;
	}
		
	pParams->blockCurPos = 0;
	
	c = pParams->myBlock[pParams->blockCurPos++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		//wprintf(L"Errore ParseStreamObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		fwprintf(pParams->fpErrors, L"Errore ParseStreamObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
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
		//wprintf(L"Errore ParseStreamXObject: objNum non valido -> %d\n", objNum);
		fwprintf(pParams->fpErrors, L"Errore ParseStreamXObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Offset - 1, SEEK_SET) )
	{
		wprintf(L"Errore ParseStreamXObject fseek\n");
		fwprintf(pParams->fpErrors, L"Errore ParseStreamXObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
	if ( pParams->blockLen < 3 )
	{
		//wprintf(L"Errore ParseStreamXObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		fwprintf(pParams->fpErrors, L"Errore ParseStreamXObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		retValue = 0;
		goto uscita;
	}
		
	pParams->blockCurPos = 0;
	
	c = pParams->myBlock[pParams->blockCurPos++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		//wprintf(L"Errore ParseStreamXObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		fwprintf(pParams->fpErrors, L"Errore ParseStreamXObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
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
	int codeSpaceRange1 = -1;
	int codeSpaceRange2 = -1;
	
	int bCidRangeState;
	int cidRange1 = -1;
	int cidRange2 = -1;
	int cidRange3 = -1;
	
	int bCidCharState;
	int cidChar1 = -1;
	int cidChar2 = -1;	
	
	int bNotdefRangeState;
	int notdefRange1 = -1;
	int notdefRange2 = -1;
	int notdefRange3 = -1;
	
	int bNotdefCharState;
	int notdefChar1 = -1;
	int notdefChar2 = -1;	
	
	int lastInteger = 0;
	int idxCodeSpace = 0;
	
	// ----------------------------------------------------------------------------------------------------
	int nRes;
	size_t tnameLen;
	uint32_t nDataSize;
	uint32_t bContentAlreadyProcessed;	
		
	uint32_t codepoint;
	uint16_t lead;
	uint16_t trail;
				
	int bBfCharState;
	int bfChar1 = -1;
	int bfChar2 = -1;
	
	int bBfRangeState;
	int bfRange1 = -1;
	int bfRange2 = -1;
	int bfRange3 = -1;	
	// ----------------------------------------------------------------------------------------------------
			
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
	UNUSED(objNum);
	#endif
	
	pParams->bStringIsDecoded = 1;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
	wprintf(L"\n\nParseCMapStream(OBJ %d) -> INIZIO STREAM DECODIFICATO DOPO myInflate:\n", objNum);
	for ( unsigned long int k = 0; k < DecodedStreamSize; k++ )
	{
		if ( pszDecodedStream[k] == '\0' )
			wprintf(L"\\0");
		else
			wprintf(L"%c", pszDecodedStream[k]);
	}
	wprintf(L"\nParseCMapStream(OBJ %d) -> FINE STREAM DECODIFICATO DOPO myInflate:\n\n", objNum);	
	#endif
		
	pParams->myBlockToUnicode = pszDecodedStream;
	pParams->blockLenToUnicode = DecodedStreamSize;
	pParams->blockCurPosToUnicode = 0;	
	
	//if ( !(pParams->bEncodigArrayAlreadyInit) )
	//{
	//	for ( i = 0; i < pParams->dimCustomizedFont_CharSet; i++ )
	//	{
	//		pParams->paCustomizedFont_CharSet[i] = pParams->pArrayUnicode[i];
	//	}
	//}
	
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
					fwprintf(pParams->fpErrors, L"ParseCMapStream: impossibile allocare la memoria per pParams->pCodeSpaceRangeArray\n");
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
					wprintf(L"ParseCMapStream: impossibile allocare la memoria per pParams->pCodeSpaceRangeArray\n");
					#endif
					retValue = 0;
					goto uscita;
				}
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
				else
				{
					wprintf(L"\tParseCMapStream: ALLOCATI CORRETTAMENTE %lu BYTE per pParams->pCodeSpaceRangeArray. lastInteger = %d\n", sizeof(CodeSpaceRange_t) * lastInteger, lastInteger);
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
				codeSpaceRange1 = -1;
				codeSpaceRange2 = -1;
				break;				
			case T_CONTENT_OP_endcodespacerange:
				bCodeSpaceRangeState = 0;
				codeSpaceRange1 = -1;
				codeSpaceRange2 = -1;
				break;
			case T_CONTENT_OP_begincidrange:
				bCidRangeState = 1;
				cidRange1 = -1;
				cidRange2 = -1;
				cidRange3 = -1;
				break;
			case T_CONTENT_OP_endcidrange:
				bCidRangeState = 0;
				cidRange1 = -1;
				cidRange2 = -1;
				cidRange3 = -1;
				break;
			case T_CONTENT_OP_begincidchar:
				bCidCharState = 1;
				cidChar1 = -1;
				cidChar2 = -1;
				break;
			case T_CONTENT_OP_endcidchar:
				bCidCharState = 0;
				cidChar1 = -1;
				cidChar2 = -1;
				break;				
			case T_CONTENT_OP_beginnotdefrange:
				bNotdefRangeState = 1;
				notdefRange1 = -1;
				notdefRange2 = -1;
				notdefRange3 = -1;
				break;
			case T_CONTENT_OP_endnotdefrange:
				bNotdefRangeState = 0;
				notdefRange1 = -1;
				notdefRange2 = -1;
				notdefRange3 = -1;
				break;	
			case T_CONTENT_OP_beginnotdefchar:
				bNotdefCharState = 1;
				notdefChar1 = -1;
				notdefChar2 = -1;
				break;
			case T_CONTENT_OP_endnotdefchar:
				bNotdefCharState = 0;
				notdefChar1 = -1;
				notdefChar2 = -1;
				break;
			case T_CONTENT_OP_beginbfchar:
				bBfCharState = 1;
				bfChar1 = -1;
				bfChar2 = -1;
				codepoint = lead = trail = 0;
				break;
			case T_CONTENT_OP_endbfchar:
				bBfCharState = 0;
				bfChar1 = -1;
				bfChar2 = -1;				
				codepoint = lead = trail = 0;
				break;
			case T_CONTENT_OP_beginbfrange:
				bBfRangeState = 1;
				bfRange1 = -1;
				bfRange2 = -1;
				bfRange3 = -1;
				codepoint = lead = trail = 0;
				break;
			case T_CONTENT_OP_endbfrange:
				bBfRangeState = 0;
				bfRange1 = -1;
				bfRange2 = -1;
				bfRange3 = -1;
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
				
				if ( bBfRangeState && bfRange2 >= 0 && bfRange3 < 0 )
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
					if ( codeSpaceRange1 < 0 )
					{
						codeSpaceRange1 = myValue;
					}
					else
					{
						codeSpaceRange2 = myValue;
						
						//if ( codeSpaceRange2 > 0xFF )
						if ( codeSpaceRange2 > 0xFFFF )
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ParseCMapStream: VALORI ARRAY UNICODE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> codeSpaceRange1 = %d, codeSpaceRange2 = %d\n", codeSpaceRange1, codeSpaceRange2);
							#endif
							retValue = 0;
							goto uscita;
						}
						else if ( codeSpaceRange2 <= 0xFF )
						{
							pParams->bHasCodeSpaceOneByte = 1;
						}
						else if ( codeSpaceRange2 <= 0xFFFF )
						{
							pParams->bHasCodeSpaceTwoByte = 1;
						}						
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
						else
						{
							wprintf(L"ParseCMapStream: OK!!! ARRAY UNICODE CODE SPACE RANGE -> codeSpaceRange1 = %d, codeSpaceRange2 = %d\n", codeSpaceRange1, codeSpaceRange2);	
						}	
						#endif	
						
						pParams->pCodeSpaceRangeArray[idxCodeSpace].nFrom = codeSpaceRange1;
						pParams->pCodeSpaceRangeArray[idxCodeSpace].nTo = codeSpaceRange2;
						idxCodeSpace++;

						codeSpaceRange1 = -1;
						codeSpaceRange2 = -1;
					}
				}				
				else if ( bCidRangeState )
				{
					if ( cidRange1 < 0 )
					{
						cidRange1 = myValue;
					}
					else if ( cidRange2 < 0 )
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
							wprintf(L"ERRORE ParseCMapStream: atteso T_INT_LITERAL\n");
							#endif
							fwprintf(pParams->fpErrors, L"ERRORE ParseCMapStream: atteso T_INT_LITERAL\n");
							retValue = 0;
							goto uscita;							
						}

						if ( cidRange1 <= 0xFFFF && cidRange2 <= 0xFFFF )
						{
							for ( i = cidRange1; i <= cidRange2; i++ )
							{
								pParams->paCustomizedFont_CharSet[i] = cidRange3;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
								wprintf(L"ParseCMapStream: OK!!! ARRAY UNICODE -> pParams->paCustomizedFont_CharSet[%d] = %d\n", i, cidRange3);
								#endif								
								
								cidRange3++;
							}														
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ParseCMapStream: VALORI CIDRANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> cidRange1 = %d, cidRange2 = %d, cidRange3 = %d\n", cidRange1, cidRange2, cidRange3);
							#endif
						}
						
						cidRange1 = -1;
						cidRange2 = -1;
						cidRange3 = -1;
					}
				}
				else if ( bCidCharState )
				{
					if ( cidChar1 < 0 )
					{
						cidChar1 = myValue;
						
						GetNextTokenFromToUnicodeStream(pParams);
						
						if ( T_INT_LITERAL == pParams->myToken.Type )
						{
							cidChar2 = pParams->myToken.Value.vInt;
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ERRORE ParseCMapStream: atteso T_INT_LITERAL\n");
							#endif
							fwprintf(pParams->fpErrors, L"ERRORE ParseCMapStream: atteso T_INT_LITERAL\n");
							retValue = 0;
							goto uscita;							
						}
												
						
						if ( cidChar1 <= 0xFFFF )
						{
							pParams->paCustomizedFont_CharSet[cidChar1] = cidChar2;
							
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ParseCMapStream: OK!!! ARRAY UNICODE -> pParams->paCustomizedFont_CharSet[%d] = %d\n", cidChar1, cidChar2);
							#endif							
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ParseCMapStream: VALORI CIDCHAR > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> cidChar1 = %d, cidChar2 = %d\n", cidChar1, cidChar2);
							#endif							
						}						
						
						cidChar1 = -1;
						cidChar2 = -1;
					}
				}				
				else if ( bNotdefRangeState )
				{
					if ( notdefRange1 < 0 )
					{
						notdefRange1 = myValue;
					}
					else if ( notdefRange2 < 0 )
					{
						notdefRange2 = myValue;
						
						GetNextTokenFromToUnicodeStream(pParams);
						
						if ( T_INT_LITERAL == pParams->myToken.Type )
						{
							notdefRange3 = pParams->myToken.Value.vInt;
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ERRORE ParseCMapStream: atteso T_INT_LITERAL\n");
							#endif
							fwprintf(pParams->fpErrors, L"ERRORE ParseCMapStream: atteso T_INT_LITERAL\n");
							retValue = 0;
							goto uscita;							
						}

						if ( notdefRange1 <= 0xFFFF && notdefRange2 <= 0xFFFF )
						{
							for ( i = notdefRange1; i <= notdefRange2; i++ )
							{
								pParams->paCustomizedFont_CharSet[i] = notdefRange3;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
								wprintf(L"ParseCMapStream: OK!!! NOTDEFRANGE -> pParams->paCustomizedFont_CharSet[%d] = %d\n", i, notdefRange3);
								#endif								
								
								notdefRange3++;
							}
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ParseCMapStream: VALORI NOTDEFRANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> notdefRange1 = %d, notdefRange2 = %d, notdefRange3 = %d\n", notdefRange1, notdefRange2, notdefRange3);
							#endif
						}
						
						notdefRange1 = -1;
						notdefRange2 = -1;
						notdefRange3 = -1;
					}
				}
				else if ( bNotdefCharState )
				{
					if ( notdefChar1 < 0 )
					{
						notdefChar1 = myValue;
						
						GetNextTokenFromToUnicodeStream(pParams);
						
						if ( T_INT_LITERAL == pParams->myToken.Type )
						{
							notdefChar2 = pParams->myToken.Value.vInt;
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ERRORE ParseCMapStream: atteso T_INT_LITERAL\n");
							#endif
							fwprintf(pParams->fpErrors, L"ERRORE ParseCMapStream: atteso T_INT_LITERAL\n");
							retValue = 0;
							goto uscita;							
						}
												
						
						if ( notdefChar1 <= 0xFFFF )
						{
							pParams->paCustomizedFont_CharSet[cidChar1] = notdefChar2;
							
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ParseCMapStream: OK!!! NOTDEFCHAR -> pParams->paCustomizedFont_CharSet[%d] = %d\n", notdefChar1, notdefChar2);
							#endif							
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ParseCMapStream: VALORI NOTDEFCHAR > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> notdefChar1 = %d, notdefChar2 = %d\n", notdefChar1, notdefChar2);
							#endif							
						}						
						
						notdefChar1 = -1;
						notdefChar2 = -1;
					}
				}
				else if ( bBfCharState )
				{
					if ( bfChar1 < 0 )
					{
						bfChar1 = myValue;
						
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
							
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
								wprintf(L"ParseCMapStream: OK!!! BFCHAR -> pParams->paCustomizedFont_CharSet[%d] = %d\n", bfChar1, bfChar2);
								#endif
							}
							else
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
								wprintf(L"ParseCMapStream: VALORI BFCHAR > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> bfChar1 = %d, bfChar2 = %d\n", bfChar1, bfChar2);
								#endif
							}
						
							bfChar1 = -1;
							bfChar2 = -1;
						}
						else if ( T_NAME == pParams->myToken.Type )
						{
							tnameLen = strnlen(pParams->myToken.Value.vString, 4096);
							nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.Value.vString, tnameLen + sizeof(char), (void*)&bfChar2, &nDataSize, &bContentAlreadyProcessed);
							if ( nRes >= 0 ) // TROVATO
							{				
								pParams->paCustomizedFont_CharSet[bfChar1] = bfChar2;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
								wprintf(L"ParseCMapStream: OK!!! BFCHAR -> pParams->paCustomizedFont_CharSet[%d] = %d -> (%s)\n", bfChar1, bfChar2, pParams->myToken.Value.vString);
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
							fwprintf(pParams->fpErrors, L"ERRORE ParseCMapStream: atteso T_STRING_HEXADECIMAL o T_NAME\n");
							retValue = 0;
							goto uscita;
						}						
					}
				}
				else if ( bBfRangeState )
				{
					if ( bfRange1 < 0 )
					{
						bfRange1 = myValue;
					}
					else if ( bfRange2 < 0 )
					{
						bfRange2 = myValue;
					}					
					else
					{
						bfRange3 = myValue;
						
						if ( bfRange1 <= 0xFFFF && bfRange2 <= 0xFFFF )
						{
							for ( i = bfRange1; i <= bfRange2; i++ )
							{
								pParams->paCustomizedFont_CharSet[i] = bfRange3;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
								wprintf(L"ParseCMapStream: OK!!! BFRANGE -> pParams->paCustomizedFont_CharSet[%d] = %d\n", i, bfRange3);
								#endif								
								
								bfRange3++;
							}
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
							wprintf(L"ParseCMapStream: VALORI BFRANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> bfRange1 = %d, bfRange2 = %d, bfRange3 = %d\n", bfRange1, bfRange2, bfRange3);
							#endif
						}
						
						bfRange1 = -1;
						bfRange2 = -1;
						bfRange3 = -1;
					}
				}
				
				break;				
			case T_QOPAREN:
				if ( bCidRangeState )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
					wprintf(L"ParseCMapStream: VALORI CIDFRANGE ALL'INTERNO DI UN ARRAY PER IL MOMENTO NON SUPPORTATI.\n");
					#endif
					
					cidRange1 = -1;
					cidRange2 = -1;
					cidRange3 = -1;
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
	int codeSpaceRange1 = -1;
	int codeSpaceRange2 = -1;
			
	int bBfCharState;
	int bfChar1 = -1;
	int bfChar2 = -1;
	
	int bBfRangeState;
	int bfRange1 = -1;
	int bfRange2 = -1;
	int bfRange3 = -1;
	
	int lastInteger = 0;
	int idxCodeSpace = 0;
				
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
	UNUSED(objNum);
	#endif
	
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
	wprintf(L"\nParseToUnicodeStream(OBJ %d) -> FINE STREAM DECODIFICATO DOPO myInflate:\n\n", objNum);	
	#endif
		
	pParams->myBlockToUnicode = pszDecodedStream;
	pParams->blockLenToUnicode = DecodedStreamSize;
	pParams->blockCurPosToUnicode = 0;	
	
	//if ( !(pParams->bEncodigArrayAlreadyInit) )
	//{
	//	for ( i = 0; i < pParams->dimCustomizedFont_CharSet; i++ )
	//	{
	//		pParams->paCustomizedFont_CharSet[i] = pParams->pArrayUnicode[i];
	//	}
	//}
	
	bCodeSpaceRangeState = 0;
	bBfCharState = 0;
	bBfRangeState = 0;
	
	codepoint = lead = trail = 0;
	
	
	/*
	typedef struct tagCodeSpaceRange
	{
		uint32_t nFrom;
		uint32_t nTo;
	} CodeSpaceRange_t;
	
	uint32_t nCurrentFontCodeSpacesNum;
	CodeSpaceRange_t *pCodeSpaceRangeArray;
	*/
		
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
					fwprintf(pParams->fpErrors, L"ParseToUnicodeStream: impossibile allocare la memoria per pParams->pCodeSpaceRangeArray\n");
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
					wprintf(L"ParseToUnicodeStream: impossibile allocare la memoria per pParams->pCodeSpaceRangeArray\n");
					#endif
					retValue = 0;
					goto uscita;
				}
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
				else
				{
					wprintf(L"\tParseToUnicodeStream: ALLOCATI CORRETTAMENTE %lu BYTE per pParams->pCodeSpaceRangeArray. lastInteger = %d\n", sizeof(CodeSpaceRange_t) * lastInteger, lastInteger);
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
				codeSpaceRange1 = -1;
				codeSpaceRange2 = -1;
				break;
			case T_CONTENT_OP_endcodespacerange:
				bCodeSpaceRangeState = 0;
				codeSpaceRange1 = -1;
				codeSpaceRange2 = -1;
				break;
			case T_CONTENT_OP_beginbfchar:
				bBfCharState = 1;
				bfChar1 = -1;
				bfChar2 = -1;
				codepoint = lead = trail = 0;
				break;
			case T_CONTENT_OP_endbfchar:
				bBfCharState = 0;
				bfChar1 = -1;
				bfChar2 = -1;				
				codepoint = lead = trail = 0;
				break;
			case T_CONTENT_OP_beginbfrange:
				bBfRangeState = 1;
				bfRange1 = -1;
				bfRange2 = -1;
				bfRange3 = -1;
				codepoint = lead = trail = 0;
				break;
			case T_CONTENT_OP_endbfrange:
				bBfRangeState = 0;
				bfRange1 = -1;
				bfRange2 = -1;
				bfRange3 = -1;
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
				
				if ( bBfRangeState && bfRange2 >= 0 && bfRange3 < 0 )
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
					if ( codeSpaceRange1 < 0 )
					{
						codeSpaceRange1 = myValue;
					}
					else
					{
						codeSpaceRange2 = myValue;
						
						//if ( codeSpaceRange2 > 0xFF )
						if ( codeSpaceRange2 > 0xFFFF )
						{
							fwprintf(pParams->fpErrors, L"ParseToUnicodeStream: VALORI CODESPACERANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> codeSpaceRange1 = %d, codeSpaceRange2 = %d\n", codeSpaceRange1, codeSpaceRange2);
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
							wprintf(L"ParseToUnicodeStream: VALORI CODESPACERANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> codeSpaceRange1 = %d, codeSpaceRange2 = %d\n", codeSpaceRange1, codeSpaceRange2);
							#endif
							pParams->bHasCodeSpaceOneByte = pParams->bHasCodeSpaceTwoByte = 0;
							retValue = 0;
							goto uscita;
						}
						else if ( codeSpaceRange2 <= 0xFF )
						{
							pParams->bHasCodeSpaceOneByte = 1;
						}
						else if ( codeSpaceRange2 <= 0xFFFF )
						{
							pParams->bHasCodeSpaceTwoByte = 1;
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
						else
						{
							wprintf(L"ParseToUnicodeStream: OK!!! CODESPACERANGE -> codeSpaceRange1 = %d, codeSpaceRange2 = %d\n", codeSpaceRange1, codeSpaceRange2);	
						}	
						#endif			
						
						pParams->pCodeSpaceRangeArray[idxCodeSpace].nFrom = codeSpaceRange1;
						pParams->pCodeSpaceRangeArray[idxCodeSpace].nTo = codeSpaceRange2;
						idxCodeSpace++;		

						codeSpaceRange1 = -1;
						codeSpaceRange2 = -1;
					}
				}
				else if ( bBfCharState )
				{
					if ( bfChar1 < 0 )
					{
						bfChar1 = myValue;
						
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
								wprintf(L"ParseToUnicodeStream: OK!!! BFCHAR -> pParams->paCustomizedFont_CharSet[%d] = %d\n", bfChar1, bfChar2);
								#endif
							}
							else
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
								wprintf(L"ParseToUnicodeStream: VALORI BFCHAR > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> bfChar1 = %d, bfChar2 = %d\n", bfChar1, bfChar2);
								#endif
							}
						
							bfChar1 = -1;
							bfChar2 = -1;
						}
						else if ( T_NAME == pParams->myToken.Type )
						{
							tnameLen = strnlen(pParams->myToken.Value.vString, 4096);
							nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.Value.vString, tnameLen + sizeof(char), (void*)&bfChar2, &nDataSize, &bContentAlreadyProcessed);
							if ( nRes >= 0 ) // TROVATO
							{				
								pParams->paCustomizedFont_CharSet[bfChar1] = bfChar2;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
								wprintf(L"ParseToUnicodeStream: OK!!! BFCHAR -> pParams->paCustomizedFont_CharSet[%d] = %d -> (%s)\n", bfChar1, bfChar2, pParams->myToken.Value.vString);
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
							fwprintf(pParams->fpErrors, L"ERRORE ParseToUnicodeStream: atteso T_STRING_HEXADECIMAL o T_NAME\n");
							retValue = 0;
							goto uscita;
						}						
					}
				}
				else if ( bBfRangeState )
				{
					if ( bfRange1 < 0 )
					{
						bfRange1 = myValue;
					}
					else if ( bfRange2 < 0 )
					{
						bfRange2 = myValue;
					}					
					else
					{
						bfRange3 = myValue;
						
						if ( bfRange1 <= 0xFFFF && bfRange2 <= 0xFFFF )
						{
							for ( i = bfRange1; i <= bfRange2; i++ )
							{
								pParams->paCustomizedFont_CharSet[i] = bfRange3;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
								wprintf(L"ParseToUnicodeStream: OK!!! BFRANGE -> pParams->paCustomizedFont_CharSet[%d] = %d\n", i, bfRange3);
								#endif								
								
								bfRange3++;
							}
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
							wprintf(L"ParseToUnicodeStream: VALORI BFRANGE > 0xFFFF PER IL MOMENTO NON SUPPORTATI -> bfRange1 = %d, bfRange2 = %d, bfRange3 = %d\n", bfRange1, bfRange2, bfRange3);
							#endif
						}
						
						bfRange1 = -1;
						bfRange2 = -1;
						bfRange3 = -1;
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
	unsigned char c;
	
	unsigned long int DecodedStreamSize = 0;
	
	unsigned char *pszEncodedStream = NULL;
	unsigned char *pszDecodedStream = NULL;
	
	size_t bytesRead = 0;
	
	int ret = 0;
		
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		//wprintf(L"Errore ParseCMapObject: objNum non valido -> %d\n", objNum);
		fwprintf(pParams->fpErrors, L"Errore ParseCMapObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Offset - 1, SEEK_SET) )
	{
		wprintf(L"Errore ParseCMapObject fseek\n");
		fwprintf(pParams->fpErrors, L"Errore ParseCMapObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
	if ( pParams->blockLen < 3 )
	{
		//wprintf(L"Errore ParseCMapObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		fwprintf(pParams->fpErrors, L"Errore ParseCMapObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		retValue = 0;
		goto uscita;
	}
		
	pParams->blockCurPos = 0;
	
	c = pParams->myBlock[pParams->blockCurPos++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		//wprintf(L"Errore ParseCMapObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		fwprintf(pParams->fpErrors, L"Errore ParseCMapObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
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
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
	wprintf(L"\n***** DECODED STREAM TO UNICODE:\n");
	#endif
		
	// ************************************************************************************************************************************************************
	
	//wprintf(L"\npParams->CurrentContent.queueFilters.count = %d\n", pParams->CurrentContent.queueFilters.count);
	//wprintf(L"\tpParams->CurrentContent.decodeParms.count = %d\n", pParams->CurrentContent.decodeParms.count);
		
	if ( pParams->CurrentContent.queueFilters.count <= 0 )
	{
		pszEncodedStream = NULL;
		
		DecodedStreamSize = pParams->CurrentContent.LengthFromPdf * sizeof(unsigned char) + sizeof(unsigned char);
					
		pszDecodedStream = (unsigned char *)malloc( DecodedStreamSize );
		if ( NULL == pszDecodedStream )
		{
			wprintf(L"ERRORE ParseCMapObject: impossibile allocare la memoria per pszDecodedStream.\n");
			fwprintf(pParams->fpErrors, L"ERRORE ParseCMapObject: impossibile allocare la memoria per pszDecodedStream.\n");
			pParams->nStreamsStackTop--;
			retValue = 0;
			goto uscita;		
		}
		//wprintf(L"\tALLOCATI %lu BYTE PER pszDecodedStream\n", DecodedStreamSize);
		
		fseek(pParams->fp, pParams->CurrentContent.Offset, SEEK_SET);
						
		bytesRead = fread(pszDecodedStream, 1, pParams->CurrentContent.LengthFromPdf, pParams->fp);
		if ( bytesRead < pParams->CurrentContent.LengthFromPdf )
		{
			wprintf(L"\nERRORE ParseCMapObject: fread ha letto %lu byte quando il pdf ne specifica, invece, %lu\n", bytesRead, pParams->CurrentContent.LengthFromPdf);
			fwprintf(pParams->fpErrors, L"\nERRORE ParseCMapObject: fread ha letto %lu byte quando il pdf ne specifica, invece, %lu\n", bytesRead, pParams->CurrentContent.LengthFromPdf);
			retValue = 0;
			goto uscita;
		}											
		pszDecodedStream[pParams->CurrentContent.LengthFromPdf] = '\0';
		//wprintf(L"\tLETTI %d BYTE SU pszDecodedStream\n", bytesRead);
	}
	else
	{
		pszEncodedStream = (unsigned char*)malloc( pParams->CurrentContent.LengthFromPdf * sizeof(unsigned char) + sizeof(unsigned char) );
		if ( NULL == pszEncodedStream )
		{
			wprintf(L"ERRORE ParseCMapObject: impossibile allocare %lu byte per leggere lo stream\n", pParams->CurrentContent.LengthFromPdf * sizeof(unsigned char) + sizeof(unsigned char));
			fwprintf(pParams->fpErrors, L"ERRORE ParseCMapObject: impossibile allocare %lu byte per leggere lo stream\n", pParams->CurrentContent.LengthFromPdf * sizeof(unsigned char) + sizeof(unsigned char));
			retValue = 0;
			goto uscita;
		}
		//wprintf(L"\tALLOCATI %lu BYTE PER pszEncodedStream\n", pParams->CurrentContent.LengthFromPdf * sizeof(unsigned char) + sizeof(unsigned char));
			
		DecodedStreamSize = ( pParams->CurrentContent.LengthFromPdf * sizeof(unsigned char) ) * 55 + sizeof(unsigned char);
	
		if ( DecodedStreamSize > 409600000 )
			DecodedStreamSize = 409600000;
		
		if ( DecodedStreamSize < pParams->CurrentContent.LengthFromPdf )
			DecodedStreamSize = pParams->CurrentContent.LengthFromPdf + (4096 * 89);
				
		pszDecodedStream = (unsigned char *)malloc( DecodedStreamSize );
		if ( NULL == pszDecodedStream )
		{
			wprintf(L"ERRORE ParseCMapObject: impossibile allocare la memoria per pszDecodedStream.\n");
			fwprintf(pParams->fpErrors, L"ERRORE ParseCMapObject: impossibile allocare la memoria per pszDecodedStream.\n");
			pParams->nStreamsStackTop--;
			retValue = 0;
			goto uscita;		
		}
		//wprintf(L"\tALLOCATI %lu BYTE PER pszDecodedStream\n", DecodedStreamSize);
		
		fseek(pParams->fp, pParams->CurrentContent.Offset, SEEK_SET);
						
		bytesRead = fread(pszEncodedStream, 1, pParams->CurrentContent.LengthFromPdf, pParams->fp);
		if ( bytesRead < pParams->CurrentContent.LengthFromPdf )
		{
			wprintf(L"\nERRORE ParseCMapObject: fread ha letto %lu byte quando il pdf ne specifica, invece, %lu\n", bytesRead, pParams->CurrentContent.LengthFromPdf);
			fwprintf(pParams->fpErrors, L"\nERRORE ParseCMapObject: fread ha letto %lu byte quando il pdf ne specifica, invece, %lu\n", bytesRead, pParams->CurrentContent.LengthFromPdf);
			retValue = 0;
			goto uscita;
		}											
		pszEncodedStream[pParams->CurrentContent.LengthFromPdf] = '\0';
		//wprintf(L"\tLETTI %d BYTE SU pszEncodedStream\n", bytesRead);
			
		pszDecodedStream[0] = '\0';		
		
		ret = myInflate(&(pszDecodedStream), &DecodedStreamSize, pszEncodedStream, pParams->CurrentContent.LengthFromPdf);
		if ( Z_OK != ret )
		{
			zerr(ret, pParams->fpErrors);
			retValue = 0;
			goto uscita;
		}
		pszDecodedStream[DecodedStreamSize] = '\0';		
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

	if ( NULL != pszEncodedStream )
	{
		free(pszEncodedStream);
		pszEncodedStream = NULL;
	}
	
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
	unsigned char c;
		
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		//wprintf(L"Errore ParseFontObject: objNum non valido -> %d\n", objNum);
		fwprintf(pParams->fpErrors, L"Errore ParseFontObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Offset - 1, SEEK_SET) )
	{
		wprintf(L"Errore ParseFontObject fseek\n");
		fwprintf(pParams->fpErrors, L"Errore ParseFontObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
	if ( pParams->blockLen < 3 )
	{
		//wprintf(L"Errore ParseFontObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		fwprintf(pParams->fpErrors, L"Errore ParseFontObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		retValue = 0;
		goto uscita;
	}
		
	pParams->blockCurPos = 0;
	
	c = pParams->myBlock[pParams->blockCurPos++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		//wprintf(L"Errore ParseFontObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		fwprintf(pParams->fpErrors, L"Errore ParseFontObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
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
	unsigned char c;
		
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		//wprintf(L"Errore ParseEncodingObject: objNum non valido -> %d\n", objNum);
		fwprintf(pParams->fpErrors, L"Errore ParseEncodingObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Offset - 1, SEEK_SET) )
	{
		wprintf(L"Errore ParseEncodingObject fseek\n");
		fwprintf(pParams->fpErrors, L"Errore ParseEncodingObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
	if ( pParams->blockLen < 3 )
	{
		//wprintf(L"Errore ParseEncodingObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		fwprintf(pParams->fpErrors, L"Errore ParseEncodingObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		retValue = 0;
		goto uscita;
	}
		
	pParams->blockCurPos = 0;
	
	c = pParams->myBlock[pParams->blockCurPos++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		//wprintf(L"Errore ParseEncodingObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		fwprintf(pParams->fpErrors, L"Errore ParseEncodingObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
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
	unsigned char c;
	
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		//wprintf(L"Errore ParseDictionaryObject: objNum non valido -> %d\n", objNum);
		fwprintf(pParams->fpErrors, L"Errore ParseDictionaryObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
	
	pParams->blockLen = 0;
	pParams->blockCurPos = 0;
		
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fp, pParams->myObjsTable[objNum]->Offset - 1, SEEK_SET) )
	{
		wprintf(L"Errore ParseDictionaryObject fseek\n");
		fwprintf(pParams->fpErrors, L"Errore ParseDictionaryObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
	
	if ( pParams->blockLen < 3 )
	{
		//wprintf(L"Errore ParseDictionaryObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		fwprintf(pParams->fpErrors, L"Errore ParseDictionaryObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		retValue = 0;
		goto uscita;
	}
			
	pParams->blockCurPos = 0;
	
	c = pParams->myBlock[pParams->blockCurPos++];
	if ( (c != '\n' && c != '\r' && c != ' ' && c != '\t' && c != '\f' && c != '\b' && c != '\0') )
	{
		//wprintf(L"Errore ParseDictionaryObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		fwprintf(pParams->fpErrors, L"Errore ParseDictionaryObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		retValue = 0;
		goto uscita;		
	}
	
	mynumstacklist_Init( &(pParams->myNumStack) );
	
	GetNextToken(pParams);

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
		//wprintf(L"\nERRORE ParseDictionaryObject resourcesdictionary -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
		fwprintf(pParams->fpErrors, L"\nERRORE ParseDictionaryObject resourcesdictionary -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
		
		PrintThisObject(pParams, pParams->nObjToParse, 0, pParams->nCurrentPageNum, pParams->fpErrors);
		
		retValue = 0;
		goto uscita;
	}
		
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
		//wprintf(L"Errore ParseLengthObject: objNum non valido -> %d\n", objNum);
		fwprintf(pParams->fpErrors, L"Errore ParseLengthObject: objNum non valido -> %d\n", objNum);
		goto uscita;
	}
		
	pParams->blockLenLengthObj = 0;
	pParams->blockCurPosLengthObj = 0;
	
	pParams->fpLengthObjRef = NULL;
	pParams->fpLengthObjRef = fopen(pParams->szFileName, "rb");
	if ( pParams->fpLengthObjRef == NULL )
	{
		wprintf(L"Errore nell'apertura del file '%s'.\n\n", pParams->szFileName);
		fwprintf(pParams->fpErrors, L"Errore nell'apertura del file '%s'.\n\n", pParams->szFileName);
		retValue = 0;
		goto uscita;
	}	
	
	pParams->nObjToParse = objNum;
		
	if ( fseek(pParams->fpLengthObjRef, pParams->myObjsTable[objNum]->Offset - 1, SEEK_SET) )
	{
		wprintf(L"Errore ParseLengthObject fseek\n");
		fwprintf(pParams->fpErrors, L"Errore ParseLengthObject fseek\n");
		retValue = 0;
		goto uscita;
	}		
	
	pParams->blockLenLengthObj = fread(pParams->myBlockLengthObj, 1, BLOCK_SIZE, pParams->fpLengthObjRef);
	
	if ( pParams->blockLenLengthObj < 3 )
	{
		//wprintf(L"Errore ParseLengthObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
		fwprintf(pParams->fpErrors, L"Errore ParseLengthObject: Offset oggetto n. %d errato -> %d\n", objNum, pParams->myObjsTable[objNum]->Offset);
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
		//wprintf(L"Errore ParseLengthObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
		fwprintf(pParams->fpErrors, L"Errore ParseLengthObject: oggetto n. %d; atteso spazio, trovato '%c'\n", objNum, c);
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
		//wprintf(L"\nERRORE ParseLengthObject lengthobj -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
		fwprintf(pParams->fpErrors, L"\nERRORE ParseLengthObject lengthobj -> pParams->nObjToParse = %d\n", pParams->nObjToParse);
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
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	
	if ( pParams->myToken.Type == T_INT_LITERAL )
	{
		pParams->nCurrentObjNum = pParams->myToken.Value.vInt;
	}
	if ( pParams->nObjToParse != pParams->nCurrentObjNum )
	{
		//wprintf(L"ERRORE parsing obj: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		fwprintf(pParams->fpErrors, L"ERRORE parsing obj: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		return 0;
	}
		
	if ( !match(pParams, T_INT_LITERAL, "obj") )
		return 0;
		
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
			//wprintf(L"Errore parsing objitem: pParams->myToken.Type = %d\n", pParams->myToken.Type);
			fwprintf(pParams->fpErrors, L"Errore parsing objitem: pParams->myToken.Type = %d\n", pParams->myToken.Type);
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
	
	if ( pParams->myToken.Type == T_INT_LITERAL )
	{
		pParams->nCurrentObjNum = pParams->myToken.Value.vInt;		
	}
	if ( pParams->nObjToParse != pParams->nCurrentObjNum )
	{
		//wprintf(L"ERRORE parsing prepagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		fwprintf(pParams->fpErrors, L"ERRORE parsing prepagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		return 0;
	}
		
	if ( !match(pParams, T_INT_LITERAL, "prepagetree") )
		return 0;
				
	if ( !match(pParams, T_INT_LITERAL, "prepagetree") )
		return 0;
		
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
			//wprintf(L"Errore parsing pagetreearrayobjs: token non valido: %d\n", pParams->myToken.Type);
			fwprintf(pParams->fpErrors, L"Errore parsing pagetreearrayobjs: token non valido: %d\n", pParams->myToken.Type);
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
					
				//if ( pParams->nCountPageFound > pParams->nCountPagesFromPdf )
				//{
				//	wprintf(L"ERRORE parsing prepagetree: numero indice pagina corrente %d maggiore del numero totale di pagine attesi -> %d\n", pParams->nCountPageFound, pParams->nCountPagesFromPdf);
				//	return 0;
				//}
					
				pParams->pPagesArray[pParams->nCountPageFound].numObjNumber = pParams->nCurrentObjNum;
				pParams->pPagesArray[pParams->nCountPageFound].numObjContent = 0;
					
				mycontentqueuelist_Init(&(pParams->pPagesArray[pParams->nCountPageFound].queueContens));
				myintqueuelist_Init(&(pParams->pPagesArray[pParams->nCountPageFound].queueContentsObjRefs));
			}
			//else
			//{
			//	wprintf(L"ERRORE parsing prepagetree: Type non valido: '%s'\n", pParams->myToken.Value.vString);
			//	return 0;
			//}
			
			//return 1;
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

					
	//if ( !match(pParams, T_KW_ENDOBJ, "prepagetree") )
	//	return 0;	
						
	return 1;
}	


//*************************************************************************************************************************

//pagetree          : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ pagetreebody T_KW_ENDOBJ;
int pagetree(Params *pParams)
{
	//pParams->nCountPageAlreadyDone = 0;

	// nCurrentPageParent            -> 0 se nodo radice; altrimenti intero > 0 indica il nodo genitore della pagina corrente
	
	// nCurrentPageResources         -> 0 se la pagina non ha riferimenti a Resources;
	//                                  -1 se la pagina eredita Resources da uno dei suoi parenti;
	//                                  altrimenti un intero > 0 che indica il riferimento al numero dell'oggetto Resources.
	
	// bCurrentPageDirectResources   -> 1 Se risorsa diretta; 0 altrimenti.
	
	//pParams->nCountPagesFromPdf = 0;
		
	pParams->bIsInXObjState = 0;
	
	pParams->nCurrentPageParent = 0;
	
	//pParams->nCurrentPageResources = -1;
	//pParams->bCurrentPageHasDirectResources = 0;
		
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
					
	if ( pParams->myToken.Type == T_INT_LITERAL )
	{
		pParams->nCurrentObjNum = pParams->myToken.Value.vInt;		
	}
	if ( pParams->nObjToParse != pParams->nCurrentObjNum )
	{
		//wprintf(L"ERRORE parsing pagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		fwprintf(pParams->fpErrors, L"ERRORE parsing pagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		return 0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	
	if ( !match(pParams, T_INT_LITERAL, "pagetree") )
		return 0;
		
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
	//	wprintf(L"ERRORE parsing prepagetree: numero indice pagina corrente %d maggiore del numero totale di pagine attesi -> %d\n", pParams->nCountPageFound, pParams->nCountPagesFromPdf);
	//	return 0;
	//}	
	
	if ( pParams->nCurrentPageResources > 0 )
	{
		pParams->nDictionaryType = DICTIONARY_TYPE_RESOURCES;
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentPageResources) )
			return 0;		
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
				{
					//wprintf(L"Errore parsing pagetreeobj: atteso T_KW_R, trovato %d\n", pParams->myToken.Type);
					fwprintf(pParams->fpErrors, L"Errore parsing pagetreeobj: atteso T_KW_R, trovato %d\n", pParams->myToken.Type);
					return 0;
				}
				
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
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
							wprintf(L"ECCO, pagetreeobj -> metto in coda(pParams->myXObjRefList) l'XObjRef(Key = '%s') %d della pagina %d\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCountPageFound);
							#endif						
							myobjreflist_Enqueue(&(pParams->myXObjRefList), pParams->szCurrResourcesKeyName, iNum);
						}
						else
						{
							if ( strncmp(pParams->szCurrResourcesKeyName, "XObject", 4096) == 0  )
								pParams->nCurrentXObjRef = iNum;
						}
					
						if ( pParams->bFontsKeys )
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
							wprintf(L"ECCO, pagetreeobj -> metto in coda(pParams->myFontsRefList) il FontsRef(Key = '%s') %d della pagina %d\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCountPageFound);
							#endif						
							myobjreflist_Enqueue(&(pParams->myFontsRefList), pParams->szCurrResourcesKeyName, iNum);
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
			//wprintf(L"Errore parsing pagetreeobj: token non valido: %d\n", pParams->myToken.Type);
			fwprintf(pParams->fpErrors, L"Errore parsing pagetreeobj: token non valido: %d\n", pParams->myToken.Type);
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
				//wprintf(L"Errore parsing pagetreearrayobjs: token non valido: %d\n", pParams->myToken.Type);
				fwprintf(pParams->fpErrors, L"Errore parsing pagetreearrayobjs: token non valido: %d\n", pParams->myToken.Type);
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
			
			strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.Value.vString, 4096 - 1);
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
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
				
	if ( !match(pParams, T_INT_LITERAL, "contentobj") )
		return 0;
		
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
		//wprintf(L"ERRORE contentobj: la chiave 'Length' e' assente.\n");
		fwprintf(pParams->fpErrors, L"ERRORE contentobj: la chiave 'Length' e' assente.\n");
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
		//wprintf(L"ERRORE contentobjbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		fwprintf(pParams->fpErrors, L"ERRORE contentobjbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		retValue = 0;
		goto uscita;
	}
	pParams->CurrentContent.Offset = pParams->nNumBytesReadFromCurrentStream;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	
	//pos1 = ftell(pParams->fp);	
	//GetNextToken(pParams);
	if ( fseek(pParams->fp, pParams->CurrentContent.Offset + pParams->CurrentContent.LengthFromPdf, SEEK_SET) != 0 )
	{
		wprintf(L"ERRORE contentobjbody: fseek\n");
		fwprintf(pParams->fpErrors, L"ERRORE contentobjbody: fseek\n");
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
		
	/*
	while ( pParams->myToken.Type != T_KW_ENDSTREAM )
	{		
		if ( pParams->myToken.Type == T_ERROR || pParams->myToken.Type == T_EOF )
		{
			wprintf(L"ERRORE contentobjbody: Atteso token T_KW_ENDSTREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			retValue = 0;
			goto uscita;
		}
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
		
		GetNextToken(pParams);
	}
	*/
	
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
			//wprintf(L"ERRORE parsing contentobjbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
			fwprintf(pParams->fpErrors, L"ERRORE parsing contentobjbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
			retValue = 0;
			goto uscita;
		}
		
		if ( pParams->bStreamStateToUnicode )
		{
			//wprintf(L"ERRORE parsing contentobjbody(bStreamStateToUnicode): DecodeParams non supportati in questa versione del programma\n");
			fwprintf(pParams->fpErrors, L"ERRORE parsing contentobjbody(bStreamStateToUnicode): DecodeParams non supportati in questa versione del programma\n");
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
		//wprintf(L"ERRORE streamdictitems: Atteso token T_NAME, trovato TOKEN n° %d\n", pParams->myToken.Type);
		fwprintf(pParams->fpErrors, L"ERRORE streamdictitems: Atteso token T_NAME, trovato TOKEN n° %d\n", pParams->myToken.Type);
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
						//wprintf(L"ERRORE contentkeyvalue streamobj number %d\n", n1);
						fwprintf(pParams->fpErrors, L"ERRORE contentkeyvalue streamobj number %d\n", n1);
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
					//wprintf(L"ERRORE contentkeyvalue(bStreamStateToUnicode): Flitri diversi da FlateDecode non supportati in questa versione del programma.\n");
					fwprintf(pParams->fpErrors, L"ERRORE contentkeyvalue(bStreamStateToUnicode): Flitri diversi da FlateDecode non supportati in questa versione del programma.\n");
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
			//wprintf(L"ERRORE contentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			fwprintf(pParams->fpErrors, L"ERRORE contentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
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
			//wprintf(L"ERRORE contentkeyarray: Atteso uno di questi token: T_NAME, T_KW_NULL, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			fwprintf(pParams->fpErrors, L"ERRORE contentkeyarray: Atteso uno di questi token: T_NAME, T_KW_NULL, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
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
		wprintf(L"ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		fwprintf(pParams->fpErrors, L"ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
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
			wprintf(L"ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			fwprintf(pParams->fpErrors, L"ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
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
			//wprintf(L" '%s'", pParams->myToken.Value.vString);
			/*
			if ( (strncmp(pParams->szCurrKeyName, "Filter", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
			{
				mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.Value.vString);
				if ( (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
					pParams->CurrentContent.bExternalFile = 1;
			}
			*/
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
			//wprintf(L"ERRORE contentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
			fwprintf(pParams->fpErrors, L"ERRORE contentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d\n", pParams->myToken.Type);
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
		//wprintf(L"\nERRORE lengthobj -> errore di sintassi: Atteso token T_INT_LITERAL, trovato token n° %d\n", pParams->myTokenLengthObj.Type);
		fwprintf(pParams->fpErrors, L"\nERRORE lengthobj -> errore di sintassi: Atteso token T_INT_LITERAL, trovato token n° %d\n", pParams->myTokenLengthObj.Type);
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

// resourcesdictionary      : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ T_DICT_BEGIN resourcesdictionarybody T_DICT_END T_KW_ENDOBJ;
int resourcesdictionary(Params *pParams)
{	
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
				{
					//wprintf(L"Errore parsing resourcesdictionaryitems: atteso T_KW_R, trovato %d\n", pParams->myToken.Type);
					fwprintf(pParams->fpErrors, L"Errore parsing resourcesdictionaryitems: atteso T_KW_R, trovato %d\n", pParams->myToken.Type);
					return 0;
				}
				
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
			wprintf(L"Errore parsing resourcesdictionaryitems: token non valido: %d\n", pParams->myToken.Type);
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
			//wprintf(L"Errore parsing resourcesarrayobjs: token non valido: %d\n", pParams->myToken.Type);
			fwprintf(pParams->fpErrors, L"Errore parsing resourcesarrayobjs: token non valido: %d\n", pParams->myToken.Type);
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
			
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_INT_LITERAL, "contentxobj") )
		return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
			
	if ( !match(pParams, T_INT_LITERAL, "contentxobj") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
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
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_ENDOBJ, "contentxobj") )
		return 0;
			
	if ( !(pParams->bStreamLengthIsPresent) )
	{
		//wprintf(L"ERRORE contentxobj: la chiave 'Length' e' assente.\n");
		fwprintf(pParams->fpErrors, L"ERRORE contentxobj: la chiave 'Length' è assente.\n");
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
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
			
	if ( !match(pParams, T_DICT_END, "contentxobjbody") )
	{
		retValue = 0;
		goto uscita;
	}
				
	if ( pParams->myToken.Type != T_KW_STREAM )
	{
		//wprintf(L"ERRORE contentxobjbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		fwprintf(pParams->fpErrors, L"ERRORE contentxobjbody: Atteso token T_KW_STREAM, trovato TOKEN n° %d\n", pParams->myToken.Type);
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		retValue = 0;
		goto uscita;
	}
	pParams->CurrentContent.Offset = pParams->nNumBytesReadFromCurrentStream;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)	
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	//GetNextToken(pParams);
	if ( fseek(pParams->fp, pParams->CurrentContent.Offset + pParams->CurrentContent.LengthFromPdf, SEEK_SET) != 0 )
	{
		wprintf(L"ERRORE contentxobjbody: fseek\n");
		fwprintf(pParams->fpErrors, L"ERRORE contentxobjbody: fseek\n");
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
			//wprintf(L"ERRORE parsing contentxobjbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
			fwprintf(pParams->fpErrors, L"ERRORE parsing contentxobjbody: il numero di DecodeParams(%d) non coincide col numero di Filters(%d)\n", pParams->nCountDecodeParams, pParams->nCountFilters);
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
	while ( pParams->myToken.Type ==  T_NAME )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
		
		if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
			pParams->bStreamLengthIsPresent = 1;		
						
		if ( strncmp(pParams->szCurrKeyName, "F", 1024) == 0 )
			pParams->CurrentContent.bExternalFile = 1;	
			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
			
		GetNextToken(pParams);
		
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
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.Value.vInt;
							
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)	
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
						//wprintf(L"ERRORE xobjcontentkeyvalue streamobj number %d\n", n1);
						fwprintf(pParams->fpErrors, L"ERRORE xobjcontentkeyvalue streamobj number %d\n", n1);
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
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)	
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
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);			
			#endif			
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_KW_TRUE:
		case T_KW_FALSE:
			// IGNORIAMO
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
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
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			GetNextToken(pParams);
			break;
		case T_QOPAREN:
			if ( !xobjcontentkeyarray(pParams) )
				return 0;
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
			break;
		default:
			//wprintf(L"ERRORE xobjcontentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			fwprintf(pParams->fpErrors, L"ERRORE xobjcontentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
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
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);
				
		goto ricomincia;
	}

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
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
			//wprintf(L"ERRORE xobjcontentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			fwprintf(pParams->fpErrors, L"ERRORE xobjcontentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			return 0;
		}
		strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.Value.vString, len + sizeof(char));
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);			
				
		if ( !xobjcontentkeyvalue(pParams) )
		{
			return 0;
		}		
	}
	
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
	
	pParams->bCurrentFontHasDirectEncodingArray = 0;
	pParams->nCurrentEncodingObj = 0;
	pParams->nToUnicodeStreamObjRef = 0;
	pParams->szTemp[0] = '\0';
	
	pParams->nCurrentUseCMapRef = 0;
	pParams->szUseCMap[0] = '\0';
	
	for ( k = 0; k < 256; k++ )
		pParams->paCustomizedFont_CharSet[k] = 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
			
	if ( !match(pParams, T_INT_LITERAL, "contentfontobj") )
	{
		return 0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	if ( !match(pParams, T_INT_LITERAL, "contentfontobj") )
	{
		return 0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_KW_OBJ, "contentfontobj") )
	{
		return 0;
	}
	
	if ( !contentfontobjbody(pParams) )
		return 0;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_ENDOBJ, "contentfontobj") )
	{
		return 0;
	}
	
	pParams->bHasCodeSpaceTwoByte = 0;
	pParams->bHasCodeSpaceOneByte = 1;
	
	switch ( pParams->nCurrentFontSubtype )
	{
		case FONT_SUBTYPE_Type0:
			strncpy(szFontType, "Type0", 6);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\n\n***** TROVATO FONT Type0 !!! *****\n");
			#endif
			break;
		case FONT_SUBTYPE_Type1:
			strncpy(szFontType, "Type1", 6);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\n\n***** TROVATO FONT Type1 !!! *****\n");
			#endif
			break;
		case FONT_SUBTYPE_MMType1:
			strncpy(szFontType, "MMType1", 8);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\n\n***** TROVATO FONT MMType1 !!! *****\n");
			#endif
			break;
		case FONT_SUBTYPE_Type3:
			strncpy(szFontType, "Type3", 6);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\n\n***** TROVATO FONT Type3 !!! *****\n");
			#endif
			break;
		case FONT_SUBTYPE_TrueType:
			strncpy(szFontType, "TrueType", 9);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\n\n***** TROVATO FONT TrueType !!! *****\n");
			#endif
			break;
		case FONT_SUBTYPE_CIDFontType0:
			strncpy(szFontType, "CIDFontType0", 13);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\n\n***** TROVATO FONT CIDFontType0 !!! *****\n");
			#endif
			break;
		case FONT_SUBTYPE_CIDFontType2:
			strncpy(szFontType, "CIDFontType2", 13);
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\n\n***** TROVATO FONT CIDFontType2 !!! *****\n");
			#endif
			break;
		default:
			szFontType[0] = '\0';
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\n\n***** TROVATO UNKNOWN FONT *****\n");
			#endif
			break;
	}
		

	if ( FONT_SUBTYPE_Type0 == pParams->nCurrentFontSubtype && pParams->nToUnicodeStreamObjRef <= 0 )
	{
		//wprintf(L"\n\n***** FONT Type0 NON IMPLEMENTATO PER IL MOMENTO!!! *****\n\n");
		//fwprintf(pParams->fpErrors, L"\n***** FONT Type0 NON IMPLEMENTATO PER IL MOMENTO!!! *****\n");
		//return 0;
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		wprintf(L"\t***** 1 pParams->nToUnicodeStreamObjRef = %d, pParams->nCurrentUseCMapRef = %d *****\n", pParams->nToUnicodeStreamObjRef, pParams->nCurrentUseCMapRef);
		#endif
				
		if ( pParams->nCurrentEncodingObj > 0 )
		{
			// Non c'è bisogno di inizializzare qui. Lo fa la funzione 'ParseCMapStream'.
			for ( int i = 0; i < pParams->dimCustomizedFont_CharSet; i++ )
				pParams->paCustomizedFont_CharSet[i] = pParams->pArrayUnicode[i];
				
			for ( int j = pParams->dimCustomizedFont_CharSet; j < 0xFFFF; j++ )
				pParams->paCustomizedFont_CharSet[j] = L' ';				
				
			pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
				
			for ( int j = pParams->dimCustomizedFont_CharSet; j < 0xFFFF; j++ )
				pParams->paCustomizedFont_CharSet[j] = L' ';
			
			if ( pParams->nCurrentUseCMapRef > 0 )
			{
				pParams->bEncodigArrayAlreadyInit = 0;
				
				pParams->bStreamType = STREAM_TYPE_CMAP;
				if ( !ParseCMapObject(pParams, pParams->nCurrentUseCMapRef) )
					return 0;
					
				pParams->bEncodigArrayAlreadyInit = 1;
			
				//pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
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
					wprintf(L"\t***** USECMAP PREDEFINITO -> '%s'. OK! *****\n\n", pParams->szUseCMap);
					#endif
					return 1;
				}				
				else
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					wprintf(L"\t***** USECMAP PREDEFINITO -> '%s', NON IMPLEMENTATO PER IL MOMENTO!!! *****\n\n", pParams->szUseCMap);
					#endif
					fwprintf(pParams->fpErrors, L"\n***** FONT '%s' USECMAP PREDEFINITO -> '%s', NON IMPLEMENTATO PER IL MOMENTO!!! *****\n", szFontType, pParams->szUseCMap);
					return 0;				
				}
			}
		
			// pParams->nCurrentEncodingObj
			pParams->bStreamType = STREAM_TYPE_CMAP;
			if ( !ParseCMapObject(pParams, pParams->nCurrentEncodingObj) )
				return 0;
			
			pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
		}
		else   // QUI GESTIONE CMAP PREDEFINITO
		{
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
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\t***** CMAP PREDEFINITO -> '%s', NON IMPLEMENTATO PER IL MOMENTO!!! *****\n\n", pParams->szTemp);
				#endif
				fwprintf(pParams->fpErrors, L"\n***** FONT '%s' CMAP PREDEFINITO -> '%s', NON IMPLEMENTATO PER IL MOMENTO!!! *****\n", szFontType, pParams->szTemp);
				return 0;			
			}
		}
		
		return 1;
	}
	
	if ( pParams->nToUnicodeStreamObjRef > 0 )
	{
		// Non c'è bisogno di inizializzare qui. Lo fa la funzione 'ParseToUnicodeStream'.
		for ( int i = 0; i < pParams->dimCustomizedFont_CharSet; i++ )
			pParams->paCustomizedFont_CharSet[i] = pParams->pArrayUnicode[i];
			
		for ( int j = pParams->dimCustomizedFont_CharSet; j < 0xFFFF; j++ )
			pParams->paCustomizedFont_CharSet[j] = L' ';
			
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
			
			//pParams->pCurrentEncodingArray = &(pParams->paCustomizedFont_CharSet[0]);
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
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\t***** USECMAP TOUNICODE PREDEFINITO -> '%s', NON IMPLEMENTATO PER IL MOMENTO!!! *****\n\n", pParams->szUseCMap);
				#endif
				fwprintf(pParams->fpErrors, L"\n***** FONT '%s' USECMAP TOUNICODE PREDEFINITO -> '%s', NON IMPLEMENTATO PER IL MOMENTO!!! *****\n", szFontType, pParams->szUseCMap);
				return 0;
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
	
	//if ( pParams->nCurrentFontSubtype == FONT_SUBTYPE_Type0 )
	//{		
	//	PrintThisObject(pParams, 1030, 0, 0, NULL);
	//	PrintThisObject(pParams, 2562, 0, 0, NULL);
	//	PrintThisObject(pParams, 4072, 0, 0, NULL);
	//}
		
	return 1;
}

// contentfontobjbody     : T_DICT_BEGIN fontobjstreamdictitems T_DICT_END;
int contentfontobjbody(Params *pParams)
{
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_BEGIN, "contentfontobjbody") )
	{
		return 0;
	}
	
	if ( !fontobjstreamdictitems(pParams) )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_END, "contentfontobjbody") )
	{
		return 0;
	}
	
	return 1;
}

// fontobjstreamdictitems : {T_NAME fontobjcontentkeyvalue};
int fontobjstreamdictitems(Params *pParams)
{
	while ( pParams->myToken.Type == T_NAME )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.Value.vString, strnlen(pParams->myToken.Value.vString, 4096) + 1);
	
		//if ( strncmp(pParams->szCurrKeyName, "Length", 1024) == 0 )
		//	pParams->bStreamLengthIsPresent = 1;		
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
		
		//free(pParams->myToken.Value.vString);
		//pParams->myToken.Value.vString = NULL;		
		
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
				{
					//if ( NULL != pParams->myToken.Value.vString )
					//{
					//	free(pParams->myToken.Value.vString);
					//	pParams->myToken.Value.vString = NULL;
					//}

					pParams->nCurrentFontSubtype = FONT_SUBTYPE_Type0;
					
					//wprintf(L"FONT Type0 NON IMPLEMENTATO!!! ****************************************************************************************************\n");
					//return 0; // NON IMPLEMENTATO, PER IL MOMENTO.
				}
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
			//wprintf(L"ERRORE fontobjcontentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			fwprintf(pParams->fpErrors, L"ERRORE fontobjcontentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
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
			//wprintf(L"ERRORE fontdirectencodingobjarray: Atteso T_NAME, trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			fwprintf(pParams->fpErrors, L"ERRORE fontdirectencodingobjarray: Atteso T_NAME, trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
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
				//if ( !match(pParams, T_KW_R, "fontobjcontentkeyarray") )
				//	return 0;				
				if ( pParams->myToken.Type == T_KW_R )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					PrintToken(&(pParams->myToken), ' ', ' ', 1);
					#endif
					
					GetNextToken(pParams);
				}
				//else // T_INT_LITERAL OPPURE T_REAL_LITERAL
				//{
				//}
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
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_INT_LITERAL, "encodingobj") )
	{
		return 0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	
	if ( !match(pParams, T_INT_LITERAL, "encodingobj") )
	{
		return 0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_KW_OBJ, "encodingobj") )
	{
		return 0;
	}
	
	if ( !encodingobjbody(pParams) )
		return 0;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_ENDOBJ, "encodingobj") )
	{
		return 0;
	}
	
	
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
		//wprintf(L"ERRORE encodingobjbody: Atteso uno di questi token: T_NAME, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
		fwprintf(pParams->fpErrors, L"ERRORE encodingobjbody: Atteso uno di questi token: T_NAME, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
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
			//wprintf(L"ERRORE encodingobjarray: Atteso T_NAME, trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			fwprintf(pParams->fpErrors, L"ERRORE encodingobjarray: Atteso T_NAME, trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
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

int pdf(Params *pParams)
{
	int retValue = 1;
	
	retValue = ReadTrailer(pParams);
	
	return retValue;
}

