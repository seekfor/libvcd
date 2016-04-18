#ifndef __MPGENC_H__
#define __MPGENC_H__


typedef struct
{
	int cs;
	int min_qp;
	int max_qp;
	int cbr;
	int width;
	int height;
	int fps;
	int gop;
	int max_bitrate;
	int avg_bitrate;
	

	unsigned int bitrate;
	int qp;
	int num_pic_encoded;
	int num_gop_encoded;
}mpgenc_info_t;


#ifdef __cplusplus
extern "C"
{
#endif

	void* mpgenc_create(int bufsize);
	int mpgenc_encode(void* enc,char* y,char* u,char* v);
	int mpgenc_getinfo(void* enc,mpgenc_info_t* info);
	int mpgenc_setinfo(void* enc,mpgenc_info_t* info);
	int mpgenc_getbitstream(void* enc,char** es,mpgenc_info_t* info);
	int mpgenc_destroy(void* enc);

#ifdef __cplusplus
}
#endif




#endif
