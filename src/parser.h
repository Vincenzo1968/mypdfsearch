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

int OnTraverseTST(const wchar_t* key, void* data, uint32_t dataSize);

int InsertWordIntoTst(Params *pParams);

int InitializeCharSetHashTable(Params *pParams);
int InitializeUnicodeArray(Params *pParams);
void InitializeCharSetArrays(Params *pParams);
void MakeDifferencesArrayCodes(Params *pParams);
void MakeMacExpertArrayCodes(Params *pParams);

void PrintToken(Token *pToken, char cCarattereIniziale, char cCarattereFinale, int bPrintACapo);
int PrintThisObject(Params *pParams, int objNum, int bDecodeStream, int nPageNumber, FILE* fpErrors);

int ParseObject(Params *pParams, int objNum);
int ParseNextObject(Params *pParams, int objNum);
int ParseStreamObject(Params *pParams, int objNum);
int ParseStreamXObject(Params *pParams, int objNum);
int ParseToUnicodeStream(Params *pParams, int objNum, unsigned char *pszDecodedStream, unsigned long int DecodedStreamSize);
int ParseToUnicodeObject(Params *pParams, int objNum);
int ParseFontObject(Params *pParams, int objNum);
int ParseEncodingObject(Params *pParams, int objNum);
int ParseDictionaryObject(Params *pParams, int objNum);
int ParseLengthObject(Params *pParams, int objNum);

int PushXObjDecodedContent(Params *pParams, int nPageNumber, int nXObjNumber);
int ManageDecodedContent(Params *pParams, int nPageNumber);
int ManageContent(Params *pParams, int nPageNumber);


int Parse(Params *pParams, FilesList* myFilesList, int bPrintObjsAndExit);


int match(Params *pParams, TokenTypeEnum ExpectedToken, char *pszFunctionName);
int matchLengthObj(Params *pParams, TokenTypeEnum ExpectedToken, char *pszFunctionName);


// void Panic();


int pdf(Params *pParams);

int obj(Params *pParams);
int objbody(Params *pParams);
int objitem(Params *pParams);
int array(Params *pParams);
int dictionary(Params *pParams);

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

#endif /* __MYPARSER__ */
