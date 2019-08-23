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
MYDEBUG_PRINT_ALL
MYDEBUG_PRINT_ON_MATCH
MYDEBUG_PRINT_ON_PARSE_OBJ
MYDEBUG_PRINT_ON_PARSE_STREAMOBJ
MYDEBUG_PRINT_ON_PARSE_STREAMXOBJ
MYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM
MYDEBUG_PRINT_ON_PARSE_CMAP_STREAM
MYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ
MYDEBUG_PRINT_ON_PARSE_LENGTHOBJ
MYDEBUG_PRINT_ON_PARSE_XOBJDICTOBJ
MYDEBUG_PRINT_ON_PARSE_RESOURCESDICT
MYDEBUG_PRINT_ON_PARSE_FONTOBJ
MYDEBUG_PRINT_ON_PARSE_ENCODINGOBJ

MYDEBUG_PRINT_TST
 
MYDEBUG_PRINT_ON_GetNextToken_FN
 
MYDEBUG_PRINT_ON_ReadHeader_FN
MYDEBUG_PRINT_ON_ReadTrailer_FN
MYDEBUG_PRINT_ON_ReadTrailer_OBJ

MYDEBUG_PRINT_ON_PARSE_FN
MYDEBUG_PRINT_ON_ManageContent_FN
MYDEBUG_PRINT_ON_ManageContent_PrintPageNum
MYDEBUG_PRINT_ON_ManageContent_PrintContent
MYDEBUG_PRINT_ON_ManageContent_PrintContent_HEXCODECHAR
MYDEBUG_PRINT_ON_ManageContent_PrintStrings
MYDEBUG_PRINT_ON_ManageContent_PrintStrings_HEXADECIMAL
MYDEBUG_PRINT_ON_ParseObject_FN
MYDEBUG_PRINT_ON_ParseStreamObject_FN
MYDEBUG_PRINT_ON_ParseDictionaryObject_FN
MYDEBUG_PRINT_ON_ParseLengthObject_FN
*/


/*
gcc -Wall -W -pedantic -Wno-overlength-strings -O3 -std=c99 -D_GNU_SOURCE myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz

gcc -Wall -W -pedantic -Wno-overlength-strings -O3 -std=c99 -D_GNU_SOURCE myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz -DMYDEBUG_PRINT_ON_ManageContent_PrintContent -DMYDEBUG_PRINT_ON_ManageContent_PrintContent_HEXCODECHAR

gcc -Wall -W -pedantic -Wno-overlength-strings -O3 -std=c99 -D_GNU_SOURCE myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz -DMYDEBUG_PRINT_ON_PARSE_FONTOBJ -DMYDEBUG_PRINT_ON_PARSE_TOUNICODE_STREAM -DMYDEBUG_PRINT_ON_PARSE_CMAP_STREAM
gcc -Wall -W -pedantic -Wno-overlength-strings -O3 -std=c99 -D_GNU_SOURCE myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz -DMYDEBUG_PRINT_ON_PARSE_FONTOBJ

*/


/*
------------------------------------------------------------------------------------------------------------------------------------

./mypdfsearch --extracttextfrom="/home/vincenzo/progetti/Files/Giapponesi/SoloGiapponese/kk190531a.pdf" --frompage=1 --topage=1
 
./mypdfsearch --extracttextfrom="/home/vincenzo/progetti/Files/Giapponesi/SoloGiapponese/ohome.pdf" --frompage=1 --topage=1

./mypdfsearch --extracttextfrom="/home/vincenzo/progetti/Files/Giapponesi/SoloGiapponese/VerticalWriting.pdf" --frompage=2 --topage=2

./mypdfsearch --extracttextfrom="/home/vincenzo/progetti/Files/Giapponesi/SoloGiapponese/H21 monodukuri report.pdf" --frompage=1 --topage=1
 
------------------------------------------------------------------------------------------------------------------------------------
*/

