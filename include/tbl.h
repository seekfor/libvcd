/**********seekfor mpegdec v20.15.12.10*********************************************
	seekfor mpegdec v20.15.12.10是免费的MPEG解码软件,您可以自由使用和拷贝本程序!
	在使用和拷贝时请注意保留原来的版权信息.
	如果您有什么问题和意见,请和我联系!
	QQ:82054357
	QQ群:24613876
	MSN:sfrad32@hotmail.com
	Mail:Seek_for@163.com
***********************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

	vld_t m1vtbl_mba[33]; 
	vld_t m1vtbl_mbti[2];
	vld_t m1vtbl_mbtp[7]; 
	vld_t m1vtbl_mbtb[11];
	vld_t m1vtbl_mbtd[1];
	vld_t m1vtbl_cbp[63];
	vld_t m1vtbl_mv[33];
	vld_t m1vtbl_dct_ysize[9];
	vld_t m1vtbl_dct_csize[9];
	vld_t m1vtbl_dct[113];

	short int default_scan[8][8];
	const short int default_quantizer_matrix[64];
	const short int default_none_quantizer_matrix[64];

#ifdef __cplusplus
}
#endif
