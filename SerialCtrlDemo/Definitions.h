#pragma once
// "Definitions.h"
typedef union
{
	BYTE b[2];
	UINT16 integer;
} MConvertType;

#define CR 13
#define	STX '>'
#define	DLE '/'
#define	ETX CR

//#define MAX_DATA_BYTES 256
//#define MAXPACKETSIZE (MAX_DATA_BYTES * 4)

#define SUCCESS 0