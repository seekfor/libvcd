/**********seekfor mpegdec v20.15.12.10*********************************************
	seekfor mpegdec v20.15.12.10是免费的MPEG解码软件,您可以自由使用和拷贝本程序!
	在使用和拷贝时请注意保留原来的版权信息.
	如果您有什么问题和意见,请和我联系!
	QQ:82054357
	QQ群:24613876
	MSN:sfrad32@hotmail.com
	Mail:Seek_for@163.com
***********************************************************************************/
#ifndef __COMMON_H__
#define __COMMON_H__


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <malloc.h>

#include <windows.h>
#include <mmsystem.h>

#include <os.h>

#include <config.h>

#define FOURCC(a,b,c,d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d) )

typedef struct
{
	int bit;
	int value;
	int vlc;
}vld_t;

typedef vld_t vlc_t;

#define		MPG_TYPE_I			1
#define		MPG_TYPE_P			2
#define		MPG_TYPE_B			3
#define		MPG_TYPE_D			4


#define		MPG_MBT_QUANT		0x10	
#define		MPG_MBT_MVF			0x08
#define		MPG_MBT_MVB			0x04
#define		MPG_MBT_CBP			0x02
#define		MPG_MBT_INTRA		0x01


static short int Sign(short int val)
{
	if(val < 0)
	{
		return -1;
	}
	else if(val)
	{
		return 1;
	}
	return 0;
}

static short int NINTDIV(int value,int div)
{
	if(value > 0)
	{
		value += div - 1;
	}
	else if(value < 0)
	{
		value -= div - 1;
	}
	return value / div;
}

/*libmpeg*/
#include <tbl.h>
#include <bs.h>
#include <img.h>

#include <vld.h>
#include <idct.h>
#include <mpgdec.h>

#include <me.h>
#include <vlc.h>
#include <fdct.h>
#include <mpgenc.h>

/*libdemux*/
#include <mpgdmx.h>
#include <m1vdmx.h>
#include <datdmx.h>

/*libmp3*/
#include <mp3dec.h>

/*libstd*/
#include <std.h>

/*libfile*/
#include <file.h>

#include <pcm.h>


#endif
