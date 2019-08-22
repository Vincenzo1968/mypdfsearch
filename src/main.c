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

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <locale.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <pthread.h>

#include <wchar.h>

#if defined(_WIN64) || defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

#include "mypdfsearch.h"
#include "parser.h"

void printError(int numError);

FilesList* addFileToFilesList(FilesList *first, FilesList *newFile);
void freeFilesList(FilesList *first);

FilesList* addFileToFilesList(FilesList *first, FilesList *newFile)
{
	FilesList *n;

	n = (FilesList*)malloc(sizeof(FilesList));

	if( n == NULL )
		return NULL;
	
	#if !defined(_WIN64) && !defined(_WIN32)
	strcpy(n->myPathName, newFile->myPathName);
	#endif
	strcpy(n->myFileName, newFile->myFileName);
	
	n->next = NULL;
	
	if ( first != NULL )
		n->next = first;

	return n;
}

void freeFilesList(FilesList *first)
{
	FilesList *n1 = first, *n2;
	while ( n1 != NULL )
	{
		n2 = n1->next;
		free(n1);
		n1 = n2;
	}
}


#if defined(_WIN64) || defined(_WIN32)
void GetErrorString(DWORD err, char *szError, int len)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf,
		0, NULL);

	strncpy(szError, lpMsgBuf, len);

	LocalFree(lpMsgBuf);
}

FilesList* getFilesRecursive(char *dirName, int lenOrig, FilesList* myFilesList)
{
	char pathName[MAX_PATH + 1];

	WIN32_FIND_DATAA data;

	char fname[_MAX_DIR];
	
	FilesList myFile;

	strncpy(fname, dirName, _MAX_DIR - 1);
	//strncpy(fname, dirName, strnlen(dirName, _MAX_DIR));
	//strncat(fname, "\\*.pdf", _MAX_DIR - 1);
	strncat(fname, "\\*.*", _MAX_DIR - 1);

	HANDLE h = FindFirstFileA(fname, &data);
	if (h != INVALID_HANDLE_VALUE)
	{
		do {

			strncpy(pathName, dirName, MAX_PATH);
			strncat(pathName, "\\", MAX_PATH);
			strncat(pathName, data.cFileName, MAX_PATH);
			
			
			//int lung = strnlen(pathName, MAX_LEN_STR);
			//wprintf(L"PATH NAME = <");
			//for ( int idx = 0; idx < lung; idx++ )
			//{
			//	wprintf(L"%c", pathName[idx]);
			//}
			//wprintf(L">\n\n");
			

			if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				//if (!(data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
				if ( !(data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) )
				{
					if (strncmp(data.cFileName, ".", _MAX_DIR) != 0 && strncmp(data.cFileName, "..", _MAX_DIR) != 0)
					{
						myFilesList = getFilesRecursive(pathName, lenOrig, myFilesList);
					}
				}
			}
			else
			{
				int len = strnlen(pathName, MAX_LEN_STR);
				if ( len > 4 ) // .pdf = 4
				{
					if ( tolower(pathName[len - 1]) == 'f' && tolower(pathName[len - 2]) == 'd' && tolower(pathName[len - 3]) == 'p' && pathName[len - 4] == '.' )
					{
						strncpy(myFile.myFileName, pathName, len + 1);										
						myFilesList = addFileToFilesList(myFilesList, &myFile);
						if ( !myFilesList )
							goto uscita;
					}
				}
				
			}
		} while (FindNextFileA(h, &data) != 0);
		FindClose(h);
	}
	
	uscita:

	return myFilesList;
}
#else
FilesList* getFilesRecursive(char *dirName, int lenOrig, FilesList* myFilesList)
{
	FilesList myFile;		
	DIR *dir = NULL;
	struct dirent entry;
	struct dirent *entryPtr = NULL;
	char pathName[PATH_MAX + 1];
	
	int i;
	int len;
	int len2;
	//int len3;
	int index;
	char *szExt;	

	dir = opendir(dirName);
	if( dir == NULL )
	{
		wprintf(L"Errore nell'apertura della directory '%s': %s\n", dirName, strerror(errno));
		return myFilesList;
	}	
	
	//myFile.relativeName[0] = '\0';
	myFile.myPathName[0] = '\0';
	myFile.myFileName[0] = '\0';
	
	/* retval = */ readdir_r(dir, &entry, &entryPtr);
	while( entryPtr != NULL )
	{
		struct stat entryInfo;
		
		if( ( strncmp(entry.d_name, ".", PATH_MAX) == 0 ) ||
		    ( strncmp(entry.d_name, "..", PATH_MAX) == 0 ) )
		{
			/* retval = */ readdir_r(dir, &entry, &entryPtr);
		    continue;
		}
		 		
		strncpy(pathName, dirName, PATH_MAX);
		strncat(pathName, "/", PATH_MAX);
		strncat(pathName, entry.d_name, PATH_MAX);
				
		if( lstat(pathName, &entryInfo) == 0 )
		{						
			if( S_ISDIR(entryInfo.st_mode) )		/* directory */
			{								
				myFilesList = getFilesRecursive(pathName, lenOrig, myFilesList);
			}
			else if( S_ISREG(entryInfo.st_mode) )	/* regular file */
			{
				len = strlen(entry.d_name);				
				
				index = 0;
				for ( i = len - 1; i >= 0; i-- )
				{
						
					if ( entry.d_name[i] == '.' )
					{						
						index = i;
						break;
					}
				}
											
				if ( index > 0 )
				{
					szExt = &entry.d_name[index];
					if ( strcmp(".pdf", szExt) == 0 )
					{
						len2 = strnlen(pathName, PATH_MAX);
						strncpy(myFile.myPathName, pathName, len2 + 1);
						index = strlen(pathName) - len - 1;
						myFile.myPathName[index] = '\0';
				
						strncpy(myFile.myFileName, entry.d_name, PATH_MAX);
						
						myFilesList = addFileToFilesList(myFilesList, &myFile);
						if ( !myFilesList )
							return NULL;
					}
				}								
			}			
		}
		else
		{
			wprintf(L"Errore lstat %s: %s\n", pathName, strerror(errno));
		}

		/* retval = */ readdir_r(dir, &entry, &entryPtr);
	}
		
	closedir(dir);	
	
	return myFilesList;
}
#endif

