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

int InitializeCharSetHashTable(Params *pParams)
{
	char szKey[256];
	size_t sizekey;
	
	uint32_t nData;
	
	if ( !htInit(&(pParams->myCharSetHashTable), 8191, StringHashFunc, StringCompareFunc) )
	{
		wprintf(L"ERRORE InitializeCharSetHashTable -> htInit.\n");
		fwprintf(pParams->fpErrors, L"ERRORE InitializeCharSetHashTable -> htInit.\n");
		return 0;
	}	
	
	strcpy(szKey, "A");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0041;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "AE");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00C6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "AEacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x01FC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Aacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00C1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Abreve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0102;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Acircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00C2;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Adieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00C4;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Agrave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00C0;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Alpha");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0391;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Alphatonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0386;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Amacron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0100;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Aogonek");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0104;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Aring");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00C5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Aringacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x01FA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Atilde");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00C3;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "B");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0042;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Beta");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0392;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "C");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0043;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Cacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0106;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ccaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x010C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ccedilla");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00C7;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ccircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0108;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Cdotaccent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x010A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Chi");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03A7;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "D");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0044;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Dcaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x010E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Dcroat");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0110;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Delta");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2206;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "E");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0045;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Eacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00C9;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ebreve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0114;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ecaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x011A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ecircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00CA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Edieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00CB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Edotaccent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0116;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Egrave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00C8;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Emacron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0112;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Eng");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x014A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Eogonek");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0118;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Epsilon");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0395;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Epsilontonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0388;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Eta");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0397;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Etatonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0389;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Eth");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00D0;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Euro");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x20AC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "F");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0046;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "G");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0047;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Gamma");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0393;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Gbreve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x011E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Gcaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x01E6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Gcircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x011C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Gdotaccent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0120;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "H");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0048;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "H18533");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25CF;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "H18543");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25AA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "H18551");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25AB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "H22073");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25A1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Hbar");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0126;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Hcircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0124;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "I");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0049;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "IJ");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0132;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Iacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00CD;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ibreve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x012C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Icircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00CE;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Idieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00CF;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Idotaccent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0130;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ifraktur");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2111;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Igrave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00CC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Imacron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x012A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Iogonek");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x012E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Iota");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0399;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Iotadieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03AA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Iotatonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x038A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Itilde");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0128;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "J");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x004A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Jcircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0134;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "K");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x004B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Kappa");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x039A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "L");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x004C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Lacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0139;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Lambda");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x039B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Lcaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x013D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ldot");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x013F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Lslash");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0141;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "M");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x004D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Mu");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x039C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "N");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x004E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Nacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0143;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ncaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0147;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ntilde");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00D1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Nu");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x039D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "O");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x004F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "OE");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0152;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Oacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00D3;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Obreve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x014E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ocircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00D4;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Odieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00D6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ograve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00D2;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ohorn");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x01A0;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ohungarumlaut");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0150;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Omacron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x014C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Omega");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2126;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Omegatonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x038F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Omicron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x039F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Omicrontonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x038C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Oslash");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00D8;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Oslashacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x01FE;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Otilde");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00D5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "P");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0050;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Phi");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03A6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Pi");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03A0;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Psi");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03A8;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Q");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0051;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "R");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0052;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Racute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0154;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Rcaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0158;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Rfraktur");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x211C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Rho");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03A1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "S");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0053;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF010000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x250C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF020000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2514;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF030000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2510;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF040000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2518;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF050000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x253C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF060000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x252C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF070000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2534;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF080000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x251C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF090000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2524;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF100000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2500;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF110000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2502;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF190000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2561;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF200000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2562;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF210000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2556;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF220000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2555;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF230000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2563;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF240000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2551;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF250000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2557;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF260000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x255D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF270000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x255C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF280000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x255B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF360000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x255E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF370000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x255F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF380000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x255A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF390000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2554;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF400000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2569;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF410000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2566;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF420000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2560;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF430000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2550;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF440000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x256C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF450000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2567;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF460000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2568;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF470000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2564;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF480000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2565;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF490000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2559;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF500000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2558;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF510000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2552;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF520000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2553;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF530000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x256B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "SF540000");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x256A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Sacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x015A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Scaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0160;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Scedilla");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x015E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Scircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x015C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Sigma");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03A3;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "T");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0054;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Tau");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03A4;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Tbar");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0166;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Tcaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0164;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Theta");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0398;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Thorn");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00DE;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "U");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0055;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Uacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00DA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ubreve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x016C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ucircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00DB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Udieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00DC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ugrave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00D9;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Uhorn");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x01AF;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Uhungarumlaut");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0170;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Umacron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x016A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Uogonek");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0172;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Upsilon");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03A5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Upsilon1");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03D2;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Upsilondieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03AB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Upsilontonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x038E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Uring");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x016E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Utilde");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0168;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "V");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0056;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "W");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0057;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Wacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x1E82;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Wcircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0174;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Wdieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x1E84;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Wgrave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x1E80;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "X");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0058;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Xi");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x039E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Y");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0059;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Yacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00DD;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ycircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0176;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ydieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0178;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Ygrave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x1EF2;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Z");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x005A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Zacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0179;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Zcaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x017D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Zdotaccent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x017B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "Zeta");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0396;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "a");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0061;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "aacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00E1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "abreve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0103;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "acircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00E2;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "acute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00B4;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "acutecomb");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0301;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "adieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00E4;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ae");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00E6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "aeacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x01FD;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "agrave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00E0;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "aleph");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2135;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "alpha");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03B1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "alphatonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03AC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "amacron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0101;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ampersand");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0026;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "angle");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2220;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "angleleft");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2329;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "angleright");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x232A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "anoteleia");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0387;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "aogonek");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0105;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "approxequal");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2248;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "aring");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00E5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "aringacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x01FB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "arrowboth");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2194;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "arrowdblboth");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x21D4;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "arrowdbldown");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x21D3;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "arrowdblleft");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x21D0;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "arrowdblright");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x21D2;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "arrowdblup");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x21D1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "arrowdown");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2193;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "arrowleft");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2190;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "arrowright");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2192;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "arrowup");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2191;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "arrowupdn");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2195;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "arrowupdnbse");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x21A8;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "asciicircum");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x005E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "asciitilde");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x007E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "asterisk");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x002A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "asteriskmath");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2217;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "at");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0040;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "atilde");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00E3;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "b");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0062;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "backslash");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x005C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "bar");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x007C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "beta");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03B2;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "block");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2588;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "braceleft");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x007B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "braceright");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x007D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "bracketleft");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x005B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "bracketright");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x005D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "breve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x02D8;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "brokenbar");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00A6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "bullet");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2022;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "c");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0063;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "cacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0107;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "caron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x02C7;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "carriagereturn");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x21B5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ccaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x010D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ccedilla");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00E7;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ccircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0109;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "cdotaccent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x010B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "cedilla");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00B8;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "cent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00A2;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "chi");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03C7;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "circle");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25CB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "circlemultiply");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2297;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "circleplus");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2295;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "circumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x02C6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "club");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2663;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "colon");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x003A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "colonmonetary");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x20A1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "comma");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x002C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "congruent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2245;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "copyright");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00A9;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "currency");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00A4;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "d");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0064;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dagger");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2020;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "daggerdbl");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2021;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dcaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x010F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dcroat");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0111;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "degree");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00B0;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "delta");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03B4;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "diamond");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2666;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00A8;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dieresistonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0385;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "divide");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00F7;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dkshade");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2593;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dnblock");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2584;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dollar");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0024;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dong");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x20AB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dotaccent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x02D9;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dotbelowcomb");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0323;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dotlessi");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0131;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "dotmath");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x22C5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "e");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0065;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "eacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00E9;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ebreve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0115;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ecaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x011B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ecircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00EA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "edieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00EB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "edotaccent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0117;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "egrave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00E8;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "eight");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0038;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "element");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2208;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ellipsis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2026;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "emacron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0113;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "emdash");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2014;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "emptyset");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2205;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "endash");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2013;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "eng");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x014B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "eogonek");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0119;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "epsilon");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03B5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "epsilontonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03AD;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "equal");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x003D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "equivalence");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2261;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "estimated");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x212E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "eta");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03B7;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "etatonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03AE;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "eth");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00F0;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "exclam");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0021;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "exclamdbl");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x203C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "exclamdown");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00A1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "existential");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2203;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "f");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0066;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "female");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2640;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "figuredash");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2012;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "filledbox");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25A0;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "filledrect");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25AC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "five");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0035;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "fiveeighths");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x215D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "florin");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0192;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "four");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0034;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "fraction");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2044;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "franc");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x20A3;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "g");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0067;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "gamma");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03B3;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "gbreve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x011F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "gcaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x01E7;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "gcircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x011D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "gdotaccent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0121;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "germandbls");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00DF;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "gradient");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2207;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "grave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0060;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "gravecomb");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0300;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "greater");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x003E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "greaterequal");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2265;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "guillemotleft");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00AB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "guillemotright");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00BB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "guilsinglleft");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2039;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "guilsinglright");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x203A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "h");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0068;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "hbar");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0127;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "hcircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0125;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "heart");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2665;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "hookabovecomb");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0309;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "house");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2302;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "hungarumlaut");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x02DD;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "hyphen");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x002D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "i");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0069;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "iacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00ED;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ibreve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x012D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "icircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00EE;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "idieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00EF;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "igrave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00EC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ij");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0133;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "imacron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x012B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "infinity");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x221E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "integral");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x222B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "integralbt");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2321;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "integraltp");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2320;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "intersection");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2229;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "invbullet");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25D8;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "invcircle");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25D9;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "invsmileface");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x263B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "iogonek");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x012F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "iota");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03B9;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "iotadieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03CA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "iotadieresistonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0390;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "iotatonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03AF;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "itilde");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0129;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "j");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x006A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "jcircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0135;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "k");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x006B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "kappa");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03BA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "kgreenlandic");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0138;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "l");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x006C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "lacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x013A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "lambda");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03BB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "lcaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x013E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ldot");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0140;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "less");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x003C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "lessequal");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2264;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "lfblock");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x258C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "lira");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x20A4;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "logicaland");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2227;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "logicalnot");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00AC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "logicalor");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2228;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "longs");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x017F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "lozenge");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25CA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "lslash");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0142;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ltshade");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2591;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "m");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x006D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "macron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00AF;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "male");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2642;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "minus");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2212;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "minute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2032;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "mu");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00B5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "multiply");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00D7;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "musicalnote");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x266A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "musicalnotedbl");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x266B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "n");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x006E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "nacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0144;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "napostrophe");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0149;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ncaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0148;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "nine");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0039;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "notelement");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2209;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "notequal");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2260;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "notsubset");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2284;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ntilde");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00F1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "nu");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03BD;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "numbersign");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0023;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "o");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x006F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "oacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00F3;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "obreve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x014F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ocircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00F4;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "odieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00F6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "oe");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0153;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ogonek");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x02DB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ograve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00F2;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ohorn");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x01A1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ohungarumlaut");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0151;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "omacron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x014D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "omega");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03C9;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "omega1");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03D6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "omegatonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03CE;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "omicron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03BF;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "omicrontonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03CC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "one");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0031;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "onedotenleader");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2024;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "oneeighth");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x215B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "onehalf");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00BD;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "onequarter");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00BC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "onethird");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2153;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "openbullet");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25E6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ordfeminine");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00AA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ordmasculine");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00BA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "orthogonal");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x221F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "oslash");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00F8;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "oslashacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x01FF;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "otilde");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00F5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "p");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0070;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "paragraph");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00B6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "parenleft");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0028;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "parenright");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0029;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "partialdiff");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2202;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "percent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0025;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "period");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x002E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "periodcentered");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00B7;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "perpendicular");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x22A5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "perthousand");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2030;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "peseta");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x20A7;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "phi");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03C6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "phi1");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03D5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "pi");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03C0;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "plus");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x002B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "plusminus");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00B1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "prescription");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x211E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "product");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x220F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "propersubset");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2282;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "propersuperset");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2283;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "proportional");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x221D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "psi");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03C8;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "q");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0071;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "question");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x003F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "questiondown");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00BF;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "quotedbl");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0022;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "quotedblbase");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x201E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "quotedblleft");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x201C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "quotedblright");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x201D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "quoteleft");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2018;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "quotereversed");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x201B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "quoteright");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2019;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "quotesinglbase");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x201A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "quotesingle");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0027;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "r");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0072;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "racute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0155;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "radical");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x221A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "rcaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0159;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "reflexsubset");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2286;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "reflexsuperset");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2287;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "registered");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00AE;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "revlogicalnot");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2310;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "rho");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03C1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ring");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x02DA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "rtblock");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2590;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "s");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0073;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "sacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x015B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "scaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0161;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "scedilla");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x015F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "scircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x015D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "second");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2033;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "section");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00A7;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "semicolon");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x003B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "seven");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0037;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "seveneighths");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x215E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "shade");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2592;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "sigma");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03C3;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "sigma1");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03C2;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "similar");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x223C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "six");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0036;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "slash");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x002F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "smileface");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x263A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "space");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0020;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "spade");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2660;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "sterling");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00A3;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "suchthat");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x220B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "summation");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2211;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "sun");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x263C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "t");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0074;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "tau");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03C4;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "tbar");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0167;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "tcaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0165;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "therefore");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2234;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "theta");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03B8;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "theta1");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03D1;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "thorn");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00FE;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "three");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0033;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "threeeighths");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x215C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "threequarters");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00BE;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "tilde");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x02DC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "tildecomb");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0303;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "tonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0384;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "trademark");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2122;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "triagdn");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25BC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "triaglf");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25C4;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "triagrt");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25BA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "triagup");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x25B2;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "two");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0032;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "twodotenleader");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2025;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "twothirds");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2154;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "u");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0075;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "uacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00FA;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ubreve");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x016D;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ucircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00FB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "udieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00FC;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ugrave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00F9;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "uhorn");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x01B0;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "uhungarumlaut");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0171;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "umacron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x016B;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "underscore");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x005F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "underscoredbl");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2017;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "union");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x222A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "universal");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2200;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "uogonek");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0173;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "upblock");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2580;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "upsilon");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03C5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "upsilondieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03CB;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "upsilondieresistonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03B0;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "upsilontonos");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03CD;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "uring");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x016F;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "utilde");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0169;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "v");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0076;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "w");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0077;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "wacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x1E83;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "wcircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0175;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "wdieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x1E85;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "weierstrass");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x2118;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "wgrave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x1E81;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "x");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0078;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "xi");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03BE;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "y");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0079;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "yacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00FD;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ycircumflex");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0177;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ydieresis");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00FF;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "yen");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x00A5;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "ygrave");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x1EF3;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "z");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x007A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "zacute");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x017A;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "zcaron");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x017E;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "zdotaccent");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x017C;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "zero");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x0030;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);

	strcpy(szKey, "zeta");
	sizekey = strlen(szKey) + sizeof(char);
	nData = 0x03B6;
	htInsert(&(pParams->myCharSetHashTable), szKey, sizekey, &nData, sizeof(nData), 0);
		
	return 1;		
}

