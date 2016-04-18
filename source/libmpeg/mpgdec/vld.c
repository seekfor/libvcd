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

vld_t* vld_get(vld_t* vld,int num,int vlc)
{
	int i;
	unsigned int val = vlc;
	for(i = 0; i < num; i ++,vld ++)
	{
		if((val >> (32 - vld->bit)) == (unsigned int)vld->value)
		{
			return vld;
		}
	}
	return (vld_t*)0;
}







