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
	char myPathName[PATH_MAX + 1];
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

MYDEBUG_PRINT_ON_PARSE_FN
MYDEBUG_PRINT_ON_ManageContent_FN
MYDEBUG_PRINT_ON_ManageContent_PrintPageNum
MYDEBUG_PRINT_ON_ManageContent_PrintContent
MYDEBUG_PRINT_ON_ManageContent_PrintStrings
MYDEBUG_PRINT_ON_ParseObject_FN
MYDEBUG_PRINT_ON_ParseStreamObject_FN
MYDEBUG_PRINT_ON_ParseDictionaryObject_FN
MYDEBUG_PRINT_ON_ParseLengthObject_FN
*/


/*
gcc -Wall -W -pedantic -O3 -std=c99 -D_GNU_SOURCE myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz

gcc -Wall -W -pedantic -O3 -std=c99 -D_GNU_SOURCE -DMYDEBUG_PRINT_ON_PARSE_STREAMXOBJ myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz

gcc -Wall -W -pedantic -O3 -std=c99 -D_GNU_SOURCE -DMYDEBUG_PRINT_ON_PARSE_PAGETREEOBJ myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz 

gcc -Wall -W -pedantic -O3 -std=c99 -D_GNU_SOURCE -DMYDEBUG_PRINT_ON_ParseObject_FN myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz

gcc -Wall -W -pedantic -O3 -std=c99 -D_GNU_SOURCE -DMYDEBUG_PRINT_TST myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz

gcc -Wall -W -pedantic -O3 -std=c99 -D_GNU_SOURCE -DMYDEBUG_PRINT_ON_ManageContent_PrintStrings myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz

gcc -Wall -W -pedantic -O3 -std=c99 -D_GNU_SOURCE -DMYDEBUG_PRINT_ON_ManageContent_PrintContent myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz
 
gcc -Wall -W -pedantic -O3 -std=c99 -D_GNU_SOURCE -DMYDEBUG_PRINT_ON_ManageContent_FN myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz
 
gcc -Wall -W -pedantic -O3 -std=c99 -D_GNU_SOURCE -DMYDEBUG_PRINT_ON_GetNextToken_FN myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz
*/

/*
./mypdfsearch --outputfile=/home/vincenzo/Varie/FormatiFile/PDF/Codice/myPdfSearch/Files/FilesNew/FileNew/mypdfsearchOutput.txt --path=/home/vincenzo/Varie/FormatiFile/PDF/Codice/myPdfSearch/Files/FilesNew/File01
 
./mypdfsearch --outputfile=/home/vincenzo/Varie/FormatiFile/PDF/Codice/myPdfSearch/Files/FilesNew/FileNew/mypdfsearchOutput.txt --path=/home/vincenzo/Varie/FormatiFile/PDF/Codice/myPdfSearch/Files/FilesNew/FileProblematici
 
./mypdfsearch --words="Virginia campidoglio Orbán" --path=/home/vincenzo/Varie/FormatiFile/PDF/Codice/myPdfSearch/Files/FilesNew/File01
 
./mypdfsearch --words="Virginia campidoglio Orbán" --path=../Files/File02
 
./mypdfsearch --words="Virginia campidoglio Orbán" --path=/home/vincenzo/progetti/Files/File02

./mypdfsearch -w"Virginia campidoglio Orbán" --path=/home/vincenzo/Varie/FormatiFile/PDF/Codice/myPdfSearch/Files/FilesNew/File01
 
 
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File01" --frompage=3 --topage=4
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File02" --frompage=3 --topage=4
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File03" --frompage=3 --topage=4
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File04" --frompage=3 --topage=4
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File05" --frompage=3 --topage=4
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File06" --frompage=3 --topage=4
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File07" --frompage=3 --topage=4
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File08" --frompage=3 --topage=4 
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File09" --frompage=3 --topage=4
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/File10" --frompage=3 --topage=4
./mypdfsearch --words="Virginia campidoglio Orbán" --path="../Files/FileProblematici" --frompage=3 --topage=4
 
 
./mypdfsearch --extracttextfrom="../Files/File01/Corriere della Sera - 20 luglio 2019.pdf" --frompage=3 --topage=4
./mypdfsearch --extracttextfrom="../Files/File02/Il Fatto Quotidiano - 9 aprile 2019.pdf" --frompage=3 --topage=4
./mypdfsearch --extracttextfrom="../Files/File03/Il Giornale - 8 aprile 2019.pdf" --frompage=3 --topage=4
./mypdfsearch --extracttextfrom="../Files/File04/Il Manifesto - 9 aprile 2019.pdf" --frompage=3 --topage=4
./mypdfsearch --extracttextfrom="../Files/File05/Il Sole 24 Ore - 9 aprile 2019.pdf" --frompage=3 --topage=4
./mypdfsearch --extracttextfrom="../Files/File06/Il Sole 24 Ore Gli Speciali - Università - I nuovi test - 9 aprile 2019.pdf" --frompage=3 --topage=4
./mypdfsearch --extracttextfrom="../Files/File07/Il Sole 24 Ore Inserto - 9 aprile 2019.pdf" --frompage=3 --topage=4
./mypdfsearch --extracttextfrom="../Files/File08/La Repubblica - 9 aprile 2019.pdf" --frompage=3 --topage=4 
./mypdfsearch --extracttextfrom="../Files/File09/La Stampa - 9 aprile 2019.pdf" --frompage=3 --topage=4
./mypdfsearch --extracttextfrom="../Files/File10/Libero - 8 aprile 2019.pdf" --frompage=3 --topage=4
./mypdfsearch --extracttextfrom="../Files/FileProblematici/La Stampa - 20 luglio 2019 .pdf" --frompage=3 --topage=4
*/