void InitializeCharSetArrays(Params *pParams)
{
	uint32_t k;
		
	for ( k = 0; k < 32; k++ )
		pParams->aUtf8CharSet[k] = k;
	
	pParams->aUtf8CharSet[32] = (int)L' ';
	pParams->aUtf8CharSet[33] = (int)L'!';
	pParams->aUtf8CharSet[34] = (int)L'"';
	pParams->aUtf8CharSet[35] = (int)L'#';
	pParams->aUtf8CharSet[36] = (int)L'$';
	pParams->aUtf8CharSet[37] = (int)L'%';
	pParams->aUtf8CharSet[38] = (int)L'&';
	pParams->aUtf8CharSet[39] = (int)L'\'';
	pParams->aUtf8CharSet[40] = (int)L'(';
	pParams->aUtf8CharSet[41] = (int)L')';
	pParams->aUtf8CharSet[42] = (int)L'*';
	pParams->aUtf8CharSet[43] = (int)L'+';
	pParams->aUtf8CharSet[44] = (int)L',';
	pParams->aUtf8CharSet[45] = (int)L'-';
	pParams->aUtf8CharSet[46] = (int)L'.';
	pParams->aUtf8CharSet[47] = (int)L'/';
	pParams->aUtf8CharSet[48] = (int)L'0';
	pParams->aUtf8CharSet[49] = (int)L'1';
	pParams->aUtf8CharSet[50] = (int)L'2';
	pParams->aUtf8CharSet[51] = (int)L'3';
	pParams->aUtf8CharSet[52] = (int)L'4';
	pParams->aUtf8CharSet[53] = (int)L'5';
	pParams->aUtf8CharSet[54] = (int)L'6';
	pParams->aUtf8CharSet[55] = (int)L'7';
	pParams->aUtf8CharSet[56] = (int)L'8';
	pParams->aUtf8CharSet[57] = (int)L'9';
	pParams->aUtf8CharSet[58] = (int)L':';
	pParams->aUtf8CharSet[59] = (int)L';';
	pParams->aUtf8CharSet[60] = (int)L'<';
	pParams->aUtf8CharSet[61] = (int)L'=';
	pParams->aUtf8CharSet[62] = (int)L'>';
	pParams->aUtf8CharSet[63] = (int)L'?';
	pParams->aUtf8CharSet[64] = (int)L'@';
	pParams->aUtf8CharSet[65] = (int)L'A';
	pParams->aUtf8CharSet[66] = (int)L'B';
	pParams->aUtf8CharSet[67] = (int)L'C';
	pParams->aUtf8CharSet[68] = (int)L'D';
	pParams->aUtf8CharSet[69] = (int)L'E';
	pParams->aUtf8CharSet[70] = (int)L'F';
	pParams->aUtf8CharSet[71] = (int)L'G';
	pParams->aUtf8CharSet[72] = (int)L'H';
	pParams->aUtf8CharSet[73] = (int)L'I';
	pParams->aUtf8CharSet[74] = (int)L'J';
	pParams->aUtf8CharSet[75] = (int)L'K';
	pParams->aUtf8CharSet[76] = (int)L'L';
	pParams->aUtf8CharSet[77] = (int)L'M';
	pParams->aUtf8CharSet[78] = (int)L'N';
	pParams->aUtf8CharSet[79] = (int)L'O';
	pParams->aUtf8CharSet[80] = (int)L'P';
	pParams->aUtf8CharSet[81] = (int)L'Q';
	pParams->aUtf8CharSet[82] = (int)L'R';
	pParams->aUtf8CharSet[83] = (int)L'S';
	pParams->aUtf8CharSet[84] = (int)L'T';
	pParams->aUtf8CharSet[85] = (int)L'U';
	pParams->aUtf8CharSet[86] = (int)L'V';
	pParams->aUtf8CharSet[87] = (int)L'W';
	pParams->aUtf8CharSet[88] = (int)L'X';
	pParams->aUtf8CharSet[89] = (int)L'Y';
	pParams->aUtf8CharSet[90] = (int)L'Z';
	pParams->aUtf8CharSet[91] = (int)L'[';
	pParams->aUtf8CharSet[92] = (int)L'\\';
	pParams->aUtf8CharSet[93] = (int)L']';
	pParams->aUtf8CharSet[94] = (int)L'^';
	pParams->aUtf8CharSet[95] = (int)L'_';
	pParams->aUtf8CharSet[96] = (int)L'`';
	pParams->aUtf8CharSet[97] = (int)L'a';
	pParams->aUtf8CharSet[98] = (int)L'b';
	pParams->aUtf8CharSet[99] = (int)L'c';
	pParams->aUtf8CharSet[100] = (int)L'd';
	pParams->aUtf8CharSet[101] = (int)L'e';
	pParams->aUtf8CharSet[102] = (int)L'f';
	pParams->aUtf8CharSet[103] = (int)L'g';
	pParams->aUtf8CharSet[104] = (int)L'h';
	pParams->aUtf8CharSet[105] = (int)L'i';
	pParams->aUtf8CharSet[106] = (int)L'j';
	pParams->aUtf8CharSet[107] = (int)L'k';
	pParams->aUtf8CharSet[108] = (int)L'l';
	pParams->aUtf8CharSet[109] = (int)L'm';
	pParams->aUtf8CharSet[110] = (int)L'n';
	pParams->aUtf8CharSet[111] = (int)L'o';
	pParams->aUtf8CharSet[112] = (int)L'p';
	pParams->aUtf8CharSet[113] = (int)L'q';
	pParams->aUtf8CharSet[114] = (int)L'r';
	pParams->aUtf8CharSet[115] = (int)L's';
	pParams->aUtf8CharSet[116] = (int)L't';
	pParams->aUtf8CharSet[117] = (int)L'u';
	pParams->aUtf8CharSet[118] = (int)L'v';
	pParams->aUtf8CharSet[119] = (int)L'w';
	pParams->aUtf8CharSet[120] = (int)L'x';
	pParams->aUtf8CharSet[121] = (int)L'y';
	pParams->aUtf8CharSet[122] = (int)L'z';
	pParams->aUtf8CharSet[123] = (int)L'{';
	pParams->aUtf8CharSet[124] = (int)L'|';
	pParams->aUtf8CharSet[125] = (int)L'}';
	pParams->aUtf8CharSet[126] = (int)L'~';
	pParams->aUtf8CharSet[127] = 0x7F;
	
	pParams->aUtf8CharSet[128] = (int)L'€';   // €   Euro sign
	pParams->aUtf8CharSet[129] = 0xC280;      //     UNDEFINED
	pParams->aUtf8CharSet[130] = (int)L'‚';   // ‚   Single low-9 quotation mark
	pParams->aUtf8CharSet[131] = (int)L'ƒ';   // ƒ   Latin small letter f with hook
	pParams->aUtf8CharSet[132] = (int)L'„';   // „   Double low-9 quotation mark
	pParams->aUtf8CharSet[133] = (int)L'…';   // …   Horizontal ellipsis
	pParams->aUtf8CharSet[134] = (int)L'†';   // †   Dagger
	pParams->aUtf8CharSet[135] = (int)L'‡';   // ‡   Double dagger
	pParams->aUtf8CharSet[136] = (int)L'ˆ';   // ˆ   Modifier letter circumflex accent
	pParams->aUtf8CharSet[137] = (int)L'‰';   // ‰   Per mille sign
	pParams->aUtf8CharSet[138] = (int)L'Š';   // Š   Latin capital letter S with caron
	pParams->aUtf8CharSet[139] = (int)L'‹';   // ‹   Single left-pointing angle quotation
	pParams->aUtf8CharSet[140] = (int)L'Œ';   // Œ   Latin capital ligature OE
	pParams->aUtf8CharSet[141] = 0xC28D;      //     UNDEFINED
	pParams->aUtf8CharSet[142] = (int)L'Ž';   // Ž   Latin captial letter Z with caron
	pParams->aUtf8CharSet[143] = 0xC28F;      //     UNDEFINED
	pParams->aUtf8CharSet[144] = 0xC290;      //     UNDEFINED
	pParams->aUtf8CharSet[145] = (int)L'‘';   // ‘   Left single quotation mark
	pParams->aUtf8CharSet[146] = (int)L'’';   // ’   Right single quotation mark
	pParams->aUtf8CharSet[147] = (int)L'“';   // “   Left double quotation mark
	pParams->aUtf8CharSet[148] = (int)L'”';   // ”   Right double quotation mark
	pParams->aUtf8CharSet[149] = (int)L'•';   // •   Bullet
	pParams->aUtf8CharSet[150] = (int)L'–';   // –   En dash
	pParams->aUtf8CharSet[151] = (int)L'—';   // —   Em dash
	pParams->aUtf8CharSet[152] = (int)L'˜';   // ˜   Small tilde
	pParams->aUtf8CharSet[153] = (int)L'™';   // ™   Trade mark sign
	pParams->aUtf8CharSet[154] = (int)L'š';   // š   Latin small letter S with caron
	pParams->aUtf8CharSet[155] = (int)L'›';   // ›   Single right-pointing angle quotation mark
	pParams->aUtf8CharSet[156] = (int)L'œ';   // œ   Latin small ligature oe
	pParams->aUtf8CharSet[157] = 0xC29D;      //     UNDEFINED
	pParams->aUtf8CharSet[158] = (int)L'ž';   // ž   Latin small letter z with caron
	pParams->aUtf8CharSet[159] = (int)L'Ÿ';   // Ÿ   Latin capital letter Y with diaeresis
	pParams->aUtf8CharSet[160] = 0x00A0;      //     NON BREAKING SPACE -> https://it.wikipedia.org/wiki/Spazio_unificatore
	
	pParams->aUtf8CharSet[161] = (int)L'¡';
	pParams->aUtf8CharSet[162] = (int)L'¢';
	pParams->aUtf8CharSet[163] = (int)L'£';
	pParams->aUtf8CharSet[164] = (int)L'¤';
	pParams->aUtf8CharSet[165] = (int)L'¥';
	pParams->aUtf8CharSet[166] = (int)L'¦';
	pParams->aUtf8CharSet[167] = (int)L'§';
	pParams->aUtf8CharSet[168] = (int)L'¨';
	pParams->aUtf8CharSet[169] = (int)L'©';
	pParams->aUtf8CharSet[170] = (int)L'ª';
	pParams->aUtf8CharSet[171] = (int)L'«';
	pParams->aUtf8CharSet[172] = (int)L'¬';
	pParams->aUtf8CharSet[173] = (int)L'-';	
	pParams->aUtf8CharSet[174] = (int)L'®';
	pParams->aUtf8CharSet[175] = (int)L'¯';
	pParams->aUtf8CharSet[176] = (int)L'°';
	pParams->aUtf8CharSet[177] = (int)L'±';
	pParams->aUtf8CharSet[178] = (int)L'²';
	pParams->aUtf8CharSet[179] = (int)L'³';
	pParams->aUtf8CharSet[180] = (int)L'´';
	pParams->aUtf8CharSet[181] = (int)L'µ';
	pParams->aUtf8CharSet[182] = (int)L'¶';
	pParams->aUtf8CharSet[183] = (int)L'·';
	pParams->aUtf8CharSet[184] = (int)L'¸';
	pParams->aUtf8CharSet[185] = (int)L'¹';
	pParams->aUtf8CharSet[186] = (int)L'º';
	pParams->aUtf8CharSet[187] = (int)L'»';
	pParams->aUtf8CharSet[188] = (int)L'¼';
	pParams->aUtf8CharSet[189] = (int)L'½';
	pParams->aUtf8CharSet[190] = (int)L'¾';
	pParams->aUtf8CharSet[191] = (int)L'¿';
	pParams->aUtf8CharSet[192] = (int)L'À';
	pParams->aUtf8CharSet[193] = (int)L'Á';
	pParams->aUtf8CharSet[194] = (int)L'Â';
	pParams->aUtf8CharSet[195] = (int)L'Ã';
	pParams->aUtf8CharSet[196] = (int)L'Ä';
	pParams->aUtf8CharSet[197] = (int)L'Å';
	pParams->aUtf8CharSet[198] = (int)L'Æ';
	pParams->aUtf8CharSet[199] = (int)L'Ç';
	pParams->aUtf8CharSet[200] = (int)L'È';
	pParams->aUtf8CharSet[201] = (int)L'É';
	pParams->aUtf8CharSet[202] = (int)L'Ê';
	pParams->aUtf8CharSet[203] = (int)L'Ë';
	pParams->aUtf8CharSet[204] = (int)L'Ì';
	pParams->aUtf8CharSet[205] = (int)L'Í';
	pParams->aUtf8CharSet[206] = (int)L'Î';
	pParams->aUtf8CharSet[207] = (int)L'Ï';
	pParams->aUtf8CharSet[208] = (int)L'Ð';
	pParams->aUtf8CharSet[209] = (int)L'Ñ';
	pParams->aUtf8CharSet[210] = (int)L'Ò';
	pParams->aUtf8CharSet[211] = (int)L'Ó';
	pParams->aUtf8CharSet[212] = (int)L'Ô';
	pParams->aUtf8CharSet[213] = (int)L'Õ';
	pParams->aUtf8CharSet[214] = (int)L'Ö';
	pParams->aUtf8CharSet[215] = (int)L'×';
	pParams->aUtf8CharSet[216] = (int)L'Ø';
	pParams->aUtf8CharSet[217] = (int)L'Ù';
	pParams->aUtf8CharSet[218] = (int)L'Ú';
	pParams->aUtf8CharSet[219] = (int)L'Û';
	pParams->aUtf8CharSet[220] = (int)L'Ü';
	pParams->aUtf8CharSet[221] = (int)L'Ý';
	pParams->aUtf8CharSet[222] = (int)L'Þ';
	pParams->aUtf8CharSet[223] = (int)L'ß';
	pParams->aUtf8CharSet[224] = (int)L'à';
	pParams->aUtf8CharSet[225] = (int)L'á';
	pParams->aUtf8CharSet[226] = (int)L'â';
	pParams->aUtf8CharSet[227] = (int)L'ã';
	pParams->aUtf8CharSet[228] = (int)L'ä';
	pParams->aUtf8CharSet[229] = (int)L'å';
	pParams->aUtf8CharSet[230] = (int)L'æ';
	pParams->aUtf8CharSet[231] = (int)L'ç';
	pParams->aUtf8CharSet[232] = (int)L'è';
	pParams->aUtf8CharSet[233] = (int)L'é';
	pParams->aUtf8CharSet[234] = (int)L'ê';
	pParams->aUtf8CharSet[235] = (int)L'ë';
	pParams->aUtf8CharSet[236] = (int)L'ì';
	pParams->aUtf8CharSet[237] = (int)L'í';
	pParams->aUtf8CharSet[238] = (int)L'î';
	pParams->aUtf8CharSet[239] = (int)L'ï';
	pParams->aUtf8CharSet[240] = (int)L'ð';
	pParams->aUtf8CharSet[241] = (int)L'ñ';
	pParams->aUtf8CharSet[242] = (int)L'ò';
	pParams->aUtf8CharSet[243] = (int)L'ó';
	pParams->aUtf8CharSet[244] = (int)L'ô';
	pParams->aUtf8CharSet[245] = (int)L'õ';
	pParams->aUtf8CharSet[246] = (int)L'ö';
	pParams->aUtf8CharSet[247] = (int)L'÷';
	pParams->aUtf8CharSet[248] = (int)L'ø';
	pParams->aUtf8CharSet[249] = (int)L'ù';
	pParams->aUtf8CharSet[250] = (int)L'ú';
	pParams->aUtf8CharSet[251] = (int)L'û';
	pParams->aUtf8CharSet[252] = (int)L'ü';
	pParams->aUtf8CharSet[253] = (int)L'ý';
	pParams->aUtf8CharSet[254] = (int)L'þ';
	pParams->aUtf8CharSet[255] = (int)L'ÿ';	
			
	for ( k = 0; k < 256; k++ )
	{
		pParams->aSTD_CharSet[k]  =  pParams->aUtf8CharSet[k];
		pParams->aMAC_CharSet[k]  =  pParams->aUtf8CharSet[k];
		pParams->aWIN_CharSet[k]  =  pParams->aUtf8CharSet[k];
		pParams->aPDF_CharSet[k]  =  pParams->aUtf8CharSet[k];
		
		pParams->aMACEXP_CharSet[k]  =  pParams->aUtf8CharSet[k];
	}
	
	MakeDifferencesArrayCodes(pParams);	
	
	MakeMacExpertArrayCodes(pParams);
		
	for ( k = 0; k < 256; k++ )
	{
		pParams->aCustomizedFont_CharSet[k] = 0;
	}	
}

