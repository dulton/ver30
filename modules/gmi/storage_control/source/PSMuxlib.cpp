#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PSMuxlib.h"
#include "Putbits.h"
#include <assert.h>

#define ERROR(...) do{fprintf(stderr,"[%s:%d]\t",__FILE__,__LINE__);fprintf(stderr,__VA_ARGS__);}while(0)

#if 0
unsigned char SystemHead[24] = 
{ 
	0x00, 0x00, 0x01, 0xBB, 0x00, 0x12, 0x81, 0x47, 0xB1, 0x04,
	0xE1, 0x7F, 0xE0, 0xE0, 0x80, 0xC0, 0xC0, 0x08, 0xBD, 0xE0,
	0x80, 0xBF, 0xE0, 0x80
};
unsigned char ProgramMap[84] = 
{
	0x00, 0x00, 0x01, 0xBC, 0x00, 0x4E, 0xF1, 0xFF, 0x00, 0x24,
	0x40, 0x0E, 0x48, 0x4B, 0x00, 0x01, 0x0D, 0x58, 0xB6, 0x08,
	0x20, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x12, 0x48, 0x4B,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x20, 0x1B, 0xE0,
	0x00, 0x1C, 0x42, 0x0E, 0x00, 0x7B, 0x0E, 0x8D, 0x05, 0x00,
	0x02, 0xD0, 0x11, 0x0F, 0xFF, 0x00, 0x1C, 0x21, 0x2A, 0x0A,
	0x7F, 0xFF, 0x00, 0x00, 0x07, 0x08, 0x1F, 0xFE, 0xA0, 0x5A,
	0x0F, 0xE4, 0xDA, 0xF5
};
#endif

void* Malloc(int size)
{
	return malloc(size);
}

void Free(void *p)
{
	free(p);
}
#if 0

static inline void init_put_bits(PutBitContext *s, unsigned char *buffer, int buffer_size)
{
    s->size_in_bits= 8*buffer_size;
    s->buf = buffer;
    s->buf_end = s->buf + buffer_size;
    s->buf_ptr = s->buf;
    s->bit_left=32;
    s->bit_buf=0;
}

/**
 * @return the total number of bits written to the bitstream.
 */
static inline int put_bits_count(PutBitContext *s)
{
    return (s->buf_ptr - s->buf) * 8 + 32 - s->bit_left;
}


/**
 * Pad the end of the output stream with zeros.
 */
static inline void flush_put_bits(PutBitContext *s)
{
    if (s->bit_left < 32)
        s->bit_buf<<= s->bit_left;
    while (s->bit_left < 32)
	{
        *s->buf_ptr++=s->bit_buf >> 24;
        s->bit_buf<<=8;
        s->bit_left+=8;
    }
    s->bit_left=32;
    s->bit_buf=0;
}

/**
 * Write up to 31 bits into a bitstream.
 * Use put_bits32 to write 32 bits.
 */
static inline void put_bits(PutBitContext *s, int n, unsigned int value)
{
    unsigned int bit_buf;
    int bit_left;

    bit_buf = s->bit_buf;
    bit_left = s->bit_left;


    if (n < bit_left) 
	{
        bit_buf = (bit_buf<<n) | value;
        bit_left-=n;
    } 
	else
	{
        bit_buf<<=bit_left;
        bit_buf |= value >> (n - bit_left);
#ifdef BIG_LETTER
		{
			int *pInt = (int*)s->buf_ptr;
			*pInt = bit_buf;
		}
#else
		s->buf_ptr[3] = (unsigned char)(bit_buf&0xff);
		s->buf_ptr[2] = (unsigned char)((bit_buf>>8)&0xff);
		s->buf_ptr[1] = (unsigned char)((bit_buf>>16)&0xff);
		s->buf_ptr[0] = (unsigned char)((bit_buf>>24)&0xff);
#endif
        s->buf_ptr+=4;
        bit_left+=32 - n;
        bit_buf = value;
    }

    s->bit_buf = bit_buf;
    s->bit_left = bit_left;
}

static inline void put_sbits(PutBitContext *pb, int n, int value)
{
    put_bits(pb, n, value & ((1<<n)-1));
}

/**
 * Write exactly 32 bits into a bitstream.
 */
static void put_direct_bits32(PutBitContext *s, unsigned int value)
{
#ifdef BIG_LETTER
		{
			int *pInt = (int*)s->buf_ptr;
			*pInt = bit_buf;
		}
#else
		s->buf_ptr[3] = (unsigned char)(value&0xff);
		s->buf_ptr[2] = (unsigned char)((value>>8)&0xff);
		s->buf_ptr[1] = (unsigned char)((value>>16)&0xff);
		s->buf_ptr[0] = (unsigned char)((value>>24)&0xff);
#endif
		s->buf_ptr+=4;
}

