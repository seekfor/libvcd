#ifndef __FILE_H__
#define __FILE_H__

#ifdef __cplusplus
extern "C"
{
#endif

	void* WAVWriterCreate(BYTE* lpszOutputFileName,DWORD dwChannel,DWORD dwBitPerSample,DWORD dwSampleRate);
	BOOL WAVWriterProcess(void* pWav,BYTE* lpszBuffer,DWORD dwSize);
	BOOL WAVWriterDestroy(void* pWav);


#ifdef __cplusplus
}
#endif



#endif