void MakeDifferencesArrayCodes(Params *pParams)
{
	pParams->aSTD_CharSet[0341]  =  (int)L'Æ';   // AE
	pParams->aMAC_CharSet[0256]  =  (int)L'Æ';
		
	pParams->aMAC_CharSet[0347]  =  (int)L'Á';   // Aacute

	pParams->aMAC_CharSet[0345]  =  (int)L'Â';   // Acircumflex
	
	pParams->aMAC_CharSet[0200]  =  (int)L'Ä';   // Adieresis
	
	pParams->aMAC_CharSet[0313]  =  (int)L'À';   // Agrave
	
	pParams->aMAC_CharSet[0201]  =  (int)L'Å';   // Aring
	
	pParams->aMAC_CharSet[0314]  =  (int)L'Ã';   // Atilde
	
	pParams->aMAC_CharSet[0202]  =  (int)L'Ç';   // Ccedilla
	
	pParams->aMAC_CharSet[0203]  =  (int)L'É';   // Eacute
	
	pParams->aMAC_CharSet[0346]  =  (int)L'Ê';   // Ecircumflex
	
	pParams->aMAC_CharSet[0350]  =  (int)L'Ë';   // Edieresis
	
	pParams->aMAC_CharSet[0351]  =  (int)L'È';   // Egrave
	
	pParams->aWIN_CharSet[0200]  =  (int)L'€';   // Euro
	pParams->aPDF_CharSet[0240]  =  (int)L'€';   
	
	pParams->aMAC_CharSet[0352]  =  (int)L'Í';   // Iacute
	
	pParams->aMAC_CharSet[0353]  =  (int)L'Î';   // Icircumflex
	
	pParams->aMAC_CharSet[0354]  =  (int)L'Ï';   // Idieresis
	
	pParams->aMAC_CharSet[0355]  =  (int)L'Ì';   // Igrave
	
	pParams->aSTD_CharSet[0350]  =  (int)L'Ł';   // Lslash
	pParams->aMAC_CharSet[0225]  =  (int)L'Ł';   
	
	pParams->aMAC_CharSet[0204]  =  (int)L'Ñ';   // Ntilde
	
	pParams->aSTD_CharSet[0352]  =  (int)L'Œ';   // OE
	pParams->aMAC_CharSet[0316]  =  (int)L'Œ';   
	pParams->aWIN_CharSet[0214]  =  (int)L'Œ';   
	pParams->aPDF_CharSet[0226]  =  (int)L'Œ';   
	
	pParams->aMAC_CharSet[0356]  =  (int)L'Ó';   // Oacute
	
	pParams->aMAC_CharSet[0357]  =  (int)L'Ô';   // Ocircumflex
	
	pParams->aMAC_CharSet[0205]  =  (int)L'Ö';   // Odieresis
	
	pParams->aMAC_CharSet[0361]  =  (int)L'Ò';   // Ograve
	
	pParams->aSTD_CharSet[0351]  =  (int)L'Ø';   // Oslash
	pParams->aMAC_CharSet[0257]  =  (int)L'Ø';   
	
	pParams->aMAC_CharSet[0315]  =  (int)L'Õ';   // Otilde
	
	pParams->aWIN_CharSet[0212]  =  (int)L'Š';   // Scaron
	pParams->aPDF_CharSet[0227]  =  (int)L'Š';   
	
	pParams->aMAC_CharSet[0362]  =  (int)L'Ú';   // Uacute
	
	pParams->aMAC_CharSet[0363]  =  (int)L'Û';   // Ucircumflex
	
	pParams->aMAC_CharSet[0206]  =  (int)L'Ü';   // Udieresis
	
 	pParams->aMAC_CharSet[0364]  =  (int)L'Ù';   // Ugrave
	
	pParams->aMAC_CharSet[0331]  =  (int)L'Ÿ';   // Ydieresis
	pParams->aWIN_CharSet[0237]  =  (int)L'Ÿ';   
	pParams->aPDF_CharSet[0230]  =  (int)L'Ÿ';   
	
	pParams->aWIN_CharSet[0216]  =  (int)L'Ž';   // Zcaron
	pParams->aPDF_CharSet[0231]  =  (int)L'Ž';  
	
	pParams->aMAC_CharSet[0207]  =  (int)L'á';   // aacute
	
	pParams->aMAC_CharSet[0211]  =  (int)L'â';   // acircumflex
	
	pParams->aSTD_CharSet[0302]  =  (int)L'´';   // acute
 	pParams->aMAC_CharSet[0253]  =  (int)L'´';   
	
	pParams->aMAC_CharSet[0212]  =  (int)L'ä';   // adieresis
	
	pParams->aSTD_CharSet[0361]  =  (int)L'æ';   // ae
	pParams->aMAC_CharSet[0276]  =  (int)L'æ';   
	
	pParams->aMAC_CharSet[0210]  =  (int)L'à';   // agrave
	
	pParams->aMAC_CharSet[0214]  =  (int)L'å';   // aring
	
 	pParams->aMAC_CharSet[0213]  =  (int)L'ã';   // atilde
	
	pParams->aSTD_CharSet[0306]  =  (int)L'˘';   // breve
	pParams->aMAC_CharSet[0371]  =  (int)L'˘';   
	pParams->aPDF_CharSet[0030]  =  (int)L'˘';   
	
	pParams->aSTD_CharSet[0267]  =  (int)L'•';   // bullet
	pParams->aMAC_CharSet[0245]  =  (int)L'•';   
	pParams->aWIN_CharSet[0225]  =  (int)L'•';   
	pParams->aPDF_CharSet[0200]  =  (int)L'•';   
	
	pParams->aSTD_CharSet[0317]  =  (int)L'ˇ';   // caron
	pParams->aMAC_CharSet[0377]  =  (int)L'ˇ';   
	pParams->aPDF_CharSet[0031]  =  (int)L'ˇ';   
	
	pParams->aMAC_CharSet[0215]  =  (int)L'ç';   // ccedilla
	
	pParams->aSTD_CharSet[0313]  =  (int)L'¸';   // cedilla
 	pParams->aMAC_CharSet[0374]  =  (int)L'¸';   
	
	pParams->aSTD_CharSet[0303]  =  (int)L'ˆ';   // circumflex
	pParams->aMAC_CharSet[0366]  =  (int)L'ˆ';   // 
	pParams->aWIN_CharSet[0210]  =  (int)L'ˆ';   // 
	pParams->aPDF_CharSet[0032]  =  (int)L'ˆ';   // 
	
	pParams->aSTD_CharSet[0250]  =  (int)L'¤';   // currency
	pParams->aMAC_CharSet[0333]  =  (int)L'¤';   // 
	
	pParams->aSTD_CharSet[0262]  =  (int)L'†';   // dagger
	pParams->aMAC_CharSet[0240]  =  (int)L'†';   // 
	pParams->aWIN_CharSet[0206]  =  (int)L'†';   // 
	pParams->aPDF_CharSet[0201]  =  (int)L'†';   // 
	
	pParams->aSTD_CharSet[0263]  =  (int)L'‡';   // daggerdbl
	pParams->aMAC_CharSet[0340]  =  (int)L'‡';   // 
	pParams->aWIN_CharSet[0207]  =  (int)L'‡';   // 
	pParams->aPDF_CharSet[0202]  =  (int)L'‡';   // 
	
 	pParams->aMAC_CharSet[0241]  =  (int)L'°';   // degree
	
	pParams->aSTD_CharSet[0310]  =  (int)L'¨';   // dieresis
	pParams->aMAC_CharSet[0254]  =  (int)L'¨';   
	
	pParams->aMAC_CharSet[0326]  =  (int)L'÷';   // divide
	
	pParams->aSTD_CharSet[0307]  =  (int)L'˙';   // dotaccent
	pParams->aMAC_CharSet[0372]  =  (int)L'˙';   
	pParams->aPDF_CharSet[0033]  =  (int)L'˙';   
	
	pParams->aSTD_CharSet[0365]  =  (int)L'ı';   // dotlessi
	pParams->aMAC_CharSet[0365]  =  (int)L'ı';   
	pParams->aPDF_CharSet[0232]  =  (int)L'ı';  	
	
 	pParams->aMAC_CharSet[0216]  =  (int)L'é';   // eacute
	
	pParams->aMAC_CharSet[0220]  =  (int)L'ê';   // ecircumflex
	
	pParams->aMAC_CharSet[0221]  =  (int)L'ë';   // edieresis
	
	pParams->aMAC_CharSet[0217]  =  (int)L'è';   // egrave
	
	pParams->aSTD_CharSet[0274]  =  (int)L'…';   // ellipsis
	pParams->aMAC_CharSet[0311]  =  (int)L'…';   
	pParams->aWIN_CharSet[0205]  =  (int)L'…';   
	pParams->aPDF_CharSet[0203]  =  (int)L'…';   

	pParams->aSTD_CharSet[0320]  =  (int)L'—';   // emdash
	pParams->aMAC_CharSet[0321]  =  (int)L'—';   // emdash
	pParams->aWIN_CharSet[0227]  =  (int)L'—';   // emdash
	pParams->aPDF_CharSet[0204]  =  (int)L'—';   // emdash
	
	pParams->aSTD_CharSet[0261]  =  (int)L'–';   // endash
	pParams->aWIN_CharSet[0320]  =  (int)L'–';   // endash
	pParams->aMAC_CharSet[0226]  =  (int)L'–';   // endash
	pParams->aPDF_CharSet[0205]  =  (int)L'–';   // endash
	
	pParams->aMAC_CharSet[0301]  =  (int)L'¡';   // exclamdown
	
	pParams->aSTD_CharSet[0256]  =  (int)L'ﬁ';   // fi
	pParams->aMAC_CharSet[0336]  =  (int)L'ﬁ';   
	pParams->aPDF_CharSet[0223]  =  (int)L'ﬁ';   
	
	pParams->aSTD_CharSet[0257]  =  (int)L'ﬂ';   // fl
	pParams->aMAC_CharSet[0337]  =  (int)L'ﬂ';   
	pParams->aPDF_CharSet[0224]  =  (int)L'ﬂ';
	
	pParams->aSTD_CharSet[0246]  =  (int)L'ƒ';   // florin
	pParams->aMAC_CharSet[0304]  =  (int)L'ƒ';   //
	pParams->aWIN_CharSet[0203]  =  (int)L'ƒ';   //
	pParams->aPDF_CharSet[0206]  =  (int)L'ƒ';   //
	
	pParams->aSTD_CharSet[0244]  =  (int)L'⁄';   // fraction
	pParams->aMAC_CharSet[0332]  =  (int)L'⁄';   
	pParams->aPDF_CharSet[0207]  =  (int)L'⁄';  
	
	pParams->aSTD_CharSet[0373]  =  (int)L'ß';   // germandbls
	pParams->aMAC_CharSet[0247]  =  (int)L'ß';   
	
	pParams->aSTD_CharSet[0301]  =  (int)L'`';   // grave
	
	pParams->aSTD_CharSet[0253]  =  (int)L'«';   // guillemotleft
	pParams->aMAC_CharSet[0307]  =  (int)L'«';   //
	
	pParams->aSTD_CharSet[0273]  =  (int)L'»';   // guillemotright
	pParams->aMAC_CharSet[0310]  =  (int)L'»';   //
		
	pParams->aSTD_CharSet[0254]  =  (int)L'‹';   // guilsinglleft
	pParams->aMAC_CharSet[0334]  =  (int)L'‹';   // 
	pParams->aWIN_CharSet[0213]  =  (int)L'‹';   // 
	pParams->aPDF_CharSet[0210]  =  (int)L'‹';   // 
	
	pParams->aSTD_CharSet[0255]  =  (int)L'›';   // guilsinglright
	pParams->aMAC_CharSet[0335]  =  (int)L'›';   //
	pParams->aWIN_CharSet[0233]  =  (int)L'›';   //
	pParams->aPDF_CharSet[0211]  =  (int)L'›';   //

	pParams->aSTD_CharSet[0315]  =  0x02DD; //  (int)L'̋ '  // hungarumlaut
	pParams->aMAC_CharSet[0375]  =  0x02DD; //  (int)L'̋ '  
	pParams->aPDF_CharSet[0034]  =  0x02DD; //  (int)L'̋ '  	
				
	pParams->aMAC_CharSet[0222]  =  (int)L'í';   // iacute
	
	pParams->aMAC_CharSet[0224]  =  (int)L'î';   // icircumflex
	
	pParams->aMAC_CharSet[0225]  =  (int)L'ï';   // idieresis
	
	pParams->aMAC_CharSet[0223]  =  (int)L'ì';   // igrave
	
	pParams->aMAC_CharSet[0302]  =  (int)L'¬';   // logicalnot
	
	pParams->aSTD_CharSet[0370]  =  (int)L'ł';   // lslash
	pParams->aMAC_CharSet[0233]  =  (int)L'ł';   // 
	
	//pParams->aSTD_CharSet[0305]  =  0xC2AF; // (int)L'̄ '  // macron
	//pParams->aMAC_CharSet[0370]  =  0xC2AF; // (int)L'̄ '  
	
	pParams->aSTD_CharSet[0305]  =  0x00AF; // (int)L'̄ '  // macron
	pParams->aMAC_CharSet[0370]  =  0x00AF; // (int)L'̄ '  	
	
	pParams->aPDF_CharSet[0212]  =  (int)L'−';   // minus
	
	pParams->aMAC_CharSet[0226]  =  (int)L'ñ';   // ntilde
	
	pParams->aMAC_CharSet[0227]  =  (int)L'ó';   // oacute
	
	pParams->aMAC_CharSet[0231]  =  (int)L'ô';   // ocircumflex
	
	pParams->aMAC_CharSet[0232]  =  (int)L'ö';   // odieresis
	
	pParams->aSTD_CharSet[0372]  =  (int)L'œ';   // oe
	pParams->aMAC_CharSet[0317]  =  (int)L'œ';   
	
	pParams->aSTD_CharSet[0316]  =  (int)L'˛';   // ogonek
	pParams->aMAC_CharSet[0376]  =  (int)L'˛';   // 
	pParams->aPDF_CharSet[0035]  =  (int)L'˛';   // 
	
	pParams->aMAC_CharSet[0230]  =  (int)L'ò';   // ograve
	
	pParams->aSTD_CharSet[0343]  =  (int)L'ª';   // ordfeminine
	pParams->aMAC_CharSet[0273]  =  (int)L'ª';   //
	
	pParams->aSTD_CharSet[0353]  =  (int)L'º';   // ordmasculine
	pParams->aMAC_CharSet[0274]  =  (int)L'º';   //
	
	pParams->aSTD_CharSet[0371]  =  (int)L'ø';   // oslash
	pParams->aMAC_CharSet[0277]  =  (int)L'ø';   //	
	
	pParams->aMAC_CharSet[0233]  =  (int)L'õ';   // otilde
	
	pParams->aMAC_CharSet[0246]  =  (int)L'¶';   // paragraph
	
	pParams->aSTD_CharSet[0264]  =  (int)L'·';   // periodcentered
	pParams->aMAC_CharSet[0341]  =  (int)L'·';   //
	
	pParams->aSTD_CharSet[0275]  =  (int)L'‰';   // perthousand
	pParams->aMAC_CharSet[0344]  =  (int)L'‰';   // 
	pParams->aWIN_CharSet[0211]  =  (int)L'‰';   // 
	pParams->aPDF_CharSet[0213]  =  (int)L'‰';   // 
	
	pParams->aSTD_CharSet[0277]  =  (int)L'¿';   // questiondown
	pParams->aMAC_CharSet[0300]  =  (int)L'¿';   //

	pParams->aSTD_CharSet[0271]  =  (int)L'„';   // quotedblbase
	pParams->aMAC_CharSet[0343]  =  (int)L'„';   //
	pParams->aWIN_CharSet[0204]  =  (int)L'„';   //
	pParams->aPDF_CharSet[0214]  =  (int)L'„';   //
	
	pParams->aSTD_CharSet[0252]  =  (int)L'“';   // quotedblleft
	pParams->aMAC_CharSet[0322]  =  (int)L'“';   //
	pParams->aWIN_CharSet[0223]  =  (int)L'“';   //
	pParams->aPDF_CharSet[0215]  =  (int)L'“';   //	
	
	pParams->aSTD_CharSet[0272]  =  (int)L'”';   // quotedblright
	pParams->aMAC_CharSet[0323]  =  (int)L'”';   //
	pParams->aWIN_CharSet[0224]  =  (int)L'”';   //
	pParams->aPDF_CharSet[0216]  =  (int)L'”';   //	

	pParams->aSTD_CharSet[0140]  =  (int)L'‘';   // quoteleft
	pParams->aMAC_CharSet[0324]  =  (int)L'‘';   //
	pParams->aWIN_CharSet[0221]  =  (int)L'‘';   //
	pParams->aPDF_CharSet[0217]  =  (int)L'‘';   //	

	pParams->aSTD_CharSet[0047]  =  (int)L'’';   // quoteright
	pParams->aMAC_CharSet[0325]  =  (int)L'’';   //
	pParams->aWIN_CharSet[0222]  =  (int)L'’';   //
	pParams->aPDF_CharSet[0220]  =  (int)L'’';   //	

	pParams->aSTD_CharSet[0270]  =  (int)L'‚';   // quotesinglbase
	pParams->aMAC_CharSet[0342]  =  (int)L'‚';   //
	pParams->aWIN_CharSet[0202]  =  (int)L'‚';   //
	pParams->aPDF_CharSet[0221]  =  (int)L'‚';   //	

	pParams->aSTD_CharSet[0251]  =  (int)L'\'';   // quotesingle

	pParams->aMAC_CharSet[0250]  =  (int)L'®';   // registered

	pParams->aSTD_CharSet[0312]  =  (int)L'∘';   // ring
	pParams->aMAC_CharSet[0373]  =  (int)L'∘';   //
	pParams->aPDF_CharSet[0036]  =  (int)L'∘';   //	
	
	pParams->aWIN_CharSet[0232]  =  (int)L'š';   // scaron
	pParams->aPDF_CharSet[0235]  =  (int)L'š';   // 
	
	pParams->aMAC_CharSet[0244]  =  (int)L'§';   // section
	
	pParams->aSTD_CharSet[0304]  =  (int)L'~';   // tilde
	pParams->aMAC_CharSet[0367]  =  (int)L'~';   //
	pParams->aWIN_CharSet[0230]  =  (int)L'~';   //
	pParams->aPDF_CharSet[0037]  =  (int)L'~';   //	
	
	pParams->aMAC_CharSet[0252]  =  (int)L'™';   // trademark
	pParams->aWIN_CharSet[0231]  =  (int)L'™';   //
	pParams->aPDF_CharSet[0222]  =  (int)L'™';   //		

	pParams->aMAC_CharSet[0234]  =  (int)L'ú';   // uacute
	
	pParams->aMAC_CharSet[0236]  =  (int)L'û';   // ucircumflex
	
	pParams->aMAC_CharSet[0237]  =  (int)L'ü';   // udieresis

	pParams->aMAC_CharSet[0235]  =  (int)L'ù';   // ugrave
	
	pParams->aMAC_CharSet[0330]  =  (int)L'ÿ';   // ydieresis
	
	pParams->aMAC_CharSet[0264]  =  (int)L'¥';   // yen
}

