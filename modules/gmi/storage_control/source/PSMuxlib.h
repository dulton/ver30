#ifndef PSMUXLIB_H
#define PSMUXLIB_H

#include "PSMuxlib_Interface.h"
#define PACK_START_CODE 0x000001BA
#define PACK_START_CODE_PREFIX  0x00000100
#define ESINFO_MAX   2
#define PS_TIMESTAMP_BASE  90000LL
#define PS_MIN_SIZE        100
typedef struct PESInfo_st
{
	int estype;
	int streamtype;
}ESInfo;
typedef struct PSMux_st
{
	ESInfo esinfos[ESINFO_MAX];
	int    escount;
	int    videoindex;
	int    audioindex;
	int    StreamMode;
	int    MaxPacketLen;
	int    muxrate;
	int    mHandleDatas;
	int    packetnumber;
	int    packheaderfreq;
	int    is_mpeg2;
	float  framerate;
	long long lastsrc;
}PSMux;


//function
void* Malloc(int size);
void Free(void *p);
#define Safe_free(x) {if (x) Free(x); x=NULL;}
#define Memset memset
#define Memcpy memcpy
#endif

