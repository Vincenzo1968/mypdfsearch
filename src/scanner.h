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
#include "myTree.h"
#include "myobjrefqueuelist.h"
#include "mynumstacklist.h"
#include "myintqueuelist.h"
#include "mystringqueuelist.h"
#include "mydictionaryqueuelist.h"
#include "mycontentqueuelist.h"
#include "myScopeHashTable.h"
#include "myTernarySearchTree.h"
#include "mydecode.h"
#include "myGenHashTable.h"
#include "vlRedBlackTrees.h"
//#include "myInitPredefCMapHT.h"

#ifndef MAX_STRLEN
#define MAX_STRLEN 4096
#endif

#define MAX_STRING_LENTGTH_IN_CONTENT_STREAM 32767

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
#define FONT_SUBTYPE_Unknown        7

#define BASEFONT_NoPredef                 0
#define BASEFONT_Courier                  1
#define BASEFONT_Courier_Bold             2
#define BASEFONT_Courier_BoldOblique      3
#define BASEFONT_Courier_Oblique          4
#define BASEFONT_Helvetica                5
#define BASEFONT_Helvetica_Bold           6
#define BASEFONT_Helvetica_BoldOblique    7
#define BASEFONT_Helvetica_Oblique        8
#define BASEFONT_Symbol                   9
#define BASEFONT_Times_Bold              10
#define BASEFONT_Times_BoldItalic        11
#define BASEFONT_Times_Italic            12
#define BASEFONT_Times_Roman             13
#define BASEFONT_ZapfDingbats            14

#define DICTIONARY_TYPE_GENERIC     0
#define DICTIONARY_TYPE_RESOURCES   1
#define DICTIONARY_TYPE_XOBJ        2
#define DICTIONARY_TYPE_FONT        3
#define DICTIONARY_TYPE_GS          4

#define MACHINE_ENDIANNESS_UNKNOWN         0
#define MACHINE_ENDIANNESS_LITTLE_ENDIAN   1
#define MACHINE_ENDIANNESS_BIG_ENDIAN      2

#define STREAM_TYPE_GENERIC     0
#define STREAM_TYPE_CMAP        1
#define STREAM_TYPE_TOUNICODE   2

#define OBJ_TYPE_FREE       0
#define OBJ_TYPE_IN_USE     1
#define OBJ_TYPE_STREAM     2
#define OBJ_TYPE_RESERVED   3
#define OBJ_TYPE_UNKNOWN    4

#define TJSTACK_ITEMTYPE_STRING 1
#define TJSTACK_ITEMTYPE_NUMBER 2

#define WRITING_MODE_HORIZONTAL 0
#define WRITING_MODE_VERTICAL   1

// appearance order
#define SORT_GLYPHS_BY_ROW 1
#define SORT_GLYPHS_BY_APPEARANCE 2