/* -------------------------------------------------------------------------------------------------- */

/*
Per generare le dipendenze per il Makefile, usare -MM:

[vincenzo]$ gcc -MM -D_GNU_SOURCE myoctal.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -lz

myoctal.o: myoctal.c myoctal.h mypdfsearch.h

mydictionaryqueuelist.o: mydictionaryqueuelist.c scanner.h mypdfsearch.h \
 myobjrefqueuelist.h mynumstacklist.h myintqueuelist.h \
 mystringqueuelist.h mydictionaryqueuelist.h mycontentqueuelist.h \
 myScopeHashTable.h myTernarySearchTree.h mydecode.h

mystringqueuelist.o: mystringqueuelist.c mystringqueuelist.h

mycontentqueuelist.o: mycontentqueuelist.c mycontentqueuelist.h \
 mystringqueuelist.h mydictionaryqueuelist.h

mynumstacklist.o: mynumstacklist.c mynumstacklist.h

myintqueuelist.o: myintqueuelist.c myintqueuelist.h

mydecode.o: mydecode.c mydecode.h myoctal.h mypdfsearch.h
scanner.o: scanner.c myoctal.h mypdfsearch.h scanner.h \
 myobjrefqueuelist.h mynumstacklist.h myintqueuelist.h \
 mystringqueuelist.h mydictionaryqueuelist.h mycontentqueuelist.h \
 myScopeHashTable.h myTernarySearchTree.h mydecode.h

parser.o: parser.c parser.h mypdfsearch.h scanner.h myobjrefqueuelist.h \
 mynumstacklist.h myintqueuelist.h mystringqueuelist.h \
 mydictionaryqueuelist.h mycontentqueuelist.h myScopeHashTable.h \
 myTernarySearchTree.h mydecode.h

main.o: main.c mypdfsearch.h parser.h scanner.h myobjrefqueuelist.h \
 mynumstacklist.h myintqueuelist.h mystringqueuelist.h \
 mydictionaryqueuelist.h mycontentqueuelist.h myScopeHashTable.h \
 myTernarySearchTree.h mydecode.h
*/

/* -------------------------------------------------------------------------------------------------- */

long GetProcessors()
{
	//long nProcs = -1;
	long nProcs_max = -1;

#if defined(_WIN64) || defined(_WIN32)
	SYSTEM_INFO info;
	//GetSystemInfo(&info);
	GetNativeSystemInfo(&info); 
	nProcs_max = info.dwNumberOfProcessors;
#else
	//nProcs_max = sysconf(_SC_NPROCESSORS_ONLN);
	nProcs_max = sysconf(_SC_NPROCESSORS_CONF);	
#endif

	return nProcs_max;
}

wchar_t* myMultibyteToWidechar(const char *psz, size_t mbslen)
{			
	wchar_t *wcs = NULL; 
		
	wcs = calloc(mbslen + 1, sizeof(wchar_t));
	if (wcs == NULL)
	{
		//perror("calloc");
		wprintf(L"ERRORE myMultibyteToWidechar calloc: %s \n", strerror(errno));
  		
		return NULL;
	}	
		
	if (mbstowcs(wcs, (char*)psz, mbslen + 1) == (size_t) -1)
	{
		//perror("mbstowcs");
		wprintf(L"ERRORE myMultibyteToWidechar mbstowcs: %s \n", strerror(errno));
				
		free(wcs);
		wcs = NULL;
		return NULL;
	}
								
	return wcs;
}

