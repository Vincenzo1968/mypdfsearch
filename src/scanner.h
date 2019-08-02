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

#ifndef __MYSCANNER__
#define __MYSCANNER__

#include "mypdfsearch.h"
#include "myobjrefqueuelist.h"
#include "mynumstacklist.h"
#include "myintqueuelist.h"
#include "mystringqueuelist.h"
#include "mydictionaryqueuelist.h"
#include "mycontentqueuelist.h"
#include "myScopeHashTable.h"
#include "myTernarySearchTree.h"
#include "mydecode.h"

#ifndef MAX_STRLEN
#define MAX_STRLEN 4096
#endif


#define STREAMS_STACK_SIZE    128

#define DELIM_FALSE           0
#define DELIM_SPACECHAR       1
#define DELIM_SPECIALSYMBOL   2

#define CHARSET_ENC_UTF8   0
#define CHARSET_ENC_STD    1
#define CHARSET_ENC_MAC    2 
#define CHARSET_ENC_WIN    3 
#define CHARSET_ENC_PDF    4 

#define FONT_SUBTYPE_Type0          0
#define FONT_SUBTYPE_Type1          1
#define FONT_SUBTYPE_MMType1        2
#define FONT_SUBTYPE_Type3          3
#define FONT_SUBTYPE_TrueType       4 
#define FONT_SUBTYPE_CIDFontType0   5
#define FONT_SUBTYPE_CIDFontType2   6

#define DICTIONARY_TYPE_GENERIC     0
#define DICTIONARY_TYPE_RESOURCES   1
#define DICTIONARY_TYPE_XOBJ        2
#define DICTIONARY_TYPE_FONT        3

typedef enum tagTokenType
{
	/*  0 */ T_ERROR = 0,
	/*  1 */ T_UNKNOWN,
	/*  2 */ T_EOF,
	/*  3 */ T_STRING,
	/*  4 */ T_STRING_LITERAL,
	/*  5 */ T_STRING_HEXADECIMAL,
	/*  6 */ T_NAME,
	/*  7 */ T_INT_LITERAL,
	/*  8 */ T_REAL_LITERAL,
	/*  9 */ T_OPAREN,		// '('
	/* 10 */ T_CPAREN,		// ')'
	/* 11 */ T_QOPAREN,		// '['
	/* 12 */ T_QCPAREN,		// ']'
	/* 13 */ T_DICT_BEGIN,   // '<<'
	/* 14 */ T_DICT_END,     // '>>'
	/* 15 */ T_KW_NULL,
	/* 16 */ T_KW_OBJ,
	/* 17 */ T_KW_ENDOBJ,
	/* 18 */ T_KW_STREAM,
	/* 19 */ T_KW_ENDSTREAM,
	/* 20 */ T_KW_FALSE,
	/* 21 */ T_KW_TRUE,
	/* 22 */ T_KW_R, 			     // 'R'
	/* 23 */ T_CONTENT_LQUOTE,       // '<'
	/* 24 */ T_CONTENT_RQUOTE,       // '>'
	/* 25 */ T_CONTENT_Do_COMMAND,   // 'Do'
	/* 26 */ T_CONTENT_KW_BT,        // 'BT'   -> Begin Text
	/* 27 */ T_CONTENT_KW_ET,        // 'ET'   -> End Text
	/* 28 */ T_CONTENT_OP_TD,
	/* 29 */ T_CONTENT_OP_Td,
	/* 30 */ T_CONTENT_OP_Tm,
	/* 31 */ T_CONTENT_OP_TASTERISCO,
	/* 32 */ T_CONTENT_OP_Tj,
	/* 33 */ T_CONTENT_OP_TJ,
	/* 34 */ T_CONTENT_OP_SINGLEQUOTE,   // '''
	/* 35 */ T_CONTENT_OP_DOUBLEQUOTE,    // '"'
	/* 36 */ T_CONTENT_OP_Tc,
	/* 37 */ T_CONTENT_OP_Tw,
	/* 38 */ T_CONTENT_OP_Tf,
	/* 39 */ T_VOID_STRING,
	/* 40 */ T_CONTENT_OP_BI   // 'BI'   -> Begin Image, ignoriamo tutto ciò che segue
}TokenTypeEnum;

