/**********seekfor mpegdec v20.15.12.10*********************************************
	seekfor mpegdec v20.15.12.10是免费的MPEG解码软件,您可以自由使用和拷贝本程序!
	在使用和拷贝时请注意保留原来的版权信息.
	如果您有什么问题和意见,请和我联系!
	QQ:82054357
	QQ群:24613876
	MSN:sfrad32@hotmail.com
	Mail:Seek_for@163.com
***********************************************************************************/
#ifndef __IMAGE_H__
#define __IMAGE_H__

#define IMG_TYPE_YUV420			0
#define IMG_TYPE_YUV422			1
#define IMG_TYPE_RGB8888		2

typedef struct
{
	int type;
	double dts;
	double pts;
	int width;
	int height;
	int pad_width;
	int pad_height;
	unsigned char* y;
	unsigned char* u;
	unsigned char* v;


}img_frame_t;

#ifdef __cplusplus
extern "C"
{
#endif

	img_frame_t* img_create(int type,int width,int height,double dts,double pts);

	void img_destroy(img_frame_t* img);



#ifdef __cplusplus
}
#endif


#endif