static inline void put_bits32(PutBitContext *s, unsigned int value)
{
    int lo = value & 0xffff;
    int hi = value >> 16;
    put_bits(s, 16, hi);
    put_bits(s, 16, lo);
}
/**
 * Return the pointer to the byte where the bitstream writer will put
 * the next bit.
 */
static inline unsigned char* put_bits_ptr(PutBitContext *s)
{
	return s->buf_ptr;
}

/**
 * Skip the given number of bytes.
 * PutBitContext must be flushed & aligned to a byte boundary before calling this.
 */
static inline void skip_put_bytes(PutBitContext *s, int n)
{
	s->buf_ptr += n;
}

/**
 * Skip the given number of bits.
 * Must only be used if the actual values in the bitstream do not matter.
 * If n is 0 the behavior is undefined.
 */
static inline void skip_put_bits(PutBitContext *s, int n)
{
    s->bit_left -= n;
    s->buf_ptr-= 4*(s->bit_left>>5);
    s->bit_left &= 31;
}

/**
 * Change the end of the buffer.
 *
 * @param size the new size in bytes of the buffer where to put bits
 */
static inline void set_put_bits_buffer_size(PutBitContext *s, int size)
{
    s->buf_end = s->buf + size;
}


static int put_pack_header(PSMux *pMux, PutBitContext *pb, long long timestamp)
{

    put_direct_bits32(pb, PACK_START_CODE);
//    if (s->is_mpeg2) 
        put_bits(pb, 2, 0x1);
//	else
//        put_bits(&pb, 4, 0x2);
	put_bits(pb, 3,  (unsigned int)((timestamp >> 30) & 0x07));
    put_bits(pb, 1, 1);
    put_bits(pb, 15, (unsigned int)((timestamp >> 15) & 0x7fff));
    put_bits(pb, 1, 1);
    put_bits(pb, 15, (unsigned int)((timestamp      ) & 0x7fff));
    put_bits(pb, 1, 1);
//    if (s->is_mpeg2)
	{
        /* clock extension */
        put_bits(pb, 9, 0);
    }
    put_bits(pb, 1, 1);
    put_bits(pb, 22, pMux->muxrate);
    put_bits(pb, 1, 1);
 //   if (s->is_mpeg2) 
	{
        put_bits(pb, 1, 1);
        put_bits(pb, 5, 0x1f); /* reserved */
        put_bits(pb, 3, 0); /* stuffing length */
    }
    flush_put_bits(pb);
    return 0;
}

#define MPEG_TIME_DIVISOR (90000)
#define TIME_DIVISOR (10000000)
static inline void put_timestamp(PutBitContext *pb, int id, long long timestamp)
{
	timestamp = timestamp*MPEG_TIME_DIVISOR/TIME_DIVISOR;
    put_bits(pb, 8, (id << 4)|(((timestamp >> 30) & 0x07) << 1) |1);
    put_bits(pb, 16, (unsigned short)((((timestamp >> 15) & 0x7fff) << 1) | 1));
    put_bits(pb, 16, (unsigned short)((((timestamp      ) & 0x7fff) << 1) | 1));
}


