#ifndef __c_h_s_h__
#define __c_h_s_h__


extern void BufferToHex(unsigned char *inbuf , short inbuf_len , char *out , short outlen);
extern int HexToBuffer(char *hexstr , unsigned char *out , short outlen);

#endif
