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

// https://www.adobe.com/content/dam/acom/en/devnet/pdf/pdfs/PDF32000_2008.pdf

// http://www.eprg.org/pdfcorner/text2pdf/



#ifndef __MYPDFSEARCH__
#define __MYPDFSEARCH__

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <wctype.h>
#include <stdint.h>

//#define HT_SIZE 8191
#define BLOCK_SIZE 4096
//#define BLOCK_SIZE 4096 * 21

#ifndef MAX_LEN_STR
	#define MAX_LEN_STR 1024
#endif

#ifndef MAX_WORDS_TO_SEARCH
	#define MAX_WORDS_TO_SEARCH 100
#endif

#define UNUSED(x) (void)(x)


typedef struct tagFilesList
{
	#if !defined(_WIN64) && !defined(_WIN32)
	char myPathName[PATH_MAX + 1];
	#endif
	char myFileName[PATH_MAX + 1];
	//char relativeName[PATH_MAX + 1];
	struct tagFilesList *next;
} FilesList;

#endif /* __MYPDFSEARCH__ */


/*
MYDEBUG_PRINT_SINGLE_CHAR_ENCODING
MYDEBUG_PRINT_SINGLE_CHAR_COORD

MYDEBUG_PRINT_ON_SHOW_COORDINATE_STRING
MYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR
MYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR
MYDEBUG_PRINT_ON_TEXT_STATE_OPERATOR
MYDEBUG_PRINT_ON_CTM_OPERATOR

MYPDFSEARCH_SHOW_TIME
MYDEBUG_PRINT_COUNT_CONTENT_TYPE
MYDEBUG_PRINT_DECODED_CONTENT_TOKENS
MYDEBUG_PRINT_DECODED_CONTENT_NUMSTACK

MYDEBUG_PRINT_ALL
MYDEBUG_PRINT_ON_MATCH
MYDEBUG_PRINT_ON_PARSE_OBJ
MYDEBUG_PRINT_ON_PARSE_XREF_STREAMOBJ
MYDEBUG_PRINT_ON_PARSE_STMOBJ
MYDEBUG_PRINT_ON_PARSE_STREAMOBJ
MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ
MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM
MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM
MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ
MYDEBUG_PRINT_ON_PARSE_LENGTHOBJ
MYDEBUG_PRINT_ON_PARSE_INTEGEROBJ
MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT
MYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ
MYDEBUG_PRINT_ON_PARSE_FONTOBJ
MYDEBUG_PRINT_ON_PARSE_FONTOBJ_WIDTHS
MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ
MYDEBUG_PRINT_ON_PARSE_COT

MYDEBUG_PRINT_TST
 
MYDEBUG_PRINT_ON_GetNextToken_FN
 
MYDEBUG_PRINT_ON_getObjsOffsets_FN
 
MYDEBUG_PRINT_ON_ReadHeader_FN
MYDEBUG_PRINT_ON_ReadTrailer_FN
MYDEBUG_PRINT_ON_ReadTrailer_OBJ
 
MYDEBUG_PRINT_ON_ManageDecodeParams_FN
MYDEBUG_PRINT_ON_ParseTrailerXRefStreamObject_FN

MYDEBUG_PRINT_ON_PARSE_FN
MYDEBUG_PRINT_ON_ParseStmObj_FN
MYDEBUG_PRINT_ON_ManageContent_FN
MYDEBUG_PRINT_ON_ManageContent_FN_ShowResourceSelected
MYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected
MYDEBUG_PRINT_ON_ManageContent_FN_ShowGsSelected
MYDEBUG_PRINT_ON_ManageContent_PrintPageNum
MYDEBUG_PRINT_ON_ManageContent_PrintContent
MYDEBUG_PRINT_ON_ManageContent_PrintContent_HEXCODECHAR
MYDEBUG_PRINT_ON_ManageContent_PrintStrings
MYDEBUG_PRINT_ON_ManageContent_PrintStrings_HEXADECIMAL
MYDEBUG_PRINT_ON_ParseObject_FN
MYDEBUG_PRINT_ON_myTreeTraversePostOrderLeafOnly_FN
MYDEBUG_PRINT_ON_ParseStreamObject_FN
MYDEBUG_PRINT_ON_ParseDictionaryObject_FN
MYDEBUG_PRINT_ON_ParseLengthObject_FN
*/


