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

#define VCD_SECTOR_SIZE		2352
#define VCD_HDR_SIZE		24
#define VCD_CS_SIZE			4
#define VCD_DATA_SIZE		VCD_SECTOR_SIZE - VCD_HDR_SIZE - VCD_CS_SIZE

typedef struct
{
	int step;
	void* dmx;

	int size;
	char* es;
}datdmx_t;

static int datdmx_parse_buffer(datdmx_t* dmx)
{
	char* buf = dmx->es;
	int size = dmx->size;

	if(dmx->step == 0)
	{
		while(size >= 4)
		{
			if(buf[0] == 'R' && buf[1] == 'I' && buf[2] == 'F' && buf[3] == 'F')
			{
				dmx->step = 1;
				buf += 0x2c;
				size -= 0x2c;
				break;
			}
			buf++;
			size --;
		}
	}

	while(size >= VCD_SECTOR_SIZE)
	{
		mpgdmx_process(dmx->dmx,buf + VCD_HDR_SIZE,VCD_DATA_SIZE);
		buf += VCD_SECTOR_SIZE;
		size -= VCD_SECTOR_SIZE;
	}
	if(size > 0)
	{
		memcpy(dmx->es,buf,size);
		dmx->size = size;
	}
	else
	{
		dmx->size = 0;
	}
	return 0;
}

void* datdmx_create(int max_es_size)
{
	datdmx_t* dmx = (datdmx_t*)osMalloc(sizeof(datdmx_t) + max_es_size);
	if(dmx)
	{
		dmx->size = 0;
		dmx->step = 0;
		dmx->es = (char*)(dmx + 1);
		dmx->dmx = mpgdmx_create(max_es_size);
	}
	return dmx;
}

int datdmx_process(void* dmx,char* buf,int size)
{
	datdmx_t* dat = (datdmx_t*)dmx;
	if(dat)
	{
			memcpy(dat->es + dat->size,buf,size);
			dat->size += size;
			datdmx_parse_buffer(dat);		
	}
	return 0;
}

int datdmx_getframe(void* dmx,int vid,char* buf,double* dts,double* pts)
{
	datdmx_t* dat = (datdmx_t*)dmx;
	if(dat && dat->dmx)
	{
		return mpgdmx_getframe(dat->dmx,vid,buf,dts,pts);
	}
	return 0;
}

void datdmx_destroy(void* dmx)
{
	datdmx_t* dat = (datdmx_t*)dmx;
	if(dat)
	{
		m1vdmx_destroy(dat->dmx);	
		osFree(dmx);
	}
}

int datdmx_probe(char* buf,int size)
{
	int score = 0;
	bitstream_t bs;
	memset(&bs,0,sizeof(bs));
	bs_reload(&bs,buf,size);
	/*find RIFF*/
	while(bs_size(&bs) > 4)
	{
		if(bs_get(&bs,32,0) == FOURCC('R','I','F','F'))
		{
			score ++;
			break;
		}
		bs_skip(&bs,8);
	}
	if(score)
	{
		/*find CDXA*/
		while(bs_size(&bs) > 4)
		{
			if(bs_get(&bs,32,0) == FOURCC('C','D','X','A'))
			{
				score ++;
				break;
			}
			bs_skip(&bs,8);
		}
		if(score > 1)
		{
			/*find data*/
			while(bs_size(&bs) > 4)
			{
				if(bs_get(&bs,32,0) == FOURCC('d','a','t','a'))
				{
					score ++;
					break;
				}
				bs_skip(&bs,8);
			}
		}
	}
	if(score >= 3)
	{
		return 100;
	}
	return 0;
}