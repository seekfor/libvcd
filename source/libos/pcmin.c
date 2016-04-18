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

#define WAV_SIZE	(44100 * 4 / 10) /*100ms*/

#define WAVELEN		(WAV_SIZE * 4)

static HWAVEIN hWave = NULL;
static BYTE lpszWavBuf[2][WAV_SIZE];
static WAVEHDR wh[2];
static WAVEFORMATEX wf;
static BYTE lpszPCM[WAVELEN];
static volatile DWORD dwRD = 0,dwWR = 0;

static DWORD GetPCMSize(DWORD bLoop)
{
	int size;
	if(dwRD <= dwWR)
	{
		size = dwWR - dwRD;
	}
	else
	{
		size = WAVELEN - dwRD + (bLoop?dwWR:0);
	}
	return size;
}


static DWORD FillPCM(BYTE* lpszBuf,DWORD dwSize)
{
	if(dwWR + dwSize > WAVELEN)
	{
		memcpy(lpszPCM + dwWR,lpszBuf,WAVELEN - dwWR);
		memcpy(lpszPCM,lpszBuf + WAVELEN - dwWR,dwSize - (WAVELEN - dwWR));
		dwWR = dwSize - (WAVELEN - dwWR);
	}
	else
	{
		memcpy(lpszPCM + dwWR,lpszBuf,dwSize);
		dwWR += dwSize;
	}
	dwWR = dwWR % WAVELEN;
	return 0;
}


static void WINAPI waveInCallback(HWAVEIN hWav,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	WAVEHDR* pHdr;
	if(uMsg==MM_WIM_DATA)
	{
		pHdr = (WAVEHDR*)dwParam1;
		waveInUnprepareHeader(hWav,pHdr,sizeof(WAVEHDR));
		pHdr->dwBytesRecorded = FillPCM(pHdr->lpData,pHdr->dwBytesRecorded);
		waveInPrepareHeader(hWave,pHdr,sizeof(WAVEHDR));
		waveInAddBuffer(hWave,pHdr,sizeof(WAVEHDR));
	}
}



HANDLE WAVInCreate(DWORD dwChannels,DWORD dwBitsPerSample,DWORD dwSampleRate)
{
	int i;
	if(!hWave)
	{
		memset(&wh,0,sizeof(wh));
		wf.nAvgBytesPerSec = dwChannels * dwSampleRate * (dwBitsPerSample >> 3);
		wf.cbSize = sizeof(wf);
		wf.nChannels = (WORD)dwChannels;
		wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.nBlockAlign = (WORD)(dwBitsPerSample >> 3);
		wf.nSamplesPerSec = dwSampleRate;
		wf.wBitsPerSample = (WORD)dwBitsPerSample;
		if(0 == waveInOpen(&hWave,WAVE_MAPPER,&wf,(DWORD)waveInCallback,0,CALLBACK_FUNCTION ))
		{
			for(i = 0; i < 2 ; i ++)
			{
				wh[i].dwBufferLength = WAV_SIZE;
				wh[i].dwBytesRecorded = 0;
				wh[i].dwFlags = 0;
				wh[i].dwLoops = 0;
				wh[i].dwUser = 0;
				wh[i].lpData = (LPSTR)&lpszWavBuf[i];
				wh[i].lpNext = NULL;
				wh[i].reserved = 0;
				waveInPrepareHeader(hWave,&wh[i],sizeof(WAVEHDR));
				waveInAddBuffer(hWave,&wh[i],sizeof(WAVEHDR));
			}
			waveInStart(hWave);
		}
	}
	return hWave;
}

DWORD WAVInProcess(HANDLE hWave,BYTE* lpszBuf,DWORD dwOutSize)
{
	DWORD dwSize = GetPCMSize(1);
	if(dwSize == 0)
	{
		return 0;
	}
	if(dwSize > dwOutSize)
	{
		dwSize = dwOutSize;
	}
	if(dwSize + dwRD > WAVELEN)
	{
		memcpy(lpszBuf,lpszPCM + dwRD,WAVELEN - dwRD);
		memcpy(lpszBuf + WAVELEN - dwRD,lpszPCM + WAVELEN - dwRD,dwSize - (WAVELEN - dwRD));
		dwRD = dwSize - (WAVELEN - dwRD);
	}
	else
	{
		memcpy(lpszBuf,lpszPCM + dwRD,dwSize);
		dwRD += dwSize;
	}
	dwRD = dwRD % WAVELEN;
	return dwSize;
}

BOOL WAVInDestroy(HANDLE hWave)
{
	if(hWave)
	{
		waveInUnprepareHeader(hWave,wh + 0,sizeof(WAVEHDR));
		waveInUnprepareHeader(hWave,wh + 1,sizeof(WAVEHDR));
		waveInClose(hWave);
		return 1;
	}
	return 0;
}