typedef struct tagParams* ParamsPtr;
typedef unsigned char (*pfnReadNextChar)(ParamsPtr pParams);



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

	/* 23 */ T_LEFT_CURLY_BRACKET,   // '{'
	/* 24 */ T_RIGHT_CURLY_BRACKET,   // '}'
	
	/* 25 */ T_VOID_STRING,

	/* 26 */ T_CONTENT_LQUOTE,       // '<'
	/* 27 */ T_CONTENT_RQUOTE,       // '>'
	
	/* 28 */ T_CONTENT_OP_BI,   // 'BI'
	/* 29 */ T_CONTENT_OP_ID,   // 'ID'
	/* 30 */ T_CONTENT_OP_EI,   // 'EI'
	
	/* 31 */ T_CONTENT_OP_begincodespacerange,
	/* 32 */ T_CONTENT_OP_endcodespacerange,
	/* 33 */ T_CONTENT_OP_beginbfchar,
	/* 34 */ T_CONTENT_OP_endbfchar,
	/* 35 */ T_CONTENT_OP_beginbfrange,
	/* 36 */ T_CONTENT_OP_endbfrange,
	/* 37 */ T_CONTENT_OP_begincidchar,
	/* 38 */ T_CONTENT_OP_endcidchar,
	/* 39 */ T_CONTENT_OP_begincidrange,
	/* 40 */ T_CONTENT_OP_endcidrange,
	/* 41 */ T_CONTENT_OP_beginnotdefrange,
	/* 42 */ T_CONTENT_OP_endnotdefrange,
	/* 43 */ T_CONTENT_OP_beginnotdefchar,
	/* 44 */ T_CONTENT_OP_endnotdefchar,
	/* 45 */ T_CONTENT_OP_usecmap,
	
	/* 46 */ T_CONTENT_OP_BT,   // 'BT'   -> Begin Text
	/* 47 */ T_CONTENT_OP_ET,   // 'ET'   -> End Text
	/* 48 */ T_CONTENT_OP_TD,
	/* 49 */ T_CONTENT_OP_Td,
	/* 50 */ T_CONTENT_OP_Tm,
	/* 51 */ T_CONTENT_OP_TSTAR,   // 'T*'
	/* 52 */ T_CONTENT_OP_Tj,
	/* 53 */ T_CONTENT_OP_TJ,
	/* 54 */ T_CONTENT_OP_SINGLEQUOTE,   // '''
	/* 55 */ T_CONTENT_OP_DOUBLEQUOTE,   // '"'
	/* 56 */ T_CONTENT_OP_Tc,
	/* 57 */ T_CONTENT_OP_Tw,
	/* 58 */ T_CONTENT_OP_Tz,
	/* 59 */ T_CONTENT_OP_TL,
	/* 60 */ T_CONTENT_OP_Ts,
	/* 61 */ T_CONTENT_OP_Tr,
	/* 62 */ T_CONTENT_OP_Tf,
	/* 63 */ T_CONTENT_OP_Do,   // 'Do'

	/* 64 */ T_CONTENT_OP_cm,
	/* 65 */ T_CONTENT_OP_q,
	/* 66 */ T_CONTENT_OP_Q,
	/* 67 */ T_CONTENT_OP_MP,
	/* 68 */ T_CONTENT_OP_DP,
	/* 69 */ T_CONTENT_OP_BMC,
	/* 70 */ T_CONTENT_OP_BDC,
	/* 71 */ T_CONTENT_OP_EMC,
	
	/* 72 */ T_CONTENT_OP_BX,
	/* 73 */ T_CONTENT_OP_EX,

	/* 74 */ T_CONTENT_OP_w,
	/* 75 */ T_CONTENT_OP_J,
	/* 76 */ T_CONTENT_OP_j,
	/* 77 */ T_CONTENT_OP_M,
	/* 78 */ T_CONTENT_OP_d,
	/* 79 */ T_CONTENT_OP_ri,
	/* 80 */ T_CONTENT_OP_i,
	/* 81 */ T_CONTENT_OP_gs,
		
	/* 82 */ T_CONTENT_OP_m,
	/* 83 */ T_CONTENT_OP_l,
	/* 84 */ T_CONTENT_OP_c,
	/* 85 */ T_CONTENT_OP_v,
	/* 86 */ T_CONTENT_OP_y,
	/* 87 */ T_CONTENT_OP_h,
	/* 88 */ T_CONTENT_OP_re,

	/* 89 */ T_CONTENT_OP_S,
	/* 90 */ T_CONTENT_OP_s,
	/* 91 */ T_CONTENT_OP_f,
	/* 92 */ T_CONTENT_OP_F,
	/* 93 */ T_CONTENT_OP_fSTAR,   // 'f*'
	/* 94 */ T_CONTENT_OP_B,
	/* 95 */ T_CONTENT_OP_BSTAR,   // 'B*'
	/* 96 */ T_CONTENT_OP_b,		
	/* 97 */ T_CONTENT_OP_bSTAR,   // 'b*'
	/* 98 */ T_CONTENT_OP_n,
	
	/* 99 */ T_CONTENT_OP_W,		
	/* 100 */ T_CONTENT_OP_WSTAR,   // 'W*'
	
	/* 101 */ T_CONTENT_OP_d0,		
	/* 102 */ T_CONTENT_OP_d1,
		
	/* 103 */ T_CONTENT_OP_CS,
	/* 104 */ T_CONTENT_OP_cs,
	/* 105 */ T_CONTENT_OP_SC,
	/* 106 */ T_CONTENT_OP_SCN,
	/* 107 */ T_CONTENT_OP_sc,
	/* 108 */ T_CONTENT_OP_scn,
	/* 109 */ T_CONTENT_OP_G,
	/* 110 */ T_CONTENT_OP_g,		
	/* 111 */ T_CONTENT_OP_RG,
	/* 112 */ T_CONTENT_OP_rg,	
	/* 113 */ T_CONTENT_OP_K,		
	/* 114 */ T_CONTENT_OP_k,
	
	/* 115 */ T_CONTENT_OP_sh	
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
	//union tagValue
	//{
	char vChar;
	int vInt;
	double vDouble;
	char *vString;
	//} Value;
	
	//int line;
	//int column;
} Token;

