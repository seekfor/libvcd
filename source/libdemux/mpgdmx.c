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
	int vidpids[32];
	int audpids[32];

	int vidpid;
	int audpid;

	int essize;
	char* es;

	double last_video_dts;
	double last_video_pts;


	void* m1vdmx;
	char* video;

	int audwr;
	int audrd;
	int audsize[8];
	double auddts[8];
	double audpts[8];
	char* audio[8];
}mpgdmx_t;

static void mpgdmx_parse_pack(mpgdmx_t*ps,bitstream_t* bs)
{
	bs_skip(bs,32);
	bs_skip(bs,8);
	bs_skip(bs,16);
	bs_skip(bs,16);
	bs_skip(bs,24);
}

static void mpgdmx_parse_header(mpgdmx_t*ps,bitstream_t* bs)
{
	int vidx = 0;
	int aidx = 0;
	int idx;
	int len;
	bs_skip(bs,32);
	len = bs_get(bs,16,1);
	bs_skip(bs,24);
	bs_skip(bs,24);
	while(bs_get(bs,1,0) == 1)
	{
		idx = bs_get(bs,8,1);
		bs_skip(bs,16);
		if((idx >= 0xc0) && (idx <= 0xdf) && (aidx < 32) )
		{
			ps->audpids[aidx] = idx;
			aidx ++;
		}
		else if((idx >= 0xe0) && (idx <= 0xef)  && (vidx < 32) )
		{
			ps->vidpids[vidx] = idx;
			vidx ++;
		}
	}
	
	if((vidx == 0) && (ps->vidpids[0] == 0))
	{
		ps->vidpids[0] = 0xe0;
	}
	if((aidx == 0) && (ps->audpids[0] == 0))
	{
		ps->audpids[0] = 0xc0;
	}
	
	ps->vidpid = ps->vidpids[0];
	ps->audpid = ps->audpids[0];
}

static void mpgdmx_parse_packet(mpgdmx_t*ps,bitstream_t* bs)
{
	int pts_hi,pts_med,pts_low;
	__int64 pts = 0,dts = 0;
	int idx;
	int len;
	int hdr = 0;
	int pts_ok = 0;

	char* es;
	int size;

	bs_skip(bs,24);
	idx = bs_get(bs,8,1);
	len = bs_get(bs,16,1);
	if((idx == ps->vidpid) || (idx == ps->audpid))
	{
		while(bs_get(bs,8,0) == 0xff)
		{
			bs_skip(bs,8);
			hdr ++;
		}
		if(bs_get(bs,2,0) == 0x01)
		{
			bs_skip(bs,16);
			hdr += 2;
		}
		switch(bs_get(bs,4,0))
		{
		default:
			break;
		case 0x02:
			pts_hi = (bs_get(bs,8,1) >> 1) & 0x07;
			pts_med = bs_get(bs,16,1) >> 1;
			pts_low = bs_get(bs,16,1) >> 1;
			hdr += 5;
			dts = pts = (pts_hi << 30) | (pts_med << 15) | pts_low;
			pts_ok = 1;
			break;
		case 0x03:
			pts_hi = (bs_get(bs,8,1) >> 1) & 0x07;
			pts_med = bs_get(bs,16,1) >> 1;
			pts_low = bs_get(bs,16,1) >> 1;
			pts = (pts_hi << 30) | (pts_med << 15) | pts_low;

			pts_hi = (bs_get(bs,8,1) >> 1) & 0x07;
			pts_med = bs_get(bs,16,1) >> 1;
			pts_low = bs_get(bs,16,1) >> 1;
			dts = (pts_hi << 30) | (pts_med << 15) | pts_low;
			pts_ok = 1;
			hdr += 10;
			break;
		case 0x00:
			bs_skip(bs,8);
			hdr += 1;
			break;
		}
		
		if(idx == ps->vidpid)
		{
			int i;
			es = ps->video;
			size = len - hdr;
			for(i = 0; i < size; i ++)
			{
				*es++ = bs_get(bs,8,1);
			}
			m1vdmx_process(ps->m1vdmx,ps->video,size);
			if(pts_ok)
			{
				ps->last_video_dts = dts / 90.0f;
				ps->last_video_pts = pts / 90.0f;
			}
		}
		else
		{
			size = len - hdr;
			if(size > 0)
			{
				int i;
				if(pts_ok)
				{
					ps->auddts[ps->audwr] = dts / 90.0f;
					ps->audpts[ps->audwr] = pts / 90.0f;
				}
				else
				{
					ps->auddts[ps->audwr] = 0;
					ps->audpts[ps->audwr] = 0;
				}
				ps->audsize[ps->audwr] = size;
				es = ps->audio[ps->audwr];
				for(i = 0; i < size; i ++)
				{
					*es++ = bs_get(bs,8,1);
				}
				ps->audwr = (ps->audwr + 1) & 0x07;
			}
		}
	}
}


