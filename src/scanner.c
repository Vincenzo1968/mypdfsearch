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
#include <string.h>
#include <malloc.h>

#include "myoctal.h"
#include "scanner.h"

int ReadTrailerBody(Params *pParams, unsigned char *szInput, int index)
{
	unsigned char c;
	TrailerStates myState = S_Error;
	int x;
	char szTemp[1024];
	int bSizeYes, bRootYes;
		
	c = szInput[index++];		
	while ( (c == '\n' || c == '\r' || c == '\t' || c == ' ' || c == '\f' || c == '\b') )
	{
		c = szInput[index++];
	}
	
	bSizeYes = bRootYes = 0;
	pParams->myPdfTrailer.Prev = 0;
	
	if ( c != '<' )
		return 0;			
		
	c = szInput[index++];		
	
	if ( c != '<' )
		return 0;			

ciclo1:

	c = szInput[index++];
	while ( (c == '\n' || c == '\r' || c == '\t' || c == ' ' || c == '\f' || c == '\b') )
	{
		c = szInput[index++];
	}	
	
ciclo2:	
	if ( c == '>' )
		goto ciclo3;
	
	if ( c != '/' )
		return 0;
				
	c = szInput[index++];
	switch ( c )
	{
		case 'S':
			c = szInput[index++];
			if ( c != 'i' )
				return 0;
			c = szInput[index++];
			if ( c != 'z' )
				return 0;
			c = szInput[index++];
			if ( c != 'e' )
				return 0;	
			c = szInput[index++];
			if ( (c != ' ' && c != '\r' && c != '\n' && c != '\t' && c != '\f' && c != '\b') )
				return 0;
			myState = S_Size;
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
			wprintf(L"/Size OK\n");
			#endif
			break;
		case 'P':
			c = szInput[index++];			
			if ( c != 'r' )
				return 0;
			c = szInput[index++];
			if ( c != 'e' )
				return 0;
			c = szInput[index++];
			if ( c != 'v' )
				return 0;	
			c = szInput[index++];
			if ( (c != ' ' && c != '\r' && c != '\n' && c != '\t' && c != '\f' && c != '\b') )
				return 0;
			myState = S_Prev;
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
			wprintf(L"/Prev OK\n");
			#endif
			break;
		case 'R':
			c = szInput[index++];
			if ( c != 'o' )
				return 0;
			c = szInput[index++];
			if ( c != 'o' )
				return 0;
			c = szInput[index++];
			if ( c != 't' )
				return 0;	
			c = szInput[index++];
			if ( (c != ' ' && c != '\r' && c != '\n' && c != '\t' && c != '\f' && c != '\b') )
				return 0;
			myState = S_Root;
			#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
			wprintf(L"/Root OK\n");
			#endif
			break;
		case 'I':
			c = szInput[index++];
			if ( c == 'D' )
			{
				myState = S_ID;
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
				wprintf(L"/ID OK\n");
				#endif
				c = szInput[index++];
				if ( (c != '[' && c != ' ' && c != '\r' && c != '\n' && c != '\t' && c != '\f' && c != '\b') )
					return 0;								
			}
			else if ( c == 'n' )
			{
				c = szInput[index++];
				if ( c != 'f' )
					return 0;
				c = szInput[index++];
				if ( c != 'o' )
					return 0;
				myState = S_Info;
				#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
				wprintf(L"/Info OK\n");
				#endif
				c = szInput[index++];
				if ( (c != ' ' && c != '\r' && c != '\n' && c != '\t' && c != '\f' && c != '\b') )
					return 0;				
			}
			else
			{
				return 0;
			}
			break;			
		case 'E':
			pParams->isEncrypted = 1;
			return 0;
			break;						
		default:
			return 0;
	}
	
	
	c = szInput[index++];
	while ( (c == '\n' || c == '\r' || c == '\t' || c == ' ' || c == '\f' || c == '\b') )
	{
		c = szInput[index++];
	}
	if ( c == '\0' )
		return 0;
	
	switch ( myState )
	{
		case S_Size:
			x = 0;
			while ( c >= '0' && c <= '9' && x < 1024 )
			{
				szTemp[x++] = c;
				c = szInput[index++];
			}
			if ( x <= 0 || x >= 1024 )
				return 0;
			szTemp[x] = '\0';
			if ( pParams->myPdfTrailer.Size <= 0 )
				pParams->myPdfTrailer.Size = atoi(szTemp);
			bSizeYes = 1;
			break;
		case S_Prev:
			x = 0;
			while ( c >= '0' && c <= '9' && x < 1024 )
			{
				szTemp[x++] = c;
				c = szInput[index++];
			}
			if ( x <= 0 || x >= 1024 )
				return 0;
			szTemp[x] = '\0';
			pParams->myPdfTrailer.Prev = atoi(szTemp);
			break;
		case S_Root:
			x = 0;
			while ( c >= '0' && c <= '9' && x < 1024 )
			{
				szTemp[x++] = c;
				c = szInput[index++];
			}
			if ( x <= 0 || x >= 1024 )
				return 0;
			szTemp[x] = '\0';
			if ( pParams->myPdfTrailer.Root.Number <= 0 )
				pParams->myPdfTrailer.Root.Number = atoi(szTemp);
			
			if ( (c != ' ' && c != '\r' && c != '\n' && c != '\t' && c != '\f' && c != '\b') )
				return 0;				
			c = szInput[index++];
			while ( (c == '\n' || c == '\r' || c == '\t' || c == ' ' || c == '\f' || c == '\b') )
			{
				c = szInput[index++];
			}
			if ( c == '\0' )
				return 0;
				
			x = 0;			
			while ( c >= '0' && c <= '9' && x < 1024 )
			{
				szTemp[x++] = c;
				c = szInput[index++];
			}
			if ( x <= 0 || x >= 1024 )
				return 0;
			szTemp[x] = '\0';
			if ( pParams->myPdfTrailer.Root.Number <= 0 )
				pParams->myPdfTrailer.Root.Generation = atoi(szTemp);
			
			if ( (c != ' ' && c != '\r' && c != '\n' && c != '\t' && c != '\f' && c != '\b') )
				return 0;				
			c = szInput[index++];
			while ( (c == '\n' || c == '\r' || c == '\t' || c == ' ' || c == '\f' || c == '\b') )
			{
				c = szInput[index++];
			}
			if ( c != 'R' )
				return 0;
			c = szInput[index++];
			if ( (c != '/' && c != ' ' && c != '\r' && c != '\n' && c != '\t' && c != '\f' && c != '\b') )
				return 0;
						
			bRootYes = 1;
			break;
		case S_ID:
			while ( c != '\0' && c != ']' )
			{
				c = szInput[index++];
			}
			if ( c == '\0' )
				return 0;
			c = szInput[index++];
			break;		
		case S_Info:		
			while ( c != '\0' && c != '/' && c != '>' )
			{
				c = szInput[index++];
			}
			if ( c == '\0' )
				return 0;
			break;
		default:
			return 0;
	}
			
	if ( c == '/' )
		goto ciclo2;
	else if ( c == '>' )
		goto ciclo3;
	else
		goto ciclo1;
	
ciclo3:

	c = szInput[index++];
	if ( c != '>' )
		return 0;
		
	if ( !bSizeYes )
	{
		//wprintf(L"Errore ReadTrailerBody: manca la voce 'Size' nel trailer\n");
		fwprintf(pParams->fpErrors, L"Errore ReadTrailerBody: manca la voce 'Size' nel trailer\n");
		return 0;
	}

	if ( !bRootYes )
	{
		wprintf(L"Errore ReadTrailerBody: manca la voce 'Root' nel trailer\n");
		fwprintf(pParams->fpErrors, L"Errore ReadTrailerBody: manca la voce 'Root' nel trailer\n");
		return 0;
	}
		
	return 1;
}

int GetLenNextInput(Params *pParams, int startxref, int *len)
{
	int retValue = 1;
	unsigned char *myBlock = NULL;
	
	unsigned char c;
	int blockLen = 0;
	int index1 = 0;	
	
	int eofOK = 0;
		
	States state = S0;
	
	if ( fseek(pParams->fp, startxref, SEEK_SET) )
	{
		wprintf(L"Errore GetLenNextInput fseek\n");
		fwprintf(pParams->fpErrors, L"Errore GetLenNextInput fseek\n");
		retValue = 0;
		goto uscita;
	}	
	
	myBlock = (unsigned char *)malloc(sizeof(unsigned char) * BLOCK_SIZE);
	if ( !myBlock )
	{
		wprintf(L"Errore GetLenNextInput: memoria insufficiente.\n");
		fwprintf(pParams->fpErrors, L"Errore GetLenNextInput: memoria insufficiente.\n");
		retValue = 0;
		goto uscita;
	}
	
	*len = 0;
	while ( (blockLen = fread(myBlock, 1, BLOCK_SIZE, pParams->fp)) )
	{
		index1 = 0;
		while ( index1 < blockLen )
		{		
			c = myBlock[index1++];
			(*len)++;
			
			switch ( state )
			{
				case S0:
					if ( c == '%' )
						state = S1;
					break;
				case S1:
					if ( c == '%' )
						state = S2;
					else
						state = S0;
					break;
				case S2:
					if ( c == 'E' )
						state = S3;
					else
						state = S0;
					break;
				case S3:
					if ( c == 'O' )
						state = S4;
					else
						state = S0;										
					break;
				case S4:
					if ( c == 'F' )
						state = S5;
					else
						state = S0;						
					break;
				case S5:
					if ( c == '\r' || c == '\n' )
						eofOK = 1;	
					else
						state = S0;
					break;
				default:
					state = S0;
					break;					
			}
		
			if ( eofOK )
				break;
		}
	}
	
	if ( !eofOK )
		retValue = 0;
	
uscita:

	if ( NULL != myBlock )
		free(myBlock);
		
	return retValue;
}

