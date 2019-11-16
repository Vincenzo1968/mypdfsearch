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
			wprintf(L"T_NAME = '%s'", pToken->vString);
			break;
		case T_STRING:
			wprintf(L"T_STRING = '%s'", pToken->vString);
			break;
		case T_STRING_LITERAL:
			wprintf(L"T_STRING_LITERAL = '%s'", pToken->vString);
			break;
		case T_STRING_HEXADECIMAL:
			wprintf(L"T_STRING_HEXADECIMAL = '%s'", pToken->vString);
			break;
		case T_INT_LITERAL:
			wprintf(L"T_INT_LITERAL = %d", pToken->vInt);			
			break;
		case T_REAL_LITERAL:
			wprintf(L"T_REAL_LITERAL = %f", pToken->vDouble);
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
		case T_CONTENT_OP_Do:
			wprintf(L"T_CONTENT_OP_Do = 'Do'");
			break;
		case T_CONTENT_OP_BT:
			wprintf(L"T_CONTENT_OP_BT = 'BT -> Begin Text'");
			break;
		case T_CONTENT_OP_ET:
			wprintf(L"T_CONTENT_OP_ET = 'ET -> End Text'");
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
		case T_CONTENT_OP_TSTAR:
			wprintf(L"T_CONTENT_OP_TSTAR = 'T*'");
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
		case T_CONTENT_OP_Tz:
			wprintf(L"T_CONTENT_OP_Tz = 'Tz'");
			break;
		case T_CONTENT_OP_TL:
			wprintf(L"T_CONTENT_OP_TL = 'TL'");
			break;
		case T_CONTENT_OP_Ts:
			wprintf(L"T_CONTENT_OP_Ts = 'Ts'");
			break;
		case T_CONTENT_OP_Tr:
			wprintf(L"T_CONTENT_OP_Tr = 'Tr'");
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
		case T_CONTENT_OP_cm:
			wprintf(L"T_CONTENT_OP_cm = 'cm'");
			break;
		case T_CONTENT_OP_q:
			wprintf(L"T_CONTENT_OP_q = 'q'");
			break;
		case T_CONTENT_OP_Q:
			wprintf(L"T_CONTENT_OP_Q = 'Q'");
			break;
		case T_CONTENT_OP_MP:
			wprintf(L"T_CONTENT_OP_MP = 'MP'");
			break;
		case T_CONTENT_OP_DP:
			wprintf(L"T_CONTENT_OP_DP = 'DP'");
			break;
		case T_CONTENT_OP_BMC:
			wprintf(L"T_CONTENT_OP_BMC = 'BMC'");
			break;
		case T_CONTENT_OP_BDC:
			wprintf(L"T_CONTENT_OP_BDC = 'BDC'");
			break;
		case T_CONTENT_OP_EMC:
			wprintf(L"T_CONTENT_OP_EMV = 'EMC'");
			break;
		case T_CONTENT_OP_w:
			wprintf(L"T_CONTENT_OP_w = 'w'");
			break;
		case T_CONTENT_OP_J:
			wprintf(L"T_CONTENT_OP_J = 'J'");
			break;
		case T_CONTENT_OP_j:
			wprintf(L"T_CONTENT_OP_j = 'j'");
			break;
		case T_CONTENT_OP_M:
			wprintf(L"T_CONTENT_OP_M = 'M'");
			break;
		case T_CONTENT_OP_d:
			wprintf(L"T_CONTENT_OP_d = 'd'");
			break;
		case T_CONTENT_OP_ri:
			wprintf(L"T_CONTENT_OP_ri = 'ri'");
			break;
		case T_CONTENT_OP_i:
			wprintf(L"T_CONTENT_OP_i = 'i'");
			break;
		case T_CONTENT_OP_gs:
			wprintf(L"T_CONTENT_OP_gs = 'gs'");
			break;
		case T_CONTENT_OP_BI:
			wprintf(L"T_CONTENT_OP_BI = 'BI'");
			break;
		case T_CONTENT_OP_ID:
			wprintf(L"T_CONTENT_OP_ID = 'ID'");
			break;
		case T_CONTENT_OP_EI:
			wprintf(L"T_CONTENT_OP_EI = 'EI'");
			break;
		case T_CONTENT_OP_BX:
			wprintf(L"T_CONTENT_OP_EI = 'BX'");
			break;
		case T_CONTENT_OP_EX:
			wprintf(L"T_CONTENT_OP_EX = 'EX'");
			break;
		case T_CONTENT_OP_m:
			wprintf(L"T_CONTENT_OP_m = 'm");
			break;
		case T_CONTENT_OP_l:
			wprintf(L"T_CONTENT_OP_l = 'l'");
			break;
		case T_CONTENT_OP_c:
			wprintf(L"T_CONTENT_OP_c = 'c'");
			break;
		case T_CONTENT_OP_v:
			wprintf(L"T_CONTENT_OP_v = 'v'");
			break;
		case T_CONTENT_OP_y:
			wprintf(L"T_CONTENT_OP_y = 'y'");
			break;
		case T_CONTENT_OP_h:
			wprintf(L"T_CONTENT_OP_h = 'h'");
			break;
		case T_CONTENT_OP_re:
			wprintf(L"T_CONTENT_OP_re = 're'");
			break;
		case T_CONTENT_OP_S:
			wprintf(L"T_CONTENT_OP_S = 'S'");
			break;
		case T_CONTENT_OP_s:
			wprintf(L"T_CONTENT_OP_s = 's'");
			break;
		case T_CONTENT_OP_f:
			wprintf(L"T_CONTENT_OP_f = 'f'");
			break;
		case T_CONTENT_OP_F:
			wprintf(L"T_CONTENT_OP_F = 'F'");
			break;
		case T_CONTENT_OP_fSTAR:
			wprintf(L"T_CONTENT_OP_fSTAR = 'f*'");
			break;
		case T_CONTENT_OP_B:
			wprintf(L"T_CONTENT_OP_B = 'B'");
			break;
		case T_CONTENT_OP_BSTAR:
			wprintf(L"T_CONTENT_OP_BSTAR = 'B*'");
			break;
		case T_CONTENT_OP_b:
			wprintf(L"T_CONTENT_OP_b = 'b'");
			break;
		case T_CONTENT_OP_bSTAR:
			wprintf(L"T_CONTENT_OP_bSTAR = 'b*'");
			break;
		case T_CONTENT_OP_n:
			wprintf(L"T_CONTENT_OP_n = 'n'");
			break;
		case T_CONTENT_OP_W:		
			wprintf(L"T_CONTENT_OP_W = 'W'");
			break;
		case T_CONTENT_OP_WSTAR:
			wprintf(L"T_CONTENT_OP_WSTAR = 'W*'");
			break;
		case T_CONTENT_OP_d0:
			wprintf(L"T_CONTENT_OP_d0 = 'd0'");
			break;
		case T_CONTENT_OP_d1:
			wprintf(L"T_CONTENT_OP_d1 = 'd1'");
			break;
		case T_CONTENT_OP_CS:
			wprintf(L"T_CONTENT_OP_CS = 'CS'");
			break;
		case T_CONTENT_OP_cs:
			wprintf(L"T_CONTENT_OP_cs = 'cs'");
			break;
		case T_CONTENT_OP_SC:
			wprintf(L"T_CONTENT_OP_sc = 'sc'");
			break;
		case T_CONTENT_OP_SCN:
			wprintf(L"T_CONTENT_OP_SCN = 'SCN'");
			break;
		case T_CONTENT_OP_sc:
			wprintf(L"T_CONTENT_OP_sc = 'sc'");
			break;
		case T_CONTENT_OP_scn:
			wprintf(L"T_CONTENT_OP_scn = 'scn'");
			break;
		case T_CONTENT_OP_G:
			wprintf(L"T_CONTENT_OP_G = 'G'");
			break;
		case T_CONTENT_OP_g:
			wprintf(L"T_CONTENT_OP_g = 'g'");
			break;
		case T_CONTENT_OP_RG:
			wprintf(L"T_CONTENT_OP_RG = 'RG'");
			break;
		case T_CONTENT_OP_rg:
			wprintf(L"T_CONTENT_OP_rg = 'rg'");
			break;
		case T_CONTENT_OP_K:
			wprintf(L"T_CONTENT_OP_K = 'K'");
			break;
		case T_CONTENT_OP_k:
			wprintf(L"T_CONTENT_OP_k = 'k'");
			break;
		case T_CONTENT_OP_sh:
			wprintf(L"T_CONTENT_OP_sh = 'sh'");
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
		
	unsigned char c;
	unsigned char lexeme[128];
	
	unsigned char NumberLexeme[128];
	int idxNumberLexeme = 0;
	int lenNumberLexeme = 0;
	
	char key[256];
	uint32_t keyLength;
	GenHashTable_t myHT;
	PdfIndirectObject myHT_Data;
	PdfIndirectObject *pmyHT_Data;
	uint32_t myHT_DataSize = 0;
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
	int bSubtypeState = 0;
	int bCurrentSubtypeIsImage = 0;
	int cDelim;
	char keyImageObj[256];
	uint32_t keyImageObjLength;
	
	PreParseStates state = S_PP0;
	
	fp = fopen(pszFileName, "rb");
	if ( fp == NULL )
	{
		snprintf(pParams->szError, 8192, "ERROR getObjsOffsets: fopen failed for file '%s'.\n", pszFileName);
		myShowErrorMessage(pParams, pParams->szError, 1);
		retValue = 0;
		goto uscita;
	}
	
	if ( !genhtInit(&myHT, GENHT_SIZE, GenStringHashFunc, GenStringCompareFunc) )
	{
		snprintf(pParams->szError, 8192, "ERROR getObjsOffsets: genhtInit failed for file '%s'.\n", pszFileName);
		myShowErrorMessage(pParams, pParams->szError, 1);
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
					else if ( !bStreamState && '/' == c )
					{
						if ( !bSubtypeState )
						{
							//wprintf(L"OK, sono in S_PP0; TROVATO '/'; vado in state S_PP19\n");
							state = S_PP19;
						}
						else
						{
							//wprintf(L"OK, sono in S_PP0; TROVATO '/'; vado in state S_PP27\n");
							state = S_PP27;
						}
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
												
						pmyHT_Data = &myHT_Data;
						myHT_DataSize = 0;
						retKeyFind = genhtFind(&myHT, key, keyLength, (void**)&pmyHT_Data, &myHT_DataSize);
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
							
							if ( bCurrentSubtypeIsImage )
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_getObjsOffsets_FN) || defined(MYDEBUG_PRINT_ON_getObjsOffsets3_FN)
								wprintf(L"\nENDSTREAM -> OBJECT[number %lu, generation %lu] IS IMAGE.\n", myHT_Data.Number, myHT_Data.Generation);
								#endif
								
								snprintf(keyImageObj, 256, "k%u", myHT_Data.Number);
								keyImageObjLength = strnlen(keyImageObj, 256);
								genhtInsert(&(pParams->myHT_ImageObjs), keyImageObj, keyImageObjLength + sizeof(char), NULL, 0);
								
								bCurrentSubtypeIsImage = 0;
							}
							
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
						if ( bCurrentSubtypeIsImage )
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_getObjsOffsets_FN) || defined(MYDEBUG_PRINT_ON_getObjsOffsets3_FN)
							wprintf(L"\nENDOBJ -> OBJECT[number %lu, generation %lu] IS IMAGE.\n", myHT_Data.Number, myHT_Data.Generation);
							#endif
							
							snprintf(keyImageObj, 256, "k%u", myHT_Data.Number);
							keyImageObjLength = strnlen(keyImageObj, 256);
							genhtInsert(&(pParams->myHT_ImageObjs), keyImageObj + sizeof(char), keyImageObjLength, NULL, 0);
							
							bCurrentSubtypeIsImage = 0;
						}
						
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
									retValue = 0;
									goto uscita;
								}
								
								snprintf(pParams->szError, 8192, "\nWARNING OBJECT(Number: %d, Generation: %d): STREAM NOT CLOSED BY 'endstream', but 'endobj' keyword.\n", myHT_Data.Number, myHT_Data.Generation);
								myShowErrorMessage(pParams, pParams->szError, 1);
								
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
				case S_PP19:
					if ( 'S' == c )
					{
						//wprintf(L"OK, sono in S_PP19; TROVATO 'S'; vado in state S_PP20\n");
						state = S_PP20;
					}
					else
						state = S_PP0;
					break;
				case S_PP20:
					if ( 'u' == c )
						state = S_PP21;
					else
						state = S_PP0;
					break;
				case S_PP21:
					if ( 'b' == c )
						state = S_PP22;
					else
						state = S_PP0;
					break;
				case S_PP22:
					if ( 't' == c )
						state = S_PP23;
					else
						state = S_PP0;
					break;
				case S_PP23:
					if ( 'y' == c )
						state = S_PP24;
					else
						state = S_PP0;
					break;
				case S_PP24:
					if ( 'p' == c )
						state = S_PP25;
					else
						state = S_PP0;
					break;
				case S_PP25:
					if ( 'e' == c )
						state = S_PP26;
					else
						state = S_PP0;
					break;
				case S_PP26:
					cDelim = IsDelimiterChar(c);
					if ( cDelim == DELIM_SPACECHAR )
					{
						bSubtypeState = 1;
						state = S_PP0;
					}
					else if ( cDelim == '/' )
					{
						//wprintf(L"OK, sono in S_PP27; TROVATO '/'; vado in state S_PP27\n");
						state = S_PP27;
					}
					else
					{
						state = S_PP0;
					}
					break;
				case S_PP27:
					if ( 'I' == c )
						state = S_PP28;
					else
						state = S_PP0;
					break;
				case S_PP28:
					if ( 'm' == c )
						state = S_PP29;
					else
						state = S_PP0;
					break;
				case S_PP29:
					if ( 'a' == c )
						state = S_PP30;
					else
						state = S_PP0;
					break;
				case S_PP30:
					if ( 'g' == c )
						state = S_PP31;
					else
						state = S_PP0;
					break;
				case S_PP31:
					if ( 'e' == c )
						state = S_PP32;
					else
						state = S_PP0;
					break;
				case S_PP32:
					if ( IsDelimiterChar(c) )
					{
						bSubtypeState = 0;
						bCurrentSubtypeIsImage = 1;
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
		pmyHT_Data = &myHT_Data;
		myHT_DataSize = 0;
		retKeyFind = genhtFind(&myHT, key, strnlen(key, 256), (void**)&pmyHT_Data, &myHT_DataSize);
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
			pParams->myObjsTable[k]->Obj.pTreeNode = NULL;
			
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
			pParams->myObjsTable[k]->Obj.pTreeNode = NULL;
			
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
		myobjreflist_Init(&(pParams->myObjsTable[k]->myGsRefList));
		myintqueuelist_Init(&(pParams->myObjsTable[k]->queueContentsObjRefs));
		//pParams->myObjsTable[k]->dFontSpaceWidth = -1.0;
		pParams->myObjsTable[k]->pGlyphsWidths = NULL;
		pParams->myObjsTable[k]->pszDirectFontResourceString = NULL;
		pParams->myObjsTable[k]->lenDirectFontResourceString = 0;
		pParams->myObjsTable[k]->pszDirectGsResourceString = NULL;
		pParams->myObjsTable[k]->lenDirectGsResourceString = 0;
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
	pParams->myToken.vString = NULL;
	
	pParams->myTokenLengthObj.Type = T_NAME;
	pParams->myTokenLengthObj.vString = NULL;
		
	pParams->isEncrypted = 0;
	
	pParams->nXRefStreamObjNum = 0;
	pParams->offsetXRefObjStream = 0;
	
	pParams->myObjsTable = NULL;
	pParams->nObjsTableSizeFromPrescanFile = 0;
	pParams->pPagesTree = NULL;
	pParams->pPagesArray = NULL;
	
	pParams->bUpdateNumBytesReadFromCurrentStream = 1;
		
	pParams->bStreamState = 0;
	pParams->bStringIsDecoded = 0;
	pParams->bStreamType = STREAM_TYPE_GENERIC;
	
	//pParams->pEncodingString = NULL;
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
	
	pParams->bMakeDirectFontResourceString = 0;
	pParams->bMakeDirectGsResourceString = 0;
	pParams->pszDirectFontResourceString = NULL;
	pParams->lenDirectFontResourceString = 0;
	pParams->pszDirectGsResourceString = NULL;
	pParams->lenDirectGsResourceString = 0;
	pParams->nSizeDirectResourceString = 4096 * 21;
	
	pParams->bParsingFontObj = 0;
	pParams->bParsingGsObj = 0;
			
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
	
	vlrbtInit(&(pParams->myRedBlackTree), VlRbtCompareFuncOrd, VlRbtOnTraverseFunc);
	
	pParams->pText = NULL;
	
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
	
	/*
	#if defined(MYPDFSEARCH_USE_TST)
	tstInit(&(pParams->myTST));
	#else
	genhtInit(&(pParams->myHT), 0, GenWideStringHashFunc, GenWideStringCompareFunc);
	#endif
	
	genhtInit(&(pParams->myHT_EncodingArray), 0, GenStringHashFunc, GenStringCompareFunc);
	*/
		
	myobjreflist_Init(&(pParams->myXObjRefList));
	myobjreflist_Init(&(pParams->myFontsRefList));
	myobjreflist_Init(&(pParams->myGsRefList));
		
	/*
	pParams->myToken.vString = (char *)malloc( sizeof(char) * MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(char) );
	if ( !(pParams->myToken.vString) )
	{
		snprintf(pParams->szError, 8192, "ERRORE Parse 1: Memoria insufficiente.\n\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE Parse 1: Memoria insufficiente.\n\n");
		//fwprintf(pParams->fpErrors, L"ERRORE Parse 1: Memoria insufficiente.\n\n");
		retValue = 0;
		goto uscita;
	}
	for ( x = 0; x < MAX_STRING_LENTGTH_IN_CONTENT_STREAM; x++ )
		pParams->myToken.vString[x] = '\0';
		
	pParams->myTokenLengthObj.vString = (char *)malloc( sizeof(char) * MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(char) );
	if ( !(pParams->myTokenLengthObj.vString) )
	{
		snprintf(pParams->szError, 8192, "ERRORE Parse 1: Memoria insufficiente.\n\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE Parse 1: Memoria insufficiente.\n\n");
		//fwprintf(pParams->fpErrors, L"ERRORE Parse 1: Memoria insufficiente.\n\n");
		retValue = 0;
		goto uscita;
	}
	for ( x = 0; x < MAX_STRING_LENTGTH_IN_CONTENT_STREAM; x++ )
		pParams->myTokenLengthObj.vString[x] = '\0';
	*/
	
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
			
			
	//pParams->pEncodingString = (wchar_t*)malloc( sizeof(wchar_t) * MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(wchar_t) );
	//if ( !(pParams->pEncodingString) )
	//{
	//	snprintf(pParams->szError, 8192, "ERRORE Parse 2: Memoria insufficiente.\n\n");
	//	myShowErrorMessage(pParams, pParams->szError, 1);
	//	//wprintf(L"ERRORE Parse 2: Memoria insufficiente.\n\n");
	//	//fwprintf(pParams->fpErrors, L"ERRORE Parse 2: Memoria insufficiente.\n\n");
	//	retValue = 0;
	//	goto uscita;
	//}
	pParams->pUtf8String = (wchar_t*)malloc( sizeof(wchar_t) * MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(wchar_t) );
	if ( !(pParams->pUtf8String) )
	{
		snprintf(pParams->szError, 8192, "ERRORE Parse 2 bis: Memoria insufficiente.\n\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE Parse 2: Memoria insufficiente.\n\n");
		//fwprintf(pParams->fpErrors, L"ERRORE Parse 2: Memoria insufficiente.\n\n");
		retValue = 0;
		goto uscita;
	}
	for ( x = 0; x < MAX_STRING_LENTGTH_IN_CONTENT_STREAM; x++ )
	{
		//pParams->pEncodingString[x] = L'\0';
		pParams->pUtf8String[x] = L'\0';
	}
	
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
		
		pParams->bPdfHasText = 0;
		//pParams->nCountImageContent = 0;
		//pParams->nCountNotImageContent = 0;
		
		pParams->bParsingFontObj = 0;
		pParams->bParsingGsObj = 0;
		
		pParams->xCoordNext = 0.0;
		
		pParams->pReadNextChar = ReadNextChar;
		
		if ( !genhtInit(&pParams->myHT_ImageObjs, GENHT_SIZE, GenStringHashFunc, GenStringCompareFunc) )
		{
			snprintf(pParams->szError, 8192, "ERROR Parse 4 bis: genhtInit failed for pParams->myHT_ImageObjs. File '%s'.\n", pParams->szFileName);
			myShowErrorMessage(pParams, pParams->szError, 1);
			retValue = 0;
			goto successivo;
		}
		
		tstInit(&(pParams->myTST));
						
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
			
			mynumstacklist_Free( &(pParams->myNumStack) );
			
			retValue = 0;
			goto successivo;		
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
				myobjreflist_Init(&(pParams->myObjsTable[k]->myGsRefList));
				
				pParams->myObjsTable[k]->pGlyphsWidths = NULL;
				pParams->myObjsTable[k]->pszDirectFontResourceString = NULL;
				pParams->myObjsTable[k]->lenDirectFontResourceString = 0;
				pParams->myObjsTable[k]->pszDirectGsResourceString = NULL;
				pParams->myObjsTable[k]->lenDirectGsResourceString = 0;
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
		
		// *************************************************************
		//PrintThisObject(pParams, 1317, 1, 0, NULL);
		//wprintf(L"\n**********************************************************************************************************************************\n");
		//PrintThisObject(pParams, 1320, 1, 0, NULL);
		//PrintThisObject(pParams, 1504, 1, 0, NULL);
		//PrintThisObject(pParams, 1505, 1, 0, NULL);
		//PrintThisObject(pParams, 1506, 1, 0, NULL);
		//goto uscita;
		// *************************************************************
		
		#if defined(MAKE_PDFTK_LIBERO_SRC)
		PrintFileProva_Libero(pParams, "../Files/aandy/mylibero/mylibero_src.pdf");
		goto uscita;
		#endif
		
		#if defined(MAKE_PDFTK_LASTAMPA_SRC)
		PrintFileProva_LaStampa(pParams, "../Files/aandy/mylastampa/mylastampa_src.pdf");
		//PrintThisObject(pParams, 34, 0, 0, NULL);
		//PrintThisObject(pParams, 35, 0, 0, NULL);
		//PrintThisObject(pParams, 27, 0, 0, NULL);
		//PrintThisObject(pParams, 28, 0, 0, NULL);
		goto uscita;
		#endif
		
		#if defined(MAKE_PDFTK_ILGIORNALE_SRC)
		PrintFileProva_IlGiornale(pParams, "../Files/aandy/myilgiornale/myilgiornale_src.pdf");
		//PrintThisObject(pParams, 23, 0, 0, NULL);
		//PrintThisObject(pParams, 16, 0, 0, NULL);
		//PrintThisObject(pParams, 50, 0, 0, NULL);
		//PrintThisObject(pParams, 51, 0, 0, NULL);
		goto uscita;
		#endif

		//PrintThisObject(pParams, 1504, 0, 0, NULL);
		//goto uscita;
					
		if ( !ParseObject(pParams, pParams->myPdfTrailer.Root.Number) )
		{
			retValue = 0;
			goto successivo;
		}
				
		//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN) || defined(MYDEBUG_PRINT_COUNT_CONTENT_TYPE)
		//wprintf(L"\n\nTotal number of Image Contents = %u;\nTotal number of content other than Image = %u\n\n", pParams->nCountImageContent, pParams->nCountNotImageContent);
		//#endif
				
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
					myobjreflist_Free(&(pParams->myObjsTable[x]->myGsRefList));
					myintqueuelist_Free(&(pParams->myObjsTable[x]->queueContentsObjRefs));
					
					if ( NULL != pParams->myObjsTable[x]->pGlyphsWidths )
					{
						if ( NULL != pParams->myObjsTable[x]->pGlyphsWidths->pWidths )
						{
							free(pParams->myObjsTable[x]->pGlyphsWidths->pWidths);
							pParams->myObjsTable[x]->pGlyphsWidths->pWidths = NULL;
						}
						
						pParams->myObjsTable[x]->pGlyphsWidths->WidthsArraySize = 0;
						pParams->myObjsTable[x]->pGlyphsWidths->FirstChar = 0;
						pParams->myObjsTable[x]->pGlyphsWidths->LastChar = 0;
						pParams->myObjsTable[x]->pGlyphsWidths->MissingWidth = 0;
						pParams->myObjsTable[x]->pGlyphsWidths->dFontSpaceWidth = -1.0;
						
						free(pParams->myObjsTable[x]->pGlyphsWidths);
						pParams->myObjsTable[x]->pGlyphsWidths = NULL;
					}
					
					if ( NULL != pParams->myObjsTable[x]->pszDirectFontResourceString )
					{
						free(pParams->myObjsTable[x]->pszDirectFontResourceString);
						pParams->myObjsTable[x]->pszDirectFontResourceString = NULL;
						pParams->myObjsTable[x]->lenDirectFontResourceString = 0;
					}
					if ( NULL != pParams->myObjsTable[x]->pszDirectGsResourceString )
					{
						free(pParams->myObjsTable[x]->pszDirectGsResourceString);
						pParams->myObjsTable[x]->pszDirectGsResourceString = NULL;
						pParams->myObjsTable[x]->lenDirectGsResourceString = 0;
					}
					
					free(pParams->myObjsTable[x]);
					pParams->myObjsTable[x] = NULL;
				}
			}
		
			free(pParams->myObjsTable);
			pParams->myObjsTable = NULL;
		}
		
		if ( NULL != pParams->pPagesTree )
		{
			treeFree(pParams->pPagesTree);
			pParams->pPagesTree = NULL;
		}
		
		if ( NULL != pParams->pPagesArray )
		{
			free(pParams->pPagesArray);
			pParams->pPagesArray = NULL;
		}
		
		genhtFree(&(pParams->myHT_ImageObjs));
			
		if ( NULL != pParams->myTST.pRoot )
		{
			tstFreeRecursive(&(pParams->myTST), pParams->myTST.pRoot);
		}

		//if ( NULL != pParams->myRedBlackTree.pRoot )
		//	vlrbtFree(&(pParams->myRedBlackTree));
		
		if ( NULL != pParams->pText )
		{
			free(pParams->pText);
			pParams->pText = NULL;
		}
				
		if ( NULL != pParams->pwszCurrentWord )
		{
			free(pParams->pwszCurrentWord);
			pParams->pwszCurrentWord = NULL;
		}
		
		if ( NULL != pParams->pCodeSpaceRangeArray )
		{
			free(pParams->pCodeSpaceRangeArray);
			pParams->pCodeSpaceRangeArray = NULL;
		}
	
		if ( NULL != pParams->pwszPreviousWord )
		{
			free(pParams->pwszPreviousWord);
			pParams->pwszPreviousWord = NULL;
		}		
		
		if ( NULL != pParams->pszDirectFontResourceString )
		{
			free(pParams->pszDirectFontResourceString);
			pParams->pszDirectFontResourceString = NULL;
		}
		if ( NULL != pParams->pszDirectGsResourceString )
		{
			free(pParams->pszDirectGsResourceString);
			pParams->pszDirectGsResourceString = NULL;
		}
			
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}
		
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
	
	/*
	if ( pParams->myToken.vString != NULL )
	{
		free(pParams->myToken.vString);
		pParams->myToken.vString = NULL;
	}
	
	if ( pParams->myTokenLengthObj.vString != NULL )
	{
		free(pParams->myTokenLengthObj.vString);
		pParams->myTokenLengthObj.vString = NULL;
	}
	*/
	
	if ( pParams->lexemeTemp != NULL )
	{
		free(pParams->lexemeTemp);
		pParams->lexemeTemp = NULL;
	}	
	
	//if ( pParams->pEncodingString != NULL )
	//{
	//	free(pParams->pEncodingString);
	//	pParams->pUtf8String = NULL;
	//}	
	
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
				myobjreflist_Free(&(pParams->myObjsTable[x]->myGsRefList));
				myintqueuelist_Free(&(pParams->myObjsTable[x]->queueContentsObjRefs));
				
				if ( NULL != pParams->myObjsTable[x]->pGlyphsWidths )
				{
					if ( NULL != pParams->myObjsTable[x]->pGlyphsWidths->pWidths )
					{
						free(pParams->myObjsTable[x]->pGlyphsWidths->pWidths);
						pParams->myObjsTable[x]->pGlyphsWidths->pWidths = NULL;
					}
						
					pParams->myObjsTable[x]->pGlyphsWidths->WidthsArraySize = 0;
					pParams->myObjsTable[x]->pGlyphsWidths->FirstChar = 0;
					pParams->myObjsTable[x]->pGlyphsWidths->LastChar = 0;
					pParams->myObjsTable[x]->pGlyphsWidths->MissingWidth = 0;
					pParams->myObjsTable[x]->pGlyphsWidths->dFontSpaceWidth = -1.0;
						
					free(pParams->myObjsTable[x]->pGlyphsWidths);
					pParams->myObjsTable[x]->pGlyphsWidths = NULL;
				}
				
				if ( NULL != pParams->myObjsTable[x]->pszDirectFontResourceString )
				{
					free(pParams->myObjsTable[x]->pszDirectFontResourceString);
					pParams->myObjsTable[x]->pszDirectFontResourceString = NULL;
					pParams->myObjsTable[x]->lenDirectFontResourceString = 0;
				}
				if ( NULL != pParams->myObjsTable[x]->pszDirectGsResourceString )
				{
					free(pParams->myObjsTable[x]->pszDirectGsResourceString);
					pParams->myObjsTable[x]->pszDirectGsResourceString = NULL;
					pParams->myObjsTable[x]->lenDirectGsResourceString = 0;
				}
				
				free(pParams->myObjsTable[x]);
				pParams->myObjsTable[x] = NULL;
			}
		}
		
		free(pParams->myObjsTable);
		pParams->myObjsTable = NULL;
	}
	
	if ( NULL != pParams->pPagesTree )
	{
		treeFree(pParams->pPagesTree);
		pParams->pPagesTree = NULL;
	}
		
	if ( NULL != pParams->pPagesArray )
	{
		free(pParams->pPagesArray);
		pParams->pPagesArray = NULL;
	}
		
	htFree(&(pParams->myCharSetHashTable));
	
	genhtFree(&(pParams->myHT_ImageObjs));
		
	if ( NULL != pParams->myTST.pRoot )
	{
		tstFreeRecursive(&(pParams->myTST), pParams->myTST.pRoot);
	}
	
	//if ( NULL != pParams->myRedBlackTree.pRoot )
	//	vlrbtFree(&(pParams->myRedBlackTree));
	
	if ( NULL != pParams->pText )
	{
		free(pParams->pText);
		pParams->pText = NULL;
	}
		
	if ( NULL != pParams->pCodeSpaceRangeArray )
	{
		free(pParams->pCodeSpaceRangeArray);
		pParams->pCodeSpaceRangeArray = NULL;
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
	
	if ( NULL != pParams->pszDirectFontResourceString )
	{
		free(pParams->pszDirectFontResourceString);
		pParams->pszDirectFontResourceString = NULL;
	}
	if ( NULL != pParams->pszDirectGsResourceString )
	{
		free(pParams->pszDirectGsResourceString);
		pParams->pszDirectGsResourceString = NULL;
	}
			
	//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
	//{
	//	if ( NULL != pParams->myToken.vString )
	//	{
	//		free(pParams->myToken.vString);
	//		pParams->myToken.vString = NULL;
	//	}
	//}
	
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

void MultiplyTransMatrix(TransMatrix *pA, TransMatrix *pB, TransMatrix *pRes)
{
	// [Aa Ab 0]   [Ba Bb 0]   [aNew bNew 0]
	// [Ac Ad 0] X [Bc Bd 0] = [cNew dNew 0]
	// [Ae Af 1]   [Be Bf 1]   [eNew fNew 1]
			
	// aNew  = (Aa * Ba) + (Ab * Bc) + (0 * Be)
	// bNew  = (Aa * Bb) + (Ab * Bd) + (0 * Bf)
	// 0     = (Aa * 0 ) + (Ab * 0 ) + (0 * 1 )
	// cNew  = (Ac * Ba) + (Ad * Bc) + (0 * Be)
	// dNew  = (Ac * Bb) + (Ad * Bd) + (0 * Bf)
	// 0     = (Ac * 0 ) + (Ad * 0 ) + (0 * 1 )
	// eNew  = (Ae * Ba) + (Af * Bc) + (1 * Be)
	// fNew  = (Ae * Bb) + (Af * Bd) + (1 * Bf)
	// 1     = (Ae * 0 ) + (Af * 0 ) + (1 * 1 )
	
	pRes->a = (pA->a * pB->a) + (pA->b * pB->c);
	pRes->b = (pA->a * pB->b) + (pA->b * pB->d);

	pRes->c = (pA->c * pB->a) + (pA->d * pB->c);
	pRes->d = (pA->c * pB->b) + (pA->d * pB->d);

	pRes->e = (pA->e * pB->a) + (pA->f * pB->c) + pB->e;
	pRes->f = (pA->e * pB->b) + (pA->f * pB->d) + pB->f;
}

int VlRbtCompareFuncCol(const void* pKey1, uint32_t keysize1, const void* pKey2, uint32_t keysize2)
{
	UNUSED(keysize1);
	UNUSED(keysize2);
	
	vlrbtKey_t *pMyKey1 = (vlrbtKey_t*)pKey1;
	vlrbtKey_t *pMyKey2 = (vlrbtKey_t*)pKey2;
		
	if ( pMyKey1->col < pMyKey2->col )
	{
		return -1;
	}
	else if ( pMyKey1->col > pMyKey2->col )
	{
		return 1;
	}
	else // pMyKey1->col == pMyKey2->col; CONFRONTIAMO LE RIGHE.
	{
		if ( pMyKey1->row > pMyKey2->row )
		{
			//return 1;
			return -1;   // Perché le righe vanno dal basso in alto
		}
		else if ( pMyKey1->row < pMyKey2->row )
		{
			//return -1;
			return 1;    // Perché le righe vanno dal basso in alto
		}
		else // pMyKey1->row == pMyKey2->row; CONFRONTIAMO L'ordine di lettura.
		{
			if ( pMyKey1->ord > pMyKey2->ord )
			{
				return 1;
			}
			else if ( pMyKey1->ord < pMyKey2->ord )
			{
				return -1;
			}
			else
			{
				return 0;
			}
		}
	}
}

// https://scfbm.biomedcentral.com/articles/10.1186/1751-0473-7-7

int VlRbtCompareFuncRow(const void* pKey1, uint32_t keysize1, const void* pKey2, uint32_t keysize2)
{
	UNUSED(keysize1);
	UNUSED(keysize2);
	
	vlrbtKey_t *pMyKey1 = (vlrbtKey_t*)pKey1;
	vlrbtKey_t *pMyKey2 = (vlrbtKey_t*)pKey2;
		
	if ( pMyKey1->row > pMyKey2->row )
	{
		//return 1;
		return -1;   // Perché le righe vanno dal basso in alto
	}
	else if ( pMyKey1->row < pMyKey2->row )
	{
		//return -1;
		return 1;    // Perché le righe vanno dal basso in alto
	}
	else // pMyKey1->row == pMyKey2->row; CONFRONTIAMO LE COLONNE.
	{
		if ( pMyKey1->col < pMyKey2->col )
		{
			return -1;
		}
		else if ( pMyKey1->col > pMyKey2->col )
		{
			return 1;
		}
		else // pMyKey1->col == pMyKey2->col; CONFRONTIAMO L'ordine di lettura.
		{
			if ( pMyKey1->ord > pMyKey2->ord )
			{
				return 1;
			}
			else if ( pMyKey1->ord < pMyKey2->ord )
			{
				return -1;
			}
			else
			{
				return 0;
			}
		}
	}
}

int VlRbtCompareFuncOrd(const void* pKey1, uint32_t keysize1, const void* pKey2, uint32_t keysize2)
{
	UNUSED(keysize1);
	UNUSED(keysize2);
	
	vlrbtKey_t *pMyKey1 = (vlrbtKey_t*)pKey1;
	vlrbtKey_t *pMyKey2 = (vlrbtKey_t*)pKey2;
		
	if ( pMyKey1->ord > pMyKey2->ord )
	{
		return 1;
	}
	else if ( pMyKey1->ord < pMyKey2->ord )
	{
		return -1;
	}
	else // pMyKey1->ord == pMyKey2->ord; CONFRONTIAMO LE COLONNE.
	{
		if ( pMyKey1->col < pMyKey2->col )
		{
			return -1;
		}
		else if ( pMyKey1->col > pMyKey2->col )
		{
			return 1;
		}
		else // pMyKey1->col == pMyKey2->col; CONFRONTIAMO LE RIGHE.
		{
			if ( pMyKey1->row > pMyKey2->row )
			{
				return -1; // Perché le righe vanno dal basso in alto
			}
			else if ( pMyKey1->row < pMyKey2->row )
			{
				return 1; // Perché le righe vanno dal basso in alto
			}
			else
			{
				return 0;
			}
		}
	}
}

/*
int VlRbtOnTraverseFuncNew(void* pCurrNode)
{
	double xDiff;
	
	//double xCoordNext = 0.0;
	//double yCoordNext = 0.0;
	
	wchar_t prevChar;
	double prevRow;
	double prevCol;
	//double prevOrd;
	//double prevWidth;
	//double prevWidthScaled;
	double prevXCoordNext;
	//double prevYCoordNext;	

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
	wchar_t nextChar;
	#endif
	double nextRow;
	double nextCol;
	//double nextOrd;
	//double nextWidth;
	//double nextWidthScaled;
	//double nextXCoordNext;
	//double nextYCoordNext;	
	
	vlrbtTreeNode *pPredecessor;
	vlrbtTreeNode *pSuccessor;
		
	vlrbtTreeNode *pMyCurrNode = (vlrbtTreeNode*)pCurrNode;
	
	vlrbtKey_t *pMyKey = (vlrbtKey_t*)pMyCurrNode->pKey;
	vlrbtData_t *pMyData = (vlrbtData_t*)pMyCurrNode->pData;

	vlrbtKey_t *pMyKeyPredecessor = NULL;
	vlrbtData_t *pMyDataPredecessor = NULL;
	
	vlrbtKey_t *pMyKeySuccessor = NULL;
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
	vlrbtData_t *pMyDataSuccessor = NULL;
	#endif
		
	vlrbtTreePredecessor(pCurrNode, &pPredecessor);
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
	vlrbtTreeSuccessor(pCurrNode, &pSuccessor);
	#endif
		
	if ( NULL != pPredecessor )
	{
		pMyKeyPredecessor = (vlrbtKey_t*)pPredecessor->pKey;
		pMyDataPredecessor = (vlrbtData_t*)pPredecessor->pData;

		prevChar = pMyDataPredecessor->c;
		prevRow = pMyKeyPredecessor->row;
		prevCol = pMyKeyPredecessor->col;
		//prevOrd = pMyKeyPredecessor->ord;
		//prevWidth = pMyDataPredecessor->width;
		//prevWidthScaled = pMyDataPredecessor->widthScaled;
		prevXCoordNext = pMyDataPredecessor->xCoordNext;
		//prevYCoordNext = pMyDataPredecessor->yCoordNext;
	}
	else
	{
		prevChar = L' ';
		prevRow = 0;
		prevCol = 0;
		//prevOrd = 0;
		//prevWidth = 0;
		//prevWidthScaled = 0;
		prevXCoordNext = 0;
		//prevYCoordNext = 0;
	}
	
	if ( NULL != pSuccessor )
	{
		pMyKeySuccessor = (vlrbtKey_t*)pSuccessor->pKey;
		//pMyDataSuccessor = (vlrbtData_t*)pSuccessor->pData;

		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
		nextChar = pMyDataSuccessor->c;
		#endif
		nextRow = pMyKeySuccessor->row;
		nextCol = pMyKeySuccessor->col;
		//nextOrd = pMyKeySuccessor->ord;
		//nextWidth = pMyDataSuccessor->width;
		//nextWidthScaled = pMyDataSuccessor->widthScaled;
		//nextXCoordNext = pMyDataSuccessor->xCoordNext;
		//nextYCoordNext = pMyDataSuccessor->yCoordNext;
	}
	else
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
		nextChar = L'§';
		#endif
		nextRow = 0;
		nextCol = 0;
		//nextOrd = 0;
		//nextWidth = 0;
		//nextWidthScaled = 0;
		//nextXCoordNext = 0;
		//nextYCoordNext = 0;
	}
		
	//if ( (NULL != pSuccessor) && (pMyKey->row == nextRow) && (pMyKey->col == nextCol) )
	//if ( (NULL != pSuccessor) && (0.0 == nextRow - pMyKey->row) && (0.0 == nextCol - pMyKey->col) )
	if ( nextRow - pMyKey->row < 0.000000001 && nextCol - pMyKey->col < 0.000000001 && L' ' == pMyData->c )
	{
		pMyData->xCoordNext = pMyDataPredecessor->xCoordNext;
		goto uscita;
	}
	
	if ( pMyKey->row - prevRow < 0.000000001 && pMyKey->col - prevCol < 0.000000001 && L' ' == pMyData->c )
	{
		pMyData->xCoordNext = pMyDataPredecessor->xCoordNext;
		goto uscita;
	}	
	
	if ( pMyKey->row == prevRow )
	{
		xDiff = pMyKey->col - prevXCoordNext;
	}
	else
	{
		if ( prevRow > 0.0 )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
			wprintf(L"VADO A CAPO: pMyKey->row = %f; prevRow = %f\n", pMyKey->row, prevRow);
			#endif
		
			if ( NULL != pMyData->pParams->fpOutput )	
				fwprintf(pMyData->pParams->fpOutput, L"\n");
			else
				wprintf(L"\n");
		}
		
		xDiff = 0.0;
	}
	
	if ( xDiff < 0.0 && L' ' == pMyData->c )
		goto uscita;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
	wprintf(L"[%lc](row = %f; col = %f; ord = %d); width = [%f Scaled %f]; nextXCoord = %f; prevChar = '%lc'; nextChar = '%lc'\n", pMyData->c, pMyKey->row, pMyKey->col, pMyKey->ord, pMyData->width, pMyData->widthScaled, pMyData->xCoordNext, prevChar, nextChar);
	wprintf(L"\tFontSpaceWidth = [%f Scaled %f]; pMyData->currFontSize = %f\n", pMyData->currFontSpaceWidth, pMyData->currFontSpaceWidthScaled, pMyData->currFontSize);
	wprintf(L"\txDiff = %f -> pMyKey->col(%f) - prevXCoordNext(%f)\n", xDiff, pMyKey->col, prevXCoordNext);
	#endif
		
	if ( xDiff > pMyData->currFontSpaceWidth && pMyData->c != L' ' && prevChar != L' ' )
	//if ( xDiff > pMyData->CurrFontSpaceWidthScaled && pMyData->c != L' ' && prevChar != L' ' )
	{	
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
		wprintf(L"\t***** INSERISCO SPAZIO PRIMA DI STAMPARE IL CARATTERE '%lc' -> xDiff(%f) >= dCurrFontSpaceWidth(%f)\n", pMyData->c, xDiff, pMyData->currFontSpaceWidth);
		//wprintf(L"\t***** INSERISCO SPAZIO PRIMA DI STAMPARE IL CARATTERE '%lc' -> xDiff(%f) >= dFontSpaceWidthScaled(%f)\n", pMyData->c, xDiff, pMyData->currFontSpaceWidthScaled);
		#endif
							
		if ( NULL != pMyData->pParams->fpOutput )
			fwprintf(pMyData->pParams->fpOutput, L" ");
		else
			wprintf(L" ");
	}
									
	if ( NULL != pMyData->pParams->fpOutput )	
		fwprintf(pMyData->pParams->fpOutput, L"%lc", pMyData->c);
	else
		wprintf(L"%lc", pMyData->c);
		
	//pMyData->prevChar = pMyData->c;
	//pMyData->prevRow = pMyKey->row;
	//pMyData->prevCol = pMyKey->col;
	//pMyData->prevWidth = pMyData->width;
	//pMyData->prevWidthScaled = pMyData->widthScaled;
	
	uscita:
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
	wprintf(L"\n");
	#endif
	
	return 1;
}
*/

int VlRbtOnTraverseFunc(void* pCurrNode)
{
	double xDiff;
	
	double dFontSpaceWidth;
	//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
	double dFontSpaceWidthScaled;
	double dFontSpaceWidthPrevScaled;
	//#endif
	double dFontSpaceWidthCurr;
	double dFontSpaceWidthPrev;
	
	
	//double xCoordNext = 0.0;
	//double yCoordNext = 0.0;
	
	wchar_t prevChar;
	double prevRow;
	double prevXCoordNext;
	
	vlrbtTreeNode *pPredecessor;
	
	vlrbtTreeNode *pMyCurrNode = (vlrbtTreeNode*)pCurrNode;
	
	vlrbtKey_t *pMyKey = (vlrbtKey_t*)pMyCurrNode->pKey;
	vlrbtData_t *pMyData = (vlrbtData_t*)pMyCurrNode->pData;
	
	vlrbtKey_t *pMyKeyPredecessor = NULL;
	vlrbtData_t *pMyDataPredecessor = NULL;
	
	vlrbtTreePredecessor(pCurrNode, &pPredecessor);
		
	if ( NULL != pPredecessor )
	{
		pMyKeyPredecessor = (vlrbtKey_t*)pPredecessor->pKey;
		pMyDataPredecessor = (vlrbtData_t*)pPredecessor->pData;

		prevChar = pMyDataPredecessor->c;
		prevRow = pMyKeyPredecessor->row;
		//prevCol = pMyKeyPredecessor->col;
		//prevOrd = pMyKeyPredecessor->ord;
		//prevWidth = pMyDataPredecessor->width;
		//prevWidthScaled = pMyDataPredecessor->widthScaled;
		prevXCoordNext = pMyDataPredecessor->xCoordNext;
		//prevYCoordNext = pMyDataPredecessor->yCoordNext;
		dFontSpaceWidthPrev = pMyDataPredecessor->currFontSpaceWidth;
		//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
		dFontSpaceWidthPrevScaled = pMyDataPredecessor->currFontSpaceWidthScaled;
		//#endif
	}
	else
	{
		prevChar = L' ';
		prevRow = 0;
		//prevCol = 0;
		//prevOrd = 0;
		//prevWidth = 0;
		//prevWidthScaled = 0;
		prevXCoordNext = 0;
		//prevYCoordNext = 0;
		dFontSpaceWidthPrev = pMyData->currFontSpaceWidth;
		//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
		dFontSpaceWidthPrevScaled = pMyData->currFontSpaceWidthScaled;
		//#endif
	}
	
	dFontSpaceWidthCurr = pMyData->currFontSpaceWidth;
	
	if ( dFontSpaceWidthCurr != dFontSpaceWidthPrev )
	{
		dFontSpaceWidth = (dFontSpaceWidthCurr + dFontSpaceWidthPrev) * 0.5;
		//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
		dFontSpaceWidthScaled = (pMyData->currFontSpaceWidthScaled + dFontSpaceWidthPrevScaled) * 0.5; 
		//#endif
	}
	else
	{
		dFontSpaceWidth = dFontSpaceWidthCurr;
		//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
		dFontSpaceWidthScaled = pMyData->currFontSpaceWidthScaled;
		//#endif
	}
	
	//dFontSpaceWidth = dFontSpaceWidthCurr;
	
	//if ( 1.0 != pMyData->currFontSize )
	if ( (1.0 != pMyData->currFontSize) && (FONT_SUBTYPE_Type0 != pMyData->nCurrentFontSubtype) )
		dFontSpaceWidth = dFontSpaceWidthScaled;
					
	if ( pMyKey->row == prevRow )
	{
		//xDiff = pMyKey->col - (pMyData->prevCol + pMyData->prevWidthScaled);
		xDiff = pMyKey->col - prevXCoordNext;
	}
	else
	{
		if ( prevRow > 0.0 )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
			wprintf(L"VADO A CAPO: pMyKey->row = %f; pMyData->prevRow = %f\n", pMyKey->row, prevRow);
			#endif
		
			//if ( NULL != pMyData->fpOutput )	
			//	fwprintf(pMyData->fpOutput, L"\n");
			//else
			//	wprintf(L"\n");
				
			pMyData->pParams->pText[pMyData->pParams->TextLength] = L'\n';
			pMyData->pParams->TextLength++;
		}
		
		xDiff = 0.0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
	wprintf(L"[%lc](row = %f; col = %f; ord = %d); width = [%f Scaled %f]; nextXCoord = %f\n", pMyData->c, pMyKey->row, pMyKey->col, pMyKey->ord, pMyData->width, pMyData->widthScaled, pMyKey->col + pMyData->widthScaled);
	wprintf(L"\tdFontSpaceWidth = [%f; Scaled = %f]; currFontSpaceWidth = %f; prevFontSpaceWidth = %f; FontSize = %f\n", dFontSpaceWidth, dFontSpaceWidthScaled, dFontSpaceWidthCurr, dFontSpaceWidthPrev, pMyData->currFontSize);
	wprintf(L"\txDiff = %f; pMyKey->col = %f; prevXCoordNext = %f\n", xDiff, pMyKey->col, prevXCoordNext);
	#endif
		
	if ( xDiff > dFontSpaceWidth && pMyData->c != L' ' && prevChar != L' ' )
	//if ( xDiff > dFontSpaceWidthScaled && pMyData->c != L' ' && prevChar != L' ' )
	//if ( xDiff > pMyData->currFontSpaceWidth && pMyData->c != L' ' && prevChar != L' ' )
	//if ( xDiff > pMyData->currFontSpaceWidthScaled && pMyData->c != L' ' && prevChar != L' ' )
	//if ( xDiff > (pMyData->currFontSize/2.0) && pMyData->c != L' ' && prevChar != L' ' )
	{	
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD3)
		wprintf(L"\t***** INSERISCO SPAZIO PRIMA DI STAMPARE IL CARATTERE '%lc' -> xDiff(%f) >= dFontSpaceWidth(%f)\n", pMyData->c, xDiff, dFontSpaceWidth);
		//wprintf(L"\t***** INSERISCO SPAZIO PRIMA DI STAMPARE IL CARATTERE '%lc' -> xDiff(%f) >= dFontSpaceWidthScaled(%f)\n", pMyData->c, xDiff, dFontSpaceWidthScaled);
		//wprintf(L"\t***** INSERISCO SPAZIO PRIMA DI STAMPARE IL CARATTERE '%lc' -> xDiff(%f) >= dCurrFontSpaceWidth(%f)\n", pMyData->c, xDiff, pMyData->currFontSpaceWidth);
		//wprintf(L"\t***** INSERISCO SPAZIO PRIMA DI STAMPARE IL CARATTERE '%lc' -> xDiff(%f) >= dFontSpaceWidthScaled(%f)\n", pMyData->c, xDiff, pMyData->currFontSpaceWidthScaled);
		//wprintf(L"\t***** INSERISCO SPAZIO PRIMA DI STAMPARE IL CARATTERE '%lc' -> xDiff(%f) >= pMyData->currFontSize(%f) pMyData->currFontSize()/2.0 = %f\n", pMyData->c, xDiff, pMyData->currFontSize, pMyData->currFontSize/2.0);
		#endif
							
		//if ( NULL != pMyData->fpOutput )
		//	fwprintf(pMyData->fpOutput, L" ");
		//else
		//	wprintf(L" ");
			
		pMyData->pParams->pText[pMyData->pParams->TextLength] = L' ';
		pMyData->pParams->TextLength++;
	}
									
	//if ( NULL != pMyData->fpOutput )	
	//	fwprintf(pMyData->fpOutput, L"%lc", pMyData->c);
	//else
	//	wprintf(L"%lc", pMyData->c);
		
	pMyData->pParams->pText[pMyData->pParams->TextLength] = pMyData->c;
	pMyData->pParams->TextLength++;
		
	//pMyData->prevChar = pMyData->c;
	//pMyData->prevRow = pMyKey->row;
	//pMyData->prevCol = pMyKey->col;
	//pMyData->prevWidth = pMyData->width;
	//pMyData->prevWidthScaled = pMyData->widthScaled;
	
	return 1;
}

double getCurrCharWidth(Params *pParams, wchar_t c)
{
	double dblCurrCharWidth = -1.0;
	
	if (FONT_SUBTYPE_Type0 != pParams->nCurrentFontSubtype )
	{
		//if ( cEncoding >= pParams->pCurrFontGlyphsWidths->FirstChar && cEncoding <= pParams->pCurrFontGlyphsWidths->LastChar )
		//	dblCurrCharWidth = pParams->pCurrFontGlyphsWidths->pWidths[cEncoding - pParams->pCurrFontGlyphsWidths->FirstChar]/1000.0;
		if ( c >= pParams->pCurrFontGlyphsWidths->FirstChar && c <= pParams->pCurrFontGlyphsWidths->LastChar )
		{
			dblCurrCharWidth = pParams->pCurrFontGlyphsWidths->pWidths[c - pParams->pCurrFontGlyphsWidths->FirstChar];
		}
		else
		{
			dblCurrCharWidth = pParams->pCurrFontGlyphsWidths->MissingWidth;
		}
	}
	else
	{
		if ( pParams->bWisPresent )
		{
			//if ( cEncoding >= pParams->pCurrFontGlyphsWidths->FirstChar && cEncoding <= pParams->pCurrFontGlyphsWidths->LastChar )
			//	dblCurrCharWidth = pParams->pCurrFontGlyphsWidths->pWidths[cEncoding - pParams->pCurrFontGlyphsWidths->FirstChar]/1000.0;
			if ( c >= pParams->pCurrFontGlyphsWidths->FirstChar && c <= pParams->pCurrFontGlyphsWidths->LastChar )
			{
				dblCurrCharWidth = pParams->pCurrFontGlyphsWidths->pWidths[c - pParams->pCurrFontGlyphsWidths->FirstChar];
			}
			else
			{
				dblCurrCharWidth = pParams->pCurrFontGlyphsWidths->MissingWidth;
			}
		}
		else
		{
			dblCurrCharWidth = pParams->pCurrFontGlyphsWidths->DW;
		}
	}
	
	if ( dblCurrCharWidth <= 0.0 )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD2)
		wprintf(L"ATTENZIONE: Current Char Width = %f; Lo imposto a pParams->pCurrFontGlyphsWidths->MissingWidth(%f) = %f\n", dblCurrCharWidth, pParams->pCurrFontGlyphsWidths->MissingWidth, pParams->pCurrFontGlyphsWidths->MissingWidth);
		wprintf(L"\tIn alternativa potrei NON impostarlo a pParams->pCurrFontGlyphsWidths->DW(%f) = %f\n", pParams->pCurrFontGlyphsWidths->DW, pParams->pCurrFontGlyphsWidths->DW);
		#endif
			
		if ( FONT_SUBTYPE_Type0 != pParams->nCurrentFontSubtype )
		{
			dblCurrCharWidth = pParams->pCurrFontGlyphsWidths->MissingWidth;
		}
		else
		{
			if ( pParams->bWisPresent )
			{
				dblCurrCharWidth = pParams->pCurrFontGlyphsWidths->MissingWidth;
			}
			else
			{
				dblCurrCharWidth = pParams->pCurrFontGlyphsWidths->DW;
			}
		}
	}
	
	return dblCurrCharWidth;
}

int ManageShowTextOperator(Params *pParams, const char *szOpName, wchar_t *pszString, size_t lenString)
{									
	double xCoordNext = 0.0;
	double yCoordNext = 0.0;
	
	double xCoordCurrent;
	double yCoordCurrent;
	
	double Tx = 0.0;
	double Ty = 0.0;
		
	TransMatrix tempTextMatrixA;
	TransMatrix tempTextMatrixB;
	
	TransMatrix UserSpaceMatrix;
	
	//wchar_t cEncoding;
	wchar_t c;
	double dblCurrCharWidth;
	
	size_t i = 0;
		
	//vlrbtKey_t myRedBlackTreeKey;
	//vlrbtData_t myRedBlackTreeData;
	

	
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR) && !defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD)
	UNUSED(szOpName);
	#endif

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR)
	wprintf(L"\nOPERATOR %s:\n", szOpName);
	wprintf(L"\tSTRING pszString = (%ls)\n", pszString);
	wprintf(L"\tTextState      -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
			pParams->dsTextState.Tc,
			pParams->dsTextState.Tw,
			pParams->dsTextState.Th,
			pParams->dsTextState.Tl,
			pParams->dsTextState.Tfs,
			pParams->dsTextState.Trise);
	if ( NULL != pParams->pCurrFontGlyphsWidths )
	{
		wprintf(L"\tGlyphsWidths -> FirstChar = %d; LastChar = %d; WidthsArraySize = %d; MissingWidth = %f; DW = %f; dFontSpaceWidth = %f; pParams->dCurrFontSpaceWidth = %f; WritingMode = %d\n",
				pParams->pCurrFontGlyphsWidths->FirstChar,
				pParams->pCurrFontGlyphsWidths->LastChar,
				pParams->pCurrFontGlyphsWidths->WidthsArraySize,
				pParams->pCurrFontGlyphsWidths->MissingWidth,
				pParams->pCurrFontGlyphsWidths->DW,
				pParams->pCurrFontGlyphsWidths->dFontSpaceWidth,
				pParams->dCurrFontSpaceWidth,
				pParams->pCurrFontGlyphsWidths->WritingMode);
	}
	else
	{
		wprintf(L"\tGlyphsWidths -> NULL\n");
	}
	wprintf(L"\tCTM            -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
			pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
			pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
			pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
			pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
			pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
			pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
	wprintf(L"\tTextMatrix     -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
			pParams->dsTextMatrix.a,
			pParams->dsTextMatrix.b,
			pParams->dsTextMatrix.c,
			pParams->dsTextMatrix.d,
			pParams->dsTextMatrix.e,
			pParams->dsTextMatrix.f);
	wprintf(L"\tLineMatrix     -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
			pParams->dsLineMatrix.a,
			pParams->dsLineMatrix.b,
			pParams->dsLineMatrix.c,
			pParams->dsLineMatrix.d,
			pParams->dsLineMatrix.e,
			pParams->dsLineMatrix.f);
	#endif
	
	pParams->dCurrFontSpaceWidthScaled = 0.0;
					
	for ( i = 0; i < lenString; i++ )
	{
		//wprintf(L"i = %d; lenString = %d\n");
		c = pszString[i];
		//wprintf(L"\tc = '%lc'\n");
						
		//cEncoding = pParams->pEncodingString[i];
		
		// **************************************************************************************************************************
		if ( pParams->Tj != 0.0 )
		{
			dblCurrCharWidth = -1.0;
					
			//dblCurrCharWidth = getCurrCharWidth(pParams, cEncoding);
			dblCurrCharWidth = getCurrCharWidth(pParams, pParams->cLastChar);
																							
			
			
			if ( WRITING_MODE_HORIZONTAL == pParams->pCurrFontGlyphsWidths->WritingMode )
			{
				Ty = 0.0;
				Tx = ((dblCurrCharWidth - pParams->Tj) * pParams->dsTextState.Tfs + pParams->dsTextState.Tc + pParams->dsTextState.Tw) * pParams->dsTextState.Th;
					
				//if ( L' ' == pParams->cLastChar )
				//	pParams->dCurrFontSpaceWidth = Tx;
			}
			else
			{
				Tx = 0.0;
				Ty = (dblCurrCharWidth - pParams->Tj) * pParams->dsTextState.Tfs + pParams->dsTextState.Tc + pParams->dsTextState.Tw;
					
				//if ( L' ' == pParams->cLastChar )
				//	pParams->dCurrFontSpaceWidth = Ty;
			}
			
			
			
			tempTextMatrixA.a = 1;
			tempTextMatrixA.b = 0;
			tempTextMatrixA.c = 0;
			tempTextMatrixA.d = 1;
			if ( WRITING_MODE_HORIZONTAL == pParams->pCurrFontGlyphsWidths->WritingMode )
			{
				tempTextMatrixA.e = Tx - pParams->dLastCharWidth;
				tempTextMatrixA.f = Ty;
			}
			else
			{
				tempTextMatrixA.e = Tx;
				tempTextMatrixA.f = Ty - pParams->dLastCharWidth;
			}
					
			tempTextMatrixB.a = pParams->dsTextMatrix.a;
			tempTextMatrixB.b = pParams->dsTextMatrix.b;
			tempTextMatrixB.c = pParams->dsTextMatrix.c;
			tempTextMatrixB.d = pParams->dsTextMatrix.d;
			tempTextMatrixB.e = pParams->dsTextMatrix.e;
			tempTextMatrixB.f = pParams->dsTextMatrix.f;
					
			MultiplyTransMatrix(&tempTextMatrixA, &tempTextMatrixB, &(pParams->dsTextMatrix));
								
			//pParams->Tj = 0.0;
		}
		// **************************************************************************************************************************
									
		tempTextMatrixA.a = pParams->dsTextState.Tfs * pParams->dsTextState.Th;
		tempTextMatrixA.b = 0;
		tempTextMatrixA.c = 0;
		tempTextMatrixA.d = pParams->dsTextState.Tfs;
		tempTextMatrixA.e = 0;
		tempTextMatrixA.f = pParams->dsTextState.Trise;
					
		MultiplyTransMatrix(&tempTextMatrixA, &(pParams->dsTextMatrix), &UserSpaceMatrix);
		
		MultiplyTransMatrix(&tempTextMatrixA, &(pParams->dsTextMatrix), &tempTextMatrixB);
		MultiplyTransMatrix(&tempTextMatrixB, &(pParams->dCTM_Stack[pParams->nCTM_StackTop]), &(pParams->dsRenderingMatrix));
	
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR)
		wprintf(L"\n\tRenderingMatrix -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
				pParams->dsRenderingMatrix.a,
				pParams->dsRenderingMatrix.b,
				pParams->dsRenderingMatrix.c,
				pParams->dsRenderingMatrix.d,
				pParams->dsRenderingMatrix.e,
				pParams->dsRenderingMatrix.f);
		#endif
	
		xCoordCurrent = pParams->dsRenderingMatrix.e;
		yCoordCurrent = pParams->dsRenderingMatrix.f;		
		//xCoordCurrent = UserSpaceMatrix.e;
		//yCoordCurrent = UserSpaceMatrix.f;
						
		dblCurrCharWidth = -1.0;
			
		//dblCurrCharWidth = getCurrCharWidth(pParams, cEncoding);
		dblCurrCharWidth = getCurrCharWidth(pParams, c);
																													
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_ENCODING)
		wprintf(L"i = %d, c = '%lc', Value = %d; cEncoding = '%lc', Value = %d; dblCurrCharWidth = %f\n",
				i, c, c, cEncoding, cEncoding, dblCurrCharWidth);
		#endif
												
		//pParams->dCurrFontSpaceWidth += pParams->dsTextState.Tw;
		

		// https://stackoverflow.com/questions/57988812/exploring-horizontal-parsing-tj-in-pdfdetail-understanding-of-tx-formula

		if ( WRITING_MODE_HORIZONTAL == pParams->pCurrFontGlyphsWidths->WritingMode )
		{
			Ty = 0.0;
			//Tx = ((dblCurrCharWidth - pParams->Tj) * pParams->dsTextState.Tfs + pParams->dsTextState.Tc + pParams->dsTextState.Tw) * pParams->dsTextState.Th;
			Tx = (dblCurrCharWidth * pParams->dsTextState.Tfs + pParams->dsTextState.Tc + pParams->dsTextState.Tw) * pParams->dsTextState.Th;
					
			//if ( L' ' == c )
			//	pParams->dCurrFontSpaceWidth = Tx;
		}
		else
		{
			Tx = 0.0;
			//Ty = (dblCurrCharWidth - pParams->Tj) * pParams->dsTextState.Tfs + pParams->dsTextState.Tc + pParams->dsTextState.Tw;
			Ty = dblCurrCharWidth * pParams->dsTextState.Tfs + pParams->dsTextState.Tc + pParams->dsTextState.Tw;
				
			//if ( L' ' == c )
			//	pParams->dCurrFontSpaceWidth = Ty;
		}
		
		
											
		if ( pParams->dCurrFontSpaceWidth < 0.0 )
			pParams->dCurrFontSpaceWidth = 0.0;


				
		//pParams->dCurrFontAvgWidthScaled = pParams->dCurrFontAvgWidth * pParams->dsRenderingMatrix.a;
		//pParams->dCurrFontMaxWidthScaled = pParams->dCurrFontMaxWidth * pParams->dsRenderingMatrix.a;
		//pParams->dCurrFontSpaceWidthScaled = pParams->dCurrFontSpaceWidth * pParams->dsRenderingMatrix.a;
		if ( pParams->dsTextState.Tfs != 1.0 )
		{
			pParams->dCurrFontAvgWidthScaled = (pParams->dCurrFontAvgWidth * pParams->dsTextState.Tfs + pParams->dsTextState.Tc + pParams->dsTextState.Tw) * pParams->dsTextState.Th;
			pParams->dCurrFontMaxWidthScaled = (pParams->dCurrFontMaxWidth * pParams->dsTextState.Tfs + pParams->dsTextState.Tc + pParams->dsTextState.Tw) * pParams->dsTextState.Th;
			pParams->dCurrFontSpaceWidthScaled = (pParams->dCurrFontSpaceWidth * pParams->dsTextState.Tfs + pParams->dsTextState.Tc + pParams->dsTextState.Tw) * pParams->dsTextState.Th;
			//pParams->dCurrFontSpaceWidthScaled *= (pParams->dsTextState.Tfs/((pParams->dCurrFontSpaceWidth * pParams->dsTextState.Tfs + pParams->dsTextState.Tw) * pParams->dsTextState.Th));
			//pParams->dCurrFontSpaceWidthScaled *= (pParams->dsTextState.Tfs/pParams->dsRenderingMatrix.a);
			
			xCoordNext = pParams->dsRenderingMatrix.e + Tx;
			yCoordNext = pParams->dsRenderingMatrix.f + Ty;
			//xCoordNext = UserSpaceMatrix.e + Tx;
			//yCoordNext = UserSpaceMatrix.f + Ty;	
			
			//pParams->dCurrFontSpaceWidth = pParams->dCurrFontSpaceWidthScaled;
		}
		else
		{	
			pParams->dCurrFontAvgWidthScaled = pParams->dCurrFontAvgWidth * pParams->dsRenderingMatrix.a;
			pParams->dCurrFontMaxWidthScaled = pParams->dCurrFontMaxWidth * pParams->dsRenderingMatrix.a;
			pParams->dCurrFontSpaceWidthScaled = pParams->dCurrFontSpaceWidth * pParams->dsRenderingMatrix.a;
						
			xCoordNext = pParams->dsRenderingMatrix.e + (Tx * pParams->dsRenderingMatrix.a);
			yCoordNext = pParams->dsRenderingMatrix.f + (Ty * pParams->dsRenderingMatrix.a);
			//xCoordNext = UserSpaceMatrix.e + (Tx * UserSpaceMatrix.a);
			//yCoordNext = UserSpaceMatrix.f + (Ty * UserSpaceMatrix.a);
		}
		
		//xCoordNext = UserSpaceMatrix.e + Tx;
		//yCoordNext = UserSpaceMatrix.f + Ty;
		
		
						
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD)
		wprintf(L"\t%s: %lc -> TextX = %f; TextY = %f; <> Tx = %f; CurrCharWidth = %f; MissingWidth = %f\n",
				szOpName,
				c,
				pParams->dsTextMatrix.e,
				pParams->dsTextMatrix.f,
				Tx,
				dblCurrCharWidth,
				pParams->pCurrFontGlyphsWidths->MissingWidth);
		wprintf(L"\txCoordNext = %f; FontSpaceWidth = %f; FontSize = %f; FontSpaceWidth * FontSize = %f\n",
				xCoordNext,
				pParams->dCurrFontSpaceWidth,
				pParams->dsTextState.Tfs,
				pParams->dCurrFontSpaceWidth * pParams->dsTextState.Tfs);
		#endif
							
		pParams->xCoordPrev = pParams->dsRenderingMatrix.e;
						
		tempTextMatrixA.a = 1;
		tempTextMatrixA.b = 0;
		tempTextMatrixA.c = 0;
		tempTextMatrixA.d = 1;
		tempTextMatrixA.e = Tx;
		tempTextMatrixA.f = Ty;
				
		tempTextMatrixB.a = pParams->dsTextMatrix.a;
		tempTextMatrixB.b = pParams->dsTextMatrix.b;
		tempTextMatrixB.c = pParams->dsTextMatrix.c;
		tempTextMatrixB.d = pParams->dsTextMatrix.d;
		tempTextMatrixB.e = pParams->dsTextMatrix.e;
		tempTextMatrixB.f = pParams->dsTextMatrix.f;
					
		MultiplyTransMatrix(&tempTextMatrixA, &tempTextMatrixB, &(pParams->dsTextMatrix));	
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD2)
		wprintf(L"c = '%lc', XCoord = %f; YCoord = %f; CharWidth = %f; Tx = %f, xCoordNext = %f; FontSpaceWidthScaled = %f\n",
				c, xCoordCurrent, yCoordCurrent, dblCurrCharWidth, Tx, xCoordNext, pParams->dCurrFontSpaceWidthScaled);
		wprintf(L"\tdCurrFontSpaceWidth = [%f Scaled: %f]; dCurrFontAvgWidth = [%f Scaled : %f]; dCurrFontMaxWidth = [%f Scaled : %f];\n",
				pParams->dCurrFontSpaceWidth,
				pParams->dCurrFontSpaceWidthScaled,
				pParams->dCurrFontAvgWidth,
				pParams->dCurrFontAvgWidthScaled,
				pParams->dCurrFontMaxWidth,
				pParams->dCurrFontMaxWidthScaled);
		wprintf(L"\tUserSpaceMatrix -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
				UserSpaceMatrix.a,
				UserSpaceMatrix.b,
				UserSpaceMatrix.c,
				UserSpaceMatrix.d,
				UserSpaceMatrix.e,
				UserSpaceMatrix.f);
		#endif
			
		pParams->myRedBlackTreeKey.row = yCoordCurrent;
		pParams->myRedBlackTreeKey.col = xCoordCurrent;
		pParams->myRedBlackTreeKey.ord++;
		
		pParams->myRedBlackTreeData.c = c;
		pParams->myRedBlackTreeData.currFontSize = pParams->dsTextState.Tfs;
		pParams->myRedBlackTreeData.currFontSpaceWidth = pParams->dCurrFontSpaceWidth;
		pParams->myRedBlackTreeData.currFontSpaceWidthScaled = pParams->dCurrFontSpaceWidthScaled;
		pParams->myRedBlackTreeData.width = dblCurrCharWidth;
		pParams->myRedBlackTreeData.widthScaled = Tx;
		pParams->myRedBlackTreeData.xCoordNext = xCoordNext;
		pParams->myRedBlackTreeData.yCoordNext = yCoordNext;
		pParams->myRedBlackTreeData.nCurrentFontSubtype = pParams->nCurrentFontSubtype;
		pParams->myRedBlackTreeData.nCurrentCIDFontSubtype = pParams->nCurrentCIDFontSubtype;
		
		vlrbtInsertNode(&(pParams->myRedBlackTree), &(pParams->myRedBlackTreeKey), sizeof(pParams->myRedBlackTreeKey), &(pParams->myRedBlackTreeData), sizeof(pParams->myRedBlackTreeData));
		
		//pParams->myRedBlackTreeData.prevChar = c;
		//pParams->myRedBlackTreeData.prevRow = yCoordCurrent;
		//pParams->myRedBlackTreeData.prevCol = xCoordCurrent;
		//pParams->myRedBlackTreeData.prevWidth = dblCurrCharWidth;
		//pParams->myRedBlackTreeData.prevWidthScaled = Tx;
		//pParams->myRedBlackTreeData.prevXCoordNext = xCoordNext;
		//pParams->myRedBlackTreeData.prevYCoordNext = yCoordNext;		
		
		pParams->xCoordNextPrev = xCoordNext;
		pParams->yCoordNextPrev = yCoordNext;
		
		pParams->Tj = 0.0;
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR)
		wprintf(L"\tTextMatrix AFTER PRINT CHAR WITH %s OPERATOR -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
				szOpName,
				pParams->dsTextMatrix.a,
				pParams->dsTextMatrix.b,
				pParams->dsTextMatrix.c,
				pParams->dsTextMatrix.d,
				pParams->dsTextMatrix.e,
				pParams->dsTextMatrix.f);
		wprintf(L"\tLineMatrix AFTER PRINT CHAR WITH TJ OPERATOR -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
				szOpName,
				pParams->dsLineMatrix.a,
				pParams->dsLineMatrix.b,
				pParams->dsLineMatrix.c,
				pParams->dsLineMatrix.d,
				pParams->dsLineMatrix.e,
				pParams->dsLineMatrix.f);
		#endif
	}
	
	if ( lenString > 0 )
	{
		pParams->cLastChar = pszString[lenString - 1];
	}
	else
	{
		pParams->cLastChar = L' ';
		//wprintf(L"\nlenString = %d\n", lenString);
	}
		
	pParams->dLastCharWidth = Tx;

	return 1;
}

int ManageDecodedContentText(Params *pParams, int nPageNumber)
{
	int retValue = 1;
	
	char szName[128];
	char szPrevFontResName[128];
	
	char keyImageObj[256];
	uint32_t keyImageObjLength;
	void *pData = NULL;
	int nFindImageObj;
	
	unsigned long int nBlockSize = BLOCK_SIZE;
	unsigned long int x;
	
	TokenTypeEnum PrevType = T_UNKNOWN;
	
	unsigned char *pszString = NULL;
	size_t len;
			
	uint32_t bContentAlreadyProcessed;
	int nTemp;
	uint32_t nDataSize;
	int nRes;
	
	//int nCurrFontObjRef = 0;
		
	int bArrayState = 0;
						
	double dNumbersStack[1024];
	int nNumsStackTop = -1;
	
	//double xCoordPrev = 0.0;
	//double yCoordPrev = 0.0;
	
	//double xCoordNext = 0.0;
	//double yCoordNext = 0.0;
	
	//double xCoordNextPrev = 0.0;
	//double yCoordNextPrev = 0.0;
			
	TransMatrix tempTextMatrixA;
	//TransMatrix tempTextMatrixB;
	
	TJStackItem TJStack[1024];
	int nTJStackTop = -1;
	
	pParams->nCurrentPageNum = nPageNumber;
		
	pParams->cLastChar = L' ';
	pParams->xCoordPrev = pParams->xCoordNextPrev = 0.0;
	pParams->yCoordNextPrev = 0.0;
	
	pParams->nCTM_StackTop = 0;
	pParams->dCTM_Stack[pParams->nCTM_StackTop].a = 1;
	pParams->dCTM_Stack[pParams->nCTM_StackTop].b = 0;
	pParams->dCTM_Stack[pParams->nCTM_StackTop].c = 0;
	pParams->dCTM_Stack[pParams->nCTM_StackTop].d = 1;
	pParams->dCTM_Stack[pParams->nCTM_StackTop].e = 0;
	pParams->dCTM_Stack[pParams->nCTM_StackTop].f = 0;
	
	pParams->dsTextMatrix.a = 1;
	pParams->dsTextMatrix.b = 0;
	pParams->dsTextMatrix.c = 0;
	pParams->dsTextMatrix.d = 1;
	pParams->dsTextMatrix.e = 0;
	pParams->dsTextMatrix.f = 0;
	
	pParams->dsLineMatrix.a = 1;
	pParams->dsLineMatrix.b = 0;
	pParams->dsLineMatrix.c = 0;
	pParams->dsLineMatrix.d = 1;
	pParams->dsLineMatrix.e = 0;
	pParams->dsLineMatrix.f = 0;
	
	pParams->dsTextState.Tc = 0;
	pParams->dsTextState.Tw = 0;
	pParams->dsTextState.Th = 1; // 100% = 100/100;
	pParams->dsTextState.Tl = 0;
	pParams->dsTextState.Tfs = 12;
	pParams->dsTextState.Trise = 0;
	
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
	
	for ( nTemp = 0; nTemp < 128; nTemp++ )
		szPrevFontResName[nTemp] = '\0';
		
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
			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
		wprintf(L"\n\nManageDecodeContent -> START NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
		#endif
		
		while ( T_EOF != pParams->myToken.Type )
		{					
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
			//PrintToken(&(pParams->myToken), '\t', '\0', 1);
			PrintToken(&(pParams->myToken), '\0', '\0', 1);
			#endif
			
			switch ( pParams->myToken.Type )
			{
				case T_STRING_LITERAL:
				case T_STRING_HEXADECIMAL:
				{
					size_t i;
					
					pParams->bPdfHasText = 1;
				
					len = strnlen(pParams->myToken.vString, MAX_STRING_LENTGTH_IN_CONTENT_STREAM);
					memcpy(pszString, (unsigned char*)pParams->myToken.vString, len + 1);
											
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD) && !defined(MYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR)
					//wprintf(L"\tSTRINGA: (%s) <-> UTF-8: (%ls)\n", pszString, (wchar_t*)(pParams->pUtf8String));
					//wprintf(L"\nSTRING -> <%ls>\n\n", (wchar_t*)(pParams->pUtf8String));
					wprintf(L"\nSTRING <%ls>\n\n", pParams->pUtf8String);
					#endif
					
					pParams->lenUtf8String = wcsnlen(pParams->pUtf8String, MAX_STRING_LENTGTH_IN_CONTENT_STREAM);
					
					if ( bArrayState )
					{
						nTJStackTop++;
						if ( nTJStackTop >= 1024 )
						{
							snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: TJStack pieno.\n\n");
							myShowErrorMessage(pParams, pParams->szError, 1);
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
							wprintf(L"ERRORE ManageDecodedContent: TJStack pieno.\n\n");
							#endif
							retValue = 0;
							goto uscita;
						}
						TJStack[nTJStackTop].Type = TJSTACK_ITEMTYPE_STRING;
						TJStack[nTJStackTop].pUtf8String = NULL;
						
						//len = wcsnlen(pParams->pUtf8String, MAX_STRING_LENTGTH_IN_CONTENT_STREAM);
						
						//TJStack[nTJStackTop].pEncodingString = (wchar_t*)malloc(sizeof(wchar_t) * pParams->lenUtf8String + sizeof(wchar_t));
						//if ( NULL == TJStack[nTJStackTop].pEncodingString )
						//{
						//	snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: malloc failed for TJStack[%d].pUtf8String.\n\n", nTJStackTop);
						//	myShowErrorMessage(pParams, pParams->szError, 1);
						//	retValue = 0;
						//	goto uscita;
						//}
						
						TJStack[nTJStackTop].pUtf8String = (wchar_t*)malloc(sizeof(wchar_t) * pParams->lenUtf8String + sizeof(wchar_t));							
						if ( NULL == TJStack[nTJStackTop].pUtf8String )
						{
							snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: malloc failed for TJStack[%d].pUtf8String.\n\n", nTJStackTop);
							myShowErrorMessage(pParams, pParams->szError, 1);
							retValue = 0;
							goto uscita;
						}
						
						for ( i = 0; i < pParams->lenUtf8String; i++ )
						{
							//TJStack[nTJStackTop].pEncodingString[i] = pParams->pEncodingString[i];
							TJStack[nTJStackTop].pUtf8String[i] = pParams->pUtf8String[i];
						}
							
						//TJStack[nTJStackTop].pEncodingString[i] = L'\0';
						TJStack[nTJStackTop].pUtf8String[i] = L'\0';
						TJStack[nTJStackTop].lenUtf8String = pParams->lenUtf8String;
						//wcsncpy(TJStack[nTJStackTop].pUtf8String, pParams->pUtf8String, pParams->lenUtf8String);
						
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR)
						wprintf(L"\nENQUEUE(%d) STRING -> (%ls) len = %lu\n", nTJStackTop, TJStack[nTJStackTop].pUtf8String, TJStack[nTJStackTop].lenUtf8String);
						#endif
					}
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR)
					else
					{
						wprintf(L"\nSTRING <%ls>\n", pParams->pUtf8String);
					}
					#endif								
				}
				break;
				case T_INT_LITERAL:
				{
					double dblLastNumber = (double)pParams->myToken.vInt;
				
					if ( !bArrayState )
					{
						nNumsStackTop++;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
						wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
						#endif
						if ( nNumsStackTop >= 1024 )
						{
							snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack pieno.\n\n");
							myShowErrorMessage(pParams, pParams->szError, 1);
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
							wprintf(L"ERRORE ManageDecodedContent: dNumbersStack pieno.\n\n");
							#endif
							retValue = 0;
							goto uscita;
						}
						dNumbersStack[nNumsStackTop] = dblLastNumber;						
					}
					else
					{
						nTJStackTop++;
						if ( nTJStackTop >= 1024 )
						{
							snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: TJStack pieno.\n\n");
							myShowErrorMessage(pParams, pParams->szError, 1);
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
							wprintf(L"ERRORE ManageDecodedContent: TJStack pieno.\n\n");
							#endif
							retValue = 0;
							goto uscita;
						}
						TJStack[nTJStackTop].Type = TJSTACK_ITEMTYPE_NUMBER;
						TJStack[nTJStackTop].dNum = dblLastNumber;
						TJStack[nTJStackTop].pUtf8String = NULL;
						
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR)
						wprintf(L"\nENQUEUE(%d) NUMBER -> %f\n", nTJStackTop, TJStack[nTJStackTop].dNum);
						#endif
					}
				}
				break;
				case T_REAL_LITERAL:
				{
					double dblLastNumber = pParams->myToken.vDouble;
				
					if ( !bArrayState )
					{
						nNumsStackTop++;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
						wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
						#endif
						if ( nNumsStackTop >= 1024 )
						{
							snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack pieno.\n\n");
							myShowErrorMessage(pParams, pParams->szError, 1);
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
							wprintf(L"ERRORE ManageDecodedContent: dNumbersStack pieno.\n\n");
							#endif
							retValue = 0;
							goto uscita;
						}
						dNumbersStack[nNumsStackTop] = dblLastNumber;						
					}
					else
					{
						nTJStackTop++;
						if ( nTJStackTop >= 1024 )
						{
							snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: TJStack pieno.\n\n");
							myShowErrorMessage(pParams, pParams->szError, 1);
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
							wprintf(L"ERRORE ManageDecodedContent: TJStack pieno.\n\n");
							#endif
							retValue = 0;
							goto uscita;
						}
						TJStack[nTJStackTop].Type = TJSTACK_ITEMTYPE_NUMBER;
						TJStack[nTJStackTop].dNum = dblLastNumber;
						TJStack[nTJStackTop].pUtf8String = NULL;
						
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR)
						wprintf(L"\nENQUEUE(%d) NUMBER -> %f\n", nTJStackTop, TJStack[nTJStackTop].dNum);
						#endif
					}
				}
				break;
				case T_NAME:
				{
					strncpy(szName, pParams->myToken.vString, 127);
				}
				break;
				case T_QOPAREN:
				{
					bArrayState = 1;
				}
				break;
				case T_QCPAREN:
				{
					bArrayState = 0;
				}
				break;
				case T_CONTENT_OP_Td:
				{
					double dX;
					double dY;
					
					//double xTemp;
					//int bSpace = 0;
				
					TransMatrix tmA;
					TransMatrix tmB;
				
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
									
					if ( nNumsStackTop < 1 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack operatore 'Td': lo stack contiene meno di 2 numeri: %d.\n\n", nNumsStackTop);
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"ERRORE ManageDecodedContent: dNumbersStack operatore 'Td': lo stack contiene meno di 2 numeri : %d.\n\n", nNumsStackTop);
						#endif
						retValue = 0;
						goto uscita;
					}
				
					dY = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
					dX = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
									
					tmA.a = 1;
					tmA.b = 0;
					tmA.c = 0;
					tmA.d = 1;
					tmA.e = dX;
					tmA.f = dY;
				
					tmB.a = pParams->dsLineMatrix.a;
					tmB.b = pParams->dsLineMatrix.b;
					tmB.c = pParams->dsLineMatrix.c;
					tmB.d = pParams->dsLineMatrix.d;
					tmB.e = pParams->dsLineMatrix.e;
					tmB.f = pParams->dsLineMatrix.f;
				
					MultiplyTransMatrix(&tmA, &tmB, &(pParams->dsLineMatrix));
					
					pParams->dsTextMatrix.a = pParams->dsLineMatrix.a;
					pParams->dsTextMatrix.b = pParams->dsLineMatrix.b;
					pParams->dsTextMatrix.c = pParams->dsLineMatrix.c;
					pParams->dsTextMatrix.d = pParams->dsLineMatrix.d;
					pParams->dsTextMatrix.e = pParams->dsLineMatrix.e;
					pParams->dsTextMatrix.f = pParams->dsLineMatrix.f;
					
					if ( dY != 0 )
					{
						pParams->cLastChar = L' ';
						
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"\n\tVADO A CAPO CON L'OPERATORE 'Td'\n");
						#endif
												
						//if ( pParams->szOutputFile[0] != '\0' )
						//	fwprintf(pParams->fpOutput, L"\n");
						//else
						//	wprintf(L"\n");
					}
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR Td(dX = %f; dY = %f):\n", dX, dY);
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);

					#endif
				}
				break;
				case T_CONTENT_OP_TD:
				{
					double dX;
					double dY;
					
					//double xTemp;
				
					TransMatrix tmA;
					TransMatrix tmB;
				
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
									
					if ( nNumsStackTop < 1 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack operatore 'Td': lo stack contiene meno di 2 numeri: %d.\n\n", nNumsStackTop);
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"ERRORE ManageDecodedContent: dNumbersStack operatore 'Td': lo stack contiene meno di 2 numeri : %d.\n\n", nNumsStackTop);
						#endif
						retValue = 0;
						goto uscita;
					}
				
					dY = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
					dX = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
					
					pParams->dsTextState.Tl = -dY;
									
					tmA.a = 1;
					tmA.b = 0;
					tmA.c = 0;
					tmA.d = 1;
					tmA.e = dX;
					tmA.f = dY;

				
					tmB.a = pParams->dsLineMatrix.a;
					tmB.b = pParams->dsLineMatrix.b;
					tmB.c = pParams->dsLineMatrix.c;
					tmB.d = pParams->dsLineMatrix.d;
					tmB.e = pParams->dsLineMatrix.e;
					tmB.f = pParams->dsLineMatrix.f;
									
					MultiplyTransMatrix(&tmA, &tmB, &(pParams->dsLineMatrix));
					
					pParams->dsTextMatrix.a = pParams->dsLineMatrix.a;
					pParams->dsTextMatrix.b = pParams->dsLineMatrix.b;
					pParams->dsTextMatrix.c = pParams->dsLineMatrix.c;
					pParams->dsTextMatrix.d = pParams->dsLineMatrix.d;
					pParams->dsTextMatrix.e = pParams->dsLineMatrix.e;
					pParams->dsTextMatrix.f = pParams->dsLineMatrix.f;
					
					if ( dY != 0 )
					{
						pParams->cLastChar = L' ';
						
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"\n\tVADO A CAPO CON L'OPERATORE 'TD'\n");
						#endif	
						
						//if ( pParams->szOutputFile[0] != '\0' )
						//	fwprintf(pParams->fpOutput, L"\n");
						//else
						//	wprintf(L"\n");					
					}
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR TD(dX = %f; dY = %f):\n", dX, dY);
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif					
				}
				break;
				case T_CONTENT_OP_Tm:
				{
					double dblPrevY = 0.0;
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
					wprintf(L"\n");
					#endif
				
					if ( nNumsStackTop < 5 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack operatore 'cm': lo stack contiene meno di 6 numeri: %d.\n\n", nNumsStackTop);
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"ERRORE ManageDecodedContent: dNumbersStack operatore 'cm': lo stack contiene meno di 6 numeri : %d.\n\n", nNumsStackTop);
						#endif
						retValue = 0;
						goto uscita;
					}
					
					dblPrevY = pParams->dsTextMatrix.f;
				
					pParams->dsTextMatrix.f = pParams->dsLineMatrix.f = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
					pParams->dsTextMatrix.e = pParams->dsLineMatrix.e = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
					pParams->dsTextMatrix.d = pParams->dsLineMatrix.d = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
					pParams->dsTextMatrix.c = pParams->dsLineMatrix.c = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
					pParams->dsTextMatrix.b = pParams->dsLineMatrix.b = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
					pParams->dsTextMatrix.a = pParams->dsLineMatrix.a = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
							
					if ( pParams->dsTextMatrix.f != dblPrevY )
					{
						pParams->cLastChar = L' ';
						
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"\n\tVADO A CAPO CON L'OPERATORE 'Tm'\n");
						#endif
						
						//if ( pParams->szOutputFile[0] != '\0' )
						//	fwprintf(pParams->fpOutput, L"\n");
						//else
						//	wprintf(L"\n");
					}
																
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR Tm:\n");
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif
				}
				break;
				case T_CONTENT_OP_TSTAR:
				{
					double dY = -(pParams->dsTextState.Tl);
					TransMatrix tmA;
					TransMatrix tmB;
				
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, nCTM_StackTop);
					#endif
							
					if ( pParams->dsTextState.Tl != 0.0 )
					{						
						tmA.a = 1;
						tmA.b = 0;
						tmA.c = 0;
						tmA.d = 1;
						tmA.e = 0;
						tmA.f = dY;
				
						tmB.a = pParams->dsLineMatrix.a;
						tmB.b = pParams->dsLineMatrix.b;
						tmB.c = pParams->dsLineMatrix.c;
						tmB.d = pParams->dsLineMatrix.d;
						tmB.e = pParams->dsLineMatrix.e;
						tmB.f = pParams->dsLineMatrix.f;
									
						MultiplyTransMatrix(&tmA, &tmB, &(pParams->dsLineMatrix));
					
						pParams->dsTextMatrix.a = pParams->dsLineMatrix.a;
						pParams->dsTextMatrix.b = pParams->dsLineMatrix.b;
						pParams->dsTextMatrix.c = pParams->dsLineMatrix.c;
						pParams->dsTextMatrix.d = pParams->dsLineMatrix.d;
						pParams->dsTextMatrix.e = pParams->dsLineMatrix.e;
						pParams->dsTextMatrix.f = pParams->dsLineMatrix.f;	

						pParams->cLastChar = L' ';
						
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SINGLE_CHAR_COORD) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"\n\tVADO A CAPO CON L'OPERATORE 'T*'\n");
						#endif		
					
						//if ( pParams->szOutputFile[0] != '\0' )
						//	fwprintf(pParams->fpOutput, L"\n");
						//else
						//	wprintf(L"\n");						
										
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings)
						if ( '\0' != pParams->szWordsToSearch[0] )
							wprintf(L"\n");
						#endif
					}
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR T*:\n");
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif		
				}
				break;
				case T_CONTENT_OP_Tj:
				{	
					ManageShowTextOperator(pParams, "Tj", pParams->pUtf8String, pParams->lenUtf8String);
				}
				break;
				case T_CONTENT_OP_SINGLEQUOTE:
				{
					double dY = -(pParams->dsTextState.Tl);
					TransMatrix tmA;
					TransMatrix tmB;
				
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
							
					if ( pParams->dsTextState.Tl != 0.0 )
					{						
						tmA.a = 1;
						tmA.b = 0;
						tmA.c = 0;
						tmA.d = 1;
						tmA.e = 0;
						tmA.f = dY;
				
						tmB.a = pParams->dsTextMatrix.a;
						tmB.b = pParams->dsTextMatrix.b;
						tmB.c = pParams->dsTextMatrix.c;
						tmB.d = pParams->dsTextMatrix.d;
						tmB.e = pParams->dsTextMatrix.e;
						tmB.f = pParams->dsTextMatrix.f;
				
						MultiplyTransMatrix(&tmA, &tmB, &(pParams->dsTextMatrix));
					
						pParams->cLastChar = L' ';
					
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"\n\tVADO A CAPO CON L'OPERATORE \'\n\n");
						#endif								
					
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings)
						if ( '\0' != pParams->szWordsToSearch[0] )
							wprintf(L"\n");
						#endif	
					}
					
					ManageShowTextOperator(pParams, "'", pParams->pUtf8String, pParams->lenUtf8String);					
				}
				break;
				case T_CONTENT_OP_DOUBLEQUOTE:
				{
					double dY = -(pParams->dsTextState.Tl);
					TransMatrix tmA;
					TransMatrix tmB;
				
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
					
					if ( nNumsStackTop < 1 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack operatore '\"': lo stack contiene meno di 2 numeri: %d.\n\n", nNumsStackTop);
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"ERRORE ManageDecodedContent: dNumbersStack operatore '\"': lo stack contiene meno di 2 numeri : %d.\n\n", nNumsStackTop);
						#endif
						retValue = 0;
						goto uscita;
					}
				
					pParams->dsTextState.Tc = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
					pParams->dsTextState.Tw = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
							
					if ( pParams->dsTextState.Tl != 0.0 )
					{						
						tmA.a = 1;
						tmA.b = 0;
						tmA.c = 0;
						tmA.d = 1;
						tmA.e = 0;
						tmA.f = dY;
				
						tmB.a = pParams->dsLineMatrix.a;
						tmB.b = pParams->dsLineMatrix.b;
						tmB.c = pParams->dsLineMatrix.c;
						tmB.d = pParams->dsLineMatrix.d;
						tmB.e = pParams->dsLineMatrix.e;
						tmB.f = pParams->dsLineMatrix.f;
									
						MultiplyTransMatrix(&tmA, &tmB, &(pParams->dsLineMatrix));
					
						pParams->dsTextMatrix.a = pParams->dsLineMatrix.a;
						pParams->dsTextMatrix.b = pParams->dsLineMatrix.b;
						pParams->dsTextMatrix.c = pParams->dsLineMatrix.c;
						pParams->dsTextMatrix.d = pParams->dsLineMatrix.d;
						pParams->dsTextMatrix.e = pParams->dsLineMatrix.e;
						pParams->dsTextMatrix.f = pParams->dsLineMatrix.f;
					
						pParams->cLastChar = L' ';
					
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"\n\tVADO A CAPO CON L'OPERATORE \"\n\n");
						#endif								
					
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings)
						if ( '\0' != pParams->szWordsToSearch[0] )
							wprintf(L"\n");
						#endif	
					}

					ManageShowTextOperator(pParams, "\"", pParams->pUtf8String, pParams->lenUtf8String);
				}
				break;
				case T_CONTENT_OP_TJ:
				{
					// https://stackoverflow.com/questions/16462708/extract-text-from-pdf-document-based-on-position-c
					// And how does parameters w0 and w1 evolve during glyph painting? are they initially (0,0)

					// w0 and w1 denote the glyph's horizontal and vertical displacements.
					// In horizontal writing mode, w0 is the glyph widths transformed to text mode
					// (i.e. most often merely divided by 1000) and w1 is 0.
					// For vertical writing mode text inspect sections 9.2.4 and 9.7.4.3 in ISO 32000-1:2008.
										
					int k = 0;
					while ( k <= nTJStackTop )
					{						
						switch ( TJStack[k].Type )
						{
							case TJSTACK_ITEMTYPE_STRING:
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
								wprintf(L"TJStack[%d].pUtf8String = '%ls'\n\n", nTJStackTop, TJStack[k].pUtf8String);
								#endif 
								
								ManageShowTextOperator(pParams, "TJ", TJStack[k].pUtf8String, TJStack[k].lenUtf8String);
								
								//free(TJStack[k].pEncodingString);
								//TJStack[k].pEncodingString = NULL;
								free(TJStack[k].pUtf8String);
								TJStack[k].pUtf8String = NULL;
																
								pParams->Tj = 0.0;
								
								break;
							case TJSTACK_ITEMTYPE_NUMBER:
								pParams->Tj = TJStack[k].dNum * 0.001;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR)
								wprintf(L"\n\tDEQUEUE NUMBER TJStack[%d].dNum = '%f'\n", k, TJStack[k].dNum);
								#endif								
								break;
							default:
								break;
						}
						
						k++;
					}
					nTJStackTop = -1;					
				}
				break;
				case T_CONTENT_OP_Tc:
				{
					if ( nNumsStackTop < 0 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack operatore 'Tc': è vuoto.\n\n");
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) 
						wprintf(L"ERRORE ManageDecodedContent: dNumbersStack operatore 'Tc': lo stack è vuoto.\n\n");
						#endif
						retValue = 0;
						goto uscita;
					}
				
					pParams->dsTextState.Tc = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
									
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR Tc:\n");
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif
				
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
				}
				break;
				case T_CONTENT_OP_Tw:
				{
					if ( nNumsStackTop < 0 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack operatore 'Tw': è vuoto.\n\n");
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"ERRORE ManageDecodedContent: dNumbersStack operatore 'Tw': lo stack è vuoto.\n\n");
						#endif
						retValue = 0;
						goto uscita;
					}
				
					pParams->dsTextState.Tw = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
								 
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR Tw:\n");
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif

					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
				}
				break;
				case T_CONTENT_OP_Tz:
				{
					if ( nNumsStackTop < 0 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack operatore 'Tz': è vuoto.\n\n");
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"ERRORE ManageDecodedContent: dNumbersStack operatore 'Tz': lo stack è vuoto.\n\n");
						#endif
						retValue = 0;
						goto uscita;
					}
				
					pParams->dsTextState.Th = dNumbersStack[nNumsStackTop]/100.0; // percentale, dunque diviso 100 -> cento.
					nNumsStackTop--;
							
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR Tz:\n");
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif
								 
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
				}
				break;
				case T_CONTENT_OP_TL:
				{
					if ( nNumsStackTop < 0 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack operatore 'TL': è vuoto.\n\n");
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"ERRORE ManageDecodedContent: dNumbersStack operatore 'TL': lo stack è vuoto.\n\n");
						#endif
						retValue = 0;
						goto uscita;
					}
				
					pParams->dsTextState.Tl = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
				
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR TL:\n");
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif
				
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
				}
				break;
				case T_CONTENT_OP_Ts:
				{
					if ( nNumsStackTop < 0 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack operatore 'Ts': è vuoto.\n\n");
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"ERRORE ManageDecodedContent: dNumbersStack operatore 'Ts': lo stack è vuoto.\n\n");
						#endif
						retValue = 0;
						goto uscita;
					}
				
					pParams->dsTextState.Trise = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
				
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR Ts:\n");
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
				}
				break;
				case T_CONTENT_OP_Tr:
				{
					if ( nNumsStackTop < 0 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack operatore 'Tr': è vuoto.\n\n");
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"ERRORE ManageDecodedContent: dNumbersStack operatore 'Tr': lo stack è vuoto.\n\n");
						#endif
						retValue = 0;
						goto uscita;
					}				
					nNumsStackTop--;
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR Tr(%f):\n", dNumbersStack[nNumsStackTop + 1]);
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, nCTM_StackTop);
					#endif
				}
				break;
				case T_CONTENT_OP_Do:
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, nCTM_StackTop);
					#endif
				
					if ( '\0' != szName[0] )
					{
						len = strnlen(szName, 128);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowResourceSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"TROVATO 'Do' command: vado a prendere la Resource %s\n", szName);
						#endif
									
						//nRes = scopeFind(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef), szName, len + sizeof(char), (void*)&nTemp, &nDataSize, &bContentAlreadyProcessed, 1);
						nRes = scopeFind(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef), szName, len + sizeof(char), (void*)&nTemp, &nDataSize, &bContentAlreadyProcessed, 0);
						if ( nRes >= 0 ) // TROVATO
						{
							if ( !bContentAlreadyProcessed )
							{
								snprintf(keyImageObj, 256, "k%u", nTemp);
								keyImageObjLength = strnlen(keyImageObj, 256);
								pData = NULL;
								nDataSize = 0;
								//wprintf(L"keyImageObj = <%s> keyImageObjLength = %u\n", keyImageObj, keyImageObjLength);
								nFindImageObj = genhtFind(&(pParams->myHT_ImageObjs), keyImageObj, keyImageObjLength + sizeof(char), &pData, &nDataSize);

								if ( nFindImageObj < 0 )
								{
									#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowResourceSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
									wprintf(L"\tVado a fare il parsing dell'oggetto %d 0 R e torno subito.\n", nTemp);
									#endif
					
									bContentAlreadyProcessed = 1;
									//if ( !scopeUpdateValue(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef), szName, len + sizeof(char), (void*)&nTemp, nDataSize, bContentAlreadyProcessed, 1, 1) )
									if ( !scopeUpdateValue(&(pParams->pPagesArray[nPageNumber].myScopeHT_XObjRef), szName, len + sizeof(char), (void*)&nTemp, nDataSize, bContentAlreadyProcessed, 1, 0) )
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
									scopePush(&(pParams->pPagesArray[nPageNumber].myScopeHT_GsRef));
							
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
										//	pParams->nCountNotImageContent++;
								
										PushXObjDecodedContent(pParams, nPageNumber, nTemp);
									}
									else
									{
										//	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowResourceSelected)
										//	wprintf(L"\tManageDecodedContent: L'oggetto %d 0 R è un'immagine. Esco.\n\n", nTemp);
										//	#endif
										
										//	pParams->nCountImageContent++;
															
										pParams->bStreamState = pParams->myStreamsStack[pParams->nStreamsStackTop].bStreamState;
										pParams->bStringIsDecoded = pParams->myStreamsStack[pParams->nStreamsStackTop].bStringIsDecoded;
										pParams->blockCurPos = pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos;
									}
							
									mystringqueuelist_Free(&(pParams->CurrentContent.queueFilters));
									mydictionaryqueuelist_Free(&(pParams->CurrentContent.decodeParms));
									pParams->myDataDecodeParams.numFilter = 0;
							
									goto qui_dopo_push;
								}
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowResourceSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
								else
								{
									wprintf(L"\tManageDecodedContent: L'oggetto %d 0 R è un'immagine. Esco. CIAO CIAO\n\n", nTemp);
								}
								#endif
								// **********************************************************************
							}
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowResourceSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
							else
							{
								wprintf(L"\tOggetto %d 0 R già processato.\n", nTemp);
							}
							#endif
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowResourceSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
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
				break;
				case T_CONTENT_OP_Tf:
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, nCTM_StackTop);
					#endif
				
					if ( '\0' != szName[0] )
					{
						len = strnlen(szName, 128);
											
						if ( nNumsStackTop < 0 )
						{
							snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack operatore 'Tf': lo stack contiene meno di 1 numero: %d.\n\n", nNumsStackTop);
							myShowErrorMessage(pParams, pParams->szError, 1);
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
							wprintf(L"ERRORE ManageDecodedContent: dNumbersStack operatore 'Tf': lo stack contiene meno di 1 numero: %d.\n\n", nNumsStackTop);
							#endif
							retValue = 0;
							goto uscita;
						}	
				
						pParams->dsTextState.Tfs = dNumbersStack[nNumsStackTop];
						nNumsStackTop--;	
											
					//if ( strncmp(szName, szPrevFontResName, 128) != 0 )
					//{
						//len = strnlen(szName, 128);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						//wprintf(L"\nEHI 1: szName = '%s' -> szPrevFontResName = '%s'\n", szName, szPrevFontResName);
						wprintf(L"\nTROVATO 'Tf FONT SELECTOR' command: vado a prendere la Resource %s\n", szName);
						#endif
								
						//nCurrFontObjRef = 0;
							
						nRes = scopeFind(&(pParams->pPagesArray[nPageNumber].myScopeHT_FontsRef), szName, len + sizeof(char), (void*)&nTemp, &nDataSize, &bContentAlreadyProcessed, 0);
						if ( nRes >= 0 ) // TROVATO
						{
							//nCurrFontObjRef = nTemp;
								
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
							wprintf(L"\tVado a fare il parsing dell'oggetto FONT %d 0 R e torno subito.\n", nTemp);
							#endif
																			
							pParams->bStreamState = 0;
							pParams->bStringIsDecoded = 0;
							pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos = pParams->blockCurPos;	
								
							//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
							//wprintf(L"\tOggetto FONT %d 0 R NON trovato nella HashTable. È NECESSARIO EFFETTUARE IL PARSING DELL'OGGETTO.\n", nTemp);
							//#endif
							
							pParams->bCurrParsingFontIsCIDFont = 0;
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
							
							pParams->pCurrFontGlyphsWidths = pParams->myObjsTable[nTemp]->pGlyphsWidths;
							//pParams->dCurrFontSpaceWidth = pParams->pCurrFontGlyphsWidths->dFontSpaceWidth/1000.0;
							pParams->dCurrFontSpaceWidth = pParams->pCurrFontGlyphsWidths->dFontSpaceWidth;
														
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
							wprintf(L"\tFONT %d 0 R -> dCurrFontSpaceWidth -> %f; FontSize = %f\n", nTemp, pParams->dCurrFontSpaceWidth, pParams->dsTextState.Tfs);
							wprintf(L"\tTextState      -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
									pParams->dsTextState.Tc,
									pParams->dsTextState.Tw,
									pParams->dsTextState.Th,
									pParams->dsTextState.Tl,
									pParams->dsTextState.Tfs,
									pParams->dsTextState.Trise);
									
							wprintf(L"\tFirstChar = %d; LastChar = %d; WidthsArraySize = %d; MissingWidth = %f; AvgWidth = %f; MaxWidth = %f; DW = %f\n\n",
									pParams->pCurrFontGlyphsWidths->FirstChar,
									pParams->pCurrFontGlyphsWidths->LastChar,
									pParams->pCurrFontGlyphsWidths->WidthsArraySize,
									pParams->pCurrFontGlyphsWidths->MissingWidth,
									pParams->pCurrFontGlyphsWidths->AvgWidth,
									pParams->pCurrFontGlyphsWidths->MaxWidth,
									pParams->pCurrFontGlyphsWidths->DW);
							#endif
																					
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
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						else
						{
							wprintf(L"\tRISORSA FONT '%s' NON TROVATA!!!\n", szName);
						}	
						#endif					
					//}					
					}
					else
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: trovato operatore 'Tf' ma non e' preceduto dal nome della Resource!\n");
						myShowErrorMessage(pParams, pParams->szError, 1);
						//wprintf(L"ERRORE ManageDecodedContent: trovato operatore 'Tf' ma non e' preceduto dal nome della Resource!\n");
						retValue = 0;
						goto uscita;
					}
				
					//strncpy(szPrevFontResName, szName, len);
					//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected)
					//wprintf(L"\nEHI 2: szName = '%s' -> szPrevFontResName = '%s'\n", szName, szPrevFontResName);
					//#endif
					//szName[0] = '\0';
				}
				break;
				case T_CONTENT_OP_gs:
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, nCTM_StackTop);
					#endif
				
					if ( '\0' != szName[0] )
					{
						len = strnlen(szName, 128);
										
					//if ( strncmp(szName, szPrevFontResName, 128) != 0 )
					//{
						//len = strnlen(szName, 128);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowGsSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"\nTROVATO 'gs GRAPHIC STATE SELECTOR' command: vado a prendere la Resource %s\n", szName);
						#endif
									
						nRes = scopeFind(&(pParams->pPagesArray[nPageNumber].myScopeHT_GsRef), szName, len + sizeof(char), (void*)&nTemp, &nDataSize, &bContentAlreadyProcessed, 0);
						if ( nRes >= 0 ) // TROVATO
						{							
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowGsSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
							wprintf(L"\tVado a fare il parsing dell'oggetto GS %d 0 R e torno subito.\n", nTemp);
							#endif
							
							pParams->bStreamState = 0;
							pParams->bStringIsDecoded = 0;
							pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos = pParams->blockCurPos;

							if ( !ParseGsObject(pParams, nTemp) )
							{
								snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent ParseGsObject.\n"); 
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
							
							if ( pParams->nCurrentGsObjFontObjNum > 0 )
							{						
								//dFontSize = pParams->dCurrentGsObjFontSize;
								pParams->dsTextState.Tfs = pParams->dCurrentGsObjFontSize;
								
								pParams->bCurrParsingFontIsCIDFont = 0;
								if ( !ParseFontObject(pParams, pParams->nCurrentGsObjFontObjNum) )
								{
									snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent ParseFontObject.\n"); 
									myShowErrorMessage(pParams, pParams->szError, 1);
							
									snprintf(pParams->szError, 8192, "\n***** ECCO L'OGGETTO ERRATO:\n");
									myShowErrorMessage(pParams, pParams->szError, 1);
								
									PrintThisObject(pParams, nTemp, 0, 0, pParams->fpErrors);
								
									snprintf(pParams->szError, 8192, "\n***** FINE OGGETTO ERRATO:\n");
									myShowErrorMessage(pParams, pParams->szError, 1);
									retValue = 0;
									goto uscita;
								}
								
								//if ( pParams->nCurrentGsObjFontObjNum > 0 )
								//	nCurrFontObjRef = pParams->nCurrentGsObjFontObjNum;
							
								pParams->pCurrFontGlyphsWidths = pParams->myObjsTable[nTemp]->pGlyphsWidths;
								//pParams->dCurrFontSpaceWidth = pParams->myObjsTable[nTemp]->pGlyphsWidths->dFontSpaceWidth/1000.0;
								pParams->dCurrFontSpaceWidth = pParams->myObjsTable[nTemp]->pGlyphsWidths->dFontSpaceWidth;
																
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowGsSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
								wprintf(L"\tFONT %d 0 R -> dCurrFontSpaceWidth -> %f; FontSize = %f\n", nTemp, pParams->dCurrFontSpaceWidth, pParams->dsTextState.Tfs);
								wprintf(L"\tTextState      -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
										pParams->dsTextState.Tc,
										pParams->dsTextState.Tw,
										pParams->dsTextState.Th,
										pParams->dsTextState.Tl,
										pParams->dsTextState.Tfs,
										pParams->dsTextState.Trise);
								wprintf(L"\tFirstChar = %d; LastChar = %d; WidthsArraySize = %d; MissingWidth = %f; AvgWidth = %f; MaxWidth = %f; DW = %f\n\n",
										pParams->pCurrFontGlyphsWidths->FirstChar,
										pParams->pCurrFontGlyphsWidths->LastChar,
										pParams->pCurrFontGlyphsWidths->WidthsArraySize,
										pParams->pCurrFontGlyphsWidths->MissingWidth,
										pParams->pCurrFontGlyphsWidths->AvgWidth,
										pParams->pCurrFontGlyphsWidths->MaxWidth,
										pParams->pCurrFontGlyphsWidths->DW);
								#endif
							}
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowGsSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
							else
							{
								wprintf(L"\tL'oggetto GS %d 0 R non contiene riferimenti a oggetti FONT.\n\n", nTemp);
							}
							#endif	
							
							pParams->bStreamState = pParams->myStreamsStack[pParams->nStreamsStackTop].bStreamState;
							pParams->bStringIsDecoded = pParams->myStreamsStack[pParams->nStreamsStackTop].bStringIsDecoded;
							pParams->blockCurPos = pParams->myStreamsStack[pParams->nStreamsStackTop].blockCurPos;
								
							strncpy(szPrevFontResName, szName, len);
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowGsSelected)
							wprintf(L"\nEHI 2: szName = '%s' -> szPrevFontResName = '%s'\n", szName, szPrevFontResName);
							#endif
							//szName[0] = '\0';
							goto qui_dopo_push;
						}
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN) || defined(MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						else
						{
							wprintf(L"\tRISORSA GS '%s' NON TROVATA!!!\n", szName);
						}
						#endif
					//}					
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
				break;
				case T_CONTENT_OP_BI:
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, nCTM_StackTop);
					#endif
				
					// IGNORIAMO L'OPERATORE BI(Begin Image) FINO ALLA FINE DELLO STREAM
					goto libera;
				}
				break;
				case T_CONTENT_OP_cm:
				{
					TransMatrix tmA;
					TransMatrix tmB;

					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
					wprintf(L"\n");
					#endif
				
					if ( nNumsStackTop < 5 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dNumbersStack operatore 'cm': lo stack contiene meno di 6 numeri: %d.\n\n", nNumsStackTop);
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"ERRORE ManageDecodedContent: dNumbersStack operatore 'cm': lo stack contiene meno di 6 numeri: %d.\n\n", nNumsStackTop);
						#endif
						retValue = 0;
						goto uscita;
					}
				
					tmA.f = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;				
					tmA.e = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
					tmA.d = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;				
					tmA.c = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
					tmA.b = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;				
					tmA.a = dNumbersStack[nNumsStackTop];
					nNumsStackTop--;
									
					tmB.f = pParams->dCTM_Stack[pParams->nCTM_StackTop].f;
					tmB.e = pParams->dCTM_Stack[pParams->nCTM_StackTop].e;
					tmB.d = pParams->dCTM_Stack[pParams->nCTM_StackTop].d;
					tmB.c = pParams->dCTM_Stack[pParams->nCTM_StackTop].c;
					tmB.b = pParams->dCTM_Stack[pParams->nCTM_StackTop].b;
					tmB.a = pParams->dCTM_Stack[pParams->nCTM_StackTop].a;
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
				
					MultiplyTransMatrix(&tmA, &tmB, &(pParams->dCTM_Stack[pParams->nCTM_StackTop]));
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR cm:\n");
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif
				}			
				break;
				case T_CONTENT_OP_q:
				{
					pParams->nCTM_StackTop++;
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
					if ( pParams->nCTM_StackTop >= 1024 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: dCTM_Stack operatore 'q': stack pieno.\n\n");
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"ERRORE ManageDecodedContent: dCTM_Stack operatore 'cm': stack pieno.\n\n");
						#endif
						retValue = 0;
						goto uscita;
					}
				
					pParams->dCTM_Stack[pParams->nCTM_StackTop].a = pParams->dCTM_Stack[pParams->nCTM_StackTop - 1].a;
					pParams->dCTM_Stack[pParams->nCTM_StackTop].b = pParams->dCTM_Stack[pParams->nCTM_StackTop - 1].b;
					pParams->dCTM_Stack[pParams->nCTM_StackTop].c = pParams->dCTM_Stack[pParams->nCTM_StackTop - 1].c;
					pParams->dCTM_Stack[pParams->nCTM_StackTop].d = pParams->dCTM_Stack[pParams->nCTM_StackTop - 1].d;
					pParams->dCTM_Stack[pParams->nCTM_StackTop].e = pParams->dCTM_Stack[pParams->nCTM_StackTop - 1].e;
					pParams->dCTM_Stack[pParams->nCTM_StackTop].f = pParams->dCTM_Stack[pParams->nCTM_StackTop - 1].f;
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR q:\n");
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif
				}
				break;
				case T_CONTENT_OP_Q:
				{
					pParams->nCTM_StackTop--;
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, pParams->nCTM_StackTop);
					#endif
					if ( pParams->nCTM_StackTop < 0 )
					{
						snprintf(pParams->szError, 8192, "ERRORE ManageDecodedContent: nCTM_StackTop operatore 'Q': lo stack contiene meno di 1 numero.\n\n");
						myShowErrorMessage(pParams, pParams->szError, 1);
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
						wprintf(L"ERRORE ManageDecodedContent: nCTM_StackTop operatore 'Q': lo stack contiene meno di 1 numero.\n\n");
						#endif
						retValue = 0;
						goto uscita;
					}
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR Q:\n");
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif
				}
				break;
				case T_CONTENT_OP_BT:
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, nCTM_StackTop);
					#endif
					
					tempTextMatrixA.a = 1;
					tempTextMatrixA.b = 0;
					tempTextMatrixA.c = 0;
					tempTextMatrixA.d = 1;
					tempTextMatrixA.e = 0;
					tempTextMatrixA.f = 0;
					
					MultiplyTransMatrix(&tempTextMatrixA, &(pParams->dCTM_Stack[pParams->nCTM_StackTop]), &(pParams->dsTextMatrix));
				
					pParams->dsLineMatrix.a = pParams->dsTextMatrix.a;
					pParams->dsLineMatrix.b = pParams->dsTextMatrix.b;
					pParams->dsLineMatrix.c = pParams->dsTextMatrix.c;
					pParams->dsLineMatrix.d = pParams->dsTextMatrix.d;
					pParams->dsLineMatrix.e = pParams->dsTextMatrix.e;
					pParams->dsLineMatrix.f = pParams->dsTextMatrix.f;
					
					pParams->Tj = 0.0;
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR BT:\n");
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif
				}
				break;
				case T_CONTENT_OP_ET:
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
					wprintf(L"ManageDecodeContent -> NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n", nNumsStackTop, nCTM_StackTop);
					#endif
					
					pParams->dsLineMatrix.a = pParams->dsTextMatrix.a = 1;
					pParams->dsLineMatrix.b = pParams->dsTextMatrix.b = 0;
					pParams->dsLineMatrix.c = pParams->dsTextMatrix.c = 0;
					pParams->dsLineMatrix.d = pParams->dsTextMatrix.d = 1;
					pParams->dsLineMatrix.e = pParams->dsTextMatrix.e = 0;
					pParams->dsLineMatrix.f = pParams->dsTextMatrix.f = 0;
											
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR)
					wprintf(L"\nOPERATOR ET:\n");
					wprintf(L"\tTextState    -> Tc = %f; Tw = %f; Th = %f; Tl = %f; Tfs = %f; Trise = %f\n",
							pParams->dsTextState.Tc,
							pParams->dsTextState.Tw,
							pParams->dsTextState.Th,
							pParams->dsTextState.Tl,
							pParams->dsTextState.Tfs,
							pParams->dsTextState.Trise);
					wprintf(L"\tCTM          -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dCTM_Stack[pParams->nCTM_StackTop].a,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].b,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].c,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].d,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].e,
							pParams->dCTM_Stack[pParams->nCTM_StackTop].f);
					wprintf(L"\tTextMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsTextMatrix.a,
							pParams->dsTextMatrix.b,
							pParams->dsTextMatrix.c,
							pParams->dsTextMatrix.d,
							pParams->dsTextMatrix.e,
							pParams->dsTextMatrix.f);
					wprintf(L"\tLineMatrix   -> a = %f; b = %f; c = %f; d = %f; e = %f; f = %f;\n",
							pParams->dsLineMatrix.a,
							pParams->dsLineMatrix.b,
							pParams->dsLineMatrix.c,
							pParams->dsLineMatrix.d,
							pParams->dsLineMatrix.e,
							pParams->dsLineMatrix.f);
					#endif
				}
				break;
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_PrintStrings) || defined(MYDEBUG_PRINT_DECODED_CONTENT_TOKENS)
				case T_STRING:
				{
					wprintf(L"\tT_STRING -> <%s>\n\n", pParams->myToken.vString);
				}
				break;
				#endif
				default:
				{
					;
				}
				break;
			} // FINE SWITCH
	
			if ( pParams->bCurrTokenIsOperator )
			{
				nNumsStackTop = -1;
				
				while ( nTJStackTop >= 0 )
				{
					//free(TJStack[nTJStackTop].pEncodingString);
					//TJStack[nTJStackTop].pEncodingString = NULL;
					free(TJStack[nTJStackTop].pUtf8String);
					TJStack[nTJStackTop].pUtf8String = NULL;
								
					nTJStackTop--;
				}
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
		scopePop(&(pParams->pPagesArray[nPageNumber].myScopeHT_GsRef));
		
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
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_SPLITTING_WORDS) || defined(MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK)
	wprintf(L"\n\nManageDecodeContent -> END NUM STACK: nNumsStackTop = %d; nCTM_StackTop = %d\n\n\n", nNumsStackTop, nCTM_StackTop);
	#endif

	pParams->bReadingStringsFromDecodedStream = 0;

	if ( NULL != pszString )
	{
		free(pszString);
		pszString = NULL;
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
	
	pParams->cLastChar = L' ';
		
	vlrbtInit(&(pParams->myRedBlackTree), VlRbtCompareFuncOrd, VlRbtOnTraverseFunc);
	
	pParams->myRedBlackTreeData.pCurrentNode = NULL;
	
	pParams->myRedBlackTreeKey.ord = 0;
	
	//pParams->myRedBlackTreeData.prevRow = 0;
	//pParams->myRedBlackTreeData.prevCol = 0;
	//pParams->myRedBlackTreeData.prevWidth = 0;
	//pParams->myRedBlackTreeData.prevWidthScaled = 0;

	pParams->myRedBlackTreeData.pParams = pParams;
	//if ( pParams->szOutputFile[0] != '\0' )
	//	pParams->myRedBlackTreeData.fpOutput = pParams->fpOutput;
	//else
	//	pParams->myRedBlackTreeData.fpOutput = NULL;
		
	//pParams->myRedBlackTreeData.prevChar = L' ';
	
	if ( SORT_GLYPHS_BY_ROW == pParams->nSortGlyphs )
		vlrbtSetCompareFunc(&(pParams->myRedBlackTree), VlRbtCompareFuncRow);
	else
		vlrbtSetCompareFunc(&(pParams->myRedBlackTree), VlRbtCompareFuncOrd);
		
	//vlrbtSetCompareFunc(&(pParams->myRedBlackTree), VlRbtCompareFuncCol);
	
	vlrbtSetOnTraverseFunc(&(pParams->myRedBlackTree), VlRbtOnTraverseFunc);
	//vlrbtSetOnTraverseFunc(&(pParams->myRedBlackTree), VlRbtOnTraverseFuncNew);
	
	pParams->TextLength = 0;	
	//retValue = ManageDecodedContent(pParams, nPageNumber);
	retValue = ManageDecodedContentText(pParams, nPageNumber);
	
	if ( retValue && pParams->myRedBlackTree.count > 0 )
	{	
		int bPrint = 0;
		int bSearch = 0;
			
		int numChars = pParams->myRedBlackTree.count;
		numChars *= 5;
		
		pParams->pText = (wchar_t*)malloc(numChars * sizeof(wchar_t) + sizeof(wchar_t));
		if ( NULL == pParams->pText )
		{
			snprintf(pParams->szError, 8192, "ERRORE ManageContent: impossibile allocare byte per il testo\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			retValue = 0;
			goto uscita;
		}
		for ( int i = 0; i < numChars; i++ )
		{
			pParams->pText[i] = L'\0';
		}
		
		vlrbtTraverseInOrder(&(pParams->myRedBlackTree));
		
		//if ( !(pParams->bOptVersionOrHelp) && (pParams->szFilePdf[0] == '\0' && pParams->szPath[0] == '\0') )
		//{
		//	wprintf(L"\n\nErrore: dev'essere specificata una(e solo una, non entrambe) delle due opzioni: '-p (--path)' o '-e(--extracttextfrom)'\n\n");
		//	return 0;
		//}
	
		if ( '\0' != pParams->szFilePdf[0] )
				bPrint = 1;
		if ( '\0' != pParams->szPath[0] )
				bSearch = 1;
				
		ManageExtractedText(pParams, bPrint, bSearch);
	}
		
uscita:

	vlrbtFree(&(pParams->myRedBlackTree));
	
	if ( NULL != pParams->pText )
	{
		free(pParams->pText);
		pParams->pText = NULL;
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

int ManageExtractedText(Params *pParams, int bPrint, int bSearch)
{
	int retValue = 1;
	
	uint32_t k;
	wchar_t c;
				
	pParams->pwszCurrentWord = (wchar_t*)malloc(MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(wchar_t));
	if ( NULL == pParams->pwszCurrentWord )
	{
		snprintf(pParams->szError, 8192, "ERRORE ManageExtractedText: impossibile allocare la memoria per pParams->pwszCurrentWord.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pwszCurrentWord.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pParams->pwszCurrentWord.\n");
		retValue = 0;
		goto uscita;
	}	
	
	pParams->pwszPreviousWord = (wchar_t*)malloc(MAX_STRING_LENTGTH_IN_CONTENT_STREAM + sizeof(wchar_t));
	if ( NULL == pParams->pwszPreviousWord )
	{
		snprintf(pParams->szError, 8192, "ERRORE ManageExtractedText: impossibile allocare la memoria per pParams->pwszPreviousWord.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pwszPreviousWord.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent: impossibile allocare la memoria per pParams->pwszPreviousWord.\n");
		retValue = 0;
		goto uscita;
	}
	
	pParams->bStateSillab = 0;

	pParams->idxCurrentWordChar = 0;
	pParams->pwszCurrentWord[0] = '\0';
	pParams->idxPreviousWordChar = 0;
	pParams->pwszPreviousWord[0] = '\0';
	
	
	
	for ( k = 0; k < pParams->TextLength; k++ )
	{
		c = pParams->pText[k];
		
		if ( bPrint )
		{
			if ( NULL != pParams->fpOutput )	
				fwprintf(pParams->fpOutput, L"%lc", c);
			else
				wprintf(L"%lc", c);
		}		
		
		
		
		//if ( '\0' != pParams->szWordsToSearch[0] )
		if ( bSearch )
		{
			// SPLIT WORDS INIZIO
			c = towlower(c);
					
			//if ( c >= L'a' && c <= L'z' )
			if (
			     (c >= 0x0041 && c <= 0x007A) ||
			     (c >= 0x00C0 && c <= 0x00FF) ||
			     (c >= 0x0100 && c <= 0x017F) ||
			     (c >= 0x0180 && c <= 0x024F) ||
			     (c >= 0x0250 && c <= 0x02AF) ||
			     (c >= 0x1D00 && c <= 0x1DBF) ||
			     (c >= 0x1E00 && c <= 0x1EFF) ||
			     (c >= 0x2C60 && c <= 0x2C7F) ||
			     (c >= 0xA720 && c <= 0xA7FF) ||
			     (c >= 0xAB30 && c <= 0xAB7F) ||
			     (c >= 0xFB00 && c <= 0xFB06) ||
			     (c >= 0x0370 && c <= 0x03FF) ||
			     (c >= 0x1F00 && c <= 0x1FFF) ||
			     (c >= 0x0400 && c <= 0x04FF) ||
			     (c >= 0x0500 && c <= 0x052F) ||
			     (c >= 0x2DE0 && c <= 0x2DFF) ||
			     (c >= 0xA640 && c <= 0xA69F) ||
			     (c >= 0x1C80 && c <= 0x1C8F) ||
			     (c >= 0x0530 && c <= 0x580F) ||
			     (c >= 0x10A0 && c <= 0x10FF) ||
			     (c >= 0x1C90 && c <= 0x1CBF) ||
			     (c >= 0x2D00 && c <= 0x2D2F) ||
			     (c >= 0x02B0 && c <= 0x02FF) ||
			     (c >= 0xA700 && c <= 0xA71F) ||
			     (c >= 0x0300 && c <= 0x036F) ||
			     (c >= 0x1AB0 && c <= 0x1AFF) ||
			     (c >= 0x1DC0 && c <= 0x1DFF) ||
			     (c >= 0x20D0 && c <= 0x20FF) ||
			     (c >= 0xFE20 && c <= 0xFE2F) ||
			     (c >= 0x16A0 && c <= 0x16F0) ||
			     (c >= 0x1680 && c <= 0x169F) ||
			     (c >= 0x0590 && c <= 0x05FF) ||
			     (c >= 0xFB1D && c <= 0xFB4F) ||
			     (c >= 0x0600 && c <= 0x06FF) ||
			     (c >= 0x0750 && c <= 0x077F) ||
			     (c >= 0x08A0 && c <= 0x08FF) ||
			     (c >= 0xFB50 && c <= 0xFDFF) ||
			     (c >= 0xFE70 && c <= 0xFEFF) ||
			     (c >= 0x0700 && c <= 0x074F) ||
			     (c >= 0x0800 && c <= 0x083F) ||
			     (c >= 0x0840 && c <= 0x085F) ||
			     (c >= 0x0900 && c <= 0x097F) ||
			     (c >= 0x0980 && c <= 0x09FF) ||
			     (c >= 0x0A00 && c <= 0x0A7F) ||
			     (c >= 0x0A80 && c <= 0x0AFF) ||
			     (c >= 0x0B00 && c <= 0x0B7F) ||
			     (c >= 0x0B80 && c <= 0x0BFF) ||
			     (c >= 0x0C00 && c <= 0x0C7F) ||
			     (c >= 0x0C80 && c <= 0x0CFF) ||
			     (c >= 0x0D00 && c <= 0x0D7F) ||
			     (c >= 0x0780 && c <= 0x07BF) ||
			     (c >= 0x0D80 && c <= 0x0DFF) ||
			     (c >= 0x0F00 && c <= 0x0FFF) ||
			     (c >= 0x1800 && c <= 0x18AF) ||
			     (c >= 0x1900 && c <= 0x194F) ||
			     (c >= 0xABC0 && c <= 0xABFF) ||
			     (c >= 0x1C50 && c <= 0x1C7F) ||
			     (c >= 0x1C00 && c <= 0x1C4F) ||
			     (c >= 0xA880 && c <= 0xA8DF) ||
			     (c >= 0xA840 && c <= 0xA87F) ||
			     (c >= 0xA800 && c <= 0xA82F) ||
			     (c >= 0x0E00 && c <= 0x0E7F) ||
			     (c >= 0x0E80 && c <= 0x0EFF) ||
			     (c >= 0x1000 && c <= 0x109F) ||
			     (c >= 0x1780 && c <= 0x17FF) ||
			     (c >= 0x1950 && c <= 0x197F) ||
			     (c >= 0x1980 && c <= 0x19DF) ||
			     (c >= 0x1A20 && c <= 0x1AAF) ||
			     (c >= 0xAA80 && c <= 0xAADF) ||
			     (c >= 0xA900 && c <= 0xA92F) ||
			     (c >= 0xAA5F && c <= 0xAA00) ||
			     (c >= 0x1700 && c <= 0x171F) ||
			     (c >= 0x1720 && c <= 0x173F) ||
			     (c >= 0x1740 && c <= 0x175F) ||
			     (c >= 0x1760 && c <= 0x177F) ||
			     (c >= 0x1A00 && c <= 0x1A1F) ||
			     (c >= 0x1B00 && c <= 0x1B7F) ||
			     (c >= 0xA980 && c <= 0xA9DF) ||
			     (c >= 0xA930 && c <= 0xA95F) ||
			     (c >= 0x1BC0 && c <= 0x1BFF) ||
			     (c >= 0x1B80 && c <= 0x1BBF) ||
			     (c >= 0xF900 && c <= 0xFAFF) ||
			     (c >= 0x3190 && c <= 0x319F) ||
			     (c >= 0x2E80 && c <= 0x2FD5) ||
			     (c >= 0x31C0 && c <= 0x31EF) ||
			     (c >= 0x1200 && c <= 0x137F) ||
			     (c >= 0x2D30 && c <= 0x2D7F) ||
			     (c >= 0x07C7 && c <= 0x07FF) ||
			     (c >= 0xA500 && c <= 0xA63F) ||
			     (c >= 0xA6A0 && c <= 0xA6FF) ||
			     (c >= 0x13A0 && c <= 0x13FF) ||
			     (c >= 0xAB70 && c <= 0xABBF) ||
			     (c >= 0x1400 && c <= 0x167F) ||
			     (c >= 0x2800 && c <= 0x28FF)
			   )
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
				InsertWordIntoTst(pParams);
			}									
			// SPLIT WORDS FINE
		}
	}
	
	uscita:
	
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
	
	if ( pParams->bParsingFontObj )
	{
		if ( NULL != pParams->myObjsTable[objNum]->pszDirectFontResourceString )
		{		
			pParams->pReadNextChar = ReadNextCharFromStmObjStream;
					
			if ( pParams->myObjsTable[objNum]->lenDirectFontResourceString > nBlockSize )
			{
				for ( x = 2; nBlockSize <= pParams->myObjsTable[objNum]->lenDirectFontResourceString; x++ )
					nBlockSize += BLOCK_SIZE;
	
				if ( NULL != pParams->myBlock )
					free(pParams->myBlock);
			
				pParams->myBlock = (unsigned char *)malloc(sizeof(unsigned char) * nBlockSize);
				if ( !(pParams->myBlock) )
				{
					snprintf(pParams->szError, 8192, "ERRORE LoadFirstBlock(%s): malloc failed for pParams->myBlock.\n\n", pszFunctionName);
					myShowErrorMessage(pParams, pParams->szError, 1);
					retValue = 0;
					goto uscita;
				}
			}
						
			memcpy(pParams->myBlock, pParams->myObjsTable[objNum]->pszDirectFontResourceString, pParams->myObjsTable[objNum]->lenDirectFontResourceString + sizeof(unsigned char));
		
			pParams->blockLen = pParams->myObjsTable[objNum]->lenDirectFontResourceString;
		
			pParams->blockCurPos = 0;
			
			retValue = 1;
			goto uscita;
		}
	}
	else if ( pParams->bParsingGsObj )
	{
		if ( NULL != pParams->myObjsTable[objNum]->pszDirectGsResourceString )
		{
			pParams->pReadNextChar = ReadNextCharFromStmObjStream;
					
			if ( pParams->myObjsTable[objNum]->lenDirectGsResourceString > nBlockSize )
			{
				for ( x = 2; nBlockSize <= pParams->myObjsTable[objNum]->lenDirectGsResourceString; x++ )
					nBlockSize += BLOCK_SIZE;
	
				if ( NULL != pParams->myBlock )
					free(pParams->myBlock);
			
				pParams->myBlock = (unsigned char *)malloc(sizeof(unsigned char) * nBlockSize);
				if ( !(pParams->myBlock) )
				{
					snprintf(pParams->szError, 8192, "ERRORE LoadFirstBlock(%s): malloc failed for pParams->myBlock.\n\n", pszFunctionName);
					myShowErrorMessage(pParams, pParams->szError, 1);
					retValue = 0;
					goto uscita;
				}
			}
						
			memcpy(pParams->myBlock, pParams->myObjsTable[objNum]->pszDirectGsResourceString, pParams->myObjsTable[objNum]->lenDirectGsResourceString + sizeof(unsigned char));
		
			pParams->blockLen = pParams->myObjsTable[objNum]->lenDirectGsResourceString;
		
			pParams->blockCurPos = 0;
		
			retValue = 1;
			goto uscita;
		}
	}
		
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

void myTreeTraversePostOrderLeafOnly(Tree *head, Params *pParams)
{
	Tree *pCurrNode = head;
	int x;
		
	if ( NULL != head->firstchild )
	{
		pCurrNode = head->firstchild;
		while ( NULL != pCurrNode )
		{
			myTreeTraversePostOrderLeafOnly(pCurrNode, pParams);
			pCurrNode = pCurrNode->sibling;		
		}
	}
	else
	{		
		pParams->nCurrentPageNum++;
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN) || defined(MYDEBUG_PRINT_ON_myTreeTraversePostOrderLeafOnly_FN)
		wprintf(L"pParams->nCurrentPageNum = %d <> numObjNumber = %d <> numObjParent = %d; ", pParams->nCurrentPageNum, head->numObjNumber, head->numObjParent);
		#endif
		
		pParams->pPagesArray[pParams->nCurrentPageNum].numObjContent = head->nCurrentNumPageObjContent;
		pParams->pPagesArray[pParams->nCurrentPageNum].bContentIsPresent = head->bCurrentContentIsPresent;
		
		//myintqueuelist_Init(&(pParams->pPagesArray[pParams->nCurrentPageNum].queueContentsObjRefs));
		//mycontentqueuelist_Init(&(pParams->pPagesArray[pParams->nCurrentPageNum].queueContens));
		
		while ( myintqueuelist_Dequeue(&(pParams->myObjsTable[head->numObjNumber]->queueContentsObjRefs), &(pParams->nTemp)) )
		{
			myintqueuelist_Enqueue(&(pParams->pPagesArray[pParams->nCurrentPageNum].queueContentsObjRefs), pParams->nTemp);
		}
						
		if ( !(head->bCurrentPageHasDirectResources) && (-1 == head->nCurrentPageResources)  )   // La pagina eredita Resources da uno dei suoi parenti.
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN) || defined(MYDEBUG_PRINT_ON_myTreeTraversePostOrderLeafOnly_FN)				
			wprintf(L"La pagina %d eredita Resources da uno dei suoi parenti.\n", pParams->nCurrentPageNum);
			#endif
			x = head->numObjParent;
			while ( x > 0 )
			{
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)						
				wprintf(L"\tParent -> %d\n", x);
				#endif
						
				while ( myobjreflist_Dequeue(&(pParams->myObjsTable[x]->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
				{
					scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096) + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
						
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
					if ( pParams->myObjsTable[x]->myXObjRefList.count <= 0 )
						scopeTraverse(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef), myOnScopeTraverse, 0);
					#endif							
				}
						
				while ( myobjreflist_Dequeue(&(pParams->myObjsTable[x]->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
				{
					scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096) + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
							
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
					if ( pParams->myObjsTable[x]->myFontsRefList.count <= 0 )
						scopeTraverse(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef), myOnScopeTraverse, 0);
					#endif							
				}						
					
				while ( myobjreflist_Dequeue(&(pParams->myObjsTable[x]->myGsRefList), pParams->szTemp, &(pParams->nTemp)) )
				{
					scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_GsRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096) + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
							
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
					if ( pParams->myObjsTable[x]->myGsRefList.count <= 0 )
						scopeTraverse(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_GsRef), myOnScopeTraverse, 0);
					#endif							
				}
					
				x = pParams->myObjsTable[x]->Obj.numObjParent;
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
				wprintf(L"\n");
				#endif
			}
		}
		else if ( !(head->bCurrentPageHasDirectResources) && (0 == head->nCurrentPageResources)  ) // La pagina non ha riferimenti a Resources.
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN) || defined(MYDEBUG_PRINT_ON_myTreeTraversePostOrderLeafOnly_FN)
			wprintf(L"La pagina %d non ha riferimenti a Resources.\n", pParams->nCurrentPageNum);
			#endif
		}
		else if ( !(head->bCurrentPageHasDirectResources) && head->nCurrentPageResources > 0 )   // Un intero > 0 indica il riferimento al numero dell'oggetto Resources.
		{
			//if ( !ParseDictionaryObject(pParams, head->nCurrentPageResources) )
			//{
			//	snprintf(pParams->szError, 8192, "ERRORE ParseObject -> ParseDictionaryObject\n");
			//	myShowErrorMessage(pParams, pParams->szError, 1);
			//}		
		
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN) || defined(MYDEBUG_PRINT_ON_myTreeTraversePostOrderLeafOnly_FN)
			wprintf(L"La pagina %d ha un oggetto Resources indiretto.\n", pParams->nCurrentPageNum);
			#endif
					
			while ( myobjreflist_Dequeue(&(pParams->myObjsTable[head->numObjNumber]->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
			{
				scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096)  + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
			}
					
			while ( myobjreflist_Dequeue(&(pParams->myObjsTable[head->numObjNumber]->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
			{
				scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096)  + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
			}	
			
			while ( myobjreflist_Dequeue(&(pParams->myObjsTable[head->numObjNumber]->myGsRefList), pParams->szTemp, &(pParams->nTemp)) )
			{
				scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_GsRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096)  + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
			}				
					
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN) || defined(MYDEBUG_PRINT_ON_myTreeTraversePostOrderLeafOnly_FN)
			scopeTraverse(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef), myOnScopeTraverse, 0);
			scopeTraverse(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef), myOnScopeTraverse, 0);
			scopeTraverse(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_GsRef), myOnScopeTraverse, 0);
			#endif					
		}
	
		if ( head->bCurrentPageHasDirectResources  )   // La pagina ha un oggetto Resources diretto.
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN) || defined(MYDEBUG_PRINT_ON_myTreeTraversePostOrderLeafOnly_FN)
			wprintf(L"La pagina %d ha un oggetto Resources diretto.\n", pParams->nCurrentPageNum);
			#endif
					
			while ( myobjreflist_Dequeue(&(pParams->myObjsTable[head->numObjNumber]->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
			{
				scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096) + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
			}
					
			while ( myobjreflist_Dequeue(&(pParams->myObjsTable[head->numObjNumber]->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
			{
				scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096) + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
			}
			
			while ( myobjreflist_Dequeue(&(pParams->myObjsTable[head->numObjNumber]->myGsRefList), pParams->szTemp, &(pParams->nTemp)) )
			{
				scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_GsRef), (void*)(pParams->szTemp), strnlen(pParams->szTemp, 4096) + sizeof(char), (void*)&(pParams->nTemp), sizeof(pParams->nTemp), 0);
			}
									
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN) || defined(MYDEBUG_PRINT_ON_myTreeTraversePostOrderLeafOnly_FN)
			scopeTraverse(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef), myOnScopeTraverse, 0);
			scopeTraverse(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef), myOnScopeTraverse, 0);
			scopeTraverse(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_GsRef), myOnScopeTraverse, 0);
			#endif
		}

		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN) || defined(MYDEBUG_PRINT_ON_myTreeTraversePostOrderLeafOnly_FN)
		wprintf(L"\n");	
		#endif
	}
}

int ParseObject(Params *pParams, int objNum)
{
	int retValue = 1;
	int nInt;
	int numObjQueueContent;
	//int x;
	
	int nFromPage;
	int nToPage;
	int nMiddlePage;
	int nCountPagesToParse;
	int nCountHalfPages;
	
	int idxWord;
	
	#if defined(MYPDFSEARCH_USE_TST)
	uint32_t res;
	#else
	int res;
	#endif
	
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
	myintqueuelist_Init(&(pParams->myPageLeafQueue));
	
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
	myobjreflist_Free(&(pParams->myGsRefList));
		
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
	
	pParams->pPagesTree = treeNewNode(pParams->ObjPageTreeRoot.Number, 0); // ROOT
	if ( NULL == pParams->pPagesTree )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Error ParseObject: treeNewNode failed for pParams->pPagesTree ROOT NODE.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		goto uscita;
	}
	pParams->myObjsTable[pParams->ObjPageTreeRoot.Number]->Obj.pTreeNode = pParams->pPagesTree;
	
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
		
		while ( myobjreflist_Dequeue(&(pParams->myGsRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)			
			wprintf(L"\tKey = '%s' -> GsObjectRef = %d 0 R\n", pParams->szTemp, pParams->nTemp);
			#endif
			myobjreflist_Enqueue( &(pParams->myObjsTable[1]->myGsRefList), pParams->szTemp, pParams->nTemp );
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
		
		while ( myobjreflist_Dequeue(&(pParams->myGsRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)			
			wprintf(L"\tKey = '%s' -> GsObjectRef = %d 0 R\n", pParams->szTemp, pParams->nTemp);
			#endif
			myobjreflist_Enqueue( &(pParams->myObjsTable[1]->myGsRefList), pParams->szTemp, pParams->nTemp );
		}	
	}
				
	myobjreflist_Free(&(pParams->myXObjRefList));
	myobjreflist_Free(&(pParams->myFontsRefList));
	myobjreflist_Free(&(pParams->myGsRefList));
	
	if ( pParams->nCountPagesFromPdf > 0 )
	{
		Tree *pParentNode = NULL;
		Tree *pFirstChildNode = NULL;
		Tree *pSiblingNode = NULL;
				
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
			scopeInit(&(pParams->pPagesArray[nInt].myScopeHT_GsRef));
		}
		
		while ( myintqueuelist_Dequeue(&(pParams->myPagesQueue), &nInt) )
		{			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)			
			wprintf(L"\n\nDEQUEUE Ecco ---> '%d'\n", nInt);
			#endif
	
			myobjreflist_Free(&(pParams->myXObjRefList));
			myobjreflist_Free(&(pParams->myFontsRefList));
			myobjreflist_Free(&(pParams->myGsRefList));
						
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
			
			
			
			pParentNode = pParams->myObjsTable[pParams->nCurrentPageParent]->Obj.pTreeNode;
			if ( NULL == pParentNode )
			{
				snprintf(pParams->szError, 8192, "ERRORE!!! ParseObject: pParentNode è NULL! -> pParams->nCurrentPageParent = %d\n", pParams->nCurrentPageParent);
				myShowErrorMessage(pParams, pParams->szError, 1);
				retValue = 0;
				goto uscita;
			}
			pFirstChildNode = pParentNode->firstchild;
			if ( NULL == pFirstChildNode )
			{
				pParentNode->firstchild = treeNewNode(nInt, pParams->nCurrentPageParent);
				if ( NULL == pParentNode->firstchild )
				{
					retValue = 0;
					snprintf(pParams->szError, 8192, "Error ParseObject: treeNewNode failed for pParentNode->firstchild NODE FIRST CHILD %d.\n", nInt);
					myShowErrorMessage(pParams, pParams->szError, 1);
					goto uscita;
				}
				
				pParentNode->firstchild->bCurrentPageHasDirectResources = pParams->bCurrentPageHasDirectResources;
				pParentNode->firstchild->nCurrentPageResources = pParams->nCurrentPageResources;
				
				pParentNode->firstchild->nCurrentNumPageObjContent = pParams->nCurrentNumPageObjContent;
				pParentNode->firstchild->bCurrentContentIsPresent = pParams->bCurrentContentIsPresent;
				
				pParams->myObjsTable[pParams->nCurrentObjNum]->Obj.pTreeNode = pParentNode->firstchild;				
			}
			else
			{
				pSiblingNode = pFirstChildNode->sibling;
				if ( NULL != pSiblingNode )
				{
					while ( pSiblingNode->sibling )
						pSiblingNode = pSiblingNode->sibling;
				
					pSiblingNode->sibling = treeNewNode(nInt, pParams->nCurrentPageParent);
					if ( NULL == pSiblingNode->sibling )
					{
						retValue = 0;
						snprintf(pParams->szError, 8192, "Error ParseObject: treeNewNode failed for pSiblingNode->sibling NODE SIBLING %d.\n", nInt);
						myShowErrorMessage(pParams, pParams->szError, 1);
						goto uscita;
					}
				
					pSiblingNode->sibling->bCurrentPageHasDirectResources = pParams->bCurrentPageHasDirectResources;
					pSiblingNode->sibling->nCurrentPageResources = pParams->nCurrentPageResources;
				
					pSiblingNode->sibling->nCurrentNumPageObjContent = pParams->nCurrentNumPageObjContent;
					pSiblingNode->sibling->bCurrentContentIsPresent = pParams->bCurrentContentIsPresent;
				
					pParams->myObjsTable[pParams->nCurrentObjNum]->Obj.pTreeNode = pSiblingNode->sibling;
				}
				else
				{
					pFirstChildNode->sibling = treeNewNode(nInt, pParams->nCurrentPageParent);
					if ( NULL == pFirstChildNode->sibling )
					{
						retValue = 0;
						snprintf(pParams->szError, 8192, "Error ParseObject: treeNewNode failed for pFirstChildNode->sibling NODE SIBLING %d.\n", nInt);
						myShowErrorMessage(pParams, pParams->szError, 1);
						goto uscita;
					}
				
					pFirstChildNode->sibling->bCurrentPageHasDirectResources = pParams->bCurrentPageHasDirectResources;
					pFirstChildNode->sibling->nCurrentPageResources = pParams->nCurrentPageResources;
				
					pFirstChildNode->sibling->nCurrentNumPageObjContent = pParams->nCurrentNumPageObjContent;
					pFirstChildNode->sibling->bCurrentContentIsPresent = pParams->bCurrentContentIsPresent;
				
					pParams->myObjsTable[pParams->nCurrentObjNum]->Obj.pTreeNode = pFirstChildNode->sibling;
				}
			}			
			
			
			
			pParams->myObjsTable[pParams->nCurrentObjNum]->Obj.numObjParent = pParams->nCurrentPageParent;
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
			wprintf(L"PAGETREE PAGES PARSING -> (obj num: %d) pParams->nCurrentPageParent = %d\n", pParams->nCurrentObjNum, pParams->nCurrentPageParent);
			#endif
	
			if ( !(pParams->bCurrentPageHasDirectResources) && (-1 == pParams->nCurrentPageResources)  )   // La pagina(o il nodo interno) eredita Resources da uno dei suoi parenti.
			{
				if ( OBJ_TYPE_PAGES == pParams->eCurrentObjType )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)					
					wprintf(L"Il nodo(obj num: %d) eredita Resources da uno dei suoi parenti.\n", pParams->nCurrentObjNum);
					#endif
				}
			}
			else if ( !(pParams->bCurrentPageHasDirectResources) && (0 == pParams->nCurrentPageResources)  ) // La pagina(o il nodo interno) non ha riferimenti a Resources.
			{
				if ( OBJ_TYPE_PAGES == pParams->eCurrentObjType )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
					wprintf(L"Il nodo(obj num: %d) non ha riferimenti a Resources\n", pParams->nCurrentObjNum);
					#endif
				}
			}
			else if ( !(pParams->bCurrentPageHasDirectResources) && pParams->nCurrentPageResources > 0 )   // Un intero > 0 indica il riferimento al numero dell'oggetto Resources.
			{
				//if ( !ParseDictionaryObject(pParams, pParams->nCurrentPageResources) )
				//{
				//	snprintf(pParams->szError, 8192, "ERRORE ParseObject -> ParseDictionaryObject\n");
				//	myShowErrorMessage(pParams, pParams->szError, 1);
				//}		
				
				if ( OBJ_TYPE_PAGES == pParams->eCurrentObjType )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)					
					wprintf(L"Il nodo(obj num: %d) ha un oggetto Resources indiretto.\n", pParams->nCurrentObjNum);
					wprintf(L"PAGETREE PAGES(NODE %d) PARSING -> pParams->nCurrentPageResources = %d 0 R\n", pParams->nCurrentObjNum, pParams->nCurrentPageResources);
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
					
					while ( myobjreflist_Dequeue(&(pParams->myGsRefList), pParams->szTemp, &(pParams->nTemp)) )
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)						
						wprintf(L"\tKey = '%s' -> GsObjectRef = %d 0 R\n", pParams->szTemp, pParams->nTemp);
						#endif
						myobjreflist_Enqueue( &(pParams->myObjsTable[pParams->nCurrentObjNum]->myGsRefList), pParams->szTemp, pParams->nTemp );
					}
				}
			}
	
			if ( pParams->bCurrentPageHasDirectResources  )   // La pagina(o il nodo interno) ha un oggetto Resources diretto.
			{
				if ( OBJ_TYPE_PAGES == pParams->eCurrentObjType )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)					
					wprintf(L"Il nodo(obj num: %d) ha un oggetto Resources diretto.\n", pParams->nCurrentObjNum);
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
					
					while ( myobjreflist_Dequeue(&(pParams->myGsRefList), pParams->szTemp, &(pParams->nTemp)) )
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)
						wprintf(L"\tKey = '%s' -> GsObjectRef = %d 0 R\n", pParams->szTemp, pParams->nTemp);
						#endif
						myobjreflist_Enqueue( &(pParams->myObjsTable[pParams->nCurrentObjNum]->myGsRefList), pParams->szTemp, pParams->nTemp );
					}
				}
			}

			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)			
			wprintf(L"\n");	
			#endif

			myobjreflist_Free(&(pParams->myXObjRefList));
			myobjreflist_Free(&(pParams->myFontsRefList));
			myobjreflist_Free(&(pParams->myGsRefList));
		}
				
		pParams->nCurrentPageNum = 0;
		
		myTreeTraversePostOrderLeafOnly(pParams->pPagesTree, pParams);
		
		pParams->nCountPageFound = pParams->nCurrentPageNum;
		
		myobjreflist_Free(&(pParams->myXObjRefList));
		myobjreflist_Free(&(pParams->myFontsRefList));
		myobjreflist_Free(&(pParams->myGsRefList));
	}


	
	nMiddlePage = 0;
								
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
		
	nCountPagesToParse = nToPage - nFromPage;
	if ( nToPage == pParams->nCountPageFound )
		nMiddlePage = (nCountPagesToParse / 2) + 1;
		
	nCountHalfPages = pParams->nCountPageFound / 2;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN) || defined(MYDEBUG_PRINT_COUNT_CONTENT_TYPE)
	wprintf(L"\n");
	#endif		
	for ( nInt = nFromPage; nInt <= nToPage; nInt++ )
	{		
		pParams->nCurrentPageNum = nInt;
		
		//wprintf(L"\n\nTotal number of Image Contents = %u;\nTotal number of content other than Image = %u\n\n", pParams->nCountImageContent, pParams->nCountNotImageContent);
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN) || defined(MYDEBUG_PRINT_COUNT_CONTENT_TYPE)
		wprintf(L"ParseObject: pParams->nCurrentPageNum = %d; nMiddlePage = %d\n", pParams->nCurrentPageNum, nMiddlePage);
		#endif
			
		if ( !(pParams->bPdfHasText) && (nCountPagesToParse > nCountHalfPages) && (nInt > nMiddlePage) )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN) || defined(MYDEBUG_PRINT_COUNT_CONTENT_TYPE)
			wprintf(L"\n\nParseObject: The first %d pages in the range %d-%d do not contain text. Stop parsing file.\n\n", ((nToPage - nFromPage) / 2) + 1, nFromPage, nToPage);
			#endif

			goto uscita;
		}
			
		//if ( pParams->nCountImageContent > 34 && 0 == pParams->nCountNotImageContent )
		//	goto uscita;
							
		//pParams->pCurrentEncodingArray = &(pParams->aUtf8CharSet[0]);
		pParams->pCurrentEncodingArray = &(pParams->aSTD_CharSet[0]);
		//pParams->pCurrentEncodingArray = &(pParams->aWIN_CharSet[0]);
		//pParams->pCurrentEncodingArray = &(pParams->aMAC_CharSet[0]);
		//pParams->pCurrentEncodingArray = &(pParams->aPDF_CharSet[0]);			
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ParseObject_FN)		
		wprintf(L"PAGINA %d -> Obj Number = %d\n", nInt, pParams->pPagesArray[nInt].numObjNumber);
		#endif
		
		//if ( pParams->pPagesArray[nInt].numObjContent <= 0 && pParams->pPagesArray[nInt].queueContentsObjRefs.count <= 0 )
		if ( pParams->pPagesArray[nInt].queueContentsObjRefs.count <= 0 )
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

		if ( NULL != pParams->myTST.pRoot )
			tstFreeRecursive(&(pParams->myTST), pParams->myTST.pRoot);			
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
		scopeFree(&(pParams->pPagesArray[nInt].myScopeHT_GsRef));
	
		
		#if defined(MYDEBUG_PRINT_ALL) || ( defined(MYDEBUG_PRINT_TST) && defined(MYPDFSEARCH_USE_TST) )
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
			//else
			//{
			//	wprintf(L"\nKey '%ls' NOT found\n", pParams->pWordsToSearchArray[idxWord]);
			//}
		}
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
		scopeFree(&(pParams->pPagesArray[nInt].myScopeHT_GsRef));
	}
	
	myintqueuelist_Free(&(pParams->myPagesQueue));
	
	myintqueuelist_Free(&(pParams->myPageLeafQueue));
	
	myobjreflist_Free(&(pParams->myXObjRefList));
	myobjreflist_Free(&(pParams->myFontsRefList));
	myobjreflist_Free(&(pParams->myGsRefList));
	
	return retValue;
}

int OnTraverseHT(const void* key, uint32_t keySize, void* data, uint32_t dataSize)
{			
	if ( NULL != data )
	{
		#if defined(_WIN64) || defined(_WIN32)
		wchar_t *pWideCharString = NULL;
		wprintf(L"KEY = '%ls' keySize = %u <-> DATA = '%ls' dataSize = %d\n", (wchar_t*)key, keySize, (wchar_t*)pWideCharString, dataSize);
		#else
		wprintf(L"KEY = '%ls' keySize = %u <->  DATA = '%s' dataSize = %d\n", (wchar_t*)key, keySize, (char*)data, dataSize);
		#endif
	}
	else
	{
		#if defined(_WIN64) || defined(_WIN32)
		wprintf(L"KEY = '%ls' keySize = %u <-> DATA = NULL\n", (wchar_t*)key, keySize);	
		#else
		wprintf(L"KEY = '%ls' keySize = %u <-> DATA = NULL\n", (wchar_t*)key, keySize);
		#endif
	}
				
	return 1;
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
	
		//pParams->bStreamStateToUnicode = 0;
		pParams->bStreamStateToUnicode = 1;
		
		if ( !contentobj(pParams) )
		//if ( !ParseStreamObject(pParams, objNum) )
		{
			//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
			//{
			//	if ( NULL != pParams->myToken.vString )
			//	{
			//		free(pParams->myToken.vString);
			//		pParams->myToken.vString = NULL;
			//	}
			//}
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

void PrintFileProva_Libero(Params *pParams, const char *pszFileName)
{
	FILE *fp;
	
	fp = fopen(pszFileName, "wb");
	if ( fp == NULL )
	{
		wprintf(L"\n\nERRORE PrintFileProva: impossibile creara il file specificato per l'output: '%s'.\n\n", pszFileName);
		return;
	}	
	
	fprintf(fp,
"%%PDF-1.4\n\
%%âãÏÓ\n\
\n\
1 0 obj \n\
<<\n\
/Kids [2 0 R]\n\
/Type /Pages\n\
/Count 1\n\
>>\n\
endobj \n\
\n\
2 0 obj \n\
<<\n\
/Resources 3 0 R\n\
/Contents [4 0 R]\n\
/Parent 1 0 R\n\
/Type /Page\n\
/MediaBox [0 0 792 1224]\n\
/ArtBox [12 83.076 779.04 1206.11]\n\
/BleedBox [12 50.003 779.04 1173.04]\n\
/CropBox [0.0 42.0 778.0 1170.0]\n\
/TrimBox [12 50.003 779.04 1173.04]\n\
>>\n\
endobj \n\
\n\
3 0 obj \n\
<<\n\
/Font\n\
<<\n\
/F1 108 0 R\n\
/F2 109 0 R\n\
/F3 110 0 R\n\
>>\n\
>>\n\
endobj \n\
\n\
108 0 obj\n\
<<\n\
   /BaseFont         /JHKDOE+HelveticaNeueLTStd-BlkCn\n\
   /Encoding         1318 0 R\n\
   /FirstChar        40\n\
   /FontDescriptor   1316 0 R\n\
   /LastChar         236\n\
   /Subtype          /Type1\n\
   /ToUnicode        1317 0 R\n\
   /Type             /Font\n\
   /Widths           [314 314 500 500 500 368 260 500 500 500 500 500 500 520 500 500 500 500 260 500 500 500 500 500 500 556 574 537 574 500 481 556 500 260 500 556 463 758 592 556 537 500 574 537 462 556 520 500 500 500 481 500 500 500 500 500 500 500 520 481 520 481 315 520 520 258 500 500 258 778 520 500 520 500 352 463 315 520 463 500 500 500 444 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 260 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 444 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 444 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 500 481 500 500 500 258]\n\
>>\n\
endobj\n\
\n\
109 0 obj\n\
<<\n\
   /BaseFont         /JHKDPE+UtopiaStd-Regular\n\
   /Encoding         1321 0 R\n\
   /FirstChar        33\n\
   /FontDescriptor   1319 0 R\n\
   /LastChar         249\n\
   /Subtype          /Type1\n\
   /ToUnicode        1320 0 R\n\
   /Type             /Font\n\
   /Widths           [231 470 470 470 758 470 224 357 357 470 470 250 368 250 423 500 500 500 500 500 500 500 500 500 500 250 250 470 470 470 422 470 635 614 655 739 577 548 710 756 333 331 643 548 894 734 725 577 725 615 512 589 706 626 896 620 470 587 470 470 470 470 470 470 498 561 467 568 490 287 490 577 275 268 497 269 869 583 543 568 561 369 411 310 573 474 722 458 470 452 470 470 470 470 470 470 470 470 840 470 470 470 470 470 470 470 470 470 424 424 262 262 470 470 470 470 470 470 470 470 470 470 276 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 438 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 438 470 470 470 470 470 470 470 470 470 470 470 470 577 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 498 498 470 470 470 470 470 470 490 490 470 470 275 470 470 470 470 470 543 543 470 470 543 470 470 573]\n\
>>\n\
endobj\n\
\n\
110 0 obj\n\
<<\n\
   /BaseFont         /JHKDPF+UtopiaStd-Italic\n\
   /Encoding         1324 0 R\n\
   /FirstChar        39\n\
   /FontDescriptor   1322 0 R\n\
   /LastChar         232\n\
   /Subtype          /Type1\n\
   /ToUnicode        1323 0 R\n\
   /Type             /Font\n\
   /Widths           [224 470 470 470 470 470 338 250 470 470 470 470 470 470 470 470 470 470 470 250 470 470 470 470 470 470 470 470 642 732 574 549 683 470 335 470 470 553 883 470 470 593 470 611 511 470 745 470 470 470 470 470 470 470 470 470 470 470 547 534 415 557 429 272 468 559 289 470 470 274 848 573 515 551 470 379 366 310 570 482 470 470 470 437 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 262 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 470 429]\n\
>>\n\
endobj\n\
\n\
1316 0 obj\n\
<<\n\
   /Ascent        712\n\
   /CapHeight     705\n\
   /CharSet       (/A/B/C/D/E/F/G/H/I/K/L/M/N/O/P/Q/R/S/T/U/V/Z/a/b/c/colon/comma/d/e/egrave/f/five/four/g/guillemotleft/guillemotright/h/hyphen/i/igrave/l/m/n/o/one/p/parenleft/parenright/period/q/quoteright/r/s/slash/space/t/three/two/u/v/z/zero)\n\
   /Descent       -225\n\
   /Flags         262148\n\
   /FontBBox      [-165 -230 1099 972]\n\
   /FontName      /JHKDOE+HelveticaNeueLTStd-BlkCn\n\
   /ItalicAngle   0\n\
   /StemH         134\n\
   /StemV         180\n\
   /FontFile3     1504 0 R\n\
   /Type          /FontDescriptor\n\
>>\n\
endobj\n\
\n\
1317 0 obj\n\
<<\n\
   /Length   341\n\
>>\n\
stream\n\
/CIDInit /ProcSet findresource begin 12 dict begin begincmap /CIDSystemInfo <<\n\
/Registry (AAAAAA+F1+0) /Ordering (T1UV) /Supplement 0 >> def\n\
/CMapName /AAAAAA+F1+0 def\n\
/CMapType 2 def\n\
1 begincodespacerange <28> <ec> endcodespacerange\n\
10 beginbfchar\n\
<35> <0035>\n\
<3a> <003A>\n\
<49> <0049>\n\
<5a> <005A>\n\
<7a> <007A>\n\
<90> <2019>\n\
<ab> <00AB>\n\
<bb> <00BB>\n\
<e8> <00E8>\n\
<ec> <00EC>\n\
endbfchar\n\
8 beginbfrange\n\
<28> <29> <0028>\n\
<2d> <2e> <002D>\n\
<41> <47> <0041>\n\
<4b> <50> <004B>\n\
<52> <56> <0052>\n\
<61> <69> <0061>\n\
<6c> <70> <006C>\n\
<72> <76> <0072>\n\
endbfrange\n\
endcmap CMapName currentdict /CMap defineresource pop end end\n\
\n\
endstream\n\
endobj\n\
\n\
1318 0 obj\n\
<<\n\
   /Differences   [40/parenleft/parenright 45/hyphen/period 53/five 58/colon 65/A/B/C/D/E/F/G 73/I 75/K/L/M/N/O/P 82/R/S/T/U/V 90/Z 97/a/b/c/d/e/f/g/h/i 108/l/m/n/o/p 114/r/s/t/u/v 122/z 144/quoteright 171/guillemotleft 187/guillemotright 232/egrave 236/igrave]\n\
   /Type          /Encoding\n\
>>\n\
endobj\n\
\n\
1319 0 obj\n\
<<\n\
   /Ascent        712\n\
   /CapHeight     727\n\
   /CharSet       (/A/B/C/D/E/Egrave/F/G/H/I/J/K/L/M/N/O/P/Q/R/S/T/U/V/W/X/Y/Z/a/aacute/agrave/asterisk/b/c/colon/comma/d/degree/e/eacute/egrave/eight/ellipsis/endash/exclam/f/five/four/g/guillemotleft/guillemotright/h/hyphen/i/igrave/j/k/l/lslash/m/n/nine/o/oacute/odieresis/ograve/one/p/parenleft/parenright/percent/period/plus/q/question/quotedblleft/quotedblright/quoteleft/quoteright/quotesingle/r/s/semicolon/seven/six/slash/space/t/three/two/u/ugrave/v/w/x/y/z/zero)\n\
   /Descent       -247\n\
   /Flags         6\n\
   /FontBBox      [-150 -252 1061 864]\n\
   /FontName      /JHKDPE+UtopiaStd-Regular\n\
   /ItalicAngle   0\n\
   /StemH         35\n\
   /StemV         91\n\
   /Type          /FontDescriptor\n\
   /FontFile3     1505 0 R\n\
   /XHeight       487\n\
>>\n\
endobj\n\
\n\
1320 0 obj\n\
<<\n\
   /Length   368\n\
>>\n\
stream\n\
/CIDInit /ProcSet findresource begin 12 dict begin begincmap /CIDSystemInfo <<\n\
/Registry (AAAAAA+F2+0) /Ordering (T1UV) /Supplement 0 >> def\n\
/CMapName /AAAAAA+F2+0 def\n\
/CMapType 2 def\n\
1 begincodespacerange <21> <f9> endcodespacerange\n\
13 beginbfchar\n\
<21> <0021>\n\
<25> <0025>\n\
<3f> <003F>\n\
<5a> <005A>\n\
<83> <2026>\n\
<85> <2013>\n\
<9b> <0142>\n\
<ab> <00AB>\n\
<bb> <00BB>\n\
<c8> <00C8>\n\
<ec> <00EC>\n\
<f6> <00F6>\n\
<f9> <00F9>\n\
endbfchar\n\
9 beginbfrange\n\
<27> <29> <0027>\n\
<2c> <3b> <002C>\n\
<41> <58> <0041>\n\
<61> <7a> <0061>\n\
<8d> <8e> <201C>\n\
<8f> <90> <2018>\n\
<e0> <e1> <00E0>\n\
<e8> <e9> <00E8>\n\
<f2> <f3> <00F2>\n\
endbfrange\n\
endcmap CMapName currentdict /CMap defineresource pop end end\n\
\n\
endstream\n\
endobj\n\
\n\
1321 0 obj\n\
<<\n\
   /Differences   [33/exclam 37/percent 39/quotesingle/parenleft/parenright 44/comma/hyphen/period/slash/zero/one/two/three/four/five/six/seven/eight/nine/colon/semicolon 63/question 65/A/B/C/D/E/F/G/H/I/J/K/L/M/N/O/P/Q/R/S/T/U/V/W/X 90/Z 97/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w/x/y/z 131/ellipsis 133/endash 141/quotedblleft/quotedblright/quoteleft/quoteright 155/lslash 171/guillemotleft 187/guillemotright 200/Egrave 224/agrave/aacute 232/egrave/eacute 236/igrave 242/ograve/oacute 246/odieresis 249/ugrave]\n\
   /Type          /Encoding\n\
>>\n\
endobj\n\
\n\
1322 0 obj\n\
<<\n\
   /Ascent      712\n\
   /CapHeight   0\n\
   /CharSet     (/A/B/C/D/E/F/G/I/K/L/M/N/P/R/S/T/U/V/W/a/agrave/b/c/colon/comma/d/e/egrave/f/five/g/h/hyphen/i/j/l/m/n/nine/o/one/p/period/quoteright/quotesingle/r/s/six/t/three/two/u/v/w/y/z)\n\
   /Descent     -247\n\
   /Flags       68\n\
   /FontBBox    [-188 -252 1058 868]\n\
   /FontName    /JHKDPF+UtopiaStd-Italic\n\
   /ItalicAngle -13\n\
   /StemH       35\n\
   /StemV       91\n\
   /FontFile3   1506 0 R\n\
   /Type        /FontDescriptor\n\
>>\n\
endobj\n\
\n\
1323 0 obj\n\
<<\n\
   /Length   326\n\
>>\n\
stream\n\
/CIDInit /ProcSet findresource begin 12 dict begin begincmap /CIDSystemInfo <<\n\
/Registry (AAAAAA+F3+0) /Ordering (T1UV) /Supplement 0 >> def\n\
/CMapName /AAAAAA+F3+0 def\n\
/CMapType 2 def\n\
1 begincodespacerange <27> <e8> endcodespacerange\n\
8 beginbfchar\n\
<27> <0027>\n\
<3a> <003A>\n\
<49> <0049>\n\
<50> <0050>\n\
<55> <0055>\n\
<7a> <007A>\n\
<90> <2019>\n\
<e8> <00E8>\n\
endbfchar\n\
7 beginbfrange\n\
<2d> <2e> <002D>\n\
<43> <47> <0043>\n\
<4c> <4d> <004C>\n\
<52> <53> <0052>\n\
<61> <69> <0061>\n\
<6c> <70> <006C>\n\
<72> <76> <0072>\n\
endbfrange\n\
endcmap CMapName currentdict /CMap defineresource pop end end\n\
\n\
endstream\n\
endobj\n\
\n\
1324 0 obj\n\
<<\n\
   /Differences   [39/quotesingle 45/hyphen/period 58/colon 67/C/D/E/F/G 73/I 76/L/M 80/P 82/R/S 85/U 97/a/b/c/d/e/f/g/h/i 108/l/m/n/o/p 114/r/s/t/u/v 122/z 144/quoteright 232/egrave]\n\
   /Type          /Encoding\n\
>>\n\
endobj\n\n");
	
	PrintThisObjectFontFile(pParams, 1504, fp);
	fprintf(fp, "\n");
	PrintThisObjectFontFile(pParams, 1505, fp);
	fprintf(fp, "\n");
	PrintThisObjectFontFile(pParams, 1506, fp);
	fprintf(fp, "\n\n");
		
	fprintf(fp,
"4 0 obj \n\
<<\n\
/Length 65\n\
>>\n\
stream\n\
BT\n\
/F1 1 Tf\n\
9.047 0 0 9.9513 46.3389 1100.0 Tm\n\
0 Tc\n\
0 Tw\n\
[(ANTONIO)-225.9(SOCCI)]TJ\n\
9.047 0.000000 0.000000 9.9513 46.338900 1090.625398 Tm\n\
(A) Tj\n\
9.047 0.000000 0.000000 9.9513 51.369032 1090.625398 Tm\n\
(N) Tj\n\
9.047 0.000000 0.000000 9.9513 56.724856 1090.625398 Tm\n\
(T) Tj\n\
9.047 0.000000 0.000000 9.9513 60.904570 1090.625398 Tm\n\
(O) Tj\n\
9.047 0.000000 0.000000 9.9513 65.934702 1090.625398 Tm\n\
(N) Tj\n\
9.047 0.000000 0.000000 9.9513 71.290526 1090.625398 Tm\n\
(I) Tj\n\
9.047 0.000000 0.000000 9.9513 73.642746 1090.625398 Tm\n\
(O) Tj\n\
\n\
9.047 0.000000 0.000000 9.9513 80.716595 1090.625398 Tm\n\
(S) Tj\n\
9.047 0.000000 0.000000 9.9513 85.574834 1090.625398 Tm\n\
(O) Tj\n\
9.047 0.000000 0.000000 9.9513 90.604966 1090.625398 Tm\n\
(C) Tj\n\
9.047 0.000000 0.000000 9.9513 95.463205 1090.625398 Tm\n\
(C) Tj\n\
9.047 0.000000 0.000000 9.9513 100.321444 1090.625398 Tm\n\
(I) Tj\n\
/F2 1 Tf\n\
9.499 0 0 9.499 46.3389 1050.5775 Tm\n\
-0.03 Tc\n\
[(Tornano)-206.8(Destra)-220.1(e)-206.7(Sinistra?)-215(Lo)-216.6(proclama)]TJ\n\
0 -1.0477 TD\n\
[(la)-178.7(copertina)-186.1(dell')]TJ\n\
6.609600 0.0 TD\n\
-0.02 Tc\n\
/F3 1 Tf\n\
(Espresso) Tj\n\
/F2 1 Tf\n\
3.3905 0.0 TD\n\
-0.03 Tc\n\
[(:)-180.0(Chi)-184.5(si)-183.5(rivede)] TJ\n\
ET \n\
BT\n\
/F2 1 Tf\n\
9.499000 0.000000 0.000000 9.499000 46.338900 1060.625398 Tm\n\
(T) Tj\n\
9.499000 0.000000 0.000000 9.499000 51.648841 1060.625398 Tm\n\
(o) Tj\n\
9.499000 0.000000 0.000000 9.499000 56.521828 1060.625398 Tm\n\
(r) Tj\n\
9.499000 0.000000 0.000000 9.499000 59.741989 1060.625398 Tm\n\
(n) Tj\n\
9.499000 0.000000 0.000000 9.499000 64.994936 1060.625398 Tm\n\
(a) Tj\n\
9.499000 0.000000 0.000000 9.499000 69.440468 1060.625398 Tm\n\
(n) Tj\n\
9.499000 0.000000 0.000000 9.499000 74.693415 1060.625398 Tm\n\
(o) Tj\n\
\n\
9.499000 0.000000 0.000000 9.499000 81.530795 1060.625398 Tm\n\
(D) Tj\n\
9.499000 0.000000 0.000000 9.499000 90.229979 1060.625398 Tm\n\
(e) Tj\n\
9.499000 0.000000 0.000000 9.499000 94.599519 1060.625398 Tm\n\
(s) Tj\n\
9.499000 0.000000 0.000000 9.499000 98.218638 1060.625398 Tm\n\
(t) Tj\n\
9.499000 0.000000 0.000000 9.499000 100.878358 1060.625398 Tm\n\
(r) Tj\n\
9.499000 0.000000 0.000000 9.499000 104.098519 1060.625398 Tm\n\
(a) Tj\n\
\n\
9.499000 0.000000 0.000000 9.499000 110.634781 1060.625398 Tm\n\
(e) Tj\n\
\n\
9.499000 0.000000 0.000000 9.499000 116.967765 1060.625398 Tm\n\
(S) Tj\n\
9.499000 0.000000 0.000000 9.499000 123.509726 1060.625398 Tm\n\
(i) Tj\n\
9.499000 0.000000 0.000000 9.499000 125.836981 1060.625398 Tm\n\
(n) Tj\n\
9.499000 0.000000 0.000000 9.499000 131.089928 1060.625398 Tm\n\
(i) Tj\n\
9.499000 0.000000 0.000000 9.499000 133.417183 1060.625398 Tm\n\
(s) Tj\n\
9.499000 0.000000 0.000000 9.499000 137.036302 1060.625398 Tm\n\
(t) Tj\n\
9.499000 0.000000 0.000000 9.499000 139.696022 1060.625398 Tm\n\
(r) Tj\n\
9.499000 0.000000 0.000000 9.499000 142.916183 1060.625398 Tm\n\
(a) Tj\n\
9.499000 0.000000 0.000000 9.499000 147.361715 1060.625398 Tm\n\
(?) Tj\n\
\n\
9.499000 0.000000 0.000000 9.499000 153.127608 1060.625398 Tm\n\
(L) Tj\n\
9.499000 0.000000 0.000000 9.499000 160.090375 1060.625398 Tm\n\
(o) Tj\n\
\n\
9.499000 0.000000 0.000000 9.499000 167.020845 1060.625398 Tm\n\
(p) Tj\n\
9.499000 0.000000 0.000000 9.499000 174.188791 1060.625398 Tm\n\
(r) Tj\n\
9.499000 0.000000 0.000000 9.499000 177.408952 1060.625398 Tm\n\
(o) Tj\n\
9.499000 0.000000 0.000000 9.499000 182.281939 1060.625398 Tm\n\
(c) Tj\n\
9.499000 0.000000 0.000000 9.499000 186.433002 1060.625398 Tm\n\
(l) Tj\n\
9.499000 0.000000 0.000000 9.499000 188.703263 1060.625398 Tm\n\
(a) Tj\n\
9.499000 0.000000 0.000000 9.499000 193.148795 1060.625398 Tm\n\
(m) Tj\n\
9.499000 0.000000 0.000000 9.499000 201.118456 1060.625398 Tm\n\
(a) Tj\n\
ET\n\
\n\
endstream \n\
endobj \n\
5 0 obj \n\
<<\n\
/Type /Catalog\n\
/Pages 1 0 R\n\
>>\n\
endobj xref\n\
0 6\n\
0000000000 65535 f \n\
0000000015 00000 n \n\
0000000074 00000 n \n\
0000000182 00000 n \n\
0000000281 00000 n \n\
0000000399 00000 n \n\
trailer\n\
\n\
<<\n\
/Root 5 0 R\n\
/Size 6\n\
>>\n\
startxref\n\
449\n\
%%%%EOF\n");
	
	fclose(fp);
}

void PrintFileProva_LaStampa(Params *pParams, const char *pszFileName)
{
	FILE *fp;
	
	fp = fopen(pszFileName, "wb");
	if ( fp == NULL )
	{
		wprintf(L"\n\nERRORE PrintFileProva: impossibile creara il file specificato per l'output: '%s'.\n\n", pszFileName);
		return;
	}	
	
	fprintf(fp,
"%%PDF-1.4\n\
%%âãÏÓ\n\
\n\
1 0 obj \n\
<<\n\
/Kids [2 0 R]\n\
/Type /Pages\n\
/Count 1\n\
>>\n\
endobj \n\
\n\
2 0 obj \n\
<<\n\
/Resources 3 0 R\n\
/Contents [4 0 R]\n\
/Parent 1 0 R\n\
/Type /Page\n\
/MediaBox [0 0 879 1263]\n\
/CropBox [0 0 879 1263]\n\
>>\n\
endobj \n\
\n\
3 0 obj \n\
<<\n\
/Font\n\
<<\n\
/R10 34 0 R\n\
/R13 27 0 R\n\
>>\n\
>>\n\
endobj \n\
\n\
34 0 obj\n\
<<\n\
   /FirstChar        65\n\
   /Subtype          /TrueType\n\
   /Type             /Font\n\
   /BaseFont         /PCPFNZ+ClarendonBT-Bold\n\
   /FontDescriptor   35 0 R\n\
   /ToUnicode        37 0 R\n\
   /Widths           [716 0 0 0 0 0 0 0 0 0 0 692 979 0 0 731 0 0 669 694]\n\
   /LastChar         84\n\
>>\n\
endobj\n\
\n\
35 0 obj\n\
<<\n\
   /Descent        -176\n\
   /MissingWidth   600\n\
   /CapHeight      705\n\
   /StemV          143\n\
   /FontFile2      36 0 R\n\
   /Type           /FontDescriptor\n\
   /Flags          4\n\
   /FontBBox       [-12 -176 959 705]\n\
   /FontName       /PCPFNZ+ClarendonBT-Bold\n\
   /ItalicAngle    0\n\
   /Ascent         705\n\
>>\n\
endobj\n\
\n\
27 0 obj\n\
<<\n\
   /FirstChar        1\n\
   /Subtype          /TrueType\n\
   /Type             /Font\n\
   /BaseFont         /MLTACB+SunDisplay-Bold\n\
   /FontDescriptor   28 0 R\n\
   /ToUnicode        30 0 R\n\
   /Widths           [783 655 640 622 610 660 320 220 466 583 320 567 441 477 341 555 669 660 232 678 596 616 600 442 452 730 658 614]\n\
   /LastChar         28\n\
>>\n\
endobj\n\
\n\
28 0 obj\n\
<<\n\
   /Descent        -16\n\
   /MissingWidth   500\n\
   /CapHeight      914\n\
   /StemV          116\n\
   /FontFile2      29 0 R\n\
   /Type           /FontDescriptor\n\
   /Flags          4\n\
   /FontBBox       [-27 -16 775 914]\n\
   /FontName       /MLTACB+SunDisplay-Bold\n\
   /ItalicAngle    0\n\
   /Ascent         914\n\
>>\n\
endobj\n\
\n\
");
	
	PrintThisObjectFontFile(pParams, 36, fp);
	fprintf(fp, "\n");
	PrintThisObjectFontFile(pParams, 37, fp);
	fprintf(fp, "\n");
	PrintThisObjectFontFile(pParams, 29, fp);
	fprintf(fp, "\n");
	PrintThisObjectFontFile(pParams, 30, fp);
	fprintf(fp, "\n");
		
	fprintf(fp,
"4 0 obj \n\
<<\n\
/Length 65\n\
>>\n\
stream\n\
BT\n\
/R10 85.5916 Tf\n\
1 0 0 1 193.495 1048.43 Tm\n\
[(LA) -160.002 (S) 46.0016 (T) 107.001 (A) -23.0017 (M) 10.0 (P)]TJ\n\
429.67 0 Td\n\
(A) Tj\n\
1 0 0 1 193.495 962.8384 Tm\n\
(L) Tj\n\
1 0 0 1 252.724387 962.8384 Tm\n\
(A) Tj\n\
1 0 0 1 327.7028 962.8384 Tm\n\
(S) Tj\n\
1 0 0 1 381.02623 962.8384 Tm\n\
(T) Tj\n\
1 0 0 1 431.268413 962.8384 Tm\n\
(A) Tj\n\
1 0 0 1 494.520751 962.8384 Tm\n\
(M) Tj\n\
1 0 0 1 577.459012 962.8384 Tm\n\
(P) Tj\n\
1 0 0 1 623.165 962.8384 Tm\n\
(A) Tj\n\
1 0 0 1 193.495 877.2468 Tm\n\
(LA STAMPA) Tj\n\
ET\n\
endstream\n\
endobj\n\
5 0 obj\n\
<<\n\
/Type /Catalog\n\
/Pages 1 0 R\n\
>>\n\
endobj xref\n\
0 6\n\
0000000000 65535 f \n\
0000000015 00000 n \n\
0000000074 00000 n \n\
0000000182 00000 n \n\
0000000281 00000 n \n\
0000000399 00000 n \n\
trailer\n\
\n\
<<\n\
/Root 5 0 R\n\
/Size 6\n\
>>\n\
startxref\n\
449\n\
%%%%EOF\n\
");
	
	fclose(fp);
}

void PrintFileProva_IlGiornale(Params *pParams, const char *pszFileName)
{
	FILE *fp;
	
	fp = fopen(pszFileName, "wb");
	if ( fp == NULL )
	{
		wprintf(L"\n\nERRORE PrintFileProva: impossibile creara il file specificato per l'output: '%s'.\n\n", pszFileName);
		return;
	}	
	
	fprintf(fp,
"%%PDF-1.4\n\
%%âãÏÓ\n\
\n\
1 0 obj \n\
<<\n\
/Kids [2 0 R]\n\
/Type /Pages\n\
/Count 1\n\
>>\n\
endobj \n\
\n\
2 0 obj \n\
<<\n\
/Resources 3 0 R\n\
/Contents [4 0 R]\n\
/Parent 1 0 R\n\
/Type /Page\n\
/MediaBox [0 0 850.44 1247]\n\
>>\n\
endobj \n\
\n\
3 0 obj \n\
<<\n\
/Font\n\
<<\n\
/C2_0 23 0 R\n\
/C2_1 16 0 R\n\
/C2_2 1000 0 R\n\
>>\n\
>>\n\
endobj \n\
\n\
23 0 obj\n\
<</BaseFont/F242HH+Chaco-Light-F242+0/DescendantFonts[82 0 R]/Encoding 80 0 R/Subtype/Type0/ToUnicode 81 0 R/Type/Font>>\n\
endobj\n\
82 0 obj\n\
<<\n\
   /BaseFont         /F242HH+Chaco-Light\n\
   /CIDSystemInfo    <<\n\
                        /Ordering     (Identity)\n\
                        /Registry     (Atex)\n\
                        /Supplement   0\n\
                     >>\n\
   /CIDToGIDMap      /Identity\n\
   /DW               1000\n\
   /FontDescriptor   83 0 R\n\
   /Subtype          /CIDFontType2\n\
   /Type             /Font\n\
   /W                [3[190 0]30[240 240]34[200 300 200]37[230 500 500]40[500 500 500]43[500 500 500]46[500 500]55[531 597 500]58[601 507 472]63[278 0]66[395 800 610]69[593 553 600]72[580 477 391]75[601 482]78[500 0]80[445 0]87[514 0]90[527 494]93[512 0]95[243 0]98[241 774 528]101[505 527]104[325 451 270]107[526 406]125[514 0]136[243 0]225[278]]\n\
>>\n\
endobj\n\
83 0 obj\n\
<<\n\
   /Ascent         690\n\
   /CapHeight      660\n\
   /Descent        310\n\
   /Flags          32\n\
   /FontBBox       [0 0 1000 1000]\n\
   /FontFile2      84 0 R\n\
   /FontName       /Chaco-Light\n\
   /ItalicAngle    0\n\
   /MissingWidth   700\n\
   /StemV          60\n\
   /Type           /FontDescriptor\n\
>>\n\
endobj\n\
16 0 obj\n\
<</BaseFont/F246HH+Chaco-Bold-F246+0/DescendantFonts[50 0 R]/Encoding 48 0 R/Subtype/Type0/ToUnicode 49 0 R/Type/Font>>\n\
endobj\n\
\n\
1000 0 obj\n\
<<\n\
/Subtype /Type1\n\
/Type /Font\n\
/BaseFont /Times-Italic\n\
>>\n\
endobj\n\
50 0 obj\n\
<</BaseFont/F246HH+Chaco-Bold/CIDSystemInfo<</Ordering(Identity)/Registry(Atex)/Supplement 0>>/CIDToGIDMap/Identity/DW 1000/FontDescriptor 51 0 R/Subtype/CIDFontType2/Type/Font/W[32[396 0]34[219 300 238]38[550 550 550]41[550 0]43[550 0]45[550 550 550]48[232 0]55[606 610 525]58[622 520 483]61[611 0]63[292 0]66[421 835 622]69[634 591]72[595 548 466]75[608 559]80[546 0]87[521 538 435]90[537 516 347]93[551 537 269]98[268 800 537]101[522 538]104[366 473 341]107[534 488 786]110[497 492 462]125[521 0]188[392 392]202[222]]>>\n\
endobj\n\
51 0 obj\n\
<</Ascent 690/CapHeight 660/Descent 310/Flags 32/FontBBox[0 0 1000 1000]/FontFile2 52 0 R/FontName/Chaco-Bold/ItalicAngle 0/MissingWidth 700/StemV 60/Type/FontDescriptor>>\n\
endobj\n\
\n\
");
	
	PrintThisObjectFontFile(pParams, 80, fp);
	fprintf(fp, "\n");
	PrintThisObjectFontFile(pParams, 81, fp);
	fprintf(fp, "\n");
	PrintThisObjectFontFile(pParams, 84, fp);
	fprintf(fp, "\n");
	
	PrintThisObjectFontFile(pParams, 48, fp);
	fprintf(fp, "\n");
	PrintThisObjectFontFile(pParams, 49, fp);
	fprintf(fp, "\n");
	PrintThisObjectFontFile(pParams, 52, fp);
	fprintf(fp, "\n");
	
	fprintf(fp,
"4 0 obj \n\
<<\n\
/Length 65\n\
>>\n\
stream\n\
0.999996 0.000000 0.000000 0.999996 0.000000 0.000000 cm\n\
BT\n\
0.0 Tr\n\
/C2_0 9.0 Tf\n\
0.996300 0.000000 0.000000 0.996300 478.024700 1000 Tm\n\
[(a) -219 (pagina)]TJ\n\
/C2_1 9.0 Tf\n\
0.996300 0.000000 0.000000 0.996300 512.197790 1000 Tm\n\
[(18)]TJ\n\
ET\n\
endstream\n\
endobj\n\
5 0 obj\n\
<<\n\
/Type /Catalog\n\
/Pages 1 0 R\n\
>>\n\
endobj xref\n\
0 6\n\
0000000000 65535 f \n\
0000000015 00000 n \n\
0000000074 00000 n \n\
0000000182 00000 n \n\
0000000281 00000 n \n\
0000000399 00000 n \n\
trailer\n\
\n\
<<\n\
/Root 5 0 R\n\
/Size 6\n\
>>\n\
startxref\n\
449\n\
%%%%EOF\n\
");
	
	fclose(fp);
}

int PrintThisObjectFontFile(Params *pParams, int objNum, FILE* fpOutput)
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
					
	unsigned char szTemp[21];
	unsigned char szTempStream[21];
	
	unsigned char *myStreamBlock = NULL;
	
	
	
	//unsigned char *pszDecodedStream = NULL;
	//unsigned long int DecodedStreamSize = 0;
	//MyContent_t myContent;
	
	
			
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
			snprintf(pParams->szError, 8192, "Errore PrintThisObjectFontFile: objNum non valido -> %d\n", objNum);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//fwprintf(fpOutput, L"Errore PrintThisObject: objNum non valido -> %d\n", objNum);
		}
		goto uscita;
	}
	
	pParams->nObjToParse = objNum;
		
	if ( !LoadFirstBlock(pParams, objNum, "PrintThisObjectFontFile") )
	{
		retValue = 0;
		goto uscita;
	}
		
	pParams->blockCurPos = 0;
		
	uStreamOffset = pParams->myObjsTable[objNum]->Obj.StreamOffset;
	uStreamLength = pParams->myObjsTable[objNum]->Obj.StreamLength;
	


	//mydictionaryqueuelist_Init(&(myContent.decodeParms), 1, 1);
	//mystringqueuelist_Init(&(myContent.queueFilters));
	//mystringqueuelist_Enqueue(&(myContent.queueFilters), "FlateDecode");
	//myContent.LengthFromPdf = uStreamLength;
	//myContent.Offset = uStreamOffset;
	
	
	
		
	k = 0;
	szTempStream[0] = '\0';
	y = 0;
	szTemp[0] = '\0';
	j = 0;
	bStreamState = 0;
	bStreamAlreadyHandled = 0;
	while ( pParams->blockLen > 0 )
	{
		//if ( OBJ_TYPE_STREAM == pParams->myObjsTable[objNum]->Obj.Type && k >= pParams->blockLen )
		//	goto uscita;
		
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
					size_t bytesRead;
					size_t bytesWrite;
					
					if ( NULL != fpOutput )
						fprintf(fpOutput, "\n");
					else
						fwprintf(stdout, L"\n");
						
					wprintf(L"\n\nTROVATA KEYWORD 'stream'. ");
									
					if ( fseek(pParams->fp, uStreamOffset, SEEK_SET) != 0 )
					{
						snprintf(pParams->szError, 8192, "ERRORE PrintThisObjectFontFile: fseek\n");
						myShowErrorMessage(pParams, pParams->szError, 1);
						retValue = 0;
						goto uscita;		
					}
					
					wprintf(L"\n\nPUNTATORE DEL FILE POSIZIONATO CON SUCCESSO ALL'OFFSET %d. MI ACCINGO A LEGGERE %d BYTE.\n", uStreamOffset, sizeof(unsigned char) * uStreamLength);
					
					myStreamBlock = (unsigned char*)malloc(sizeof(unsigned char) * uStreamLength);
					if ( NULL == myStreamBlock )
					{
						snprintf(pParams->szError, 8192, "ERRORE PrintThisObjectFontFile: malloc failed for myStreamBlock.\n");
						myShowErrorMessage(pParams, pParams->szError, 1);
						retValue = 0;
						goto uscita;		
					}
	
					bytesRead = fread(myStreamBlock, 1, sizeof(unsigned char) * uStreamLength, pParams->fp);
					if ( bytesRead != sizeof(unsigned char) * uStreamLength )
					{
						retValue = 0;
						wprintf(L"\n\nMALEDIZIONE!!! LETTI SOLTANTO %d BYTE SU %d. ", bytesRead, sizeof(unsigned char) * uStreamLength);
						goto uscita;		
					}
					
					wprintf(L"\n\nLETTI CON SUCCESSO %d BYTE. ", bytesRead);
					
					//wprintf(L"\nINIZIO STREAM:\n");
					//for ( size_t i = 0; i < bytesRead; i++ )
					//{
					//	fputc(myStreamBlock[i], fpOutput);
					//}
					//wprintf(L"\nFINE STREAM.\n");
					
					
					
					//pszDecodedStream = getDecodedStream(pParams, &DecodedStreamSize, &myContent);
					//if ( NULL == pszDecodedStream )
					//{
					//	snprintf(pParams->szError, 8192, "ERRORE PrintThisObjectFontFile: getDecodedStream\n");
					//	myShowErrorMessage(pParams, pParams->szError, 1);
					//	retValue = 0;
					//	goto uscita;
					//}
					
					//if ( NULL == fpOutput )
					//{
					//	bytesWrite = fwrite((void*)pszDecodedStream, 1, DecodedStreamSize, stdout);
					//	fwprintf(stdout, L"endstream\nendobj\n\n");
					//}
					//else
					//{
					//	bytesWrite = fwrite((void*)pszDecodedStream, 1, DecodedStreamSize, fpOutput);
					//	fprintf(fpOutput, "endstream\nendobj\n\n");
					//}
					
					if ( NULL == fpOutput )
					{
						bytesWrite = fwrite((void*)myStreamBlock, 1, bytesRead, stdout);
						fwprintf(stdout, L"endstream\nendobj\n\n");
					}
					else
					{
						bytesWrite = fwrite((void*)myStreamBlock, 1, bytesRead, fpOutput);
						fprintf(fpOutput, "endstream\nendobj\n\n");
					}
					
					if ( bytesWrite == bytesRead )
					//if ( bytesWrite == DecodedStreamSize )
					{
						wprintf(L"\n\nSCRITTI CON SUCCESSO %d BYTE.\n\n", bytesWrite);
					}
					else
					{
						int nError;
						nError = ferror(fpOutput);
						wprintf(L"\n\nACCIPICCHIA!!! SCRITTI SOLTANTO %d BYTE SU %d. nError = %d\n\n", bytesWrite, bytesRead, nError);
						perror("ERRORE FILE: ");	
					}
											
					goto uscita;
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
				fprintf(fpOutput, "\\0");
		}
		else if ( bStreamState && (c != '\n' && c != '\r') )
		{
			if ( fseek(pParams->fp, uStreamOffset + uStreamLength, SEEK_SET) != 0 )
			{
				snprintf(pParams->szError, 8192, "ERRORE PrintThisObjectFontFile: fseek\n");
				myShowErrorMessage(pParams, pParams->szError, 1);
				retValue = 0;
				goto uscita;		
			}
				
			pParams->blockLen = fread(myStreamBlock, 1, uStreamLength, pParams->fp);
			if ( pParams->blockLen == 0 )
			{
				retValue = 0;
				goto uscita;		
			}
			k = 0;
			pParams->blockCurPos = 0;
			
			bStreamState = 0;
			
			bStreamAlreadyHandled = 1;
			
			if ( NULL == fpOutput )
				fwrite(pParams->myBlock, 1, pParams->blockLen, stdout);
			else
				fwrite(pParams->myBlock, 1, pParams->blockLen, fpOutput);
			
			continue;
		}
		else if ( 's' == c )
		{
			y = 0;
			szTempStream[y++] = 's';
						
			if ( NULL == fpOutput )
				wprintf(L"s");
			else
				fprintf(fpOutput, "s");
		}
		else if ( 'e' == c )
		{
			j = 0;
			szTemp[j++] = 'e';
			if ( NULL == fpOutput )
				wprintf(L"e");
			else
				fprintf(fpOutput, "e");
		}
		else
		{
			if ( NULL == fpOutput )
				wprintf(L"%c", c);
			else
				fprintf(fpOutput, "%c", c);
		}		
		
		k++;
		if ( k >= pParams->blockLen )
		{
			pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);
			k = 0;
		}
	}
				
uscita:

	//mydictionaryqueuelist_Free(&(myContent.decodeParms));
	//mystringqueuelist_Free(&(myContent.queueFilters));	
	//if ( NULL != pszDecodedStream )
	//{
	//	free(pszDecodedStream);
	//	pszDecodedStream = NULL;
	//}


	if ( NULL != myStreamBlock )
	{
		free(myStreamBlock);
		myStreamBlock = NULL;
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
	
	mynumstacklist_Init( &(pParams->myNumStack) );
	
	/*
	if ( !LoadFirstBlock(pParams, objNum, "ParseNextObject") )
	{
		retValue = 0;
		goto uscita;
	}
	
	GetNextToken(pParams);
				
	if ( !prepagetree(pParams) )
	{
		retValue = 0;
						
		goto uscita;
	}
		
	pParams->bPrePageTreeExit = 1;
		
	mynumstacklist_Free( &(pParams->myNumStack) );
	*/
	
	// --------------------------------------------------------------------------------------------------------------------------------------
			
	if ( !LoadFirstBlock(pParams, objNum, "ParseNextObject") )
	{
		retValue = 0;
		goto uscita;
	}
		
	GetNextToken(pParams);
	
	if ( !pagetree(pParams) )
	{
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}		
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
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}		
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
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}		
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
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}
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
			if ( nSubSectionNum < pParams->myPdfTrailer.indexArraySize )
			{
				nObjNum = pParams->myPdfTrailer.pIndexArray[nSubSectionNum]->FirstObjNumber;
				nNumberOfEntries = pParams->myPdfTrailer.pIndexArray[nSubSectionNum]->NumberOfEntries;
			}
			
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
			//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
			//{
			//	if ( NULL != pParams->myToken.vString )
			//	{
			//		free(pParams->myToken.vString);
			//		pParams->myToken.vString = NULL;
			//	}
			//}
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
				if ( nSubSectionNum < pParams->myPdfTrailer.indexArraySize )
				{
					nObjNum = pParams->myPdfTrailer.pIndexArray[nSubSectionNum]->FirstObjNumber;
					nNumberOfEntries = pParams->myPdfTrailer.pIndexArray[nSubSectionNum]->NumberOfEntries;
				}
			
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
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}		
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
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}		
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
				lastInteger = pParams->myToken.vInt;
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
					pParams->pCodeSpaceRangeArray[j].nFrom = 0;
					pParams->pCodeSpaceRangeArray[j].nTo = 0;
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
				len = strnlen(pParams->myToken.vString, 1024);
				for ( i = len - 1; i >= 0; i-- ) 
				{
					if ( pParams->myToken.vString[i] >= '0' && pParams->myToken.vString[i] <= '9' ) 
					{
						myValue += (pParams->myToken.vString[i] - 48) * base;
						base = base * 16; 
					} 
					else
					{
						c = toupper(pParams->myToken.vString[i]);
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
					c = pParams->myToken.vString[0];
					if ( toupper(c) == 'D' )
					{
						c = pParams->myToken.vString[4];
						if ( toupper(c) == 'D' )
						{
							for ( i = 0; i < 4; i++ )
							{
								if ( pParams->myToken.vString[i] >= '0' && pParams->myToken.vString[i] <= '9' ) 
								{
									lead += (pParams->myToken.vString[i] - 48) * base;
									base = base * 16; 
								} 
								else
								{
									c = toupper(pParams->myToken.vString[i]);
									if ( c >= 'A' && c <= 'F' ) 
									{ 
										lead += (c - 55) * base; 
										base = base * 16;
									}
								}								
							}
							
							for ( i = 4; i < 8; i++ )
							{
								if ( pParams->myToken.vString[i] >= '0' && pParams->myToken.vString[i] <= '9' ) 
								{
									trail += (pParams->myToken.vString[i] - 48) * base;
									base = base * 16; 
								} 
								else
								{
									c = toupper(pParams->myToken.vString[i]);
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
							cidRange3 = pParams->myToken.vInt;
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
							cidChar2 = pParams->myToken.vInt;
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
							notdefRange3 = pParams->myToken.vInt;
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
							notdefChar2 = pParams->myToken.vInt;
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
							len = strnlen(pParams->myToken.vString, 1024);
							for ( i = len - 1; i >= 0; i-- ) 
							{
								if ( pParams->myToken.vString[i] >= '0' && pParams->myToken.vString[i] <= '9' ) 
								{
									myValue += (pParams->myToken.vString[i] - 48) * base;
									base = base * 16; 
								} 
								else
								{
									c = toupper(pParams->myToken.vString[i]);
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
							tnameLen = strnlen(pParams->myToken.vString, 4096);
							nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.vString, tnameLen + sizeof(char), (void*)&bfChar2, &nDataSize, &bContentAlreadyProcessed);
							if ( nRes >= 0 ) // TROVATO
							{				
								pParams->paCustomizedFont_CharSet[bfChar1] = bfChar2;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM)
								wprintf(L"ParseCMapStream: OK!!! BFCHAR -> pParams->paCustomizedFont_CharSet[%lu] = %d -> (%s)\n", bfChar1, bfChar2, pParams->myToken.vString);
								#endif																
							}
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
							else
							{
								wprintf(L"ParseCMapStream: BFCHAR -> KEY(%s) NOT FOUND\n", pParams->myToken.vString);
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
				lastInteger = pParams->myToken.vInt;
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
					pParams->pCodeSpaceRangeArray[j].nFrom = 0;
					pParams->pCodeSpaceRangeArray[j].nTo = 0;
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
				len = strnlen(pParams->myToken.vString, 1024);
				for ( i = len - 1; i >= 0; i-- ) 
				{
					if ( pParams->myToken.vString[i] >= '0' && pParams->myToken.vString[i] <= '9' ) 
					{
						myValue += (pParams->myToken.vString[i] - 48) * base;
						base = base * 16; 
					} 
					else
					{
						c = toupper(pParams->myToken.vString[i]);
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
						c = pParams->myToken.vString[0];
						if ( toupper(c) == 'D' )
						{
							c = pParams->myToken.vString[4];
							if ( toupper(c) == 'D' )
							{
								for ( i = 0; i < 4; i++ )
								{
									if ( pParams->myToken.vString[i] >= '0' && pParams->myToken.vString[i] <= '9' ) 
									{
										lead += (pParams->myToken.vString[i] - 48) * base;
										base = base * 16; 
									} 
									else
									{
										c = toupper(pParams->myToken.vString[i]);
										if ( c >= 'A' && c <= 'F' ) 
										{ 
											lead += (c - 55) * base; 
											base = base * 16;
										}
									}								
								}
							
								for ( i = 4; i < 8; i++ )
								{
									if ( pParams->myToken.vString[i] >= '0' && pParams->myToken.vString[i] <= '9' ) 
									{
										trail += (pParams->myToken.vString[i] - 48) * base;
										base = base * 16; 
									} 
									else
									{
										c = toupper(pParams->myToken.vString[i]);
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
							len = strnlen(pParams->myToken.vString, 1024);
							for ( i = len - 1; i >= 0; i-- ) 
							{
								if ( pParams->myToken.vString[i] >= '0' && pParams->myToken.vString[i] <= '9' ) 
								{
									myValue += (pParams->myToken.vString[i] - 48) * base;
									base = base * 16; 
								} 
								else
								{
									c = toupper(pParams->myToken.vString[i]);
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
								c = pParams->myToken.vString[0];
								if ( toupper(c) == 'D' )
								{
									c = pParams->myToken.vString[4];
									if ( toupper(c) == 'D' )
									{
										for ( i = 0; i < 4; i++ )
										{
											if ( pParams->myToken.vString[i] >= '0' && pParams->myToken.vString[i] <= '9' ) 
											{
												lead += (pParams->myToken.vString[i] - 48) * base;
												base = base * 16; 
											} 
											else
											{
												c = toupper(pParams->myToken.vString[i]);
												if ( c >= 'A' && c <= 'F' ) 
												{ 
													lead += (c - 55) * base; 
													base = base * 16;
												}
											}								
										}
							
										for ( i = 4; i < 8; i++ )
										{
											if ( pParams->myToken.vString[i] >= '0' && pParams->myToken.vString[i] <= '9' ) 
											{
												trail += (pParams->myToken.vString[i] - 48) * base;
												base = base * 16; 
											} 
											else
											{
												c = toupper(pParams->myToken.vString[i]);
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
							tnameLen = strnlen(pParams->myToken.vString, 4096);
							nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.vString, tnameLen + sizeof(char), (void*)&bfChar2, &nDataSize, &bContentAlreadyProcessed);
							if ( nRes >= 0 ) // TROVATO
							{				
								pParams->paCustomizedFont_CharSet[bfChar1] = bfChar2;
								
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
								wprintf(L"ParseToUnicodeStream: OK!!! BFCHAR -> pParams->paCustomizedFont_CharSet[%lu] = %u -> (%s)\n", bfChar1, bfChar2, pParams->myToken.vString);
								#endif																
							}
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM)
							else
							{
								wprintf(L"ParseToUnicodeStream: BFCHAR -> KEY(%s) NOT FOUND\n", pParams->myToken.vString);
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
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}		
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

int ParseCIDFontObject(Params *pParams, int objNum)
{	
	int retValue = 1;
			
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseCIDFontObject: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Obj.Offset;
	
	pParams->nObjToParse = objNum;
		
	pParams->bParsingFontObj = 1;
	if ( !LoadFirstBlock(pParams, objNum, "ParseCIDFontObject") )
	{
		retValue = 0;
		goto uscita;
	}	
	pParams->bParsingFontObj = 0;
	
	mynumstacklist_Init( &(pParams->myNumStack) );
	//pParams->nToUnicodeStreamObjRef = 0;
	
	mynumstacklist_Init(&(pParams->myCurrFontWidthsStack));
		
	GetNextToken(pParams);
	
	if ( !cidfontobj(pParams) )
	{
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}		
		retValue = 0;
		goto uscita;
	}
			
uscita:

	mynumstacklist_Free(&(pParams->myCurrFontWidthsStack));

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
		
	pParams->bParsingFontObj = 1;
	if ( !LoadFirstBlock(pParams, objNum, "ParseFontObject") )
	{
		retValue = 0;
		goto uscita;
	}	
	pParams->bParsingFontObj = 0;
	
	mynumstacklist_Init( &(pParams->myNumStack) );
	//pParams->nToUnicodeStreamObjRef = 0;
	
	mynumstacklist_Init(&(pParams->myCurrFontWidthsStack));
		
	GetNextToken(pParams);
	
	if ( !contentfontobj(pParams) )
	{
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}		
		retValue = 0;
		goto uscita;
	}
			
uscita:

	mynumstacklist_Free(&(pParams->myCurrFontWidthsStack));

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	return retValue;
}

int ParseGsObject(Params *pParams, int objNum)
{	
	int retValue = 1;
			
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseGsObject: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Obj.Offset;
	
	pParams->nObjToParse = objNum;
		
	pParams->bParsingGsObj = 1;
	if ( !LoadFirstBlock(pParams, objNum, "ParseGsObject") )
	{
		retValue = 0;
		goto uscita;
	}	
	pParams->bParsingGsObj = 0;
	
	mynumstacklist_Init( &(pParams->myNumStack) );
			
	GetNextToken(pParams);
	
	if ( !gsobj(pParams) )
	{
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}		
		retValue = 0;
		goto uscita;
	}
			
uscita:

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	return retValue;
}

int ParseFontWidthsArray(Params *pParams, int objNum)
{
	int retValue = 1;
			
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseFontWidthsArray: objNum non valido -> %d\n", objNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		goto uscita;
	}
	
	pParams->pReadNextChar = ReadNextChar;
	
	pParams->nCurrentParsingLengthObj = 0;
	pParams->nCurrentParsingObj = (uint32_t)objNum;
	pParams->nCurrentObjNum = objNum;
	
	pParams->nNumBytesReadFromCurrentStream = pParams->myObjsTable[objNum]->Obj.Offset;
	
	pParams->nObjToParse = objNum;
		
	if ( !LoadFirstBlock(pParams, objNum, "ParseFontWidthsArray") )
	{
		retValue = 0;
		goto uscita;
	}	
	
	mynumstacklist_Init( &(pParams->myNumStack) );
			
	GetNextToken(pParams);
	
	if ( !widthsarrayobj(pParams) )
	{
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}		
		retValue = 0;
		goto uscita;
	}
			
uscita:

	mynumstacklist_Free( &(pParams->myNumStack) );
	
	return retValue;
}

int ParseFontDescriptorObject(Params *pParams, int objNum)
{	
	int retValue = 1;
			
	if ( objNum < 1 || objNum >= pParams->myPdfTrailer.Size )
	{
		retValue = 0;
		snprintf(pParams->szError, 8192, "Errore ParseFontDescriptorObject: objNum non valido -> %d\n", objNum);
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
		
	if ( !LoadFirstBlock(pParams, objNum, "ParseFontDescriptorObject") )
	{
		retValue = 0;
		goto uscita;
	}	
	
	mynumstacklist_Init( &(pParams->myNumStack) );
	//pParams->nToUnicodeStreamObjRef = 0;
			
	GetNextToken(pParams);
	
	if ( !fontdescriptorobj(pParams) )
	{
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}		
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
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}		
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
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}
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
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}
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
		//if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.vString) )
		//{
		//	if ( NULL != pParams->myToken.vString )
		//	{
		//		free(pParams->myToken.vString);
		//		pParams->myToken.vString = NULL;
		//	}
		//}
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
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	pParams->nCurrentObjNum = pParams->myToken.vInt;
	
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
		
	//pParams->nScope++;
		
	while (pParams->myToken.Type == T_NAME)
	{				
		if ( strncmp(pParams->myToken.vString, "Pages", 1024) == 0 )
		{
			bNameIsPages = 1;			
		}
		else if ( strncmp(pParams->myToken.vString, "Version", 1024) == 0 )
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
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
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
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
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
				mynumstacklist_Push(&(pParams->myNumStack), pParams->myToken.vInt);
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_OBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			
			GetNextToken(pParams);
			
			if ( bNameIsPages && pParams->myToken.Type == T_INT_LITERAL )
			{
				mynumstacklist_Push(&(pParams->myNumStack), pParams->myToken.vInt);			
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
			strncpy(pParams->szPdfVersionFromCatalog, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
			
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

//*************************************************************************************************************************

//pagetree          : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ pagetreebody T_KW_ENDOBJ;
int pagetree(Params *pParams)
{
	//pParams->nCountPageAlreadyDone = 0;
	pParams->nCurrentPageResources = -1;
	pParams->bCurrentPageHasDirectResources = 0;

	// nCurrentPageParent               -> 0 se nodo radice. Altrimenti intero > 0 che indica il nodo genitore della pagina corrente.
	
	// nCurrentPageResources            -> 0 se la pagina non ha riferimenti a Resources;
	//                                     -1 se la pagina eredita Resources da uno dei suoi parenti;
	//                                     altrimenti un intero > 0 che indica il riferimento al numero dell'oggetto Resources.
	
	// bCurrentPageHasDirectResources   -> 1 Se risorsa diretta; 0 altrimenti.
	
	//int nCurrentPageParsingObj;
			
	pParams->bIsInXObjState = 0;
	
	pParams->nCurrentPageParent = 0;
	
	pParams->nCurrentPageNum = 0;
	
		
	pParams->szCurrKeyName[0] = '\0';
	pParams->szCurrResourcesKeyName[0] = '\0';
	
	pParams->nCurrentXObjRef = 0;
	pParams->nCurrentFontsRef = 0;
	pParams->nCurrentGsRef = 0;
		
	pParams->bXObjectKeys = 0;
	pParams->bFontsKeys = 0;
	pParams->bGsKeys = 0;	
	
	pParams->nDictionaryType = DICTIONARY_TYPE_GENERIC;
	
	if ( NULL != pParams->pszDirectFontResourceString )
	{
		free(pParams->pszDirectFontResourceString);
		pParams->pszDirectFontResourceString = NULL;
	}
	if ( NULL != pParams->pszDirectGsResourceString )
	{
		free(pParams->pszDirectGsResourceString);
		pParams->pszDirectGsResourceString = NULL;
	}
	pParams->lenDirectFontResourceString = 0;
	pParams->lenDirectGsResourceString = 0;
			
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
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
					
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	pParams->nCurrentObjNum = pParams->myToken.vInt;
	
	pParams->nCurrentPageParsingObj = pParams->myToken.vInt;
		
	if ( pParams->nObjToParse != pParams->nCurrentObjNum )
	{
		snprintf(pParams->szError, 8192, "ERRORE parsing pagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE parsing pagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		//fwprintf(pParams->fpErrors, L"ERRORE parsing pagetree: il numero %d specificato nel trailer, non corrisponde col numero corrente -> %d\n", pParams->nObjToParse, pParams->nCurrentObjNum);
		return 0;
	}
	
	GetNextToken(pParams);	
	
	myintqueuelist_Init(&(pParams->myObjsTable[pParams->nCurrentObjNum]->queueContentsObjRefs));
					
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
			
	pParams->bIsInXObjState = 0;				
	if ( pParams->nCurrentPageResources > 0 )
	{		
		pParams->nDictionaryType = DICTIONARY_TYPE_RESOURCES;
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentPageResources) )
			return 0;
			
		while ( myobjreflist_Dequeue(&(pParams->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->myXObjRefList), pParams->szTemp, pParams->nTemp);
		}
		
		while ( myobjreflist_Dequeue(&(pParams->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->myFontsRefList), pParams->szTemp, pParams->nTemp);
		}
		
		while ( myobjreflist_Dequeue(&(pParams->myGsRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->myGsRefList), pParams->szTemp, pParams->nTemp);
		}
		
		if ( NULL != pParams->pszDirectFontResourceString )
		{
			pParams->myObjsTable[pParams->nCurrentPageParsingObj]->lenDirectFontResourceString = pParams->lenDirectFontResourceString;
			pParams->myObjsTable[pParams->nCurrentPageParsingObj]->pszDirectFontResourceString = (unsigned char*)malloc(pParams->lenDirectFontResourceString + sizeof(unsigned char));
			if ( NULL == pParams->myObjsTable[pParams->nCurrentPageParsingObj]->pszDirectFontResourceString )
			{
				snprintf(pParams->szError, 8192, "ERRORE parsing pagetree: malloc failed for pParams->myObjsTable[%u]->pszDirectFontResourceString\n", pParams->nCurrentPageParsingObj);
				myShowErrorMessage(pParams, pParams->szError, 1);
				wprintf(L"ERRORE parsing pagetree: malloc failed for pParams->myObjsTable[%u]->pszDirectFontResourceString\n", pParams->nCurrentPageParsingObj);
				return 0;
			}
			memcpy(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->pszDirectFontResourceString, pParams->pszDirectFontResourceString, pParams->lenDirectFontResourceString + sizeof(unsigned char));
			free(pParams->pszDirectFontResourceString);
			pParams->pszDirectFontResourceString = NULL;
			pParams->lenDirectFontResourceString = 0;
				
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
			wprintf(L"ECCO, pagetree -> pParams->pszDirectGsResourceString -> metto in coda(pParams->myFontsRefList) il FontsRef(Key = '%s') %d della pagina(obinum = %d)\n", pParams->szDirectFontResourceName, pParams->nCurrentPageParsingObj, pParams->nCurrentPageParsingObj);
			#endif						
			myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->myFontsRefList), pParams->szDirectFontResourceName, pParams->nCurrentPageParsingObj);
		}
		
		if ( NULL != pParams->pszDirectGsResourceString )
		{
			pParams->myObjsTable[pParams->nCurrentPageParsingObj]->lenDirectGsResourceString = pParams->lenDirectGsResourceString;
			pParams->myObjsTable[pParams->nCurrentPageParsingObj]->pszDirectGsResourceString = (unsigned char*)malloc(pParams->lenDirectGsResourceString + sizeof(unsigned char));
			if ( NULL == pParams->myObjsTable[pParams->nCurrentPageParsingObj]->pszDirectGsResourceString )
			{
				snprintf(pParams->szError, 8192, "ERRORE parsing pagetree: malloc failed for pParams->myObjsTable[%u]->pszDirectGsResourceString\n", pParams->nCurrentPageParsingObj);
				myShowErrorMessage(pParams, pParams->szError, 1);
				wprintf(L"ERRORE parsing pagetree: malloc failed for pParams->myObjsTable[%u]->pszDirectGsResourceString\n", pParams->nCurrentPageParsingObj);
				return 0;
			}
			memcpy(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->pszDirectGsResourceString, pParams->pszDirectGsResourceString, pParams->lenDirectGsResourceString + sizeof(unsigned char));
			free(pParams->pszDirectGsResourceString);
			pParams->pszDirectGsResourceString = NULL;
			pParams->lenDirectGsResourceString = 0;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
			wprintf(L"ECCO, pagetree -> pParams->pszDirectGsResourceString -> metto in coda(pParams->myGsRefList) il GsRef(Key = '%s') %d della pagina(obinum = %d)\n", pParams->szDirectGsResourceName, pParams->nCurrentPageParsingObj, pParams->nCurrentPageParsingObj);
			#endif						
			myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->myGsRefList), pParams->szDirectGsResourceName, pParams->nCurrentPageParsingObj);
		}
	}
		
	if ( pParams->nCurrentXObjRef > 0 )
	{
		pParams->nDictionaryType = DICTIONARY_TYPE_XOBJ;
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentXObjRef) )
			return 0;
		
		while ( myobjreflist_Dequeue(&(pParams->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->myXObjRefList), pParams->szTemp, pParams->nTemp);
		}
	}
	
	if ( pParams->nCurrentFontsRef > 0 )
	{
		pParams->nDictionaryType = DICTIONARY_TYPE_FONT;
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentFontsRef) )
			return 0;
			
		while ( myobjreflist_Dequeue(&(pParams->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->myFontsRefList), pParams->szTemp, pParams->nTemp);
		}		
	}	
	
	if ( pParams->nCurrentGsRef > 0 )
	{
		pParams->nDictionaryType = DICTIONARY_TYPE_GS;
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentGsRef) )
			return 0;
			
		while ( myobjreflist_Dequeue(&(pParams->myGsRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->myGsRefList), pParams->szTemp, pParams->nTemp);
		}	
	}
	
	pParams->nCurrentXObjRef = 0;
	pParams->nCurrentFontsRef = 0;
	pParams->nCurrentGsRef = 0;
						
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
		strncpy(pParams->szCurrKeyName, pParams->myToken.vString, 4096 - 1);
						
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
		
		GetNextToken(pParams);
		
		if ( (T_INT_LITERAL == pParams->myToken.Type) && (strncmp(pParams->szCurrKeyName, "Resources", 4096) == 0) )
		{
			pParams->bCurrentPageHasDirectResources = 0;
			pParams->nCurrentPageResources = pParams->myToken.vInt;
		}

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
	int iNum = -1;
	//int iGen = -1;
	
	switch ( pParams->myToken.Type )
	{
		case T_NAME:							
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			break;
		case T_INT_LITERAL:
			mynumstacklist_Push(&(pParams->myNumStack), pParams->myToken.vInt);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
	
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL )
			{
				mynumstacklist_Push(&(pParams->myNumStack), pParams->myToken.vInt);
				
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
					//pParams->pPagesArray[pParams->nCountPageFound].numObjContent = iNum;
					pParams->nCurrentNumPageObjContent = iNum;
					myintqueuelist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentObjNum]->queueContentsObjRefs), iNum);
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
						pParams->bGsKeys = 0;
					}
					else
					{
						if ( pParams->bXObjectKeys )
						{
							if ( strncmp(pParams->szCurrResourcesKeyName, "XObject", 4096) != 0  )
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
								wprintf(L"ECCO, pagetreeobj -> metto in coda(pParams->myXObjRefList) l'XObjRef(Key = '%s') %d della pagina(objnum = %d)\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentParsingObj);
								#endif						
								//myobjreflist_Enqueue(&(pParams->myXObjRefList), pParams->szCurrResourcesKeyName, iNum);
								myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentParsingObj]->myXObjRefList), pParams->szCurrResourcesKeyName, iNum);
							}
							else
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
								wprintf(L"ECCO, pagetreeobj -> NON metto in coda pParams->nCurrentXObjRef %d della pagina(objnum %d)\n", iNum, pParams->nCurrentParsingObj);
								#endif									
								pParams->nCurrentXObjRef = iNum;
								myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentParsingObj]->myXObjRefList), pParams->szCurrResourcesKeyName, iNum);
							}							
						}
						else
						{
							if ( strncmp(pParams->szCurrResourcesKeyName, "XObject", 4096) == 0  )
							{
								pParams->nCurrentXObjRef = iNum;
								myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentParsingObj]->myXObjRefList), pParams->szCurrResourcesKeyName, iNum);
							}
						}
					
						if ( pParams->bFontsKeys )
						{
							if ( strncmp(pParams->szCurrResourcesKeyName, "Font", 4096) != 0  )
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
								wprintf(L"ECCO, pagetreeobj -> metto in coda(pParams->myFontsRefList) il FontsRef(Key = '%s') %d della pagina(objnum = %d)\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentParsingObj);
								#endif						
								//myobjreflist_Enqueue(&(pParams->myFontsRefList), pParams->szCurrResourcesKeyName, iNum);
								myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentParsingObj]->myFontsRefList), pParams->szCurrResourcesKeyName, iNum);
							}
							else
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
								wprintf(L"ECCO, pagetreeobj -> NON metto in coda pParams->nCurrentFontsRef %d della pagina(objnum = %d)\n", iNum, pParams->nCurrentParsingObj);
								#endif	
								pParams->nCurrentFontsRef = iNum;
								myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentParsingObj]->myFontsRefList), pParams->szCurrResourcesKeyName, iNum);
							}
						}
						else
						{					
							//if ( strncmp(pParams->szCurrResourcesKeyName, "Font", 4096) == 0  )
							if ( strncmp(pParams->szCurrKeyName, "Font", 4096) == 0  )
							{
								pParams->nCurrentFontsRef = iNum;
								myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentParsingObj]->myFontsRefList), pParams->szCurrResourcesKeyName, iNum);
							}
						}
						
						if ( pParams->bGsKeys )
						{
							if ( strncmp(pParams->szCurrResourcesKeyName, "ExtGState", 4096) != 0  )
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
								wprintf(L"ECCO, pagetreeobj -> metto in coda(pParams->myGsRefList) il GsRef(Key = '%s') %d della pagina(objnum = %d)\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentParsingObj);
								#endif						
								//myobjreflist_Enqueue(&(pParams->myGsRefList), pParams->szCurrResourcesKeyName, iNum);
								myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentParsingObj]->myGsRefList), pParams->szCurrResourcesKeyName, iNum);
							}
							else
							{
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)
								wprintf(L"ECCO, pagetreeobj -> NON metto in coda pParams->nCurrentGsRef %d della pagina(objnum = %d)\n", iNum, pParams->nCurrentParsingObj);
								#endif	
								pParams->nCurrentGsRef = iNum;
								myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentParsingObj]->myGsRefList), pParams->szCurrResourcesKeyName, iNum);
							}
						}
						else
						{
							//if ( strncmp(pParams->szCurrResourcesKeyName, "ExtGState", 4096) == 0  )
							if ( strncmp(pParams->szCurrKeyName, "ExtGState", 4096) == 0  )
							{
								pParams->nCurrentGsRef = iNum;
								myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentParsingObj]->myGsRefList), pParams->szCurrResourcesKeyName, iNum);
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
			nInt = pParams->myToken.vInt;
			
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
			nInt = pParams->myToken.vInt;
			
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
			wprintf(L"ECCO, Metto in coda il contenuto Obj(%d) della pagina(objnum = %d)\n", nInt, pParams->nCurrentParsingObj);
			#endif
				
			myintqueuelist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentObjNum]->queueContentsObjRefs), nInt);
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
				
				fwprintf(pParams->fpErrors, L"\n***** ECCO L'OGGETTO SCHIFOSO:\n");
				PrintThisObject(pParams, pParams->nCurrentParsingObj, 0, 0, pParams->fpErrors);
				fwprintf(pParams->fpErrors, L"\n***** FINE OGGETTO SCHIFOSO.\n");
	
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
		strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.vString, 4096 - 1);
		
		if ( strncmp(pParams->szCurrKeyName, "Resources", 4096) == 0 )
		{	
			pParams->bCurrentPageHasDirectResources = 1;
						
			if ( strncmp(pParams->szCurrResourcesKeyName, "XObject", 4096) == 0  )
			{
				pParams->bXObjectKeys = 1;
				pParams->bFontsKeys = 0;
				pParams->bGsKeys = 0;
			}
			
			if ( strncmp(pParams->szCurrResourcesKeyName, "Font", 4096) == 0  )
			{
				pParams->bFontsKeys = 1;
				pParams->bXObjectKeys = 0;
				pParams->bGsKeys = 0;
			}
			
			if ( strncmp(pParams->szCurrResourcesKeyName, "ExtGState", 4096) == 0  )
			{
				pParams->bGsKeys = 1;
				pParams->bFontsKeys = 0;
				pParams->bXObjectKeys = 0;
			}		
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
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
				
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
	strncpy(pParams->szCurrKeyName, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
	
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
		strncpy(pParams->szCurrKeyName, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
		
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
			n1 = pParams->myToken.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.vInt;
				
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
				else if ( strncmp(pParams->szCurrKeyName, "WMode", 1024) == 0 )
				{
					pParams->nCurrFontWritingMode = n1;
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
					//strncpy(pParams->szUseCMap, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
					pParams->nCurrentUseCMapRef = n1;
				}				
			}
			break;
		case T_NAME:
			if ( (strncmp(pParams->szCurrKeyName, "Filter", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
			{
				if ( (strncmp(pParams->myToken.vString, "FlateDecode", 1024) != 0) && pParams->bStreamStateToUnicode )
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
					strncpy(pParams->szUseCMap, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
				}
				
				mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.vString);
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
				len = strnlen(pParams->myToken.vString, 4096);
				strncpy(pParams->CurrentContent.szFileSpecifications, pParams->myToken.vString, len + 1);
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
				len = strnlen(pParams->myToken.vString, 4096);
				strncpy(pParams->CurrentContent.szFileSpecifications, pParams->myToken.vString, len + 1);
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
				len = strnlen(pParams->myToken.vString, 4096);
				strncpy(pParams->CurrentContent.szFileSpecifications, pParams->myToken.vString, len + 1);
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
					mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.vString);
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
	
	len = strnlen(pParams->myToken.vString, 1024);	
	pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));	
	if ( NULL == pParams->myDataDecodeParams.pszKey )
	{
		snprintf(pParams->szError, 8192, "ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		//fwprintf(pParams->fpErrors, L"ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		return 0;
	}
	strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.vString, len + sizeof(char));
	
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
		len = strnlen(pParams->myToken.vString, 1024);	
		pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));
		if ( NULL == pParams->myDataDecodeParams.pszKey )
		{
			snprintf(pParams->szError, 8192, "ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			//fwprintf(pParams->fpErrors, L"ERRORE contentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			return 0;
		}
		strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.vString, len + sizeof(char));
		
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
			n1 = pParams->myToken.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			
			GetNextToken(pParams);		
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.vInt;
				
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
					pParams->myDataDecodeParams.tok.vInt = n1;
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
			
	pParams->nCurrentStreamLenghtFromObjNum = pParams->myTokenLengthObj.vInt;

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
			
	pParams->nCurrentTrailerIntegerNum = pParams->myTokenLengthObj.vInt;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_INTEGEROBJ)	
	PrintToken(&(pParams->myTokenLengthObj), ' ', ' ', 1);
	#endif	
	
	GetNextTokenLengthObj(pParams);
		
	if ( !matchLengthObj(pParams, T_KW_ENDOBJ, "integer_obj") )
		return 0;
	
	return 1;
}

// ************************************************************************************************************************

int MakeDirectResourcesString(Params *pParams)
{
	if ( pParams->bMakeDirectFontResourceString || pParams->bMakeDirectGsResourceString)
	{
		int bFont = 0;
		int bGs = 0;
		size_t len;
		
		if ( pParams->bInFontObj )
		{
			bFont = 1;
		}
		else if ( pParams->bInGsObj )
		{
			bGs = 1;
		}
		
		switch ( pParams->myToken.Type )
		{
			case T_STRING_LITERAL:
				if ( bFont )
				{	
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "(", 1);
					pParams->lenDirectFontResourceString += 1;
						
					len = strnlen(pParams->lexeme, 4096);
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, pParams->lexeme, len);
					pParams->lenDirectFontResourceString += len;
						
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, ") ", 2);
					pParams->lenDirectFontResourceString += 2;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, "(", 1);
					pParams->lenDirectGsResourceString += 1;
						
					len = strnlen(pParams->lexeme, 4096);
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, pParams->lexeme, len);
					pParams->lenDirectGsResourceString += len;
						
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, ") ", 1);
					pParams->lenDirectGsResourceString += 2;
				}
			break;
			case T_STRING_HEXADECIMAL:
				if ( bFont )
				{	
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "<", 1);
					pParams->lenDirectFontResourceString += 1;
					
					len = strnlen(pParams->lexeme, 4096);
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, pParams->lexeme, len);
					pParams->lenDirectFontResourceString += len;
						
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "< ", 2);
					pParams->lenDirectFontResourceString += 2;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, ">", 1);
					pParams->lenDirectGsResourceString += 1;
						
					len = strnlen(pParams->lexeme, 4096);
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, pParams->lexeme, len);
					pParams->lenDirectGsResourceString += len;
						
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, "> ", 1);
					pParams->lenDirectGsResourceString += 2;
				}
			break;
			case T_NAME:
				if ( bFont )
				{	
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "/", 1);
					pParams->lenDirectFontResourceString += 1;
						
					len = strnlen(pParams->lexeme, 4096);
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, pParams->lexeme, len);
					pParams->lenDirectFontResourceString += len;
						
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, " ", 1);
					pParams->lenDirectFontResourceString += 1;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, "/", 1);
					pParams->lenDirectGsResourceString += 1;
						
					len = strnlen(pParams->lexeme, 4096);
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, pParams->lexeme, len);
					pParams->lenDirectGsResourceString += len;
						
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, " ", 1);
					pParams->lenDirectGsResourceString += 1;
				}
			break;
			case T_INT_LITERAL:
				if ( bFont )
				{
					len = strnlen(pParams->lexeme, 256); 
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, pParams->lexeme, len);
					pParams->lenDirectFontResourceString += len;
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, " ", 1);
					pParams->lenDirectFontResourceString += 1;
				}
				else if ( bGs )
				{
					len = strnlen(pParams->lexeme, 256);
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, pParams->lexeme, len);
					pParams->lenDirectGsResourceString += len;
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, " ", 1);
					pParams->lenDirectGsResourceString += 1;
				}
			break;
			case T_REAL_LITERAL:
				if ( bFont )
				{
					len = strnlen(pParams->lexeme, 256); 
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, pParams->lexeme, len);
					pParams->lenDirectFontResourceString += len;
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, " ", 1);
					pParams->lenDirectFontResourceString += 1;
				}
				else if ( bGs )
				{
					len = strnlen(pParams->lexeme, 256);
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, pParams->lexeme, len);
					pParams->lenDirectGsResourceString += len;
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, " ", 1);
					pParams->lenDirectGsResourceString += 1;
				}
			break;
			case T_QOPAREN:
				if ( bFont )
				{
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "[", 1);
					pParams->lenDirectFontResourceString += 1;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, "[", 1);
					pParams->lenDirectGsResourceString += 1;
				}
			break;
			case T_QCPAREN:
				if ( bFont )
				{
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "] ", 2);
					pParams->lenDirectFontResourceString += 2;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, "] ", 2);
					pParams->lenDirectGsResourceString += 2;
				}
			break;
			case T_LEFT_CURLY_BRACKET:
				if ( bFont )
				{
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "{", 1);
					pParams->lenDirectFontResourceString += 1;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, "{", 1);
					pParams->lenDirectGsResourceString += 1;
				}
			break;
			case T_RIGHT_CURLY_BRACKET:
				if ( bFont )
				{
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "} ", 2);
					pParams->lenDirectFontResourceString += 2;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, "} ", 2);
					pParams->lenDirectGsResourceString += 2;
				}
			break;
			case T_DICT_BEGIN:				
				if ( bFont )
				{
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "<<", 2);
					pParams->lenDirectFontResourceString += 2;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, "<<", 2);
					pParams->lenDirectGsResourceString += 2;
				}
			break;
			case T_DICT_END:
				if ( bFont )
				{
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, ">> ", 3);
					pParams->lenDirectFontResourceString += 3;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, ">> ", 3);
					pParams->lenDirectGsResourceString += 3;
				}					
			break;
			case T_KW_NULL:
				if ( bFont )
				{
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "null ", 5);
					pParams->lenDirectFontResourceString += 5;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, "null ", 5);
					pParams->lenDirectGsResourceString += 5;
				}
			break;
			case T_KW_FALSE:
				if ( bFont )
				{
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "false ", 6);
					pParams->lenDirectFontResourceString += 6;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, "false ", 6);
					pParams->lenDirectGsResourceString += 6;
				}
			break;
			case T_KW_TRUE:
				if ( bFont )
				{
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "true ", 5);
					pParams->lenDirectFontResourceString += 5;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, "true ", 5);
					pParams->lenDirectGsResourceString += 5;
				}
			break;
			case T_KW_R:
				if ( bFont )
				{
					memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, "R ", 2);
					pParams->lenDirectFontResourceString += 2;
				}
				else if ( bGs )
				{
					memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, "R ", 2);
					pParams->lenDirectGsResourceString += 2;
				}
			break;
			default:
			break;
		}			
	}
	
	return 1;
}

// resourcesdictionary      : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ T_DICT_BEGIN resourcesdictionarybody T_DICT_END T_KW_ENDOBJ;
int resourcesdictionary(Params *pParams)
{	
	pParams->bMakeDirectFontResourceString = 0;
	pParams->bMakeDirectGsResourceString = 0;
	
	pParams->szDirectFontResourceName[0] = '\0';
	pParams->szDirectGsResourceName[0] = '\0';
	
	pParams->bInFontObj = 0;
	pParams->bInGsObj = 0;
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE resourcesdictionary: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE resourcesdictionary: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE resourcesdictionary: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
				
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
	pParams->bGsKeys = 0;
		
	while ( pParams->myToken.Type == T_NAME )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.vString, 4096 - 1);
		
		if ( strncmp(pParams->szCurrKeyName, "Font", 4096) == 0 )	
		{
			pParams->bInFontObj = 1;
		}
		else if ( strncmp(pParams->szCurrKeyName, "ExtGState", 4096) == 0 )	
		{
			pParams->bInGsObj = 1;
		}
				
		if ( DICTIONARY_TYPE_XOBJ == pParams->nDictionaryType )
		{
			strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.vString, 4096 - 1);
			
			pParams->bXObjectKeys = 1;
			pParams->bFontsKeys = 0;
			pParams->bGsKeys = 0;
		}
		else if ( DICTIONARY_TYPE_FONT == pParams->nDictionaryType )
		{
			strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.vString, 4096 - 1);
			
			pParams->bXObjectKeys = 0;
			pParams->bFontsKeys = 1;
			pParams->bGsKeys = 0;
		}
		else if ( DICTIONARY_TYPE_GS == pParams->nDictionaryType )
		{
			strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.vString, 4096 - 1);
			
			pParams->bGsKeys = 1;
			pParams->bXObjectKeys = 0;
			pParams->bFontsKeys = 0;
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
//                          | T_INT_LITERAL
//                          | T_REAL_LITERAL
//                          | T_KW_TRUE
//                          | T_KW_FALSE
//                          | T_KW_NULL
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
			
			MakeDirectResourcesString(pParams);
						
			GetNextToken(pParams);
			
			break;
		case T_INT_LITERAL:
			mynumstacklist_Push(&(pParams->myNumStack), pParams->myToken.vInt);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
			wprintf(L"resourcesdictionaryitems -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
						
			MakeDirectResourcesString(pParams);
	
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL )
			{
				mynumstacklist_Push(&(pParams->myNumStack), pParams->myToken.vInt);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
				wprintf(L"resourcesdictionaryitems -> ");
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif				
				
				MakeDirectResourcesString(pParams);
				
				GetNextToken(pParams);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
				wprintf(L"resourcesdictionaryitems -> ");
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
						
				MakeDirectResourcesString(pParams);
						
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
					pParams->bGsKeys = 0;
				}
				
				if ( !(pParams->bIsInXObjState) )
				{		
					if ( pParams->bXObjectKeys )
					{
						if ( pParams->nCurrentPageNum > 0 )
						{
							Scope PageScope;
							int nRes;
							size_t len;
							uint32_t nDataSize;
							uint32_t bContentAlreadyProcessed;						
						
							PageScope = pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef;
							len = strnlen(pParams->szCurrResourcesKeyName, 128);
							nRes = scopeFind(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&iNum, &nDataSize, &bContentAlreadyProcessed, 1);
							if ( nRes < 0 ) // NON TROVATO
							{	
								bContentAlreadyProcessed = 0;
								nRes = scopeInsert(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&iNum, sizeof(iNum), bContentAlreadyProcessed);
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
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
							//wprintf(L"ECCO, metto in coda(pParams->myXObjRefList) l'XObjRef -> %s %d 0 R\n", pParams->szCurrResourcesKeyName, iNum);
							wprintf(L"ECCO, resourcesdictionaryitems -> metto in coda(pParams->myXObjRefList) l'XObjRef(Key = '%s') %d della pagina(objnum = %d)\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentPageParsingObj);
							#endif						
							myobjreflist_Enqueue(&(pParams->myXObjRefList), pParams->szCurrResourcesKeyName, iNum);	
						}
					}
					else
					{
						if ( strncmp(pParams->szCurrKeyName, "XObject", 4096) == 0  )
							pParams->nCurrentXObjRef = iNum;
					}					
					
					if ( pParams->bFontsKeys )
					{
						if ( pParams->nCurrentPageNum > 0 )
						{
							Scope PageScope;
							int nRes;
							size_t len;
							uint32_t nDataSize;
							uint32_t bContentAlreadyProcessed;						
						
							PageScope = pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef;
							len = strnlen(pParams->szCurrResourcesKeyName, 128);
							nRes = scopeFind(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&iNum, &nDataSize, &bContentAlreadyProcessed, 1);
							if ( nRes < 0 ) // NON TROVATO
							{	
								bContentAlreadyProcessed = 0;
								nRes = scopeInsert(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&iNum, sizeof(iNum), bContentAlreadyProcessed);
								if ( nRes )
								{
									#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
									wprintf(L"resourcesdictionaryitems -> INSERITO FONT Key = '%s' -> %d 0 R NELLO SCOPE\n", pParams->szCurrResourcesKeyName, iNum);
									#endif
								}
							}	
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
							//wprintf(L"ECCO, metto in coda(pParams->myFontsRefList) il FontsRef -> %s %d 0 R\n", pParams->szCurrResourcesKeyName, iNum);
							wprintf(L"ECCO, resourcesdictionaryitems -> metto in coda(pParams->myFontsRefList) il FontsRef(Key = '%s') %d della pagina(obinum = %d)\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentPageParsingObj);
							#endif						
							myobjreflist_Enqueue(&(pParams->myFontsRefList), pParams->szCurrResourcesKeyName, iNum);	
						}
						
						if ( pParams->bMakeDirectFontResourceString )
						{
							pParams->bMakeDirectFontResourceString = 0;
							free(pParams->pszDirectFontResourceString);
							pParams->pszDirectFontResourceString = NULL;
							pParams->lenDirectFontResourceString = 0;
						}				
					}
					else
					{
						if ( strncmp(pParams->szCurrKeyName, "Font", 4096) == 0  )
						{
							pParams->nCurrentFontsRef = iNum;
							
							if ( pParams->bMakeDirectFontResourceString )
							{
								pParams->bMakeDirectFontResourceString = 0;
								free(pParams->pszDirectFontResourceString);
								pParams->pszDirectFontResourceString = NULL;
								pParams->lenDirectFontResourceString = 0;
							}
						}
					}
					
					if ( pParams->bGsKeys )
					{
						if ( pParams->nCurrentPageNum > 0 )
						{
							Scope PageScope;
							int nRes;
							size_t len;
							uint32_t nDataSize;
							uint32_t bContentAlreadyProcessed;						
						
							PageScope = pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_GsRef;
							len = strnlen(pParams->szCurrResourcesKeyName, 128);
							nRes = scopeFind(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&iNum, &nDataSize, &bContentAlreadyProcessed, 1);
							if ( nRes < 0 ) // NON TROVATO
							{	
								bContentAlreadyProcessed = 0;
								nRes = scopeInsert(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&iNum, sizeof(iNum), bContentAlreadyProcessed);
								if ( nRes )
								{
									#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
									wprintf(L"resourcesdictionaryitems -> INSERITO GS Key = '%s' -> %d 0 R NELLO SCOPE\n", pParams->szCurrResourcesKeyName, iNum);
									#endif
								}
							}	
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
							//wprintf(L"ECCO, metto in coda(pParams->myGsRefList) il GsRef -> %s %d 0 R\n", pParams->szCurrResourcesKeyName, iNum);
							wprintf(L"ECCO, resourcesdictionaryitems -> metto in coda(pParams->myGsRefList) il GsRef(Key = '%s') %d della pagina(obinum = %d)\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentPageParsingObj);
							#endif						
							myobjreflist_Enqueue(&(pParams->myGsRefList), pParams->szCurrResourcesKeyName, iNum);
						}
						
						if ( pParams->bMakeDirectGsResourceString )
						{
							pParams->bMakeDirectGsResourceString = 0;
							free(pParams->pszDirectGsResourceString);
							pParams->pszDirectGsResourceString = NULL;
							pParams->lenDirectGsResourceString = 0;
						}
					}
					else
					{
						if ( strncmp(pParams->szCurrKeyName, "ExtGState", 4096) == 0  )
						{
							pParams->nCurrentGsRef = iNum;
							
							if ( pParams->bMakeDirectGsResourceString )
							{
								pParams->bMakeDirectGsResourceString = 0;
								free(pParams->pszDirectGsResourceString);
								pParams->pszDirectGsResourceString = NULL;
								pParams->lenDirectGsResourceString = 0;
							}
						}
					}
				}
				else
				{
					if ( pParams->bXObjectKeys )
					{
						if ( pParams->nCurrentPageNum > 0 )
						{
							Scope PageScope;
							int nRes;
							size_t len;
							uint32_t nDataSize;
							uint32_t bContentAlreadyProcessed;						
						
							PageScope = pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_XObjRef;
							len = strnlen(pParams->szCurrResourcesKeyName, 128);
							nRes = scopeFind(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&iNum, &nDataSize, &bContentAlreadyProcessed, 1);
							if ( nRes < 0 ) // NON TROVATO
							{	
								bContentAlreadyProcessed = 0;
								nRes = scopeInsert(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&iNum, sizeof(iNum), bContentAlreadyProcessed);
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
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
							//wprintf(L"ECCO, metto in coda(pParams->myXObjRefList) l'XObjRef -> %s %d 0 R\n", pParams->szCurrResourcesKeyName, iNum);
							wprintf(L"ECCO 2, resourcesdictionaryitems -> metto in coda(pParams->myXObjRefList) l'XObjRef(Key = '%s') %d della pagina(objnum = %d)\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentPageParsingObj);
							#endif						
							myobjreflist_Enqueue(&(pParams->myXObjRefList), pParams->szCurrResourcesKeyName, iNum);	
						}
					}
					else
					{
						if ( strncmp(pParams->szCurrKeyName, "XObject", 4096) == 0  )
							pParams->nCurrentXObjRef = iNum;
					}					
					
					if ( pParams->bFontsKeys )
					{
						if ( pParams->nCurrentPageNum > 0 )
						{
							Scope PageScope;
							int nRes;
							size_t len;
							uint32_t nDataSize;
							uint32_t bContentAlreadyProcessed;						
						
							PageScope = pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_FontsRef;
							len = strnlen(pParams->szCurrResourcesKeyName, 128);
							nRes = scopeFind(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&iNum, &nDataSize, &bContentAlreadyProcessed, 1);
							if ( nRes < 0 ) // NON TROVATO
							{	
								bContentAlreadyProcessed = 0;
								nRes = scopeInsert(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&iNum, sizeof(iNum), bContentAlreadyProcessed);
								if ( nRes )
								{
									#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
									wprintf(L"resourcesdictionaryitems -> INSERITO FONT Key = '%s' -> %d 0 R NELLO SCOPE\n", pParams->szCurrResourcesKeyName, iNum);
									#endif
								}
							}	
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
							//wprintf(L"ECCO, metto in coda(pParams->myFontsRefList) il FontsRef -> %s %d 0 R\n", pParams->szCurrResourcesKeyName, iNum);
							wprintf(L"ECCO 2, resourcesdictionaryitems -> metto in coda(pParams->myFontsRefList) il FontsRef(Key = '%s') %d della pagina(objnum = %d)\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentPageParsingObj);
							#endif						
							myobjreflist_Enqueue(&(pParams->myFontsRefList), pParams->szCurrResourcesKeyName, iNum);
						}
						
						if ( pParams->bMakeDirectFontResourceString )
						{
							pParams->bMakeDirectFontResourceString = 0;
							free(pParams->pszDirectFontResourceString);
							pParams->pszDirectFontResourceString = NULL;
							pParams->lenDirectFontResourceString = 0;
						}
					}
					else
					{
						if ( strncmp(pParams->szCurrKeyName, "Font", 4096) == 0  )
						{
							pParams->nCurrentFontsRef = iNum;
							
							if ( pParams->bMakeDirectFontResourceString )
							{
								pParams->bMakeDirectFontResourceString = 0;
								free(pParams->pszDirectFontResourceString);
								pParams->pszDirectFontResourceString = NULL;
								pParams->lenDirectFontResourceString = 0;
							}
						}
					}
					
					if ( pParams->bGsKeys )
					{
						if ( pParams->nCurrentPageNum > 0 )
						{
							Scope PageScope;
							int nRes;
							size_t len;
							uint32_t nDataSize;
							uint32_t bContentAlreadyProcessed;						
						
							PageScope = pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_GsRef;
							len = strnlen(pParams->szCurrResourcesKeyName, 128);
							nRes = scopeFind(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&iNum, &nDataSize, &bContentAlreadyProcessed, 1);
							if ( nRes < 0 ) // NON TROVATO
							{	
								bContentAlreadyProcessed = 0;
								nRes = scopeInsert(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&iNum, sizeof(iNum), bContentAlreadyProcessed);
								if ( nRes )
								{
									#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
									wprintf(L"resourcesdictionaryitems -> INSERITO GS Key = '%s' -> %d 0 R NELLO SCOPE\n", pParams->szCurrResourcesKeyName, iNum);
									#endif
								}
							}	
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
							//wprintf(L"ECCO, metto in coda(pParams->myGsRefList) il FontsRef -> %s %d 0 R\n", pParams->szCurrResourcesKeyName, iNum);
							wprintf(L"ECCO 2, resourcesdictionaryitems -> metto in coda(pParams->myGsRefList) il GsRef(Key = '%s') %d della pagina(objnum = %d)\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentPageParsingObj);
							#endif						
							myobjreflist_Enqueue(&(pParams->myGsRefList), pParams->szCurrResourcesKeyName, iNum);
						}
						
						if ( pParams->bMakeDirectGsResourceString )
						{
							pParams->bMakeDirectGsResourceString = 0;
							free(pParams->pszDirectGsResourceString);
							pParams->pszDirectGsResourceString = NULL;
							pParams->lenDirectGsResourceString = 0;
						}
					}
					else
					{
						if ( strncmp(pParams->szCurrKeyName, "ExtGState", 4096) == 0  )
						{
							pParams->nCurrentGsRef = iNum;
							
							if ( pParams->bMakeDirectGsResourceString )
							{
								pParams->bMakeDirectGsResourceString = 0;
								free(pParams->pszDirectGsResourceString);
								pParams->pszDirectGsResourceString = NULL;
								pParams->lenDirectGsResourceString = 0;
							}
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
			
			MakeDirectResourcesString(pParams);
			
			GetNextToken(pParams);
			break;
		case T_STRING_HEXADECIMAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
			wprintf(L"resourcesdictionaryitems -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
			
			MakeDirectResourcesString(pParams);
			
			GetNextToken(pParams);
			break;
		case T_REAL_LITERAL:
		case T_KW_FALSE:
		case T_KW_TRUE:
		case T_KW_NULL:
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
			
			MakeDirectResourcesString(pParams);
			
			GetNextToken(pParams);
			
			if ( !resourcesarrayobjs(pParams) )
				return 0;
				
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)	
			wprintf(L"resourcesdictionaryitems -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif				
			
			MakeDirectResourcesString(pParams);
			
			if ( !match(pParams, T_QCPAREN, "resourcesdictionaryitems") )
				return 0;
				
			pParams->countArrayScope = 0;
			break;
		case T_DICT_BEGIN:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
			wprintf(L"resourcesdictionaryitems -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			if ( pParams->bInFontObj )
			{
				size_t len;
				char szNum[256];
				
				pParams->bMakeDirectFontResourceString = 1;
				pParams->pszDirectFontResourceString = (unsigned char*)malloc(sizeof(unsigned char) * pParams->nSizeDirectResourceString);
				if ( NULL == pParams->pszDirectFontResourceString )
				{
					snprintf(pParams->szError, 8192, "ERROR resourcesdictionaryitems: malloc failed for pParams->pszDirectFontResourceString\n");
					myShowErrorMessage(pParams, pParams->szError, 1);
					wprintf(L"ERROR resourcesdictionaryitems: malloc failed for pParams->pszDirectFontResourceString\n");
					return 0;
				}
				pParams->lenDirectFontResourceString = 0;
				
				snprintf(szNum, 256, "%d 0 obj <<", pParams->nCurrentPageParsingObj);
				len = strnlen(szNum, 256);
				
				memcpy(pParams->pszDirectFontResourceString, szNum, len);
				pParams->lenDirectFontResourceString += len;				
			}
			else if ( pParams->bInGsObj )
			{
				size_t len;
				char szNum[256];
				
				pParams->bMakeDirectGsResourceString = 1;
				pParams->pszDirectGsResourceString = (unsigned char*)malloc(sizeof(unsigned char) * pParams->nSizeDirectResourceString);
				if ( NULL == pParams->pszDirectGsResourceString )
				{
					snprintf(pParams->szError, 8192, "ERROR resourcesdictionaryitems: malloc failed for pParams->pszDirectGsResourceString\n");
					myShowErrorMessage(pParams, pParams->szError, 1);
					wprintf(L"ERROR resourcesdictionaryitems: malloc failed for pParams->pszDirectGsResourceString\n");
					return 0;
				}
				pParams->lenDirectGsResourceString = 0;
				
				snprintf(szNum, 256, "%d 0 obj <<", pParams->nCurrentPageParsingObj);
				len = strnlen(szNum, 256);
				
				memcpy(pParams->pszDirectGsResourceString, szNum, len);
				pParams->lenDirectGsResourceString += len;
			}
			
			//MakeDirectResourcesString(pParams);
				
			GetNextToken(pParams);
			
			if ( !resourcesdictobjs(pParams) )
				return 0;
				
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
			wprintf(L"resourcesdictionaryitems -> ");
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif				
			
			MakeDirectResourcesString(pParams);
			
			if ( !match(pParams, T_DICT_END, "resourcesdictionaryitems") )
				return 0;
				
			if ( pParams->bMakeDirectFontResourceString )
			{
				memcpy(pParams->pszDirectFontResourceString + pParams->lenDirectFontResourceString, " endobj\n\0", 9);
				pParams->lenDirectFontResourceString += 8;
				
				pParams->bMakeDirectFontResourceString = 0;
			}
			else if ( pParams->bMakeDirectGsResourceString )
			{
				memcpy(pParams->pszDirectGsResourceString + pParams->lenDirectGsResourceString, " endobj\n\0", 9);
				pParams->lenDirectGsResourceString += 8;
				
				pParams->bMakeDirectGsResourceString = 0;
			}
			
			pParams->bInFontObj = 0;
			pParams->bInGsObj = 0;
				
			pParams->bXObjectKeys = 0;
			pParams->bFontsKeys = 0;
			pParams->bGsKeys = 0;
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
			
		MakeDirectResourcesString(pParams);
		
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
			pParams->bGsKeys = 0;
		}
		else if ( strncmp(pParams->szCurrKeyName, "Font", 4096) == 0 )	
		{
			pParams->bXObjectKeys = 0;
			pParams->bFontsKeys = 1;
			pParams->bGsKeys = 0;			
		}
		else if ( strncmp(pParams->szCurrKeyName, "ExtGState", 4096) == 0 )	
		{
			pParams->bXObjectKeys = 0;
			pParams->bFontsKeys = 0;
			pParams->bGsKeys = 1;
		}
	}
	
	while ( T_NAME == pParams->myToken.Type )
	{		
		strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.vString, 4096 - 1);
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"resourcesdictobjs -> ");
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		//wprintf(L"\n\npParams->szCurrKeyName = '%s'; pParams->szCurrResourcesKeyName = '%s'\n\n" , pParams->szCurrKeyName, pParams->szCurrResourcesKeyName);
		if ( pParams->bFontsKeys )
		{
			if ( '\0' == pParams->szDirectFontResourceName[0] )
				strncpy(pParams->szDirectFontResourceName, pParams->szCurrResourcesKeyName, strnlen(pParams->szCurrResourcesKeyName, 4096));
		}
		else if ( pParams->bGsKeys )
		{
			if ( '\0' == pParams->szDirectGsResourceName[0] )
				strncpy(pParams->szDirectGsResourceName, pParams->szCurrResourcesKeyName, strnlen(pParams->szCurrResourcesKeyName, 4096));
		}
			
		MakeDirectResourcesString(pParams);

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
	pParams->nCurrentXObjResourcesRef = 0;
	
	pParams->bIsInXObjState = 1;
	pParams->bIsInFontObjState = 1;
	pParams->bIsInGsObjState = 1;
	
	pParams->nCurrentXObjRef = 0;
	pParams->nCurrentFontsRef = 0;
	pParams->nCurrentGsRef = 0;
	
	pParams->nCountDecodeParams = pParams->nCountFilters = 0;
	
	pParams->bXObjIsIndirect = 1;
	pParams->bInXObj = 0;
	pParams->nXObjRef = 0;
	pParams->bXObjIsImage = 0;
	
	pParams->bFontObjIsIndirect = 1;
	pParams->bInFontObj = 0;
	pParams->nFontObjRef = 0;
	
	pParams->bGsObjIsIndirect = 1;
	pParams->bInGsObj = 0;
	pParams->nGsObjRef = 0;
		
	pParams->szCurrResourcesKeyName[0] = '\0';
	
	pParams->bInternalDict = 0;
	
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE contentxobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentxobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE contentxobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
				
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
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
		wprintf(L"contentxobj -> L'oggetto %d è un'immagine. Esco.\n\n", pParams->nCurrentParsingObj);
		#endif
	
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
	
	if ( pParams->nCurrentXObjResourcesRef > 0 )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
		wprintf(L"contentxobj -> VADO A PRENDERE LE RESOURCES DA %d 0 R\n", pParams->nCurrentXObjResourcesRef);
		#endif
		
		pParams->nDictionaryType = DICTIONARY_TYPE_RESOURCES;
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentXObjResourcesRef) )
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
	
	pParams->bInGsObj = 0;
	pParams->bIsInGsObjState = 0;
	
	pParams->szCurrResourcesKeyName[0] = '\0';
	
	if ( NULL != pParams->myDataDecodeParams.pszKey )
	{
		free(pParams->myDataDecodeParams.pszKey);
		pParams->myDataDecodeParams.pszKey = NULL;
	}	
	
	if ( pParams->nCurrentXObjRef > 0 )
	{
		pParams->nDictionaryType = DICTIONARY_TYPE_XOBJ;
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentXObjRef) )
			return 0;
		
		while ( myobjreflist_Dequeue(&(pParams->myXObjRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->myXObjRefList), pParams->szTemp, pParams->nTemp);
		}
	}
	
	if ( pParams->nCurrentFontsRef > 0 )
	{
		pParams->nDictionaryType = DICTIONARY_TYPE_FONT;
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentFontsRef) )
			return 0;
			
		while ( myobjreflist_Dequeue(&(pParams->myFontsRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->myFontsRefList), pParams->szTemp, pParams->nTemp);
		}		
	}	
	
	if ( pParams->nCurrentGsRef > 0 )
	{
		pParams->nDictionaryType = DICTIONARY_TYPE_GS;
		if ( !ParseDictionaryObject(pParams, pParams->nCurrentGsRef) )
			return 0;
			
		while ( myobjreflist_Dequeue(&(pParams->myGsRefList), pParams->szTemp, &(pParams->nTemp)) )
		{
			myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentPageParsingObj]->myGsRefList), pParams->szTemp, pParams->nTemp);
		}	
	}
	
	pParams->nCurrentXObjRef = 0;
	pParams->nCurrentFontsRef = 0;
	pParams->nCurrentGsRef = 0;
	
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
		strncpy(pParams->szCurrKeyName, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
		
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
                    | T_REAL_LITERAL
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
			n1 = pParams->myToken.vInt;
						
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			wprintf(L"xobjcontentkeyvalue -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.vInt;
							
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
				//else if ( strncmp(pParams->szCurrKeyName, "Resources", 1024) == 0 )
				//{
				//	pParams->nCurrentXObjResourcesRef = n1;
				//}
				
				if ( pParams->bInXObj )
				{
					//if ( 0 == pParams->bXObjIsIndirect )
					//{
						if ( strncmp(pParams->szCurrResourcesKeyName, "XObject", 4096) != 0  )
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
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
							wprintf(L"ECCO, xobjcontentkeyvalue -> NON metto in coda pParams->nCurrentXObjRef %d della pagina(objnum = %d)\n", n1, pParams->nCurrentParsingObj);
							#endif	
							pParams->nCurrentXObjRef = n1;
							pParams->bInXObj = 0;
							pParams->nXObjRef = n1;
							myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentParsingObj]->myXObjRefList), pParams->szCurrResourcesKeyName, n1);
						}
					//}
					//else
					//{
					//	pParams->bInXObj = 0;						
					//	pParams->nXObjRef = n1;
					//}
				}
				else if ( pParams->bInFontObj )
				{
					//if ( 0 == pParams->bFontObjIsIndirect )
					//{
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
					wprintf(L"ECCO, xobjcontentkeyvalue -> SONO IN pParams->bInFontObj(pParams->nCurrentParsingObj = %d); pParams->szCurrResourcesKeyName = <%s>\n", pParams->nCurrentParsingObj, pParams->szCurrResourcesKeyName);
					#endif
					
						if ( strncmp(pParams->szCurrResourcesKeyName, "Font", 4096) != 0  )
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
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
							wprintf(L"ECCO, xobjcontentkeyvalue -> NON metto in coda pParams->nCurrentFontsRef %d della pagina(objnum = %d)\n", n1, pParams->nCurrentParsingObj);
							#endif	
							pParams->nCurrentFontsRef = n1;
							pParams->bInFontObj = 0;
							pParams->nFontObjRef = n1;
							myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentParsingObj]->myFontsRefList), pParams->szCurrResourcesKeyName, n1);
						}
					//}
					//else
					//{
					//	pParams->bInFontObj = 0;
					//	pParams->nFontObjRef = n1;
					//}
				}
				else if ( pParams->bInGsObj )
				{
					//if ( 0 == pParams->bGsObjIsIndirect )
					//{
						if ( strncmp(pParams->szCurrResourcesKeyName, "ExtGState", 4096) != 0  )
						{
							Scope PageScope;
							int nRes;
							size_t len;
							uint32_t nDataSize;
							uint32_t bContentAlreadyProcessed;						
						
							PageScope = pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_GsRef;
							len = strnlen(pParams->szCurrResourcesKeyName, 128);
							nRes = scopeFind(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, &nDataSize, &bContentAlreadyProcessed, 1);
							//nRes = scopeFind(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_GsRef), pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, &nDataSize, &bContentAlreadyProcessed, 1);
							if ( nRes < 0 ) // NON TROVATO
							{
								bContentAlreadyProcessed = 0;
								nRes = scopeInsert(&PageScope, pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, sizeof(n1), bContentAlreadyProcessed);
								//nRes = scopeInsert(&(pParams->pPagesArray[pParams->nCurrentPageNum].myScopeHT_GsRef), pParams->szCurrResourcesKeyName, len + sizeof(char), (void*)&n1, sizeof(n1), bContentAlreadyProcessed);
								if ( nRes )
								{
									#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
									wprintf(L"xobjcontentkeyvalue -> INSERITO GS Key = '%s' -> %d %d R NELLO SCOPE\n", pParams->szCurrResourcesKeyName, n1, n2);
									#endif
								}
							}
						}
						else
						{
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
							wprintf(L"ECCO, xobjcontentkeyvalue -> NON metto in coda pParams->nCurrentGsRef %d della pagina(objnum = %d)\n", n1, pParams->nCurrentParsingObj);
							#endif	
							pParams->nCurrentGsRef = n1;
							pParams->bInGsObj = 0;
							pParams->nGsObjRef = n1;
							myobjreflist_Enqueue(&(pParams->myObjsTable[pParams->nCurrentParsingObj]->myGsRefList), pParams->szCurrResourcesKeyName, n1);
						}
					//}
					//else
					//{
					//	pParams->bInGsObj = 0;						
					//	pParams->nGsObjRef = n1;
					//}
				}
				else
				{
					//if ( ('\0' == pParams->szCurrResourcesKeyName[0]) && (strncmp(pParams->szCurrKeyName, "Resources", 1024) == 0) )
					if ( !(pParams->bInternalDict) && (strncmp(pParams->szCurrKeyName, "Resources", 1024) == 0) )
					{
						pParams->nCurrentXObjResourcesRef = n1;
					}
				}
				
			}
			break;
		case T_REAL_LITERAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			wprintf(L"xobjcontentkeyvalue -> L'oggetto %d è un'immagine. Esco.\n\n", pParams->nCurrentParsingObj);
			#endif
			// IGNORIAMO
			GetNextToken(pParams);
			break;
		case T_NAME:
			if ( (strncmp(pParams->szCurrKeyName, "Filter", 1024) == 0) || (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
			{
				mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.vString);
				if ( (strncmp(pParams->szCurrKeyName, "FFilter", 1024) == 0) )
					pParams->CurrentContent.bExternalFile = 1;
				pParams->nCountFilters++;
			}
			else if ( strncmp(pParams->szCurrKeyName, "Subtype", 1024) == 0 )
			{
				//if ( strncmp(pParams->myToken.vString, "Image", 128) == 0 )
				if ( 'I' == pParams->myToken.vString[0] )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
					wprintf(L"xobjcontentkeyvalue -> L'oggetto %d è un'immagine. Esco.\n\n", pParams->nCurrentParsingObj);
					#endif
					
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
	
	//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	//wprintf(L"\tcountOpen = %d\n", countOpen);
	//#endif
		
	ricomincia:
	while ( T_QCPAREN != pParams->myToken.Type )
	{			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
		wprintf(L"xobjcontentkeyarray -> ");	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);
		
		if ( T_QOPAREN == pParams->myToken.Type )
		{
			countOpen++;
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			//wprintf(L"\tcountOpen = %d\n", countOpen);
			#endif
		}
		else if ( T_QCPAREN == pParams->myToken.Type )
		{
			countOpen--;
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			//wprintf(L"\tcountOpen = %d\n", countOpen);
			#endif
		}
	}
	
	if ( countOpen > 0 )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
		wprintf(L"xobjcontentkeyarray -> ");
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		//wprintf(L"\tcountOpen = %d è maggiore di zero, ricomincio.\n", countOpen);
		#endif
		
		GetNextToken(pParams);
		
		if ( T_QOPAREN == pParams->myToken.Type )
		{
			countOpen++;
			//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			//wprintf(L"\tcountOpen = %d\n", countOpen);
			//#endif
		}
		else if ( T_QCPAREN == pParams->myToken.Type )
		{
			countOpen--;
			//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
			//wprintf(L"\tcountOpen = %d\n", countOpen);
			//#endif
		}
				
		goto ricomincia;
	}

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ)
	wprintf(L"xobjcontentkeyarray -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	//wprintf(L"\tcountOpen = %d non è maggiore di zero, esco.\n", countOpen);
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
	
	pParams->bInternalDict = 1;
		
	while ( pParams->myToken.Type == T_NAME )
	{	
		
		if ( strncmp(pParams->szCurrKeyName, "Resources", 1024) == 0 )
		{
			if ( strncmp(pParams->myToken.vString, "XObject", 1024) == 0)
				pParams->bInXObj = 1;
			if ( strncmp(pParams->myToken.vString, "Font", 1024) == 0)
				pParams->bInFontObj = 1;
			if ( strncmp(pParams->myToken.vString, "ExtGState", 1024) == 0)
				pParams->bInGsObj = 1;
		}
		else
		{
			pParams->bInXObj = 0;
			pParams->bInFontObj = 0;
			pParams->bInGsObj = 0;
			pParams->szCurrResourcesKeyName[0] = '\0';
		}
					
		if ( pParams->bInXObj || pParams->bInFontObj || pParams->bInGsObj )	
		{
			strncpy(pParams->szCurrResourcesKeyName, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
		}		
		
		if ( NULL != pParams->myDataDecodeParams.pszKey )
		{
			free(pParams->myDataDecodeParams.pszKey);
			pParams->myDataDecodeParams.pszKey = NULL;
		}
		len = strnlen(pParams->myToken.vString, 1024);	
		pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));
		if ( NULL == pParams->myDataDecodeParams.pszKey )
		{
			snprintf(pParams->szError, 8192, "ERRORE xobjcontentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE xobjcontentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			//fwprintf(pParams->fpErrors, L"ERRORE xobjcontentkeydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			return 0;
		}
		strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.vString, len + sizeof(char));
		
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
	
	pParams->bInternalDict = 0;
	
	pParams->bInXObj = 0;
	pParams->bInFontObj = 0;
	pParams->bInGsObj = 0;
	pParams->szCurrResourcesKeyName[0] = '\0';
	
	return 1;	
}

// ************************************************************************************************************************

// gsobj   : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ gsobjbody T_KW_ENDOBJ;
int gsobj(Params *pParams)
{
	pParams->nCurrentGsObjFontObjNum = 0;
	pParams->dCurrentGsObjFontSize = 0.0;
	
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE gsobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	GetNextToken(pParams);
			
	//if ( !match(pParams, T_INT_LITERAL, "gsobj") )
	//	return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	if ( !match(pParams, T_INT_LITERAL, "gsobj") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_KW_OBJ, "gsobj") )
		return 0;
	
	if ( !gsobjbody(pParams) )
		return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_KW_ENDOBJ, "gsobj") )
		return 0;	
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
	wprintf(L"\ngsobj -> Font Obj Num = %u; Font Size = %f\n", pParams->nCurrentGsObjFontObjNum, pParams->dCurrentGsObjFontSize);
	#endif
		
	return 1;	
}

// gsobjbody : T_DICT_BEGIN gsobjbody T_DICT_END;
int gsobjbody(Params *pParams)
{
	int countOpen = 0;
	//int countClose = 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
	wprintf(L"gsobjbody -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
		
	if ( !match(pParams, T_DICT_BEGIN, "gsobjbody") )
		return 0;
		
	countOpen = 1;
		
	ricomincia:
	while ( T_DICT_END != pParams->myToken.Type )
	{					
		if ( T_NAME == pParams->myToken.Type && strncmp(pParams->myToken.vString, "Font", 1024) == 0 )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
			wprintf(L"gsobjbody -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
			wprintf(L"gsobjbody -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			if ( !match(pParams, T_QOPAREN, "gsobjbody") )
				return 0;
			
			if ( T_INT_LITERAL != pParams->myToken.Type )
				return 0;
			pParams->nCurrentGsObjFontObjNum = pParams->myToken.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
			wprintf(L"gsobjbody -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
			wprintf(L"gsobjbody -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			if ( !match(pParams, T_INT_LITERAL, "gsobjbody") )
				return 0;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
			wprintf(L"gsobjbody -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			if ( !match(pParams, T_KW_R, "gsobjbody") )
				return 0;
				
			if ( T_INT_LITERAL == pParams->myToken.Type )
			{
				pParams->dCurrentGsObjFontSize = (double)pParams->myToken.vInt;
			}
			else if ( T_REAL_LITERAL == pParams->myToken.Type )
			{
				pParams->dCurrentGsObjFontSize = pParams->myToken.vDouble;
			}
			else
			{
				snprintf(pParams->szError, 8192, "ERRORE gsobjbody: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
				myShowErrorMessage(pParams, pParams->szError, 1);
				PrintToken(&(pParams->myToken), '\t', ' ', 1);
				return 0;
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
			wprintf(L"gsobjbody -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			if ( !match(pParams, T_QCPAREN, "gsobjbody") )
				return 0;
		}
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
		else
		{
			wprintf(L"gsobjbody -> ");	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
		}
		#endif
		
		GetNextToken(pParams);
		
		if ( T_DICT_BEGIN == pParams->myToken.Type )
			countOpen++;
		else if ( T_DICT_END == pParams->myToken.Type )
			countOpen--;
	}
	
	if ( countOpen > 0 )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
		wprintf(L"gsobjbody -> ");
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);
		
		if ( T_DICT_BEGIN == pParams->myToken.Type )
			countOpen++;
		else if ( T_DICT_END == pParams->myToken.Type )
			countOpen--;
				
		goto ricomincia;
	}

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_GSOBJ)
	wprintf(L"gsobjbody -> ");
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	
	if ( !match(pParams, T_DICT_END, "gsobjbody") )
		return 0;
		
	return 1;
}

// ************************************************************************************************************************

// widthsarrayobj         : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ widthsarrayobjbody T_KW_ENDOBJ;
int widthsarrayobj(Params *pParams)
{
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE widthsarrayobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
								
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	GetNextToken(pParams);
				
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	if ( !match(pParams, T_INT_LITERAL, "widthsarrayobj") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	if ( !match(pParams, T_KW_OBJ, "widthsarrayobj") )
		return 0;
	
	if ( !widthsarrayobjbody(pParams) )
		return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	if ( !match(pParams, T_KW_ENDOBJ, "widthsarrayobj") )
		return 0;	
		
	return 1;
}

// widthsarrayobjbody         : T_QOPAREN {T_INT_LITERAL | T_REAL_LITERAL}  T_QCPAREN;
int widthsarrayobjbody(Params *pParams)
{
	int x = 0;
	int n1;
	int n2;
	double dblWidth;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
			
	if ( !match(pParams, T_QOPAREN, "widthsarrayobjbody") )
		return 0;
		
	if ( pParams->bCurrParsingFontIsCIDFont )
	{
		// *************************************************************************************************************************************************************
		if ( strncmp(pParams->szExtArraySizeCurrKeyName, "W", 1024) == 0 )
		{
			while ( pParams->myToken.Type == T_INT_LITERAL )
			{
				//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[nCurrCharIndex] = dblWidth;
			
				n1 = pParams->myToken.vInt;
			
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif		
		
				GetNextToken(pParams);
			
				switch ( pParams->myToken.Type )
				{
					case T_QOPAREN:
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
						PrintToken(&(pParams->myToken), ' ', ' ', 1);
						#endif
							
						GetNextToken(pParams);
					
						while ( pParams->myToken.Type == T_INT_LITERAL || pParams->myToken.Type == T_REAL_LITERAL )
						{
							switch ( pParams->myToken.Type )
							{
								case T_INT_LITERAL:
								{
									dblWidth = (double)pParams->myToken.vInt * 0.001;
																		
									if ( pParams->dCurrFontMaxWidth < dblWidth )
										pParams->dCurrFontMaxWidth = dblWidth;
					
									//wprintf(L"n1 = %d\n", n1);
									//wprintf(L"\tpParams->myObjsTable[%d]->pGlyphsWidths->pWidths[%d] = %f\n", pParams->nCurrentParsingFontObjNum, n1, dblWidth);
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[n1] = dblWidth;
									n1++;
								}
								break;
								case T_REAL_LITERAL:
								{
									dblWidth = pParams->myToken.vDouble * 0.001;
														
									if ( pParams->dCurrFontMaxWidth < dblWidth )
										pParams->dCurrFontMaxWidth = dblWidth;
										
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[n1] = dblWidth;
									n1++;
								}
								break;
								default:
								{
									;
								}
								break;
							}
						
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							#endif
							
							GetNextToken(pParams);
						}
												
						if ( T_QCPAREN != pParams->myToken.Type )
						{
							snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso T_QCPAREN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
							myShowErrorMessage(pParams, pParams->szError, 1);
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							return 0;
						}
					}
					break;
					case T_INT_LITERAL:
					{
						n2 = pParams->myToken.vInt;
			
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
						PrintToken(&(pParams->myToken), ' ', ' ', 1);
						#endif
							
						GetNextToken(pParams);
					
						switch ( pParams->myToken.Type )
						{
							case T_INT_LITERAL:
							{
								dblWidth = (double)pParams->myToken.vInt * 0.001;
													
								if ( pParams->dCurrFontMaxWidth < dblWidth )
									pParams->dCurrFontMaxWidth = dblWidth;
										
								for ( int k = n1; k <= n2; k++ )
								{										
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = dblWidth;
								}
							}
							break;
							case T_REAL_LITERAL:
							{
								dblWidth = pParams->myToken.vDouble * 0.001;
													
								if ( pParams->dCurrFontMaxWidth < dblWidth )
									pParams->dCurrFontMaxWidth = dblWidth;
										
								for ( int k = n1; k <= n2; k++ )
								{										
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = dblWidth;
								}
							}
							break;
							default:
							{
								snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso T_INT_LITERAL o T_QOPAREN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
								myShowErrorMessage(pParams, pParams->szError, 1);
								PrintToken(&(pParams->myToken), ' ', ' ', 1);
								return 0;
							}
							break;
						}
					}
					break;
					default:
					{
						snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso T_INT_LITERAL o T_QOPAREN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
						myShowErrorMessage(pParams, pParams->szError, 1);
						PrintToken(&(pParams->myToken), ' ', ' ', 1);
						return 0;
					}
					break;
				}
			
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif		
		
				GetNextToken(pParams);
			}
		}
		else if ( strncmp(pParams->szExtArraySizeCurrKeyName, "W2", 1024) == 0 )
		{
			//snprintf(pParams->szError, 8192, "ERRORE fontobjcontentkeyarray: W2, PER IL MOMENTO, NON IMPLEMENTATO.\n");
			//myShowErrorMessage(pParams, pParams->szError, 1);
			//PrintToken(&(pParams->myToken), ' ', ' ', 1);
			//return 0;
			
			while ( pParams->myToken.Type == T_INT_LITERAL )
			{
				//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[nCurrCharIndex] = dblWidth;
			
				n1 = pParams->myToken.vInt;
			
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif		
		
				GetNextToken(pParams);
			
				switch ( pParams->myToken.Type )
				{
					case T_QOPAREN:
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
						PrintToken(&(pParams->myToken), ' ', ' ', 1);
						#endif
							
						GetNextToken(pParams);
					
						while ( pParams->myToken.Type == T_INT_LITERAL || pParams->myToken.Type == T_REAL_LITERAL )
						{
							switch ( pParams->myToken.Type )
							{
								case T_INT_LITERAL:
								{
									dblWidth = (double)pParams->myToken.vInt * 0.001;
																
									if ( pParams->dCurrFontMaxWidth < dblWidth )
										pParams->dCurrFontMaxWidth = dblWidth;
									
									if ( 32 == n1 )
										pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
					
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[n1] = dblWidth;
									n1++;
								}
								break;
								case T_REAL_LITERAL:
								{
									dblWidth = pParams->myToken.vDouble * 0.001;
														
									if ( pParams->dCurrFontMaxWidth < dblWidth )
										pParams->dCurrFontMaxWidth = dblWidth;
									
									if ( 32 == n1 )
										pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
										
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[n1] = dblWidth;
									n1++;
								}
								break;
								default:
								{
									;
								}
								break;
							}
						
							// IGNORIAMO I SUCCESSIVI DUE NUMERI v1, v2:
							// v1:
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							#endif	
							GetNextToken(pParams);
							if ( T_INT_LITERAL != pParams->myToken.Type && T_REAL_LITERAL != pParams->myToken.Type )
							{
								snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso NUMBER; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
								myShowErrorMessage(pParams, pParams->szError, 1);
								PrintToken(&(pParams->myToken), ' ', ' ', 1);
								return 0;
							}
							// v2:
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							#endif
							GetNextToken(pParams);
							if ( T_INT_LITERAL != pParams->myToken.Type && T_REAL_LITERAL != pParams->myToken.Type )
							{
								snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso NUMBER; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
								myShowErrorMessage(pParams, pParams->szError, 1);
								PrintToken(&(pParams->myToken), ' ', ' ', 1);
								return 0;
							}						
						}
												
						if ( T_QCPAREN != pParams->myToken.Type )
						{
							snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso T_QCPAREN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
							myShowErrorMessage(pParams, pParams->szError, 1);
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							return 0;
						}
					}
					break;
					case T_INT_LITERAL:
					{
						n2 = pParams->myToken.vInt;
			
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
						PrintToken(&(pParams->myToken), ' ', ' ', 1);
						#endif
							
						GetNextToken(pParams);
				
						switch ( pParams->myToken.Type )
						{
							case T_INT_LITERAL:
							{
								dblWidth = (double)pParams->myToken.vInt * 0.001;
												
								if ( pParams->dCurrFontMaxWidth < dblWidth )
									pParams->dCurrFontMaxWidth = dblWidth;
								
								if ( 32 == n1 )
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
										
								for ( int k = n1; k <= n2; k++ )
								{										
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = dblWidth;
								}
							
								// IGNORIAMO I SUCCESSIVI DUE NUMERI v1, v2:
								// v1:
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
								PrintToken(&(pParams->myToken), ' ', ' ', 1);
								#endif	
								GetNextToken(pParams);
								if ( T_INT_LITERAL != pParams->myToken.Type && T_REAL_LITERAL != pParams->myToken.Type )
								{	
									snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso NUMBER; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
									myShowErrorMessage(pParams, pParams->szError, 1);
									PrintToken(&(pParams->myToken), ' ', ' ', 1);
									return 0;
								}
								// v2:
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
								PrintToken(&(pParams->myToken), ' ', ' ', 1);
								#endif
								GetNextToken(pParams);
								if ( T_INT_LITERAL != pParams->myToken.Type && T_REAL_LITERAL != pParams->myToken.Type )
								{
									snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso NUMBER; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
									myShowErrorMessage(pParams, pParams->szError, 1);
									PrintToken(&(pParams->myToken), ' ', ' ', 1);
									return 0;
								}
							}
							break;
							case T_REAL_LITERAL:
							{
								dblWidth = pParams->myToken.vDouble * 0.001;
												
								if ( pParams->dCurrFontMaxWidth < dblWidth )
									pParams->dCurrFontMaxWidth = dblWidth;
								
								if ( 32 == n1 )
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
										
								for ( int k = n1; k <= n2; k++ )
								{										
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = dblWidth;
								}
							
								// IGNORIAMO I SUCCESSIVI DUE NUMERI v1, v2:
								// v1:
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
								PrintToken(&(pParams->myToken), ' ', ' ', 1);
								#endif	
								GetNextToken(pParams);
								if ( T_INT_LITERAL != pParams->myToken.Type && T_REAL_LITERAL != pParams->myToken.Type )
								{
									snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso NUMBER; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
									myShowErrorMessage(pParams, pParams->szError, 1);
									PrintToken(&(pParams->myToken), ' ', ' ', 1);
									return 0;
								}
								// v2:
								#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
								PrintToken(&(pParams->myToken), ' ', ' ', 1);
								#endif
								GetNextToken(pParams);
								if ( T_INT_LITERAL != pParams->myToken.Type && T_REAL_LITERAL != pParams->myToken.Type )
								{
									snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso NUMBER; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
									myShowErrorMessage(pParams, pParams->szError, 1);
									PrintToken(&(pParams->myToken), ' ', ' ', 1);
									return 0;
								}
							}
							break;
							default:
							{
								snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso T_INT_LITERAL o T_QOPAREN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
								myShowErrorMessage(pParams, pParams->szError, 1);
								PrintToken(&(pParams->myToken), ' ', ' ', 1);
								return 0;
							}
							break;
						}
					}
					break;
					default:
					{
						snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso T_INT_LITERAL o T_QOPAREN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
						myShowErrorMessage(pParams, pParams->szError, 1);
						PrintToken(&(pParams->myToken), ' ', ' ', 1);
						return 0;
					}
					break;
				}
			
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif		
		
				GetNextToken(pParams);
			}
		}
		else if ( strncmp(pParams->szExtArraySizeCurrKeyName, "DW2", 1024) == 0 )
		{
			if ( T_INT_LITERAL == pParams->myToken.Type )
			{
				//dblWidth = (double)pParams->myToken.vInt;
				//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW2.v = dblWidth;	
			}
			else if ( T_REAL_LITERAL == pParams->myToken.Type )
			{
				//dblWidth = pParams->myToken.vDouble;
				//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW2.v = dblWidth;
			}
			else
			{
				snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso T_INT_LITERAL o T_REAL_LITERAL; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
				myShowErrorMessage(pParams, pParams->szError, 1);
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				return 0;
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
		
			GetNextToken(pParams);
			
			if ( T_INT_LITERAL == pParams->myToken.Type )
			{
				dblWidth = (double)pParams->myToken.vInt * 0.001;
				//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW2.w1 = dblWidth;	
				pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW = dblWidth;	
			}
			else if ( T_REAL_LITERAL == pParams->myToken.Type )
			{
				dblWidth = pParams->myToken.vDouble * 0.001;
				//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW2.w1 = dblWidth;
				pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW = dblWidth;
			}
			else
			{
				snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: Atteso T_INT_LITERAL o T_REAL_LITERAL; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
				myShowErrorMessage(pParams, pParams->szError, 1);
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				return 0;
			}
		}
		// *************************************************************************************************************************************************************
		
		goto uscita;
	}
		
	while ( T_QCPAREN != pParams->myToken.Type )
	{
		switch( pParams->myToken.Type )
		{
			case T_INT_LITERAL:
			{
				double dblWidth = (double)pParams->myToken.vInt * 0.001;
				
				if ( x >= pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize )
				{
					snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: index %d out of range.\n", x);
					myShowErrorMessage(pParams, pParams->szError, 1);
					PrintToken(&(pParams->myToken), '\t', ' ', 1);
					return 0;
				}
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ_WIDTHS)
				wprintf(L"\twidthsarrayobjbody -> pParams->myObjsTable[%d]->pGlyphsWidths->pWidths[x] = %d\n", pParams->nCurrentParsingFontObjNum, x, pParams->myToken.vInt);
				#endif
									
				if ( pParams->dCurrFontMaxWidth < dblWidth )
					pParams->dCurrFontMaxWidth = dblWidth;
				
				pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[x] = dblWidth;
				x++;
			}
			break;
			case T_REAL_LITERAL:
			{
				double dblWidth = pParams->myToken.vDouble * 0.001;
				
				if ( x >= pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize )
				{
					snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: index %d out of range.\n", x);
					myShowErrorMessage(pParams, pParams->szError, 1);
					PrintToken(&(pParams->myToken), '\t', ' ', 1);
					return 0;
				}
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ_WIDTHS)
				wprintf(L"\twidthsarrayobjbody -> pParams->myObjsTable[%d]->pGlyphsWidths->pWidths[x] = %f\n", pParams->nCurrentParsingFontObjNum, x, dblWidth);
				#endif
									
				if ( pParams->dCurrFontMaxWidth < dblWidth )
					pParams->dCurrFontMaxWidth = dblWidth;
				
				pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[x] = dblWidth;
				x++;
			}
			break;
			//case T_QCPAREN:
			//	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			//	PrintToken(&(pParams->myToken), ' ', ' ', 1);
			//	#endif
			//	GetNextToken(pParams);
			//	return 1;
			//	break;
			default:
			{
				snprintf(pParams->szError, 8192, "ERRORE widthsarrayobjbody: atteso T_INT_LITERAL o T_REAL_LITERAL; trovato token n° pParams->myToken.Type.\n");
				myShowErrorMessage(pParams, pParams->szError, 1);
				PrintToken(&(pParams->myToken), '\t', ' ', 1);
				return 0;
			}
			break;
		}
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		GetNextToken(pParams);
	}
		
	uscita:
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( !match(pParams, T_QCPAREN, "widthsarrayobjbody") )
		return 0;
	
	return 1;
}

// ************************************************************************************************************************

// fontdescriptorobj         : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ fontdescriptorobjbody T_KW_ENDOBJ;
int fontdescriptorobj(Params *pParams)
{
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE fontdescriptorobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE contentfontobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		//fwprintf(pParams->fpErrors, L"ERRORE contentfontobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
	
	//pParams->myObjsTable[pParams->nCurrentParsingObj]->dFontSpaceWidth = -1.0;
	
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth = 0.0;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->AvgWidth = 0.0;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MaxWidth = 0.0;
					
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	GetNextToken(pParams);
			
	//if ( !match(pParams, T_INT_LITERAL, "contentfontobj") )
	//	return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	if ( !match(pParams, T_INT_LITERAL, "fontdescriptorobj") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_KW_OBJ, "fontdescriptorobj") )
		return 0;
	
	if ( !fontdescriptorobjbody(pParams) )
		return 0;
		
	if ( !match(pParams, T_KW_ENDOBJ, "fontdescriptorobj") )
		return 0;	
		
	return 1;
}

// fontdescriptorobjbody     : T_DICT_BEGIN fontdescriptorobjbodydictitems T_DICT_END;
int fontdescriptorobjbody(Params *pParams)
{
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_BEGIN, "fontdescriptorobjbody") )
		return 0;
	
	if ( !fontdescriptorobjbodydictitems(pParams) )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_END, "fontdescriptorobjbody") )
		return 0;
	
	return 1;
}

// fontdescriptorobjbodydictitems : {T_NAME fontdescriptorobjkeyvalue};
int fontdescriptorobjbodydictitems(Params *pParams)
{
	while ( pParams->myToken.Type == T_NAME )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
				
		GetNextToken(pParams);			
				
		if ( !fontdescriptorobjkeyvalue(pParams) )
			return 0;
	}
	
	return 1;
}

//fontdescriptorobjkeyvalue : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
//                       | T_REAL_LITERAL
//                       | T_STRING_LITERAL
//                       | T_STRING_HEXADECIMAL
//                       | T_NAME
//                       | fontdescriptorobjkeyarray
//                       | T_DICT_BEGIN fontdescriptorobjkeydict T_DICT_END
//                       ;
int fontdescriptorobjkeyvalue(Params *pParams)
{
	int n1 = -1;
	int n2 = -1;
		
	switch ( pParams->myToken.Type )
	{
		case T_INT_LITERAL:
			n1 = pParams->myToken.vInt;
						
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.vInt;
							
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				GetNextToken(pParams);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif				
				
				if ( !match(pParams, T_KW_R, "fontdescriptorobjkeyvalue") )
					return 0;
			}
			
			if ( n2 < 0 )
			{
				if ( strncmp(pParams->szCurrKeyName, "MissingWidth", 1024) == 0 )
				{
					pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth = (double)n1 * 0.001;	
				}
				else if ( strncmp(pParams->szCurrKeyName, "AvgWidth", 1024) == 0 )
				{
					pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->AvgWidth = (double)n1 * 0.001;
				}
				else if ( strncmp(pParams->szCurrKeyName, "MaxWidth", 1024) == 0 )
				{
					pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MaxWidth = (double)n1 * 0.001;
				}
			}
			
			break;
		case T_REAL_LITERAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			if ( strncmp(pParams->szCurrKeyName, "MissingWidth", 1024) == 0 )
			{
				pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth = pParams->myToken.vDouble * 0.001;
			}
			else if ( strncmp(pParams->szCurrKeyName, "AvgWidth", 1024) == 0 )
			{
				pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->AvgWidth = pParams->myToken.vDouble * 0.001;
			}
			else if ( strncmp(pParams->szCurrKeyName, "MaxWidth", 1024) == 0 )
			{
				pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MaxWidth = pParams->myToken.vDouble * 0.001;
			}
			
			GetNextToken(pParams);
			break;
		case T_STRING_LITERAL:
		case T_STRING_HEXADECIMAL:
		case T_NAME:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			break;
		case T_QOPAREN:
			if ( !fontdescriptorobjkeyarray(pParams) )
				return 0;
			break;
		case T_DICT_BEGIN:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
							
			GetNextToken(pParams);
			
			if ( !fontdescriptorobjkeydict(pParams) )
				return 0;
				
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif				
			
			if ( !match(pParams, T_DICT_END, "fontdescriptorobjkeyvalue") )
				return 0;				
			break;
		default:
			snprintf(pParams->szError, 8192, "ERRORE fontdescriptorobjkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE fontobjcontentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			//fwprintf(pParams->fpErrors, L"ERRORE fontobjcontentkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;			
			break;
	}
	
	return 1;
}

// fontdescriptorobjkeyarray : T_QOPAREN { T_INT_LITERAL | T_REAL_LITERAL} T_QCPAREN;
int fontdescriptorobjkeyarray(Params *pParams)
{
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_QOPAREN, "fontdescriptorobjkeyarray") )
	{
		return 0;
	}
		
	while ( pParams->myToken.Type == T_INT_LITERAL || pParams->myToken.Type == T_REAL_LITERAL )
	{					
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
		
		GetNextToken(pParams);		
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_QCPAREN, "fontdescriptorobjkeyarray") )
	{
		return 0;
	}	
	
	return 1;
}

// fontdescriptorobjkeydict  : {T_NAME fontdescriptorobjkeyvalue};
int fontdescriptorobjkeydict(Params *pParams)
{	
	while ( T_NAME == pParams->myToken.Type )
	{		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ)	
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif

		GetNextToken(pParams);
				
		if ( !fontdescriptorobjkeyvalue(pParams) )
			return 0;
	}
			
	return 1;
}

// ************************************************************************************************************************

// contentfontobj         : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ contentfontobjbody T_KW_ENDOBJ;
int contentfontobj(Params *pParams)
{
	int k;
	char szFontType[128];
	
	//int nWidthsArraySize = 0;
	double dblWidth = -1.0;
	int nCurrCharIndex;
		
	MyPredefinedCMapDef myData1;
	MyPredefinedCMapDef myData2;
	MyPredefinedCMapDef *pmyData1;
	MyPredefinedCMapDef *pmyData2;
	uint32_t myData1Size = 0;
	uint32_t myData2Size = 0;
	
	double dblCumWidths;
	int nCountWidthsNonZero;
	
	// pParams->myObjsTable[x]->pGlyphsWidths->pWidths
	
	pParams->nCurrentFontSubtype = FONT_SUBTYPE_Unknown;
	
	pmyData1 = &myData1;
	pmyData2 = &myData2;
		
	pParams->szExtArraySizeCurrKeyName[0] = '\0';
	//pParams->szExtArraySizeDW2CurrKeyName[0] = '\0';
	
	pParams->bEncodigArrayAlreadyInit = 0;
	pParams->bCurrentFontHasDirectEncodingArray = 0;
	pParams->nCurrentEncodingObj = 0;
	pParams->nToUnicodeStreamObjRef = 0;
	pParams->szTemp[0] = '\0';
	
	pParams->nCurrentUseCMapRef = 0;
	pParams->szUseCMap[0] = '\0';
	
	pParams->nCurrentFontCodeSpacesNum = 0;
		
	pParams->nCurrFontDescriptorRef = 0;
	pParams->szBaseFont[0] = '\0';
	
	pParams->nDescendantFontRef = 0;
	
	pParams->dCurrFontSpaceWidth = 0.0;
	pParams->dCurrFontSpaceWidthScaled = 0.0;
	pParams->dCurrFontAvgWidth = 0.0;
	pParams->dCurrFontMaxWidth = -1.0;
	
	pParams->bWisPresent = 0;
	//pParams->bW2isPresent = 0;
	
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
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
	
	pParams->nCurrentParsingFontObjNum = pParams->myToken.vInt;	
	
	//pParams->myObjsTable[pParams->nCurrentParsingObj]->dFontSpaceWidth = -1.0;	
	
	if ( NULL != pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths )
	{
		if ( NULL != pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths )
		{
			free(pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths = NULL;
		}
		
		free(pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths);
		pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths = NULL;
	}
	
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths = (GlyphsWidths*)malloc(sizeof(GlyphsWidths));
	if ( NULL == pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths )
	{
		snprintf(pParams->szError, 8192, "ERROR contentfontobj: malloc failed for pParams->myObjsTable[%d]->pGlyphsWidths.\n", pParams->nCurrentParsingFontObjNum);
		myShowErrorMessage(pParams, pParams->szError, 1);
		wprintf(L"ERROR contentfontobj: malloc failed for pParams->myObjsTable[%d]->pGlyphsWidths.\n", pParams->nCurrentParsingFontObjNum);
		return 0;
	}
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths = NULL;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize = 0;
		
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar = -1;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar = -1;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth = 0.0;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->AvgWidth = 0.0;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MaxWidth = 0.0;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = -1.0;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WritingMode = WRITING_MODE_HORIZONTAL;
	
	pParams->nCurrFontWritingMode = WRITING_MODE_HORIZONTAL;
	
	pParams->nCurrentParsingFontWidthsArrayRef = 0;

	pParams->bFontWidthsArrayState = 0;
	
	pParams->nCurrentBaseFont = BASEFONT_NoPredef;
								
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
				if ( strncmp(pParams->szUseCMap, "Identity-H", strnlen(pParams->szUseCMap, 4096) + 1) == 0 ) 
				{
					pParams->pCurrentEncodingArray = &(pParams->pArrayUnicode[0]);
					
					pParams->bHasCodeSpaceOneByte = 0;
					pParams->bHasCodeSpaceTwoByte = 1;
															
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					wprintf(L"\t***** USECMAP PREDEFINITO 0 -> '%s'. OK! *****\n\n", pParams->szUseCMap);
					#endif
					
					goto uscita;
					//return 1;
				}
				else if ( strncmp(pParams->szUseCMap, "Identity-V", strnlen(pParams->szUseCMap, 4096) + 1) == 0 )
				{
					pParams->pCurrentEncodingArray = &(pParams->pArrayUnicode[0]);
					
					pParams->bHasCodeSpaceOneByte = 0;
					pParams->bHasCodeSpaceTwoByte = 1;
					
					pParams->nCurrFontWritingMode = WRITING_MODE_VERTICAL;
															
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					wprintf(L"\t***** USECMAP PREDEFINITO 0 -> '%s'. OK! *****\n\n", pParams->szUseCMap);
					#endif
					
					goto uscita;
					//return 1;
				}
				else
				{					
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
					
					pmyData1 = &myData1;
					pmyData2 = &myData2;
					
					if ( genhtFind(&(pParams->myCMapHT), pParams->szUseCMap, sizeof(pParams->szUseCMap), (void**)&pmyData1, &myData1Size ) >= 0 )
					{
						pParams->bEncodigArrayAlreadyInit = 0;
						pParams->bStreamType = STREAM_TYPE_CMAP;
						
						if ( '\0' != myData1.szUseCMap[0] )
						{
							if ( genhtFind(&(pParams->myCMapHT), myData1.szUseCMap, sizeof(myData1.szUseCMap), (void**)&pmyData2, &myData2Size ) >= 0 )
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
			if ( strncmp(pParams->szTemp, "Identity-H", strnlen(pParams->szTemp, 4096) + 1) == 0 )
			{
				pParams->pCurrentEncodingArray = &(pParams->pArrayUnicode[0]);
				
				pParams->bHasCodeSpaceOneByte = 0;
				pParams->bHasCodeSpaceTwoByte = 1;
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\t***** USECMAP USECMAP PREDEFINITO -> '%s'. OK! *****\n\n", pParams->szTemp);
				#endif
				goto uscita;
				//return 1;
			}
			else if ( strncmp(pParams->szTemp, "Identity-V", strnlen(pParams->szTemp, 4096) + 1) == 0 )		
			{
				pParams->pCurrentEncodingArray = &(pParams->pArrayUnicode[0]);
				
				pParams->bHasCodeSpaceOneByte = 0;
				pParams->bHasCodeSpaceTwoByte = 1;
				
				pParams->nCurrFontWritingMode = WRITING_MODE_VERTICAL;
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\t***** USECMAP USECMAP PREDEFINITO -> '%s'. OK! *****\n\n", pParams->szTemp);
				#endif
				goto uscita;
				//return 1;
			}
			else
			{
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
					
				if ( genhtFind(&(pParams->myCMapHT), pParams->szTemp, sizeof(pParams->szTemp), (void**)&pmyData1, &myData1Size ) >= 0 )
				{
					pParams->bEncodigArrayAlreadyInit = 0;
					pParams->bStreamType = STREAM_TYPE_CMAP;
						
					if ( '\0' != myData1.szUseCMap[0] )
					{
						if ( genhtFind(&(pParams->myCMapHT), myData1.szUseCMap, sizeof(myData1.szUseCMap), (void**)&pmyData2, &myData2Size ) >= 0 )
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
		
		goto uscita;
		//return 1;
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
		
		
		
		if ( pParams->nCurrentEncodingObj > 0 )
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\t***** 2 pParams->nCurrentEncodingObj = %d *****\n\n", pParams->nCurrentEncodingObj);
			#endif
			
			if ( FONT_SUBTYPE_Type0 != pParams->nCurrentFontSubtype )
				ParseEncodingObject(pParams, pParams->nCurrentEncodingObj);
			
			//if ( !ParseEncodingObject(pParams, pParams->nCurrentEncodingObj) )
			//	return 0;
		}
		
		
		
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
			if ( strncmp(pParams->szUseCMap, "Identity-H", strnlen(pParams->szUseCMap, 4096) + 1) == 0 ) 
			{
				pParams->pCurrentEncodingArray = &(pParams->pArrayUnicode[0]);
				
				pParams->bHasCodeSpaceOneByte = 0;
				pParams->bHasCodeSpaceTwoByte = 1;
								
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\t***** USECMAP TOUNICODE PREDEFINITO -> '%s'. OK! *****\n\n", pParams->szUseCMap);
				#endif
				
				goto uscita;
				//return 1;
			}
			else if ( strncmp(pParams->szUseCMap, "Identity-V", strnlen(pParams->szUseCMap, 4096) + 1) == 0 )
			{
				pParams->pCurrentEncodingArray = &(pParams->pArrayUnicode[0]);
				
				pParams->bHasCodeSpaceOneByte = 0;
				pParams->bHasCodeSpaceTwoByte = 1;
				
				pParams->nCurrFontWritingMode = WRITING_MODE_VERTICAL;
								
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\t***** USECMAP TOUNICODE PREDEFINITO -> '%s'. OK! *****\n\n", pParams->szUseCMap);
				#endif
				
				goto uscita;
				//return 1;
			}
			else
			{
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
					
				if ( genhtFind(&(pParams->myCMapHT), pParams->szUseCMap, sizeof(pParams->szUseCMap), (void**)&pmyData1, &myData1Size ) >= 0 )
				{
					pParams->bEncodigArrayAlreadyInit = 0;
					pParams->bStreamType = STREAM_TYPE_CMAP;
						
					if ( '\0' != myData1.szUseCMap[0] )
					{
						if ( genhtFind(&(pParams->myCMapHT), myData1.szUseCMap, sizeof(myData1.szUseCMap), (void**)&pmyData2, &myData2Size ) >= 0 )
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
			
uscita:

	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WritingMode = pParams->nCurrFontWritingMode;
	
	if ( FONT_SUBTYPE_Type0 != pParams->nCurrentFontSubtype )
	{
		if ( pParams->nCurrentBaseFont >= 1 && pParams->nCurrentBaseFont <= 14 )
		{
			pParams->pCurrentEncodingArray = &(pParams->aSTD_CharSet[0]);
		
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar = 0;
			
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar = 255;
		}
		
		pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar - pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar + 1;

		if ( pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize <= 0 )
		{
			snprintf(pParams->szError, 8192, "ERROR contentfontobj: WidthsArraySize = %d -> pParams->myObjsTable[%d]->pGlyphsWidths->FirstChar = %d; pParams->myObjsTable[%d]->pGlyphsWidths->LastChar = %d.\n", pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize, pParams->nCurrentParsingFontObjNum, pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar, pParams->nCurrentParsingFontObjNum, pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar);
			myShowErrorMessage(pParams, pParams->szError, 1);
			return 0;
		}

		pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths = (double*)malloc(sizeof(double) * pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize);
		if ( NULL == pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths )
		{
			snprintf(pParams->szError, 8192, "ERROR contentfontobj: malloc failed for pParams->myObjsTable[%d]->pGlyphsWidths->pWidths.\n", pParams->nCurrentParsingFontObjNum);
			myShowErrorMessage(pParams, pParams->szError, 1);
			wprintf(L"ERROR contentfontobj: malloc failed for pParams->myObjsTable[%d]->pGlyphsWidths->pWidths.\n", pParams->nCurrentParsingFontObjNum);
			return 0;
		}
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ_WIDTHS)
		wprintf(L"\tpParams->myObjsTable[%d]->pGlyphsWidths->WidthsArraySize = %d\n", pParams->nCurrentParsingFontObjNum, pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize);
		#endif
		
		pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth = 0.0;
		pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->AvgWidth = 0.0;
		pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MaxWidth = 0.0;
		
		if ( pParams->nCurrFontDescriptorRef > 0 )
			ParseFontDescriptorObject(pParams, pParams->nCurrFontDescriptorRef);
		
		for ( int k = 0; k < pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize; k++ )
		{
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth;
		}
	
		if ( pParams->nCurrentParsingFontWidthsArrayRef > 0 )
		{
			if ( !ParseFontWidthsArray(pParams, pParams->nCurrentParsingFontWidthsArrayRef) )
			{
				free(pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths);
				pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths = NULL;
				return 0;
			}
		}
		else
		{
			if ( BASEFONT_NoPredef == pParams->nCurrentBaseFont )
			{
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ_WIDTHS)
				wchar_t cTemp;
				#endif
				
				nCurrCharIndex = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar;
				nCurrCharIndex -= pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar;
		
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ_WIDTHS)
				wprintf(L"\tcontentfontobj -> FirstChar = %d; LastChar = %d; nCurrCharIndex = %d\n", pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar, pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar, nCurrCharIndex);
				#endif
			
				while ( mynumstacklist_Pop(&(pParams->myCurrFontWidthsStack), &dblWidth) )
				{	
					//if ( dblWidth > 0 )
						pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[nCurrCharIndex] = dblWidth;
					//else
					//	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[nCurrCharIndex] = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth;
			
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ_WIDTHS)
					wprintf(L"\tcontentfontobj -> STACK POP(nCurrCharIndex = %d) -> pParams->myObjsTable[%d]->pGlyphsWidths->pWidths[%d] = %f\n", nCurrCharIndex, pParams->nCurrentParsingFontObjNum, nCurrCharIndex, pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[nCurrCharIndex]);
					cTemp = pParams->pCurrentEncodingArray[nCurrCharIndex + pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar];
					if ( (cTemp >= 32 && cTemp <= 126) || (cTemp >= 128 && cTemp != 129 && cTemp != 141 && cTemp != 143 && cTemp != 144 && cTemp != 157 && cTemp != 160 && cTemp != 173) )
					{
						wprintf(L"\tc = '%lc' -> Width = %f\n", cTemp, dblWidth);
					}
					else
					{
						wprintf(L"\tc = '<%X>' -> Width = %f\n", cTemp, dblWidth);
					}
					#endif
					
					if ( L' ' == pParams->pCurrentEncodingArray[nCurrCharIndex + pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar] )
						pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
										
					nCurrCharIndex--;
				}
			}
			else
			{
				setPredefFontsWidthsArray(pParams);
			
				nCurrCharIndex = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar;
				nCurrCharIndex -= pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar;
			
				while ( nCurrCharIndex >= 0 )
				{
					dblWidth = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[nCurrCharIndex];
								
					if ( pParams->dCurrFontMaxWidth < dblWidth )
						pParams->dCurrFontMaxWidth = dblWidth;
					
					nCurrCharIndex--;
				}
			}
		}
		
		dblCumWidths = 0.0;
		nCountWidthsNonZero = 0;
		for ( int k = 0; k < pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize; k++ )
		{
			dblCumWidths += (pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k]);
			
			if ( pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] > 0.0 )
				nCountWidthsNonZero++;
		}
		if ( nCountWidthsNonZero <= 0 )
			nCountWidthsNonZero = 1;
		//pParams->dCurrFontAvgWidth = dblCumWidths / pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize;
		pParams->dCurrFontAvgWidth = dblCumWidths / nCountWidthsNonZero;
		
		if ( pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->AvgWidth <= 0.0 )
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->AvgWidth = pParams->dCurrFontAvgWidth;
		
		if ( pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MaxWidth <= 0.0 )
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MaxWidth = pParams->dCurrFontMaxWidth;
	}
	else
	{
		if ( 0 == pParams->nDescendantFontRef )
		{
			snprintf(pParams->szError, 8192, "ERROR contentfontobj: pParams->nDescendantFontRef = 0 for Type0 Font.\n");
			myShowErrorMessage(pParams, pParams->szError, 1);
			return 0;
		}
		
		pParams->bCurrParsingFontIsCIDFont = 1;
		
		if ( WRITING_MODE_HORIZONTAL == pParams->nCurrFontWritingMode )
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW = 1.0;
		else
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW = -1.0;
	
		//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW2.v = 0.880;
		//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW2.w1 = -1.0;
		
		if ( !ParseCIDFontObject(pParams, pParams->nDescendantFontRef) )
			return 0;
			
		if ( pParams->bCidFontArrayInsteadDict )
		{
			if ( !ParseCIDFontObject(pParams, pParams->nDescendantFontRef) )
				return 0;
		}		
	}
		
	if ( pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth <= 0.0 )
	{
		//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth;
		
		//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.0;
		
		if ( pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth > 0.0 )
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth;
		else
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = pParams->dCurrFontAvgWidth;
		//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = pParams->dCurrFontMaxWidth;
				
		pParams->bCurrFontSpaceWidthIsZero = 1;
		
		if ( pParams->bCurrParsingFontIsCIDFont && !(pParams->bWisPresent) )
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW;
	}
	else
	{
		pParams->bCurrFontSpaceWidthIsZero = 0;
	}
	
	//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = pParams->dCurrFontAvgWidth;
											
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ_WIDTHS)
	wprintf(L"\tcontentfontobj -> pParams->myObjsTable[%d]->pGlyphsWidths->dFontSpaceWidth = %f; pParams->myObjsTable[%d]->pGlyphsWidths->AvgWidth = %f; pParams->myObjsTable[%d]->pGlyphsWidths->MaxWidth = %f; pParams->myObjsTable[%d]->pGlyphsWidths->MissingWidth = %f\n\n",
			pParams->nCurrentParsingFontObjNum,
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth,
			pParams->nCurrentParsingFontObjNum,
			pParams->dCurrFontAvgWidth,
			pParams->nCurrentParsingFontObjNum,
			pParams->dCurrFontMaxWidth,
			pParams->nCurrentParsingFontObjNum,
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth
			);
	#endif
	
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
		strncpy(pParams->szCurrKeyName, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
		
		if ( strncmp(pParams->szCurrKeyName, "Widths", 1024) == 0 )
			pParams->bFontWidthsArrayState = 1;
			
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
			n1 = pParams->myToken.vInt;
						
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.vInt;
							
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
				else if ( strncmp(pParams->szCurrKeyName, "FontDescriptor", 1024) == 0 )
				{
					pParams->nCurrFontDescriptorRef = n1;
					pParams->szTemp[0] = '0';			
				}			
				else if ( strncmp(pParams->szCurrKeyName, "Widths", 1024) == 0 )
				{
					strncpy(pParams->szExtArraySizeCurrKeyName, pParams->szCurrKeyName, strnlen(pParams->szCurrKeyName, MAX_STRLEN)); 
					pParams->nCurrentParsingFontWidthsArrayRef = n1;
					pParams->szTemp[0] = '0';			
				}
				else if ( strncmp(pParams->szCurrKeyName, "DescendantFonts", 1024) == 0 )
				{
					pParams->nDescendantFontRef= n1;
					pParams->szTemp[0] = '0';
				}				
			}
			else
			{
				if ( strncmp(pParams->szCurrKeyName, "FirstChar", 1024) == 0 )
				{
					pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar = n1;
				}
				else if ( strncmp(pParams->szCurrKeyName, "LastChar", 1024) == 0 )
				{
					pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar = n1;
				}
			}
			
			break;
		case T_NAME:
			if ( strncmp(pParams->szCurrKeyName, "Encoding", 1024) == 0 )
			{
				// MacRomanEncoding, MacExpertEncoding, or WinAnsiEncoding
				strncpy(pParams->szTemp, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
				pParams->nCurrentEncodingObj = 0;
			}			
			else if ( strncmp(pParams->szCurrKeyName, "Subtype", 1024) == 0 )
			{
				if ( strncmp(pParams->myToken.vString, "Type1", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_Type1;
				else if ( strncmp(pParams->myToken.vString, "Type0", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_Type0;
				else if ( strncmp(pParams->myToken.vString, "Type3", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_Type3;
				else if ( strncmp(pParams->myToken.vString, "MMType1", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_MMType1;
				else if ( strncmp(pParams->myToken.vString, "TrueType", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_TrueType;
				else if ( strncmp(pParams->myToken.vString, "CIDFontType0", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_CIDFontType0;
				else if ( strncmp(pParams->myToken.vString, "CIDFontType2", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_CIDFontType2;
				else
					pParams->nCurrentFontSubtype = FONT_SUBTYPE_Unknown;
			}
			else if ( strncmp(pParams->szCurrKeyName, "BaseFont", 1024) == 0 )
			{
				char *psz;
				
				strncpy(pParams->szBaseFont, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
				
				psz = &(pParams->szBaseFont[1]);
				switch ( pParams->szBaseFont[0] )
				{
					case 'C':
						if ( strncmp(psz, "ourier", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Courier;
						else if ( strncmp(psz, "ourier-Bold", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Courier_Bold;
						else if ( strncmp(psz, "ourier-BoldOblique", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Courier_BoldOblique;
						else if ( strncmp(psz, "ourier-Oblique", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Courier_Oblique;
						else
							pParams->nCurrentBaseFont = BASEFONT_NoPredef;
						break;
					case 'H':
						if ( strncmp(psz, "elvetica", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Helvetica;
						else if ( strncmp(psz, "elvetica-Bold", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Helvetica_Bold;
						else if ( strncmp(psz, "elvetica-BoldOblique", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Helvetica_BoldOblique;
						else if ( strncmp(psz, "elvetica-Oblique", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Helvetica_Oblique;
						else
							pParams->nCurrentBaseFont = BASEFONT_NoPredef;
						break;
					case 'T':
						if ( strncmp(psz, "imes-Bold", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Times_Bold;
						else if ( strncmp(psz, "imes-BoldItalic", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Times_BoldItalic;
						else if ( strncmp(psz, "imes-Italic", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Times_Italic;
						else if ( strncmp(psz, "imes-Roman", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Times_Roman;
						else
							pParams->nCurrentBaseFont = BASEFONT_NoPredef;
						break;
					case 'S':
						if ( strncmp(psz, "ymbol", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Symbol;
						else
							pParams->nCurrentBaseFont = BASEFONT_NoPredef;
						break;
					case 'Z':
						if ( strncmp(psz, "apfDingbats", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_ZapfDingbats;
						else
							pParams->nCurrentBaseFont = BASEFONT_NoPredef;
						break;
					default:
						pParams->nCurrentBaseFont = BASEFONT_NoPredef;
						break;
				}
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
		keyValue = pParams->myToken.vInt;
		
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
		
		len = strnlen(pParams->myToken.vString, 4096);
		nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.vString, len + sizeof(char), (void*)&nData, &nDataSize, &bContentAlreadyProcessed);
		if ( nRes >= 0 ) // TROVATO
		{				
			pParams->paCustomizedFont_CharSet[keyValue] = nData;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\tfontdirectencodingobjarray KEY -> '%s' -> Value = %d -> char = '%lc'\n", pParams->myToken.vString, keyValue, (wchar_t)pParams->paCustomizedFont_CharSet[keyValue]);
			#endif			
		}
		else
		{
			//wprintf(L"encodingobjarray -> WARNING: KEY '%s' NON TROVATA\n", pParams->myToken.vString);
			pParams->paCustomizedFont_CharSet[keyValue] = L' ';
		}
		
		GetNextToken(pParams);
		
		while ( pParams->myToken.Type == T_NAME )
		{
			keyValue++;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif				
			
			len = strnlen(pParams->myToken.vString, 4096);
			nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.vString, len + sizeof(char), (void*)&nData, &nDataSize, &bContentAlreadyProcessed);
			if ( nRes >= 0 ) // TROVATO
			{							
				pParams->paCustomizedFont_CharSet[keyValue] = nData;
								
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\tfontdirectencodingobjarray KEY -> '%s' -> Value = %d -> char = '%lc'\n", pParams->myToken.vString, keyValue, (wchar_t)pParams->paCustomizedFont_CharSet[keyValue]);
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
	double dblWidth = 0.0;	
	
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
			if ( strncmp(pParams->szCurrKeyName, "Widths", 1024) == 0 )
			{
				dblWidth = (double)pParams->myToken.vInt * 0.001;
									
				if ( pParams->dCurrFontMaxWidth < dblWidth )
					pParams->dCurrFontMaxWidth = dblWidth;
					
				mynumstacklist_Push(&(pParams->myCurrFontWidthsStack), dblWidth);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ_WIDTHS)
				wprintf(L"\tcontentfontobj -> STACK PUSH(pParams->myCurrFontWidthsStack.count = %d) -> %d\n", pParams->myCurrFontWidthsStack.count, pParams->myToken.vInt);
				#endif
			}
			
			n1 = pParams->myToken.vInt;
		}
		else
		{
			dblWidth = pParams->myToken.vDouble * 0.001;
			
			if ( strncmp(pParams->szCurrKeyName, "Widths", 1024) == 0 )
			{					
				if ( pParams->dCurrFontMaxWidth < dblWidth )
					pParams->dCurrFontMaxWidth = dblWidth;
					
				mynumstacklist_Push(&(pParams->myCurrFontWidthsStack), dblWidth);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ_WIDTHS)
				wprintf(L"\tcontentfontobj -> STACK PUSH(pParams->myCurrFontWidthsStack.count = %d) -> %f\n", pParams->myCurrFontWidthsStack.count, pParams->myToken.vDouble);
				#endif
			}
			
			n1 = n2 = -1;
		}
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
		
		GetNextToken(pParams);
		
		if ( !(pParams->bFontWidthsArrayState) && n1 > 0 && pParams->myToken.Type == T_INT_LITERAL )
		{
			n2 = pParams->myToken.vInt;
							
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
					if ( strncmp(pParams->szCurrKeyName, "DescendantFonts", 1024) == 0 )
					{
						pParams->nDescendantFontRef = n1;
						pParams->szTemp[0] = '0';
					}
					
					//#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					//PrintToken(&(pParams->myToken), ' ', ' ', 1);
					//#endif
					
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
	
	pParams->bFontWidthsArrayState = 0;	
	
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

// ************************************************************************************************************************

// cidfontobj         : T_INT_LITERAL T_INT_LITERAL T_KW_OBJ cidfontobjbody T_KW_ENDOBJ;
int cidfontobj(Params *pParams)
{
	//int k;
	char szFontType[128];
	
	double dblCumWidths;
	int nCountWidthsNonZero;
	
	//int nWidthsArraySize = 0;
	//double dblWidth = -1.0;
				
	//pParams->szTemp[0] = '\0';
	
	pParams->nCurrentCIDFontSubtype = FONT_SUBTYPE_Unknown;
		
	pParams->bCidFontArrayInsteadDict = 0;
	
	pParams->szExtArraySizeCurrKeyName[0] = '\0';
	//pParams->szExtArraySizeDW2CurrKeyName[0] = '\0';
		
	pParams->nCurrentParsingFontWidthsArrayRef = 0;
	pParams->nCurrentParsingFontDW2WidthsArrayRef = 0;
			
	pParams->nCurrFontDescriptorRef = 0;
	pParams->szBaseFont[0] = '\0';
		
	pParams->dCurrFontSpaceWidth = 0.0;
	pParams->dCurrFontSpaceWidthScaled = 0.0;
	pParams->dCurrFontAvgWidth = 0.0;
	pParams->dCurrFontMaxWidth = -1.0;
		
	if ( T_INT_LITERAL != pParams->myToken.Type )
	{
		snprintf(pParams->szError, 8192, "ERRORE cidfontobj: atteso T_INT_LITERAL; trovato token n° pParams->myToken.Type.\n");
		myShowErrorMessage(pParams, pParams->szError, 1);
		PrintToken(&(pParams->myToken), '\t', ' ', 1);
		return 0;
	}
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
	
	pParams->nCurrentParsingCIDFontObjNum = pParams->myToken.vInt;	
							
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	GetNextToken(pParams);
			
	//if ( !match(pParams, T_INT_LITERAL, "contentfontobj") )
	//	return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	if ( !match(pParams, T_INT_LITERAL, "cidfontobj") )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_KW_OBJ, "cidfontobj") )
		return 0;
	
	if ( !cidfontobjbody(pParams) )
		return 0;

	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	//if ( pParams->bCidFontArrayInsteadDict )
	//	goto uscita;
	
	if ( !match(pParams, T_KW_ENDOBJ, "cidfontobj") )
		return 0;

	
	switch ( pParams->nCurrentCIDFontSubtype )
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
						
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ_WIDTHS)
	wprintf(L"\tpParams->myObjsTable[%d]->pGlyphsWidths->WidthsArraySize = %d\n", pParams->nCurrentParsingFontObjNum, pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize);
	#endif
		
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth = 0.0;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->AvgWidth = 0.0;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MaxWidth = 0.0;
			
	if ( pParams->nCurrFontDescriptorRef > 0 )
		ParseFontDescriptorObject(pParams, pParams->nCurrFontDescriptorRef);

	if ( pParams->nCurrentParsingFontWidthsArrayRef > 0 )
	{
		if ( !ParseFontWidthsArray(pParams, pParams->nCurrentParsingFontWidthsArrayRef) )
		{
			free(pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths = NULL;
			return 0;
		}
	}
	
	if ( pParams->nCurrentParsingFontDW2WidthsArrayRef > 0 )
	{
		pParams->szExtArraySizeCurrKeyName[0] = 'D';
		pParams->szExtArraySizeCurrKeyName[0] = 'W';
		pParams->szExtArraySizeCurrKeyName[0] = '2';
		pParams->szExtArraySizeCurrKeyName[0] = '\0';
		if ( !ParseFontWidthsArray(pParams, pParams->nCurrentParsingFontDW2WidthsArrayRef) )
		{
			free(pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths = NULL;
			return 0;
		}
	}
	
	//pParams->szExtArraySizeCurrKeyName[0] = '\0';
	//pParams->szExtArraySizeDW2CurrKeyName[0] = '\0';
				
	dblCumWidths = 0.0;
	nCountWidthsNonZero = 0;
	if ( pParams->bWisPresent )
	{			
		for ( int k = 0; k < pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize; k++ )
		{
			if ( pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] < 0.0 )
				pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth;
				
			if ( pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] > 0.0 )
				nCountWidthsNonZero++;
				
			dblCumWidths += (pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k]);
		}
		//pParams->dCurrFontAvgWidth = dblCumWidths / pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize;
		if ( nCountWidthsNonZero <= 0 )
			nCountWidthsNonZero = 1;
		pParams->dCurrFontAvgWidth = dblCumWidths / nCountWidthsNonZero;
	}
	else
	{
		pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW;
		
		pParams->dCurrFontAvgWidth = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW;
	}
	
	if ( pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->AvgWidth <= 0.0 )
		pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->AvgWidth = pParams->dCurrFontAvgWidth;
		
	if ( pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MaxWidth <= 0.0 )
		pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MaxWidth = pParams->dCurrFontMaxWidth;
									
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ_WIDTHS)
	wprintf(L"\tcidfontobj -> pParams->myObjsTable[%d]->pGlyphsWidths->dFontSpaceWidth = %f; pParams->myObjsTable[%d]->pGlyphsWidths->AvgWidth = %f; pParams->myObjsTable[%d]->pGlyphsWidths->MaxWidth = %f; pParams->myObjsTable[%d]->pGlyphsWidths->MissingWidth = %f\n\n",
			pParams->nCurrentParsingFontObjNum,
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth,
			pParams->nCurrentParsingFontObjNum,
			pParams->dCurrFontAvgWidth,
			pParams->nCurrentParsingFontObjNum,
			pParams->dCurrFontMaxWidth,
			pParams->nCurrentParsingFontObjNum,
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth
			);
	#endif
	
	//uscita:
		
	return 1;
}

// cidfontobjbody     : T_DICT_BEGIN cidfontobjdictitems T_DICT_END;
int cidfontobjbody(Params *pParams)
{	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
	
	if ( T_DICT_BEGIN != pParams->myToken.Type )
	{
		if ( T_QOPAREN != pParams->myToken.Type )
			return 0;
				
		pParams->bCidFontArrayInsteadDict = 1;	
		
		GetNextToken(pParams);
	
		if ( T_INT_LITERAL != pParams->myToken.Type )
			return 0;
			
		pParams->nDescendantFontRef = pParams->myToken.vInt;
				
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif

		GetNextToken(pParams);
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
				
		if ( !match(pParams, T_INT_LITERAL, "cidfontobjbody") )
			return 0;
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
			
		if ( !match(pParams, T_KW_R, "cidfontobjbody") )
			return 0;
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif
		
		if ( !match(pParams, T_QCPAREN, "cidfontobjbody") )
			return 0;
			
		//return ParseCIDFontObject(pParams, pParams->nDescendantFontRef);
		return 1;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif

	GetNextToken(pParams);
	
	if ( !cidfontobjdictitems(pParams) )
		return 0;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_DICT_END, "cidfontobjbody") )
		return 0;
	
	return 1;
}

// cidfontobjdictitems : {T_NAME cidfontobjkeyvalue};
int cidfontobjdictitems(Params *pParams)
{
	while ( pParams->myToken.Type == T_NAME )
	{
		strncpy(pParams->szCurrKeyName, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
		
		if ( strncmp(pParams->szCurrKeyName, "W", 1024) == 0 )
		{
			//strncpy(pParams->szExtArraySizeCurrKeyName, pParams->szCurrKeyName, strnlen(pParams->szCurrKeyName, MAX_STRLEN)); 
			
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar = 0;
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar = 0x10000;
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar - pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar + 1;
			
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths = (double*)malloc(sizeof(double) * pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize);
			if ( NULL == pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths )
			{
				snprintf(pParams->szError, 8192, "ERROR cidfontobjdictitems: malloc failed for pParams->myObjsTable[%d]->pGlyphsWidths->pWidths.\n", pParams->nCurrentParsingFontObjNum);
				myShowErrorMessage(pParams, pParams->szError, 1);
				wprintf(L"ERROR cidfontobjdictitems: malloc failed for pParams->myObjsTable[%d]->pGlyphsWidths->pWidths.\n", pParams->nCurrentParsingFontObjNum);
				return 0;
			}
			for ( int k = 0; k < pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize; k++ )
			{
				pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = -1.0;
			}
			
			pParams->bFontWidthsArrayState = 1;
			pParams->bWisPresent = 1;
		}
		else if ( strncmp(pParams->szCurrKeyName, "W2", 1024) == 0 )
		{
			//strncpy(pParams->szExtArraySizeCurrKeyName, pParams->szCurrKeyName, strnlen(pParams->szCurrKeyName, MAX_STRLEN)); 
			
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar = 0;
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar = 0x10000;
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar - pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar + 1;
			
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths = (double*)malloc(sizeof(double) * pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize);
			if ( NULL == pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths )
			{
				snprintf(pParams->szError, 8192, "ERROR cidfontobjdictitems: malloc failed for pParams->myObjsTable[%d]->pGlyphsWidths->pWidths.\n", pParams->nCurrentParsingFontObjNum);
				myShowErrorMessage(pParams, pParams->szError, 1);
				wprintf(L"ERROR cidfontobjdictitems: malloc failed for pParams->myObjsTable[%d]->pGlyphsWidths->pWidths.\n", pParams->nCurrentParsingFontObjNum);
				return 0;
			}
			for ( int k = 0; k < pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize; k++ )
			{
				pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = -1.0;
			}
			
			pParams->bFontWidthsArrayState = 1;
			//pParams->bW2isPresent = 1;
			pParams->bWisPresent = 1;
		}

		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
				
		GetNextToken(pParams);			
				
		if ( !cidfontobjkeyvalue(pParams) )
			return 0;
	}
	
	return 1;
}

//cidfontobjkeyvalue : T_INT_LITERAL [ T_INT_LITERAL T_KW_R ]
//                       | T_NAME
//                       | T_T_STRING_LITERAL
//                       | T_STRING_HEXADECIMAL
//                       | fontobjcontentkeyarray
//                       | fontobjcontentkeydict
//                       ;
int cidfontobjkeyvalue(Params *pParams)
{
	int n1 = -1;
	int n2 = -1;
		
	switch ( pParams->myToken.Type )
	{
		case T_INT_LITERAL:
			n1 = pParams->myToken.vInt;
						
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.vInt;
							
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)	
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif
				
				GetNextToken(pParams);
				
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				PrintToken(&(pParams->myToken), ' ', ' ', 1);
				#endif				
				
				if ( !match(pParams, T_KW_R, "cidfontobjkeyvalue") )
					return 0;
			}
			
			if ( n2 >= 0 )
			{
				if ( strncmp(pParams->szCurrKeyName, "FontDescriptor", 1024) == 0 )
				{
					pParams->nCurrFontDescriptorRef = n1;
					//pParams->szTemp[0] = '0';			
				}
				else if ( strncmp(pParams->szCurrKeyName, "W", 1024) == 0 || strncmp(pParams->szCurrKeyName, "W2", 1024) == 0 )
				{
					//strncpy(pParams->szExtArraySizeCurrKeyName, pParams->szCurrKeyName, 3); 
					pParams->szExtArraySizeCurrKeyName[0] = pParams->szCurrKeyName[0];
					pParams->szExtArraySizeCurrKeyName[1] = pParams->szCurrKeyName[1];
					if ( '\0' != pParams->szCurrKeyName[1] )
						pParams->szExtArraySizeCurrKeyName[2] = '\0';
					pParams->nCurrentParsingFontWidthsArrayRef = n1;
										
					//pParams->szTemp[0] = '0';
				}
				else if ( strncmp(pParams->szCurrKeyName, "DW2", 1024) == 0 )
				{
					//strncpy(pParams->szExtArraySizeDW2CurrKeyName, pParams->szCurrKeyName, strnlen(pParams->szCurrKeyName, MAX_STRLEN)); 
					//pParams->nCurrentParsingFontWidthsArrayRef = n1;
					pParams->nCurrentParsingFontDW2WidthsArrayRef = n1;
				}
			}
			else
			{
				if ( strncmp(pParams->szCurrKeyName, "DW", 1024) == 0 )
				{
					pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW = (double)n1 * 0.001;
				}
			}
			
			break;
		case T_NAME:
			if ( strncmp(pParams->szCurrKeyName, "Subtype", 1024) == 0 )
			{
				if ( strncmp(pParams->myToken.vString, "Type1", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentCIDFontSubtype = FONT_SUBTYPE_Type1;
				else if ( strncmp(pParams->myToken.vString, "Type0", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentCIDFontSubtype = FONT_SUBTYPE_Type0;
				else if ( strncmp(pParams->myToken.vString, "Type3", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentCIDFontSubtype = FONT_SUBTYPE_Type3;
				else if ( strncmp(pParams->myToken.vString, "MMType1", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentCIDFontSubtype = FONT_SUBTYPE_MMType1;
				else if ( strncmp(pParams->myToken.vString, "TrueType", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentCIDFontSubtype = FONT_SUBTYPE_TrueType;
				else if ( strncmp(pParams->myToken.vString, "CIDFontType0", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentCIDFontSubtype = FONT_SUBTYPE_CIDFontType0;
				else if ( strncmp(pParams->myToken.vString, "CIDFontType2", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
					pParams->nCurrentCIDFontSubtype = FONT_SUBTYPE_CIDFontType2;
				else
					pParams->nCurrentCIDFontSubtype = FONT_SUBTYPE_Unknown;
			}
			else if ( strncmp(pParams->szCurrKeyName, "BaseFont", 1024) == 0 )
			{
				char *psz;
				
				strncpy(pParams->szBaseFont, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
				
				psz = &(pParams->szBaseFont[1]);
				switch ( pParams->szBaseFont[0] )
				{
					case 'C':
						if ( strncmp(psz, "ourier", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Courier;
						else if ( strncmp(psz, "ourier-Bold", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Courier_Bold;
						else if ( strncmp(psz, "ourier-BoldOblique", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Courier_BoldOblique;
						else if ( strncmp(psz, "ourier-Oblique", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Courier_Oblique;
						else
							pParams->nCurrentBaseFont = BASEFONT_NoPredef;
						break;
					case 'H':
						if ( strncmp(psz, "elvetica", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Helvetica;
						else if ( strncmp(psz, "elvetica-Bold", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Helvetica_Bold;
						else if ( strncmp(psz, "elvetica-BoldOblique", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Helvetica_BoldOblique;
						else if ( strncmp(psz, "elvetica-Oblique", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Helvetica_Oblique;
						else
							pParams->nCurrentBaseFont = BASEFONT_NoPredef;
						break;
					case 'T':
						if ( strncmp(psz, "imes-Bold", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Times_Bold;
						else if ( strncmp(psz, "imes-BoldItalic", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Times_BoldItalic;
						else if ( strncmp(psz, "imes-Italic", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Times_Italic;
						else if ( strncmp(psz, "imes-Roman", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Times_Roman;
						else
							pParams->nCurrentBaseFont = BASEFONT_NoPredef;
						break;
					case 'S':
						if ( strncmp(psz, "ymbol", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_Symbol;
						else
							pParams->nCurrentBaseFont = BASEFONT_NoPredef;
						break;
					case 'Z':
						if ( strncmp(psz, "apfDingbats", 1024) == 0 )
							pParams->nCurrentBaseFont = BASEFONT_ZapfDingbats;
						else
							pParams->nCurrentBaseFont = BASEFONT_NoPredef;
						break;
					default:
						pParams->nCurrentBaseFont = BASEFONT_NoPredef;
						break;
				}
			}

			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			break;
		case T_STRING_LITERAL:
		case T_STRING_HEXADECIMAL:
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			break;
		case T_QOPAREN:
			if ( !cidfontobjkeyarray(pParams) )
				return 0;
			break;
		case T_DICT_BEGIN:
			if ( !cidfontobjkeydict(pParams) )
				return 0;
			break;
		default:
			snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyvalue: Atteso uno di questi token: T_INT_LITERAL, T_NAME, T_STRING_LITERAL, T_STRING_HEXADECIMAL, T_QOPAREN, TDICT_BEGIN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;			
			break;
	}
	
	return 1;
}

// cidfontobjkeyarray : T_QOPAREN { T_INT_LITERAL [T_INT_LITERAL T_KW_R ] | T_REAL_LITERAL} T_QCPAREN;
int cidfontobjkeyarray(Params *pParams)
{
	int n1 = -1;
	int n2 = -1;
	double dblWidth = 0.0;	
			
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_QOPAREN, "cidfontobjkeyarray") )
	{
		return 0;
	}
	
	// *************************************************************************************************************************************************************
	if ( strncmp(pParams->szCurrKeyName, "W", 1024) == 0 )
	{
		while ( pParams->myToken.Type == T_INT_LITERAL )
		{
			//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[nCurrCharIndex] = dblWidth;
			
			n1 = pParams->myToken.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
		
			GetNextToken(pParams);
			
			switch ( pParams->myToken.Type )
			{
				case T_QOPAREN:
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					PrintToken(&(pParams->myToken), ' ', ' ', 1);
					#endif
							
					GetNextToken(pParams);
					
					while ( pParams->myToken.Type == T_INT_LITERAL || pParams->myToken.Type == T_REAL_LITERAL )
					{
						switch ( pParams->myToken.Type )
						{
							case T_INT_LITERAL:
							{
								dblWidth = (double)pParams->myToken.vInt * 0.001;
																
								if ( pParams->dCurrFontMaxWidth < dblWidth )
									pParams->dCurrFontMaxWidth = dblWidth;
									
								if ( 32 == n1 )
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
					
								pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[n1] = dblWidth;
								n1++;
							}
							break;
							case T_REAL_LITERAL:
							{
								dblWidth = pParams->myToken.vDouble * 0.001;
														
								if ( pParams->dCurrFontMaxWidth < dblWidth )
									pParams->dCurrFontMaxWidth = dblWidth;
									
								if ( 32 == n1 )
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
										
								pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[n1] = dblWidth;
								n1++;
							}
							break;
							default:
							{
								;
							}
							break;
						}
						
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
						PrintToken(&(pParams->myToken), ' ', ' ', 1);
						#endif
							
						GetNextToken(pParams);
					}
												
					if ( T_QCPAREN != pParams->myToken.Type )
					{
						snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso T_QCPAREN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
						myShowErrorMessage(pParams, pParams->szError, 1);
						PrintToken(&(pParams->myToken), ' ', ' ', 1);
						return 0;
					}
				}
				break;
				case T_INT_LITERAL:
				{
					n2 = pParams->myToken.vInt;
			
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					PrintToken(&(pParams->myToken), ' ', ' ', 1);
					#endif
							
					GetNextToken(pParams);
				
					switch ( pParams->myToken.Type )
					{
						case T_INT_LITERAL:
						{
							dblWidth = (double)pParams->myToken.vInt * 0.001;
												
							if ( pParams->dCurrFontMaxWidth < dblWidth )
								pParams->dCurrFontMaxWidth = dblWidth;
								
							if ( 32 == n1 )
								pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
										
							for ( int k = n1; k <= n2; k++ )
							{										
								pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = dblWidth;
							}
						}
						break;
						case T_REAL_LITERAL:
						{
							dblWidth = pParams->myToken.vDouble * 0.001;
												
							if ( pParams->dCurrFontMaxWidth < dblWidth )
								pParams->dCurrFontMaxWidth = dblWidth;
								
							if ( 32 == n1 )
								pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
										
							for ( int k = n1; k <= n2; k++ )
							{										
								pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = dblWidth;
							}
						}
						break;
						default:
						{
							snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso T_INT_LITERAL o T_QOPAREN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
							myShowErrorMessage(pParams, pParams->szError, 1);
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							return 0;
						}
						break;
					}
				}
				break;
				default:
				{
					snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso T_INT_LITERAL o T_QOPAREN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
					myShowErrorMessage(pParams, pParams->szError, 1);
					PrintToken(&(pParams->myToken), ' ', ' ', 1);
					return 0;
				}
				break;
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
		
			GetNextToken(pParams);
		}
	}
	else if ( strncmp(pParams->szCurrKeyName, "W2", 1024) == 0 )
	{
		//snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: W2, PER IL MOMENTO, NON IMPLEMENTATO.\n");
		//myShowErrorMessage(pParams, pParams->szError, 1);
		//PrintToken(&(pParams->myToken), ' ', ' ', 1);
		//return 0;
		
		while ( pParams->myToken.Type == T_INT_LITERAL )
		{
			//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[nCurrCharIndex] = dblWidth;
			
			n1 = pParams->myToken.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
		
			GetNextToken(pParams);
			
			switch ( pParams->myToken.Type )
			{
				case T_QOPAREN:
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					PrintToken(&(pParams->myToken), ' ', ' ', 1);
					#endif
							
					GetNextToken(pParams);
					
					while ( pParams->myToken.Type == T_INT_LITERAL || pParams->myToken.Type == T_REAL_LITERAL )
					{
						switch ( pParams->myToken.Type )
						{
							case T_INT_LITERAL:
							{
								dblWidth = (double)pParams->myToken.vInt * 0.001;
																
								if ( pParams->dCurrFontMaxWidth < dblWidth )
									pParams->dCurrFontMaxWidth = dblWidth;
									
								if ( 32 == n1 )
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
					
								pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[n1] = dblWidth;
								n1++;
							}
							break;
							case T_REAL_LITERAL:
							{
								dblWidth = pParams->myToken.vDouble * 0.001;
														
								if ( pParams->dCurrFontMaxWidth < dblWidth )
									pParams->dCurrFontMaxWidth = dblWidth;
									
								if ( 32 == n1 )
									pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
										
								pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[n1] = dblWidth;
								n1++;
							}
							break;
							default:
							{
								;
							}
							break;
						}
						
						// IGNORIAMO I SUCCESSIVI DUE NUMERI v1, v2:
						// v1:
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
						PrintToken(&(pParams->myToken), ' ', ' ', 1);
						#endif	
						GetNextToken(pParams);
						if ( T_INT_LITERAL != pParams->myToken.Type && T_REAL_LITERAL != pParams->myToken.Type )
						{
							snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso NUMBER; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
							myShowErrorMessage(pParams, pParams->szError, 1);
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							return 0;
						}
						// v2:
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
						PrintToken(&(pParams->myToken), ' ', ' ', 1);
						#endif
						GetNextToken(pParams);
						if ( T_INT_LITERAL != pParams->myToken.Type && T_REAL_LITERAL != pParams->myToken.Type )
						{
							snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso NUMBER; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
							myShowErrorMessage(pParams, pParams->szError, 1);
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							return 0;
						}						
					}
												
					if ( T_QCPAREN != pParams->myToken.Type )
					{
						snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso T_QCPAREN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
						myShowErrorMessage(pParams, pParams->szError, 1);
						PrintToken(&(pParams->myToken), ' ', ' ', 1);
						return 0;
					}
				}
				break;
				case T_INT_LITERAL:
				{
					n2 = pParams->myToken.vInt;
			
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
					PrintToken(&(pParams->myToken), ' ', ' ', 1);
					#endif
							
					GetNextToken(pParams);
				
					switch ( pParams->myToken.Type )
					{
						case T_INT_LITERAL:
						{
							dblWidth = (double)pParams->myToken.vInt * 0.001;
												
							if ( pParams->dCurrFontMaxWidth < dblWidth )
								pParams->dCurrFontMaxWidth = dblWidth;
								
							if ( 32 == n1 )
								pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
										
							for ( int k = n1; k <= n2; k++ )
							{										
								pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = dblWidth;
							}
							
							// IGNORIAMO I SUCCESSIVI DUE NUMERI v1, v2:
							// v1:
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							#endif	
							GetNextToken(pParams);
							if ( T_INT_LITERAL != pParams->myToken.Type && T_REAL_LITERAL != pParams->myToken.Type )
							{
								snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso NUMBER; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
								myShowErrorMessage(pParams, pParams->szError, 1);
								PrintToken(&(pParams->myToken), ' ', ' ', 1);
								return 0;
							}
							// v2:
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							#endif
							GetNextToken(pParams);
							if ( T_INT_LITERAL != pParams->myToken.Type && T_REAL_LITERAL != pParams->myToken.Type )
							{
								snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso NUMBER; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
								myShowErrorMessage(pParams, pParams->szError, 1);
								PrintToken(&(pParams->myToken), ' ', ' ', 1);
								return 0;
							}
						}
						break;
						case T_REAL_LITERAL:
						{
							dblWidth = pParams->myToken.vDouble * 0.001;
												
							if ( pParams->dCurrFontMaxWidth < dblWidth )
								pParams->dCurrFontMaxWidth = dblWidth;
								
							if ( 32 == n1 )
								pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = dblWidth;
										
							for ( int k = n1; k <= n2; k++ )
							{										
								pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = dblWidth;
							}
							
							// IGNORIAMO I SUCCESSIVI DUE NUMERI v1, v2:
							// v1:
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							#endif	
							GetNextToken(pParams);
							if ( T_INT_LITERAL != pParams->myToken.Type && T_REAL_LITERAL != pParams->myToken.Type )
							{
								snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso NUMBER; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
								myShowErrorMessage(pParams, pParams->szError, 1);
								PrintToken(&(pParams->myToken), ' ', ' ', 1);
								return 0;
							}
							// v2:
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							#endif
							GetNextToken(pParams);
							if ( T_INT_LITERAL != pParams->myToken.Type && T_REAL_LITERAL != pParams->myToken.Type )
							{
								snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso NUMBER; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
								myShowErrorMessage(pParams, pParams->szError, 1);
								PrintToken(&(pParams->myToken), ' ', ' ', 1);
								return 0;
							}
						}
						break;
						default:
						{
							snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso T_INT_LITERAL o T_QOPAREN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
							myShowErrorMessage(pParams, pParams->szError, 1);
							PrintToken(&(pParams->myToken), ' ', ' ', 1);
							return 0;
						}
						break;
					}
				}
				break;
				default:
				{
					snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso T_INT_LITERAL o T_QOPAREN; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
					myShowErrorMessage(pParams, pParams->szError, 1);
					PrintToken(&(pParams->myToken), ' ', ' ', 1);
					return 0;
				}
				break;
			}
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif		
		
			GetNextToken(pParams);
		}
	}
	else if ( strncmp(pParams->szCurrKeyName, "DW2", 1024) == 0 )
	{
		if ( T_INT_LITERAL == pParams->myToken.Type )
		{
			//dblWidth = (double)pParams->myToken.vInt;
			//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW2.v = dblWidth;	
		}
		else if ( T_REAL_LITERAL == pParams->myToken.Type )
		{
			//dblWidth = pParams->myToken.vDouble;
			//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW2.v = dblWidth;
		}
		else
		{
			snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso T_INT_LITERAL o T_REAL_LITERAL; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
		}
			
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
		
		GetNextToken(pParams);
			
		if ( T_INT_LITERAL == pParams->myToken.Type )
		{
			dblWidth = (double)pParams->myToken.vInt * 0.001;
			//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW2.w1 = dblWidth;	
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW = dblWidth;	
		}
		else if ( T_REAL_LITERAL == pParams->myToken.Type )
		{
			dblWidth = pParams->myToken.vDouble * 0.001;
			//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW2.w1 = dblWidth;
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->DW = dblWidth;
		}
		else
		{
			snprintf(pParams->szError, 8192, "ERRORE cidfontobjkeyarray: Atteso T_INT_LITERAL o T_REAL_LITERAL; trovato invece TOKEN n° %d:\n", pParams->myToken.Type);
			myShowErrorMessage(pParams, pParams->szError, 1);
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			return 0;
		}
	}
	// *************************************************************************************************************************************************************
						
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif
		
	if ( !match(pParams, T_QCPAREN, "cidfontobjkeyarray") )
	{
		return 0;
	}
	
	pParams->bFontWidthsArrayState = 0;	
	
	return 1;
}

// cidfontobjkeydict  : T_DICT_BEGIN { T_NAME cidfontobjkeyvalue } T_DICT_END;
int cidfontobjkeydict(Params *pParams)
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
		
		if ( !cidfontobjkeyvalue(pParams) )
			return 0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
	PrintToken(&(pParams->myToken), ' ', ' ', 1);
	#endif	
	
	if ( !match(pParams, T_DICT_END, "cidfontobjkeydict") )
	{
		return 0;
	}	
	
	return 1;
}

// ------------------------------------------------------------------------------------------------------------------------
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
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
				
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
		if ( strncmp(pParams->szTemp, "WinAnsiEncoding", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
		{
			pParams->pCurrentEncodingArray = &(pParams->aWIN_CharSet[0]);
		}
		else if ( strncmp(pParams->szTemp, "MacRomanEncoding", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
		{
			pParams->pCurrentEncodingArray = &(pParams->aMAC_CharSet[0]);
		}
		else if ( strncmp(pParams->szTemp, "MacExpertEncoding", strnlen(pParams->myToken.vString, 4096) + 1) == 0 )
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
		strncpy(pParams->szCurrKeyName, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
		
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
//                              | T_INT_LITERAL 
//                              | encodingobjarray
//                              ;
int encodingobjdictitemskeyvalues(Params *pParams)
{	
	if ( T_NAME == pParams->myToken.Type )
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
		
		if ( strncmp(pParams->szCurrKeyName, "BaseEncoding", 1024) == 0 )
		{
			strncpy(pParams->szTemp, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
		}
		
		GetNextToken(pParams);
	}
	else if (T_INT_LITERAL == pParams->myToken.Type || T_REAL_LITERAL == pParams->myToken.Type)
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
		PrintToken(&(pParams->myToken), ' ', ' ', 1);
		#endif		
				
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
		keyValue = pParams->myToken.vInt;
		
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
		
		len = strnlen(pParams->myToken.vString, 4096);
		nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.vString, len + sizeof(char), (void*)&nData, &nDataSize, &bContentAlreadyProcessed);
		if ( nRes >= 0 ) // TROVATO
		{				
			pParams->paCustomizedFont_CharSet[keyValue] = nData;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
			wprintf(L"\tencodingobjarray KEY -> '%s' -> Value = %d -> char = '%lc'\n", pParams->myToken.vString, keyValue, (wchar_t)pParams->paCustomizedFont_CharSet[keyValue]);
			#endif			
		}
		else
		{
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
			wprintf(L"encodingobjarray -> WARNING: KEY '%s' NON TROVATA\n", pParams->myToken.vString);
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
			
			len = strnlen(pParams->myToken.vString, 4096);
			nRes = htFind(&(pParams->myCharSetHashTable), pParams->myToken.vString, len + sizeof(char), (void*)&nData, &nDataSize, &bContentAlreadyProcessed);
			if ( nRes >= 0 ) // TROVATO
			{							
				pParams->paCustomizedFont_CharSet[keyValue] = nData;
								
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
				wprintf(L"\tencodingobjarray KEY -> '%s' -> Value = %d -> char = '%lc'\n", pParams->myToken.vString, keyValue, (wchar_t)pParams->paCustomizedFont_CharSet[keyValue]);
				#endif			
			}
			else
			{
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
				wprintf(L"encodingobjarray -> WARNING: KEY(%d) '%s' NON TROVATA\n", keyValue, pParams->myToken.vString);
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
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
				
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
		strncpy(pParams->szCurrKeyName, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
		
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
			n1 = pParams->myToken.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.vInt;
				
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
				if ( strncmp(pParams->myToken.vString, "FlateDecode", 1024) != 0 )
				{
					snprintf(pParams->szError, 8192, "ERRORE xrefstream_keyvalue(bStreamStateToUnicode): Flitri diversi da FlateDecode non supportati in questa versione del programma.\n");
					myShowErrorMessage(pParams, pParams->szError, 1);
					//wprintf(L"ERRORE xrefstream_keyvalue(bStreamStateToUnicode): Flitri diversi da FlateDecode non supportati in questa versione del programma.\n");
					//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_keyvalue(bStreamStateToUnicode): Flitri diversi da FlateDecode non supportati in questa versione del programma.\n");
					return 0;					
				}
				
				mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.vString);
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
					myintqueuelist_Enqueue(	&(pParams->queueTrailerIndex), pParams->myToken.vInt);
				}
				else if ( strncmp(pParams->szCurrKeyName, "W", 1024) == 0 )
				{
					myintqueuelist_Enqueue(	&(pParams->queueTrailerW), pParams->myToken.vInt);
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
					mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.vString);
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
	
	len = strnlen(pParams->myToken.vString, 1024);	
	pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));	
	if ( NULL == pParams->myDataDecodeParams.pszKey )
	{
		snprintf(pParams->szError, 8192, "ERRORE xrefstream_keydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE xrefstream_keydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_keydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
		return 0;
	}
	strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.vString, len + sizeof(char));
	
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
		len = strnlen(pParams->myToken.vString, 1024);	
		pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));
		if ( NULL == pParams->myDataDecodeParams.pszKey )
		{
			snprintf(pParams->szError, 8192, "ERRORE xrefstream_keydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE xrefstream_keydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			//fwprintf(pParams->fpErrors, L"ERRORE xrefstream_keydict: impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", sizeof(char) * len + sizeof(char));
			return 0;
		}
		strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.vString, len + sizeof(char));
		
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
			n1 = pParams->myToken.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STREAMOBJ) || defined(MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			
			GetNextToken(pParams);		
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.vInt;
				
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
					pParams->myDataDecodeParams.tok.vInt = n1;
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
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
				
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
		strncpy(pParams->szCurrKeyName, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
		
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
			n1 = pParams->myToken.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif
			
			GetNextToken(pParams);
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.vInt;
				
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
				if ( (strncmp(pParams->myToken.vString, "FlateDecode", 1024) == 0) )
				{
					mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.vString);
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
					mystringqueuelist_Enqueue(&(pParams->CurrentContent.queueFilters), pParams->myToken.vString);
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
	
	len = strnlen(pParams->myToken.vString, 1024);	
	pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));	
	if ( NULL == pParams->myDataDecodeParams.pszKey )
	{
		snprintf(pParams->szError, 8192, "ERRORE stmobjkeydict(OBJ num = %u, gen = %u): impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, sizeof(char) * len + sizeof(char));
		myShowErrorMessage(pParams, pParams->szError, 1);
		//wprintf(L"ERRORE stmobjkeydict(OBJ num = %lu, gen = %lu): impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, sizeof(char) * len + sizeof(char));
		//fwprintf(pParams->fpErrors, L"ERRORE stmobjkeydict(OBJ num = %lu, gen = %lu): impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, sizeof(char) * len + sizeof(char));
		return 0;
	}
	strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.vString, len + sizeof(char));
	
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
		len = strnlen(pParams->myToken.vString, 1024);	
		pParams->myDataDecodeParams.pszKey = (char*)malloc(sizeof(char) * len + sizeof(char));
		if ( NULL == pParams->myDataDecodeParams.pszKey )
		{
			snprintf(pParams->szError, 8192, "ERRORE stmobjkeydict(OBJ num = %u, gen = %u): impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, sizeof(char) * len + sizeof(char));
			myShowErrorMessage(pParams, pParams->szError, 1);
			//wprintf(L"ERRORE stmobjkeydict(OBJ num = %lu, gen = %lu): impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, sizeof(char) * len + sizeof(char));
			//fwprintf(pParams->fpErrors, L"ERRORE stmobjkeydict(OBJ num = %lu, gen = %lu): impossibile allocare %lu bytes di memoria per pParams->myDataDecodeParams.pszKey\n", pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Number, pParams->myObjsTable[pParams->nCurrentParsingObj]->Obj.Generation, sizeof(char) * len + sizeof(char));
			return 0;
		}
		strncpy(pParams->myDataDecodeParams.pszKey, pParams->myToken.vString, len + sizeof(char));
		
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
			n1 = pParams->myToken.vInt;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_STMOBJ)	
			PrintToken(&(pParams->myToken), ' ', ' ', 1);
			#endif			
			
			GetNextToken(pParams);		
			
			if ( pParams->myToken.Type == T_INT_LITERAL)
			{
				n2 = pParams->myToken.vInt;
				
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
					pParams->myDataDecodeParams.tok.vInt = n1;
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
	pParams->nCurrentParsingObj = pParams->myToken.vInt;
				
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
		strncpy(pParams->szCurrKeyName, pParams->myToken.vString, strnlen(pParams->myToken.vString, 4096) + 1);
						
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
				if ( strncmp(pParams->myToken.vString, "ObjStm", 1024) == 0 )
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

// ******************************************************************************************************************************************

void setPredefFontsWidthsArray(Params *pParams)
{
	int k;
	
	//for ( k = 0; k < pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->WidthsArraySize; k++ )
	//{
	//	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->MissingWidth;
	//}
		
	switch ( pParams->nCurrentBaseFont )
	{
		case BASEFONT_Courier:
			//for ( k = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar; k <= pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar; k++ )
			//{
			//	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = 0.600;
			//}
			//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.600;
			//break;
		case BASEFONT_Courier_Bold:
			//for ( k = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar; k <= pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar; k++ )
			//{
			//	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = 0.600;
			//}
			//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.600;
			//break;
		case BASEFONT_Courier_BoldOblique:
			//for ( k = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar; k <= pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar; k++ )
			//{
			//	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = 0.600;
			//}
			//pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.600;
			//break;
		case BASEFONT_Courier_Oblique:
			for ( k = pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->FirstChar; k <= pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->LastChar; k++ )
			{
				pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[k] = 0.600;
			}
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.600;
			break;
		case BASEFONT_Helvetica:
			set_Helvetica(pParams);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.278;
			break;
		case BASEFONT_Helvetica_Bold:
			set_Helvetica_Bold(pParams);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.278;
			break;
		case BASEFONT_Helvetica_BoldOblique:
			set_Helvetica_BoldOblique(pParams);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.278;
			break;
		case BASEFONT_Helvetica_Oblique:
			set_Helvetica_Oblique(pParams);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.278;
			break;
		case BASEFONT_Symbol:
			set_Symbol(pParams);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.250;
			break;
		case BASEFONT_Times_Bold:
			set_Times_Bold(pParams);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.250;
			break;
		case BASEFONT_Times_BoldItalic:
			set_Times_BoldItalic(pParams);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.250;
			break;
		case BASEFONT_Times_Italic:
			set_Times_Italic(pParams);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.250;
			break;
		case BASEFONT_Times_Roman:
			set_Times_Roman(pParams);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.250;
			break;
		case BASEFONT_ZapfDingbats:
			set_ZapfDingbats(pParams);
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.278;
			break;
		default:				
			pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->dFontSpaceWidth = 0.250;
			break;
	}
}

void set_Helvetica(Params *pParams)
{
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[32] = 0.278; // space ; B 0 0 0 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[33] = 0.278; // exclam ; B 90 0 187 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[34] = 0.355; // quotedbl ; B 70 463 285 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[35] = 0.556; // numbersign ; B 28 0 529 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[36] = 0.556; // dollar ; B 32 -115 520 775 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[37] = 0.889; // percent ; B 39 -19 850 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[38] = 0.667; // ampersand ; B 44 -15 645 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[39] = 0.222; // quoteright ; B 53 463 157 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[40] = 0.333; // parenleft ; B 68 -207 299 733 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[41] = 0.333; // parenright ; B 34 -207 265 733 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[42] = 0.389; // asterisk ; B 39 431 349 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[43] = 0.584; // plus ; B 39 0 545 505 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[44] = 0.278; // comma ; B 87 -147 191 106 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[45] = 0.333; // hyphen ; B 44 232 289 322 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[46] = 0.278; // period ; B 87 0 191 106 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[47] = 0.278; // slash ; B -17 -19 295 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[48] = 0.556; // zero ; B 37 -19 519 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[49] = 0.556; // one ; B 101 0 359 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[50] = 0.556; // two ; B 26 0 507 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[51] = 0.556; // three ; B 34 -19 522 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[52] = 0.556; // four ; B 25 0 523 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[53] = 0.556; // five ; B 32 -19 514 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[54] = 0.556; // six ; B 38 -19 518 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[55] = 0.556; // seven ; B 37 0 523 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[56] = 0.556; // eight ; B 38 -19 517 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[57] = 0.556; // nine ; B 42 -19 514 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[58] = 0.278; // colon ; B 87 0 191 516 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[59] = 0.278; // semicolon ; B 87 -147 191 516 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[60] = 0.584; // less ; B 48 11 536 495 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[61] = 0.584; // equal ; B 39 115 545 390 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[62] = 0.584; // greater ; B 48 11 536 495 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[63] = 0.556; // question ; B 56 0 492 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[64] = 1.015; // at ; B 147 -19 868 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[65] = 0.667; // A ; B 14 0 654 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[66] = 0.667; // B ; B 74 0 627 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[67] = 0.722; // C ; B 44 -19 681 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[68] = 0.722; // D ; B 81 0 674 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[69] = 0.667; // E ; B 86 0 616 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[70] = 0.611; // F ; B 86 0 583 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[71] = 0.778; // G ; B 48 -19 704 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[72] = 0.722; // H ; B 77 0 646 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[73] = 0.278; // I ; B 91 0 188 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[74] = 0.500; // J ; B 17 -19 428 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[75] = 0.667; // K ; B 76 0 663 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[76] = 0.556; // L ; B 76 0 537 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[77] = 0.833; // M ; B 73 0 761 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[78] = 0.722; // N ; B 76 0 646 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[79] = 0.778; // O ; B 39 -19 739 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[80] = 0.667; // P ; B 86 0 622 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[81] = 0.778; // Q ; B 39 -56 739 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[82] = 0.722; // R ; B 88 0 684 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[83] = 0.667; // S ; B 49 -19 620 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[84] = 0.611; // T ; B 14 0 597 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[85] = 0.722; // U ; B 79 -19 644 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[86] = 0.667; // V ; B 20 0 647 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[87] = 0.944; // W ; B 16 0 928 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[88] = 0.667; // X ; B 19 0 648 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[89] = 0.667; // Y ; B 14 0 653 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[90] = 0.611; // Z ; B 23 0 588 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[91] = 0.278; // bracketleft ; B 63 -196 250 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[92] = 0.278; // backslash ; B -17 -19 295 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[93] = 0.278; // bracketright ; B 28 -196 215 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[94] = 0.469; // asciicircum ; B -14 264 483 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[95] = 0.556; // underscore ; B 0 -125 556 -75 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[96] = 0.222; // quoteleft ; B 65 470 169 725 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[97] = 0.556; // a ; B 36 -15 530 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[98] = 0.556; // b ; B 58 -15 517 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[99] = 0.500; // c ; B 30 -15 477 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[100] = 0.556; // d ; B 35 -15 499 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[101] = 0.556; // e ; B 40 -15 516 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[102] = 0.278; // f ; B 14 0 262 728 ; L i fi ; L l fl ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[103] = 0.556; // g ; B 40 -220 499 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[104] = 0.556; // h ; B 65 0 491 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[105] = 0.222; // i ; B 67 0 155 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[106] = 0.222; // j ; B -16 -210 155 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[107] = 0.500; // k ; B 67 0 501 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[108] = 0.222; // l ; B 67 0 155 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[109] = 0.833; // m ; B 65 0 769 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[110] = 0.556; // n ; B 65 0 491 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[111] = 0.556; // o ; B 35 -14 521 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[112] = 0.556; // p ; B 58 -207 517 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[113] = 0.556; // q ; B 35 -207 494 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[114] = 0.333; // r ; B 77 0 332 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[115] = 0.500; // s ; B 32 -15 464 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[116] = 0.278; // t ; B 14 -7 257 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[117] = 0.556; // u ; B 68 -15 489 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[118] = 0.500; // v ; B 8 0 492 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[119] = 0.722; // w ; B 14 0 709 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[120] = 0.500; // x ; B 11 0 490 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[121] = 0.500; // y ; B 11 -214 489 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[122] = 0.500; // z ; B 31 0 469 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[123] = 0.334; // braceleft ; B 42 -196 292 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[124] = 0.260; // bar ; B 94 -225 167 775 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[125] = 0.334; // braceright ; B 42 -196 292 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[126] = 0.584; // asciitilde ; B 61 180 523 326 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[161] = 0.333; // exclamdown ; B 118 -195 215 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[162] = 0.556; // cent ; B 51 -115 513 623 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[163] = 0.556; // sterling ; B 33 -16 539 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[164] = 0.167; // fraction ; B -166 -19 333 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[165] = 0.556; // yen ; B 3 0 553 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[166] = 0.556; // florin ; B -11 -207 501 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[167] = 0.556; // section ; B 43 -191 512 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[168] = 0.556; // currency ; B 28 99 528 603 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[169] = 0.191; // quotesingle ; B 59 463 132 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[170] = 0.333; // quotedblleft ; B 38 470 307 725 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[171] = 0.556; // guillemotleft ; B 97 108 459 446 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[172] = 0.333; // guilsinglleft ; B 88 108 245 446 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[173] = 0.333; // guilsinglright ; B 88 108 245 446 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[174] = 0.500; // fi ; B 14 0 434 728 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[175] = 0.500; // fl ; B 14 0 432 728 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[177] = 0.556; // endash ; B 0 240 556 313 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[178] = 0.556; // dagger ; B 43 -159 514 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[179] = 0.556; // daggerdbl ; B 43 -159 514 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[180] = 0.278; // periodcentered ; B 77 190 202 315 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[182] = 0.537; // paragraph ; B 18 -173 497 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[183] = 0.350; // bullet ; B 18 202 333 517 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[184] = 0.222; // quotesinglbase ; B 53 -149 157 106 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[185] = 0.333; // quotedblbase ; B 26 -149 295 106 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[186] = 0.333; // quotedblright ; B 26 463 295 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[187] = 0.556; // guillemotright ; B 97 108 459 446 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[188] = 1.000; // ellipsis ; B 115 0 885 106 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[189] = 1.000; // perthousand ; B 7 -19 994 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[191] = 0.611; // questiondown ; B 91 -201 527 525 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[193] = 0.333; // grave ; B 14 593 211 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[194] = 0.333; // acute ; B 122 593 319 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[195] = 0.333; // circumflex ; B 21 593 312 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[196] = 0.333; // tilde ; B -4 606 337 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[197] = 0.333; // macron ; B 10 627 323 684 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[198] = 0.333; // breve ; B 13 595 321 731 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[199] = 0.333; // dotaccent ; B 121 604 212 706 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[200] = 0.333; // dieresis ; B 40 604 293 706 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[202] = 0.333; // ring ; B 75 572 259 756 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[203] = 0.333; // cedilla ; B 45 -225 259 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[205] = 0.333; // hungarumlaut ; B 31 593 409 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[206] = 0.333; // ogonek ; B 73 -225 287 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[207] = 0.333; // caron ; B 21 593 312 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[208] = 1.000; // emdash ; B 0 240 1000 313 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[225] = 1.000; // AE ; B 8 0 951 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[227] = 0.370; // ordfeminine ; B 24 405 346 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[232] = 0.556; // Lslash ; B -20 0 537 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[233] = 0.778; // Oslash ; B 39 -19 740 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[234] = 1.000; // OE ; B 36 -19 965 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[235] = 0.365; // ordmasculine ; B 25 405 341 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[241] = 0.889; // ae ; B 36 -15 847 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[245] = 0.278; // dotlessi ; B 95 0 183 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[248] = 0.222; // lslash ; B -20 0 242 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[249] = 0.611; // oslash ; B 28 -22 537 545 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[250] = 0.944; // oe ; B 35 -15 902 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[251] = 0.611; // germandbls ; B 67 -15 571 728 ;
}

void set_Helvetica_Bold(Params *pParams)
{
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[32] = 0.278; // N space ; B 0 0 0 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[33] = 0.333; // exclam ; B 90 0 244 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[34] = 0.474; // quotedbl ; B 98 447 376 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[35] = 0.556; // numbersign ; B 18 0 538 698 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[36] = 0.556; // dollar ; B 30 -115 523 775 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[37] = 0.889; // percent ; B 28 -19 861 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[38] = 0.722; // ampersand ; B 54 -19 701 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[39] = 0.278; // quoteright ; B 69 445 209 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[40] = 0.333; // parenleft ; B 35 -208 314 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[41] = 0.333; // parenright ; B 19 -208 298 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[42] = 0.389; // asterisk ; B 27 387 362 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[43] = 0.584; // plus ; B 40 0 544 506 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[44] = 0.278; // comma ; B 64 -168 214 146 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[45] = 0.333; // hyphen ; B 27 215 306 345 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[46] = 0.278; // period ; B 64 0 214 146 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[47] = 0.278; // slash ; B -33 -19 311 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[48] = 0.556; // zero ; B 32 -19 524 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[49] = 0.556; // one ; B 69 0 378 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[50] = 0.556; // two ; B 26 0 511 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[51] = 0.556; // three ; B 27 -19 516 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[52] = 0.556; // four ; B 27 0 526 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[53] = 0.556; // five ; B 27 -19 516 698 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[54] = 0.556; // six ; B 31 -19 520 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[55] = 0.556; // seven ; B 25 0 528 698 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[56] = 0.556; // eight ; B 32 -19 524 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[57] = 0.556; // nine ; B 30 -19 522 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[58] = 0.333; // colon ; B 92 0 242 512 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[59] = 0.333; // semicolon ; B 92 -168 242 512 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[60] = 0.584; // less ; B 38 -8 546 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[61] = 0.584; // equal ; B 40 87 544 419 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[62] = 0.584; // greater ; B 38 -8 546 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[63] = 0.611; // question ; B 60 0 556 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[64] = 0.975; // at ; B 118 -19 856 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[65] = 0.722; // A ; B 20 0 702 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[66] = 0.722; // B ; B 76 0 669 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[67] = 0.722; // C ; B 44 -19 684 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[68] = 0.722; // D ; B 76 0 685 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[69] = 0.667; // E ; B 76 0 621 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[70] = 0.611; // F ; B 76 0 587 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[71] = 0.778; // G ; B 44 -19 713 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[72] = 0.722; // H ; B 71 0 651 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[73] = 0.278; // I ; B 64 0 214 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[74] = 0.556; // J ; B 22 -18 484 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[75] = 0.722; // K ; B 87 0 722 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[76] = 0.611; // L ; B 76 0 583 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[77] = 0.833; // M ; B 69 0 765 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[78] = 0.722; // N ; B 69 0 654 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[79] = 0.778; // O ; B 44 -19 734 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[80] = 0.667; // P ; B 76 0 627 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[81] = 0.778; // Q ; B 44 -52 737 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[82] = 0.722; // R ; B 76 0 677 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[83] = 0.667; // S ; B 39 -19 629 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[84] = 0.611; // T ; B 14 0 598 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[85] = 0.722; // U ; B 72 -19 651 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[86] = 0.667; // V ; B 19 0 648 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[87] = 0.944; // W ; B 16 0 929 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[88] = 0.667; // X ; B 14 0 653 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[89] = 0.667; // Y ; B 15 0 653 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[90] = 0.611; // Z ; B 25 0 586 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[91] = 0.333; // bracketleft ; B 63 -196 309 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[92] = 0.278; // backslash ; B -33 -19 311 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[93] = 0.333; // bracketright ; B 24 -196 270 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[94] = 0.584; // asciicircum ; B 62 323 522 698 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[95] = 0.556; // underscore ; B 0 -125 556 -75 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[96] = 0.278; // quoteleft ; B 69 454 209 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[97] = 0.556; // a ; B 29 -14 527 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[98] = 0.611; // b ; B 61 -14 578 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[99] = 0.556; // c ; B 34 -14 524 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[100] = 0.611; // d ; B 34 -14 551 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[101] = 0.556; // e ; B 23 -14 528 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[102] = 0.333; // f ; B 10 0 318 727 ; L i fi ; L l fl ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[103] = 0.611; // g ; B 40 -217 553 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[104] = 0.611; // h ; B 65 0 546 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[105] = 0.278; // i ; B 69 0 209 725 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[106] = 0.278; // j ; B 3 -214 209 725 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[107] = 0.556; // k ; B 69 0 562 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[108] = 0.278; // l ; B 69 0 209 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[109] = 0.889; // m ; B 64 0 826 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[110] = 0.611; // n ; B 65 0 546 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[111] = 0.611; // o ; B 34 -14 578 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[112] = 0.611; // p ; B 62 -207 578 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[113] = 0.611; // q ; B 34 -207 552 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[114] = 0.389; // r ; B 64 0 373 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[115] = 0.556; // s ; B 30 -14 519 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[116] = 0.333; // t ; B 10 -6 309 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[117] = 0.611; // u ; B 66 -14 545 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[118] = 0.556; // v ; B 13 0 543 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[119] = 0.778; // w ; B 10 0 769 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[120] = 0.556; // x ; B 15 0 541 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[121] = 0.556; // y ; B 10 -214 539 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[122] = 0.500; // z ; B 20 0 480 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[123] = 0.389; // braceleft ; B 48 -196 365 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[124] = 0.280; // bar ; B 84 -225 196 775 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[125] = 0.389; // braceright ; B 24 -196 341 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[126] = 0.584; // asciitilde ; B 61 163 523 343 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[161] = 0.333; // exclamdown ; B 90 -186 244 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[162] = 0.556; // cent ; B 34 -118 524 628 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[163] = 0.556; // sterling ; B 28 -16 541 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[164] = 0.167; // fraction ; B -170 -19 336 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[165] = 0.556; // yen ; B -9 0 565 698 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[166] = 0.556; // florin ; B -10 -210 516 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[167] = 0.556; // section ; B 34 -184 522 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[168] = 0.556; // currency ; B -3 76 559 636 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[169] = 0.238; // quotesingle ; B 70 447 168 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[170] = 0.500; // quotedblleft ; B 64 454 436 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[171] = 0.556; // guillemotleft ; B 88 76 468 484 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[172] = 0.333; // guilsinglleft ; B 83 76 250 484 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[173] = 0.333; // guilsinglright ; B 83 76 250 484 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[174] = 0.611; // fi ; B 10 0 542 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[175] = 0.611; // fl ; B 10 0 542 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[177] = 0.556; // endash ; B 0 227 556 333 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[178] = 0.556; // dagger ; B 36 -171 520 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[179] = 0.556; // daggerdbl ; B 36 -171 520 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[180] = 0.278; // periodcentered ; B 58 172 220 334 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[182] = 0.556; // paragraph ; B -8 -191 539 700 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[183] = 0.350; // bullet ; B 10 194 340 524 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[184] = 0.278; // quotesinglbase ; B 69 -146 209 127 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[185] = 0.500; // quotedblbase ; B 64 -146 436 127 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[186] = 0.500; // quotedblright ; B 64 445 436 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[187] = 0.556; // guillemotright ; B 88 76 468 484 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[188] = 1.000; // ellipsis ; B 92 0 908 146 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[189] = 1.000; // perthousand ; B -3 -19 1003 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[191] = 0.611; // questiondown ; B 55 -195 551 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[193] = 0.333; // grave ; B -23 604 225 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[194] = 0.333; // acute ; B 108 604 356 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[195] = 0.333; // circumflex ; B -10 604 343 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[196] = 0.333; // tilde ; B -17 610 350 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[197] = 0.333; // macron ; B -6 604 339 678 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[198] = 0.333; // breve ; B -2 604 335 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[199] = 0.333; // dotaccent ; B 104 614 230 729 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[200] = 0.333; // dieresis ; B 6 614 327 729 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[202] = 0.333; // ring ; B 59 568 275 776 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[203] = 0.333; // cedilla ; B 6 -228 245 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[205] = 0.333; // hungarumlaut ; B 9 604 486 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[206] = 0.333; // ogonek ; B 71 -228 304 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[207] = 0.333; // caron ; B -10 604 343 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[208] = 1.000; // emdash ; B 0 227 1000 333 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[225] = 1.000; // AE ; B 5 0 954 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[227] = 0.370; // ordfeminine ; B 22 401 347 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[232] = 0.611; // Lslash ; B -20 0 583 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[233] = 0.778; // Oslash ; B 33 -27 744 745 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[234] = 1.000; // OE ; B 37 -19 961 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[235] = 0.365; // ordmasculine ; B 6 401 360 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[241] = 0.889; // ae ; B 29 -14 858 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[245] = 0.278; // dotlessi ; B 69 0 209 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[248] = 0.278; // lslash ; B -18 0 296 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[249] = 0.611; // oslash ; B 22 -29 589 560 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[250] = 0.944; // oe ; B 34 -14 912 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[251] = 0.611; // germandbls ; B 69 -14 579 731 ;
}

void set_Helvetica_BoldOblique(Params *pParams)
{
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[32] = 0.278; // space ; B 0 0 0 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[33] = 0.333; // exclam ; B 94 0 397 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[34] = 0.474; // quotedbl ; B 193 447 529 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[35] = 0.556; // numbersign ; B 60 0 644 698 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[36] = 0.556; // dollar ; B 67 -115 622 775 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[37] = 0.889; // percent ; B 136 -19 901 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[38] = 0.722; // ampersand ; B 89 -19 732 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[39] = 0.278; // quoteright ; B 167 445 362 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[40] = 0.333; // parenleft ; B 76 -208 470 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[41] = 0.333; // parenright ; B -25 -208 369 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[42] = 0.389; // asterisk ; B 146 387 481 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[43] = 0.584; // plus ; B 82 0 610 506 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[44] = 0.278; // comma ; B 28 -168 245 146 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[45] = 0.333; // hyphen ; B 73 215 379 345 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[46] = 0.278; // period ; B 64 0 245 146 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[47] = 0.278; // slash ; B -37 -19 468 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[48] = 0.556; // zero ; B 86 -19 617 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[49] = 0.556; // one ; B 173 0 529 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[50] = 0.556; // two ; B 26 0 619 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[51] = 0.556; // three ; B 65 -19 608 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[52] = 0.556; // four ; B 60 0 598 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[53] = 0.556; // five ; B 64 -19 636 698 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[54] = 0.556; // six ; B 85 -19 619 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[55] = 0.556; // seven ; B 125 0 676 698 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[56] = 0.556; // eight ; B 69 -19 616 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[57] = 0.556; // nine ; B 78 -19 615 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[58] = 0.333; // colon ; B 92 0 351 512 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[59] = 0.333; // semicolon ; B 56 -168 351 512 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[60] = 0.584; // less ; B 82 -8 655 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[61] = 0.584; // equal ; B 58 87 633 419 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[62] = 0.584; // greater ; B 36 -8 609 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[63] = 0.611; // question ; B 165 0 671 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[64] = 0.975; // at ; B 186 -19 954 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[65] = 0.722; // A ; B 20 0 702 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[66] = 0.722; // B ; B 76 0 764 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[67] = 0.722; // pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[; B 107 -19 789 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[68] = 0.722; // D ; B 76 0 777 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[69] = 0.667; // E ; B 76 0 757 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[70] = 0.611; // F ; B 76 0 740 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[71] = 0.778; // G ; B 108 -19 817 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[72] = 0.722; // H ; B 71 0 804 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[73] = 0.278; // I ; B 64 0 367 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[74] = 0.556; // J ; B 60 -18 637 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[75] = 0.722; // K ; B 87 0 858 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[76] = 0.611; // L ; B 76 0 611 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[77] = 0.833; // M ; B 69 0 918 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[78] = 0.722; // N ; B 69 0 807 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[79] = 0.778; // O ; B 107 -19 823 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[80] = 0.667; // P ; B 76 0 738 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[81] = 0.778; // Q ; B 107 -52 823 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[82] = 0.722; // R ; B 76 0 778 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[83] = 0.667; // S ; B 81 -19 718 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[84] = 0.611; // T ; B 140 0 751 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[85] = 0.722; // U ; B 116 -19 804 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[86] = 0.667; // V ; B 172 0 801 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[87] = 0.944; // W ; B 169 0 1082 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[88] = 0.667; // X ; B 14 0 791 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[89] = 0.667; // Y ; B 168 0 806 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[90] = 0.611; // Z ; B 25 0 737 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[91] = 0.333; // bracketleft ; B 21 -196 462 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[92] = 0.278; // backslash ; B 124 -19 307 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[93] = 0.333; // bracketright ; B -18 -196 423 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[94] = 0.584; // asciicircum ; B 131 323 591 698 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[95] = 0.556; // underscore ; B -27 -125 540 -75 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[96] = 0.278; // quoteleft ; B 165 454 361 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[97] = 0.556; // a ; B 55 -14 583 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[98] = 0.611; // b ; B 61 -14 645 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[99] = 0.556; // c ; B 79 -14 599 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[100] = 0.611; // d ; B 82 -14 704 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[101] = 0.556; // e ; B 70 -14 593 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[102] = 0.333; // f ; B 87 0 469 727 ; L i fi ; L l fl ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[103] = 0.611; // g ; B 38 -217 666 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[104] = 0.611; // h ; B 65 0 629 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[105] = 0.278; // i ; B 69 0 363 725 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[106] = 0.278; // j ; B -42 -214 363 725 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[107] = 0.556; // k ; B 69 0 670 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[108] = 0.278; // l ; B 69 0 362 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[109] = 0.889; // m ; B 64 0 909 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[110] = 0.611; // n ; B 65 0 629 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[111] = 0.611; // o ; B 82 -14 643 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[112] = 0.611; // p ; B 18 -207 645 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[113] = 0.611; // q ; B 80 -207 665 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[114] = 0.389; // r ; B 64 0 489 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[115] = 0.556; // s ; B 63 -14 584 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[116] = 0.333; // t ; B 100 -6 422 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[117] = 0.611; // u ; B 98 -14 658 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[118] = 0.556; // v ; B 126 0 656 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[119] = 0.778; // w ; B 123 0 882 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[120] = 0.556; // x ; B 15 0 648 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[121] = 0.556; // y ; B 42 -214 652 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[122] = 0.500; // z ; B 20 0 583 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[123] = 0.389; // braceleft ; B 94 -196 518 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[124] = 0.280; // bar ; B 36 -225 361 775 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[125] = 0.389; // braceright ; B -18 -196 407 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[126] = 0.584; // asciitilde ; B 115 163 577 343 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[161] = 0.333; // exclamdown ; B 50 -186 353 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[162] = 0.556; // cent ; B 79 -118 599 628 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[163] = 0.556; // sterling ; B 50 -16 635 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[164] = 0.167; // fraction ; B -174 -19 487 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[165] = 0.556; // yen ; B 60 0 713 698 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[166] = 0.556; // florin ; B -50 -210 669 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[167] = 0.556; // section ; B 61 -184 598 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[168] = 0.556; // currency ; B 27 76 680 636 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[169] = 0.238; // quotesingle ; B 165 447 321 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[170] = 0.500; // quotedblleft ; B 160 454 588 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[171] = 0.556; // guillemotleft ; B 135 76 571 484 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[172] = 0.333; // guilsinglleft ; B 130 76 353 484 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[173] = 0.333; // guilsinglright ; B 99 76 322 484 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[174] = 0.611; // fi ; B 87 0 696 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[175] = 0.611; // fl ; B 87 0 695 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[177] = 0.556; // endash ; B 48 227 627 333 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[178] = 0.556; // dagger ; B 118 -171 626 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[179] = 0.556; // daggerdbl ; B 46 -171 628 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[180] = 0.278; // periodcentered ; B 110 172 276 334 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[182] = 0.556; // paragraph ; B 98 -191 688 700 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[183] = 0.350; // bullet ; B 83 194 420 524 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[184] = 0.278; // quotesinglbase ; B 41 -146 236 127 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[185] = 0.500; // quotedblbase ; B 36 -146 463 127 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[186] = 0.500; // quotedblright ; B 162 445 589 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[187] = 0.556; // guillemotright ; B 104 76 540 484 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[188] = 1.000; // ellipsis ; B 92 0 939 146 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[189] = 1.000; // perthousand ; B 76 -19 1038 710 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[191] = 0.611; // questiondown ; B 53 -195 559 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[193] = 0.333; // grave ; B 136 604 353 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[194] = 0.333; // acute ; B 236 604 515 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[195] = 0.333; // circumflex ; B 118 604 471 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[196] = 0.333; // tilde ; B 113 610 507 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[197] = 0.333; // macron ; B 122 604 483 678 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[198] = 0.333; // breve ; B 156 604 494 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[199] = 0.333; // dotaccent ; B 235 614 385 729 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[200] = 0.333; // dieresis ; B 137 614 482 729 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[202] = 0.333; // ring ; B 200 568 420 776 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[203] = 0.333; // cedilla ; B -37 -228 220 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[205] = 0.333; // hungarumlaut ; B 137 604 645 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[206] = 0.333; // ogonek ; B 41 -228 264 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[207] = 0.333; // caron ; B 149 604 502 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[208] = 1.000; // emdash ; B 48 227 1071 333 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[225] = 1.000; // AE ; B 5 0 1100 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[227] = 0.370; // ordfeminine ; B 125 401 465 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[232] = 0.611; // Lslash ; B 34 0 611 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[233] = 0.778; // Oslash ; B 35 -27 894 745 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[234] = 1.000; // OE ; B 99 -19 1114 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[235] = 0.365; // ordmasculine ; B 123 401 485 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[241] = 0.889; // ae ; B 56 -14 923 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[245] = 0.278; // dotlessi ; B 69 0 322 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[248] = 0.278; // lslash ; B 40 0 407 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[249] = 0.611; // oslash ; B 22 -29 701 560 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[250] = 0.944; // oe ; B 82 -14 977 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[251] = 0.611; // germandbls ; B 69 -14 657 731 ;
}

void set_Helvetica_Oblique(Params *pParams)
{
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[32] = 0.278; // space ; B 0 0 0 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[33] = 0.278; // exclam ; B 90 0 340 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[34] = 0.355; // quotedbl ; B 168 463 438 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[35] = 0.556; // numbersign ; B 73 0 631 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[36] = 0.556; // dollar ; B 69 -115 617 775 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[37] = 0.889; // percent ; B 147 -19 889 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[38] = 0.667; // ampersand ; B 77 -15 647 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[39] = 0.222; // quoteright ; B 151 463 310 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[40] = 0.333; // parenleft ; B 108 -207 454 733 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[41] = 0.333; // parenright ; B -9 -207 337 733 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[42] = 0.389; // asterisk ; B 165 431 475 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[43] = 0.584; // plus ; B 85 0 606 505 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[44] = 0.278; // comma ; B 56 -147 214 106 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[45] = 0.333; // hyphen ; B 93 232 357 322 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[46] = 0.278; // period ; B 87 0 214 106 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[47] = 0.278; // slash ; B -21 -19 452 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[48] = 0.556; // zero ; B 93 -19 608 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[49] = 0.556; // one ; B 207 0 508 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[50] = 0.556; // two ; B 26 0 617 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[51] = 0.556; // three ; B 75 -19 610 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[52] = 0.556; // four ; B 61 0 576 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[53] = 0.556; // five ; B 68 -19 621 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[54] = 0.556; // six ; B 91 -19 615 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[55] = 0.556; // seven ; B 137 0 669 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[56] = 0.556; // eight ; B 74 -19 607 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[57] = 0.556; // nine ; B 82 -19 609 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[58] = 0.278; // colon ; B 87 0 301 516 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[59] = 0.278; // semicolon ; B 56 -147 301 516 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[60] = 0.584; // less ; B 94 11 641 495 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[61] = 0.584; // equal ; B 63 115 628 390 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[62] = 0.584; // greater ; B 50 11 597 495 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[63] = 0.556; // question ; B 161 0 610 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[64] = 1.015; // at ; B 215 -19 965 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[65] = 0.667; // A ; B 14 0 654 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[66] = 0.667; // B ; B 74 0 712 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[67] = 0.722; // C ; B 108 -19 782 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[68] = 0.722; // D ; B 81 0 764 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[69] = 0.667; // E ; B 86 0 762 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[70] = 0.611; // F ; B 86 0 736 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[71] = 0.778; // G ; B 111 -19 799 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[72] = 0.722; // H ; B 77 0 799 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[73] = 0.278; // I ; B 91 0 341 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[74] = 0.500; // J ; B 47 -19 581 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[75] = 0.667; // K ; B 76 0 808 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[76] = 0.556; // L ; B 76 0 555 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[77] = 0.833; // M ; B 73 0 914 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[78] = 0.722; // N ; B 76 0 799 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[79] = 0.778; // O ; B 105 -19 826 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[80] = 0.667; // P ; B 86 0 737 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[81] = 0.778; // Q ; B 105 -56 826 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[82] = 0.722; // R ; B 88 0 773 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[83] = 0.667; // S ; B 90 -19 713 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[84] = 0.611; // T ; B 148 0 750 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[85] = 0.722; // U ; B 123 -19 797 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[86] = 0.667; // V ; B 173 0 800 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[87] = 0.944; // W ; B 169 0 1081 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[88] = 0.667; // X ; B 19 0 790 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[89] = 0.667; // Y ; B 167 0 806 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[90] = 0.611; // Z ; B 23 0 741 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[91] = 0.278; // bracketleft ; B 21 -196 403 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[92] = 0.278; // backslash ; B 140 -19 291 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[93] = 0.278; // bracketright ; B -14 -196 368 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[94] = 0.469; // asciicircum ; B 42 264 539 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[95] = 0.556; // underscore ; B -27 -125 540 -75 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[96] = 0.222; // quoteleft ; B 165 470 323 725 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[97] = 0.556; // a ; B 61 -15 559 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[98] = 0.556; // b ; B 58 -15 584 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[99] = 0.500; // c ; B 74 -15 553 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[100] = 0.556; // d ; B 84 -15 652 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[101] = 0.556; // e ; B 84 -15 578 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[102] = 0.278; // f ; B 86 0 416 728 ; L i fi ; L l fl ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[103] = 0.556; // g ; B 42 -220 610 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[104] = 0.556; // h ; B 65 0 573 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[105] = 0.222; // i ; B 67 0 308 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[106] = 0.222; // j ; B -60 -210 308 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[107] = 0.500; // k ; B 67 0 600 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[108] = 0.222; // l ; B 67 0 308 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[109] = 0.833; // m ; B 65 0 852 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[110] = 0.556; // n ; B 65 0 573 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[111] = 0.556; // o ; B 83 -14 585 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[112] = 0.556; // p ; B 14 -207 584 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[113] = 0.556; // q ; B 84 -207 605 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[114] = 0.333; // r ; B 77 0 446 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[115] = 0.500; // s ; B 63 -15 529 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[116] = 0.278; // t ; B 102 -7 368 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[117] = 0.556; // u ; B 94 -15 600 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[118] = 0.500; // v ; B 119 0 603 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[119] = 0.722; // w ; B 125 0 820 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[120] = 0.500; // x ; B 11 0 594 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[121] = 0.500; // y ; B 15 -214 600 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[122] = 0.500; // z ; B 31 0 571 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[123] = 0.334; // braceleft ; B 92 -196 445 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[124] = 0.260; // bar ; B 46 -225 332 775 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[125] = 0.334; // braceright ; B 0 -196 354 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[126] = 0.584; // asciitilde ; B 111 180 580 326 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[161] = 0.333; // exclamdown ; B 77 -195 326 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[162] = 0.556; // cent ; B 95 -115 584 623 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[163] = 0.556; // sterling ; B 49 -16 634 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[164] = 0.167; // fraction ; B -170 -19 482 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[165] = 0.556; // yen ; B 81 0 699 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[166] = 0.556; // florin ; B -52 -207 654 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[167] = 0.556; // section ; B 76 -191 584 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[168] = 0.556; // currency ; B 60 99 646 603 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[169] = 0.191; // quotesingle ; B 157 463 285 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[170] = 0.333; // quotedblleft ; B 138 470 461 725 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[171] = 0.556; // guillemotleft ; B 146 108 554 446 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[172] = 0.333; // guilsinglleft ; B 137 108 340 446 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[173] = 0.333; // guilsinglright ; B 111 108 314 446 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[174] = 0.500; // fi ; B 86 0 587 728 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[175] = 0.500; // fl ; B 86 0 585 728 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[177] = 0.556; // endash ; B 51 240 623 313 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[178] = 0.556; // dagger ; B 135 -159 622 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[179] = 0.556; // daggerdbl ; B 52 -159 623 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[180] = 0.278; // periodcentered ; B 129 190 257 315 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[182] = 0.537; // paragraph ; B 126 -173 650 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[183] = 0.350; // bullet ; B 91 202 413 517 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[184] = 0.222; // quotesinglbase ; B 21 -149 180 106 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[185] = 0.333; // quotedblbase ; B -6 -149 318 106 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[186] = 0.333; // quotedblright ; B 124 463 448 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[187] = 0.556; // guillemotright ; B 120 108 528 446 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[188] = 1.000; // ellipsis ; B 115 0 908 106 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[189] = 1.000; // perthousand ; B 88 -19 1029 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[191] = 0.611; // questiondown ; B 85 -201 534 525 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[193] = 0.333; // grave ; B 170 593 337 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[194] = 0.333; // acute ; B 248 593 475 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[195] = 0.333; // circumflex ; B 147 593 438 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[196] = 0.333; // tilde ; B 125 606 490 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[197] = 0.333; // macron ; B 143 627 468 684 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[198] = 0.333; // breve ; B 167 595 476 731 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[199] = 0.333; // dotaccent ; B 249 604 362 706 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[200] = 0.333; // dieresis ; B 168 604 443 706 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[202] = 0.333; // ring ; B 214 572 402 756 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[203] = 0.333; // cedilla ; B 2 -225 232 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[205] = 0.333; // hungarumlaut ; B 157 593 565 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[206] = 0.333; // ogonek ; B 43 -225 249 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[207] = 0.333; // caron ; B 177 593 468 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[208] = 1.000; // emdash ; B 51 240 1067 313 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[225] = 1.000; // AE ; B 8 0 1097 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[227] = 0.370; // ordfeminine ; B 127 405 449 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[232] = 0.556; // Lslash ; B 41 0 555 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[233] = 0.778; // Oslash ; B 43 -19 890 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[234] = 1.000; // OE ; B 98 -19 1116 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[235] = 0.365; // ordmasculine ; B 141 405 468 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[241] = 0.889; // ae ; B 61 -15 909 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[245] = 0.278; // dotlessi ; B 95 0 294 523 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[248] = 0.222; // lslash ; B 41 0 347 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[249] = 0.611; // oslash ; B 29 -22 647 545 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[250] = 0.944; // oe ; B 83 -15 964 538 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[251] = 0.611; // germandbls ; B 67 -15 658 728 ;
}

void set_Symbol(Params *pParams)
{
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[32] = 0.250; // space ; B 0 0 0 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[33] = 0.333; // exclam ; B 128 -17 240 672 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[34] = 0.713; // universal ; B 31 0 681 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[35] = 0.500; // numbersign ; B 20 -16 481 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[36] = 0.549; // existential ; B 25 0 478 707 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[37] = 0.833; // percent ; B 63 -36 771 655 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[38] = 0.778; // ampersand ; B 41 -18 750 661 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[39] = 0.439; // suchthat ; B 48 -17 414 500 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[40] = 0.333; // parenleft ; B 53 -191 300 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[41] = 0.333; // parenright ; B 30 -191 277 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[42] = 0.500; // asteriskmath ; B 65 134 427 551 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[43] = 0.549; // plus ; B 10 0 539 533 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[44] = 0.250; // comma ; B 56 -152 194 104 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[45] = 0.549; // minus ; B 11 233 535 288 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[46] = 0.250; // period ; B 69 -17 181 95 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[47] = 0.278; // slash ; B 0 -18 254 646 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[48] = 0.500; // zero ; B 24 -14 476 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[49] = 0.500; // one ; B 117 0 390 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[50] = 0.500; // two ; B 25 0 475 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[51] = 0.500; // three ; B 43 -14 435 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[52] = 0.500; // four ; B 15 0 469 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[53] = 0.500; // five ; B 32 -14 445 690 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[54] = 0.500; // six ; B 34 -14 468 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[55] = 0.500; // seven ; B 24 -16 448 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[56] = 0.500; // eight ; B 56 -14 445 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[57] = 0.500; // nine ; B 30 -18 459 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[58] = 0.278; // colon ; B 81 -17 193 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[59] = 0.278; // semicolon ; B 83 -152 221 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[60] = 0.549; // less ; B 26 0 523 522 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[61] = 0.549; // equal ; B 11 141 537 390 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[62] = 0.549; // greater ; B 26 0 523 522 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[63] = 0.444; // question ; B 70 -17 412 686 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[64] = 0.549; // congruent ; B 11 0 537 475 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[65] = 0.722; // Alpha ; B 4 0 684 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[66] = 0.667; // Beta ; B 29 0 592 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[67] = 0.722; // Chi ; B -9 0 704 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[68] = 0.612; // Delta ; B 6 0 608 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[69] = 0.611; // Epsilon ; B 32 0 617 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[70] = 0.763; // Phi ; B 26 0 741 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[71] = 0.603; // Gamma ; B 24 0 609 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[72] = 0.722; // Eta ; B 39 0 729 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[73] = 0.333; // Iota ; B 32 0 316 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[74] = 0.631; // theta1 ; B 18 -18 623 689 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[75] = 0.722; // Kappa ; B 35 0 722 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[76] = 0.686; // Lambda ; B 6 0 680 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[77] = 0.889; // Mu ; B 28 0 887 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[78] = 0.722; // Nu ; B 29 -8 720 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[79] = 0.722; // Omicron ; B 41 -17 715 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[80] = 0.768; // Pi ; B 25 0 745 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[81] = 0.741; // Theta ; B 41 -17 715 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[82] = 0.556; // Rho ; B 28 0 563 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[83] = 0.592; // Sigma ; B 5 0 589 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[84] = 0.611; // Tau ; B 33 0 607 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[85] = 0.690; // Upsilon ; B -8 0 694 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[86] = 0.439; // sigma1 ; B 40 -233 436 500 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[87] = 0.768; // Omega ; B 34 0 736 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[88] = 0.645; // Xi ; B 40 0 599 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[89] = 0.795; // Psi ; B 15 0 781 684 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[90] = 0.611; // Zeta ; B 44 0 636 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[91] = 0.333; // bracketleft ; B 86 -155 299 674 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[92] = 0.863; // therefore ; B 163 0 701 487 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[93] = 0.333; // bracketright ; B 33 -155 246 674 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[94] = 0.658; // perpendicular ; B 15 0 652 674 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[95] = 0.500; // underscore ; B -2 -125 502 -75 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[96] = 0.500; // radicalex ; B 480 881 1090 917 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[97] = 0.631; // alpha ; B 41 -18 622 500 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[98] = 0.549; // beta ; B 61 -223 515 741 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[99] = 0.549; // chi ; B 12 -231 522 499 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[100] = 0.494; // delta ; B 40 -19 481 740 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[101] = 0.439; // epsilon ; B 22 -19 427 502 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[102] = 0.521; // phi ; B 28 -224 492 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[103] = 0.411; // gamma ; B 5 -225 484 499 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[104] = 0.603; // eta ; B 0 -202 527 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[105] = 0.329; // iota ; B 0 -17 301 503 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[106] = 0.603; // phi1 ; B 36 -224 587 499 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[107] = 0.549; // kappa ; B 33 0 558 501 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[108] = 0.549; // lambda ; B 24 -17 548 739 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[109] = 0.576; // mu ; B 33 -223 567 500 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[110] = 0.521; // nu ; B -9 -16 475 507 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[111] = 0.549; // omicron ; B 35 -19 501 499 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[112] = 0.549; // pi ; B 10 -19 530 487 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[113] = 0.521; // theta ; B 43 -17 485 690 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[114] = 0.549; // rho ; B 50 -230 490 499 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[115] = 0.603; // sigma ; B 30 -21 588 500 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[116] = 0.439; // tau ; B 10 -19 418 500 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[117] = 0.576; // upsilon ; B 7 -18 535 507 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[118] = 0.713; // omega1 ; B 12 -18 671 583 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[119] = 0.686; // omega ; B 42 -17 684 500 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[120] = 0.493; // xi ; B 27 -224 469 766 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[121] = 0.686; // psi ; B 12 -228 701 500 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[122] = 0.494; // zeta ; B 60 -225 467 756 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[123] = 0.480; // braceleft ; B 58 -183 397 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[124] = 0.200; // bar ; B 65 -293 135 707 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[125] = 0.480; // braceright ; B 79 -183 418 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[126] = 0.549; // similar ; B 17 203 529 307 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[160] = 0.750; // Euro ; B 20 -12 714 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[161] = 0.620; // Upsilon1 ; B -2 0 610 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[162] = 0.247; // minute ; B 27 459 228 735 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[163] = 0.549; // lessequal ; B 29 0 526 639 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[164] = 0.167; // fraction ; B -180 -12 340 677 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[165] = 0.713; // infinity ; B 26 124 688 404 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[166] = 0.500; // florin ; B 2 -193 494 686 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[167] = 0.753; // club ; B 86 -26 660 533 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[168] = 0.753; // diamond ; B 142 -36 600 550 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[169] = 0.753; // heart ; B 117 -33 631 532 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[170] = 0.753; // spade ; B 113 -36 629 548 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[171] = 1.042; // arrowboth ; B 24 -15 1024 511 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[172] = 0.987; // arrowleft ; B 32 -15 942 511 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[173] = 0.603; // arrowup ; B 45 0 571 910 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[174] = 0.987; // arrowright ; B 49 -15 959 511 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[175] = 0.603; // arrowdown ; B 45 -22 571 888 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[176] = 0.400; // degree ; B 50 385 350 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[177] = 0.549; // plusminus ; B 10 0 539 645 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[178] = 0.411; // second ; B 20 459 413 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[179] = 0.549; // greaterequal ; B 29 0 526 639 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[180] = 0.549; // multiply ; B 17 8 533 524 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[181] = 0.713; // proportional ; B 27 123 639 404 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[182] = 0.494; // partialdiff ; B 26 -20 462 746 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[183] = 0.460; // bullet ; B 50 113 410 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[184] = 0.549; // divide ; B 10 71 536 456 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[185] = 0.549; // notequal ; B 15 -25 540 549 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[186] = 0.549; // equivalence ; B 14 82 538 443 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[187] = 0.549; // approxequal ; B 14 135 527 394 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[188] = 1.000; // ellipsis ; B 111 -17 889 95 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[189] = 0.603; // arrowvertex ; B 280 -120 336 1010 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[190] = 1.000; // arrowhorizex ; B -60 220 1050 276 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[191] = 0.658; // carriagereturn ; B 15 -16 602 629 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[192] = 0.823; // aleph ; B 175 -18 661 658 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[193] = 0.686; // Ifraktur ; B 10 -53 578 740 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[194] = 0.795; // Rfraktur ; B 26 -15 759 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[195] = 0.987; // weierstrass ; B 159 -211 870 573 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[196] = 0.768; // circlemultiply ; B 43 -17 733 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[197] = 0.768; // circleplus ; B 43 -15 733 675 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[198] = 0.823; // emptyset ; B 39 -24 781 719 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[199] = 0.768; // intersection ; B 40 0 732 509 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[200] = 0.768; // union ; B 40 -17 732 492 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[201] = 0.713; // propersuperset ; B 20 0 673 470 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[202] = 0.713; // reflexsuperset ; B 20 -125 673 470 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[203] = 0.713; // notsubset ; B 36 -70 690 540 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[204] = 0.713; // propersubset ; B 37 0 690 470 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[205] = 0.713; // reflexsubset ; B 37 -125 690 470 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[206] = 0.713; // element ; B 45 0 505 468 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[207] = 0.713; // notelement ; B 45 -58 505 555 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[208] = 0.768; // angle ; B 26 0 738 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[209] = 0.713; // gradient ; B 36 -19 681 718 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[210] = 0.790; // registerserif ; B 50 -17 740 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[211] = 0.790; // copyrightserif ; B 51 -15 741 675 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[212] = 0.890; // trademarkserif ; B 18 293 855 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[213] = 0.823; // product ; B 25 -101 803 751 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[214] = 0.549; // radical ; B 10 -38 515 917 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[215] = 0.250; // dotmath ; B 69 210 169 310 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[216] = 0.713; // logicalnot ; B 15 0 680 288 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[217] = 0.603; // logicaland ; B 23 0 583 454 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[218] = 0.603; // logicalor ; B 30 0 578 477 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[219] = 1.042; // arrowdblboth ; B 27 -20 1023 510 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[220] = 0.987; // arrowdblleft ; B 30 -15 939 513 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[221] = 0.603; // arrowdblup ; B 39 2 567 911 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[222] = 0.987; // arrowdblright ; B 45 -20 954 508 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[223] = 0.603; // arrowdbldown ; B 44 -19 572 890 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[224] = 0.494; // lozenge ; B 18 0 466 745 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[225] = 0.329; // angleleft ; B 25 -198 306 746 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[226] = 0.790; // registersans ; B 50 -20 740 670 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[227] = 0.790; // copyrightsans ; B 49 -15 739 675 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[228] = 0.786; // trademarksans ; B 5 293 725 673 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[229] = 0.713; // summation ; B 14 -108 695 752 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[230] = 0.384; // parenlefttp ; B 24 -293 436 926 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[231] = 0.384; // parenleftex ; B 24 -85 108 925 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[232] = 0.384; // parenleftbt ; B 24 -293 436 926 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[233] = 0.384; // bracketlefttp ; B 0 -80 349 926 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[234] = 0.384; // bracketleftex ; B 0 -79 77 925 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[235] = 0.384; // bracketleftbt ; B 0 -80 349 926 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[236] = 0.494; // bracelefttp ; B 209 -85 445 925 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[237] = 0.494; // braceleftmid ; B 20 -85 284 935 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[238] = 0.494; // braceleftbt ; B 209 -75 445 935 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[239] = 0.494; // braceex ; B 209 -85 284 935 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[241] = 0.329; // angleright ; B 21 -198 302 746 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[242] = 0.274; // integral ; B 2 -107 291 916 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[243] = 0.686; // integraltp ; B 308 -88 675 920 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[244] = 0.686; // integralex ; B 308 -88 378 975 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[245] = 0.686; // integralbt ; B 11 -87 378 921 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[246] = 0.384; // parenrighttp ; B 54 -293 466 926 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[247] = 0.384; // parenrightex ; B 382 -85 466 925 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[248] = 0.384; // parenrightbt ; B 54 -293 466 926 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[249] = 0.384; // bracketrighttp ; B 22 -80 371 926 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[250] = 0.384; // bracketrightex ; B 294 -79 371 925 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[251] = 0.384; // bracketrightbt ; B 22 -80 371 926 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[252] = 0.494; // bracerighttp ; B 48 -85 284 925 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[253] = 0.494; // bracerightmid ; B 209 -85 473 935 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[254] = 0.494; // bracerightbt ; B 48 -75 284 935 ;
}

void set_Times_Bold(Params *pParams)
{
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[32] = 0.250; // space ; B 0 0 0 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[33] = 0.333; // exclam ; B 81 -13 251 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[34] = 0.555; // quotedbl ; B 83 404 472 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[35] = 0.500; // numbersign ; B 4 0 496 700 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[36] = 0.500; // dollar ; B 29 -99 472 750 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[37] = 1.000; // percent ; B 124 -14 877 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[38] = 0.833; // ampersand ; B 62 -16 787 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[39] = 0.333; // quoteright ; B 79 356 263 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[40] = 0.333; // parenleft ; B 46 -168 306 694 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[41] = 0.333; // parenright ; B 27 -168 287 694 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[42] = 0.500; // asterisk ; B 56 255 447 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[43] = 0.570; // plus ; B 33 0 537 506 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[44] = 0.250; // comma ; B 39 -180 223 155 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[45] = 0.333; // hyphen ; B 44 171 287 287 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[46] = 0.250; // period ; B 41 -13 210 156 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[47] = 0.278; // slash ; B -24 -19 302 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[48] = 0.500; // zero ; B 24 -13 476 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[49] = 0.500; // one ; B 65 0 442 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[50] = 0.500; // two ; B 17 0 478 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[51] = 0.500; // three ; B 16 -14 468 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[52] = 0.500; // four ; B 19 0 475 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[53] = 0.500; // five ; B 22 -8 470 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[54] = 0.500; // six ; B 28 -13 475 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[55] = 0.500; // seven ; B 17 0 477 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[56] = 0.500; // eight ; B 28 -13 472 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[57] = 0.500; // nine ; B 26 -13 473 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[58] = 0.333; // colon ; B 82 -13 251 472 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[59] = 0.333; // semicolon ; B 82 -180 266 472 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[60] = 0.570; // less ; B 31 -8 539 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[61] = 0.570; // equal ; B 33 107 537 399 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[62] = 0.570; // greater ; B 31 -8 539 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[63] = 0.500; // question ; B 57 -13 445 689 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[64] = 0.930; // at ; B 108 -19 822 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[65] = 0.722; // A ; B 9 0 689 690 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[66] = 0.667; // B ; B 16 0 619 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[67] = 0.722; // C ; B 49 -19 687 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[68] = 0.722; // D ; B 14 0 690 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[69] = 0.667; // E ; B 16 0 641 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[70] = 0.611; // F ; B 16 0 583 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[71] = 0.778; // G ; B 37 -19 755 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[72] = 0.778; // H ; B 21 0 759 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[73] = 0.389; // I ; B 20 0 370 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[74] = 0.500; // J ; B 3 -96 479 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[75] = 0.778; // K ; B 30 0 769 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[76] = 0.667; // L ; B 19 0 638 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[77] = 0.944; // M ; B 14 0 921 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[78] = 0.722; // N ; B 16 -18 701 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[79] = 0.778; // O ; B 35 -19 743 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[80] = 0.611; // P ; B 16 0 600 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[81] = 0.778; // Q ; B 35 -176 743 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[82] = 0.722; // R ; B 26 0 715 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[83] = 0.556; // S ; B 35 -19 513 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[84] = 0.667; // T ; B 31 0 636 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[85] = 0.722; // U ; B 16 -19 701 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[86] = 0.722; // V ; B 16 -18 701 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[87] = 1.000; // W ; B 19 -15 981 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[88] = 0.722; // X ; B 16 0 699 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[89] = 0.722; // Y ; B 15 0 699 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[90] = 0.667; // Z ; B 28 0 634 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[91] = 0.333; // bracketleft ; B 67 -149 301 678 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[92] = 0.278; // backslash ; B -25 -19 303 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[93] = 0.333; // bracketright ; B 32 -149 266 678 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[94] = 0.581; // asciicircum ; B 73 311 509 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[95] = 0.500; // underscore ; B 0 -125 500 -75 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[96] = 0.333; // quoteleft ; B 70 356 254 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[97] = 0.500; // a ; B 25 -14 488 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[98] = 0.556; // b ; B 17 -14 521 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[99] = 0.444; // c ; B 25 -14 430 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[100] = 0.556; // d ; B 25 -14 534 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[101] = 0.444; // e ; B 25 -14 426 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[102] = 0.333; // f ; B 14 0 389 691 ; L i fi ; L l fl ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[103] = 0.500; // g ; B 28 -206 483 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[104] = 0.556; // h ; B 16 0 534 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[105] = 0.278; // i ; B 16 0 255 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[106] = 0.333; // j ; B -57 -203 263 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[107] = 0.556; // k ; B 22 0 543 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[108] = 0.278; // l ; B 16 0 255 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[109] = 0.833; // m ; B 16 0 814 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[110] = 0.556; // n ; B 21 0 539 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[111] = 0.500; // o ; B 25 -14 476 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[112] = 0.556; // p ; B 19 -205 524 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[113] = 0.556; // q ; B 34 -205 536 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[114] = 0.444; // r ; B 29 0 434 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[115] = 0.389; // s ; B 25 -14 361 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[116] = 0.333; // t ; B 20 -12 332 630 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[117] = 0.556; // u ; B 16 -14 537 461 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[118] = 0.500; // v ; B 21 -14 485 461 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[119] = 0.722; // w ; B 23 -14 707 461 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[120] = 0.500; // x ; B 12 0 484 461 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[121] = 0.500; // y ; B 16 -205 480 461 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[122] = 0.444; // z ; B 21 0 420 461 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[123] = 0.394; // braceleft ; B 22 -175 340 698 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[124] = 0.220; // bar ; B 66 -218 154 782 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[125] = 0.394; // braceright ; B 54 -175 372 698 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[126] = 0.520; // asciitilde ; B 29 173 491 333 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[161] = 0.333; // exclamdown ; B 82 -203 252 501 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[162] = 0.500; // cent ; B 53 -140 458 588 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[163] = 0.500; // sterling ; B 21 -14 477 684 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[164] = 0.167; // fraction ; B -168 -12 329 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[165] = 0.500; // yen ; B -64 0 547 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[166] = 0.500; // florin ; B 0 -155 498 706 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[167] = 0.500; // section ; B 57 -132 443 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[168] = 0.500; // currency ; B -26 61 526 613 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[169] = 0.278; // quotesingle ; B 75 404 204 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[170] = 0.500; // quotedblleft ; B 32 356 486 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[171] = 0.500; // guillemotleft ; B 23 36 473 415 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[172] = 0.333; // guilsinglleft ; B 51 36 305 415 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[173] = 0.333; // guilsinglright ; B 28 36 282 415 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[174] = 0.556; // fi ; B 14 0 536 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[175] = 0.556; // fl ; B 14 0 536 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[177] = 0.500; // endash ; B 0 181 500 271 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[178] = 0.500; // dagger ; B 47 -134 453 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[179] = 0.500; // daggerdbl ; B 45 -132 456 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[180] = 0.250; // periodcentered ; B 41 248 210 417 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[182] = 0.540; // paragraph ; B 0 -186 519 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[183] = 0.350; // bullet ; B 35 198 315 478 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[184] = 0.333; // quotesinglbase ; B 79 -180 263 155 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[185] = 0.500; // quotedblbase ; B 14 -180 468 155 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[186] = 0.500; // quotedblright ; B 14 356 468 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[187] = 0.500; // guillemotright ; B 27 36 477 415 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[188] = 1.000; // ellipsis ; B 82 -13 917 156 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[189] = 1.000; // perthousand ; B 7 -29 995 706 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[191] = 0.500; // questiondown ; B 55 -201 443 501 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[193] = 0.333; // grave ; B 8 528 246 713 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[194] = 0.333; // acute ; B 86 528 324 713 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[195] = 0.333; // circumflex ; B -2 528 335 704 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[196] = 0.333; // tilde ; B -16 547 349 674 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[197] = 0.333; // macron ; B 1 565 331 637 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[198] = 0.333; // breve ; B 15 528 318 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[199] = 0.333; // dotaccent ; B 103 536 258 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[200] = 0.333; // dieresis ; B -2 537 335 667 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[202] = 0.333; // ring ; B 60 527 273 740 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[203] = 0.333; // cedilla ; B 68 -218 294 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[205] = 0.333; // hungarumlaut ; B -13 528 425 713 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[206] = 0.333; // ogonek ; B 90 -193 319 24 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[207] = 0.333; // caron ; B -2 528 335 704 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[208] = 1.000; // emdash ; B 0 181 1000 271 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[225] = 1.000; // AE ; B 4 0 951 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[227] = 0.300; // ordfeminine ; B -1 397 301 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[232] = 0.667; // Lslash ; B 19 0 638 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[233] = 0.778; // Oslash ; B 35 -74 743 737 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[234] = 1.000; // OE ; B 22 -5 981 684 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[235] = 0.330; // ordmasculine ; B 18 397 312 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[241] = 0.722; // ae ; B 33 -14 693 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[245] = 0.278; // dotlessi ; B 16 0 255 461 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[248] = 0.278; // lslash ; B -22 0 303 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[249] = 0.500; // oslash ; B 25 -92 476 549 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[250] = 0.722; // oe ; B 22 -14 696 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[251] = 0.556; // germandbls ; B 19 -12 517 691 ;
}

void set_Times_BoldItalic(Params *pParams)
{
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[32] = 0.250; // space ; B 0 0 0 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[33] = 0.389; // exclam ; B 67 -13 370 684 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[34] = 0.555; // quotedbl ; B 136 398 536 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[35] = 0.500; // numbersign ; B -33 0 533 700 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[36] = 0.500; // dollar ; B -20 -100 497 733 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[37] = 0.833; // percent ; B 39 -10 793 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[38] = 0.778; // ampersand ; B 5 -19 699 682 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[39] = 0.333; // quoteright ; B 98 369 302 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[40] = 0.333; // parenleft ; B 28 -179 344 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[41] = 0.333; // parenright ; B -44 -179 271 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[42] = 0.500; // asterisk ; B 65 249 456 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[43] = 0.570; // plus ; B 33 0 537 506 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[44] = 0.250; // comma ; B -60 -182 144 134 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[45] = 0.333; // hyphen ; B 2 166 271 282 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[46] = 0.250; // period ; B -9 -13 139 135 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[47] = 0.278; // slash ; B -64 -18 342 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[48] = 0.500; // zero ; B 17 -14 477 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[49] = 0.500; // one ; B 5 0 419 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[50] = 0.500; // two ; B -27 0 446 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[51] = 0.500; // three ; B -15 -13 450 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[52] = 0.500; // four ; B -15 0 503 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[53] = 0.500; // five ; B -11 -13 487 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[54] = 0.500; // six ; B 23 -15 509 679 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[55] = 0.500; // seven ; B 52 0 525 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[56] = 0.500; // eight ; B 3 -13 476 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[57] = 0.500; // nine ; B -12 -10 475 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[58] = 0.333; // colon ; B 23 -13 264 459 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[59] = 0.333; // semicolon ; B -25 -183 264 459 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[60] = 0.570; // less ; B 31 -8 539 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[61] = 0.570; // equal ; B 33 107 537 399 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[62] = 0.570; // greater ; B 31 -8 539 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[63] = 0.500; // question ; B 79 -13 470 684 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[64] = 0.832; // at ; B 63 -18 770 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[65] = 0.667; // A ; B -67 0 593 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[66] = 0.667; // B ; B -24 0 624 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[67] = 0.667; // C ; B 32 -18 677 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[68] = 0.722; // D ; B -46 0 685 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[69] = 0.667; // E ; B -27 0 653 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[70] = 0.667; // F ; B -13 0 660 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[71] = 0.722; // G ; B 21 -18 706 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[72] = 0.778; // H ; B -24 0 799 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[73] = 0.389; // I ; B -32 0 406 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[74] = 0.500; // J ; B -46 -99 524 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[75] = 0.667; // K ; B -21 0 702 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[76] = 0.611; // L ; B -22 0 590 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[77] = 0.889; // M ; B -29 -12 917 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[78] = 0.722; // N ; B -27 -15 748 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[79] = 0.722; // O ; B 27 -18 691 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[80] = 0.611; // P ; B -27 0 613 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[81] = 0.722; // Q ; B 27 -208 691 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[82] = 0.667; // R ; B -29 0 623 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[83] = 0.556; // S ; B 2 -18 526 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[84] = 0.611; // T ; B 50 0 650 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[85] = 0.722; // U ; B 67 -18 744 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[86] = 0.667; // V ; B 65 -18 715 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[87] = 0.889; // W ; B 65 -18 940 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[88] = 0.667; // X ; B -24 0 694 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[89] = 0.611; // Y ; B 73 0 659 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[90] = 0.611; // Z ; B -11 0 590 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[91] = 0.333; // bracketleft ; B -37 -159 362 674 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[92] = 0.278; // backslash ; B -1 -18 279 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[93] = 0.333; // bracketright ; B -56 -157 343 674 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[94] = 0.570; // asciicircum ; B 67 304 503 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[95] = 0.500; // underscore ; B 0 -125 500 -75 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[96] = 0.333; // quoteleft ; B 128 369 332 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[97] = 0.500; // a ; B -21 -14 455 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[98] = 0.500; // b ; B -14 -13 444 699 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[99] = 0.444; // c ; B -5 -13 392 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[100] = 0.500; // d ; B -21 -13 517 699 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[101] = 0.444; // e ; B 5 -13 398 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[102] = 0.333; // f ; B -169 -205 446 698 ; L i fi ; L l fl ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[103] = 0.500; // g ; B -52 -203 478 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[104] = 0.556; // h ; B -13 -9 498 699 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[105] = 0.278; // i ; B 2 -9 263 684 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[106] = 0.278; // j ; B -189 -207 279 684 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[107] = 0.500; // k ; B -23 -8 483 699 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[108] = 0.278; // l ; B 2 -9 290 699 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[109] = 0.778; // m ; B -14 -9 722 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[110] = 0.556; // n ; B -6 -9 493 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[111] = 0.500; // o ; B -3 -13 441 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[112] = 0.500; // p ; B -120 -205 446 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[113] = 0.500; // q ; B 1 -205 471 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[114] = 0.389; // r ; B -21 0 389 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[115] = 0.389; // s ; B -19 -13 333 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[116] = 0.278; // t ; B -11 -9 281 594 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[117] = 0.556; // u ; B 15 -9 492 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[118] = 0.444; // v ; B 16 -13 401 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[119] = 0.667; // w ; B 16 -13 614 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[120] = 0.500; // x ; B -46 -13 469 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[121] = 0.444; // y ; B -94 -205 392 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[122] = 0.389; // z ; B -43 -78 368 449 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[123] = 0.348; // braceleft ; B 5 -187 436 686 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[124] = 0.220; // bar ; B 66 -218 154 782 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[125] = 0.348; // braceright ; B -129 -187 302 686 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[126] = 0.570; // asciitilde ; B 54 173 516 333 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[161] = 0.389; // exclamdown ; B 19 -205 322 492 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[162] = 0.500; // cent ; B 42 -143 439 576 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[163] = 0.500; // sterling ; B -32 -12 510 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[164] = 0.167; // fraction ; B -169 -14 324 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[165] = 0.500; // yen ; B 33 0 628 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[166] = 0.500; // florin ; B -87 -156 537 707 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[167] = 0.500; // section ; B 36 -143 459 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[168] = 0.500; // currency ; B -26 34 526 586 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[169] = 0.278; // quotesingle ; B 128 398 268 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[170] = 0.500; // quotedblleft ; B 53 369 513 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[171] = 0.500; // guillemotleft ; B 12 32 468 415 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[172] = 0.333; // guilsinglleft ; B 32 32 303 415 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[173] = 0.333; // guilsinglright ; B 10 32 281 415 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[174] = 0.556; // fi ; B -188 -205 514 703 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[175] = 0.556; // fl ; B -186 -205 553 704 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[177] = 0.500; // endash ; B -40 178 477 269 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[178] = 0.500; // dagger ; B 91 -145 494 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[179] = 0.500; // daggerdbl ; B 10 -139 493 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[180] = 0.250; // periodcentered ; B 51 257 199 405 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[182] = 0.500; // paragraph ; B -57 -193 562 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[183] = 0.350; // bullet ; B 0 175 350 525 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[184] = 0.333; // quotesinglbase ; B -5 -182 199 134 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[185] = 0.500; // quotedblbase ; B -57 -182 403 134 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[186] = 0.500; // quotedblright ; B 53 369 513 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[187] = 0.500; // guillemotright ; B 12 32 468 415 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[188] = 1.000; // ellipsis ; B 40 -13 852 135 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[189] = 1.000; // perthousand ; B 7 -29 996 706 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[191] = 0.500; // questiondown ; B 30 -205 421 492 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[193] = 0.333; // grave ; B 85 516 297 697 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[194] = 0.333; // acute ; B 139 516 379 697 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[195] = 0.333; // circumflex ; B 40 516 367 690 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[196] = 0.333; // tilde ; B 48 536 407 655 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[197] = 0.333; // macron ; B 51 553 393 623 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[198] = 0.333; // breve ; B 71 516 387 678 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[199] = 0.333; // dotaccent ; B 163 550 298 684 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[200] = 0.333; // dieresis ; B 55 550 402 684 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[202] = 0.333; // ring ; B 127 516 340 729 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[203] = 0.333; // cedilla ; B -80 -218 156 5 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[205] = 0.333; // hungarumlaut ; B 69 516 498 697 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[206] = 0.333; // ogonek ; B 15 -183 244 34 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[207] = 0.333; // caron ; B 79 516 411 690 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[208] = 1.000; // emdash ; B -40 178 977 269 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[225] = 0.944; // AE ; B -64 0 918 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[227] = 0.266; // ordfeminine ; B 16 399 330 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[232] = 0.611; // Lslash ; B -22 0 590 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[233] = 0.722; // Oslash ; B 27 -125 691 764 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[234] = 0.944; // OE ; B 23 -8 946 677 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[235] = 0.300; // ordmasculine ; B 56 400 347 685 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[241] = 0.722; // ae ; B -5 -13 673 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[245] = 0.278; // dotlessi ; B 2 -9 238 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[248] = 0.278; // lslash ; B -7 -9 307 699 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[249] = 0.500; // oslash ; B -3 -119 441 560 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[250] = 0.722; // oe ; B 6 -13 674 462 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[251] = 0.500; // germandbls ; B -200 -200 473 705 ;
}

void set_Times_Italic(Params *pParams)
{
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[32] = 0.250; // space ; B 0 0 0 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[33] = 0.333; // exclam ; B 39 -11 302 667 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[34] = 0.420; // quotedbl ; B 144 421 432 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[35] = 0.500; // numbersign ; B 2 0 540 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[36] = 0.500; // dollar ; B 31 -89 497 731 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[37] = 0.833; // percent ; B 79 -13 790 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[38] = 0.778; // ampersand ; B 76 -18 723 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[39] = 0.333; // quoteright ; B 151 436 290 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[40] = 0.333; // parenleft ; B 42 -181 315 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[41] = 0.333; // parenright ; B 16 -180 289 669 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[42] = 0.500; // asterisk ; B 128 255 492 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[43] = 0.675; // plus ; B 86 0 590 506 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[44] = 0.250; // comma ; B -4 -129 135 101 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[45] = 0.333; // hyphen ; B 49 192 282 255 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[46] = 0.250; // period ; B 27 -11 138 100 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[47] = 0.278; // slash ; B -65 -18 386 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[48] = 0.500; // zero ; B 32 -7 497 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[49] = 0.500; // one ; B 49 0 409 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[50] = 0.500; // two ; B 12 0 452 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[51] = 0.500; // three ; B 15 -7 465 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[52] = 0.500; // four ; B 1 0 479 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[53] = 0.500; // five ; B 15 -7 491 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[54] = 0.500; // six ; B 30 -7 521 686 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[55] = 0.500; // seven ; B 75 -8 537 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[56] = 0.500; // eight ; B 30 -7 493 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[57] = 0.500; // nine ; B 23 -17 492 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[58] = 0.333; // colon ; B 50 -11 261 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[59] = 0.333; // semicolon ; B 27 -129 261 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[60] = 0.675; // less ; B 84 -8 592 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[61] = 0.675; // equal ; B 86 120 590 386 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[62] = 0.675; // greater ; B 84 -8 592 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[63] = 0.500; // question ; B 132 -12 472 664 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[64] = 0.920; // at ; B 118 -18 806 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[65] = 0.611; // A ; B -51 0 564 668 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[66] = 0.611; // B ; B -8 0 588 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[67] = 0.667; // C ; B 66 -18 689 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[68] = 0.722; // D ; B -8 0 700 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[69] = 0.611; // E ; B -1 0 634 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[70] = 0.611; // F ; B 8 0 645 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[71] = 0.722; // G ; B 52 -18 722 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[72] = 0.722; // H ; B -8 0 767 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[73] = 0.333; // I ; B -8 0 384 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[74] = 0.444; // J ; B -6 -18 491 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[75] = 0.667; // K ; B 7 0 722 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[76] = 0.556; // L ; B -8 0 559 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[77] = 0.833; // M ; B -18 0 873 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[78] = 0.667; // N ; B -20 -15 727 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[79] = 0.722; // O ; B 60 -18 699 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[80] = 0.611; // P ; B 0 0 605 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[81] = 0.722; // Q ; B 59 -182 699 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[82] = 0.611; // R ; B -13 0 588 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[83] = 0.500; // S ; B 17 -18 508 667 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[84] = 0.556; // T ; B 59 0 633 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[85] = 0.722; // U ; B 102 -18 765 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[86] = 0.611; // V ; B 76 -18 688 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[87] = 0.833; // W ; B 71 -18 906 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[88] = 0.611; // X ; B -29 0 655 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[89] = 0.556; // Y ; B 78 0 633 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[90] = 0.556; // Z ; B -6 0 606 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[91] = 0.389; // bracketleft ; B 21 -153 391 663 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[92] = 0.278; // backslash ; B -41 -18 319 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[93] = 0.389; // bracketright ; B 12 -153 382 663 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[94] = 0.422; // asciicircum ; B 0 301 422 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[95] = 0.500; // underscore ; B 0 -125 500 -75 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[96] = 0.333; // quoteleft ; B 171 436 310 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[97] = 0.500; // a ; B 17 -11 476 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[98] = 0.500; // b ; B 23 -11 473 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[99] = 0.444; // c ; B 30 -11 425 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[100] = 0.500; // d ; B 15 -13 527 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[101] = 0.444; // e ; B 31 -11 412 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[102] = 0.278; // f ; B -147 -207 424 678 ; L i fi ; L l fl ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[103] = 0.500; // g ; B 8 -206 472 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[104] = 0.500; // h ; B 19 -9 478 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[105] = 0.278; // i ; B 49 -11 264 654 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[106] = 0.278; // j ; B -124 -207 276 654 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[107] = 0.444; // k ; B 14 -11 461 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[108] = 0.278; // l ; B 41 -11 279 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[109] = 0.722; // m ; B 12 -9 704 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[110] = 0.500; // n ; B 14 -9 474 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[111] = 0.500; // o ; B 27 -11 468 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[112] = 0.500; // p ; B -75 -205 469 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[113] = 0.500; // q ; B 25 -209 483 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[114] = 0.389; // r ; B 45 0 412 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[115] = 0.389; // s ; B 16 -13 366 442 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[116] = 0.278; // t ; B 37 -11 296 546 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[117] = 0.500; // u ; B 42 -11 475 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[118] = 0.444; // v ; B 21 -18 426 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[119] = 0.667; // w ; B 16 -18 648 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[120] = 0.444; // x ; B -27 -11 447 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[121] = 0.444; // y ; B -24 -206 426 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[122] = 0.389; // z ; B -2 -81 380 428 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[123] = 0.400; // braceleft ; B 51 -177 407 687 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[124] = 0.275; // bar ; B 105 -217 171 783 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[125] = 0.400; // braceright ; B -7 -177 349 687 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[126] = 0.541; // asciitilde ; B 40 183 502 323 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[161] = 0.389; // exclamdown ; B 59 -205 322 473 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[162] = 0.500; // cent ; B 77 -143 472 560 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[163] = 0.500; // sterling ; B 10 -6 517 670 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[164] = 0.167; // fraction ; B -169 -10 337 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[165] = 0.500; // yen ; B 27 0 603 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[166] = 0.500; // florin ; B 25 -182 507 682 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[167] = 0.500; // section ; B 53 -162 461 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[168] = 0.500; // currency ; B -22 53 522 597 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[169] = 0.214; // quotesingle ; B 132 421 241 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[170] = 0.556; // quotedblleft ; B 166 436 514 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[171] = 0.500; // guillemotleft ; B 53 37 445 403 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[172] = 0.333; // guilsinglleft ; B 51 37 281 403 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[173] = 0.333; // guilsinglright ; B 52 37 282 403 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[174] = 0.500; // fi ; B -141 -207 481 681 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[175] = 0.500; // fl ; B -141 -204 518 682 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[177] = 0.500; // endash ; B -6 197 505 243 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[178] = 0.500; // dagger ; B 101 -159 488 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[179] = 0.500; // daggerdbl ; B 22 -143 491 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[180] = 0.250; // periodcentered ; B 70 199 181 310 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[182] = 0.523; // paragraph ; B 55 -123 616 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[183] = 0.350; // bullet ; B 40 191 310 461 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[184] = 0.333; // quotesinglbase ; B 44 -129 183 101 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[185] = 0.556; // quotedblbase ; B 57 -129 405 101 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[186] = 0.556; // quotedblright ; B 151 436 499 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[187] = 0.500; // guillemotright ; B 55 37 447 403 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[188] = 0.889; // ellipsis ; B 57 -11 762 100 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[189] = 1.000; // perthousand ; B 25 -19 1010 706 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[191] = 0.500; // questiondown ; B 28 -205 368 471 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[193] = 0.333; // grave ; B 121 492 311 664 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[194] = 0.333; // acute ; B 180 494 403 664 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[195] = 0.333; // circumflex ; B 91 492 385 661 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[196] = 0.333; // tilde ; B 100 517 427 624 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[197] = 0.333; // macron ; B 99 532 411 583 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[198] = 0.333; // breve ; B 117 492 418 650 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[199] = 0.333; // dotaccent ; B 207 548 305 646 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[200] = 0.333; // dieresis ; B 107 548 405 646 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[202] = 0.333; // ring ; B 155 492 355 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[203] = 0.333; // cedilla ; B -30 -217 182 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[205] = 0.333; // hungarumlaut ; B 93 494 486 664 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[206] = 0.333; // ogonek ; B 20 -169 203 40 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[207] = 0.333; // caron ; B 121 492 426 661 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[208] = 0.889; // emdash ; B -6 197 894 243 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[225] = 0.889; // AE ; B -27 0 911 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[227] = 0.276; // ordfeminine ; B 42 406 352 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[232] = 0.556; // Lslash ; B -8 0 559 653 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[233] = 0.722; // Oslash ; B 60 -105 699 722 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[234] = 0.944; // OE ; B 49 -8 964 666 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[235] = 0.310; // ordmasculine ; B 67 406 362 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[241] = 0.667; // ae ; B 23 -11 640 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[245] = 0.278; // dotlessi ; B 49 -11 235 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[248] = 0.278; // lslash ; B 41 -11 312 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[249] = 0.500; // oslash ; B 28 -135 469 554 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[250] = 0.667; // oe ; B 20 -12 646 441 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[251] = 0.500; // germandbls ; B -168 -207 493 679 ;
}

void set_Times_Roman(Params *pParams)
{
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[32] = 0.250; // space ; B 0 0 0 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[33] = 0.333; // exclam ; B 130 -9 238 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[34] = 0.408; // quotedbl ; B 77 431 331 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[35] = 0.500; // numbersign ; B 5 0 496 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[36] = 0.500; // dollar ; B 44 -87 457 727 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[37] = 0.833; // percent ; B 61 -13 772 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[38] = 0.778; // ampersand ; B 42 -13 750 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[39] = 0.333; // quoteright ; B 79 433 218 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[40] = 0.333; // parenleft ; B 48 -177 304 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[41] = 0.333; // parenright ; B 29 -177 285 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[42] = 0.500; // asterisk ; B 69 265 432 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[43] = 0.564; // plus ; B 30 0 534 506 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[44] = 0.250; // comma ; B 56 -141 195 102 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[45] = 0.333; // hyphen ; B 39 194 285 257 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[46] = 0.250; // period ; B 70 -11 181 100 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[47] = 0.278; // slash ; B -9 -14 287 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[48] = 0.500; // zero ; B 24 -14 476 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[49] = 0.500; // one ; B 111 0 394 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[50] = 0.500; // two ; B 30 0 475 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[51] = 0.500; // three ; B 43 -14 431 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[52] = 0.500; // four ; B 12 0 472 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[53] = 0.500; // five ; B 32 -14 438 688 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[54] = 0.500; // six ; B 34 -14 468 684 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[55] = 0.500; // seven ; B 20 -8 449 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[56] = 0.500; // eight ; B 56 -14 445 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[57] = 0.500; // nine ; B 30 -22 459 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[58] = 0.278; // colon ; B 81 -11 192 459 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[59] = 0.278; // semicolon ; B 80 -141 219 459 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[60] = 0.564; // less ; B 28 -8 536 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[61] = 0.564; // equal ; B 30 120 534 386 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[62] = 0.564; // greater ; B 28 -8 536 514 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[63] = 0.444; // question ; B 68 -8 414 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[64] = 0.921; // at ; B 116 -14 809 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[65] = 0.722; // A ; B 15 0 706 674 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[66] = 0.667; // B ; B 17 0 593 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[67] = 0.667; // C ; B 28 -14 633 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[68] = 0.722; // D ; B 16 0 685 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[69] = 0.611; // E ; B 12 0 597 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[70] = 0.556; // F ; B 12 0 546 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[71] = 0.722; // G ; B 32 -14 709 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[72] = 0.722; // H ; B 19 0 702 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[73] = 0.333; // I ; B 18 0 315 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[74] = 0.389; // J ; B 10 -14 370 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[75] = 0.722; // K ; B 34 0 723 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[76] = 0.611; // L ; B 12 0 598 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[77] = 0.889; // M ; B 12 0 863 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[78] = 0.722; // N ; B 12 -11 707 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[79] = 0.722; // O ; B 34 -14 688 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[80] = 0.556; // P ; B 16 0 542 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[81] = 0.722; // Q ; B 34 -178 701 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[82] = 0.667; // R ; B 17 0 659 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[83] = 0.556; // S ; B 42 -14 491 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[84] = 0.611; // T ; B 17 0 593 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[85] = 0.722; // U ; B 14 -14 705 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[86] = 0.722; // V ; B 16 -11 697 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[87] = 0.944; // W ; B 5 -11 932 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[88] = 0.722; // X ; B 10 0 704 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[89] = 0.722; // Y ; B 22 0 703 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[90] = 0.611; // Z ; B 9 0 597 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[91] = 0.333; // bracketleft ; B 88 -156 299 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[92] = 0.278; // backslash ; B -9 -14 287 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[93] = 0.333; // bracketright ; B 34 -156 245 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[94] = 0.469; // asciicircum ; B 24 297 446 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[95] = 0.500; // underscore ; B 0 -125 500 -75 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[96] = 0.333; // quoteleft ; B 115 433 254 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[97] = 0.444; // a ; B 37 -10 442 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[98] = 0.500; // b ; B 3 -10 468 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[99] = 0.444; // c ; B 25 -10 412 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[100] = 0.500; // d ; B 27 -10 491 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[101] = 0.444; // e ; B 25 -10 424 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[102] = 0.333; // f ; B 20 0 383 683 ; L i fi ; L l fl ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[103] = 0.500; // g ; B 28 -218 470 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[104] = 0.500; // h ; B 9 0 487 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[105] = 0.278; // i ; B 16 0 253 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[106] = 0.278; // j ; B -70 -218 194 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[107] = 0.500; // k ; B 7 0 505 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[108] = 0.278; // l ; B 19 0 257 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[109] = 0.778; // m ; B 16 0 775 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[110] = 0.500; // n ; B 16 0 485 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[111] = 0.500; // o ; B 29 -10 470 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[112] = 0.500; // p ; B 5 -217 470 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[113] = 0.500; // q ; B 24 -217 488 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[114] = 0.333; // r ; B 5 0 335 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[115] = 0.389; // s ; B 51 -10 348 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[116] = 0.278; // t ; B 13 -10 279 579 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[117] = 0.500; // u ; B 9 -10 479 450 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[118] = 0.500; // v ; B 19 -14 477 450 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[119] = 0.722; // w ; B 21 -14 694 450 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[120] = 0.500; // x ; B 17 0 479 450 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[121] = 0.500; // y ; B 14 -218 475 450 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[122] = 0.444; // z ; B 27 0 418 450 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[123] = 0.480; // braceleft ; B 100 -181 350 680 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[124] = 0.200; // bar ; B 67 -218 133 782 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[125] = 0.480; // braceright ; B 130 -181 380 680 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[126] = 0.541; // asciitilde ; B 40 183 502 323 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[161] = 0.333; // exclamdown ; B 97 -218 205 467 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[162] = 0.500; // cent ; B 53 -138 448 579 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[163] = 0.500; // sterling ; B 12 -8 490 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[164] = 0.167; // fraction ; B -168 -14 331 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[165] = 0.500; // yen ; B -53 0 512 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[166] = 0.500; // florin ; B 7 -189 490 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[167] = 0.500; // section ; B 70 -148 426 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[168] = 0.500; // currency ; B -22 58 522 602 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[169] = 0.180; // quotesingle ; B 48 431 133 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[170] = 0.444; // quotedblleft ; B 43 433 414 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[171] = 0.500; // guillemotleft ; B 42 33 456 416 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[172] = 0.333; // guilsinglleft ; B 63 33 285 416 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[173] = 0.333; // guilsinglright ; B 48 33 270 416 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[174] = 0.556; // fi ; B 31 0 521 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[175] = 0.556; // fl ; B 32 0 521 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[177] = 0.500; // endash ; B 0 201 500 250 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[178] = 0.500; // dagger ; B 59 -149 442 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[179] = 0.500; // daggerdbl ; B 58 -153 442 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[180] = 0.250; // periodcentered ; B 70 199 181 310 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[182] = 0.453; // paragraph ; B -22 -154 450 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[183] = 0.350; // bullet ; B 40 196 310 466 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[184] = 0.333; // quotesinglbase ; B 79 -141 218 102 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[185] = 0.444; // quotedblbase ; B 45 -141 416 102 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[186] = 0.444; // quotedblright ; B 30 433 401 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[187] = 0.500; // guillemotright ; B 44 33 458 416 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[188] = 1.000; // ellipsis ; B 111 -11 888 100 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[189] = 1.000; // perthousand ; B 7 -19 994 706 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[191] = 0.444; // questiondown ; B 30 -218 376 466 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[193] = 0.333; // grave ; B 19 507 242 678 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[194] = 0.333; // acute ; B 93 507 317 678 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[195] = 0.333; // circumflex ; B 11 507 322 674 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[196] = 0.333; // tilde ; B 1 532 331 638 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[197] = 0.333; // macron ; B 11 547 322 601 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[198] = 0.333; // breve ; B 26 507 307 664 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[199] = 0.333; // dotaccent ; B 118 581 216 681 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[200] = 0.333; // dieresis ; B 18 581 315 681 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[202] = 0.333; // ring ; B 67 512 266 711 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[203] = 0.333; // cedilla ; B 52 -215 261 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[205] = 0.333; // hungarumlaut ; B -3 507 377 678 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[206] = 0.333; // ogonek ; B 62 -165 243 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[207] = 0.333; // caron ; B 11 507 322 674 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[208] = 1.000; // emdash ; B 0 201 1000 250 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[225] = 0.889; // AE ; B 0 0 863 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[227] = 0.276; // ordfeminine ; B 4 394 270 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[232] = 0.611; // Lslash ; B 12 0 598 662 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[233] = 0.722; // Oslash ; B 34 -80 688 734 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[234] = 0.889; // OE ; B 30 -6 885 668 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[235] = 0.310; // ordmasculine ; B 6 394 304 676 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[241] = 0.667; // ae ; B 38 -10 632 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[245] = 0.278; // dotlessi ; B 16 0 253 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[248] = 0.278; // lslash ; B 19 0 259 683 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[249] = 0.500; // oslash ; B 29 -112 470 551 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[250] = 0.722; // oe ; B 30 -10 690 460 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[251] = 0.500; // germandbls ; B 12 -9 468 683 ;
}

void set_ZapfDingbats(Params *pParams)
{
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[32] = 0.278; // space ; B 0 0 0 0 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[33] = 0.974; // a1 ; B 35 72 939 621 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[34] = 0.961; // a2 ; B 35 81 927 611 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[35] = 0.974; // a202 ; B 35 72 939 621 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[36] = 0.980; // a3 ; B 35 0 945 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[37] = 0.719; // a4 ; B 34 139 685 566 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[38] = 0.789; // a5 ; B 35 -14 755 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[39] = 0.790; // a119 ; B 35 -14 755 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[40] = 0.791; // a118 ; B 35 -13 761 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[41] = 0.690; // a117 ; B 34 138 655 553 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[42] = 0.960; // a11 ; B 35 123 925 568 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[43] = 0.939; // a12 ; B 35 134 904 559 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[44] = 0.549; // a13 ; B 29 -11 516 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[45] = 0.855; // a14 ; B 34 59 820 632 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[46] = 0.911; // a15 ; B 35 50 876 642 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[47] = 0.933; // a16 ; B 35 139 899 550 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[48] = 0.911; // a105 ; B 35 50 876 642 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[49] = 0.945; // a17 ; B 35 139 909 553 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[50] = 0.974; // a18 ; B 35 104 938 587 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[51] = 0.755; // a19 ; B 34 -13 721 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[52] = 0.846; // a20 ; B 36 -14 811 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[53] = 0.762; // a21 ; B 35 0 727 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[54] = 0.761; // a22 ; B 35 0 727 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[55] = 0.571; // a23 ; B -1 -68 571 661 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[56] = 0.677; // a24 ; B 36 -13 642 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[57] = 0.763; // a25 ; B 35 0 728 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[58] = 0.760; // a26 ; B 35 0 726 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[59] = 0.759; // a27 ; B 35 0 725 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[60] = 0.754; // a28 ; B 35 0 720 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[61] = 0.494; // a6 ; B 35 0 460 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[62] = 0.552; // a7 ; B 35 0 517 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[63] = 0.537; // a8 ; B 35 0 503 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[64] = 0.577; // a9 ; B 35 96 542 596 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[65] = 0.692; // a10 ; B 35 -14 657 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[66] = 0.786; // a29 ; B 35 -14 751 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[67] = 0.788; // a30 ; B 35 -14 752 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[68] = 0.788; // a31 ; B 35 -14 753 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[69] = 0.790; // a32 ; B 35 -14 756 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[70] = 0.793; // a33 ; B 35 -13 759 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[71] = 0.794; // a34 ; B 35 -13 759 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[72] = 0.816; // a35 ; B 35 -14 782 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[73] = 0.823; // a36 ; B 35 -14 787 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[74] = 0.789; // a37 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[75] = 0.841; // a38 ; B 35 -14 807 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[76] = 0.823; // a39 ; B 35 -14 789 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[77] = 0.833; // a40 ; B 35 -14 798 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[78] = 0.816; // a41 ; B 35 -13 782 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[79] = 0.831; // a42 ; B 35 -14 796 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[80] = 0.923; // a43 ; B 35 -14 888 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[81] = 0.744; // a44 ; B 35 0 710 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[82] = 0.723; // a45 ; B 35 0 688 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[83] = 0.749; // a46 ; B 35 0 714 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[84] = 0.790; // a47 ; B 34 -14 756 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[85] = 0.792; // a48 ; B 35 -14 758 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[86] = 0.695; // a49 ; B 35 -14 661 706 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[87] = 0.776; // a50 ; B 35 -6 741 699 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[88] = 0.768; // a51 ; B 35 -7 734 699 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[89] = 0.792; // a52 ; B 35 -14 757 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[90] = 0.759; // a53 ; B 35 0 725 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[91] = 0.707; // a54 ; B 35 -13 672 704 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[92] = 0.708; // a55 ; B 35 -14 672 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[93] = 0.682; // a56 ; B 35 -14 647 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[94] = 0.701; // a57 ; B 35 -14 666 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[95] = 0.826; // a58 ; B 35 -14 791 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[96] = 0.815; // a59 ; B 35 -14 780 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[97] = 0.789; // a60 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[98] = 0.789; // a61 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[99] = 0.707; // a62 ; B 34 -14 673 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[100] = 0.687; // a63 ; B 36 0 651 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[101] = 0.696; // a64 ; B 35 0 661 691 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[102] = 0.689; // a65 ; B 35 0 655 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[103] = 0.786; // a66 ; B 34 -14 751 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[104] = 0.787; // a67 ; B 35 -14 752 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[105] = 0.713; // a68 ; B 35 -14 678 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[106] = 0.791; // a69 ; B 35 -14 756 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[107] = 0.785; // a70 ; B 36 -14 751 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[108] = 0.791; // a71 ; B 35 -14 757 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[109] = 0.873; // a72 ; B 35 -14 838 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[110] = 0.761; // a73 ; B 35 0 726 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[111] = 0.762; // a74 ; B 35 0 727 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[112] = 0.762; // a203 ; B 35 0 727 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[113] = 0.759; // a75 ; B 35 0 725 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[114] = 0.759; // a204 ; B 35 0 725 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[115] = 0.892; // a76 ; B 35 0 858 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[116] = 0.892; // a77 ; B 35 -14 858 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[117] = 0.788; // a78 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[118] = 0.784; // a79 ; B 35 -14 749 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[119] = 0.438; // a81 ; B 35 -14 403 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[120] = 0.138; // a82 ; B 35 0 104 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[121] = 0.277; // a83 ; B 35 0 242 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[122] = 0.415; // a84 ; B 35 0 380 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[123] = 0.392; // a97 ; B 35 263 357 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[124] = 0.392; // a98 ; B 34 263 357 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[125] = 0.668; // a99 ; B 35 263 633 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[126] = 0.668; // a100 ; B 36 263 634 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[128] = 0.390; // a89 ; B 35 -14 356 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[129] = 0.390; // a90 ; B 35 -14 355 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[130] = 0.317; // a93 ; B 35 0 283 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[131] = 0.317; // a94 ; B 35 0 283 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[132] = 0.276; // a91 ; B 35 0 242 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[133] = 0.276; // a92 ; B 35 0 242 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[134] = 0.509; // a205 ; B 35 0 475 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[135] = 0.509; // a85 ; B 35 0 475 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[136] = 0.410; // a206 ; B 35 0 375 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[137] = 0.410; // a86 ; B 35 0 375 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[138] = 0.234; // a87 ; B 35 -14 199 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[139] = 0.234; // a88 ; B 35 -14 199 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[140] = 0.334; // a95 ; B 35 0 299 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[141] = 0.334; // a96 ; B 35 0 299 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[161] = 0.732; // a101 ; B 35 -143 697 806 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[162] = 0.544; // a102 ; B 56 -14 488 706 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[163] = 0.544; // a103 ; B 34 -14 508 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[164] = 0.910; // a104 ; B 35 40 875 651 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[165] = 0.667; // a106 ; B 35 -14 633 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[166] = 0.760; // a107 ; B 35 -14 726 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[167] = 0.760; // a108 ; B 0 121 758 569 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[168] = 0.776; // a112 ; B 35 0 741 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[169] = 0.595; // a111 ; B 34 -14 560 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[170] = 0.694; // a110 ; B 35 -14 659 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[171] = 0.626; // a109 ; B 34 0 591 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[172] = 0.788; // a120 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[173] = 0.788; // a121 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[174] = 0.788; // a122 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[175] = 0.788; // a123 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[176] = 0.788; // a124 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[177] = 0.788; // a125 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[178] = 0.788; // a126 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[179] = 0.788; // a127 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[180] = 0.788; // a128 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[181] = 0.788; // a129 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[182] = 0.788; // a130 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[183] = 0.788; // a131 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[184] = 0.788; // a132 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[185] = 0.788; // a133 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[186] = 0.788; // a134 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[187] = 0.788; // a135 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[188] = 0.788; // a136 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[189] = 0.788; // a137 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[190] = 0.788; // a138 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[191] = 0.788; // a139 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[192] = 0.788; // a140 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[193] = 0.788; // a141 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[194] = 0.788; // a142 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[195] = 0.788; // a143 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[196] = 0.788; // a144 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[197] = 0.788; // a145 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[198] = 0.788; // a146 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[199] = 0.788; // a147 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[200] = 0.788; // a148 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[201] = 0.788; // a149 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[202] = 0.788; // a150 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[203] = 0.788; // a151 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[204] = 0.788; // a152 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[205] = 0.788; // a153 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[206] = 0.788; // a154 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[207] = 0.788; // a155 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[208] = 0.788; // a156 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[209] = 0.788; // a157 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[210] = 0.788; // a158 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[211] = 0.788; // a159 ; B 35 -14 754 705 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[212] = 0.894; // a160 ; B 35 58 860 634 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[213] = 0.838; // a161 ; B 35 152 803 540 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[214] = 1.016; // a163 ; B 34 152 981 540 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[215] = 0.458; // a164 ; B 35 -127 422 820 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[216] = 0.748; // a196 ; B 35 94 698 597 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[217] = 0.924; // a165 ; B 35 140 890 552 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[218] = 0.748; // a192 ; B 35 94 698 597 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[219] = 0.918; // a166 ; B 35 166 884 526 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[220] = 0.927; // a167 ; B 35 32 892 660 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[221] = 0.928; // a168 ; B 35 129 891 562 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[222] = 0.928; // a169 ; B 35 128 893 563 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[223] = 0.834; // a170 ; B 35 155 799 537 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[224] = 0.873; // a171 ; B 35 93 838 599 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[225] = 0.828; // a172 ; B 35 104 791 588 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[226] = 0.924; // a173 ; B 35 98 889 594 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[227] = 0.924; // a162 ; B 35 98 889 594 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[228] = 0.917; // a174 ; B 35 0 882 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[229] = 0.930; // a175 ; B 35 84 896 608 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[230] = 0.931; // a176 ; B 35 84 896 608 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[231] = 0.463; // a177 ; B 35 -99 429 791 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[232] = 0.883; // a178 ; B 35 71 848 623 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[233] = 0.836; // a179 ; B 35 44 802 648 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[234] = 0.836; // a193 ; B 35 44 802 648 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[235] = 0.867; // a180 ; B 35 101 832 591 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[236] = 0.867; // a199 ; B 35 101 832 591 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[237] = 0.696; // a181 ; B 35 44 661 648 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[238] = 0.696; // a200 ; B 35 44 661 648 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[239] = 0.874; // a182 ; B 35 77 840 619 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[241] = 0.874; // a201 ; B 35 73 840 615 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[242] = 0.760; // a183 ; B 35 0 725 692 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[243] = 0.946; // a184 ; B 35 160 911 533 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[244] = 0.771; // a197 ; B 34 37 736 655 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[245] = 0.865; // a185 ; B 35 207 830 481 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[246] = 0.771; // a194 ; B 34 37 736 655 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[247] = 0.888; // a198 ; B 34 -19 853 712 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[248] = 0.967; // a186 ; B 35 124 932 568 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[249] = 0.888; // a195 ; B 34 -19 853 712 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[250] = 0.831; // a187 ; B 35 113 796 579 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[251] = 0.873; // a188 ; B 36 118 838 578 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[252] = 0.927; // a189 ; B 35 150 891 542 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[253] = 0.970; // a190 ; B 35 76 931 616 ;
	pParams->myObjsTable[pParams->nCurrentParsingFontObjNum]->pGlyphsWidths->pWidths[254] = 0.918; // a191 ; B 34 99 884 593 ;
}