int SplitWords(Params *pParams)
{
	int retValue = 1;
	
	wchar_t c;
	int x;
	
	size_t len;

	pParams->pwszCurrentWord = (wchar_t*)malloc(MAX_STRING_LENTGTH_IN_CONTENT_STREAM * sizeof(wchar_t) + sizeof(wchar_t));
	if ( NULL == pParams->pwszCurrentWord )
	{
		wprintf(L"ERRORE SplitWords: impossibile allocare la memoria per pszString.\n");
		retValue = 0;
		goto uscita;
	}
	
	pParams->idxCurrentWordChar = 0;
	
	// SPLIT WORDS INIZIO
	c = (wchar_t)pParams->pwszWordsToSearch[0];
	x = 1;
	while ( L'\0' != c )
	{
		c = towlower(c);
					
		if ( c >= L'a' && c <= L'z' )
		{
			pParams->pwszCurrentWord[pParams->idxCurrentWordChar++] = c;
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
									
			pParams->pwszCurrentWord[pParams->idxCurrentWordChar] = L'\0';
			pParams->idxCurrentWordChar = 0;
			
			len = wcslen(pParams->pwszCurrentWord);
			pParams->pWordsToSearchArray[pParams->countWordsToSearch] = (wchar_t*)malloc(len * sizeof(wchar_t) + sizeof(wchar_t));
			if ( NULL == pParams->pWordsToSearchArray[pParams->countWordsToSearch] )
			{
				wprintf(L"ERRORE SplitWords: impossibile allocare la memoria per pszString.\n");
				retValue = 0;
				goto uscita;
			}
			wcscpy(pParams->pWordsToSearchArray[pParams->countWordsToSearch], pParams->pwszCurrentWord);
			pParams->countWordsToSearch++;			
		}
					
		letterastrana:
		c = (wchar_t)pParams->pwszWordsToSearch[x++];
		
		if ( L'\0' == c )
		{
			pParams->pwszCurrentWord[pParams->idxCurrentWordChar] = L'\0';
			pParams->idxCurrentWordChar = 0;
			
			len = wcslen(pParams->pwszCurrentWord);
			pParams->pWordsToSearchArray[pParams->countWordsToSearch] = (wchar_t*)malloc(len * sizeof(wchar_t) + sizeof(wchar_t));
			if ( NULL == pParams->pWordsToSearchArray[pParams->countWordsToSearch] )
			{
				wprintf(L"ERRORE SplitWords: impossibile allocare la memoria per pszString.\n");
				retValue = 0;
				goto uscita;
			}
			wcscpy(pParams->pWordsToSearchArray[pParams->countWordsToSearch], pParams->pwszCurrentWord);
			pParams->countWordsToSearch++;
		}
	}					
	// SPLIT WORDS FINE
	
	uscita:
		
	if ( NULL != pParams->pwszCurrentWord )
	{
		free(pParams->pwszCurrentWord);
		pParams->pwszCurrentWord = NULL;
	}
	
	return retValue;
}

void PrintHelpCommandLine()
{
	wprintf(L"\nUsage:\n");
	wprintf(L"./mypdfsearch Options\n");
	wprintf(L"Options:\n");
	wprintf(L"At least one of:\n");
	wprintf(L"'-h, --help' (optional)	Print this message and exit\n");
	wprintf(L"'-v, --version' (optional)	Print version info and exit\n");
	wprintf(L"'-e, --extracttextfrom' (optional)	Extract text from specified file and exit\n");
	wprintf(L"   Arguments for option '-e, --extracttextfrom': exactly 1(string)\n");
	wprintf(L"'-f, --frompage' (optional)	Extract text from pagenum specified\n");
	wprintf(L"   Arguments for option '-f, --frompage': exactly 1(int)\n");
	wprintf(L"'-t, --topage' (optional)	Extract text to pagenum specified\n");
	wprintf(L"   Arguments for option '-t, --topage': exactly 1(int)\n");
	wprintf(L"'-p, --path' (optional)	Specifies the input file\n");
	wprintf(L"   Arguments for option '-p, --path': one or more(string)\n");
	wprintf(L"'-w, --words' (optional)	Words to search\n");
	wprintf(L"   Arguments for option '-w, --words': one or more(string)\n");
	wprintf(L"'-o, --outputfile' (optional)	Output file: specifies the output file\n");
	wprintf(L"   Arguments for option '-o, --outputfile': one or more(string)\n");
}