int ReadLastTrailer(Params *pParams, unsigned char *szInput)
{
	int index;
	unsigned char c;
	
	char *pszTemp;
	
	index = 0;
	
	c = szInput[index++];	
	if ( c != 'x' )
	{
		//wprintf(L"Errore ReadLastTrailer: atteso 'x' trovato '%c'\n", c);
		fwprintf(pParams->fpErrors, L"Errore ReadLastTrailer: atteso 'x' trovato '%c'\n", c);
		pszTemp = (char*)szInput + index - 1;
		pszTemp[index + 144] = '\0';
		//wprintf(L"szInput = <%s>\n", pszTemp);
		fwprintf(pParams->fpErrors, L"szInput = <%s>\n", pszTemp);
		
		return 0;
	}

	c = szInput[index++];	
	if ( c != 'r' )
		return 0;

	c = szInput[index++];	
	if ( c != 'e' )
		return 0;

	c = szInput[index++];	
	if ( c != 'f' )
		return 0;
				
	while ( c != '\0' && c != 't' )
	{
		c = szInput[index++];
	}
	if ( c == '\0' )
		return 0;
				
	c = szInput[index - 2];
	if ( (c != '\n' && c != '\r' && c != '\t' && c != ' ' && c != '\f' && c != '\b') )
		return 0;
		
	
	c = szInput[index++];
	if ( c != 'r' )
		return 0;
		
	c = szInput[index++];
	if ( c != 'a' )
		return 0;
		
	c = szInput[index++];
	if ( c != 'i' )
		return 0;
		
	c = szInput[index++];
	if ( c != 'l' )
		return 0;
		
	c = szInput[index++];
	if ( c != 'e' )
		return 0;
		
	c = szInput[index++];
	if ( c != 'r' )
		return 0;
			
	c = szInput[index];
	if ( (c != '\n' && c != '\r' && c != '\t' && c != ' ' && c != '\f' && c != '\b' && c != '<') )
		return 0;
						
	if ( !ReadTrailerBody(pParams, szInput, index) )
		return 0;
			
	return 1;
}

int ReadSubSectionBody(Params *pParams, unsigned char *szInput, int fromNum, int numObjs, int *index)
{
	int x;
	unsigned char c;
	unsigned char c2;
	char szTemp[1024];
	int numInit;
	int toNum;
	
	int Offset;
	int GenNum;
	
	numInit = fromNum;
	toNum = fromNum + numObjs - 1;
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
	wprintf(L"ReadSubSectionBody: fromNum %d toNum %d\n", fromNum, toNum);
	#endif
		
	while ( numInit <= toNum )	
	{		
		for ( x = 0; x < 10; x++ )
		{
			c = szInput[(*index)++];
			if ( c < '0' || c > '9' )
				return 0;
			szTemp[x] = c;			
		}
		szTemp[x] = '\0';
						
		Offset = atoi(szTemp);
				
		if ( szInput[(*index)++] != ' ' )
			return 0;
					
		for ( x = 0; x < 5; x++ )
		{
			c = szInput[(*index)++];
			if ( c < '0' || c > '9' )
				return 0;
			szTemp[x] = c;			
		}
		szTemp[x] = '\0';
				
		GenNum = atoi(szTemp);
				
		if ( szInput[(*index)++] != ' ' )
			return 0;
							
		c = szInput[(*index)++];
		
		if ( c != 'f' && c != 'n' )
			return 0;
									
		if ( NULL == pParams->myObjsTable[numInit] )
		{
			pParams->myObjsTable[numInit] = (PdfObjsTableItem *)malloc(sizeof(PdfObjsTableItem));
			if ( !(pParams->myObjsTable) )
			{
				//wprintf(L"Memoria insufficiente per allocare l'oggetto numero %d sulla tabella degli oggetti.\n\n", numInit);
				fwprintf(pParams->fpErrors, L"Memoria insufficiente per allocare l'oggetto numero %d sulla tabella degli oggetti.\n\n", numInit);
				return 0;
			}
			
			pParams->myObjsTable[numInit]->Obj.Number = numInit;
			pParams->myObjsTable[numInit]->Obj.Generation = GenNum;
			pParams->myObjsTable[numInit]->Offset = Offset;
			pParams->myObjsTable[numInit]->numObjParent = -1;
			myobjreflist_Init(&(pParams->myObjsTable[numInit]->myXObjRefList));
			myobjreflist_Init(&(pParams->myObjsTable[numInit]->myFontsRefList));
		}
						
		c = szInput[(*index)++];
		switch( c )
		{
		case ' ':
			c2 = szInput[(*index)++];
			if ( c2 != '\r' && c2 != '\n' )
			{
				//wprintf(L"ERRORE ReadSubSectionBody 1: c = '%c', c2 = '%c'\n", c, c2);
				fwprintf(pParams->fpErrors, L"ERRORE ReadSubSectionBody 1: c = '%c', c2 = '%c'\n", c, c2);
				return 0;
			}
			break;
		case '\r':
			c2 = szInput[(*index)++];
			if ( c2 != '\n' )
			{
				//wprintf(L"ERRORE ReadSubSectionBody 2: c = '%c', c2 = '%c'\n", c, c2);
				fwprintf(pParams->fpErrors, L"ERRORE ReadSubSectionBody 2: c = '%c', c2 = '%c'\n", c, c2);
				return 0;	
			}
			break;
		default:
			//wprintf(L"Errore ReadSubSectionBody: atteso spazio trovato '%c'\n", c);
			fwprintf(pParams->fpErrors, L"Errore ReadSubSectionBody: atteso spazio trovato '%c'\n", c);
			return 0;
			break;
		}
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
		wprintf(L"ReadSubSectionBody: <ObjNum = %d> <Offset = %d> <GenNum = %d>\n", numInit, Offset, GenNum);
		#endif
		
		numInit++;
	}
		
	return 1;
}

int ReadObjsTable(Params *pParams, unsigned char *szInput, int bIsLastTrailer)
{
	int retValue = 1;
	int x;
	int index;
	unsigned char c;
	char szTemp[1024];
		
	int fromNum;
	int numObjs;
	
	if ( bIsLastTrailer )
	{	
		if ( pParams->myObjsTable )
		{
			free(pParams->myObjsTable);
			pParams->myObjsTable = NULL;
		}
	
		pParams->myObjsTable = (PdfObjsTableItem **)malloc(sizeof(PdfObjsTableItem*) * pParams->myPdfTrailer.Size);
		if ( !(pParams->myObjsTable) )
		{
			//wprintf(L"Memoria insufficiente per la tabella degli oggetti.\n\n");
			fwprintf(pParams->fpErrors,L"Memoria insufficiente per la tabella degli oggetti.\n\n");
			retValue = 0;
			goto uscita;
		}
	
		for ( x = 0; x < pParams->myPdfTrailer.Size; x++ )
		{
			pParams->myObjsTable[x] = NULL;
		}
	}
		
	// saltiamo "xref"
	index = 4;
	
	c = szInput[index];
	
	if ( (c != '\n' && c != '\r' && c != '\t' && c != ' ' && c != '\f' && c != '\b') )
	{
		retValue = 0;
		goto uscita;		
	}
			
	while ( 1 )
	{		
		c = szInput[index++];
						
		while ( (c == '\n' || c == '\r' || c == '\t' || c == ' ' || c == '\f' || c == '\b') )
		{
			c = szInput[index++];
		}
		
		if ( c == 't' || c == '\0' )
		{
			break;
		}		
				
		if ( c < '0' || c > '9' )
		{
			retValue = 0;
			goto uscita;
		}
				
		x = 0;
		szTemp[x++] = c;		
		while ( c >= '0' && c <= '9' )
		{
			c = szInput[index++];
			szTemp[x++] = c;
		}
		szTemp[x] = '\0';
		x = 0;		
		
		if ( c != ' ' )
		{
			retValue = 0;
			goto uscita;		
		}		
		
		fromNum = atoi(szTemp);

		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
		wprintf(L"ReadObjsTable: fromNum = %d\n", fromNum);
		#endif
				
		c = szInput[index++];
		
		if ( c < '0' || c > '9' )
		{
			retValue = 0;
			goto uscita;
		}		
		
		x = 0;
		szTemp[x++] = c;		
		while ( c >= '0' && c <= '9' )
		{
			c = szInput[index++];
			szTemp[x++] = c;
		}
		szTemp[x] = '\0';
		x = 0;
		
		numObjs = atoi(szTemp);
		
		#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
		wprintf(L"ReadObjsTable: numObjs = %d\n", numObjs);
		#endif
								
		if ( c != '\r' && c != '\n' )
		{
			retValue = 0;
			goto uscita;
		}
		
		while ( c == '\r' || c == '\n' )
		{
			c = szInput[index++];
		}
		
		if ( c >= '0' && c <= '9' )
			index--;
								
		if ( !ReadSubSectionBody(pParams, szInput, fromNum, numObjs, &index) )
		{
			retValue = 0;
			goto uscita;			
		}		
	}
	
	if ( c != 't' )
	{
		retValue = 0;
		goto uscita;
	}
	
uscita:	
		
	return retValue;
}

int ReadHeader(Params *pParams)
{
	int bytereads = 0;

	unsigned char c;
	
	int idx;
	int idxVersion;

	unsigned char szTemp[BLOCK_SIZE];
	
	pParams->szPdfVersionFromCatalog[0] = '\0';

	if ( fseek(pParams->fp, 0, SEEK_CUR) )
	{
		//wprintf(L"Errore ReadHeader fseek\n");
		fwprintf(pParams->fpErrors, L"Errore ReadHeader fseek\n");
		return 0;
	}		
	
	bytereads = fread(szTemp, 1, BLOCK_SIZE, pParams->fp);
	if ( bytereads <= 5 )
	{
		//wprintf(L"Errore ReadHeader fread\n");
		fwprintf(pParams->fpErrors, L"Errore ReadHeader fread\n");
		return 0;
	}
	
	idx = idxVersion = 0;
	while ( idx < bytereads )
	{
		c = szTemp[idx];
		if ( '\n' == c || '\r' == c || idx > 127 )
		{
			pParams->szPdfHeader[idx] = '\0';
			pParams->szPdfVersion[idxVersion] = '\0';
			break;
		}
		pParams->szPdfHeader[idx] = c;
		if ( idx > 4 )
			pParams->szPdfVersion[idxVersion++] = c;
		
		idx++;		
	}
	
	if ( pParams->szPdfHeader[0] != '%' )
	{
		//wprintf(L"ERRORE ReadHeader: formaro file non valido\n");
		fwprintf(pParams->fpErrors, L"ERRORE ReadHeader: formaro file non valido\n");
		return 0;
	}
	
	if ( pParams->szPdfHeader[1] != 'P' )
	{
		//wprintf(L"ERRORE ReadHeader: formaro file non valido\n");
		fwprintf(pParams->fpErrors, L"ERRORE ReadHeader: formaro file non valido\n");
		return 0;
	}
	
	if ( pParams->szPdfHeader[2] != 'D' )
	{
		//wprintf(L"ERRORE ReadHeader: formaro file non valido\n");
		fwprintf(pParams->fpErrors, L"ERRORE ReadHeader: formaro file non valido\n");
		return 0;
	}

	if ( pParams->szPdfHeader[3] != 'F' )
	{
		//wprintf(L"ERRORE ReadHeader: formaro file non valido\n");
		fwprintf(pParams->fpErrors, L"ERRORE ReadHeader: formaro file non valido\n");
		return 0;
	}
	
	if ( pParams->szPdfHeader[4] != '-' )
	{
		//wprintf(L"ERRORE ReadHeader: formaro file non valido\n");
		fwprintf(pParams->fpErrors, L"ERRORE ReadHeader: formaro file non valido\n");
		return 0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadHeader_FN)	
	wprintf(L"\nPDF HEADER  = %s\n", pParams->szPdfHeader);
	wprintf(L"PDF VERSION = %s\n", pParams->szPdfVersion);
	#endif	
	
	return 1;
}