typedef struct tagPdfIndirectObject 
{
	uint32_t Type;
	uint32_t Number;
	uint32_t Generation;
	uint32_t Offset;
	uint32_t StreamOffset;
	uint32_t StreamLength;
	unsigned char *pszDecodedStream;
	int32_t  numObjParent;    // Usato per gli oggetti di tipo 'Pages'(nodi interni del page tree) o 'Page'
	uint32_t genObjParent;    // Usato per gli oggetti di tipo 'Pages'(nodi interni del page tree) o 'Page'
	Tree *pTreeNode;
} PdfIndirectObject;

typedef struct tagTrailerIndex
{
	int FirstObjNumber;
	int NumberOfEntries;
} TrailerIndex;

typedef struct tagPdfTrailer
{
	int Size;
	int Prev;
	PdfIndirectObject Root;
	TrailerIndex **pIndexArray;
	int indexArraySize;
} PdfTrailer;

typedef struct tagTransMatrix
{
	double a; // Scaling sx
	double b; // Rotating counterclockwise by x radians about (b, 0)
	double c; // Rotating counterclockwise by x radians about (0, c)
	double d; // Scaling sy
	double e; // Translation dx
	double f; // Translation dy
} TransMatrix;

//typedef struct tagVerticalDataWriting
//{
//	double v;
//	double w1;
//} VerticalDataWriting;

typedef struct tagGlyphsWidths
{
	int FirstChar;
	int LastChar;
	int WidthsArraySize;
	double *pWidths;
	double MissingWidth;
	double AvgWidth;
	double MaxWidth;
	double dFontSpaceWidth;
	int WritingMode;
	double DW;  // Type0 Font only; Default Width.
	//VerticalDataWriting    DW2; // Type0 Font only; Default Width; Vertical writing.
} GlyphsWidths;

typedef struct tagTextState
{
	double Tc;
	double Tw;
	double Th;
	double Tl;
	double Tfs;
	double Trise;
} TextState;

typedef struct tagTJStackItem
{
	int     Type;
	double  dNum;
	wchar_t *pEncodingString;
	wchar_t *pUtf8String;
	size_t  lenUtf8String;
} TJStackItem;

typedef struct tagPdfObjsTableItem
{
	PdfIndirectObject Obj;
	MyObjRefList_t myXObjRefList;
	MyObjRefList_t myFontsRefList;
	MyObjRefList_t myGsRefList;
	MyIntQueueList_t queueContentsObjRefs; // Coda di riferimenti agli obj che contengono i contents(stream) della pagina
	// solo per oggetti font:
	GlyphsWidths *pGlyphsWidths; 
	unsigned char *pszDirectFontResourceString;
	uint32_t lenDirectFontResourceString;
	unsigned char *pszDirectGsResourceString;
	uint32_t lenDirectGsResourceString;
} PdfObjsTableItem;