void MakeMacExpertArrayCodes(Params *pParams)
{
	// http://www.fileformat.info/info/unicode/char/1d01/index.htm
	pParams->aMACEXP_CharSet[0276]  =  (int)L'ᴁ';   // AEsmall
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'ɶ';   // OEsmall
	
	/*
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	
	pParams->aMACEXP_CharSet[0]  =  (int)L'';   // 
	*/
}

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
	
	pParams->pUtf8String = NULL;
	
	pParams->pwszCurrentWord = NULL;
	pParams->pwszPreviousWord = NULL;
	
	pParams->nDictionaryType = DICTIONARY_TYPE_GENERIC;
	
	//pParams->pCurrentEncodingArray = &(pParams->aUtf8CharSet[0]);
	pParams->pCurrentEncodingArray = &(pParams->aSTD_CharSet[0]);
	//pParams->pCurrentEncodingArray = &(pParams->aWIN_CharSet[0]);
	//pParams->pCurrentEncodingArray = &(pParams->aMAC_CharSet[0]);
	//pParams->pCurrentEncodingArray = &(pParams->aPDF_CharSet[0]);
	
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
	
	#if !defined(MYDEBUG_PRINT_ALL) && !defined(MYDEBUG_PRINT_ON_ManageContent_PrintContent)
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
						zerr(ret);
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
					wprintf(L"\nPushXObjDecodedContent(XOBJ %d) -> FINE STREAM DECODIFICATO DOPO myInflate:\n", nXObjNumber);
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
								if ( FONT_SUBTYPE_Type0 == pParams->nCurrentFontSubtype )
								{
									#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
									wprintf(L"\tIl font corrente e' Type0. Per il momento non e' implementato.\n");
									#endif									
								}
								else
								{
									//wprintf(L"ERRORE ManageDecodedContent ParseFontObject.\n"); 
									fwprintf(pParams->fpErrors, L"ERRORE ManageDecodedContent ParseFontObject.\n"); 
									//wprintf(L"\n***** ECCO L'OGGETTO ERRATO:\n");
									fwprintf(pParams->fpErrors, L"\n***** ECCO L'OGGETTO ERRATO:\n");
								
									PrintThisObject(pParams, nTemp, 0, 0, pParams->fpErrors);
																
									//wprintf(L"\n***** FINE OGGETTO ERRATO\n");
									fwprintf(pParams->fpErrors, L"\n***** FINE OGGETTO ERRATO\n");
									retValue = 0;
									goto uscita;
								}
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
						zerr(ret);
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
							zerr(ret);
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
		
	wprintf(L"\n*****FINE OBJ(%d)\n\n", objNum);
	
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
	
	//if ( !contentxobj(pParams) )
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
					//if ( -1 == pParams->nCurrentPageResources )
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
							if ( strncmp(pParams->szCurrResourcesKeyName, "Font", 4096) == 0  )
								pParams->nCurrentFontsRef = iNum;
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
	}	
		
	mycontentqueuelist_Enqueue(&(pParams->pPagesArray[pParams->nCurrentPageNum].queueContens), &(pParams->CurrentContent));
	