static int flush_packet(PSMux *pMux, PutBitContext *pb, int stream_index, long long pts, long long dts, long long scr, unsigned char *pbData, int DataLen)
{
    ESInfo *pES = &(pMux->esinfos[stream_index]);
    int payload_size, startcode, header_len;
    int packet_size;
    int pes_flags;

	if (pMux->mHandleDatas==0/*&& pMux->packetnumber>=pMux->packheaderfreq*/)
	{
		//write packheader
		put_pack_header(pMux, pb, scr);
		if (STREAM_TYPE_H264 == pES->streamtype)
		{
			int i;
			bool bfindsps = false;
			for (i=0; i<DataLen-5; i++)
			{
				if (pbData[i] == 0x00 && pbData[i+1] == 0x00 && pbData[i+2]==0x00 && pbData[i+3]==0x01 && pbData[i+4]==0x67)
				{
					bfindsps = true;
					break;
				}
			}
			if (bfindsps)
			{
				for (i=0; i<24; i++)
					put_bits(pb, 8, SystemHead[i]);
				for (i=0; i<84; i++)
					put_bits(pb, 8, ProgramMap[i]);
			}
		}
        pMux->lastsrc= scr;
		pMux->packetnumber = 0;
	}

    packet_size = (pb->size_in_bits - put_bits_count(pb))/8;
    packet_size -= 6;

    /* packet header */
    if (pMux->is_mpeg2) 
	{
        header_len = 3;
    } 
	else
	{
        header_len = 0;
    }
    if (pMux->mHandleDatas==0 && pts != -1) 
	{
        if (dts != pts)
            header_len += 5 + 5;
        else
            header_len += 5;
    } 
	else
	{
        if (!pMux->is_mpeg2)
            header_len++;
    }

    payload_size = packet_size - header_len;
	if (payload_size > DataLen - pMux->mHandleDatas)
	{
		payload_size = DataLen - pMux->mHandleDatas;
		packet_size = payload_size + header_len;
	}
    startcode = PACK_START_CODE_PREFIX + pES->streamtype;

    put_direct_bits32(pb, startcode);
    put_bits(pb, 16, packet_size);

	if (pMux->is_mpeg2)
	{
        put_bits(pb, 8, 0x80); /* mpeg2 id */

        pes_flags=0;

        if (pMux->mHandleDatas==0 && pts != -1) 
		{
            pes_flags |= 0x80;
            if (dts != pts)
                pes_flags |= 0x40;
        }
        put_bits(pb, 8, pes_flags); /* flags */
        put_bits(pb, 8, header_len - 3);

        if (pes_flags & 0x80)  /*write pts*/
            put_timestamp(pb, (pes_flags & 0x40) ? 0x03 : 0x02, pts);
        if (pes_flags & 0x40)  /*write dts*/
            put_timestamp(pb, 0x01, dts);

    } 
	else 
	{
        if (pts != -1)
		{
            if (dts != pts) 
			{
                put_timestamp(pb, 0x03, pts);
                put_timestamp(pb, 0x01, dts);
            } 
			else
			{
                put_timestamp(pb, 0x02, pts);
            }
        }
		else
		{
            put_bits(pb, 8, 0x0f);
        }
    }
	flush_put_bits(pb);
	Memcpy(pb->buf_ptr, pbData+pMux->mHandleDatas, payload_size);
	pMux->mHandleDatas += payload_size;
    pMux->packetnumber++;

    return payload_size+put_bits_count(pb)/8;
}
#endif

int PSMuxInit(PSmux_handle *PSMuxHandle,  PSmux_init_param *InitParam)
{
	PSMux *pMux = NULL;
	if (PSMuxHandle==NULL || InitParam==NULL || InitParam->StreamMode > MPEG2MUX_AUDIO_VIDEO_STREAM)
		return PS_ERROR_PARAM;
	pMux = (PSMux *)Malloc(sizeof(PSMux));
	if (pMux == NULL)
		return PS_ERROR_MEM;
	Memset(pMux, 0, sizeof(PSMux));
	pMux->StreamMode = InitParam->StreamMode;
	if (pMux->StreamMode & MPEG2MUX_VIDEO_STREAM)
	{
		pMux->esinfos[pMux->escount].estype = MPEG2MUX_VIDEO_STREAM;
		pMux->esinfos[pMux->escount].streamtype = InitParam->s_VideoStreamType;
		pMux->videoindex = pMux->escount;
		pMux->escount++;
	}
	if (pMux->StreamMode & MPEG2MUX_AUDIO_STREAM)
	{
		pMux->esinfos[pMux->escount].estype = MPEG2MUX_AUDIO_STREAM;
		pMux->esinfos[pMux->escount].streamtype = InitParam->s_AudioStreamType;
		pMux->audioindex = pMux->escount;
		pMux->escount++;
	}
 	pMux->MaxPacketLen = InitParam->s_MaxPacketlength;
 	pMux->muxrate = InitParam->MuxRate;
 	pMux->is_mpeg2 = 1;
 	*PSMuxHandle = (PSmux_handle)pMux;
 	return PS_ERROR_NONE;
 }
  

