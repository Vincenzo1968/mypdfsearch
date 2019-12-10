#ifndef	__MY_ENCODING_H
#define	__MY_ENCODING_H

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

#ifdef _MSC_VER
#define INLINE __forceinline /* use __forceinline (VC++ specific) */
#else
#define INLINE inline        /* use standard inline */
#endif

#include <stdint.h>

#include "mypdfsearch.h"

#define ASCIIHEX_ERROR_NONE                   0
#define ASCIIHEX_ERROR_EOD_NOT_FOUND          1
#define ASCIIHEX_ERROR_MEMALLOC_FAILED        2
#define ASCIIHEX_ERROR_UNEXPECTED_CHAR        3

#define ASCII85_ERROR_NONE                    0
#define ASCII85_ERROR_EOD_NOT_FOUND           1
#define ASCII85_ERROR_MEMALLOC_FAILED         2
#define ASCII85_ERROR_UNEXPECTED_CHAR         3

#define LZW_ERROR_NONE                        0
#define LZW_ERROR_UNEXPECTED_CODE             1
#define LZW_ERROR_MEMALLOC_FAILED             2
#define LZW_ERROR_TABLE_INIT_FAILED           3
#define LZW_ERROR_TABLE_INSERT_FAILED         4
#define LZW_ERROR_TABLE_FIND_FAILED           5
#define LZW_ERROR_WRITEBITS_FAILED            6
#define LZW_ERROR_READBITS_FAILED             7
#define LZW_ERROR_STRING_NOT_FOUND_IN_TABLE   8
#define LZW_ERROR_EOD_NOT_FOUND               9

#define MY_BUFFER_SIZE 4096

#define SwapTwoBytes(data) \
( (((data) >> 8) & 0x00FF) | (((data) << 8) & 0xFF00) ) 

#define SwapFourBytes(data)   \
( (((data) >> 24) & 0x000000FF) | (((data) >>  8) & 0x0000FF00) | \
  (((data) <<  8) & 0x00FF0000) | (((data) << 24) & 0xFF000000) ) 

#define SwapEightBytes(data)   \
( (((data) >> 56) & 0x00000000000000FF) | (((data) >> 40) & 0x000000000000FF00) | \
  (((data) >> 24) & 0x0000000000FF0000) | (((data) >>  8) & 0x00000000FF000000) | \
  (((data) <<  8) & 0x000000FF00000000) | (((data) << 24) & 0x0000FF0000000000) | \
  (((data) << 40) & 0x00FF000000000000) | (((data) << 56) & 0xFF00000000000000) ) 

#define CHAR_BITS    8  // size of character
#define INT_BITS_64  ( sizeof(uint64_t) * CHAR_BITS)
#define INT_BITS_32  ( sizeof(uint32_t) * CHAR_BITS)
#define INT_BITS_16  ( sizeof(uint16_t) * CHAR_BITS)
#define INT_BITS_8   ( sizeof(uint8_t)  * CHAR_BITS)

// https://it.wikipedia.org/wiki/Lempel-Ziv-Welch

/* ----------- LZW stuff -------------- */ 
#define M_CLR 256 /* clear table marker */
#define M_EOD 257 /* end-of-data marker */
#define M_NEW 258 /* new code index */


#define BIT_SET(byte,nbit)   ((byte) |=  (1 << (nbit)))
#define BIT_UNSET(byte,nbit) ((byte) &= ~(1 << (nbit)))
#define BIT_CHECK(byte,nbit) ((byte) &   (1 << (nbit)))
#define BIT_FLIP(byte,nbit)  ((byte) ^=  (1 << (nbit))) // Se l'ennesimo bit(nbit) è zero, lo imposta a uno; e viceversa.

typedef struct tagEncodeParams
{
	unsigned char *pOut;
	size_t sizeOutputStream;
	uint32_t currBitsNum;
	uint32_t offsetOutputStream;
	uint32_t BitsWritten;
} EncodeParams;

typedef struct tagDecodeParams
{
	unsigned char *pIn;
	size_t lenInput;
	uint32_t currBitsNum;
	uint32_t offsetInputStream;
	uint32_t BitsWritten;
} DecodeParams;

BEGIN_C_DECLS

int IsLittleEndian();

int InitEncodeParams(EncodeParams *p, unsigned char *pOut, size_t sizeOutputStream,  uint32_t BitsNum);
int InitDecodeParams(DecodeParams *p, unsigned char *pIn, size_t lenInput, uint32_t BitsNum);

int WriteBits(uint16_t code, EncodeParams *p);
int ReadBits(uint16_t *code, DecodeParams *p);

void PrintInBinary64_Wide(uint64_t n);
void PrintInBinary64(uint64_t n);

void PrintInBinary32_Wide(uint32_t n);
void PrintInBinary32(uint32_t n);

void PrintInBinary16_Wide(uint16_t n);
void PrintInBinary16(uint16_t n);

void PrintInBinary8_Wide(uint8_t n);
void PrintInBinary8(uint8_t n);

// ----------------------------------------------- Public Interface Functions -------------------------------------------------

unsigned char * asciiHexEncode(unsigned char * pStream, size_t len, size_t *pLenOut, uint32_t *pErrorCode);
unsigned char * asciiHexDecode(unsigned char * pStream, size_t len, size_t *pLenOut, uint32_t *pErrorCode);

unsigned char * ascii85Encode(unsigned char * pStream, size_t len, size_t *pLenOut, uint32_t *pErrorCode);
unsigned char * ascii85Decode(unsigned char * pStream, size_t len, size_t *pLenOut, uint32_t *pErrorCode);

unsigned char * lzwEncode(unsigned char * pStream, size_t len, size_t *pLenOut, uint32_t *pErrorCode);
unsigned char * lzwDecode(unsigned char * pStream, size_t len, size_t *pLenOut, uint32_t *pErrorCode);

END_C_DECLS

#endif /* __MY_ENCODING_H */
