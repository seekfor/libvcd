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

#include <stdio.h>
#include <fcntl.h>

#ifdef CFG_MPEGENC_SUPPORT

#define W			352
#define H			288
#define FPS			25
#define GOP			25
#define CBR			1
#define BITRATE		1500000

#define MAX_BS_SIZE	(1024 * 1024)
#define YSIZE		(W * H)
#define UVSIZE		(YSIZE / 4)


static	char y[YSIZE];
static	char u[UVSIZE];
static	char v[UVSIZE];

int main(int argc,char* argv[])
{
	FILE* out,*in;
	int size;
	char* es;
	int bssize = 0;
	int frame = 0;
	mpgenc_info_t info;
	void* enc;
	char* in_file,*out_file;

	if(argc > 1)
	{
		in_file = argv[1];
	}
	else
	{
		in_file = "c:\\debug.yuv";
	}
	if(argc > 2)
	{
		out_file = argv[2];
	}
	else
	{
		out_file = "c:\\test.m1v";
	}

	in = fopen(in_file,"rb");
	if(!in)
	{
		return -1;
	}
	out = fopen(out_file,"wb");
	if(!out)
	{
		fclose(in);
		return -2;
	}
	enc = mpgenc_create(MAX_BS_SIZE);
	if(!enc)
	{
		fclose(in);
		fclose(out);
		return -3;
	}
	mpgenc_getinfo(enc,&info);
	info.width = W;
	info.height = H;
	info.fps = FPS;
	info.gop = GOP;
	info.cbr = CBR;
	info.avg_bitrate = BITRATE;
	info.max_bitrate = BITRATE + 200000;
	mpgenc_setinfo(enc,&info);

	while(1)
	{
		if(YSIZE != fread(y,1,YSIZE,in))
		{
			break;
		}
		if(UVSIZE != fread(u,1,UVSIZE,in))
		{
			break;
		}
		if(UVSIZE != fread(v,1,UVSIZE,in))
		{
			break;
		}
		mpgenc_encode(enc,y,u,v);
		size = mpgenc_getbitstream(enc,&es,&info);
		if(size > 0)
		{
			frame ++;
			printf("frame %d encoded,QP = %d,bitrate = %u,size = %d\n",frame,info.qp,info.bitrate,size);
			fwrite(es,1,size,out);
			fflush(out);
		}
	}
	fclose(in);
	fclose(out);
	return 0;
}


#endif

#ifdef CFG_MPEGDEC_SUPPORT

typedef void* (*lpfn_create)(int max_es_size);
typedef int (*lpfn_process)(void* dmx,char* buf,int size);
typedef int (*lpfn_getframe)(void* dmx,int vid,char*buf,double* dts,double* pts);
typedef void (*lpfn_destroy)(void* dmx);

static char es[1024 * 1024];
static char pcm[65536];

int main(int argc,char* argv[])
{
	int is_dat,is_mpg,is_m1v;
	mpegdec_info_t info;
	img_frame_t* frm;
	void* dmx;
	void* dec;
	int frame = 0;
	FILE* in,*out;
	char* y,*u,*v;
	double dts,pts;
	int size;
	char* in_file;
	char* out_file;

	lpfn_create c;
	lpfn_process p;
	lpfn_getframe g;
	lpfn_destroy d;
/*
	if(argc < 2)
	{
		printf("Usage:mpegdec <data filename> [yuv filename]\n");
		return -1;
	}
*/
	if(argc > 1)
	{
		in_file = argv[1];
	}
	else
	{
		in_file = "c:\\vcd\\avseq03.dat";
	}
	
	if(argc > 2)
	{
		out_file = argv[2];
	}
	else
	{
		out_file = "c:\\debug.yuv";
	}

	in = fopen(in_file,"rb");
	size = fread(es,1,1024 * 1024,in);
	fseek(in,0,SEEK_SET);
	
	is_dat = datdmx_probe(es,size);
	is_mpg = mpgdmx_probe(es,size);
	is_m1v = m1vdmx_probe(es,size);
	if(is_dat == 100)
	{
		printf("file %s:it is VCD format\n",in_file);
		c = datdmx_create;
		p = datdmx_process;
		g = datdmx_getframe;
		d = datdmx_destroy;
	}
	else if(is_mpg == 100)
	{
		printf("file %s:it is MPG format\n",in_file);
		c = mpgdmx_create;
		p = mpgdmx_process;
		g = mpgdmx_getframe;
		d = mpgdmx_destroy;
	}
	else if(is_m1v == 100)
	{
		printf("file %s:it is M1V format\n",in_file);
		c = m1vdmx_create;
		p = m1vdmx_process;
		g = m1vdmx_getframe;
		d = m1vdmx_destroy;
	}
	else
	{
		fclose(in);
		printf("file %s:unknown file format!\n",in_file);
		return 0;
	}

	dmx = c(512 * 1024);
	dec = mpgdec_create();
	out = fopen(out_file,"wb");
	while((size = fread(es,1,4096,in)) > 0)
	{
		int len;
		p(dmx,es,size);
	
		while((len = g(dmx,1,es,&dts,&pts)) > 0)
		{
			int consumed;
			consumed = mpgdec_decode(dec,es,len,dts,pts);
			while((frm = mpgdec_getframe(dec,-1,&info)) != NULL)
			{
				++frame;
				printf("frame %d decoded!\n",frame);
				y = frm->y;
				u = frm->u;
				v = frm->v;
				fwrite(y,1,frm->pad_height * frm->pad_width,out);
				fwrite(u,1,frm->pad_height * frm->pad_width / 4,out);
				fwrite(v,1,frm->pad_height * frm->pad_width / 4,out);
			}
		}
	}
	fclose(in);
	fclose(out);
	mpgdec_destroy(dec);
	d(dmx);
	return 0;
}

#endif