/*
gcc -Wall -W -pedantic -Wno-overlength-strings -O3 -std=c99 -D_GNU_SOURCE mytime.c myTree.c vlRedBlackTrees.c myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz -lm

gcc -Wall -W -pedantic -Wno-overlength-strings -O3 -std=c99 -D_GNU_SOURCE mytime.c myTree.c vlRedBlackTrees.c myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz -lm -DMYDEBUG_PRINT_ON_getObjsOffsets_FN 
*/



/*
git clone https://github.com/Vincenzo1968/mypdfsearch

https://vlcfreecode.netsons.org/wp-admin/
------------------------------------------------------------------------------------------------------------------------------------
 
./mypdfsearch --extracttextfrom="../Files/Giapponesi/SoloGiapponese/kk190531a.pdf" --frompage=1 --topage=1
 
./mypdfsearch --extracttextfrom="../Files/Giapponesi/SoloGiapponese/ohome.pdf" --frompage=1 --topage=1

./mypdfsearch --extracttextfrom="../Files/Giapponesi/SoloGiapponese/VerticalWriting.pdf" --frompage=2 --topage=2

 
 
./mypdfsearch --extracttextfrom="../Files/FileProblematico2/corriere_della_sera_-_03_gennaio_2018.pdf" --frompage=17 --topage=17



./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan01.pdf" --frompage=5 --topage=5
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan03.pdf" --frompage=9 --topage=9
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan04.pdf" --frompage=6 --topage=6
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan05.pdf" --frompage=5 --topage=5
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan06.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan07.pdf" --frompage=21 --topage=21
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan08.pdf" --frompage=24 --topage=24
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan09.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan11.pdf" --frompage=10 --topage=10
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan12.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan13.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan14.pdf" --frompage=3 --topage=3
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan15.pdf" --frompage=1 --topage=1

./mypdfsearch --path="../Files/gccFiles/Giornali/Manzoni/manzoni_i_promessi_sposi.pdf" --words="Virginia Orbán branco"

------------------------------------------------------------------------------------------------------------------------------------
*/

