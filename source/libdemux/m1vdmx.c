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

typedef struct
{

	int essize;
	char* es;
	int wr;
	int rd;
	char* video[8];
	int vidsize[8];
}m1vdmx_t;

static unsigned char* m1vdmx_find(unsigned char* buf,int size,char** tail,int* rmd)
{
	while(size >= 4)
	{
		if(!buf[0] && !buf[1] && (buf[2] == 0x01) )
		{
			switch(buf[3])
			{
			case 0x00:
			case 0xb3:
			case 0xb8:
				*tail = buf;
				*rmd = size;
				return buf;
			}
		}
		size --;
		buf ++;
	}
	*tail = buf;
	*rmd = size;
	return NULL;
}

static int m1vdmx_parse_buffer(m1vdmx_t* ps)
{
	char* tail;
	int rmd;
	unsigned char* first;
	unsigned char* next;
	unsigned char* buf = (unsigned char*)ps->es;
	int size = ps->essize;
	int essize = ps->essize;
	int idx = 0;
	while(size > 4)
	{
		first = m1vdmx_find(buf,size,&tail,&rmd);
		if(!first)
		{
			ps->essize = rmd;
			memcpy(ps->es,tail,rmd);
			return 0;
		}
		if(size > 8)
		{
			next = m1vdmx_find(first + 4,size - 4,&tail,&rmd);
		}
		else
		{
			next = NULL;
		}
		if(!next)
		{
			if(first != ps->es)
			{
				size = essize - ((unsigned int)first - (unsigned int)ps->es);
				memcpy(ps->es,first,size);
				ps->essize = size;
			}
			return 0;
		}
		size = (unsigned int)next - (unsigned int)first;
		idx ++;
		memcpy(ps->video[ps->wr],first,size);
		ps->vidsize[ps->wr] = size;
		ps->wr = (ps->wr + 1) & 0x07;
		
		buf = next;
		size = essize - ((unsigned int)next - (unsigned int)ps->es);
	};
	ps->essize = size;
	memcpy(ps->es,buf,size);
	return 0;

}



void* m1vdmx_create(int max_es_size)
{
	int i;
	char* buf;
	m1vdmx_t* dmx = (m1vdmx_t*)osMalloc(sizeof(m1vdmx_t) + max_es_size * 9);
	if(dmx)
	{
		memset(dmx,0,sizeof(m1vdmx_t));
		dmx->es = (char*)(dmx + 1);
		buf = dmx->es + max_es_size;
		for(i = 0; i < 8; i ++)
		{
			dmx->video[i] = buf;
			buf += max_es_size;
		}
	}
	return dmx;
}

int m1vdmx_process(void* dmx,char* buf,int size)
{
	m1vdmx_t* ps = (m1vdmx_t*)dmx;
	if(ps)
	{
		memcpy(ps->es + ps->essize,buf,size);
		ps->essize += size;
		m1vdmx_parse_buffer(ps);		
	}
	return 0;
}

int m1vdmx_getframe(void* dmx,int vid,char* buf,double* dts,double* pts)
{
	int size = 0;
	int wr,rd;
	m1vdmx_t* ps = (m1vdmx_t*)dmx;
	if(ps)
	{
		if(vid == 1)
		{
			wr = ps->wr;
			rd = ps->rd;
			if(wr != rd)
			{
				size = ps->vidsize[rd];
				memcpy(buf,ps->video[rd],ps->vidsize[rd]);
				ps->rd = (rd + 1) & 0x07;
			}
		}
	}
	return size;
}

void m1vdmx_destroy(void* dmx)
{
	osFree(dmx);
}

int m1vdmx_probe(char* buf,int size)
{
	int score = 0;
	bitstream_t bs;
	memset(&bs,0,sizeof(bs));
	bs_reload(&bs,buf,size);
	while(bs_size(&bs) > 4)
	{
		switch(bs_get(&bs,32,0))
		{
		case 0x00000100:
		case 0x000001b3:
		case 0x000001b8:
			score ++;
			break;
		}
		bs_skip(&bs,8);
	}
	if(score > 3)
	{
		return 100;
	}
	return 0;
}