#if 0
int PSMuxProcess(PSmux_handle PSMuxHandle,  PSmux_input_info *ESInInfo,  PSmux_out_info *PSMuxOutInfo, int bNewESInfo)
{
	PSMux *pMux = NULL;
	PutBitContext pb;
	long long dts = -1;
	int stream_index = 0;
	if (PSMuxHandle == NULL || ESInInfo == NULL || PSMuxOutInfo == NULL)
		return PS_ERROR_PARAM;
	PSMuxOutInfo->s_PSOutLen = 0;
	if (PSMuxOutInfo->s_PSOutBufferSize < PS_MIN_SIZE)
		return PS_ERROR_NOT_ENOUGH_MEM;
	pMux = (PSMux*)PSMuxHandle;
	if (bNewESInfo)// new esdata	
		pMux->mHandleDatas = 0;
	if (ESInInfo->bVideo)
	{
		stream_index = pMux->videoindex;
		dts = ESInInfo->s_Pts;
	}
	else
	{
		stream_index = pMux->audioindex;
		dts = ESInInfo->s_Pts;
	}
	while(pMux->mHandleDatas < ESInInfo->s_ESInLen)
	{
		if (PSMuxOutInfo->s_PSOutBufferSize-PSMuxOutInfo->s_PSOutLen < PS_MIN_SIZE)
			return PS_ERROR_NOT_ENOUGH_MEM;
		init_put_bits(&pb, PSMuxOutInfo->s_PSOutBuffer+PSMuxOutInfo->s_PSOutLen, 
			PSMuxOutInfo->s_PSOutBufferSize-PSMuxOutInfo->s_PSOutLen>pMux->MaxPacketLen?pMux->MaxPacketLen:PSMuxOutInfo->s_PSOutBufferSize-PSMuxOutInfo->s_PSOutLen);
		PSMuxOutInfo->s_PSOutLen  += flush_packet(pMux, &pb, stream_index, ESInInfo->s_Pts, dts, -1, ESInInfo->s_ESInBuffer, ESInInfo->s_ESInLen);
	}
	return PS_ERROR_NONE;
}
#endif



/*************************************************
these are the code for handle 
*************************************************/

#define  SAFE_COPY_OUTPTR(pout,pin,size) \
do\
{\
	if (leftsize < (size))\
	{\
		ret = PS_ERROR_NOT_ENOUGH_MEM;\
		goto fail;\
	}\
	memcpy(pout,pin,(size));\
	pout += (size);\
	leftsize -= (size);\
	fillsize += (size);\
}while(0)

#define SAFE_COPY_OUT_IN_PTR(pout,pin,size) \
do\
{\
	if ((infillsize + (size)) > insize)\
	{\
		ret = PS_ERROR_PARAM;\
		goto fail;\
	}\
	SAFE_COPY_OUTPTR(pout,pin,size);\
	infillsize += (size);\
	pin += (size);\
}while(0)

#define SAFE_COPY_SCR(pout,pts) \
do\
{\
	unsigned char __fmtscr[6];\
	unsigned long long  __pts = (pts);\
        /*ERROR("scr 0x%llx\n",__pts);*/\
	__fmtscr[0] = (1<<6) | (((__pts >> 30) & 0x7)<<3) | (1<<2) | ((__pts >> 28) & 3);\
	__fmtscr[1] = (__pts >> 20) & 0xff;\
	__fmtscr[2] = ((( __pts >> 15) & 0x1f) << 3) | (1 << 2) | ((__pts >> 13) & 3);\
	__fmtscr[3] = ((__pts >> 5))&0xff;\
	__fmtscr[4] = (__pts & 0x1f) << 3 | (1<<2) | 0;\
	__fmtscr[5] = 1;\
	SAFE_COPY_OUTPTR(pout,__fmtscr,sizeof(__fmtscr));\
}while(0)


#define SAFE_COPY_PTS(pout,pts) \
do\
{\
	unsigned char __fmtpts[5];\
	unsigned long long  __pts = pts;\
	/*ERROR("pts 0x%llx\n",__pts);*/\
	__fmtpts[0] = (2<<4) | (((__pts >> 30) & 0x7)<<1) |1;\
	__fmtpts[1] = (__pts >> 22) & 0xff;\
	__fmtpts[2] = ((( __pts >> 15) & 0x7f) << 1)|1;\
	__fmtpts[3] = ((__pts >> 7))&0xff;\
	__fmtpts[4] = (__pts & 0x7f) << 1 | 1;\
	SAFE_COPY_OUTPTR(pout,__fmtpts,sizeof(__fmtpts));\
}while(0)


#define  SAFE_COPY_LENGTH(pout,size) \
do\
{\
	unsigned char __lenbuf[2];\
	unsigned short __len = size;\
	__lenbuf[0] = (__len >> 8) & 0xff;\
	__lenbuf[1] = __len & 0xff;\
	memcpy(pout,__lenbuf,2);\
}while(0)

