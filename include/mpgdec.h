/**********seekfor mpegdec v20.15.12.10*********************************************
	seekfor mpegdec v20.15.12.10是免费的MPEG解码软件,您可以自由使用和拷贝本程序!
	在使用和拷贝时请注意保留原来的版权信息.
	如果您有什么问题和意见,请和我联系!
	QQ:82054357
	QQ群:24613876
	MSN:sfrad32@hotmail.com
	Mail:Seek_for@163.com
***********************************************************************************/
#ifndef __MPGDEC_H__
#define __MPGDEC_H__

typedef struct
{
	int idx;
	int pct;
	int width;
	int height;
	double fps;
	double bps;
}mpegdec_info_t;


#ifdef __cplusplus
extern "C"
{
#endif

	void* mpgdec_create();

	int mpgdec_decode(void* dec,char* es,int size,double dts,double pts);

	img_frame_t* mpgdec_getframe(void* dec,int idx,mpegdec_info_t* info);

	int mpgdec_destroy(void* dec);


#ifdef __cplusplus
}
#endif



#endif