/*
File: '../Files/FileProblematici/manzoni_i_promessi_sposi.pdf'
ERRORE Parse 6 nella lettura del trailer del file '../Files/FileProblematici/manzoni_i_promessi_sposi.pdf'.

File: '../Files/FileProblematici/corriere_della_sera_-_03_gennaio_2018.pdf'
Errore ReadLastTrailer: atteso 'x' trovato '1'
<</DecodeParms<</Columns 5/Predictor 12>>/Filter/FlateDecode/ID[<665FD8676C98808A3C960B8E789B0A4E><456B13A2FFCB364B9618DC640546DBB8>]/>
ERRORE Parse 6 nella lettura del trailer del file '../Files/FileProblematici/corriere_della_sera_-_03_gennaio_2018.pdf'.

File: '../Files/FileProblematici/corriere_della_sera_-_02_gennaio_2018.pdf'
Errore ReadLastTrailer: atteso 'x' trovato '1'
<</DecodeParms<</Columns 5/Predictor 12>>/Filter/FlateDecode/ID[<53B40E70C3D98AA461B83F01FE8A0376><FFC5D8F31B298847A4DAFC5CEC3E6C39>]/>
ERRORE Parse 6 nella lettura del trailer del file '../Files/FileProblematici/corriere_della_sera_-_02_gennaio_2018.pdf'.

File: '../Files/FileProblematici/la_repubblica_-_02_gennaio_2018.pdf'
Errore ReadLastTrailer: atteso 'x' trovato '1'
<</DecodeParms<</Columns 5/Predictor 12>>/Filter/FlateDecode/ID[<01C6E223E840B3333505663E1E0B832C><270EF99E22D29D4B98916E8D124BFD94>]/>
ERRORE Parse 6 nella lettura del trailer del file '../Files/FileProblematici/la_repubblica_-_02_gennaio_2018.pdf'.

File: '../Files/FileProblematici/La Verita 5 Gennaio 2018.pdf'
Errore ParseObject: oggetto n. 1; atteso spazio, trovato 'F'
File: '../Files/FileProblematici/Libero - 2 Gennaio 2018.pdf'
ERRORE Parse 6 nella lettura del trailer del file '../Files/FileProblematici/Libero - 2 Gennaio 2018.pdf'.

File: '../Files/FileProblematici/il Fatto Quotidiano 02 Gennaio 2018.pdf'
ERRORE parsing prepagetree: Type non valido: 'Group'
ERRORE!!! ParseNextObject 2
 
***** INIZIO OBJ(48) = 
48 0 obj
<< /Type /Page
   /Parent 1 0 R
   /MediaBox [ 0 0 864.757 1275.22 ]
   /Contents 3 0 R
   /Group <<
      /Type /Group
      /S /Transparency
      /I true
      /CS /DeviceRGB
   >>
   /Resources 2 0 R
>>
endobj
*****FINE OBJ(48)
*/