/*
./mypdfsearch --path="../Files/gccFiles/Giornali" --words="Virginia Orbán branco"
 
------------------------------------------------------------------------------------------------------------------------------------
CON TERNARY SEARCH TREE:

Tempo impiegato(in secondi): 
Elapsed time (in seconds): 
Real Time  : 73.83000
User Time  : 69.69000
System Time: 4.12000
 
Tempo impiegato(in secondi): 
Elapsed time (in seconds): 
Real Time  : 66.23000
User Time  : 65.05000
System Time: 1.17000
-------------------------------------------------------------------------------------------------------------------------------------
CON HASHTABLE:
 
Tempo impiegato(in secondi): 
Elapsed time (in seconds): 
Real Time  : 66.21000
User Time  : 65.63000
System Time: 0.57000

------------------------------------------------------------------------------------------------------------------------------------

time pdfgrep -i --with-filename -r -c 'Virginia|Orbán|branco' ../Files/gccFiles/Giornali

real	1m53,395s
user	1m50,817s
sys	0m2,461s

real	1m53,354s
user	1m51,021s
sys	0m2,332s

real	1m53,009s
user	1m50,873s
sys	0m2,136s

-------------------------------------------------------------------------------------------------------------------------------------

time ./mypdfsearch --path="../Files/gccFiles/Giornali" --words="Virginia Orbán branco"

time mypdfsearch --path="../Files/gccFiles/Giornali" --words="Virginia Orbán branco"

real	0m55,380s
user	0m49,691s
sys	0m1,365s

real	0m50,096s
user	0m48,573s
sys	0m1,219s

real	0m49,773s
user	0m48,472s
sys	0m1,300s

-------------------------------------------------------------------------------------------------------------------------------------

OLD (version 1.9.3):
real	1m13,360s
user	1m7,520s
sys	0m1,470s

real	1m8,006s
user	1m6,876s
sys	0m1,128s

real	1m7,952s
user	1m6,782s
sys	0m1,164s

-------------------------------------------------------------------------------------------------------------------------------------

./mypdfsearch --words="sound アリス" --path="../Files/JapaneseGrammarGuide.pdf" --frompage=34 --topage=34
./mypdfsearch --words="sound" --path="../Files/JapaneseGrammarGuide.pdf"
./mypdfsearch --extracttextfrom="../Files/JapaneseGrammarGuide.pdf" --frompage=1 --topage=5

-------------------------------------------------------------------------------------------------------
 
time ./mypdfsearch --path="/home/vincenzo/MyJob/Progetti/Files/FileProblematici" --words="Virginia Orbán branco"


-------------------------------------------------------------------------------------------------------

./mypdfsearch --words="Virginia sound Orbán" --path="../Files/Prova"
./mypdfsearch --words="Virginia sound Orbán" --path="../Files/Prova" --nosubdirs
 
 
 
./mypdfsearch --path="../Files/FileProblematico3" --words="Virginia Orbán branco"
./mypdfsearch --path="../Files/FileProblematico4" --words="Virginia Orbán branco"
 
 

./mypdfsearch --path="../Files/gccFiles/Giornali" --words="Virginia Orbán branco"
  
./mypdfsearch --path="../Files/gccFiles/Giornali/Manzoni" --words="Virginia branco Milano"
  
./mypdfsearch --extracttextfrom="../Files/gccFiles/Giornali/Manzoni/manzoni_i_promessi_sposi.pdf" --frompage=254 --topage=254



./mypdfsearch --extracttextfrom="../Files/FileProblematico2/corriere_della_sera_-_03_gennaio_2018.pdf" --frompage=17 --topage=17



./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File01" --frompage=1 --topage=1 > AAA_01_DEBUG.txt
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File02" --frompage=1 --topage=1 > AAA_02_DEBUG.txt
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File03" --frompage=1 --topage=1 > AAA_03_DEBUG.txt
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File04" --frompage=1 --topage=1 > AAA_04_DEBUG.txt
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File05" --frompage=1 --topage=1 > AAA_05_DEBUG.txt
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File06" --frompage=1 --topage=1 > AAA_06_DEBUG.txt
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File07" --frompage=1 --topage=1 > AAA_07_DEBUG.txt
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File08" --frompage=1 --topage=1 > AAA_08_DEBUG.txt
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File09" --frompage=1 --topage=1 > AAA_09_DEBUG.txt
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File10" --frompage=1 --topage=1 > AAA_10_DEBUG.txt
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/FileProblematici" --frompage=3 --topage=4
 

 
./mypdfsearch --extracttextfrom="../Files/File01/Corriere della Sera - 20 luglio 2019.pdf" --frompage=1 --topage=1 > AAA_01.txt
./mypdfsearch --extracttextfrom="../Files/File02/Il Fatto Quotidiano - 9 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_02.txt
./mypdfsearch --extracttextfrom="../Files/File03/Il Giornale - 8 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_03.txt
./mypdfsearch --extracttextfrom="../Files/File04/Il Manifesto - 9 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_04.txt
./mypdfsearch --extracttextfrom="../Files/File05/Il Sole 24 Ore - 9 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_05.txt
./mypdfsearch --extracttextfrom="../Files/File06/Il Sole 24 Ore Gli Speciali - Università - I nuovi test - 9 aprile 2019.pdf" --frompage=3 --topage=3 > AAA_06.txt
./mypdfsearch --extracttextfrom="../Files/File07/Il Sole 24 Ore Inserto - 9 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_07.txt
./mypdfsearch --extracttextfrom="../Files/File08/La Repubblica - 9 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_08.txt
./mypdfsearch --extracttextfrom="../Files/File09/La Stampa - 9 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_09.txt
./mypdfsearch --extracttextfrom="../Files/File10/Libero - 8 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_10.txt


 
./mypdfsearch --extracttextfrom="../Files/File01/Corriere della Sera - 20 luglio 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_01.txt" > AAA_01.txt
./mypdfsearch --extracttextfrom="../Files/File02/Il Fatto Quotidiano - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_02.txt" > AAA_02.txt
./mypdfsearch --extracttextfrom="../Files/File03/Il Giornale - 8 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_03.txt" > AAA_03.txt
./mypdfsearch --extracttextfrom="../Files/File04/Il Manifesto - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_04.txt" > AAA_04.txt
./mypdfsearch --extracttextfrom="../Files/File05/Il Sole 24 Ore - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_05.txt" > AAA_05.txt
./mypdfsearch --extracttextfrom="../Files/File06/Il Sole 24 Ore Gli Speciali - Università - I nuovi test - 9 aprile 2019.pdf" --frompage=3 --topage=3 --outputfile="AAA_OUTPUT_06.txt" > AAA_06.txt
./mypdfsearch --extracttextfrom="../Files/File07/Il Sole 24 Ore Inserto - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_07.txt" > AAA_07.txt
./mypdfsearch --extracttextfrom="../Files/File08/La Repubblica - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_08.txt" > AAA_08.txt
./mypdfsearch --extracttextfrom="../Files/File09/La Stampa - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_09.txt" > AAA_09.txt
./mypdfsearch --extracttextfrom="../Files/File10/Libero - 8 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_10.txt" > AAA_10.txt
*/



