#ifndef __PCM_H__
#define __PCM_H__

typedef struct
{
	int type;
	double dts;
	double pts;
	int size;
	short int buf[0];
}pcm_frame_t;

#ifdef __cplusplus
extern "C"
{
#endif

	BOOL WAVOutDestroy(HANDLE hWav);
	BOOL WAVOutProcess(HANDLE hWav,BYTE* lpszBuf,DWORD dwSize);
	HANDLE WAVOutCreate(DWORD dwChannels,DWORD dwBitsPerSample,DWORD dwSampleRate);


	HANDLE WAVInCreate(DWORD dwChannels,DWORD dwBitsPerSample,DWORD dwSampleRate);
	DWORD WAVInProcess(HANDLE hWave,BYTE* lpszBuf,DWORD dwOutSize);
	BOOL WAVInDestroy(HANDLE hWave);

#ifdef __cplusplus
}
#endif



#endif
