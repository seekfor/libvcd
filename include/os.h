/**********seekfor mpegdec v20.15.12.10*********************************************
	seekfor mpegdec v20.15.12.10是免费的MPEG解码软件,您可以自由使用和拷贝本程序!
	在使用和拷贝时请注意保留原来的版权信息.
	如果您有什么问题和意见,请和我联系!
	QQ:82054357
	QQ群:24613876
	MSN:sfrad32@hotmail.com
	Mail:Seek_for@163.com
***********************************************************************************/
#ifndef __OS_H__
#define __OS_H__

typedef void* (*thread_func)(void* args);

#ifdef __cplusplus
extern "C"
{
#endif

	void* osCreateSemaphore(char* name,int value);
	int osWaitSemaphore(void* sem);
	int osPostSemaphore(void* sem);
	void osDestroySemaphore(void* sem);

	void* osMalloc(int size);
	void* osRealloc(void* block,int size);
	void osFree(void* block);

	void* osCreateThread(int prior,int stack,void* args,thread_func func);
	int osJoinThread(void* thread);
	int osDestroyThread(void* thread,int wait);



#ifdef __cplusplus
}
#endif



#endif