typedef enum tagTrailerStates
{
	S_Error,
	S_Size,
	S_Prev,	
	S_Root,
	S_ID,
	S_Info
} TrailerStates;

typedef enum tagObjsTypes
{
	OBJ_TYPE_GENERIC,
	OBJ_TYPE_PAGES,
	OBJ_TYPE_PAGE
} ObjsTipes;

typedef struct tagToken
{
	TokenTypeEnum Type;
	union tagValue
	{
		char vChar;
		int vInt;
		double vDouble;
		char *vString;
	} Value;
	
	//int line;
	//int column;
} Token;

typedef struct tagPdfIndirectObject 
{
	int Number;
	int Generation;
} PdfIndirectObject;

typedef struct tagPdfTrailer
{
	int Size;
	int Prev;
	PdfIndirectObject Root;
} PdfTrailer;

typedef struct tagPdfObjsTableItem
{
	PdfIndirectObject Obj;
	int Offset;
	int numObjParent;   // Usato per gli oggetti di tipo Pages (nodi interni del page tree)
	MyObjRefList_t myXObjRefList;
	MyObjRefList_t myFontsRefList;
} PdfObjsTableItem;

typedef struct tagPage
{
	int numObjNumber;
	int numObjParent;
	Scope myScopeHT_XObjRef;
	Scope myScopeHT_FontsRef;
	int numObjContent;   
	int bContentIsPresent;
	MyIntQueueList_t queueContentsObjRefs; // Coda di riferimenti agli obj che contengono i contents(stream) della pagina
	MyContentQueueList_t queueContens;     // Coda di content della pagina. Ogni elemento della coda contiene l'offset dello stream e la sua lunghezza, non lo stream vero e proprio.
} Page;

typedef struct tagStreamsStack
{
	int blockLen;
	int blockCurPos;		
	int bStreamState;
	int bStringIsDecoded;
	unsigned char *pszDecodedStream;
	unsigned long int DecodedStreamSize;
} StreamsStack;