uscita:
	
	mystringqueuelist_Free(&(pParams->CurrentContent.queueFilters));
	
	mydictionaryqueuelist_Free(&(pParams->CurrentContent.decodeParms));
	
	pParams->myDataDecodeParams.numFilter = 0;

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
						wprintf(L"ECCO, pagetreeobj -> metto in coda(pParams->myXObjRefList) l'XObjRef(Key = '%s') %d della pagina %d\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentPageNum);
						#endif						
						myobjreflist_Enqueue(&(pParams->myXObjRefList), pParams->szCurrResourcesKeyName, iNum);					
					}
					
					if ( pParams->bFontsKeys )
					{
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT) || defined(MYDEBUG_PRINT_ON_ManageContent_FN)
						//wprintf(L"ECCO, metto in coda(pParams->myFontsRefList) il FontsRef -> %s %d 0 R\n", pParams->szCurrResourcesKeyName, iNum);
						wprintf(L"ECCO, pagetreeobj -> metto in coda(pParams->myFontsRefList) il FontsRef(Key = '%s') %d della pagina %d\n", pParams->szCurrResourcesKeyName, iNum, pParams->nCurrentPageNum);
						#endif						
						myobjreflist_Enqueue(&(pParams->myFontsRefList), pParams->szCurrResourcesKeyName, iNum);					
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
	pParams->bCurrentFontHasDirectEncodingArray = 0;
	pParams->nCurrentEncodingObj = 0;
	pParams->szTemp[0] = '\0';
		
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
	
	if ( pParams->nCurrentEncodingObj > 0 )
	{				
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
				if ( pParams->aCustomizedFont_CharSet[k] == 0 )
				{
					pParams->aCustomizedFont_CharSet[k] = pParams->aWIN_CharSet[k];
				}
			}
		}
		else if ( strncmp(pParams->szTemp, "MacRomanEncoding", strnlen(pParams->szTemp, 4096) + 1) == 0 )
		{
			for ( k = 0; k < 256; k++ )
			{
				if ( pParams->aCustomizedFont_CharSet[k] == 0 )
					pParams->aCustomizedFont_CharSet[k] = pParams->aMAC_CharSet[k];
			}
		}	
		else if ( strncmp(pParams->szTemp, "MacExpertEncoding", strnlen(pParams->szTemp, 4096) + 1) == 0 )
		{
			for ( k = 0; k < 256; k++ )
			{
				if ( pParams->aCustomizedFont_CharSet[k] == 0 )
					pParams->aCustomizedFont_CharSet[k] = pParams->aMACEXP_CharSet[k];
			}
		}
		else
		{
			for ( k = 0; k < 256; k++ )
			{
				if ( pParams->aCustomizedFont_CharSet[k] == 0 )
					pParams->aCustomizedFont_CharSet[k] = pParams->aSTD_CharSet[k];
			}		
		}

		pParams->pCurrentEncodingArray = &(pParams->aCustomizedFont_CharSet[0]);		
	}
	else
	{
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
		wprintf(L"contentfontobj ENCODING -> %s\n", pParams->szTemp);
		#endif
		
		// MacRomanEncoding, MacExpertEncoding, or WinAnsiEncoding
		if ( strncmp(pParams->szTemp, "WinAnsiEncoding", strnlen("WinAnsiEncoding", 4096) + 1) == 0 )
		{
			pParams->pCurrentEncodingArray = &(pParams->aWIN_CharSet[0]);
		}
		else if ( strncmp(pParams->szTemp, "MacRomanEncoding", strnlen("MacRomanEncoding", 4096) + 1) == 0 )
		{
			pParams->pCurrentEncodingArray = &(pParams->aMAC_CharSet[0]);
		}
		else if ( strncmp(pParams->szTemp, "MacExpertEncoding", strnlen("MacExpertEncoding", 4096) + 1) == 0 )
		{
			pParams->pCurrentEncodingArray = &(pParams->aMACEXP_CharSet[0]);
		}
		else
		{
			pParams->pCurrentEncodingArray = &(pParams->aSTD_CharSet[0]);
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
					return 0; // NON IMPLEMENTATO, PER IL MOMENTO.
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
			pParams->aCustomizedFont_CharSet[keyValue] = nData;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
			wprintf(L"\tfontdirectencodingobjarray KEY -> '%s' -> Value = %d -> char = '%lc'\n", pParams->myToken.Value.vString, keyValue, (wchar_t)pParams->aCustomizedFont_CharSet[keyValue]);
			#endif			
		}
		else
		{
			//wprintf(L"encodingobjarray -> WARNING: KEY '%s' NON TROVATA\n", pParams->myToken.Value.vString);
			pParams->aCustomizedFont_CharSet[keyValue] = L' ';
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
				pParams->aCustomizedFont_CharSet[keyValue] = nData;
								
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_FONTOBJ)
				wprintf(L"\tfontdirectencodingobjarray KEY -> '%s' -> Value = %d -> char = '%lc'\n", pParams->myToken.Value.vString, keyValue, (wchar_t)pParams->aCustomizedFont_CharSet[keyValue]);
				#endif			
			}
			else
			{
				pParams->aCustomizedFont_CharSet[keyValue] = L' ';
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
	{
		pParams->aCustomizedFont_CharSet[k] = 0;
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
			if ( pParams->aCustomizedFont_CharSet[k] == 0 )
			{
				pParams->aCustomizedFont_CharSet[k] = pParams->aWIN_CharSet[k];
			}
		}
	}
	else if ( strncmp(pParams->szTemp, "MacRomanEncoding", strnlen(pParams->szTemp, 4096) + 1) == 0 )
	{
		for ( k = 0; k < 256; k++ )
		{
			if ( pParams->aCustomizedFont_CharSet[k] == 0 )
				pParams->aCustomizedFont_CharSet[k] = pParams->aMAC_CharSet[k];
		}
	}	
	else if ( strncmp(pParams->szTemp, "MacExpertEncoding", strnlen(pParams->szTemp, 4096) + 1) == 0 )
	{
		for ( k = 0; k < 256; k++ )
		{
			if ( pParams->aCustomizedFont_CharSet[k] == 0 )
				pParams->aCustomizedFont_CharSet[k] = pParams->aMACEXP_CharSet[k];
		}
	}
	else
	{
		for ( k = 0; k < 256; k++ )
		{
			if ( pParams->aCustomizedFont_CharSet[k] == 0 )
				pParams->aCustomizedFont_CharSet[k] = pParams->aSTD_CharSet[k];
		}		
	}

	pParams->pCurrentEncodingArray = &(pParams->aCustomizedFont_CharSet[0]);
	
		
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
			pParams->aCustomizedFont_CharSet[keyValue] = nData;
			
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
			wprintf(L"\tencodingobjarray KEY -> '%s' -> Value = %d -> char = '%lc'\n", pParams->myToken.Value.vString, keyValue, (wchar_t)pParams->aCustomizedFont_CharSet[keyValue]);
			#endif			
		}
		else
		{
			//wprintf(L"encodingobjarray -> WARNING: KEY '%s' NON TROVATA\n", pParams->myToken.Value.vString);
			pParams->aCustomizedFont_CharSet[keyValue] = L' ';
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
				pParams->aCustomizedFont_CharSet[keyValue] = nData;
								
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ)
				wprintf(L"\tencodingobjarray KEY -> '%s' -> Value = %d -> char = '%lc'\n", pParams->myToken.Value.vString, keyValue, (wchar_t)pParams->aCustomizedFont_CharSet[keyValue]);
				#endif			
			}
			else
			{
				//wprintf(L"encodingobjarray -> WARNING: KEY(%d) '%s' NON TROVATA\n", keyValue, pParams->myToken.Value.vString);
				pParams->aCustomizedFont_CharSet[keyValue] = L' ';
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