void PrintVersionInfo()
{
	wprintf(L"\n   mypdfsearch version 1.3.0\n");	
   
	wprintf(L"\n   Copyright (C) 2019 Vincenzo Lo Cicero\n\n");

	wprintf(L"   Author: Vincenzo Lo Cicero.\n");
	wprintf(L"   e-mail: vincenzolocicero68@gmail.com\n\n");
          
	wprintf(L"   mypdfsearch is free software: you can redistribute it and/or modify\n");
	wprintf(L"   it under the terms of the GNU General Public License as published by\n");
	wprintf(L"   the Free Software Foundation, either version 3 of the License, or\n");
	wprintf(L"   (at your option) any later version.\n\n");

	wprintf(L"   mypdfsearch is distributed in the hope that it will be useful,\n");
	wprintf(L"   but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	wprintf(L"   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	wprintf(L"   GNU General Public License for more details.\n\n");

	wprintf(L"   You should have received a copy of the GNU General Public License\n");
	wprintf(L"   along with mypdfsearch.  If not, see <http://www.gnu.org/licenses/>.\n\n");	
}

int myParseOption(Params *pParams, char *pszOption)
{
	int retValue = 1;
	
	int bLongName = 0;
	int bEqualCharFound = 0;
	
	char c;
	int x;
	int y;
	
	int n;
	
	
	char szName[MAX_LEN_STR + 1];
		
	szName[0] = '\0';
	
	x = 0;
	y = 0;
	n = 0;
	
	n = strnlen(pszOption, MAX_LEN_STR);
	if ( n < 2 )
	{
		retValue = 0;
		wprintf(L"\n\nError: unrecognized option '%s'\n\n", pszOption);
		goto uscita;
	}
	
	n = 0;
	
	
	x = 0;
	c = pszOption[x];
	if ( c != '-' )
	{
		retValue = 0;
		wprintf(L"\n\nError: invalid option '%s'\n\n", pszOption);
		goto uscita;
	}
	
	x = 1;
	c = pszOption[x];
	if ( c == '-' )
	{
		bLongName = 1;
		x++;	
		c = pszOption[x];
		szName[0] = c;
		szName[1] = '\0';
		y = 1;
		goto readlongname;
	}
	else if ( c == '\0' )
	{
		retValue = 0;
		wprintf(L"\n\nError: invalid option '%s'\n\n", pszOption);
		goto uscita;
	}
	else
	{
		szName[0] = c;
		szName[1] = '\0';
		x++;
				
		if ( c == 'h' )
		{
			pParams->bOptVersionOrHelp = 1;
			PrintHelpCommandLine();
			goto uscita;			
		}
		else if ( c == 'v' )
		{
			pParams->bOptVersionOrHelp = 1;
			PrintVersionInfo();
			goto uscita;
		}
				
		goto readvalue;
	}	
	
	
	
	readlongname:
	
	if ( bLongName )
	{
		c = pszOption[x];
		
		x++;
		c = pszOption[x];
		while ( c != '\0' )
		{				
			szName[y] = c;
			y++;
			
			if ( y > MAX_LEN_STR )
			{
				retValue = 0;
				wprintf(L"Error: option string too long.\n");
				goto uscita;
			}
			
			if ( c == '=' )
			{
				y--;
				break;
			}
				
			x++;
			c = pszOption[x];
		}
		
		if ( y < 0 )
		{
			retValue = 0;
			wprintf(L"\n\nError: invalid option '%s'\n\n", pszOption);
			goto uscita;			
		}
		
		szName[y] = '\0';
				
 		if ( strcmp(szName, "path") == 0 )
		{
			goto readvalue;
		}
		else if ( strcmp(szName, "words") == 0 )
		{
			goto readvalue;
		}
		else if ( strcmp(szName, "extracttextfrom") == 0 )
		{
			goto readvalue;
		}
		else if ( strcmp(szName, "frompage") == 0 )
		{
			goto readvalue;
		}	
		else if ( strcmp(szName, "topage") == 0 )
		{
			goto readvalue;
		}
		else if ( strcmp(szName, "outputfile") == 0 )
		{
			goto readvalue;
		}	
		else if ( strcmp(szName, "version") == 0 )
		{
			pParams->bOptVersionOrHelp = 1;
			PrintVersionInfo();
			goto uscita;
		}
		else if ( strcmp(szName, "help") == 0 )
		{
			pParams->bOptVersionOrHelp = 1;
			PrintHelpCommandLine();
			goto uscita;
		}		
		else
		{
			retValue = 0;
			wprintf(L"\n\nError: unrecognized option '%s'\n\n", pszOption);
			goto uscita;			
		}
	}

	readvalue:
		
	c = szName[0];
	y = 0;
	switch ( c )
	{
		case 'p':   // path
			c = pszOption[x];			
			while ( c != '\0' )
			{				
				pParams->szPath[y] = c;
				y++;
				if ( c == '=' )
				{
					y = 0;
					pParams->szPath[0] = '\0';
				}
				x++;
				c = pszOption[x];
			}
			pParams->szPath[y] = '\0';
			break;
		case 'w':   // words
			c = pszOption[x];			
			while ( c != '\0' )
			{
				pParams->szWordsToSearch[y] = c;
				y++;
				if ( c == '=' )
				{
					y = 0;
					pParams->szWordsToSearch[0] = '\0';
				}				
				x++;
				c = pszOption[x];
			}
			pParams->szWordsToSearch[y] = '\0';
			
			
			if ( NULL != pParams->pwszWordsToSearch )
			{
				free(pParams->pwszWordsToSearch);
				pParams->pwszWordsToSearch = NULL;
			}
			pParams->pwszWordsToSearch = myMultibyteToWidechar(pParams->szWordsToSearch, y);
			if ( NULL == pParams->pwszWordsToSearch )
			{
				retValue = 0;
				wprintf(L"\n\nError myParseOption: memory allocation failed for pwszWordsToSearch\n\n");
				goto uscita;				
			}
						
			if ( !SplitWords(pParams) )
			{
				retValue = 0;
				goto uscita;
			}
										
			break;		
		case 'e':   // extracttextfrom
			c = pszOption[x];			
			while ( c != '\0' )
			{
				pParams->szFilePdf[y] = c;
				y++;
				if ( c == '=' )
				{
					y = 0;
					pParams->szFilePdf[0] = '\0';
				}				
				x++;
				c = pszOption[x];
			}
			pParams->szFilePdf[y] = '\0';
			break;
		case 'f':   // frompage
			n = 0;
			c = pszOption[x];			
			while ( c != '\0' )
			{
				if ( c >= '0' && c <= '9' )
				{
					n = n * 10 + c - '0';
					x++;
					c = pszOption[x];
				}
				else if ( c == '=' )
				{
					bEqualCharFound = 1;
					x++;
					c = pszOption[x];
				}
				else
				{
					if ( bEqualCharFound )
					{
						retValue = 0;
						wprintf(L"\n\nError: invalid option value -> '%s'. Must be a integer number\n\n", pszOption);
						goto uscita;
					}
					
					x++;
					c = pszOption[x];					
				}
			}
			pParams->fromPage = n;
			break;
		case 't':   // topage
			n = 0;
			c = pszOption[x];			
			while ( c != '\0' )
			{
				if ( c >= '0' && c <= '9' )
				{
					n = n * 10 + c - '0';
					x++;
					c = pszOption[x];
				}
				else if ( c == '=' )
				{
					bEqualCharFound = 1;
					x++;
					c = pszOption[x];
				}
				else
				{
					if ( bEqualCharFound )
					{
						retValue = 0;
						wprintf(L"\n\nError: invalid option value -> '%s'. Must be a integer number\n\n", pszOption);
						goto uscita;
					}
					
					x++;
					c = pszOption[x];					
				}
			}
			pParams->toPage = n;
			break;
		case 'o':   // outputfile
			c = pszOption[x];			
			while ( c != '\0' )
			{
				pParams->szOutputFile[y] = c;
				y++;
				if ( c == '=' )
				{
					y = 0;
					pParams->szOutputFile[0] = '\0';
				}				
				x++;
				c = pszOption[x];
			}
			pParams->szOutputFile[y] = '\0';
			break;			
		default:
			retValue = 0;
			wprintf(L"\n\nError: unrecognized option '%s'\n\n", pszOption);
			goto uscita;			
			break;		
	}
	
	uscita:
	
	if ( NULL != pParams->pwszWordsToSearch )
	{
		free(pParams->pwszWordsToSearch);
		pParams->pwszWordsToSearch = NULL;
	}

	return retValue;
}

int myParseCommandLine(Params *pParams, int argc, char **argv)
{
	int retValue = 1;
	
	int x;
	int n;
	
	pParams->bOptVersionOrHelp = 0;
	pParams->szPath[0] = '\0';
	pParams->szFilePdf[0] = '\0';
	pParams->fromPage = 0;
	pParams->toPage = 0;
	pParams->szOutputFile[0] = '\0';
	pParams->szWordsToSearch[0] = '\0';	
	pParams->countWordsToSearch = 0;
	
	pParams->pwszWordsToSearch = NULL;
	
	pParams->pWordsToSearchArray = NULL;
	
	
	pParams->pWordsToSearchArray = (wchar_t**)malloc(sizeof(wchar_t*) * MAX_WORDS_TO_SEARCH);
	if ( NULL == pParams->pWordsToSearchArray )
	{
		retValue = 0;
		wprintf(L"\n\nError myParseOption: memory allocation failed for pWordsToSearchArray\n\n");
		goto uscita;				
	}
	for ( n = 0; n < MAX_WORDS_TO_SEARCH; n++ )
	{
		pParams->pWordsToSearchArray[n] = NULL;
	}	
			
	x = 1;
	if ( argc > 1 )
	{
		while ( x < argc )
		{
			retValue = myParseOption(pParams, argv[x]);
			if ( !retValue )
				goto uscita;
			x++;
		}
	}
	else
	{
		retValue = 0;
	}
		
	if ( !(pParams->bOptVersionOrHelp) && (pParams->szFilePdf[0] == '\0' && pParams->szPath[0] == '\0') )
	{
		wprintf(L"\n\nErrore: dev'essere specificata una(e solo una, non entrambe) delle due opzioni: '-p (--path)' o '-e(--extracttextfrom)'\n\n");
		return 0;
	}
	
	if ( !(pParams->bOptVersionOrHelp) && (pParams->szFilePdf[0] != '\0' && pParams->szPath[0] != '\0') )
	{
		wprintf(L"\n\nErrore: dev'essere specificata soltanto una delle due opzioni mutualmente esclusive: '-p (--path)' o '-e(--extracttextfrom)'\n\n");
		return 0;
	}
	
	if ( !(pParams->bOptVersionOrHelp) && (pParams->toPage > 0 && pParams->toPage < pParams->fromPage) )
	{
		wprintf(L"\n\nErrore: il valore specificato per l'opzione topage e' minore del valore specificato per l'opzione frompage. Dev'essere maggiore o uguale.\n\n");
		return 0;
	}
	
	if ( !(pParams->bOptVersionOrHelp) && (pParams->szPath[0] != '\0' && pParams->szWordsToSearch[0] == '\0') )
	{
		wprintf(L"\n\nErrore: bisogna specificare le parole da cercare tramite l'opzione '-w (--words)'\n\n");
		return 0;		
	}
	
	if ( !(pParams->bOptVersionOrHelp) && (pParams->szPath[0] == '\0' && pParams->szWordsToSearch[0] != '\0') )
	{
		wprintf(L"\n\nErrore: bisogna specificare la cartella contenente i file pdf tramite l'opzione '-p (--path)'\n\n");
		return 0;		
	}
	
uscita:
	
	return retValue;
}

int MakeAndOpenOutputFile(Params *pParams)
{
	int retValue = 1;
			
	#if !defined(_WIN64) && !defined(_WIN32)
	unsigned char szBOM[21];
	// UTF-8 BOM -> EF BB BF 
	szBOM[0] = 0xEF;
	szBOM[1] = 0xBB;
	szBOM[2] = 0xBF;
	szBOM[3] = '\0';
	#endif
	
	pParams->fpOutput = fopen(pParams->szOutputFile, "rb");
	if ( pParams->fpOutput != NULL )
	{
		wprintf(L"\n\nERRORE: il file specificato per l'output esiste già.\nBisogna indicare il nome di un file non esistente.\n\n");
		retValue = 0;
		goto uscita;
	}
	
	pParams->fpOutput = fopen(pParams->szOutputFile, "wb");
	if ( pParams->fpOutput == NULL )
	{
		wprintf(L"\n\nERRORE: impossibile creara il file specificato per l'output: '%s'.\n\n", pParams->szOutputFile);
		retValue = 0;
		goto uscita;
	}	
	
	#if defined(_WIN64) || defined(_WIN32)
	_setmode(_fileno(pParams->fpOutput), _O_U8TEXT);
	#endif	
	
	#if !defined(_WIN64) && !defined(_WIN32)
	// UTF-8 BOM -> EF BB BF 
	fwprintf(pParams->fpOutput, L"%s", szBOM);
	#endif
		
	uscita:
	
	return retValue;
}

int MakeAndOpenErrorsFile(Params *pParams)
{
	int retValue = 1;
			
	#if !defined(_WIN64) && !defined(_WIN32)
	unsigned char szBOM[21];
	// UTF-8 BOM -> EF BB BF 
	szBOM[0] = 0xEF;
	szBOM[1] = 0xBB;
	szBOM[2] = 0xBF;
	szBOM[3] = '\0';
	#endif
	
	pParams->fpErrors = fopen("AAA_mypdfsearch_parsing_errors.txt", "wb");
	if ( pParams->fpErrors == NULL )
	{
		wprintf(L"\n\nERRORE: impossibile creara il file specificato per i messaggi d'errore.\n\n");
		retValue = 0;
		goto uscita;
	}	
	
	#if defined(_WIN64) || defined(_WIN32)
	_setmode(_fileno(pParams->fpErrors), _O_U8TEXT);	
	#endif	
	
	#if !defined(_WIN64) && !defined(_WIN32)
	// UTF-8 BOM -> EF BB BF 
	fwprintf(pParams->fpErrors, L"%s", szBOM);
	#endif
		
	uscita:
	
	return retValue;
}

void checkEndianness(Params *pParams)
{
	//int i = 0xDEED1234;
	//int i = 0xABCDEF12;
	uint32_t i = 0xABCDEF12;
	//int j;
	unsigned char *buff;

	//wprintf(L"\nvalue = %lX\n", i);

	buff = (unsigned char*)&i;
	
	pParams->nThisMachineEndianness = MACHINE_ENDIANNESS_UNKNOWN;

	if( buff[0] == 0xAB )
	{
		//wprintf(L"This machine is BIG endian - HIGHER order bytes come first");
		pParams->nThisMachineEndianness = MACHINE_ENDIANNESS_BIG_ENDIAN;
	}
	else
	{
		pParams->nThisMachineEndianness = MACHINE_ENDIANNESS_LITTLE_ENDIAN;
		//wprintf(L"This machine is LITTLE endian - LOWER order bytes come first");
	}

	//wprintf(L"\nhere are the 4 bytes\n");

	//for( j = 0; j < 4; j++ )
	//{
	//	wprintf(L"byte [%d] = %X\n", j, buff[j]);
	//}

	//wprintf(L"\n");
}

/*
IMPORTANTE, ATTENZIONE!!! compilare su Windows con MINGW64, richiede di specificare '-liconv' alla fine, dopo '-lz'
                          altrimenti il linker dà errore.
*/

/*
MINGW:
 
gcc -Wall -W -pedantic -Wno-overlength-strings -O3 -std=c99 -D_GNU_SOURCE myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz -liconv

vanno bene entrambe le modalità: e con slash, e con backslash:
mypdfsearch --path="C:\AAA_ProgrammiLibrerie\myPdfSearch\Files\Prova" --words="Virginia campidoglio Orbán"
mypdfsearch --path="C:/AAA_ProgrammiLibrerie/myPdfSearch/Files/Prova" --words="Virginia campidoglio Orbán"
*/

/*
MSYS2:

cd /c/AAA_ProgrammiLibrerie/myPdfSearch

gcc -Wall -W -pedantic -Wno-overlength-strings -O3 -std=c99 -D_GNU_SOURCE myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz -liconv


C:\AAA_ProgrammiLibrerie\myPdfSearch\Files
./mypdfsearch --path="/c/AAA_ProgrammiLibrerie/myPdfSearch/Files/Prova" --words="Virginia campidoglio Orbán"
./mypdfsearch --path="C:\AAA_ProgrammiLibrerie\myPdfSearch\Files\Prova" --words="Virginia campidoglio Orbán"
 
mypdfsearch --path="C:\AAA_ProgrammiLibrerie\myPdfSearch\Files\Prova" --words="Virginia campidoglio Orbán"
mypdfsearch --path="C:/AAA_ProgrammiLibrerie/myPdfSearch/Files/Prova" --words="Virginia campidoglio Orbán"
*/

/*
ATTENZIONE: IMPORTANTE:
Se si vuole compilare con l'opzione -str=c99, bisogna aggiungere anche -D_GNU_SOURCE, altrimenti succede il bordello:
errori e warning stravaganti e sensa senso.

gcc -Wall -Wextra -pedantic -Wno-overlength-strings -O0 -g -std=c99 -D_GNU_SOURCE myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearchdebug -lz
 
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --words="Virginia campidoglio Orbán" --path="../Files/Tutti/SottoCartella"

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --words="Virginia campidoglio Orbán" --path="../Files/File08"
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/File08/La Repubblica - 9 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_08.txt" 

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/File03/Il Giornale - 8 aprile 2019.pdf" --frompage=1 --topage=1 --outputfile="AAA_OUTPUT_03.txt"
 
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/File01/Corriere della Sera - 20 luglio 2019.pdf" --frompage=1 --topage=1

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="../Files/FileProblematici/La Stampa - 20 luglio 2019 .pdf" --frompage=3 --topage=4

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --words="Virginia campidoglio Orbán" --path="../Files/File01" --frompage=6 --topage=6

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --words="Virginia campidoglio Orbán" --path="../Files/FileProblematici" --frompage=3 --topage=4

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --words="Virginia campidoglio Orbán" --path="../Files/File03"

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug -path="/home/vincenzo/Varie/GCC/Varie/Files/Giornali" --words="Virginia campidoglio Orbán"

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --path="../Files/FileProblematico" --words="Virginia campidoglio Orbán"
 
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="/home/vincenzo/Varie/GCC/Varie/Files/Giornali/A2018/Q20180103/Libero - 3 Gennaio 2018.pdf" --frompage=2 --topage=2
 
valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug --extracttextfrom="/home/vincenzo/Varie/GCC/Varie/Files/Manzoni/manzoni_i_promessi_sposi.pdf" --frompage=254 --topage=254

valgrind --leak-check=full --show-reachable=yes --track-origins=yes --log-file=AAA_outputValgrind.txt ./mypdfsearchdebug -path="/home/vincenzo/Varie/GCC/Varie/Files/Giornali" --words="Virginia Orbán Milano"

-------------------------------------------------------------------------------------------------------------------------------------

gcc -Wall -W -pedantic -Wno-overlength-strings -O3 -std=c99 -D_GNU_SOURCE myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c myobjrefqueuelist.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz
gcc -Wall -W -pedantic -Wno-overlength-strings -O3 myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myTernarySearchTree.c myScopeHashTable.c mydictionaryqueuelist.c myobjrefqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -o mypdfsearch -lz
 
./mypdfsearch --path="/home/vincenzo/Varie/GCC/Varie/Files/Giornali" --words="Virginia Orbán branco"
  
./mypdfsearch --path="/home/vincenzo/Varie/GCC/Varie/Files/Giornali/Manzoni" --words="Virginia branco Milano"
  
./mypdfsearch --extracttextfrom="/home/vincenzo/Varie/GCC/Varie/Files/Giornali/Manzoni/manzoni_i_promessi_sposi.pdf" --frompage=254 --topage=254

 
Per generare le dipendenze per il Makefile, usare -MM:
gcc -MM -D_GNU_SOURCE myGenHashTable.c myInitPredefCMapHT.c myinitarray.c myoctal.c myScopeHashTable.c mydictionaryqueuelist.c mystringqueuelist.c mycontentqueuelist.c mynumstacklist.c myintqueuelist.c mydecode.c scanner.c parser.c main.c -lz
*/

int main(int argc, char **argv)
{
	int retValue = EXIT_SUCCESS;
	
	Params myParams;
					
	FilesList* myFilesList = NULL;
	
	//long numProcessors;
		
	int len;
		
	myParams.countWordsToSearch = 0;
	myParams.pWordsToSearchArray = NULL;
	
	myParams.bOptVersionOrHelp = 0;
	myParams.szPath[0] = '\0';
	myParams.szFilePdf[0] = '\0';
	myParams.fromPage = 0;
	myParams.toPage = 0;
	myParams.szOutputFile[0] = '\0';
	myParams.szWordsToSearch[0] = '\0';	
	myParams.countWordsToSearch = 0;
	
	myParams.fpOutput = NULL;
	
	myParams.fpErrors = NULL;
	
#if defined(_WIN64) || defined(_WIN32)
	//SetConsoleOutputCP(65001);
	//_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stdout), _O_U8TEXT);
#else
	//if ( !setlocale(LC_CTYPE, "it_IT.utf8") )
	if ( !setlocale(LC_ALL, "C.UTF-8") )
	{
		wprintf(L"Impossibile selezionare utf8!\n");
		printf("Impossibile selezionare utf8!\n");
		retValue = EXIT_FAILURE;
		goto uscita;
	}
#endif	

	if ( !MakeAndOpenErrorsFile(&myParams) )
	{
		retValue = EXIT_FAILURE;
		goto uscita;
	}
	
	//numProcessors = GetProcessors();
	//wprintf(L"\n\nQUESTA MACCHINA HA %ld PROCESSORI.\n\n", numProcessors);
	
	checkEndianness(&myParams);

	if ( !myParseCommandLine(&myParams, argc, argv) )
	{
		retValue = EXIT_FAILURE;
		goto uscita;
	}
		
	if ( myParams.bOptVersionOrHelp )
	{
		retValue = EXIT_SUCCESS;
		goto uscita;
	}
		
	if ( myParams.szOutputFile[0] != '\0' )
	{
		if ( !MakeAndOpenOutputFile(&myParams) )
		{
			retValue = EXIT_FAILURE;
			goto uscita;
		}
	}
		
	if ( myParams.szFilePdf[0] != '\0' )
	{
		#if !defined(_WIN64) && !defined(_WIN32)
		char* psz = NULL;
		int x;
		int y;		
		#endif
		
		myFilesList = (FilesList*)malloc(sizeof(FilesList));
		
		if( myFilesList == NULL )
		{
			wprintf(L"Errore main: impossibile aggiungere il file alla lista.\n");
			retValue = EXIT_FAILURE;
			goto uscita;			
		}

		#if !defined(_WIN64) && !defined(_WIN32)
		strcpy(myFilesList->myPathName, myParams.szFilePdf);
		myFilesList->next = NULL;
						
		x = strlen(myFilesList->myPathName);
		while ( x >= 0 )
		{
			if ( myFilesList->myPathName[x] == '/' )
			{
				myFilesList->myPathName[x] = '\0';
				break;
			}
			x--;
		}
		
		if ( x > 0 )
			x++;
		psz = &(myParams.szFilePdf[x]);
		y = 0;
		while ( *psz != '\0' )
			myFilesList->myFileName[y++] = *(psz++);
		myFilesList->myFileName[y] = '\0';		
		#else
		strcpy(myFilesList->myFileName, myParams.szFilePdf);
		myFilesList->next = NULL;	
		#endif
	}
	else
	{
		len = strnlen(myParams.szPath, MAX_LEN_STR);
		myFilesList = getFilesRecursive(myParams.szPath, len, myFilesList);	
		if ( !myFilesList )
		{
			wprintf(L"Errore: impossibile ottenere la lista dei file.\n");
			retValue = EXIT_FAILURE;
			goto uscita;
		}
	}
				
	wprintf(L"\n");
			
	Parse(&myParams, myFilesList, 0);
				
uscita:
	
	
	freeFilesList(myFilesList);
		
	if ( NULL != myParams.pWordsToSearchArray )
	{
		int myIndex;
				
		for ( myIndex = 0; myIndex < myParams.countWordsToSearch; myIndex++ )
		{
			if ( NULL != myParams.pWordsToSearchArray[myIndex] )
			{
				free(myParams.pWordsToSearchArray[myIndex]);
				myParams.pWordsToSearchArray[myIndex] = NULL;
			}

		}		
		free(myParams.pWordsToSearchArray);
		myParams.pWordsToSearchArray = NULL;
	}
	
	wprintf(L"\n");
	
	if ( NULL != myParams.fpErrors )
		fclose(myParams.fpErrors);
				
	return retValue;
}
