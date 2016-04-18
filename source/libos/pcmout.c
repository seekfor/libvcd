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

#define WAV_SIZE	(44100)
static HWAVEOUT hWave = NULL;
static BYTE lpszWavBuf[2][WAV_SIZE];
static WAVEHDR wh[2];
static WAVEFORMATEX wf;
static BYTE lpszPCM[WAV_SIZE * 4];
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
		size = sizeof(lpszPCM) - dwRD + (bLoop?dwWR:0);
	}
	return size;
}


static DWORD FillPCM(BYTE* dst)
{
	int size = GetPCMSize(0);
	if(size > WAV_SIZE)
	{
		size = WAV_SIZE;
	}
	memcpy(dst,lpszPCM + dwRD,size);
	dwRD = (dwRD + size) % sizeof(lpszPCM);
	return size;
}


static void WINAPI waveOutCallback(HWAVEOUT hWav,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	WAVEHDR* pHdr;
	if(uMsg == WOM_DONE)
	{
			pHdr = (WAVEHDR*)dwParam1;
			waveOutUnprepareHeader(hWav,pHdr,sizeof(WAVEHDR));
			pHdr->dwBufferLength = FillPCM((BYTE*)pHdr->lpData);
			waveOutPrepareHeader(hWav,pHdr,sizeof(WAVEHDR));
			waveOutWrite(hWav,pHdr,sizeof(WAVEHDR));
	}
}

HANDLE WAVOutCreate(DWORD dwChannels,DWORD dwBitsPerSample,DWORD dwSampleRate)
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
		if(0 == waveOutOpen(&hWave,WAVE_MAPPER,&wf,(DWORD)waveOutCallback,0,CALLBACK_FUNCTION ))
		{
			for(i = 0; i < 2 ; i ++)
			{
				wh[i].dwBufferLength = 0;
				wh[i].dwBytesRecorded = 0;
				wh[i].dwFlags = 0;
				wh[i].dwLoops = 0;
				wh[i].dwUser = 0;
				wh[i].lpData = (LPSTR)&lpszWavBuf[i];
				wh[i].lpNext = NULL;
				wh[i].reserved = 0;
				waveOutPrepareHeader(hWave,&wh[i],sizeof(WAVEHDR));
			}
			waveOutWrite(hWave,wh + 0,sizeof(WAVEHDR));
			waveOutWrite(hWave,wh + 1,sizeof(WAVEHDR));
		}
	}
	return (HANDLE)hWave;
}

BOOL WAVOutProcess(HANDLE hWav,BYTE* lpszBuf,DWORD dwSize)
{
	int size = GetPCMSize(1);
	while(size > sizeof(lpszPCM) / 2)
	{
		Sleep(1);
		size = GetPCMSize(1);
	};
	if(dwWR + dwSize >= sizeof(lpszPCM))
	{
		memcpy(lpszPCM + dwWR,lpszBuf,sizeof(lpszPCM) - dwWR);
		memcpy(lpszPCM,lpszBuf + sizeof(lpszPCM) - dwWR,dwSize - (sizeof(lpszPCM) - dwWR));
		dwWR = dwSize - (sizeof(lpszPCM) - dwWR);
	}
	else
	{
		memcpy(lpszPCM + dwWR,lpszBuf,dwSize);
		dwWR += dwSize;
	}
	return 0;
}


BOOL WAVOutDestroy(HANDLE hWav)
{
	HWAVEOUT hWAV = (HWAVEOUT)hWav;
	waveOutUnprepareHeader(hWAV,wh + 0,sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWAV,wh + 1,sizeof(WAVEHDR));
	waveOutClose(hWAV);
	return 0;
}

