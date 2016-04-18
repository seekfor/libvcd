/**********seekfor mpegdec v20.15.12.10*********************************************
	seekfor mpegdec v20.15.12.10是免费的MPEG解码软件,您可以自由使用和拷贝本程序!
	在使用和拷贝时请注意保留原来的版权信息.
	如果您有什么问题和意见,请和我联系!
	QQ:82054357
	QQ群:24613876
	MSN:sfrad32@hotmail.com
	Mail:Seek_for@163.com
***********************************************************************************/
#ifndef __STD_H__
#define __STD_H__

#ifdef __cplusplus
extern "C"
{
#endif

	void* std_create();
	
	int std_load(void* std,char* filename);

	int std_get_video_frame(void* std,int type,img_frame_t** img);

	int std_get_audio_frame(void* std,int type,short int* pcm,int* sr);

	int std_destroy(void* std);



#ifdef __cplusplus
}
#endif




#endif