// ***********************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************

/*
gcc -Wall -Wextra -pedantic -Wno-overlength-strings -O0 -g -std=c99 -D_GNU_SOURCE mytime.c myTree.c vlRedBlackTrees.c myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearchdebug -lz -lm

------------------------------------------------------------------------------------------------------------------------------------
 
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan04.pdf" --frompage=6 --topage=6
 
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --path='../Files/gccFiles/Giornali/A2018/Q20180104/la Repubblica 4 Gennaio 2018.pdf' --words="Virginia Orbán branco"

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --path="../Files/gccFiles/Giornali/Manzoni/manzoni_i_promessi_sposi.pdf" --words="Virginia Orbán branco"

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/File10/aandy01.pdf" --frompage=1 --topage=1

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/File10/aandy02.pdf" --frompage=1 --topage=1

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/File09/La Stampa - 9 aprile 2019.pdf" --frompage=1 --topage=1

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom='../Files/Nuovi/Q20191012/La Repubblica - 12 ottobre 2019.pdf' --frompage=1 --topage=1

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/File03/Il Giornale - 8 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile=AAA_OUT_ILGIORNALE_FONTS.txt > AAA_ILGIORNALE_FONTS.txt

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/File10/Libero - 8 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_FONTS_LIBERO.txt

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/File10/Libero - 8 aprile 2019.pdf" --frompage=1 --topage=1
 
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/chapter6/kerning.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/chapter6/rise.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/chapter6/spacing.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/chapter6/textmodes.pdf" --frompage=1 --topage=1 
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/chapter6/texttransforms.pdf" --frompage=1 --topage=1

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_00bis.txt ./mypdfsearchdebug --path="../Files/gccFiles/Giornali/A2018/Q20180102/corriere_della_sera_-_02_gennaio_2018.pdf" --words="Virginia Orbán branco"

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_01.txt ./mypdfsearchdebug --extracttextfrom="../Files/File01/Corriere della Sera - 20 luglio 2019.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_02.txt ./mypdfsearchdebug --extracttextfrom="../Files/File02/Il Fatto Quotidiano - 9 aprile 2019.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_03.txt ./mypdfsearchdebug --extracttextfrom="../Files/File03/Il Giornale - 8 aprile 2019.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_04.txt ./mypdfsearchdebug --extracttextfrom="../Files/File04/Il Manifesto - 9 aprile 2019.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_05.txt ./mypdfsearchdebug --extracttextfrom="../Files/File05/Il Sole 24 Ore - 9 aprile 2019.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_06.txt ./mypdfsearchdebug --extracttextfrom="../Files/File06/Il Sole 24 Ore Gli Speciali - Università - I nuovi test - 9 aprile 2019.pdf" --frompage=3 --topage=3
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_07.txt ./mypdfsearchdebug --extracttextfrom="../Files/File07/Il Sole 24 Ore Inserto - 9 aprile 2019.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_08.txt ./mypdfsearchdebug --extracttextfrom="../Files/File08/La Repubblica - 9 aprile 2019.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_09.txt ./mypdfsearchdebug --extracttextfrom="../Files/File09/La Stampa - 9 aprile 2019.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_10.txt ./mypdfsearchdebug --extracttextfrom="../Files/File10/Libero - 8 aprile 2019.pdf" --frompage=1 --topage=1

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_11.txt ./mypdfsearchdebug --extracttextfrom="../Files/FileProblematico2/corriere_della_sera_-_03_gennaio_2018.pdf" --frompage=17 --topage=17
 
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_12.txt ./mypdfsearchdebug  --extracttextfrom="../Files/gccFiles/Giornali/Manzoni/manzoni_i_promessi_sposi.pdf" --frompage=254 --topage=254

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_13.txt ./mypdfsearchdebug --words="sound アリス" --path="../Files/JapaneseGrammarGuide.pdf" --frompage=34 --topage=34
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_14.txt ./mypdfsearchdebug --words="sound" --path="../Files/JapaneseGrammarGuide.pdf"
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_15.txt ./mypdfsearchdebug --extracttextfrom="../Files/JapaneseGrammarGuide.pdf" --frompage=1 --topage=5
 
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_16.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/SoloGiapponese/kk190531a.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_17.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/SoloGiapponese/ohome.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_18.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/SoloGiapponese/VerticalWriting.pdf" --frompage=2 --topage=2

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_19.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan01.pdf" --frompage=5 --topage=5
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_20.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan03.pdf" --frompage=9 --topage=9
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_21.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan04.pdf" --frompage=6 --topage=6
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_22.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan05.pdf" --frompage=5 --topage=5
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_23.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan06.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_24.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan07.pdf" --frompage=21 --topage=21
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_25.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan08.pdf" --frompage=24 --topage=24
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_26.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan09.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_27.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan11.pdf" --frompage=10 --topage=10
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_28.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan12.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_29.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan13.pdf" --frompage=1 --topage=1
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_30.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan14.pdf" --frompage=3 --topage=3
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=../../Valgrind/AAA_outputValgrind_31.txt ./mypdfsearchdebug --extracttextfrom="../Files/Giapponesi/Misto/japan15.pdf" --frompage=1 --topage=1 



valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind_32.txt ./mypdfsearchdebug --path="../Files/gccFiles/Giornali" --words="Virginia Orbán branco"
*/