#define SKIP_SIZE_OUTPTR(pout,size)\
do\
{\
	if ((size) > leftsize)\
	{\
		ret = PS_ERROR_NOT_ENOUGH_MEM;\
		goto fail;\
	}\
	pout  += (size);\
	fillsize += (size);\
	leftsize -= (size);\
}while(0)




static int SystemHeaderOut(unsigned char *pOutBuf,int outsize)
{
	static unsigned char syshdr[4] = {0x0,0x0,0x1,0xbb};
	static unsigned char ratehdr[] = {0x81,0x47,0xb1,0x04,0xe1,0x7f,0xe0,0xe0,0x80};
	static unsigned char maphdr[4] = {0x0,0x0,0x1,0xbc};
	static unsigned char systable[] = {0x00,0x4e,0xec,0xff,0x00,0x10,0x00,0x00,
								0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
								0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,
								0x1b,0xe0,0x00,0x1c,0x00,0x00,0x00,0x00,
								0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
								0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
								0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	unsigned char *pCurOut = pOutBuf;
	int fillsize = 0;
	unsigned int leftsize= outsize;
	int ret;
	unsigned char* pLengthPtr;
	int slen;

	SAFE_COPY_OUTPTR(pCurOut,syshdr,sizeof(syshdr));
	pLengthPtr = pCurOut;
	SKIP_SIZE_OUTPTR(pCurOut,2);
	SAFE_COPY_OUTPTR(pCurOut,ratehdr,sizeof(ratehdr));
	slen = (pCurOut - pLengthPtr - 2);
	SAFE_COPY_LENGTH(pLengthPtr,slen);
	SAFE_COPY_OUTPTR(pCurOut,maphdr,sizeof(maphdr));
	SAFE_COPY_OUTPTR(pCurOut,systable,sizeof(systable));

	return fillsize;
fail:
	return ret;
}



typedef struct
{
	int m_sidx;
	int m_eidx;
	int m_Type;
} se_idx_t;

typedef struct 
{
	int m_num;
	se_idx_t *m_pIdxs;
} se_idx_mgmt_t;

static int InitSEIdx(se_idx_mgmt_t *pSEIdxMgmt )
{
	memset(pSEIdxMgmt,0,sizeof(*pSEIdxMgmt));
	return 0;
}

static void FreeSEIdx(se_idx_mgmt_t *pSEIdxMgmt)
{
	if (pSEIdxMgmt->m_pIdxs)
	{
		Free(pSEIdxMgmt->m_pIdxs);
	}
	memset(pSEIdxMgmt,0,sizeof(*pSEIdxMgmt));
	return;
}

static int AddSEIdx(se_idx_mgmt_t *pSEIdxMgmt,int sidx,int eidx,int type)
{
	se_idx_t *pIdx=NULL;
	int size = (pSEIdxMgmt->m_num + 1)*sizeof(pSEIdxMgmt->m_pIdxs[0]);
	int cpysize = (pSEIdxMgmt->m_num )*sizeof(pSEIdxMgmt->m_pIdxs[0]);
	pIdx = (se_idx_t*)Malloc(size);
	if (pIdx == NULL)
	{
		return PS_ERROR_MEM;
	}

	if (cpysize)
	{
		memcpy(pIdx,pSEIdxMgmt->m_pIdxs,cpysize);
	}
	pIdx[pSEIdxMgmt->m_num] .m_sidx = sidx;
	pIdx[pSEIdxMgmt->m_num].m_eidx = eidx;
	pIdx[pSEIdxMgmt->m_num].m_Type= type;
	if (pSEIdxMgmt->m_pIdxs)
	{
		Free(pSEIdxMgmt->m_pIdxs);
	}
	pSEIdxMgmt->m_pIdxs = pIdx;
	pSEIdxMgmt->m_num += 1;
	return 0;
}



static int PESHeaderOut(unsigned char* pInBuf,unsigned int insize,
	int* pInFillSize,
	unsigned long long pts,
	unsigned char* pOutBuf,int outsize,int maxpacksize,int ptsset)
{
	static unsigned char noptshdr[] = {0x0,0x2,0xff,0xfc};
	static unsigned char hdr8c [] = {0x8c};
	static unsigned char hdr8007[] = {0x80,0x07};
	static unsigned char hdrfffc[] = {0xff,0xfc};
	static unsigned char e0hdr[]={0x0,0x0,0x1,0xe0};
	int fillsize =0;
	unsigned int infillsize = 0;
	unsigned int leftsize=outsize;
	unsigned int curlen ;
	unsigned char *pCurOut = pOutBuf;
	unsigned char *pCurIn = pInBuf;
	int ret = 0;
	unsigned char* pLengthPtr=NULL;
	int slen;
	int ps =ptsset;

	infillsize = 0;
	leftsize = outsize;
	fillsize = 0;
	while(infillsize < insize)
	{
		curlen = maxpacksize;
		if (curlen > (insize - infillsize))
		{
			curlen = insize - infillsize;
		}

		SAFE_COPY_OUTPTR(pCurOut,e0hdr,sizeof(e0hdr));
		pLengthPtr = pCurOut;
		SKIP_SIZE_OUTPTR(pCurOut,2);
		SAFE_COPY_OUTPTR(pCurOut,hdr8c,sizeof(hdr8c));
		if (ps == 0)
		{
			SAFE_COPY_OUTPTR(pCurOut,hdr8007,sizeof(hdr8007));
			SAFE_COPY_PTS(pCurOut,pts);
			SAFE_COPY_OUTPTR(pCurOut,hdrfffc,sizeof(hdrfffc));
			ps = 1;
		}
		else
		{
			SAFE_COPY_OUTPTR(pCurOut,noptshdr,sizeof(noptshdr));
		}

		SAFE_COPY_OUT_IN_PTR(pCurOut,pCurIn,curlen);
		assert(pLengthPtr);
		slen = (pCurOut - pLengthPtr - 2);
		//ERROR("pCurOut %p pLengthPtr %p slen %d curlen %d insize %d infillsize %d\n",pCurOut,pLengthPtr,slen,curlen,insize,infillsize);
		/*to put the point with the length*/
		SAFE_COPY_LENGTH(pLengthPtr,slen);
	}

	*pInFillSize = infillsize;

	return fillsize;
fail:
	return ret;
	
}


static int GetVideoSection(unsigned char* pChar,int startidx,int len,int *pStartIdx,int *pEndIdx)
{
	static unsigned char vsync_code[4] = {0x0,0x0,0x0,0x1};
	unsigned char *pPtr=pChar;
	unsigned int left;
	int sidx = -1,eidx = -1;
	int type=PS_ERROR_EOF_STREAM;
	if (startidx >= len)
	{
		*pStartIdx = len;
		*pEndIdx = len;
		ERROR("\n");
		return PS_ERROR_EOF_STREAM;
	}

	*pStartIdx = -1;
	*pEndIdx = -1;

	pPtr = &(pChar[startidx]);

	while(pPtr)
	{
		left = len - (int)(pPtr-pChar);
		pPtr = (unsigned char*)memchr(pPtr,0x0,left);
		if (pPtr)
		{
			left = len - (int)(pPtr-pChar);
			//ERROR("pPtr %p pChar %p [0x%02x:0x%02x:0x%02x:0x%02x] left %d \n",pPtr,pChar,pPtr[0],pPtr[1],pPtr[2],pPtr[3]);
			if (left >= sizeof(vsync_code) && memcmp(pPtr,vsync_code,sizeof(vsync_code))==0)
			{
				if (sidx == -1)
				{
					sidx = (pPtr - pChar);
					if (left >= (sizeof(vsync_code) + 2))
					{
						type =(int) (pPtr[4] & 0x1f);
						//ERROR("type %d\n",type);
					}
				}
				else
				{
					/*we find all*/
					eidx = (pPtr - pChar);
					break;
				}
			}
			/*for next search*/
			pPtr += 1;
		}	
	}

	if (sidx >= 0)
	{
		*pStartIdx = sidx;
	}

	
	if (eidx >= 0)
	{
		*pEndIdx = eidx;
	}
	else if (sidx >= 0)
	{
		*pEndIdx = len;
	}

	return type;
}

#undef  PTS_FIXUP
//#define  PTS_FIXUP   1

static int OutputProgramPackHeader(unsigned char* pInBuf,int insize,int* pInFillSize,unsigned long long pts,unsigned char* pOutBuf,int outsize,int maxpacksize)
{
#ifdef PTS_FIXUP
	static unsigned long long st_CurPts=0;	
#endif
	static unsigned long long st_Offset=0;
	unsigned int leftsize=outsize;
	static unsigned char pphdr[4] = {0x0,0x0,0x1,0xba};
	static unsigned char ppafthdr[] = {0x02,0x8f,0x63,0xfe,0xff,0xff,0x00,0x02,0xf7,0x2e};
	int fillsize=0;
	int infillsize = 0;
	unsigned char *pCurIn = pInBuf;
	unsigned char *pCurOut = pOutBuf;
	int ret;
	int syshdrinit;
	int lastsec=0;
	int sidx,eidx;
	int sectype;
	int donesec = 0;
	se_idx_mgmt_t seidxmgt;
	int i;
	int retinfillsize;

	infillsize = 0;
	fillsize = 0;
	leftsize = outsize;
	syshdrinit = 0;
	pCurOut = pOutBuf;
	pCurIn = pInBuf;

	ret = InitSEIdx(&seidxmgt);
	if (ret < 0)
	{
		ret = PS_ERROR_MEM;
		goto fail;
	}
#ifdef PTS_FIXUP	
	if (st_CurPts == 0)
	{
		st_CurPts = 9000;
	}
#endif
	SAFE_COPY_OUTPTR(pCurOut,pphdr,sizeof(pphdr));
#ifdef PTS_FIXUP
	SAFE_COPY_SCR(pCurOut,st_CurPts);
#else
	SAFE_COPY_SCR(pCurOut,pts);
#endif
	SAFE_COPY_OUTPTR(pCurOut,ppafthdr,sizeof(ppafthdr));

	lastsec = infillsize;
	donesec = 0;
	while(donesec == 0)
	{
		//ERROR("lastsec 0x%x insize 0x%x\n",lastsec,(insize-infillsize));
		ret = GetVideoSection(pInBuf,lastsec,(insize-infillsize),&sidx,&eidx);
		if (ret == PS_ERROR_EOF_STREAM)
		{
			break;
		}
		if (ret < 0)
		{
			goto fail;
		}
		assert(sidx >= 0 && eidx >= 0);
		lastsec = eidx;
		sectype = ret;
		//ERROR("type %d offset 0x%llx -- 0x%llx 0x%x\n",sectype,st_Offset ,st_Offset + (eidx - sidx),(eidx-sidx));
		st_Offset += (eidx - sidx);
		switch(sectype)
		{
		case 5:
			syshdrinit = 1;
			/*fall thuru */
		case 1:
			donesec = 1;
			/*fall thuru*/
		case 9:
		case 8:
		case 7:
			ret = AddSEIdx(&seidxmgt,sidx,eidx,sectype);
			break;
		}

		if (ret < 0)
		{
			goto fail;
		}
	}

	//ERROR("num %d\n",seidxmgt.m_num);
#ifdef PTS_FIXUP
	st_CurPts += 3600;
#endif
	if (syshdrinit)
	{
		//ERROR("offset 0x%x\n",fillsize);
		ret = SystemHeaderOut(pCurOut,leftsize);
		if (ret < 0)
		{
			goto fail;
		}
		pCurOut += ret;
		fillsize += ret;
		leftsize -= ret;
	}

	for (i=0;i<seidxmgt.m_num;i++)
	{	
		int ps=i;
		unsigned nalutype;
		//ERROR("offset 0x%x from 0x%x - 0x%x \n",fillsize,seidxmgt.m_pIdxs[i].m_sidx,seidxmgt.m_pIdxs[i].m_eidx);

		nalutype = pInBuf[seidxmgt.m_pIdxs[i].m_sidx + 4];
		/*we should put the type of I-frame or P-Frame before packet*/
		if (seidxmgt.m_pIdxs[i].m_Type == 7 ||seidxmgt.m_pIdxs[i].m_Type == 1 )
		{
			static unsigned char pType7Sec[] = {0x0,0x0,0x0,0x1,0x09,0x10,0x00,0x00};
			static unsigned char pType1Sec[] = {0x0,0x0,0x0,0x1,0x09,0x30,0x00,0x00};
			if (nalutype == 1)
			{
				ret = PESHeaderOut(pType1Sec,sizeof(pType1Sec),&retinfillsize,
#ifdef PTS_FIXUP					
					st_CurPts,
#else
					pts,
#endif					
					pCurOut,leftsize,maxpacksize,ps);
			}
			else
			{
				ret = PESHeaderOut(pType7Sec,sizeof(pType7Sec),&retinfillsize,
#ifdef PTS_FIXUP					
					st_CurPts,
#else					
					pts,
#endif					
					pCurOut,leftsize,maxpacksize,ps);
			}
			if (ret < 0)
			{
				goto fail;
			}
			pCurOut += ret;
			leftsize -= ret;
			fillsize += ret;
			ps ++;
		}

		ret = PESHeaderOut((pInBuf+seidxmgt.m_pIdxs[i].m_sidx),
			(seidxmgt.m_pIdxs[i].m_eidx - seidxmgt.m_pIdxs[i].m_sidx),
			&retinfillsize,
#ifdef PTS_FIXUP			
			st_CurPts,
#else			
			pts,
#endif			
			pCurOut,leftsize,maxpacksize,ps);
		if (ret < 0)
		{
			goto fail;
		}
		pCurOut += ret;
		fillsize += ret;
		leftsize -= ret;
		/*we assume it is ascending one so we make the last for it*/
		infillsize = (pCurIn - pInBuf) + seidxmgt.m_pIdxs[i].m_eidx;
	}

	FreeSEIdx(&seidxmgt);
	*pInFillSize =infillsize;
	return fillsize;
fail:
	FreeSEIdx(&seidxmgt);
	return ret;
}
 
 static int OutputProgramPack(unsigned char* pInBuf,int insize,
 	unsigned long long pts,unsigned char* pOutBuf,
 	int outsize,int maxpacksize)
 {
 	int infillsize;
	int fillsize;
	unsigned char* pCurIn = pInBuf;
	unsigned char* pCurOut = pOutBuf;
	int leftsize ;
	int ret;
	int retinfillsize;

	infillsize = 0;
	leftsize = outsize;
	fillsize = 0;
	while(infillsize < insize)
	{
		ret = OutputProgramPackHeader(pCurIn,
				(insize - infillsize),&retinfillsize,
				pts,pCurOut,leftsize,maxpacksize);
		if (ret < 0)
		{
			return ret;
		}

		fillsize += ret;
		pCurOut += ret;
		leftsize -= ret;

		if (retinfillsize == 0)
		{
			/*we means nothing to input ,so just return*/
			return fillsize;
		}
		pCurIn += retinfillsize;
		infillsize += retinfillsize;
	}

	return fillsize;
 }



int PSMuxProcess(PSmux_handle PSMuxHandle,  PSmux_input_info *ESInInfo,  PSmux_out_info *PSMuxOutInfo, int bNewESInfo)
{
	int ret;	
	PSMux *pMux = (PSMux*)PSMuxHandle;
	unsigned long long pts;
	if (PSMuxHandle == NULL || ESInInfo == NULL || PSMuxOutInfo == NULL)
		return PS_ERROR_PARAM;
	if ( !ESInInfo->bVideo)
	{
		return PS_ERROR_PARAM;
	}

	pts = ESInInfo->s_Pts;
	if (pts < 10000)
	{
		/*make sure it is not overflow*/
		pts *= 9;
		pts /= 1000;
	}
	else
	{
		pts  /= 1000;
		pts *= 9;
	}
	ret = OutputProgramPack(ESInInfo->s_ESInBuffer,ESInInfo->s_ESInLen,pts,
			PSMuxOutInfo->s_PSOutBuffer,PSMuxOutInfo->s_PSOutBufferSize,
			pMux->MaxPacketLen);
	if (ret >= 0)
	{
		PSMuxOutInfo->s_PSOutLen = ret;
		return PS_ERROR_NONE;
	}
	return ret;
}

 
/*******************
*name：PSMuxRelease
*func:资源释放；
*param: PSMuxHandle -- PSMux句柄指针，
===================*/
void PSMuxRelease(PSmux_handle PSMuxHandle )
{
	Safe_free(PSMuxHandle);
}



int InitPSOutput(PSmux_out_info* pOutput)
{
	memset(pOutput,0,sizeof(*pOutput));
	return 0;
}
int ResetPSOutputBuffer(PSmux_out_info* pOutput,int size,int bVideo)
{
	if (pOutput->s_PSOutBuffer == NULL || pOutput->s_PSOutBufferSize < size)
	{
		if (pOutput->s_PSOutBuffer)
		{
			free(pOutput->s_PSOutBuffer);
			pOutput->s_PSOutBuffer = NULL;
		}

		pOutput->s_PSOutBuffer = (unsigned char*)malloc(size);
		if (pOutput->s_PSOutBuffer == NULL)
		{
			pOutput->s_PSOutBufferSize = 0;
			return PS_ERROR_MEM;
		}
		pOutput->s_PSOutBufferSize = size;
	}

	return 0;
}
void FreePSOutput(PSmux_out_info* pOutput)
{
	if (pOutput->s_PSOutBuffer)
	{
		free(pOutput->s_PSOutBuffer);
	}
	memset(pOutput,0,sizeof(*pOutput));
	return ;
}


void* GetOutputPtr(PSmux_out_info * pOutput)
{
	return pOutput->s_PSOutBuffer;
}

int GetOutputLength(PSmux_out_info * pOutput)
{
	return pOutput->s_PSOutLen;
}

