/**********seekfor libvcd v20.16.4.5*********************************************
	seekfor libvcd v20.16.4.5是免费的MPEG编解码软件,您可以自由使用和拷贝本程序!
	在使用和拷贝时请注意保留原来的版权信息.
	如果您有什么问题和意见,请和我联系!
	QQ:82054357
	QQ群:24613876
	MSN:sfrad32@hotmail.com
	Mail:Seek_for@163.com
***********************************************************************************/
#include <common.h>

static void bs_read(bitstream_t* bs,int num)
{
	unsigned int value = 0;
	int i;
	for(i = 0; i < num; i ++,bs->src++)
	{
		value <<= 8;
		value |= *bs->src;
	}
	bs->value |= (value << bs->rd);
}

bitstream_t* bs_create(int size)
{
	bitstream_t* bs = (bitstream_t*)osMalloc(sizeof(bitstream_t) + size);
	if(bs)
	{
		bs->src = NULL;
		bs->wr = 0;
		bs->rd = 0;
		bs->value = 0;
		bs->size = 0;
		if(size > 0)
		{
			bs->src = (unsigned char*)(bs + 1);
			bs->base = bs->src;
			bs->size = size;
			bs->bufsize = size;
			memset(bs->src,0x00,size);
		}

	}
	return bs;
}


int bs_reload(bitstream_t*bs,char* src,int size)
{
	if(bs->size == 0)
	{
		bs->wr = 0;
		bs->rd = 0;
		bs->value = 0;
		bs->bufsize = size;
		bs->src = src;
		bs->base = src;
		bs_read(bs,4);
	}
	return 0;
}


void bs_skip(bitstream_t* bs,int bit)
{
	int num;
	if(bit < 32)
	{
		bs->value <<= bit;
	}
	else
	{
		bs->value = 0;
	}
	bs->rd += bit;
	bs->wr += bit;
	num = (bs->rd >> 3);
	if(num)
	{
		bs->rd -= (num << 3);
		bs_read(bs,num);
	}
}

int bs_get(bitstream_t* bs,int bit,int skip)
{
	unsigned int value;

	if(bit < 32)
	{
		value = bs->value >> (32 - bit);
	}
	else
	{
		value = bs->value;
	}
	if(skip)
	{
		bs_skip(bs,bit);
	}
	return value;
}

int bs_put(bitstream_t* bs,int bit,unsigned int value)
{
	if(bs->wr)
	{
		int rmd = 8 - bs->wr;
		if(rmd >= bit)
		{
			bs->src[0] |= value << (rmd - bit);
			rmd = (bs->wr + bit) & 0x07;
			if(rmd == 0)
			{
				bs->src ++;
			}
			bs->wr = rmd;
			return 0;
		}
		else
		{
			value <<= (32 - bit);
			bs->src[0] |= value >> (32 - rmd);
			bs->src ++;
			bit -= rmd;
			value <<= rmd;
		}
	}
	else if(bit < 32)
	{
		value <<= (32 - bit);
	}
	while(bit >= 8)
	{
		bs->src[0] |= value >> 24;
		bs->src ++;
		value <<= 8;
		bit -= 8;
	}

	bs->wr = 0;
	if(bit)
	{
		bs->src[0] |= (value >> (32 - bit)) << (8 - bit);
		bs->wr = bit;	
	}
	return 0;
}

void bs_flush(bitstream_t* bs)
{
	bs->src ++;
}

int bs_align(bitstream_t* bs)
{
	if(bs->size == 0)
	{
		int rd;
		rd = bs->rd & 0x07;
		if(rd)
		{
			bs_skip(bs,8 - rd);
		}
	}
	else
	{
		if(bs->wr)
		{
			bs->src ++;
			bs->wr = 0;
		}
	}
	return 0;
}

int bs_reset(bitstream_t* bs)
{
	if(bs->size)
	{
		memset(bs->base,0,bs->size);
	}
	bs->src = bs->base;
	bs->wr = 0;
	bs->rd = 0;
	return 0;
}

int bs_size(bitstream_t* bs)
{
	if(bs->size == 0)
	{
		return bs->bufsize - bs_consumed(bs);
	}
	return (bs->wr + 7) / 8;
}

int bs_consumed(bitstream_t* bs)
{
	if(bs->size == 0)
	{
		int size = (bs->wr + 7) >> 3;
		return size;
	}
	return (bs->src - bs->base);
}

int bs_destroy(bitstream_t* bs)
{
	osFree(bs);
	return 0;
}




