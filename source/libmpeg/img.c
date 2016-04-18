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

img_frame_t* img_create(int type,int width,int height,double dts,double pts)
{
	int size;
	img_frame_t* img;
	int pad_w = 0,pad_h = 0;
	if(width & 0x0f)
	{
		pad_w = 16 - (width & 0x0f);
	}
	if(height & 0x0f)
	{
		pad_h = 16 - (height & 0x0f);
	}

	pad_w += width;
	pad_h += height;

	switch(type)
	{
	case IMG_TYPE_YUV420:
		size = sizeof(img_frame_t) + pad_w * pad_h * 3 / 2 * sizeof(char);
		break;
	case IMG_TYPE_YUV422:
		size = sizeof(img_frame_t) + pad_w * pad_h * 2 * sizeof(char);
		break;
	case IMG_TYPE_RGB8888:
		size = sizeof(img_frame_t) + pad_w * pad_h * 4;
		break;
	default:
		return NULL;
	}
	img = (img_frame_t*)osMalloc(size);
	if(img == NULL)
	{
		return NULL;
	}
	img->type = type;
	img->dts = dts;
	img->pts = pts;
	img->width = width;
	img->height = height;
	img->pad_width = pad_w;
	img->pad_height = pad_h;
	if(type == IMG_TYPE_RGB8888)
	{
		img->y = (char*)(img + 1);
		img->u = (char*)NULL;
		img->v = (char*)NULL;
		memset(img->y,0x00,size);
	}
	else if(type == IMG_TYPE_YUV420)
	{
		img->y = (char*)(img + 1);
		img->u = img->y + pad_w * pad_h;
		img->v = img->u + (pad_w * pad_h / 4);
		memset(img->y,0x10,pad_w * pad_h);
		memset(img->u,0x80,pad_w * pad_h / 2);
	}
	else
	{
		img->y = (char*)(img + 1);
		img->u = img->y + pad_w * pad_h;
		img->v = img->u + (pad_w * pad_h / 2);
		memset(img->y,0x10,pad_w * pad_h);
		memset(img->u,0x80,pad_w * pad_h);
	}
	return img;
}

void img_destroy(img_frame_t* img)
{
	osFree(img);
}