int ReadTrailer(Params *pParams)
{
	//int dimFile = 0;
	int byteOffsetXRef = 0;
	int bytereads = 0;
	unsigned char *szInput = NULL;
	int dimInput = 0;
	unsigned char c;
	
	int index;
	int index1;
	int index2;	
	int x;
	int y;
	unsigned char szTemp[BLOCK_SIZE];
	unsigned char szOffsetXRef[128];


	if ( fseek(pParams->fp, 0, SEEK_END) )
	{
		return 0;
	}

	pParams->dimFile = ftell(pParams->fp);
	if ( pParams->dimFile <= 0 )
	{
		return 0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
	wprintf(L"ReadTrailer: dimFile = %d\n", pParams->dimFile);
	#endif
	
	if ( pParams->dimFile >= BLOCK_SIZE )
	{
		if ( fseek(pParams->fp, -BLOCK_SIZE, SEEK_CUR) )
		{
			//wprintf(L"Errore ReadTrailer: fseek 1\n");
			fwprintf(pParams->fpErrors, L"Errore ReadTrailer: fseek 1\n");
			return 0;
		}
	}
	else
	{
		if ( fseek(pParams->fp, 0, SEEK_SET) )
		{
			//wprintf(L"Errore ReadTrailer: fseek 2\n");
			fwprintf(pParams->fpErrors, L"Errore ReadTrailer: fseek 2\n");
			return 0;
		}		
	}
	
	bytereads = fread(szTemp, 1, BLOCK_SIZE, pParams->fp);
	if ( bytereads <= 5 )
	{
		wprintf(L"Errore ReadTrailer: fread 1\n");
		return 0;
	}
	
	index = bytereads - 1;
		
	while ( index && (
					  szTemp[index] == '\n' || 
					  szTemp[index] == '\r' ||
					  szTemp[index] == '\t' ||
					  szTemp[index] == ' ' ||
					  szTemp[index] == '\f' ||
					  szTemp[index] == '\b'
					  )
		  )
	{
		index--;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
	wprintf(L"ReadTrailer: index = %d\n", index);
	#endif
	if ( index <= 5 )
		return 0;	
	
	c = szTemp[index--];
	if ( c != 'F')
		return 0;

	c = szTemp[index--];
	if ( c != 'O')
		return 0;

	c = szTemp[index--];
	if ( c != 'E')
		return 0;
		
	c = szTemp[index--];
	if ( c != '%')
		return 0;

	c = szTemp[index--];
	if ( c != '%')
		return 0;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
	wprintf(L"ReadTrailer: %%PDF OK!\n");
	#endif
	
	index1 = index2 = 0;
	
	while ( index )
	{
		if ( szTemp[index] >= '0' && szTemp[index] <= '9' )
		{
			index2 = index;
			index--;
			break;
		}
		index--;	
	}
	
	if ( index < 0 )
		return 0;	
	
	while ( (szTemp[index] >= '0' && szTemp[index] <= '9') && index >= 0 )
	{
		index--;
	}
	
	if ( index < 0 )
		return 0;
		
	index1 = index;
		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
	wprintf(L"ReadTrailer: index1 = %d\nReadTrailer: index2 = %d\n", index1, index2);
	#endif
	
	y = 0;
	for(x = index1; x <= index2; x++)
	{
		szOffsetXRef[y++] = szTemp[x];
	}
	if ( y == 0 )
		return 0;
	szOffsetXRef[y] = '\0';
	byteOffsetXRef = atoi((char*)szOffsetXRef);	
		
	dimInput = pParams->dimFile - byteOffsetXRef;
	
	if ( dimInput <= 0 )
	{
		//wprintf(L"Errore ReadTrailer: manca l'offset per xref.\n");
		fwprintf(pParams->fpErrors, L"Errore ReadTrailer: manca l'offset per xref.\n");
		return 0;
	}
	
	szInput = (unsigned char*)malloc(sizeof(unsigned char)*(dimInput) + 1);
	if ( !szInput )
	{
		//wprintf(L"Errore ReadTrailer: memoria insufficiente.\n");
		fwprintf(pParams->fpErrors, L"Errore ReadTrailer: memoria insufficiente.\n");
		return 0;
	}

	fseek(pParams->fp, byteOffsetXRef, SEEK_SET);
	
	bytereads = fread(szInput, 1, dimInput, pParams->fp);
	*(szInput + bytereads) = '\0';
	
	//wprintf(L"file: '%s'\n\n%s\n\n", pParams->szFileName, szInput); 		
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
	wprintf(L"szInput: <%s>\n\n", szInput);
	#endif
		
	pParams->myPdfTrailer.Size = 0;		
	pParams->myPdfTrailer.Root.Number = 0;
	pParams->myPdfTrailer.Root.Generation = 0;	
		
	if ( !ReadLastTrailer(pParams, szInput) )
	{
		free(szInput);
		return 0;
	}
		
	if ( !ReadObjsTable(pParams, szInput, 1) )
	{
		free(szInput);
		return 0;
	}
	
	#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_ReadTrailer_FN)
	wprintf(L"ReadTrailer: ReadObjsTable OK!\n");
	#endif
		
	//wprintf(L"file: '%s'\n\n%s\n\n", pParams->szFileName, szInput); 	
	//wprintf(L"file: '%s'\n\n", pParams->szFileName); 
		
	while ( pParams->myPdfTrailer.Prev > 0 )
	{
		free(szInput);
		if ( !GetLenNextInput(pParams, pParams->myPdfTrailer.Prev, &dimInput) )
		{
			//wprintf(L"Errore GetLenNextInput\n");
			fwprintf(pParams->fpErrors, L"Errore GetLenNextInput\n");
			return 0;
		}
		
		szInput = (unsigned char*)malloc(sizeof(unsigned char)*(dimInput) + 1);
		if ( !szInput )
		{
			//wprintf(L"GetLenNextInput errore ReadTrailer: memoria insufficiente.\n");
			fwprintf(pParams->fpErrors, L"GetLenNextInput errore ReadTrailer: memoria insufficiente.\n");
			return 0;
		}
		
		if ( fseek(pParams->fp, pParams->myPdfTrailer.Prev, SEEK_SET) )
		{
			//wprintf(L"Errore fseek 3\n");
			fwprintf(pParams->fpErrors, L"Errore fseek 3\n");
			free(szInput);
			return 0;
		}
	
		bytereads = fread(szInput, 1, dimInput, pParams->fp);
		*(szInput + bytereads) = '\0';
		
		//wprintf(L"\nszInput = <%s>\n", szInput);
		//wprintf(L"dimInput = %d\n", dimInput);
		
		if ( !ReadLastTrailer(pParams, szInput) )
		{
			free(szInput);
			return 0;
		}
		
		if ( !ReadObjsTable(pParams, szInput, 0) )
		{
			free(szInput);
			return 0;
		}
	}
	
	free(szInput);
	fseek(pParams->fp, 0, SEEK_SET);
	
	return 1;	
}

unsigned char GetHexChar(unsigned char c1, unsigned char c2)
{
	unsigned char c = '\0';
	
	wprintf(L"c1 = '%c <> c2 = '%c'\n", c1, c2);
	
	switch ( c1 )
	{
		case '0':
			if ( c2 == '0' )
				return '\x00';
			else if ( c2 == '1' )
				return '\x01';
			else if ( c2 == '2' )
				return '\x02';
			else if ( c2 == '3' )
				return '\x03';
			else if ( c2 == '4' )
				return '\x04';
			else if ( c2 == '5' )
				return '\x05';
			else if ( c2 == '6' )
				return '\x06';
			else if ( c2 == '7' )
				return '\x07';
			else if ( c2 == '8' )
				return '\x08';
			else if ( c2 == '9' )
				return '\x09';
			else if ( c2 == 'a' || c == 'A' )
				return '\x0A';
			else if ( c2 == 'b' || c == 'B' )
				return '\x0B';
			else if ( c2 == 'c' || c == 'C' )
				return '\x0C';
			else if ( c2 == 'd' || c == 'D' )
				return '\x0D';
			else if ( c2 == 'e' || c == 'E' )
				return '\x0E';
			else if ( c2 == 'f' || c == 'F' )
				return '\x0F';
			else
				return '\0';
			break;
		case '1':
			if ( c2 == '0' )
				return '\x10';
			else if ( c2 == '1' )
				return '\x11';
			else if ( c2 == '2' )
				return '\x12';
			else if ( c2 == '3' )
				return '\x13';
			else if ( c2 == '4' )
				return '\x14';
			else if ( c2 == '5' )
				return '\x15';
			else if ( c2 == '6' )
				return '\x16';
			else if ( c2 == '7' )
				return '\x17';
			else if ( c2 == '8' )
				return '\x18';
			else if ( c2 == '9' )
				return '\x19';
			else if ( c2 == 'a' || c == 'A' )
				return '\x1A';
			else if ( c2 == 'b' || c == 'B' )
				return '\x1B';
			else if ( c2 == 'c' || c == 'C' )
				return '\x1C';
			else if ( c2 == 'd' || c == 'D' )
				return '\x1D';
			else if ( c2 == 'e' || c == 'E' )
				return '\x1E';
			else if ( c2 == 'f' || c == 'F' )
				return '\x1F';
			else
				return '\0';		
			break;
		case '2':
			if ( c2 == '0' )
				return '\x20';
			else if ( c2 == '1' )
				return '\x21';
			else if ( c2 == '2' )
				return '\x22';
			else if ( c2 == '3' )
				return '\x23';
			else if ( c2 == '4' )
				return '\x24';
			else if ( c2 == '5' )
				return '\x25';
			else if ( c2 == '6' )
				return '\x26';
			else if ( c2 == '7' )
				return '\x27';
			else if ( c2 == '8' )
				return '\x28';
			else if ( c2 == '9' )
				return '\x29';
			else if ( c2 == 'a' || c == 'A' )
				return '\x2A';
			else if ( c2 == 'b' || c == 'B' )
				return '\x2B';
			else if ( c2 == 'c' || c == 'C' )
				return '\x2C';
			else if ( c2 == 'd' || c == 'D' )
				return '\x2D';
			else if ( c2 == 'e' || c == 'E' )
				return '\x2E';
			else if ( c2 == 'f' || c == 'F' )
				return '\x2F';
			else
				return '\0';		
			break;
		case '3':
			if ( c2 == '0' )
				return '\x30';
			else if ( c2 == '1' )
				return '\x31';
			else if ( c2 == '2' )
				return '\x32';
			else if ( c2 == '3' )
				return '\x33';
			else if ( c2 == '4' )
				return '\x34';
			else if ( c2 == '5' )
				return '\x35';
			else if ( c2 == '6' )
				return '\x36';
			else if ( c2 == '7' )
				return '\x37';
			else if ( c2 == '8' )
				return '\x38';
			else if ( c2 == '9' )
				return '\x39';
			else if ( c2 == 'a' || c == 'A' )
				return '\x3A';
			else if ( c2 == 'b' || c == 'B' )
				return '\x3B';
			else if ( c2 == 'c' || c == 'C' )
				return '\x3C';
			else if ( c2 == 'd' || c == 'D' )
				return '\x3D';
			else if ( c2 == 'e' || c == 'E' )
				return '\x3E';
			else if ( c2 == 'f' || c == 'F' )
				return '\x3F';
			else
				return '\0';				
			break;
		case '4':
			if ( c2 == '0' )
				return '\x40';
			else if ( c2 == '1' )
				return '\x41';
			else if ( c2 == '2' )
				return '\x42';
			else if ( c2 == '3' )
				return '\x43';
			else if ( c2 == '4' )
				return '\x44';
			else if ( c2 == '5' )
				return '\x45';
			else if ( c2 == '6' )
				return '\x46';
			else if ( c2 == '7' )
				return '\x47';
			else if ( c2 == '8' )
				return '\x48';
			else if ( c2 == '9' )
				return '\x49';
			else if ( c2 == 'a' || c == 'A' )
				return '\x4A';
			else if ( c2 == 'b' || c == 'B' )
				return '\x4B';
			else if ( c2 == 'c' || c == 'C' )
				return '\x4C';
			else if ( c2 == 'd' || c == 'D' )
				return '\x4D';
			else if ( c2 == 'e' || c == 'E' )
				return '\x4E';
			else if ( c2 == 'f' || c == 'F' )
				return '\x4F';
			else
				return '\0';		
			break;
		case '5':
			if ( c2 == '0' )
				return '\x50';
			else if ( c2 == '1' )
				return '\x51';
			else if ( c2 == '2' )
				return '\x52';
			else if ( c2 == '3' )
				return '\x53';
			else if ( c2 == '4' )
				return '\x54';
			else if ( c2 == '5' )
				return '\x55';
			else if ( c2 == '6' )
				return '\x56';
			else if ( c2 == '7' )
				return '\x57';
			else if ( c2 == '8' )
				return '\x58';
			else if ( c2 == '9' )
				return '\x59';
			else if ( c2 == 'a' || c == 'A' )
				return '\x5A';
			else if ( c2 == 'b' || c == 'B' )
				return '\x5B';
			else if ( c2 == 'c' || c == 'C' )
				return '\x5C';
			else if ( c2 == 'd' || c == 'D' )
				return '\x5D';
			else if ( c2 == 'e' || c == 'E' )
				return '\x5E';
			else if ( c2 == 'f' || c == 'F' )
				return '\x5F';
			else
				return '\0';
			break;
		case '6':
			if ( c2 == '0' )
				return '\x60';
			else if ( c2 == '1' )
				return '\x61';
			else if ( c2 == '2' )
				return '\x62';
			else if ( c2 == '3' )
				return '\x63';
			else if ( c2 == '4' )
				return '\x64';
			else if ( c2 == '5' )
				return '\x65';
			else if ( c2 == '6' )
				return '\x66';
			else if ( c2 == '7' )
				return '\x67';
			else if ( c2 == '8' )
				return '\x68';
			else if ( c2 == '9' )
				return '\x69';
			else if ( c2 == 'a' || c == 'A' )
				return '\x6A';
			else if ( c2 == 'b' || c == 'B' )
				return '\x6B';
			else if ( c2 == 'c' || c == 'C' )
				return '\x6C';
			else if ( c2 == 'd' || c == 'D' )
				return '\x6D';
			else if ( c2 == 'e' || c == 'E' )
				return '\x6E';
			else if ( c2 == 'f' || c == 'F' )
				return '\x6F';
			else
				return '\0';		
			break;
		case '7':
			if ( c2 == '0' )
				return '\x70';
			else if ( c2 == '1' )
				return '\x71';
			else if ( c2 == '2' )
				return '\x72';
			else if ( c2 == '3' )
				return '\x73';
			else if ( c2 == '4' )
				return '\x74';
			else if ( c2 == '5' )
				return '\x75';
			else if ( c2 == '6' )
				return '\x76';
			else if ( c2 == '7' )
				return '\x77';
			else if ( c2 == '8' )
				return '\x78';
			else if ( c2 == '9' )
				return '\x79';
			else if ( c2 == 'a' || c == 'A' )
				return '\x7A';
			else if ( c2 == 'b' || c == 'B' )
				return '\x7B';
			else if ( c2 == 'c' || c == 'C' )
				return '\x7C';
			else if ( c2 == 'd' || c == 'D' )
				return '\x7D';
			else if ( c2 == 'e' || c == 'E' )
				return '\x7E';
			else if ( c2 == 'f' || c == 'F' )
				return '\x7F';
			else
				return '\0';		
			break;
		case '8':
			if ( c2 == '0' )
				return '\x80';
			else if ( c2 == '1' )
				return '\x81';
			else if ( c2 == '2' )
				return '\x82';
			else if ( c2 == '3' )
				return '\x83';
			else if ( c2 == '4' )
				return '\x84';
			else if ( c2 == '5' )
				return '\x85';
			else if ( c2 == '6' )
				return '\x86';
			else if ( c2 == '7' )
				return '\x87';
			else if ( c2 == '8' )
				return '\x88';
			else if ( c2 == '9' )
				return '\x89';
			else if ( c2 == 'a' || c == 'A' )
				return '\x8A';
			else if ( c2 == 'b' || c == 'B' )
				return '\x8B';
			else if ( c2 == 'c' || c == 'C' )
				return '\x8C';
			else if ( c2 == 'd' || c == 'D' )
				return '\x8D';
			else if ( c2 == 'e' || c == 'E' )
				return '\x8E';
			else if ( c2 == 'f' || c == 'F' )
				return '\x8F';
			else
				return '\0';		
			break;
		case '9':
			if ( c2 == '0' )
				return '\x90';
			else if ( c2 == '1' )
				return '\x91';
			else if ( c2 == '2' )
				return '\x92';
			else if ( c2 == '3' )
				return '\x93';
			else if ( c2 == '4' )
				return '\x94';
			else if ( c2 == '5' )
				return '\x95';
			else if ( c2 == '6' )
				return '\x96';
			else if ( c2 == '7' )
				return '\x97';
			else if ( c2 == '8' )
				return '\x98';
			else if ( c2 == '9' )
				return '\x99';
			else if ( c2 == 'a' || c == 'A' )
				return '\x9A';
			else if ( c2 == 'b' || c == 'B' )
				return '\x9B';
			else if ( c2 == 'c' || c == 'C' )
				return '\x9C';
			else if ( c2 == 'd' || c == 'D' )
				return '\x9D';
			else if ( c2 == 'e' || c == 'E' )
				return '\x9E';
			else if ( c2 == 'f' || c == 'F' )
				return '\x9F';
			else
				return '\0';			
			break;
		case 'a':
		case 'A':
			if ( c2 == '0' )
				return '\xA0';
			else if ( c2 == '1' )
				return '\xA1';
			else if ( c2 == '2' )
				return '\xA2';
			else if ( c2 == '3' )
				return '\xA3';
			else if ( c2 == '4' )
				return '\xA4';
			else if ( c2 == '5' )
				return '\xA5';
			else if ( c2 == '6' )
				return '\xA6';
			else if ( c2 == '7' )
				return '\xA7';
			else if ( c2 == '8' )
				return '\xA8';
			else if ( c2 == '9' )
				return '\xA9';
			else if ( c2 == 'a' || c == 'A' )
				return '\xAA';
			else if ( c2 == 'b' || c == 'B' )
				return '\xAB';
			else if ( c2 == 'c' || c == 'C' )
				return '\xAC';
			else if ( c2 == 'd' || c == 'D' )
				return '\xAD';
			else if ( c2 == 'e' || c == 'E' )
				return '\xAE';
			else if ( c2 == 'f' || c == 'F' )
				return '\xAF';
			else
				return '\0';		
			break;
		case 'b':
		case 'B':
			if ( c2 == '0' )
				return '\xB0';
			else if ( c2 == '1' )
				return '\xB1';
			else if ( c2 == '2' )
				return '\xB2';
			else if ( c2 == '3' )
				return '\xB3';
			else if ( c2 == '4' )
				return '\xB4';
			else if ( c2 == '5' )
				return '\xB5';
			else if ( c2 == '6' )
				return '\xB6';
			else if ( c2 == '7' )
				return '\xB7';
			else if ( c2 == '8' )
				return '\xB8';
			else if ( c2 == '9' )
				return '\xB9';
			else if ( c2 == 'a' || c == 'A' )
				return '\xBA';
			else if ( c2 == 'b' || c == 'B' )
				return '\xBB';
			else if ( c2 == 'c' || c == 'C' )
				return '\xBC';
			else if ( c2 == 'd' || c == 'D' )
				return '\xBD';
			else if ( c2 == 'e' || c == 'E' )
				return '\xBE';
			else if ( c2 == 'f' || c == 'F' )
				return '\xBF';
			else
				return '\0';		
			break;
		case 'c':
		case 'C':
			if ( c2 == '0' )
				return '\xC0';
			else if ( c2 == '1' )
				return '\xC1';
			else if ( c2 == '2' )
				return '\xC2';
			else if ( c2 == '3' )
				return '\xC3';
			else if ( c2 == '4' )
				return '\xC4';
			else if ( c2 == '5' )
				return '\xC5';
			else if ( c2 == '6' )
				return '\xC6';
			else if ( c2 == '7' )
				return '\xC7';
			else if ( c2 == '8' )
				return '\xC8';
			else if ( c2 == '9' )
				return '\xC9';
			else if ( c2 == 'a' || c == 'A' )
				return '\xCA';
			else if ( c2 == 'b' || c == 'B' )
				return '\xCB';
			else if ( c2 == 'c' || c == 'C' )
				return '\xCC';
			else if ( c2 == 'd' || c == 'D' )
				return '\xCD';
			else if ( c2 == 'e' || c == 'E' )
				return '\xCE';
			else if ( c2 == 'f' || c == 'F' )
				return '\xCF';
			else
				return '\0';		
			break;
		case 'd':
		case 'D':
			if ( c2 == '0' )
				return '\xD0';
			else if ( c2 == '1' )
				return '\xD1';
			else if ( c2 == '2' )
				return '\xD2';
			else if ( c2 == '3' )
				return '\xD3';
			else if ( c2 == '4' )
				return '\xD4';
			else if ( c2 == '5' )
				return '\xD5';
			else if ( c2 == '6' )
				return '\xD6';
			else if ( c2 == '7' )
				return '\xD7';
			else if ( c2 == '8' )
				return '\xD8';
			else if ( c2 == '9' )
				return '\xD9';
			else if ( c2 == 'a' || c == 'A' )
				return '\xDA';
			else if ( c2 == 'b' || c == 'B' )
				return '\xDB';
			else if ( c2 == 'c' || c == 'C' )
				return '\xDC';
			else if ( c2 == 'd' || c == 'D' )
				return '\xDD';
			else if ( c2 == 'e' || c == 'E' )
				return '\xDE';
			else if ( c2 == 'f' || c == 'F' )
				return '\xDF';
			else
				return '\0';		
			break;
		case 'e':
		case 'E':
			if ( c2 == '0' )
				return '\xE0';
			else if ( c2 == '1' )
				return '\xE1';
			else if ( c2 == '2' )
				return '\xE2';
			else if ( c2 == '3' )
				return '\xE3';
			else if ( c2 == '4' )
				return '\xE4';
			else if ( c2 == '5' )
				return '\xE5';
			else if ( c2 == '6' )
				return '\xE6';
			else if ( c2 == '7' )
				return '\xE7';
			else if ( c2 == '8' )
				return '\xE8';
			else if ( c2 == '9' )
				return '\xE9';
			else if ( c2 == 'a' || c == 'A' )
				return '\xEA';
			else if ( c2 == 'b' || c == 'B' )
				return '\xEB';
			else if ( c2 == 'c' || c == 'C' )
				return '\xEC';
			else if ( c2 == 'd' || c == 'D' )
				return '\xED';
			else if ( c2 == 'e' || c == 'E' )
				return '\xEE';
			else if ( c2 == 'f' || c == 'F' )
				return '\xEF';
			else
				return '\0';		
			break;
		case 'f':
		case 'F':
			if ( c2 == '0' )
				return '\xF0';
			else if ( c2 == '1' )
				return '\xF1';
			else if ( c2 == '2' )
				return '\xF2';
			else if ( c2 == '3' )
				return '\xF3';
			else if ( c2 == '4' )
				return '\xF4';
			else if ( c2 == '5' )
				return '\xF5';
			else if ( c2 == '6' )
				return '\xF6';
			else if ( c2 == '7' )
				return '\xF7';
			else if ( c2 == '8' )
				return '\xF8';
			else if ( c2 == '9' )
				return '\xF9';
			else if ( c2 == 'a' || c == 'A' )
				return '\xFA';
			else if ( c2 == 'b' || c == 'B' )
				return '\xFB';
			else if ( c2 == 'c' || c == 'C' )
				return '\xFC';
			else if ( c2 == 'd' || c == 'D' )
				return '\xFD';
			else if ( c2 == 'e' || c == 'E' )
				return '\xFE';
			else if ( c2 == 'f' || c == 'F' )
				return '\xFF';
			else
				return '\0';		
			break;			
			
		default:
			return '\0';
	}
	
	return c;
}

int IsDelimiterChar(unsigned char c)
{
	switch ( c )
	{
		case ' ':
		case '\r':
		case '\n':
		case '\f':
		case '\t':
		case '\0':
			return DELIM_SPACECHAR;
			break;
		case '%':
		case '(':
		case ')':
		case '<':
		case '>':
		case '[':
		case ']':
		case '{':
		case '}':
		case '/':
			return DELIM_SPECIALSYMBOL;
			break;			
		default:
			return DELIM_FALSE;
			break;
	}
	
	return DELIM_FALSE;
}

unsigned char ReadNextChar(Params *pParams)
{
	/*
	Nel caso la funzione GetNextToken abbia impostato
		pParams->blockCurPos--;
	pParams->blockCurPos potrebbe trovarsi con un valore inferiore a 0.
	In questo caso dobbiamo leggere il blocco precedente e impostare
	pParams->blockCurPos a pParams->blockLen - 1.
	*/
	if ( pParams->blockCurPos < 0 )
	{
		if ( pParams->dimFile >= BLOCK_SIZE )
		{
			if ( fseek(pParams->fp, -BLOCK_SIZE, SEEK_CUR) )
			{
				//wprintf(L"Errore ReadNextChar fseek 1\n");
				fwprintf(pParams->fpErrors, L"Errore ReadNextChar fseek 1\n");
				return 0;
			}
		}
		else
		{
			if ( fseek(pParams->fp, 0, SEEK_SET) )
			{
				//wprintf(L"Errore ReadNextChar fseek 2\n");
				fwprintf(pParams->fpErrors, L"Errore ReadNextChar fseek 2\n");
				return 0;
			}		
		}
				
		pParams->blockCurPos = pParams->blockLen - 1;		
	}
	
	if ( pParams->blockCurPos >= pParams->blockLen )
	{
		if ( pParams->bStreamState )
		{
			pParams->myToken.Type = T_EOF;
			//return '\0';
			return ' ';
		}
		
		pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);	
		if ( pParams->blockLen == 0 )
		{
			pParams->myToken.Type = T_EOF;
			//return '\0';
			return ' ';
		}
		pParams->blockCurPos = 0;
	}	
		
	if ( pParams->bUpdateNumBytesReadFromCurrentStream )
		pParams->nNumBytesReadFromCurrentStream++;
		
	//return pParams->myBlock[pParams->blockCurPos++];
	
	/*
	if ( '\0' !=  pParams->myBlock[pParams->blockCurPos] )
	{	
		return pParams->myBlock[pParams->blockCurPos++];
	}
	else
	{
		pParams->blockCurPos++;
		return ' ';
	}
	*/
	
	if ( pParams->myBlock[pParams->blockCurPos] > 31 &&
	     pParams->myBlock[pParams->blockCurPos] != 127 &&
	     pParams->myBlock[pParams->blockCurPos] != 129 &&
	     pParams->myBlock[pParams->blockCurPos] != 141 &&
	     pParams->myBlock[pParams->blockCurPos] != 143 &&
	     pParams->myBlock[pParams->blockCurPos] != 144 &&
	     pParams->myBlock[pParams->blockCurPos] != 157 &&
	     pParams->myBlock[pParams->blockCurPos] != 173
	   )
	{	
		return pParams->myBlock[pParams->blockCurPos++];
	}	
	else
	{		
		if ( '\n' == pParams->myBlock[pParams->blockCurPos] ||
		     '\r' == pParams->myBlock[pParams->blockCurPos] ||
		     '\t' == pParams->myBlock[pParams->blockCurPos] ||
		     '\f' == pParams->myBlock[pParams->blockCurPos] ||
		     ' ' == pParams->myBlock[pParams->blockCurPos]
		   )
		{
			return pParams->myBlock[pParams->blockCurPos++];
		}
		else
		{
			pParams->blockCurPos++;
			return ' ';
		}
	}
}

void IgnoreStreamChars(Params *pParams)
{
	/*
	Nel caso la funzione GetNextToken abbia impostato
		pParams->blockCurPos--;
	pParams->blockCurPos potrebbe trovarsi con un valore inferiore a 0.
	In questo caso dobbiamo leggere il blocco precedente e impostare
	pParams->blockCurPos a pParams->blockLen - 1.
	*/
	if ( pParams->blockCurPos < 0 )
	{
		if ( pParams->dimFile >= BLOCK_SIZE )
		{
			if ( fseek(pParams->fp, -BLOCK_SIZE, SEEK_CUR) )
			{
				//wprintf(L"Errore IgnoreStreamChars fseek 1\n");
				fwprintf(pParams->fpErrors, L"Errore IgnoreStreamChars fseek 1\n");
				return;
			}
		}
		else
		{
			if ( fseek(pParams->fp, 0, SEEK_SET) )
			{
				//wprintf(L"Errore IgnoreStreamChars fseek 2\n");
				fwprintf(pParams->fpErrors, L"Errore IgnoreStreamChars fseek 2\n");
				return;
			}		
		}
				
		pParams->blockCurPos = pParams->blockLen - 1;		
	}
	
	if ( pParams->blockCurPos >= pParams->blockLen )
	{
		pParams->blockLen = fread(pParams->myBlock, 1, BLOCK_SIZE, pParams->fp);	
		if ( pParams->blockLen == 0 )
		{
			pParams->myToken.Type = T_EOF;
			return;
		}
		pParams->blockCurPos = 0;
	}	
}

void GetNextToken(Params *pParams)
{
	States state = S0;
	unsigned char c;
	unsigned char c1, c2;
	unsigned char cOctal;
	unsigned char cHexadecimal;
	int k = 0;
	int bOctalOverflow;
	int nDelimChar;
	unsigned long int w;
	
	int x;
	int y;
	int z;
		
	if ( (T_NAME == pParams->myToken.Type || T_STRING == pParams->myToken.Type || T_STRING_LITERAL == pParams->myToken.Type || T_STRING_HEXADECIMAL == pParams->myToken.Type) && (NULL != pParams->myToken.Value.vString) )
	{
		free(pParams->myToken.Value.vString);
		pParams->myToken.Value.vString = NULL;
	}
	
	if ( pParams->blockCurPos > pParams->blockLen )
	{
		pParams->myToken.Type = T_EOF;
		return;
	}
			
	c = ReadNextChar(pParams);
	if ( pParams->myToken.Type == T_EOF )
		return;
		
	while ( c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == '\f' ||  c == '\0' )
	{
		c = ReadNextChar(pParams);
				
		if ( pParams->myToken.Type == T_EOF )
			return;					
	}
		
	while ( 1 )
	{
		switch ( state )
		{
			inizioswitch:			
						
			case S0:
				k = 0;
				pParams->nStackStringOpenParen = 0;
				
				if ( c == '%' )
				{
					c = ReadNextChar(pParams);
					while ( c != '\n' && c != '\r' )
					{
						c = ReadNextChar(pParams);
						if ( pParams->myToken.Type == T_EOF )
							return;
					}
					if ( c == '\r' )
					{
						c = ReadNextChar(pParams);
						if ( pParams->myToken.Type == T_EOF )
							return;	
						if ( c != '\n' )
						{
							goto inizioswitch;
						}					
					}
				}
				else if ( c == '-' || c == '+' || ( c >= '0' && c <= '9' ) )
				{
					pParams->lexeme[k++] = c;
					state = S1;
				}
				else if ( c == '.' )
				{
					pParams->lexeme[k++] = c;
					state = S2;
				}
				else if ( c == '/' )
				{
					state = S3;
				}
				else if ( c == '(' )
				{
					pParams->nStackStringOpenParen++; // PUSH
					state = S6;
				}
				else if ( c == '<' )
				{										
					state = S11;
				}				
				else if ( c == '[' )
				{
					pParams->myToken.Type = T_QOPAREN;
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
					wprintf(L"GetNextToken: T_QOPAREN -> [\n");
					#endif
					return;
				}
				else if ( c == ']' )
				{
					pParams->myToken.Type = T_QCPAREN;
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
					wprintf(L"GetNextToken: T_QCPAREN -> ]\n");
					#endif
					return;
				}
				else if ( c == '>' )
				{					
					c = ReadNextChar(pParams);
					if ( c == '>' )
					{
						pParams->myToken.Type = T_DICT_END;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextToken: T_DICT_END -> >>\n");
						#endif
						return;
					}
					else
					{
						if ( !(pParams->bStreamState) )
						{
							pParams->myToken.Type = T_ERROR;
							wprintf(L"Errore GetNextToken: trovato '>' singolo\n");
							return;
						}
					}
				}
				else // T_STRING oppure T_KEYWORD
				{
					nDelimChar = IsDelimiterChar(c);
					while ( !nDelimChar )
					{
						if ( k < MAX_STRING_LENTGTH_IN_CONTENT_STREAM )
						{
							pParams->lexeme[k++] = c;
						}
						else
						{
							pParams->lexeme[k] = '\0';
						}
						c = ReadNextChar(pParams);
						nDelimChar = IsDelimiterChar(c);
					}
					pParams->lexeme[k] = '\0';
					if ( DELIM_SPECIALSYMBOL == nDelimChar )
					{
						pParams->blockCurPos--;
						pParams->nNumBytesReadFromCurrentStream--;
					}
					
					if ( strncmp(pParams->lexeme, "stream", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						if ( c == '\r' )
						{
							c = ReadNextChar(pParams);
							if ( c != '\n' )
							{
								pParams->myToken.Type = T_ERROR;
								//wprintf(L"Errore GetNextToken: la keyword 'stream' dev'essere seguita da '\\r\\n' oppure da '\\n'. Trovato invece '\\r'\n");
								fwprintf(pParams->fpErrors, L"Errore GetNextToken: la keyword 'stream' dev'essere seguita da '\\r\\n' oppure da '\\n'. Trovato invece '\\r'\n");
								return;
							}
						}
						else if ( c != '\n' )
						{
							pParams->myToken.Type = T_ERROR;
							//wprintf(L"Errore GetNextToken: la keyword 'stream' dev'essere seguita da '\\r\\n' oppure da '\\n'. Trovato invece '%c'\n", c);
							fwprintf(pParams->fpErrors, L"Errore GetNextToken: la keyword 'stream' dev'essere seguita da '\\r\\n' oppure da '\\n'. Trovato invece '%c'\n", c);
							return;							
						}
												
						pParams->bUpdateNumBytesReadFromCurrentStream = 0;
						for ( w = 0; w < pParams->CurrentContent.LengthFromPdf; w++ )
						{
							 ReadNextChar(pParams);		
						}
						
						// *************************************************************************************************************************************
						/*
						c = ReadNextChar(pParams);
						if ( '\n' != c && '\r' != c && 'e' != c )
						{
							pParams->myToken.Type = T_ERROR;
							wprintf(L"Errore GetNextToken: Stream Length specificato nel file %lu errato\n", pParams->CurrentContent.LengthFromPdf);
							return;								
						}
						else
						{
							if ( '\n' == c )
							{
								c = ReadNextChar(pParams);
								if ( 'e' != c )
								{
									pParams->myToken.Type = T_ERROR;
									wprintf(L"Errore GetNextToken: Stream Length specificato nel file %lu errato\n", pParams->CurrentContent.LengthFromPdf);
									return;									
								}
								else
								{
									pParams->blockCurPos--;
									//pParams->nNumBytesReadFromCurrentStream--;
								}
							}
							else if ( '\r' == c )
							{
								c = ReadNextChar(pParams);
								if ( '\n' != c && 'e' != c )
								{
									pParams->myToken.Type = T_ERROR;
									wprintf(L"Errore GetNextToken: Stream Length specificato nel file %lu errato\n", pParams->CurrentContent.LengthFromPdf);
									return;									
								}
								else if ( '\n' == c )
								{
									c = ReadNextChar(pParams);
									if ( 'e' == c )
									{
										//pParams->blockCurPos -= 2;
										pParams->blockCurPos--;
										//pParams->nNumBytesReadFromCurrentStream--;
									}
									else
									{
										pParams->myToken.Type = T_ERROR;
										wprintf(L"Errore GetNextToken: Stream Length specificato nel file %lu errato\n", pParams->CurrentContent.LengthFromPdf);
										return;										
									}
								}
								else if ( 'e' == c )
								{
									pParams->blockCurPos--;
									//pParams->nNumBytesReadFromCurrentStream--;									
								}								
							}
							else // c = 'e'
							{
								pParams->blockCurPos--;
								//pParams->nNumBytesReadFromCurrentStream--;
							}
							
						}
						*/
						// *************************************************************************************************************************************
						
						pParams->bUpdateNumBytesReadFromCurrentStream = 1;
						
						pParams->myToken.Type = T_KW_STREAM;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextToken: T_KW_STREAM -> 'stream'\n");
						#endif
						pParams->bStreamState = 1;						
						return;
					}
					else if ( strncmp(pParams->lexeme, "endstream", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_KW_ENDSTREAM;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextToken: T_KW_ENDSTREAM -> 'endstream'\n");
						#endif
						pParams->bStreamState = 0;						
						return;
					}
					else if ( strncmp(pParams->lexeme, "R", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_KW_R;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextToken: T_KW_R -> 'R'\n");
						#endif
						return;
					}
					else if ( strncmp(pParams->lexeme, "true", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_KW_TRUE;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextToken: T_KW_TRUE -> 'true'\n");
						#endif
						return;
					}
					else if ( strncmp(pParams->lexeme, "false", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_KW_FALSE;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextToken: T_KW_FALSE -> 'false'\n");
						#endif
						return;
					}
					else if ( strncmp(pParams->lexeme, "null", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_KW_NULL;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_KW_NULL -> 'null'\n");
						#endif
						return;
					}
					else if ( strncmp(pParams->lexeme, "obj", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_KW_OBJ;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextToken: T_KW_OBJ -> 'obj'\n");
						#endif
						return;
					}
					else if ( strncmp(pParams->lexeme, "endobj", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_KW_ENDOBJ;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_KW_ENDOBJ -> 'endobj'\n");
						#endif
						return;
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "Do", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_Do_COMMAND;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_Do_COMMAND -> 'Do'\n");
						#endif
						return;
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "BT", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_KW_BT;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_KW_BT -> 'BT -> Begin Text'\n");
						#endif
						return;
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "ET", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_KW_ET;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_KW_ET -> 'ET -> End Text'\n");
						#endif
						return;
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "Tc", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_OP_Tc;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_OP_Tc -> 'Tc'\n");
						#endif
						return;
					}					
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "Tw", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_OP_Tw;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_OP_Tw -> 'Tw'\n");
						#endif
						return;
					}					
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "TD", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_OP_TD;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_OP_TD -> 'TD'\n");
						#endif
						return;	
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "Td", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_OP_Td;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_OP_Td -> 'Td'\n");
						#endif
						return;
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "Tm", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_OP_Tm;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_OP_Tm -> 'Tm'\n");
						#endif
						return;
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "T*", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_OP_TASTERISCO;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_OP_TASTERISCO -> 'T*'\n");
						#endif
						return;
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "Tj", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_OP_Tj;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_OP_Tj -> 'Tj'\n");
						#endif
						return;
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "TJ", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_OP_TJ;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_OP_TJ -> 'TJ'\n");
						#endif
						return;
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "\"", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_OP_DOUBLEQUOTE;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_OP_DOUBLEQUOTE -> \"\n");
						#endif
						return;
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "'", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_OP_SINGLEQUOTE;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_OP_SINGLEQUOTE -> '\n");
						#endif
						return;
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "Tf", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_OP_Tf;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_OP_Tf -> FONT SELECTOR -> 'Tf'\n");
						#endif
						return;
					}
					else if ( (pParams->bStringIsDecoded && pParams->bStreamState) && strncmp(pParams->lexeme, "BI", MAX_STRING_LENTGTH_IN_CONTENT_STREAM) == 0 )
					{
						pParams->myToken.Type = T_CONTENT_OP_BI;
						#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
						wprintf(L"GetNextTOken: T_CONTENT_OP_BI -> BEGIN IMAGE -> 'BI'\n");
						#endif
						return;
					}
					else
					{						
						if ( k > 0 )
						{
							pParams->myToken.Type = T_STRING;
							pParams->myToken.Value.vString = (char*)malloc(sizeof(char) * k + sizeof(char));
							if (  pParams->myToken.Value.vString != NULL )
								strcpy(pParams->myToken.Value.vString, pParams->lexeme);
								
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)						
							wprintf(L"GetNextToken: T_STRING -> '%s'\n", pParams->myToken.Value.vString);
							#endif								
						}
						else
						{
							pParams->myToken.Type = T_VOID_STRING;
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)						
							wprintf(L"GetNextToken: T__VOID_STRING\n");
							#endif	
							
							ReadNextChar(pParams);
							
							/*
							c = ReadNextChar(pParams);
							nDelimChar = IsDelimiterChar(c);
							while ( !nDelimChar )
							{
								if ( T_EOF == pParams->myToken.Type )
									return;
								c = ReadNextChar(pParams);
								nDelimChar = IsDelimiterChar(c);
							}
							c = ReadNextChar(pParams);
							if ( T_EOF == pParams->myToken.Type )
								return;
							*/
						}
						
						return;
					}
				}
				break;
			case S1:
				if (  c >= '0' && c <= '9' ) 
				{
					pParams->lexeme[k++] = c;	
				}
				else if ( c == '.' )
				{
					pParams->lexeme[k++] = c;
					state = S2;
				}
				else
				{
					pParams->lexeme[k] = '\0';
					pParams->myToken.Type = T_INT_LITERAL;
					pParams->myToken.Value.vInt = atoi(pParams->lexeme);
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
					wprintf(L"GetNextToken: T_INT_LITERAL -> %d\n", pParams->myToken.Value.vInt);
					#endif
					nDelimChar = IsDelimiterChar(c);
					if ( DELIM_SPECIALSYMBOL == nDelimChar )
					{
						pParams->blockCurPos--;
						pParams->nNumBytesReadFromCurrentStream--;
					}
					return;
				}
				break;
			case S2:
				if (  c >= '0' && c <= '9' ) 
				{
					pParams->lexeme[k++] = c;	
				}
				else
				{
					pParams->lexeme[k] = '\0';

					pParams->myToken.Type = T_REAL_LITERAL;
					pParams->myToken.Value.vDouble = atof(pParams->lexeme);
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)	
					wprintf(L"GetNextTOken: T_REAL_LITERAL -> %lf\n", pParams->myToken.Value.vDouble);
					#endif
					nDelimChar = IsDelimiterChar(c);
					if ( DELIM_SPECIALSYMBOL == nDelimChar )
					{
						pParams->blockCurPos--;
						pParams->nNumBytesReadFromCurrentStream--;
					}
					return;
				}			
				break;
			case S3:
				nDelimChar = IsDelimiterChar(c);
				if ( !nDelimChar )
				{
					if ( c != '#' )
					{
						pParams->lexeme[k++] = c;
					}
					else
					{
						state = S4;
					}
				}
				else
				{
					pParams->lexeme[k] = '\0';
					pParams->myToken.Type = T_NAME;
					pParams->myToken.Value.vString = (char*)malloc(sizeof(char) * k + sizeof(char));
					if (  pParams->myToken.Value.vString != NULL )
						strcpy(pParams->myToken.Value.vString, pParams->lexeme);
						
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)						
					wprintf(L"GetNextToken: T_NAME -> '/%s'\n", pParams->myToken.Value.vString);
					#endif
					
					if ( DELIM_SPECIALSYMBOL == nDelimChar )
					{
						pParams->blockCurPos--;
						pParams->nNumBytesReadFromCurrentStream--;
					}
					
					return;
				}
				break;
			case S4:
				if ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') )
				{
					c1 = c;
					state = S5;
				}
				else
				{
					pParams->myToken.Type = T_ERROR;
					//wprintf(L"Errore GetNextToken: numero esadecimale errato.\n");
					fwprintf(pParams->fpErrors, L"Errore GetNextToken: numero esadecimale errato.\n");
					return;
				}
				break;
			case S5:
				if ( (c >= '0' && c >= '9') || (c >= 'A' && c <= 'F') || (c >= 'f' && c <= 'f') )
				{
					c2 = c;
					pParams->lexeme[k++] = GetHexChar(c1, c2);
					state = S3;
				}
				else
				{
					pParams->myToken.Type = T_ERROR;
					//wprintf(L"Errore GetNextToken: numero esadecimale errato.\n");
					fwprintf(pParams->fpErrors, L"Errore GetNextToken: numero esadecimale errato.\n");
					return;
				}			
				break;
			case S6:
				if ( c != '\\' )
				{
					if ( '\0' != c )
					{
						pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[c];
						pParams->lexeme[k++] = c;
					}
					
					if ( c == ')' )
					{
						pParams->nStackStringOpenParen--; // POP
						
						if ( pParams->nStackStringOpenParen <= 0 )
						{
							pParams->lexeme[--k] = '\0';
							pParams->pUtf8String[k] = L'\0';
							pParams->myToken.Type = T_STRING_LITERAL;
							pParams->myToken.Value.vString = (char*)malloc(sizeof(char) * k + sizeof(char));
							if (  pParams->myToken.Value.vString != NULL )
								strcpy(pParams->myToken.Value.vString, pParams->lexeme);
								
							#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)
							wprintf(L"GetNextToken: T_STRING_LITERAL -> '%s'\n", pParams->myToken.Value.vString);
							#endif
							return;
						}
						else
						{
							pParams->nStackStringOpenParen--; // POP
						}
					}
					
					if ( c == '(' )
					{
						pParams->nStackStringOpenParen++; // PUSH
					}
				}
				else
				{
					state = S7;
				}
				break;
			case S7:
				if ( c == 'n' )
				{
					pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[L'\n'];
					pParams->lexeme[k++] = '\n';
					state = S6;
				}
				else if ( c == 'r' )
				{
					pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[L'\r'];
					pParams->lexeme[k++] = '\r';
					state = S6;
				}
				else if ( c == 't' )
				{
					pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[L'\t'];
					pParams->lexeme[k++] = '\t';
					state = S6;
				}
				else if ( c == 'b' )
				{
					pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[L'\b'];
					pParams->lexeme[k++] = '\b';
					state = S6;
				}
				else if ( c == 'f' )
				{
					pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[L'\f'];
					pParams->lexeme[k++] = '\f';
					state = S6;
				}
				else if ( c == '(' )
				{
					pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[L'('];
					pParams->lexeme[k++] = '(';
					state = S6;
				}
				else if ( c == ')' )
				{
					pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[L')'];
					pParams->lexeme[k++] = ')';
					state = S6;
				}
				else if ( c == '\\' )
				{
					pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[L'\\'];
					pParams->lexeme[k++] = '\\';
					state = S6;
				}
				else if ( c == '\n' )
				{
					state = S6;					
				}
				else if ( c == '\r' )
				{
					state = S8;
				}
				else if ( c >= '0' && c <= '7' )
				{
					c1 = c;
					state = S9;
				}
				else
				{
					pParams->lexeme[k++] = c;
					state = S6;
				}
				break;
			case S8:
				if ( c != '\n' )
				{
					pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[c];
					pParams->lexeme[k++] = c;
				}
				state = S6;
				break;
			case S9:
				if ( c >= '0' && c <= '7' )
				{
					c2 = c;
					state = S10;
				}
				else
				{
					if ( !GetOctalChar(c1, '\0', '\0', &cOctal, &bOctalOverflow) )
					{
						if ( bOctalOverflow )
						{
							state = S6; // ignora
						}
						else
						{
							pParams->myToken.Type = T_ERROR;
							//wprintf(L"Errore GetNextToken: numero ottale errato.\n");
							fwprintf(pParams->fpErrors, L"Errore GetNextToken: numero ottale errato.\n");
							return;
						}
					}
					else
					{
						if ( 0 != cOctal )
						{
							pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[cOctal];
							pParams->lexeme[k++] = cOctal;
						}
						state = S6;
					}
				}
				break;
			case S10:
				if ( c >= '0' && c <= '7' )
				{
					if ( !GetOctalChar(c1, c2, c, &cOctal, &bOctalOverflow) )
					{
						if ( bOctalOverflow )
						{
							state = S6; // ignora
						}
						else
						{
							pParams->myToken.Type = T_ERROR;
							//wprintf(L"Errore GetNextToken: numero ottale errato.\n");
							fwprintf(pParams->fpErrors, L"Errore GetNextToken: numero ottale errato.\n");
							return;
						}
					}
					else
					{
						if ( 0 != cOctal )
						{
							pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[cOctal];
							pParams->lexeme[k++] = cOctal;
						}
						state = S6;
					}
				}
				else
				{
					if ( !GetOctalChar(c1, c2, '\0', &cOctal, &bOctalOverflow) )
					{
						if ( bOctalOverflow )
						{
							state = S6; // ignora
						}
						else
						{
							pParams->myToken.Type = T_ERROR;
							//wprintf(L"Errore GetNextToken: numero ottale errato.\n");
							fwprintf(pParams->fpErrors, L"Errore GetNextToken: numero ottale errato.\n");
							return;
						}
					}
					else
					{
						if ( 0 != cOctal )
						{
							pParams->pUtf8String[k] = pParams->pCurrentEncodingArray[cOctal];
							pParams->lexeme[k++] = cOctal;
						}
						state = S6;
					}
				}			
				break;
			case S11:
				if ( c == '<' )
				{
					pParams->myToken.Type = T_DICT_BEGIN;
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)					
					wprintf(L"GetNextTOken: T_DICT_BEGIN -> <<\n");
					#endif
					return;							
				}
				else if ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') )
				{
					pParams->lexeme[k++] = c;
				}
				else if ( c == '>' )
				{
					if ( k % 2 > 0 )
					{
						k++;
						pParams->lexeme[k] = '0';
					}
					
					y = 0;
					z = 0;
					for ( x = 0; x < k/2; x++ )
					{
						switch ( pParams->lexeme[y] )
						{
							case '0':
								cHexadecimal = 0;
								break;
							case '1':
								cHexadecimal = 16; // 1 * 16;
								break;
							case '2':
								cHexadecimal = 32; // 2 * 16;
								break;
							case '3':
								cHexadecimal = 48; // 3 * 16;
								break;
							case '4':
								cHexadecimal = 64; // 4 * 16;
								break;
							case '5':
								cHexadecimal = 80; // 5 * 16;
								break;
							case '6':
								cHexadecimal = 96; // 6 * 16;
								break;
							case '7':
								cHexadecimal = 112; // 7 * 16;
								break;
							case '8':
								cHexadecimal = 128; // 8 * 16;
								break;
							case '9':
								cHexadecimal = 144; // 9 * 16;
								break;
							case 'A':
							case 'a':
								cHexadecimal = 160; // 10 * 16;
								break;
							case 'B':
							case 'b':
								cHexadecimal = 176; // 11 * 16;
								break;
							case 'C':
							case 'c':
								cHexadecimal = 192; // 12 * 16;
								break;
							case 'D':
							case 'd':
								cHexadecimal = 208 ;// 13 * 16;
								break;
							case 'E':
							case 'e':
								cHexadecimal = 224; // 14 * 16;
								break;
							case 'F':
							case 'f':
								cHexadecimal = 240; // 15 * 16;
								break;
							default:
								cHexadecimal = 0;
								break;
						}
						
						switch ( pParams->lexeme[y + 1] )
						{
							case '0':
								//cHexadecimal += 0;
								break;
							case '1':
								cHexadecimal += 1; // 1 * 16;
								break;
							case '2':
								cHexadecimal += 2;
								break;
							case '3':
								cHexadecimal += 3;
								break;
							case '4':
								cHexadecimal += 4;
								break;
							case '5':
								cHexadecimal += 5;
								break;
							case '6':
								cHexadecimal += 6;
								break;
							case '7':
								cHexadecimal += 7;
								break;
							case '8':
								cHexadecimal += 8;
								break;
							case '9':
								cHexadecimal += 9;
								break;
							case 'A':
							case 'a':
								cHexadecimal += 10;
								break;
							case 'B':
							case 'b':
								cHexadecimal += 11;
								break;
							case 'C':
							case 'c':
								cHexadecimal += 12;
								break;
							case 'D':
							case 'd':
								cHexadecimal += 13;
								break;
							case 'E':
							case 'e':
								cHexadecimal += 14;
								break;
							case 'F':
							case 'f':
								cHexadecimal += 15;
								break;
							default:
								cHexadecimal += 0;
								break;
						}
						
						if ( 0 != cHexadecimal )
							pParams->pUtf8String[z++] = pParams->pCurrentEncodingArray[cHexadecimal];
												
						y += 2;
					}
					
					pParams->pUtf8String[z] = L'\0';
					pParams->lexeme[k] = '\0';
					pParams->myToken.Type = T_STRING_HEXADECIMAL;					
					pParams->myToken.Value.vString = (char*)malloc(sizeof(char) * k + sizeof(char));
					if (  pParams->myToken.Value.vString != NULL )
						strcpy(pParams->myToken.Value.vString, pParams->lexeme);
					
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)
					wprintf(L"GetNextToken: T_STRING_HEXADECIMAL -> '%s'\n", pParams->myToken.Value.vString);
					#endif
					return;					
				}
				else
				{
					pParams->pUtf8String[0] = L'\0';
					
					pParams->myToken.Type = T_ERROR;
					//wprintf(L"Errore GetNextToken: numero esadecimale errato. Carattere non valido -> codice decimale = %u\n", (unsigned int)c);
					fwprintf(pParams->fpErrors, L"Errore GetNextToken: numero esadecimale errato. Carattere non valido -> codice decimale = %u\n", (unsigned int)c);
					
					c = ReadNextChar(pParams);
					
					/*
					while ( '>' != c && ']' != c )
					{
						if ( T_EOF == pParams->myToken.Type )
							return;
						c = ReadNextChar(pParams);
					}
					c = ReadNextChar(pParams);
					if ( T_EOF == pParams->myToken.Type )
						return;					
					*/
					
					return;					
				}
				break;		
			default:
				break;
		} // FINE -> switch ( state )
		
		c = ReadNextChar(pParams);
		if ( pParams->myToken.Type == T_EOF )
			return;
			
		if ( k > MAX_STRING_LENTGTH_IN_CONTENT_STREAM )
		{
			pParams->myToken.Type = T_ERROR;
			//wprintf(L"Errore GetNextToken: lexeme troppo lungo(> %d byte)\n", MAX_STRING_LENTGTH_IN_CONTENT_STREAM);
			fwprintf(pParams->fpErrors, L"Errore GetNextToken: lexeme troppo lungo(> %d byte)\n", MAX_STRING_LENTGTH_IN_CONTENT_STREAM);
			return;
		}						
	} // FINE -> While ( 1 )
		
	return;	
}

