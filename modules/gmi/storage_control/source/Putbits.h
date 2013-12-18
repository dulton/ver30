#ifndef PUTBITS_H
#define PUTBITS_H


typedef struct PutBitContext 
{
    unsigned int bit_buf;
    int bit_left;
    unsigned char *buf, *buf_ptr, *buf_end;
    int size_in_bits;
} PutBitContext;



#endif /* AVCODEC_PUT_BITS_H */