typedef struct tagPage
{
	int32_t numObjNumber;
	int32_t numObjParent;
	Scope myScopeHT_XObjRef;
	Scope myScopeHT_FontsRef;
	Scope myScopeHT_GsRef;
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

typedef struct tagCodeSpaceRange
{
	uint32_t nFrom;
	uint32_t nTo;
	uint32_t nNumBytes;
} CodeSpaceRange_t;

typedef struct tagObjStm
{
	int N;
	int First;
	int Extend;
	unsigned char *pszDecodedStream;
	unsigned long int nDecodedStreamSize;
} ObjStm;

typedef struct tagMyPredefinedCMapDef
{
	char szUseCMap[128];
	unsigned char *pszDecodedStream;
	unsigned long int DecodedStreamSize;
} MyPredefinedCMapDef;

typedef struct tagParams* ParametersPtr;

typedef struct tag_vlrbtKey
{
	double row;
	double col;
	int32_t ord;
} vlrbtKey_t;

typedef struct tag_vlrbtData
{
	wchar_t c;
	double width;
	double widthScaled;
	double currFontSpaceWidth;
	double currFontSpaceWidthScaled;
	double currFontSize;
	
	int nCurrentFontSubtype;
	int nCurrentCIDFontSubtype;
	
	double xCoordNext;
	double yCoordNext;
	
	//Params *pParams;
	ParametersPtr pParams;
	
	vlrbtTreeNode *pCurrentNode;
	
	//wchar_t *pText;
	//uint32_t TextLength;
	
	//FILE *fpOutput;
	
	//double prevXCoordNext;
	//double prevYCoordNext;
	//wchar_t prevChar;
	//double prevRow;
	//double prevCol;
	//double prevWidth;
	//double prevWidthScaled;

} vlrbtData_t;

typedef struct tagParams
{
	Token myToken;
	Token myTokenLengthObj;
		
	int bCurrTokenIsOperator;
	
	pfnReadNextChar pReadNextChar;
	
	char szError[8192];
	
	GenHashTable_t myHT_ImageObjs;
	
	int bInternalDict;
	int bCidFontArrayInsteadDict;
	
	uint32_t bPdfHasText;
	//uint32_t nCountImageContent;
	//uint32_t nCountNotImageContent;
	
	uint32_t currentFileOffset;
	uint32_t lastTokenOffset;
	
	uint32_t nCurrentParsingObj;
	uint32_t nCurrentParsingLengthObj;
	
	uint32_t nCurrentPageParsingObj;
	
	MyNumStackList_t myCurrFontWidthsStack;
	int32_t nCurrFontDescriptorRef;
	char szBaseFont[1024];
	uint32_t nCurrentParsingFontObjNum;
	uint32_t nCurrentParsingCIDFontObjNum;
	uint32_t nCurrentParsingFontWidthsArrayRef;
	uint32_t nCurrentParsingFontDW2WidthsArrayRef;
	int bFontWidthsArrayState;
	int nCurrentBaseFont;
		
	
	
	vlRedBlackTree myRedBlackTree;
	vlrbtKey_t myRedBlackTreeKey;
	vlrbtData_t myRedBlackTreeData;
	
	wchar_t *pText;
	uint32_t TextLength;
	
	int bCurrFontSpaceWidthIsZero;
	
	wchar_t cLastChar;
	double dLastCharWidth;
	double Tj;
	double dCurrFontSpaceWidth;
	double dCurrFontSpaceWidthScaled;
	double dCurrFontAvgWidth;
	double dCurrFontMaxWidth;
	double dCurrFontAvgWidthScaled;
	double dCurrFontMaxWidthScaled;
	GlyphsWidths *pCurrFontGlyphsWidths;
	double xCoordPrev;
	double xCoordNextPrev;
	double yCoordNextPrev;
	TransMatrix dsTextMatrix;
	TransMatrix dsLineMatrix;
	TransMatrix dsRenderingMatrix;
	TextState dsTextState;
	TransMatrix dCTM_Stack[1024];
	int nCTM_StackTop;
	
	char szExtArraySizeCurrKeyName[MAX_STRLEN];
	//char szExtArraySizeDW2CurrKeyName[MAX_STRLEN];
	
	int bCurrParsingFontIsCIDFont;
	int bWisPresent;
	//int bW2isPresent;
	
	int nCurrFontWritingMode;
	
	uint32_t nDescendantFontRef;
	int bMakeDirectFontResourceString;
	int bMakeDirectGsResourceString;
	unsigned char *pszDirectFontResourceString;
	uint32_t lenDirectFontResourceString;
	char szDirectFontResourceName[MAX_STRLEN];
	unsigned char *pszDirectGsResourceString;
	uint32_t lenDirectGsResourceString;
	char szDirectGsResourceName[MAX_STRLEN];
	uint32_t nSizeDirectResourceString;
	
	double xCoordNext;
	
	int bParsingFontObj;
	int bParsingGsObj;
	
	uint32_t nCurrentGsObjFontObjNum;
	double   dCurrentGsObjFontSize;
		
	int nCotDictLevel;
	int nCotArrayLevel;
	uint32_t nCotObjType;
	
	int bOptVersionOrHelp;
	char szPath[MAX_LEN_STR + 1];	
	char szFilePdf[MAX_LEN_STR + 1];
	int fromPage;
	int toPage;	
	int bNoSubDirs;
	int nSortGlyphs;
	wchar_t szWordsToSearch[MAX_LEN_STR + 1];
	char szOutputFile[MAX_LEN_STR + 1];
	
	wchar_t **pWordsToSearchArray;
	int countWordsToSearch;
		
	char* lexeme;
	char* lexemeTemp;
	
	int nThisMachineEndianness;
	
	char szFileName[PATH_MAX];
	FILE *fp;
	FILE *fpLengthObjRef;
	
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
	
	
	unsigned char *myBlockToUnicode;
	int blockLenToUnicode;
	int blockCurPosToUnicode;
	
	
	int bStreamType;
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
	
	int bInGsObj;
	int bGsObjIsIndirect;
	int nGsObjRef;
	int bIsInGsObjState;	
	
	int nToUnicodeStreamObjRef;
	int bStreamStateToUnicode;
	
	int bReadingStringsFromDecodedStream;
	uint16_t myCID;
	int bReadingStringState;
	
	StreamsStack myStreamsStack[STREAMS_STACK_SIZE];
	int nStreamsStackTop;
	
	int nScope;
	
	int nDictionaryType;
		
	uint32_t nCurrentFontCodeSpacesNum;
	CodeSpaceRange_t *pCodeSpaceRangeArray;
	int bHasCodeSpaceOneByte;
	int bHasCodeSpaceTwoByte;
	
	// ------------------- ENCODING INIZIO ----------------------------
	
	GenHashTable_t myCMapHT;
	
	//GenHashTable_t myHT;
	TernarySearchTree_t myTST;
	
	int idxCurrentWordChar;
	int idxPreviousWordChar;
	wchar_t *pwszCurrentWord;
	wchar_t *pwszPreviousWord;	
	
	int bStateSillab;
	
	HashTable_t myCharSetHashTable;
		
	//wchar_t *pEncodingString;
	
	wchar_t *pUtf8String;
	size_t  lenUtf8String;
	
	wchar_t aUtf8CharSet[256];
	wchar_t aSTD_CharSet[256];
	wchar_t aMAC_CharSet[256];
	wchar_t aWIN_CharSet[256];
	wchar_t aPDF_CharSet[256];
	
	wchar_t aMACEXP_CharSet[256];
	
	wchar_t *paCustomizedFont_CharSet;
	int dimCustomizedFont_CharSet;
	
	wchar_t *pArrayUnicode;
	
	wchar_t *pCurrentEncodingArray;
	
	//GenHashTable_t myHT_EncodingArray;
	//unsigned char * pEncodingData;
	
	// ------------------- ENCODING FINE ----------------------------
		
	int isEncrypted;
	int dimFile;	
	
	PdfTrailer myPdfTrailer;
	int trailerW1;
	int trailerW2;
	int trailerW3;
	MyIntQueueList_t queueTrailerIndex;
	MyIntQueueList_t queueTrailerW;
	
	ObjStm currentObjStm;
	
	int nXRefStreamObjNum;
	uint32_t offsetXRefObjStream;
	int nCurrentTrailerIntegerNum;
	int bIsLastTrailer;
	
	PdfObjsTableItem **myObjsTable;
	uint32_t nObjsTableSizeFromPrescanFile;
	
	int nStackStringOpenParen;
	MyNumStackList_t myNumStack;
	char szCurrKeyName[MAX_STRLEN];
	char szCurrResourcesKeyName[MAX_STRLEN];
	PdfIndirectObject ObjPageTreeRoot;
	int nCountPagesFromPdf;
	MyIntQueueList_t myPagesQueue;
	Tree *pPagesTree;
	Page *pPagesArray;
	int eCurrentObjType;
	int nCurrentObjNum;
	int nObjToParse;
	int nCountPageFound;
	int nCountPageAlreadyDone;
	int nCurrentPageNum;
	int nPreviousPageNum;
	
	int nCurrentNumPageObjContent;   
	int bCurrentContentIsPresent;
	
	MyIntQueueList_t myPageLeafQueue;
	
	int nCurrentPageParent;      // 0 se nodo radice; altrimenti intero > 0 indica il nodo genitore della pagina corrente
	
	int nCurrentPageResources;   // 0 se la pagina non ha riferimenti a Resources; -1 se la pagina eredita Resources da uno dei suoi genitori; altrimenti un intero maggiore di zero che indica il riferimento al numero dell'oggetto Resources.
	
	int bXObjectKeys;
	int bFontsKeys;
	int bGsKeys;
	
	int bCurrentPageHasDirectResources;
	
	int bCurrentFontHasDirectEncodingArray;
	
	int bPrePageTreeExit;
	
	int nCurrentXObjResourcesRef;
	
	int nCurrentXObjRef;
	MyObjRefList_t myXObjRefList;
	char szTemp[MAX_STRLEN];
	int nTemp;
	
	int nCurrentFontsRef;
	MyObjRefList_t myFontsRefList;
	int nCurrentFontSubtype;
	int nCurrentCIDFontSubtype;
	
	int nCurrentGsRef;
	MyObjRefList_t myGsRefList;
	
	int nCurrentEncodingObj;
	
	int nCurrentUseCMapRef;
	char szUseCMap[MAX_STRLEN];
	int bEncodigArrayAlreadyInit;
	
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

int MakeAndOpenErrorsFile(Params *pParams, int bBom);
int myShowErrorMessage(Params *pParams, const char *szMsg, int bPrintToErrorFile);

unsigned char ReadNextChar(Params *pParams);

unsigned char ReadNextCharFromStmObjStream(Params *pParams);

void PrintTokenTrailer(Token *pToken, char cCarattereIniziale, char cCarattereFinale, int bPrintACapo);
int matchTrailer(Params *pParams, TokenTypeEnum ExpectedToken, char *pszFunctionName);

int trailerbody(Params *pParams);
int traileritems(Params *pParams);
int trailerobj(Params *pParams);
int trailerarrayobjs(Params *pParams);
int trailerdictobjs(Params *pParams);

int ReadTrailerBody(Params *pParams, unsigned char *szInput, int index);
int ReadSubSectionBody(Params *pParams, unsigned char *szInput, int fromNum, int numObjs, int *index);
int ReadLastTrailer(Params *pParams, unsigned char *szInput);
int GetLenNextInput(Params *pParams, int startxref, int *len);

int ManageTypeZeroHexString(Params *pParams, int lenCurrLexeme);

/* ------------------------------------------------------------------------------------------------------------- */

int ReadHeader(Params *pParams);

int ReadTrailer(Params *pParams);

unsigned char GetHexChar(unsigned char c1, unsigned char c2);

int IsDelimiterChar(unsigned char c);

int ReadObjsTable(Params *pParams, unsigned char *szInput);

void SetStringToken(Params *pParams, int k);

void GetNextToken(Params *pParams);

void GetNextTokenLengthObj(Params *pParams);

void GetNextTokenFromToUnicodeStream(Params *pParams);

#endif /* __MYSCANNER__ */