unsigned char ReadNextCharLengthObj(Params *pParams)
{	
	if ( pParams->blockCurPosLengthObj >= pParams->blockLenLengthObj )
	{
		pParams->blockLenLengthObj = fread(pParams->myBlockLengthObj, 1, BLOCK_SIZE, pParams->fpLengthObjRef);	
		if ( pParams->blockLenLengthObj == 0 )
		{
			pParams->myTokenLengthObj.Type = T_EOF;
			return '\0';
		}
		pParams->blockCurPosLengthObj = 0;
	}	
			
	return pParams->myBlockLengthObj[pParams->blockCurPosLengthObj++];
}

void GetNextTokenLengthObj(Params *pParams)
{
	States state = S0;
	unsigned char c;
	char lexeme[4096];
	int k = 0;
	int nDelimChar;
		
	if ( (T_NAME == pParams->myTokenLengthObj.Type ||
	 T_STRING == pParams->myTokenLengthObj.Type ||
	 T_STRING_LITERAL == pParams->myTokenLengthObj.Type ||
	 T_STRING_HEXADECIMAL == pParams->myTokenLengthObj.Type) && (NULL != pParams->myTokenLengthObj.Value.vString) )
	{
		free(pParams->myTokenLengthObj.Value.vString);
		pParams->myTokenLengthObj.Value.vString = NULL;
	}
			
	c = ReadNextCharLengthObj(pParams);
	if ( pParams->myTokenLengthObj.Type == T_EOF )
		return;

	while ( c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == '\f' ||  c == '\0' )
	{
		c = ReadNextCharLengthObj(pParams);
		if ( pParams->myTokenLengthObj.Type == T_EOF )
			return;					
	}
	
	while ( 1 )
	{
		switch ( state )
		{
			inizioswitch:
					
			case S0:
				k = 0;
				
				if ( c == '%' )
				{
					c = ReadNextCharLengthObj(pParams);
					while ( c != '\n' && c != '\r' )
					{
						c = ReadNextCharLengthObj(pParams);
						if ( pParams->myTokenLengthObj.Type == T_EOF )
							return;
					}
					if ( c == '\r' )
					{
						c = ReadNextCharLengthObj(pParams);
						if ( pParams->myTokenLengthObj.Type == T_EOF )
							return;	
						if ( c != '\n' )
						{
							goto inizioswitch;
						}					
					}
				}
				else if ( c == '+' || ( c >= '0' && c <= '9' ) )
				{
					lexeme[k++] = c;
					state = S1;
				}
				else if ( c == 'o' )
				{
					state = S2;
				}
				else if ( c == 'e' )
				{
					state = S4;
				}			
				break;
			case S1:
				if (  c >= '0' && c <= '9' ) 
				{
					lexeme[k++] = c;	
				}
				else
				{
					nDelimChar = IsDelimiterChar(c);
					if ( DELIM_SPACECHAR != nDelimChar )
					{
						pParams->myTokenLengthObj.Type = T_ERROR;
						//wprintf(L"Errore GetNextTokenLengthObj: atteso T_INT_LITERAL\n");
						fwprintf(pParams->fpErrors, L"Errore GetNextTokenLengthObj: atteso T_INT_LITERAL\n");
						return;							
					}
					
					lexeme[k] = '\0';
					pParams->myTokenLengthObj.Type = T_INT_LITERAL;
					pParams->myTokenLengthObj.Value.vInt = atoi(lexeme);
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)
					wprintf(L"GetNextTOkenLengthObj: T_INT_LITERAL -> %d\n", pParams->myTokenLengthObj.Value.vInt);
					#endif
					return;
				}
				break;
			case S2:
				if ( c == 'b' )
				{
					state = S3;
				}
				else
				{
					pParams->myTokenLengthObj.Type = T_ERROR;
					//wprintf(L"Errore GetNextTokenLengthObj: atteso T_KW_OBJ\n");
					fwprintf(pParams->fpErrors, L"Errore GetNextTokenLengthObj: atteso T_KW_OBJ\n");
					return;							
				}				
				break;
			case S3:
				if ( c == 'j' )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)
					wprintf(L"\nGetNextTOkenLengthObj T_KW_OBJ\n");
					#endif
					pParams->myTokenLengthObj.Type = T_KW_OBJ;
					return;
				}
				else
				{
					pParams->myTokenLengthObj.Type = T_ERROR;
					//wprintf(L"Errore GetNextTokenLengthObj: atteso T_KW_OBJ\n");
					fwprintf(pParams->fpErrors, L"Errore GetNextTokenLengthObj: atteso T_KW_OBJ\n");
					return;							
				}
				break;
			case S4:
				if ( c == 'n' )
				{
					state = S5;
				}
				else
				{
					pParams->myTokenLengthObj.Type = T_ERROR;
					//wprintf(L"Errore GetNextTokenLengthObj: atteso T_KW_ENDOBJ\n");
					fwprintf(pParams->fpErrors, L"Errore GetNextTokenLengthObj: atteso T_KW_ENDOBJ\n");
					return;							
				}				
				break;
			case S5:
				if ( c == 'd' )
				{
					state = S6;
				}
				else
				{
					pParams->myTokenLengthObj.Type = T_ERROR;
					//wprintf(L"Errore GetNextTokenLengthObj: atteso T_KW_ENDOBJ\n");
					fwprintf(pParams->fpErrors, L"Errore GetNextTokenLengthObj: atteso T_KW_ENDOBJ\n");
					return;							
				}	
				break;
			case S6:
				if ( c == 'o' )
				{
					state = S7;
				}
				else
				{
					pParams->myTokenLengthObj.Type = T_ERROR;
					//wprintf(L"Errore GetNextTokenLengthObj: atteso T_KW_ENDOBJ\n");
					fwprintf(pParams->fpErrors, L"Errore GetNextTokenLengthObj: atteso T_KW_ENDOBJ\n");
					return;							
				}	
				break;
			case S7:
				if ( c == 'b' )
				{
					state = S8;
				}
				else
				{
					pParams->myTokenLengthObj.Type = T_ERROR;
					//wprintf(L"Errore GetNextTokenLengthObj: atteso T_KW_OBJ\n");
					fwprintf(pParams->fpErrors, L"Errore GetNextTokenLengthObj: atteso T_KW_ENDOBJ\n");
					return;							
				}				
				break;
			case S8:
				if ( c == 'j' )
				{
					#if defined(MYDEBUG_PRINT_ALL) || defined(MYDEBUG_PRINT_ON_GetNextToken_FN)
					wprintf(L"\nGetNextTokenLengthObj T_KW_ENDOBJ\n");
					#endif
					pParams->myTokenLengthObj.Type = T_KW_ENDOBJ;
					return;
				}
				else
				{
					pParams->myTokenLengthObj.Type = T_ERROR;
					//wprintf(L"Errore GetNextTokenLengthObj: atteso T_KW_OBJ\n");
					fwprintf(pParams->fpErrors, L"Errore GetNextTokenLengthObj: atteso T_KW_ENDOBJ\n");
					return;							
				}
				break;
			default:
				break;
		} // FINE -> switch ( state )
		
		c = ReadNextCharLengthObj(pParams);
		if ( pParams->myTokenLengthObj.Type == T_EOF )
			return;
			
		if ( k >= 4096 )
		{
			pParams->myTokenLengthObj.Type = T_ERROR;
			//wprintf(L"Errore GetNextTokenLength: lexeme troppo lungo(> %d byte)\n", MAX_STRING_LENTGTH_IN_CONTENT_STREAM);
			fwprintf(pParams->fpErrors, L"Errore GetNextTokenLength: lexeme troppo lungo(> %d byte)\n", MAX_STRING_LENTGTH_IN_CONTENT_STREAM);
			return;
		}
	} // FINE -> While ( 1 )
		
	return;	
}