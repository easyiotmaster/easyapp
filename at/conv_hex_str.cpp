#include "conv_hex_str.h"
#include <stdio.h>
#include <string.h>

void BufferToHex(unsigned char *inbuf , short inbuf_len , char *out , short outlen)
{
	short i=0;
	short outlen_index = 0;
	char tmp[4];
	memset(out,0x0,outlen);
	for(i=0;i<inbuf_len;i++)
	{
		snprintf(out+outlen_index,4,"%02X",inbuf[i]);
		outlen_index += 2;

		if (outlen_index >= outlen)
			break;
	}

	//
}

int HexToBuffer(char *hexstr , unsigned char *out , short outlen)
{
	short i=0;
	short out_index = 0;
	short hexstrlen = strlen(hexstr);
	if((hexstrlen % 2) != 0)
		return -1;

	for(i=0;i<hexstrlen;i+=2)
	{
		char tstr[4] = {0,0,0,0};
		tstr[0] = hexstr[i];
		tstr[1] = hexstr[i + 1];
		sscanf(tstr,"%02X",&out[out_index++]);

		if (out_index == outlen)
			break;
	}

	return out_index;
	//
}