/*
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191016/Corriere della Sera - 16 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191016/Il Fatto Quotidiano - 16 ottobre 2019.pdf' --frompage=1 --topage=1

./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191014/Corriere della Sera - 14 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191014/Il Fatto Quotidiano - 14 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191014/Il Giornale - 14 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191014/Il Manifesto - 14 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191014/Il Sole 24 Ore - 14 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191014/Il Sole 24 Ore La Guida Rapida - 14 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191014/La Repubblica - 14 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191014/La Stampa - 14 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191014/Libero - 14 ottobre 2019.pdf' --frompage=1 --topage=1 

./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191013/Corriere della Sera - 13 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191013/Il Fatto Quotidiano - 13 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191013/Il Giornale - 13 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191013/Il Manifesto - 13 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191013/Il Sole 24 Ore - 13 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191013/La Repubblica - 13 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191013/La Stampa - 13 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191013/Libero - 13 ottobre 2019.pdf' --frompage=1 --topage=1
 
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191012/Corriere della Sera - 12 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191012/Il Fatto Quotidiano - 12 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191012/Il Giornale - 12 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191012/Il Manifesto 12 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191012/Il Sole 24 Ore - 12 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191012/Il Sole 24 Ore Plus - 12 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191012/La Repubblica - 12 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191012/La Stampa - 12 ottobre 2019.pdf' --frompage=1 --topage=1
./mypdfsearch --extracttextfrom='../Files/Nuovi/Q20191012/Libero - 12 ottobre 2019.pdf' --frompage=1 --topage=1

------------------------------------------------------------------------------------------------------------------------------------
 
gcc -Wall -W -pedantic -Wno-overlength-strings -O3 -std=c99 -D_GNU_SOURCE mytime.c myTree.c vlRedBlackTrees.c myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz -lm
 
./mypdfsearch --path="../Files/FileProblematici/prob01.pdf" --words="Virginia Orbán branco"
 
./mypdfsearch --path="../Files/gccFiles/Giornali/A2018/Q20180102/corriere_della_sera_-_02_gennaio_2018.pdf" --words="Virginia Orbán branco"


pdftotext -f 1 -l 1 -enc UTF-8 '../Files/File03/Il Giornale - 8 aprile 2019.pdf' AAA_PDFTOTEXT_GIORNALE_01.txt
pdftotext -f 1 -l 1 -enc UTF-8 -raw '../Files/File03/Il Giornale - 8 aprile 2019.pdf' AAA_PDFTOTEXT_GIORNALE_02.txt
pdftotext -f 1 -l 1 -enc UTF-8 -layout '../Files/File03/Il Giornale - 8 aprile 2019.pdf' AAA_PDFTOTEXT_GIORNALE_03.txt



./mypdfsearch --extracttextfrom="../Files/chapter6/mykerning.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/chapter6/myrise.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/chapter6/myspacing.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/chapter6/mytextmodes.pdf" --frompage=1 --topage=1 
./mypdfsearch --extracttextfrom="../Files/chapter6/mytexttransforms.pdf" --frompage=1 --topage=1



./mypdfsearch --extracttextfrom="../Files/File01/Corriere della Sera - 20 luglio 2019.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/File02/Il Fatto Quotidiano - 9 aprile 2019.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/File03/Il Giornale - 8 aprile 2019.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/File04/Il Manifesto - 9 aprile 2019.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/File05/Il Sole 24 Ore - 9 aprile 2019.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/File06/Il Sole 24 Ore Gli Speciali - Università - I nuovi test - 9 aprile 2019.pdf" --frompage=3 --topage=3
./mypdfsearch --extracttextfrom="../Files/File07/Il Sole 24 Ore Inserto - 9 aprile 2019.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/File08/La Repubblica - 9 aprile 2019.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/File09/La Stampa - 9 aprile 2019.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/File10/Libero - 8 aprile 2019.pdf" --frompage=1 --topage=1
 

./mypdfsearch --extracttextfrom="../Files/File10/aandy01.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/File10/aandy02.pdf" --frompage=1 --topage=1
 
./mypdfsearch --extracttextfrom="../Files/aandy/mylibero.pdf" --frompage=1 --topage=1



./mypdfsearch --path="../Files/gccFiles/Giornali" --words="Virginia Orbán branco"
 


-DMYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR -DMYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR -DMYDEBUG_PRINT_ON_TEXT_STATE_OPERATOR -DMYDEBUG_PRINT_ON_CTM_OPERATOR -DMYDEBUG_PRINT_SINGLE_CHAR_COORD2 -DMYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected -DMYDEBUG_PRINT_ON_ManageContent_FN_ShowGsSelected
-DMYDEBUG_PRINT_ON_SHOW_STRING_OPERATOR -DMYDEBUG_PRINT_ON_POSITIONING_STRING_OPERATOR -DMYDEBUG_PRINT_ON_TEXT_STATE_OPERATOR -DMYDEBUG_PRINT_ON_CTM_OPERATOR -DMYDEBUG_PRINT_SINGLE_CHAR_COORD3 -DMYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected -DMYDEBUG_PRINT_ON_ManageContent_FN_ShowGsSelected

-DMYDEBUG_PRINT_SINGLE_CHAR_COORD3 -DMYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected -DMYDEBUG_PRINT_ON_ManageContent_FN_ShowGsSelected
 
-DMYDEBUG_PRINT_ON_PARSE_FONTOBJ -DMYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ -DMYDEBUG_PRINT_ON_PARSE_FONTDESCRIPTOROBJ

-DMYDEBUG_PRINT_SINGLE_CHAR_COORD3
-DMYDEBUG_PRINT_ON_ManageContent_FN_ShowResourceSelected
-DMYDEBUG_PRINT_ON_ManageContent_FN_ShowFontSelected
-DMYDEBUG_PRINT_ON_ManageContent_FN_ShowGsSelected
-DMAKE_PDFTK_SRC

./mypdfsearch --extracttextfrom="../Files/FileProblematico2/corriere_della_sera_-_03_gennaio_2018.pdf" --frompage=17 --topage=17
 
./mypdfsearch  --extracttextfrom="../Files/gccFiles/Giornali/Manzoni/manzoni_i_promessi_sposi.pdf" --frompage=254 --topage=254

./mypdfsearch --words="sound アリス" --path="../Files/JapaneseGrammarGuide.pdf" --frompage=34 --topage=34
./mypdfsearch --words="sound  アリス" --path="../Files/JapaneseGrammarGuide.pdf"
./mypdfsearch --extracttextfrom="../Files/JapaneseGrammarGuide.pdf" --frompage=1 --topage=5
 
./mypdfsearch --extracttextfrom="../Files/Giapponesi/SoloGiapponese/kk190531a.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/Giapponesi/SoloGiapponese/ohome.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/Giapponesi/SoloGiapponese/VerticalWriting.pdf" --frompage=2 --topage=2

./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan01.pdf" --frompage=5 --topage=5
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan03.pdf" --frompage=9 --topage=9
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan04.pdf" --frompage=6 --topage=6
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan05.pdf" --frompage=5 --topage=5
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan06.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan07.pdf" --frompage=21 --topage=21
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan08.pdf" --frompage=24 --topage=24
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan09.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan11.pdf" --frompage=10 --topage=10
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan12.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan13.pdf" --frompage=1 --topage=1
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan14.pdf" --frompage=3 --topage=3
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan15.pdf" --frompage=1 --topage=1 
./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japaneseVerticalPagina97.pdf" --frompage=97 --topage=97
 

./mypdfsearch --extracttextfrom="../Files/Giapponesi/Misto/japan14.pdf" --frompage=3 --topage=5

*/
