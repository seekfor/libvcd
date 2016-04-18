/**********seekfor mpegdec v20.15.12.10*********************************************
	seekfor mpegdec v20.15.12.10是免费的MPEG解码软件,您可以自由使用和拷贝本程序!
	在使用和拷贝时请注意保留原来的版权信息.
	如果您有什么问题和意见,请和我联系!
	QQ:82054357
	QQ群:24613876
	MSN:sfrad32@hotmail.com
	Mail:Seek_for@163.com
***********************************************************************************/
#ifndef __MP3DEC_H__
#define __MP3DEC_H__


#ifdef __cplusplus
extern "C"
{
#endif

	void* mp3dec_create();
	int mp3dec_process(void* dec,char* buf,int size,short int* pcm,int* pcm_size);
	void mp3dec_destroy(void* dec);

#ifdef __cplusplus
}
#endif



#endif
