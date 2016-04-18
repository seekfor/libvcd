/**********seekfor mpegdec v20.15.12.10*********************************************
	seekfor mpegdec v20.15.12.10是免费的MPEG编解码软件,您可以自由使用和拷贝本程序!
	在使用和拷贝时请注意保留原来的版权信息.
	如果您有什么问题和意见,请和我联系!
	QQ:82054357
	QQ群:24613876
	MSN:sfrad32@hotmail.com
	Mail:Seek_for@163.com
***********************************************************************************/
#include <common.h>

typedef struct
{
	BYTE RIFF[4];/*"RIFF*/
	DWORD dwFileSize;/*FileSize - 8*/
	BYTE WAVEfmt[8];/*"WAVEfmt "*/
	DWORD dwFlagOf0x10;/*It must be 0x10*/
	WORD wFormatTag;/*PCM-0x01*/
	WORD wChannel;/*Number of channels*/
	DWORD dwSampleRate;
	DWORD dwBytesPerSecond;
	WORD wBlockAlign;/*wChannel * wBitPerSample >> 3*/
	WORD wBitPerSample;/*8/16/32*/
	BYTE data[4];/*"data"*/
	DWORD dwPCMSize;
}WAVH;


typedef struct
{
	WAVH wh;
	FILE* fp;
}WAVWRITER;

void* WAVWriterCreate(BYTE* lpszOutputFileName,DWORD dwChannel,DWORD dwBitPerSample,DWORD dwSampleRate)
{
	WAVWRITER* wf;
	FILE* fp = fopen(lpszOutputFileName,"w+b");
	if(!fp)
	{
		return NULL;
	}
	wf = (WAVWRITER*)malloc(sizeof(WAVWRITER));
	if(wf)
	{
		memcpy(wf->wh.RIFF,"RIFF",4);
		memcpy(wf->wh.WAVEfmt,"WAVEfmt ",8);
		memcpy(wf->wh.data,"data",4);
		wf->wh.wFormatTag = 0x01;
		wf->wh.dwFlagOf0x10 = 0x10;
		wf->wh.wChannel = (WORD)dwChannel;
		wf->wh.dwSampleRate = dwSampleRate;
		wf->wh.wBitPerSample = (WORD)dwBitPerSample;
		wf->wh.dwBytesPerSecond = dwSampleRate * dwChannel * (dwBitPerSample >> 3);
		wf->wh.wBlockAlign = (WORD)(dwChannel * dwBitPerSample >> 3);
		wf->wh.dwPCMSize = 0;
		fwrite(&wf->wh,1,sizeof(WAVH),fp);
		wf->fp = fp;
	}
	return wf;
}

BOOL WAVWriterProcess(void* pWav,BYTE* lpszBuffer,DWORD dwSize)
{
	WAVWRITER* wf = (WAVWRITER*)pWav;
	if(wf)
	{
		dwSize = fwrite(lpszBuffer,1,dwSize,wf->fp);
		wf->wh.dwPCMSize += dwSize;
		return 1;
	}
	return 0;
}


BOOL WAVWriterDestroy(void* pWav)
{
	WAVWRITER* wf = (WAVWRITER*)pWav;
	if(wf)
	{
		wf->wh.dwFileSize = sizeof(WAVH) + wf->wh.dwPCMSize - 8;
		fseek(wf->fp,0,SEEK_SET);
		fwrite(&wf->wh,1,sizeof(WAVH),wf->fp);
		fclose(wf->fp);
		free(wf);
		return 1;
	}
	return 0;
}

