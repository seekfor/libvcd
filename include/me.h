#ifndef __ME_H__
#define __ME_H__


#ifdef __cplusplus
extern "C"
{
#endif

	int mv_calc(unsigned char* forward,unsigned char* current,int x,int y,int w,int h,int* sad,int* mv_x,int* mv_y);
	int mv_calc_mr(int forward_f_code,int x,int prev_x,int* M,int* R);

#ifdef __cplusplus
}
#endif





#endif