typedef struct tagParams
{
	Token myToken;
	Token myTokenLengthObj;
	
	int bOptVersionOrHelp;
	char szPath[MAX_LEN_STR + 1];	
	char szFilePdf[MAX_LEN_STR + 1];
	int fromPage;
	int toPage;	
	char szWordsToSearch[MAX_LEN_STR + 1];
	wchar_t *pwszWordsToSearch;
	char szOutputFile[MAX_LEN_STR + 1];
	
	wchar_t **pWordsToSearchArray;
	int countWordsToSearch;
		
	char* lexeme;
	
	char szFileName[PATH_MAX];
	FILE *fp;
	FILE *fpLengthObjRef;
	
	//char *szOutputFileName[PATH_MAX];
	FILE *fpOutput;
	
	FILE *fpErrors;
	
	char szPdfHeader[128];
	char szPdfVersion[128];
	char szPdfVersionFromCatalog[128];
	
	int countArrayScope;
	
	unsigned char *myBlock;
	int blockLen;
	int blockCurPos;
		
	unsigned char *myBlockLengthObj;
	int blockLenLengthObj;
	int blockCurPosLengthObj;
	
	int bStreamState;
	int bStringIsDecoded;
	
	int bInXObj;
	int bXObjIsIndirect;
	int nXObjRef;
	int bIsInXObjState;
	int bXObjIsImage;
	
	int bInFontObj;
	int bFontObjIsIndirect;
	int nFontObjRef;
	int bIsInFontObjState;	
	
	StreamsStack myStreamsStack[STREAMS_STACK_SIZE];
	int nStreamsStackTop;
	
	int nScope;
	
	int nDictionaryType;
	
	// ------------------- ENCODING INIZIO ----------------------------
	
	TernarySearchTree_t myTST;
	
	int idxCurrentWordChar;
	int idxPreviousWordChar;
	wchar_t *pwszCurrentWord;
	wchar_t *pwszPreviousWord;	
	
	int bStateSillab;
	
	
	HashTable_t myCharSetHashTable;
		
	wchar_t *pUtf8String;
	
	wchar_t aUtf8CharSet[256];
	wchar_t aSTD_CharSet[256];
	wchar_t aMAC_CharSet[256];
	wchar_t aWIN_CharSet[256];
	wchar_t aPDF_CharSet[256];
	
	wchar_t aMACEXP_CharSet[256];
	
	wchar_t aCustomizedFont_CharSet[256];
	
	wchar_t *pCurrentEncodingArray;
	
	// ------------------- ENCODING FINE ----------------------------
		
	int isEncrypted;
	int dimFile;	
	PdfTrailer myPdfTrailer;
	PdfObjsTableItem **myObjsTable;
	int nStackStringOpenParen;
	MyNumStackList_t myNumStack;
	char szCurrKeyName[MAX_STRLEN];
	char szCurrResourcesKeyName[MAX_STRLEN];
	PdfIndirectObject ObjPageTreeRoot;
	int nCountPagesFromPdf;
	MyIntQueueList_t myPagesQueue;
	Page *pPagesArray;
	int eCurrentObjType;
	int nCurrentObjNum;
	int nObjToParse;
	int nCountPageFound;
	int nCountPageAlreadyDone;
	int nCurrentPageNum;
	
	int nCurrentPageParent;      // 0 se nodo radice; altrimenti intero > 0 indica il nodo genitore della pagina corrente
	
	int nCurrentPageResources;   // 0 se la pagina non ha riferimenti a Resources; -1 se la pagina eredita Resources da uno dei suoi genitori; altrimenti un intero maggiore di zero che indica il riferimento al numero dell'oggetto Resources.
	
	int bXObjectKeys;
	int bFontsKeys;
	
	int bCurrentPageHasDirectResources;
	
	int bCurrentFontHasDirectEncodingArray;
	
	int nCurrentXObjRef;
	MyObjRefList_t myXObjRefList;
	char szTemp[MAX_STRLEN];
	int nTemp;
	
	
	int nCurrentFontsRef;
	MyObjRefList_t myFontsRefList;
	int nCurrentFontSubtype;
	
	int nCurrentEncodingObj;
	
	unsigned long int nNumBytesReadFromCurrentStream;
	int bUpdateNumBytesReadFromCurrentStream;
	MyContent_t CurrentContent;
	int nCurrentStreamLenghtFromObjNum;
	int bStreamLengthIsPresent;
	MyData_t myDataDecodeParams;
	int nCountDecodeParams;
	int nCountFilters;
} Params;

typedef enum tagStates
{
	S0,
	S1,
	S2,
	S3,
	S4,
	S5,
	S6,
	S7,
	S8,
	S9,
	S10,
	S11,
	S12,
	S13,
	S14,
	S15,
	S16,
	S17,
	S18,
	S19,
	S20,
	S21
} States;

/* ------------------------------------------------------------------------------------------------------------- */

int ReadTrailerBody(Params *pParams, unsigned char *szInput, int index);
int ReadSubSectionBody(Params *pParams, unsigned char *szInput, int fromNum, int numObjs, int *index);
int ReadLastTrailer(Params *pParams, unsigned char *szInput);
int GetLenNextInput(Params *pParams, int startxref, int *len);

/* ------------------------------------------------------------------------------------------------------------- */

int ReadHeader(Params *pParams);

int ReadTrailer(Params *pParams);

unsigned char GetHexChar(unsigned char c1, unsigned char c2);

int IsDelimiterChar(unsigned char c);

int ReadObjsTable(Params *pParams, unsigned char *szInput, int bIsLastTrailer);

void GetNextToken(Params *pParams);

void GetNextTokenLengthObj(Params *pParams);

#endif /* __MYSCANNER__ */