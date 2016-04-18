/**********seekfor mpegdec v20.15.12.10*********************************************
	seekfor mpegdec v20.15.12.10是免费的MPEG解码软件,您可以自由使用和拷贝本程序!
	在使用和拷贝时请注意保留原来的版权信息.
	如果您有什么问题和意见,请和我联系!
	QQ:82054357
	QQ群:24613876
	MSN:sfrad32@hotmail.com
	Mail:Seek_for@163.com
***********************************************************************************/
#ifndef __MPGDMX_H__
#define __MPGDMX_H__


#ifdef __cplusplus
extern "C"
{
#endif

	void* mpgdmx_create(int max_es_size);
	int mpgdmx_probe(char* buf,int size);
	int mpgdmx_process(void* dmx,char* buf,int size);
	int mpgdmx_getframe(void* dmx,int vid,char* buf,double* dts,double* pts);
	void mpgdmx_destroy(void* dmx);


#ifdef __cplusplus
}
#endif



#endif