/*
./mypdfsearch --outputfile=/home/vincenzo/Varie/FormatiFile/PDF/Codice/myPdfSearch/Files/FilesNew/FileNew/mypdfsearchOutput.txt --path=/home/vincenzo/Varie/FormatiFile/PDF/Codice/myPdfSearch/Files/FilesNew/File01
 
./mypdfsearch --outputfile=/home/vincenzo/Varie/FormatiFile/PDF/Codice/myPdfSearch/Files/FilesNew/FileNew/mypdfsearchOutput.txt --path=/home/vincenzo/Varie/FormatiFile/PDF/Codice/myPdfSearch/Files/FilesNew/FileProblematici
 
./mypdfsearch --words="Virginia campidoglio Orbán" --path=/home/vincenzo/Varie/FormatiFile/PDF/Codice/myPdfSearch/Files/FilesNew/File01
 
./mypdfsearch --words="Virginia campidoglio Orbán" --path=../Files/File02
 
./mypdfsearch --words="Virginia campidoglio Orbán" --path=/home/vincenzo/progetti/Files/File02

./mypdfsearch -w"Virginia campidoglio Orbán" --path=/home/vincenzo/Varie/FormatiFile/PDF/Codice/myPdfSearch/Files/FilesNew/File01
 

./mypdfsearch --path="/home/vincenzo/Varie/GCC/Varie/Files/Giornali" --words="Virginia Orbán branco"



./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/Tutti"
 
 

./mypdfsearch --path="/home/vincenzo/Varie/GCC/Varie/Files/Giornali" --words="Virginia Orbán branco"
  
./mypdfsearch --path="/home/vincenzo/Varie/GCC/Varie/Files/Giornali/Manzoni" --words="Virginia branco Milano"
  
./mypdfsearch --extracttextfrom="/home/vincenzo/Varie/GCC/Varie/Files/Giornali/Manzoni/manzoni_i_promessi_sposi.pdf" --frompage=254 --topage=254


 
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
./mypdfsearch --extracttextfrom="../Files/File06/Il Sole 24 Ore Gli Speciali - Università - I nuovi test - 9 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_06.txt
./mypdfsearch --extracttextfrom="../Files/File07/Il Sole 24 Ore Inserto - 9 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_07.txt
./mypdfsearch --extracttextfrom="../Files/File08/La Repubblica - 9 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_08.txt
./mypdfsearch --extracttextfrom="../Files/File09/La Stampa - 9 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_09.txt
./mypdfsearch --extracttextfrom="../Files/File10/Libero - 8 aprile 2019.pdf" --frompage=1 --topage=1 > AAA_10.txt


 
./mypdfsearch --extracttextfrom="../Files/File01/Corriere della Sera - 20 luglio 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_01.txt" > AAA_01.txt
./mypdfsearch --extracttextfrom="../Files/File02/Il Fatto Quotidiano - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_02.txt" > AAA_02.txt
./mypdfsearch --extracttextfrom="../Files/File03/Il Giornale - 8 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_03.txt" > AAA_03.txt
./mypdfsearch --extracttextfrom="../Files/File04/Il Manifesto - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_04.txt" > AAA_04.txt
./mypdfsearch --extracttextfrom="../Files/File05/Il Sole 24 Ore - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_05.txt" > AAA_05.txt
./mypdfsearch --extracttextfrom="../Files/File06/Il Sole 24 Ore Gli Speciali - Università - I nuovi test - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_06.txt" > AAA_06.txt
./mypdfsearch --extracttextfrom="../Files/File07/Il Sole 24 Ore Inserto - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_07.txt" > AAA_07.txt
./mypdfsearch --extracttextfrom="../Files/File08/La Repubblica - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_08.txt" > AAA_08.txt
./mypdfsearch --extracttextfrom="../Files/File09/La Stampa - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_09.txt" > AAA_09.txt
./mypdfsearch --extracttextfrom="../Files/File10/Libero - 8 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_10.txt" > AAA_10.txt
*/

