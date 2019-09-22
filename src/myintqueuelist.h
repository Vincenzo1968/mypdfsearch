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

#ifndef	__MYINTQUEUELIST_H
#define	__MYINTQUEUELIST_H

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

#ifndef MAX_STRLEN
#define MAX_STRLEN 4096
#endif

typedef struct _MyIntQueueItem
{
	int nItem;
	struct _MyIntQueueItem* next;
} MyIntQueueItem_t;

typedef struct _MyIntQueueList
{
	unsigned int count;
	MyIntQueueItem_t* head;
	MyIntQueueItem_t* tail;
} MyIntQueueList_t;


BEGIN_C_DECLS

int  myintqueuelist_Init(MyIntQueueList_t* p);
void myintqueuelist_Free(MyIntQueueList_t* p);
int  myintqueuelist_IsEmpty(MyIntQueueList_t* p);
int  myintqueuelist_Enqueue(MyIntQueueList_t* p, int num);
int  myintqueuelist_Dequeue(MyIntQueueList_t* p, int* pNum);
int  myintqueuelist_Peek(MyIntQueueList_t* p, int* pNum);

END_C_DECLS

#endif /* __MYINTQUEUELIST_H */
