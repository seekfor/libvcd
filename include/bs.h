/**********seekfor mpegdec v20.15.12.10*********************************************
	seekfor mpegdec v20.15.12.10是免费的MPEG解码软件,您可以自由使用和拷贝本程序!
	在使用和拷贝时请注意保留原来的版权信息.
	如果您有什么问题和意见,请和我联系!
	QQ:82054357
	QQ群:24613876
	MSN:sfrad32@hotmail.com
	Mail:Seek_for@163.com
***********************************************************************************/
#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

typedef struct
{
	int bufsize;/*the buffer size*/
	unsigned char* base;/*buffer base address*/
	unsigned char* src;/*current address*/
	int size;/*0-decoing mode,!0-encoding mode*/

	unsigned int value;
	int wr;
	int rd;
}bitstream_t;


#ifdef __cplusplus
extern "C"
{
#endif
	bitstream_t* bs_create(int size);
	int bs_reset(bitstream_t* bs);
	int bs_put(bitstream_t* bs,int bit,unsigned int value);
	int bs_reload(bitstream_t* bs,char* src,int size);
	void bs_skip(bitstream_t* bs,int bit);
	int bs_get(bitstream_t* bs,int bit,int skip);
	int bs_align(bitstream_t* bs);
	int bs_destroy(bitstream_t* bs);
	int bs_size(bitstream_t* bs);
	int bs_consumed(bitstream_t* bs);
	void bs_flush(bitstream_t* bs);

#ifdef __cplusplus
}
#endif



#endif