static int mpgdmx_parse_frame(mpgdmx_t* ps,char* buf,int size)
{
	bitstream_t bs;
	int code;
	memset(&bs,0,sizeof(bs));
	bs_reload(&bs,buf,size);
	while(bs_size(&bs) > 4)
	{
		bs_align(&bs);
		code = bs_get(&bs,32,0);
		switch(code)
		{
			case 0x000001ba:
				mpgdmx_parse_pack(ps,&bs);
				continue;
			case 0x000001bb:
				mpgdmx_parse_header(ps,&bs);
				continue;
			default:
				if((code >= 0x000001c0) && (code <= 0x000001ef) )
				{
					mpgdmx_parse_packet(ps,&bs);
					continue;
				}
				break;
		}
		bs_skip(&bs,8);
	}
	return 0;
}


static unsigned char* mpgdmx_find(unsigned char* buf,int size,char** tail,int* rmd)
{
	while(size >= 4)
	{
		if(!buf[0] && !buf[1] && (buf[2] == 0x01) )
		{
			switch(buf[3])
			{
			case 0xba:
			case 0xbb:
				*tail = buf;
				*rmd = size;
				return buf;
			default:
				if((buf[3] >= 0xc0) && (buf[3] <= 0xef) )
				{
					*tail = buf;
					*rmd = size;
					return buf;
				}
				break;
			}
		}
		size --;
		buf ++;
	}
	*tail = buf;
	*rmd = size;
	return NULL;
}

static int mpgdmx_parse_buffer(mpgdmx_t* ps)
{
	char* tail;
	int rmd;
	unsigned char* first;
	unsigned char* next;
	unsigned char* buf = (unsigned char*)ps->es;
	int size = ps->essize;
	int essize = ps->essize;

	do
	{
		first = mpgdmx_find(buf,size,&tail,&rmd);
		if(!first)
		{
			ps->essize = rmd;
			memcpy(ps->es,tail,rmd);
			return 0;
		}
		if(size > 4)
		{
			next = mpgdmx_find(first + 4,size - 4,&tail,&rmd);
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
		mpgdmx_parse_frame(ps,first,size);

		size = rmd;
		buf = next;
	}while(size > 4);	
	ps->essize = rmd;
	memcpy(ps->es,next,rmd);
	return 0;

}


void* mpgdmx_create(int max_es_size)
{
	int i;
	char* buf;
	mpgdmx_t* dmx = (mpgdmx_t*)osMalloc(sizeof(mpgdmx_t) + max_es_size * 10);
	if(dmx)
	{
		memset(dmx,0,sizeof(mpgdmx_t));
		dmx->vidpid = 0xe0;
		dmx->audpid = 0xc0;
		dmx->audwr = 0;
		dmx->m1vdmx = m1vdmx_create(max_es_size);
		dmx->es = (char*)(dmx + 1);
		buf = dmx->es + max_es_size;
		dmx->video = buf;
		buf += max_es_size;
		for(i = 0; i < 8; i ++)
		{
			dmx->audio[i] = buf;
			buf += max_es_size;
		}
	}
	return dmx;
}

int mpgdmx_process(void* dmx,char* buf,int size)
{
	mpgdmx_t* ps = (mpgdmx_t*)dmx;
	if(ps)
	{
		memcpy(ps->es + ps->essize,buf,size);
		ps->essize += size;
		mpgdmx_parse_buffer(ps);		
	}
	return 0;
}

int mpgdmx_getframe(void* dmx,int vid,char* buf,double* dts,double* pts)
{
	int size = 0;
	int wr,rd;
	mpgdmx_t* ps = (mpgdmx_t*)dmx;
	if(ps)
	{
		if(vid == 1)
		{
			*dts = ps->last_video_dts;
			*pts = ps->last_video_pts;
			return m1vdmx_getframe(ps->m1vdmx,vid,buf,dts,pts);
		}
		else
		{
			wr = ps->audwr;
			rd = ps->audrd;
			if(wr != rd)
			{
				size = ps->audsize[rd];
				*dts = ps->auddts[rd];
				*pts = ps->audpts[rd];
				memcpy(buf,ps->audio[rd],size);
				ps->audrd = (rd + 1) & 0x07;
			}
		}

	}
	return size;
}

void mpgdmx_destroy(void* dmx)
{
	mpgdmx_t* ps = (mpgdmx_t*)dmx;
	m1vdmx_destroy(ps->m1vdmx);
	osFree(dmx);
}

int mpgdmx_probe(char* buf,int size)
{
	int score = 0;
	bitstream_t bs;
	memset(&bs,0,sizeof(bs));
	bs_reload(&bs,buf,size);
	while(bs_size(&bs) > 4)
	{
		switch(bs_get(&bs,32,0))
		{
		case 0x000001bb:
		case 0x000001ba:
			score ++;
			break;
		}
		bs_skip(&bs,8);
	}
	if(score > 2)
	{
		return 100;
	}
	return 0;
}