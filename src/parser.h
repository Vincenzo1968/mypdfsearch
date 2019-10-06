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

#ifndef __MYPARSER__
#define __MYPARSER__

#include "mypdfsearch.h"
#include "scanner.h"

typedef enum tagPreParseStates
{
	S_PPError,
	S_PP0,
	S_PP1,
	S_PP2,	
	S_PP3,
	S_PP4,
	S_PP5,
	S_PP6,
	S_PP7,
	S_PP8,
	S_PP9,
	S_PP10,
	S_PP11,
	S_PP12,
	S_PP13,
	S_PP14,
	S_PP15,
	S_PP16,
	S_PP17,
	S_PP18,
	S_PP19,
	S_PP20,
	S_PP21,
	S_PP22,
	S_PP23,
	S_PP24,
	S_PP25,
	S_PP26,
	S_PP27,
	S_PP28,
	S_PP29,
	S_PP30,
	S_PP31,
	S_PP32
} PreParseStates;

int getFontDataHT(Params *pParams, uint32_t objNum);
int insertFontDataHT(Params *pParams, uint32_t objNum);

unsigned char * getDecodedStream(Params *pParams, unsigned long int *pDecodedStreamSize, MyContent_t *pContent);

int OnTraverseTST(const wchar_t* key, void* data, uint32_t dataSize);

int InsertWordIntoTst(Params *pParams);

int InitializeCharSetHashTable(Params *pParams);
int InitializeUnicodeArray(Params *pParams);
void InitializeCharSetArrays(Params *pParams);
void MakeDifferencesArrayCodes(Params *pParams);
void MakeMacExpertArrayCodes(Params *pParams);

int myPrintLastBlock(Params *pParams);
void PrintToken(Token *pToken, char cCarattereIniziale, char cCarattereFinale, int bPrintACapo);
int PrintThisObject(Params *pParams, int objNum, int bDecodeStream, int nPageNumber, FILE* fpErrors);

int LoadFirstBlock(Params *pParams, int objNum, const char *pszFunctionName);

void myTreeTraversePostOrderLeafOnly(Tree *head, Params *pParams);
int ParseObject(Params *pParams, int objNum);
int ParseNextObject(Params *pParams, int objNum);
int CheckObjectType(Params *pParams, int objNum);
int getObjStmDataFromDecodedStream(Params *pParams);
int ParseStmObj(Params *pParams, int objNum);
int ParseTrailerXRefStreamObject(Params *pParams);
int ParseStreamObject(Params *pParams, int objNum);
int ParseStreamXObject(Params *pParams, int objNum);
int ParseCMapStream(Params *pParams, int objNum, unsigned char *pszDecodedStream, unsigned long int DecodedStreamSize);
int ParseToUnicodeStream(Params *pParams, int objNum, unsigned char *pszDecodedStream, unsigned long int DecodedStreamSize);
int ParseCMapObject(Params *pParams, int objNum);
int ParseFontObject(Params *pParams, int objNum);
int ParseEncodingObject(Params *pParams, int objNum);
int ParseDictionaryObject(Params *pParams, int objNum);
int ParseLengthObject(Params *pParams, int objNum);
int ParseIntegerObject(Params *pParams, int objNum);

int PushXObjDecodedContent(Params *pParams, int nPageNumber, int nXObjNumber);
int ManageDecodedContent(Params *pParams, int nPageNumber);
int ManageContent(Params *pParams, int nPageNumber);

int getObjsOffsets(Params *pParams, char *pszFileName); // preparse file

int Parse(Params *pParams, FilesList* myFilesList);

int match(Params *pParams, TokenTypeEnum ExpectedToken, char *pszFunctionName);
int matchLengthObj(Params *pParams, TokenTypeEnum ExpectedToken, char *pszFunctionName);


// void Panic();


int pdf(Params *pParams);

int obj(Params *pParams);
int objbody(Params *pParams);
int objitem(Params *pParams);
int array(Params *pParams);
int dictionary(Params *pParams);

int xrefstream_obj(Params *pParams);
int xrefstream_objbody(Params *pParams);
int xrefstream_streamdictitems(Params *pParams);
int xrefstream_keyvalue(Params *pParams);
int xrefstream_keyarray(Params *pParams);
int xrefstream_keydict(Params *pParams);
int xrefstream_keyvalueinternal(Params *pParams);
int xrefstream_keyarrayinternal(Params *pParams);

int stmobj(Params *pParams);
int stmobjbody(Params *pParams);
int stmobjstreamdictitems(Params *pParams);
int stmobjkeyvalue(Params *pParams);
int stmobjkeyarray(Params *pParams);
int stmobjkeydict(Params *pParams);
int stmobjkeyvalueinternal(Params *pParams);
int stmobjkeyarrayinternal(Params *pParams);

int prepagetree(Params *pParams);
int pagetree(Params *pParams);
int pagetreebody(Params *pParams);
int pagetreeitems(Params *pParams);
int pagetreeobj(Params *pParams);
int pagetreearrayobjs(Params *pParams);
int pagetreedictobjs(Params *pParams);

int contentobj(Params *pParams);
int contentobjbody(Params *pParams);
int streamdictitems(Params *pParams);
int contentkeyvalue(Params *pParams);
int contentkeyarray(Params *pParams);
int contentkeydict(Params *pParams);
int contentkeyvalueinternal(Params *pParams);
int contentkeyarrayinternal(Params *pParams);

int lengthobj(Params *pParams);
int integer_obj(Params *pParams);

int resourcesdictionary(Params *pParams);
int resourcesdictionarybody(Params *pParams);
int resourcesdictionaryitems(Params *pParams);
int resourcesarrayobjs(Params *pParams);
int resourcesdictobjs(Params *pParams);

int contentxobj(Params *pParams);
int contentxobjbody(Params *pParams);
int xobjstreamdictitems(Params *pParams);
int xobjcontentkeyvalue(Params *pParams);
int xobjcontentkeyarray(Params *pParams);
int xobjcontentkeydict(Params *pParams);

int contentfontobj(Params *pParams);
int contentfontobjbody(Params *pParams);
int fontobjstreamdictitems(Params *pParams);
int fontobjcontentkeyvalue(Params *pParams);
int fontdirectencodingobjarray(Params *pParams);
int fontobjcontentkeyarray(Params *pParams);
int fontobjcontentkeydict(Params *pParams);

int encodingobj(Params *pParams);
int encodingobjbody(Params *pParams);
int encodingobjdictitems(Params *pParams);
int encodingobjdictitemskeyvalues(Params *pParams);
int encodingobjarray(Params *pParams);

int cot(Params *pParams);
int cot_dictbody(Params *pParams);
int cot_dictbodyitems(Params *pParams);

#endif /* __MYPARSER__ */
