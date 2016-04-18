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
	int x;
	int y;
}point_t;

static unsigned int SAD(unsigned char* src,unsigned char* dst)
{
	unsigned int sum = 0;
	int i;
	for(i = 0; i < 256; i ++)
	{
		sum += abs(src[i] - dst[i]);
	}
	return sum;
}

static void mbcopy(unsigned char* src,int x,int y,int w,int h,unsigned char* dst)
{
	int i;
	src += y * w + x;
	for(i = 0; i < 16; i ++)
	{
		memcpy(dst,src,16);
		dst += 16;
		src += w;
	}
}

static int sdsp(unsigned char* forward,unsigned char* current,int x,int y,int w,int h,int* sad)
{
	unsigned char ref[16][16];
	unsigned char src[16][16];
	int i;
	unsigned int mv = -1;
	unsigned int min = 65536;
	unsigned int sum;
	point_t pt[5] = 
	{
		{x,y},
		{x,y - 1},
		{x - 1,y},
		{x,y + 1},
		{x + 1,y},
	};
	mbcopy(current,x,y,w,h,(unsigned char*)src);
	for(i = 0; i < 5 ; i ++)
	{
		if(pt[i].x < 0 || pt[i].y < 0)
		{
			continue;
		}
		mbcopy(forward,pt[i].x,pt[i].y,w,h,(unsigned char*)ref);
		sum = SAD((unsigned char*)ref,(unsigned char*)src);
		if(sum <= min)
		{
			min = sum;
			mv = (pt[i].x << 16) | pt[i].y;
			*sad = min;
		}
	}
	return mv;
}

static int ldsp(unsigned char* forward,unsigned char* current,int x,int y,int w,int h,int* sad,int loop,int org_x,int org_y)
{
	unsigned char ref[16][16];
	unsigned char src[16][16];
	int i;
	int new_x,new_y;
	unsigned int mv = -1;
	unsigned int min = 65536;
	unsigned int sum;
	point_t pt[9] = 
	{
		{x + 0,y + 0},
		{x + 0,y - 2},
		{x - 1,y - 1},
		{x - 2,y + 0},
		{x - 1,y + 1},
		{x + 0,y + 2},
		{x + 1,y + 1},
		{x + 2,y + 0},
		{x + 1,y - 1}
	};
	mbcopy(current,x,y,w,h,(unsigned char*)src);
	for(i = 0; i < 9 ; i ++)
	{
		if(pt[i].x < 0 || pt[i].y < 0)
		{
			continue;
		}
		mbcopy(forward,pt[i].x,pt[i].y,w,h,(unsigned char*)ref);
		sum = SAD((unsigned char*)ref,(unsigned char*)src);
		if(sum < min)
		{
			min = sum;
			new_x = pt[i].x;
			new_y = pt[i].y;
			mv = (new_x << 16) | new_y;
			*sad = min;
		}
	}

	if(mv < 0)
	{
		return -1;
	}
	
	if(((new_x == x) && (new_y == y))  || (abs(org_x - new_x) > CFG_ME_MAX) || (abs(org_y - new_y) > CFG_ME_MAX))
	{
		if((abs(org_x - new_x) > CFG_ME_MAX) || (abs(org_y - new_y) > CFG_ME_MAX) )
		{
			return -1;
		}
		return sdsp(forward,current,new_x,new_y,w,h,sad);
	}
	--loop;
	if(loop)
	{
		return ldsp(forward,current,new_x,new_y,w,h,sad,loop,org_x,org_y);
	}
	return mv;
}

int mv_calc(unsigned char* ref,unsigned char* src,int x,int y,int w,int h,int* sad,int* mv_x,int* mv_y)
{
	int loop = 4;
	int mv;
	int new_x,new_y;
	mv = ldsp(ref,src,x,y,w,h,sad,loop,x,y);
	if(mv == -1)
	{
		*mv_x = 0;
		*mv_y = 0;
		return 0;
	}
	new_x = ((mv >> 16) & 0xffff);
	new_y = (mv & 0xffff);

	if((new_x + 16) > w || (new_y + 16) > h)
	{
		new_x = x;
		new_y = y;
	}

	x = new_x - x;
	y = new_y - y;

	if((abs(x) > CFG_ME_MAX) || (abs(y) > CFG_ME_MAX) )
	{
		*mv_x = 0;
		*mv_y = 0;
		return 0;
	}
	*mv_x = x << 1;
	*mv_y = y << 1;
	return 0;
}

int mv_calc_mr(int forward_f_code,int x,int prev_x,int* M,int* R)
{
	int mod[] = 
	{
		0,32,64,128,256,512,1024,2048
	};
	int forward_f = 1 << (forward_f_code - 1);
	short int mvd;
	short int delta;
	short int value;
#if CFG_FULLPEL_SUPPORT
	x >>= 1;
#endif
	delta = x - prev_x;
	if(abs(delta) > mod[forward_f_code - 1])
	{
		mvd = delta - Sign(delta) * mod[forward_f_code];
	}
	else
	{
		mvd = delta;
	}
	value = mvd + Sign(mvd) * (forward_f - 1);
	*M = value / forward_f;
	*R = abs(value % forward_f);
	return x;
}