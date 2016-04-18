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

typedef struct
{
	int width;
	int height;
	int par;
	double fps;
	int bitrate;
	int vbv_buffer_size;

	short int load_intra_quantizer_matrix[64];
	short int load_none_intra_quantizer_matrix[64];

}mpgdec_info_t;


typedef struct
{
	img_frame_t* frame;
/*picture level*/
	int tr;
	int pct;
	int vbv_delay;
	int full_pel_forward_vector;
	int forward_f_code;
	int full_pel_backward_vector;
	int backward_f_code;
	int quantizer_scale;
/*slice level*/
	int slice_vertical_position;
/*macroblock level*/
	int mb_x;
	int mb_y;
	int mb_w;
	int mb_h;
	int macroblock_block_type;
	int macroblock_address;
	int motion_horizontal_forward_code;
	int motion_horizontal_forward_r;
	int motion_vertical_forward_code;
	int motion_vertical_forward_r;
	int motion_horizontal_backward_code;
	int motion_horizontal_backward_r;
	int motion_vertical_backward_code;
	int motion_vertical_backward_r;
/*block level*/
	short int dct_dc_y_past;
	short int dct_dc_cb_past;
	short int dct_dc_cr_past;

	short int dct_dc_size_y;
	short int dct_dc_size_c;
/*blocks*/	
	short int dct_block[6][8][8];
/*motion vectors*/
	int recon_right_for;
	int recon_down_for;
	int recon_right_for_prev;
	int recon_down_for_prev;

	int recon_right_back;
	int recon_down_back;
	int recon_right_back_prev;
	int recon_down_back_prev;

	int right_for[2];
	int down_for[2];
	int right_half_for[2];
	int down_half_for[2];

	int right_back[2];
	int down_back[2];
	int right_half_back[2];
	int down_half_back[2];


	int alone;
	int error;
}mpgdec_picture_t;



typedef struct
{
	int idx;

	int hour;
	int minute;
	int second;

	int closed_gop;
	int broken_link;

	mpgdec_picture_t* output[1024];
}mpgdec_gop_t;



typedef struct
{
	bitstream_t bs;
	mpgdec_info_t info;
	mpgdec_gop_t gop;	
	
	mpgdec_picture_t* i;
	mpgdec_picture_t* p;
	mpgdec_picture_t* cur;

}mpgdec_t;

static void mpgdec_reset_dct_params(mpgdec_picture_t* pic)
{
	pic->dct_dc_y_past = pic->dct_dc_cb_past = pic->dct_dc_cr_past = 1024;
}

static void mpgdec_reset_mv_params(mpgdec_picture_t* pic,int mbt,int pred)
{
	if(mbt & MPG_MBT_MVF)
	{
		pic->recon_right_for = 0;
		pic->recon_down_for = 0;
		pic->right_for[0] = pic->right_for[1] = 0;
		pic->right_half_for[0] = pic->right_half_for[1] = 0;
		pic->down_for[0] = pic->down_for[1] = 0;
		pic->down_half_for[0] = pic->down_half_for[1] = 0;
		if(pred)
		{
			pic->recon_right_for_prev = 0;
			pic->recon_down_for_prev = 0;
		}
	}
	if(mbt & MPG_MBT_MVB)
	{
		pic->recon_right_back = 0;
		pic->recon_down_back = 0;
		pic->right_back[0] = pic->right_back[1] = 0;
		pic->right_half_back[0] = pic->right_half_back[1] = 0;
		pic->down_back[0] = pic->down_back[1] = 0;
		pic->down_half_back[0] = pic->down_half_back[1] = 0;
		if(pred)
		{
			pic->recon_right_back_prev = 0;
			pic->recon_down_back_prev = 0;
		}
	}
}


static mpgdec_picture_t* mpgdec_alloc_picture(mpgdec_t* dec,int tr,int pct,double dts,double pts)
{
	img_frame_t* img;
	mpgdec_picture_t* pic = (mpgdec_picture_t*)osMalloc(sizeof(mpgdec_picture_t));
	if(!pic)
	{
		return NULL;
	}
	img = img_create(IMG_TYPE_YUV420,dec->info.width,dec->info.height,dts,pts);
	if(!img)
	{
		osFree(pic);
		return NULL;
	}
	memset(pic,0,sizeof(mpgdec_picture_t));
	pic->tr = tr;
	pic->pct = pct;
	pic->frame = img;
	pic->mb_w = (dec->info.width + 15) / 16;
	pic->mb_h = (dec->info.height + 15) / 16;
	dec->cur = pic;
	dec->gop.output[tr] = pic;

	return pic;
}


static char* mpgdec_pel_locate(mpgdec_picture_t* pic,int blk,int y,int x)
{
	img_frame_t* frm = pic->frame;
	char* src;
	if(blk < 4)
	{
		src = frm->y + frm->pad_width * y + x;
	}
	else if(blk == 4)
	{
		src = frm->u + (frm->pad_width / 2) * y + x;
	}
	else
	{
		src = frm->v + (frm->pad_width / 2) * y + x;
	}
	return src;
}

static void mpgdec_mb_process(mpgdec_t* dec,mpgdec_picture_t* pic)
{
	img_frame_t* frm = pic->frame;
	short int* src;
	unsigned char* dst;
	unsigned char* mb;
	int i,j,k;
	short int val;
	int x,y;
	x = pic->mb_x * 16;
	y = pic->mb_y * 16;

	mb = frm->y + frm->pad_width * y + x;
	for(k = 0; k < 4; k ++)
	{
		dst = mb + (k >> 1) * 8 * frm->pad_width + (k & 0x01) * 8;
		src = (short int*)&pic->dct_block[k][0][0];
		for(i = 0; i < 8; i ++,dst += frm->pad_width)
		{
			for(j = 0; j < 8; j ++,src ++)
			{
				val = *src;
				if(val < 0)
				{
					val = 0;
				}
				else if(val > 255)
				{
					val = 255;
				}
				*(dst + j) = val & 0xff;
			}
		}
	}

	dst = frm->u + frm->pad_width / 2 * pic->mb_y * 8 + pic->mb_x * 8;
	src = (short int*)&pic->dct_block[4][0][0];
	for(i = 0; i < 8; i ++)
	{
		for(j = 0; j < 8; j ++,src ++)
		{
			val = *src;
			if(val < 0)
			{
				val = 0;
			}
			else if(val > 255)
			{
				val = 255;
			}
		  *(dst + j) = val & 0xff;
		}
		dst += frm->pad_width / 2;
	}

	dst = frm->v + frm->pad_width / 2* pic->mb_y * 8 + pic->mb_x * 8;
	src = (short int*)&pic->dct_block[5][0][0];
	for(i = 0; i < 8; i ++)
	{
		for(j = 0; j < 8; j ++,src ++)
		{
			val = *src;
			if(val < 0)
			{
				val = 0;
			}
			else if(val > 255)
			{
				val = 255;
			}
			*(dst + j) = val & 0xff;
		}
		dst += frm->pad_width / 2;
	}

}


static void mpgdec_copy_pel(mpgdec_t* dec,int mb_x,int mb_y,mpgdec_picture_t* ref,int blk,int right,int down,int right_half,int down_half,short int* dst)
{
	int i,j;
	unsigned char* A,*B,*C,*D;
	int x,y;
	int w;
	int h;
	unsigned short int A1,B1,C1,D1;
	int pad_w,pad_h;
	if(blk < 4)
	{
		w = 16;
		h = 16;
		pad_w = ref->frame->pad_width;
		pad_h = ref->frame->pad_height;
	}
	else
	{
		w = 8;
		h = 8;
		pad_w = ref->frame->pad_width / 2;
		pad_h = ref->frame->pad_height / 2;
	}

	x = mb_x * w + right;
	y = mb_y * h + down;

	if(x < 0)
	{
		x = 0;
	}

	if(y < 0)
	{
		y = 0;
	}
	
	if((x + w) > pad_w)
	{
		x = pad_w - w;
	}

	if((y + h) > pad_h)
	{
		y = pad_h - h;
	}

	A = mpgdec_pel_locate(ref,blk,y,x);
	B = mpgdec_pel_locate(ref,blk,y + 1,x);
	C = mpgdec_pel_locate(ref,blk,y,x + 1);
	D = mpgdec_pel_locate(ref,blk,y + 1,x + 1);


	for(i = 0; i < h;i++)
	{
		for(j = 0; j < w; j ++,dst ++)
		{
			if(!right_half && !down_half)
			{
				*dst = *(A + j); 
			}
			else if(!right_half && down_half)
			{
				A1 = *(A + j);
				B1 = *(B + j);
				*dst = (A1 + B1 + 1) >> 1;
			}
			else if(right_half && !down_half)
			{
				A1 = *(A + j);
				C1 = *(C + j);
				*dst = (A1 + C1 + 1) >> 1;
			}
			else
			{
				A1 = *(A + j);
				B1 = *(B + j);
				C1 = *(C + j);
				D1 = *(D + j);
				*dst = (A1 + B1 + C1 + D1 + 2) >> 2;
			}
		}
		A += pad_w;
		B += pad_w;
		C += pad_w;
		D += pad_w;
	}


}

static void mpgdec_mc_process(mpgdec_t* dec,mpgdec_picture_t* pic,int mbt)
{
	short int c_past[8][8];
	short int c_back[8][8];
	short int past[16][16];
	short int back[16][16];
	int i,j,k;
	int right_half,down_half;
	int right,down;
	mpgdec_picture_t* ref;
	short int* src;
	short int* mb;
	short int* A,*B;
	
	int type = mbt & (MPG_MBT_MVF | MPG_MBT_MVB);

	if(type == 0)
	{
		if(pic->pct == MPG_TYPE_P)
		{
			type = MPG_MBT_MVF;
		}
		else
		{
			type = MPG_MBT_MVF | MPG_MBT_MVB;
		}
	}

	if(type == (MPG_MBT_MVF | MPG_MBT_MVB) )
	{
		if(dec->i)
		{
			right_half = pic->right_half_for[0];
			down_half = pic->down_half_for[0];
			right = pic->right_for[0];
			down = pic->down_for[0];
			mpgdec_copy_pel(dec,pic->mb_x,pic->mb_y,dec->i,0,right,down,right_half,down_half,(short int*)past);
		}
		else
		{
			pic->error ++;
			memset(past,0x00,sizeof(past));
		}
		if(dec->p)
		{
			right_half = pic->right_half_back[0];
			down_half = pic->down_half_back[0];
			right = pic->right_back[0];
			down = pic->down_back[0];
			mpgdec_copy_pel(dec,pic->mb_x,pic->mb_y,dec->p,0,right,down,right_half,down_half,(short int*)back);
		}
		else
		{
			pic->error ++;
			memcpy(back,past,sizeof(back));
		}

		for(k = 0; k < 4; k ++)
		{
			A = (short int*)past + (k >> 1) * 8 * 16 + (k & 0x01 ) * 8;
			B = (short int*)back + (k >> 1) * 8 * 16 + (k & 0x01 ) * 8;
			for(i = 0; i < 8; i ++)
			{
				for(j = 0; j < 8; j ++)
				{
					pic->dct_block[k][i][j] += ((*(A + j)) + (*(B + j)) + 1) >> 1;
				}
				A += 16;
				B += 16;
			}
		}

		for(k = 4;k < 6; k ++)
		{
			if(dec->i)
			{
				right_half = pic->right_half_for[1];
				down_half = pic->down_half_for[1];
				right = pic->right_for[1];
				down = pic->down_for[1];
				mpgdec_copy_pel(dec,pic->mb_x,pic->mb_y,dec->i,k,right,down,right_half,down_half,(short int*)c_past);
			}
			else
			{
				pic->error ++;
				memset(c_past,0x00,sizeof(c_past));
			}
			if(dec->p)
			{
				right_half = pic->right_half_back[1];
				down_half = pic->down_half_back[1];
				right = pic->right_back[1];
				down = pic->down_back[1];
				mpgdec_copy_pel(dec,pic->mb_x,pic->mb_y,dec->p,k,right,down,right_half,down_half,(short int*)c_back);
			}
			else
			{
				pic->error ++;
				memcpy(c_back,c_past,sizeof(c_back));
			}
			A = (short int*)c_past;
			B = (short int*)c_back;
			for(i = 0; i < 8; i ++)
			{
				for(j = 0; j < 8; j ++)
				{
					 pic->dct_block[k][i][j] += ((*(A + j)) + (*(B + j)) + 1) >> 1;
				}
				A += 8;
				B += 8;
			}
		}

	}
	else if((type == MPG_MBT_MVF) || (type == MPG_MBT_MVB))
	{
		if(pic->pct == MPG_TYPE_P)
		{
			ref = dec->p?dec->p:dec->i;
		}
		else
		{
			ref = (type == MPG_MBT_MVF)?dec->i:dec->p;
		}
		if(ref)
		{
			right_half = (type == MPG_MBT_MVF)?pic->right_half_for[0]:pic->right_half_back[0];
			down_half = (type == MPG_MBT_MVF)?pic->down_half_for[0]:pic->down_half_back[0];
			right = (type == MPG_MBT_MVF)?pic->right_for[0]:pic->right_back[0];
			down = (type == MPG_MBT_MVF)?pic->down_for[0]:pic->down_back[0];
			mpgdec_copy_pel(dec,pic->mb_x,pic->mb_y,ref,0,right,down,right_half,down_half,(short int*)past);
		}
		else
		{
			pic->error ++;
			memset(past,0x00,sizeof(past));
		}
		mb = (short int*)past;
		for(k = 0; k < 4; k ++)
		{
			src = mb + (k >> 1) * 8 * 16 + (k & 0x01 ) * 8;
			for(i = 0; i < 8; i ++)
			{
				for(j = 0; j < 8; j ++)
				{
					pic->dct_block[k][i][j] += *(src + j);
				}
				src += 16;
			}
		}

		right_half = (type == MPG_MBT_MVF)?pic->right_half_for[1]:pic->right_half_back[1];
		down_half = (type == MPG_MBT_MVF)?pic->down_half_for[1]:pic->down_half_back[1];
		right = (type == MPG_MBT_MVF)?pic->right_for[1]:pic->right_back[1];
		down = (type == MPG_MBT_MVF)?pic->down_for[1]:pic->down_back[1];

		for(k = 4;k < 6; k ++)
		{
			src = (short int*)c_past;
			if(ref)
			{
				mpgdec_copy_pel(dec,pic->mb_x,pic->mb_y,ref,k,right,down,right_half,down_half,(short int*)c_past);
			}
			else
			{
				pic->error ++;
				memset(c_past,0x00,sizeof(c_past));
			}
			for(i = 0; i < 8; i ++)
			{
				for(j = 0; j < 8; j ++)
				{
					 pic->dct_block[k][i][j] += *(src + j);
				}
				src += 8;
			}
		}
	}
}


static void mpgdec_calc_motion_vectors(mpgdec_t* dec,mpgdec_picture_t* pic,int mbt)
{
	int min,max;
	int new_vector;

	if(mbt & MPG_MBT_MVF)
	{
		int complement_horizontal_forward_r;
		int complement_vertical_forward_r;
		int forward_f = 1 << (pic->forward_f_code - 1);
		int right_little,right_big;
		int down_little,down_big;
		if((forward_f == 1) || (pic->motion_horizontal_forward_code == 0))
		{
			complement_horizontal_forward_r = 0;
		}
		else
		{
			complement_horizontal_forward_r = forward_f - 1 - pic->motion_horizontal_forward_r;
		}
		if((forward_f == 1) || (pic->motion_vertical_forward_code == 0) )
		{
			complement_vertical_forward_r = 0;
		}
		else
		{
			complement_vertical_forward_r = forward_f - 1 - pic->motion_vertical_forward_r;
		}
		
		right_little = pic->motion_horizontal_forward_code * forward_f;
		if(right_little == 0)
		{
			right_big = 0;
		}
		else
		{
			if(right_little > 0)
			{
				right_little = right_little - complement_horizontal_forward_r;
				right_big = right_little - (32 * forward_f);
			}
			else
			{
				right_little = right_little + complement_horizontal_forward_r;
				right_big = right_little + (32 * forward_f);
			}
		}

		down_little = pic->motion_vertical_forward_code * forward_f;
		if(down_little == 0)
		{
			down_big = 0;
		}
		else
		{
			if(down_little > 0)
			{
				down_little = down_little - complement_vertical_forward_r;
				down_big = down_little - (32 * forward_f);
			}
			else
			{
				down_little = down_little + complement_vertical_forward_r;
				down_big = down_little + (32 * forward_f);
			}
		}

		max = (16 * forward_f) - 1;
		min = -16 * forward_f;

		new_vector = pic->recon_right_for_prev + right_little;
		if((new_vector <= max) && (new_vector >= min) )
		{
			pic->recon_right_for = new_vector;
		}
		else
		{
			pic->recon_right_for = pic->recon_right_for_prev + right_big;
		}
		pic->recon_right_for_prev = pic->recon_right_for;
		if(pic->full_pel_forward_vector)
		{
			pic->recon_right_for <<= 1;
		}

		new_vector = pic->recon_down_for_prev + down_little;
		if((new_vector <= max) && (new_vector >= min) )
		{
			pic->recon_down_for = new_vector;
		}
		else
		{
			pic->recon_down_for = pic->recon_down_for_prev + down_big;
		}
		pic->recon_down_for_prev = pic->recon_down_for;
		if(pic->full_pel_forward_vector)
		{
			pic->recon_down_for <<= 1;
		}
		
		pic->right_for[0] = pic->recon_right_for >> 1;
		pic->down_for[0] = pic->recon_down_for >> 1;
		pic->right_half_for[0] = pic->recon_right_for - (2 * pic->right_for[0]);
		pic->down_half_for[0] = pic->recon_down_for - (2 * pic->down_for[0]);

		pic->right_for[1] = (pic->recon_right_for / 2) >> 1;
		pic->down_for[1] = (pic->recon_down_for / 2) >> 1;
		pic->right_half_for[1] = pic->recon_right_for / 2 - (2 * pic->right_for[1]);
		pic->down_half_for[1] = pic->recon_down_for / 2 - (2 * pic->down_for[1]);

	}

	if(mbt & MPG_MBT_MVB)
	{
		int complement_horizontal_backward_r;
		int complement_vertical_backward_r;
		int backward_f = 1 << (pic->backward_f_code - 1);
		int right_little,right_big;
		int down_little,down_big;
		if((backward_f == 1) || (pic->motion_horizontal_backward_code == 0))
		{
			complement_horizontal_backward_r = 0;
		}
		else
		{
			complement_horizontal_backward_r = backward_f - 1 - pic->motion_horizontal_backward_r;
		}
		if((backward_f == 1) || (pic->motion_vertical_backward_code == 0) )
		{
			complement_vertical_backward_r = 0;
		}
		else
		{
			complement_vertical_backward_r = backward_f - 1 - pic->motion_vertical_backward_r;
		}
		
		right_little = pic->motion_horizontal_backward_code * backward_f;
		if(right_little == 0)
		{
			right_big = 0;
		}
		else
		{
			if(right_little > 0)
			{
				right_little = right_little - complement_horizontal_backward_r;
				right_big = right_little - (32 * backward_f);
			}
			else
			{
				right_little = right_little + complement_horizontal_backward_r;
				right_big = right_little + (32 * backward_f);
			}
		}

		down_little = pic->motion_vertical_backward_code * backward_f;
		if(down_little == 0)
		{
			down_big = 0;
		}
		else
		{
			if(down_little > 0)
			{
				down_little = down_little - complement_vertical_backward_r;
				down_big = down_little - (32 * backward_f);
			}
			else
			{
				down_little = down_little + complement_vertical_backward_r;
				down_big = down_little + (32 * backward_f);
			}
		}

		max = (16 * backward_f) - 1;
		min = -16 * backward_f;

		new_vector = pic->recon_right_back_prev + right_little;
		if((new_vector <= max) && (new_vector >= min) )
		{
			pic->recon_right_back = new_vector;
		}
		else
		{
			pic->recon_right_back = pic->recon_right_back_prev + right_big;
		}
		pic->recon_right_back_prev = pic->recon_right_back;
		if(pic->full_pel_backward_vector)
		{
			pic->recon_right_back <<= 1;
		}

		new_vector = pic->recon_down_back_prev + down_little;
		if((new_vector <= max) && (new_vector >= min) )
		{
			pic->recon_down_back = new_vector;
		}
		else
		{
			pic->recon_down_back = pic->recon_down_back_prev + down_big;
		}
		pic->recon_down_back_prev = pic->recon_down_back;
		if(pic->full_pel_forward_vector)
		{
			pic->recon_down_back <<= 1;
		}
		
		pic->right_back[0] = pic->recon_right_back >> 1;
		pic->down_back[0] = pic->recon_down_back >> 1;
		pic->right_half_back[0] = pic->recon_right_back - (2 * pic->right_back[0]);
		pic->down_half_back[0] = pic->recon_down_back - (2 * pic->down_back[0]);

		pic->right_back[1] = (pic->recon_right_back / 2) >> 1;
		pic->down_back[1] = (pic->recon_down_back / 2) >> 1;
		pic->right_half_back[1] = pic->recon_right_back / 2 - (2 * pic->right_back[1]);
		pic->down_half_back[1] = pic->recon_down_back / 2 - (2 * pic->down_back[1]);

	}
}

static int mpgdec_parse_block(mpgdec_t* dec,int blk,int coded,int mbt)
{
	bitstream_t* bs = &dec->bs;
	vld_t* vld;
	short int run,level;
	int n = 0;
	int m,i;
	int intra = mbt & MPG_MBT_INTRA;
	short int* matrix;
	short int dct_recon[8][8];
	short int dct_zz[64];
	int dct_dc_differential = 0;
	mpgdec_picture_t* pic = (mpgdec_picture_t*)dec->cur;
	if(coded)
	{
		memset(dct_zz,0x00,64 * sizeof(short int));
		if(intra)
		{
			if(blk < 4)
			{
				vld = vld_get(m1vtbl_dct_ysize,sizeof(m1vtbl_dct_ysize) / sizeof(vld_t),bs->value);
				if(!vld)
				{
					pic->error ++;
					return -1;
				}
				bs_skip(bs,vld->bit);
				pic->dct_dc_size_y = vld->vlc;
				if(pic->dct_dc_size_y)
				{
					dct_dc_differential = bs_get(bs,pic->dct_dc_size_y,1);
					
					if(dct_dc_differential & (1 << (pic->dct_dc_size_y - 1)) )
					{
						dct_zz[0] = dct_dc_differential;
					}
					else
					{
						dct_zz[0] = (-1 << pic->dct_dc_size_y) | (dct_dc_differential + 1);
					}

				}
			}
			else
			{
				vld = vld_get(m1vtbl_dct_csize,sizeof(m1vtbl_dct_csize) / sizeof(vld_t),bs->value);
				if(!vld)
				{
					pic->error ++;
					return -1;
				}
				bs_skip(bs,vld->bit);
				pic->dct_dc_size_c = vld->vlc;
				if(pic->dct_dc_size_c)
				{
					dct_dc_differential = bs_get(bs,pic->dct_dc_size_c,1);
					if(dct_dc_differential & (1 << (pic->dct_dc_size_c - 1)) )
					{
						dct_zz[0] = dct_dc_differential;
					}
					else
					{
						dct_zz[0] = (-1 << pic->dct_dc_size_c) | (dct_dc_differential + 1);
					}

				}
			}
			

		}
		else
		{
			vld = vld_get(m1vtbl_dct,sizeof(m1vtbl_dct) / sizeof(vld_t),bs->value);
			if(!vld)
			{
				pic->error ++;
				return -1;
			}
			bs_skip(bs,vld->bit);
			if(vld->vlc == 0)
			{
				run = bs_get(bs,6,1);
				level = bs_get(bs,8,1);
				if(level == 0x00)
				{
					level = bs_get(bs,8,1);
				}
				else if(level == 0x80)
				{
					level = bs_get(bs,8,1) - 256;
				}
				else
				{
					level = (char)level;
				}
			}
			else
			{
				run = vld->vlc >> 16;
				level = vld->vlc & 0xffff;
				if(bs_get(bs,1,1))
				{
					level = -level;
				}
			}
			n = run;
			if(n < 64)
			{
				dct_zz[n] = level;
			}
			else
			{
				pic->error ++;
			}
		}
		if(pic->pct != MPG_TYPE_D)
		{
			while(bs_get(bs,2,0) != 0x02)
			{
				vld = vld_get(m1vtbl_dct + 1,sizeof(m1vtbl_dct) / sizeof(vld_t) - 1,bs->value);
				if(!vld)
				{
					pic->error ++;
					return -1;
				}
				bs_skip(bs,vld->bit);
				if(vld->vlc == 0)
				{
					run = bs_get(bs,6,1);
					level = bs_get(bs,8,1);
					if(level == 0x00)
					{
						level = bs_get(bs,8,1);
					}
					else if(level == 0x80)
					{
						level = bs_get(bs,8,1) - 256;
					}
					else
					{
						level = (char)level;
					}
				}
				else
				{
					run = vld->vlc >> 16;
					level = vld->vlc & 0xffff;
					if(bs_get(bs,1,1))
					{
						level = -level;
					}
				}
				n = n + run + 1;
				if(n < 64)
				{
					dct_zz[n] = level;
				}
				else
				{
					pic->error ++;
				}
			}
			bs_skip(bs,2);
		}
		matrix = intra?dec->info.load_intra_quantizer_matrix:dec->info.load_none_intra_quantizer_matrix;
		for(m = 0; m < 8 ; m ++)
		{
			for(n = 0; n < 8; n ++)
			{
				short int val;
				i = default_scan[m][n];
				if(dct_zz[i])
				{
					val = ((2 * dct_zz[i] + (intra?0:Sign(dct_zz[i]))) * pic->quantizer_scale * matrix[i]) / 16;
					if(!(val & 0x01))
					{
						val = val - Sign(val);
					}
					if(val > 2047)
					{
						val = 2047;
					}
					else if(val < -2048)
					{
						val = -2048;
					}
				}
				else
				{
					val = 0;
				}
				dct_recon[m][n] = val;
			}
		}
		if(intra)
		{
			switch(blk)
			{
				case 0:
				case 1:
				case 2:
				case 3:
					pic->dct_dc_y_past = dct_recon[0][0] = pic->dct_dc_y_past + (dct_zz[0] * 8);
					break;
				case 4:
					pic->dct_dc_cb_past = dct_recon[0][0] = pic->dct_dc_cb_past + (dct_zz[0] * 8);
					break;
				case 5:
					pic->dct_dc_cr_past = dct_recon[0][0] = pic->dct_dc_cr_past + (dct_zz[0] * 8);
					break;
			}
		}
		idct((short int*)dct_recon,(short int*)&pic->dct_block[blk][0][0]);
	}
	else
	{
		memset(pic->dct_block[blk],0x00,sizeof(short int) * 64);
	}

	return 0;
}

static int mpgdec_parse_macroblock(mpgdec_t* dec)
{
	bitstream_t* bs = &dec->bs;
	vld_t* vld;
	int i;
	int addr;
	mpgdec_picture_t* pic = (mpgdec_picture_t*)dec->cur;
	int mba = 0;
	int mbt;
	int intra;
	int cbp = 0x00;

	while(bs_get(bs,11,0) == 0x0f)
	{
		bs_skip(bs,11);
	};

	while(bs_get(bs,11,0) == 0x08)
	{
		bs_skip(bs,11);
		mba += 33;
	}

	vld = vld_get(m1vtbl_mba,sizeof(m1vtbl_mba) / sizeof(vld_t),bs->value);
	if(!vld)
	{
		pic->error ++;
		return -1;
	}
	bs_skip(bs,vld->bit);
	mba += vld->vlc;

	if(mba > 1)
	{
		mbt = pic->macroblock_block_type;
		mpgdec_reset_dct_params(pic);
		if(pic->pct == MPG_TYPE_P)
		{
			mpgdec_reset_mv_params(pic,MPG_MBT_MVF,1);
		}
		while(mba > 1)
		{
			memset(pic->dct_block,0x00,sizeof(pic->dct_block));
			pic->macroblock_address ++;
			addr = pic->slice_vertical_position * pic->mb_w + pic->macroblock_address;
			pic->mb_x = addr % pic->mb_w;
			pic->mb_y = addr / pic->mb_w;
			if(pic->mb_y < pic->mb_h)
			{
				mpgdec_mc_process(dec,pic,mbt);
				mpgdec_mb_process(dec,pic);
			}
			else
			{
				pic->error ++;
			}
			mba --;
		};
	}

	pic->macroblock_address += mba;
	addr = pic->slice_vertical_position * pic->mb_w + pic->macroblock_address;
	pic->mb_x = addr % pic->mb_w;
	pic->mb_y = addr / pic->mb_w;

	if(pic->mb_y >= pic->mb_h)
	{
		pic->error ++;
		return 0;
	}

	switch(pic->pct)
	{
		case MPG_TYPE_I:
			vld = vld_get(m1vtbl_mbti,sizeof(m1vtbl_mbti) / sizeof(vld_t),bs->value);
			if(!vld)
			{
				pic->error ++;
				return -1;
			}
			mbt = vld->vlc;
			bs_skip(bs,vld->bit);
			break;
		case MPG_TYPE_P:
			vld = vld_get(m1vtbl_mbtp,sizeof(m1vtbl_mbtp) / sizeof(vld_t),bs->value);
			if(!vld)
			{
				pic->error ++;
				return -1;
			}
			mbt = vld->vlc;
			bs_skip(bs,vld->bit);
			break;
		case MPG_TYPE_B:
			vld = vld_get(m1vtbl_mbtb,sizeof(m1vtbl_mbtb) / sizeof(vld_t),bs->value);
			if(!vld)
			{
				pic->error ++;
				return -1;
			}
			mbt = vld->vlc;
			bs_skip(bs,vld->bit);
			break;
		case MPG_TYPE_D:
			vld = vld_get(m1vtbl_mbtd,sizeof(m1vtbl_mbtd) / sizeof(vld_t),bs->value);
			if(!vld)
			{
				pic->error ++;
				return -1;
			}
			mbt = vld->vlc;
			bs_skip(bs,vld->bit);
			break;
		default:
				pic->error ++;
				return -1;
	}

	if(mbt & MPG_MBT_QUANT)
	{
		pic->quantizer_scale = bs_get(bs,5,1);
	}
	if(mbt & MPG_MBT_MVF)
	{
		vld = vld_get(m1vtbl_mv,sizeof(m1vtbl_mv) / sizeof(vld_t),bs->value);
		if(!vld)
		{
			pic->error ++;
			return -1;
		}
		bs_skip(bs,vld->bit);
		pic->motion_horizontal_forward_code = vld->vlc;
		if((pic->forward_f_code != 1) && vld->vlc)
		{
			pic->motion_horizontal_forward_r = bs_get(bs,pic->forward_f_code - 1,1);
		}
		else
		{
			pic->motion_horizontal_forward_r = 0;
		}

		vld = vld_get(m1vtbl_mv,sizeof(m1vtbl_mv) / sizeof(vld_t),bs->value);
		if(!vld)
		{
			pic->error ++;
			return -1;
		}
		bs_skip(bs,vld->bit);
		pic->motion_vertical_forward_code = vld->vlc;
		if((pic->forward_f_code != 1) && vld->vlc)
		{
			pic->motion_vertical_forward_r = bs_get(bs,pic->forward_f_code - 1,1);
		}
		else
		{
			pic->motion_vertical_forward_r = 0;
		}
		mpgdec_calc_motion_vectors(dec,pic,MPG_MBT_MVF);
	}
	else
	{
		mpgdec_reset_mv_params(pic,MPG_MBT_MVF,pic->pct == MPG_TYPE_P);
	}
	if(mbt & MPG_MBT_MVB)
	{
		vld = vld_get(m1vtbl_mv,sizeof(m1vtbl_mv) / sizeof(vld_t),bs->value);
		if(!vld)
		{
			pic->error ++;
			return -1;
		}
		bs_skip(bs,vld->bit);
		pic->motion_horizontal_backward_code = vld->vlc;
		if((pic->backward_f_code != 1) && vld->vlc)
		{
			pic->motion_horizontal_backward_r = bs_get(bs,pic->backward_f_code - 1,1);
		}

		vld = vld_get(m1vtbl_mv,sizeof(m1vtbl_mv) / sizeof(vld_t),bs->value);
		if(!vld)
		{
			pic->error ++;
			return -1;
		}
		bs_skip(bs,vld->bit);
		pic->motion_vertical_backward_code = vld->vlc;
		if((pic->backward_f_code != 1) && vld->vlc)
		{
			pic->motion_vertical_backward_r = bs_get(bs,pic->backward_f_code - 1,1);
		}
		else
		{
			pic->motion_vertical_backward_r = 0;
		}
		mpgdec_calc_motion_vectors(dec,pic,MPG_MBT_MVB);
	}
	else
	{
		mpgdec_reset_mv_params(pic,MPG_MBT_MVB,0);
	}

	intra = mbt & MPG_MBT_INTRA;


	if(mbt & MPG_MBT_CBP)
	{
		vld = vld_get(m1vtbl_cbp,sizeof(m1vtbl_cbp) / sizeof(vld_t),bs->value);
		if(!vld)
		{
			pic->error ++;
			return -1;
		}
		bs_skip(bs,vld->bit);
		cbp = vld->vlc;
	}


	pic->macroblock_block_type = mbt;

	for(i = 0; i < 6; i ++)
	{
		int coded;
		if(intra || (cbp & (1 << (5 - i))))
		{
			coded = 1;
		}
		else
		{
			coded = 0;
		}
		if(0 != mpgdec_parse_block(dec,i,coded,mbt))
		{
			pic->error ++;
			return -1;
		}
	}
	if(pic->pct == MPG_TYPE_D)
	{
		bs_skip(bs,1);
	}
	if(!intra)
	{
		mpgdec_mc_process(dec,pic,mbt);
		mpgdec_reset_dct_params(pic);
	}
	else
	{
		mpgdec_reset_mv_params(pic,MPG_MBT_MVF | MPG_MBT_MVB,1);
	}
	mpgdec_mb_process(dec,pic);
	return 0;
}

static int mpgdec_parse_slice(mpgdec_t* dec)
{
	bitstream_t* bs = &dec->bs;
	mpgdec_picture_t* pic = dec->cur;
	pic->slice_vertical_position = (bs->value & 0xff) - 1;
	bs_skip(bs,32);
	pic->quantizer_scale = bs_get(bs,5,1);
	mpgdec_reset_dct_params(pic);
	if((pic->pct == MPG_TYPE_P) || (pic->pct == MPG_TYPE_B))
	{
		mpgdec_reset_mv_params(pic,MPG_MBT_MVF | MPG_MBT_MVB,1);
	}
	while(bs_get(bs,1,1))
	{
		bs_get(bs,8,1);
	}
	pic->macroblock_address = -1;
	do
	{
		if(0 != mpgdec_parse_macroblock(dec))
		{
			pic->error ++;
			break;
		}
	}while((bs_size(bs) > 0) && bs_get(bs,23,0));
	return 0;
}

static void mpgdec_parse_sequence_header(mpgdec_t* dec)
{
	int i;
	double fps[] = 
	{
		0.00,
		23.976,
		24.00,
		25.00,
		29.97,
		30.00,
		50.00,
		59.94,
		60.00,
		0.00,
		0.00,
		0.00,
		0.00,
		0.00,
		0.00,
		0.00
	};
	bitstream_t* bs = &dec->bs;
	bs_skip(bs,32);
	dec->info.width = bs_get(bs,12,1);
	dec->info.height = bs_get(bs,12,1);
	dec->info.par = bs_get(bs,4,1);
	dec->info.fps = fps[bs_get(bs,4,1)];
	dec->info.bitrate = bs_get(bs,18,1);
	if(dec->info.bitrate == 0x3ffff)
	{
		dec->info.bitrate = 0;
	}
	else
	{
		dec->info.bitrate *= 400;
	}
	bs_get(bs,12,1);
	if(bs_get(bs,1,1))
	{
		for(i = 0; i < 64; i ++)
		{
			dec->info.load_intra_quantizer_matrix[i] = bs_get(bs,8,1);
		}
	}
	else
	{
		memcpy(dec->info.load_intra_quantizer_matrix,default_quantizer_matrix,sizeof(default_quantizer_matrix));
	}
	if(bs_get(bs,1,1))
	{
		for(i = 0; i < 64; i ++)
		{
			dec->info.load_none_intra_quantizer_matrix[i] = bs_get(bs,8,1);
		}
	}
	else
	{
		memcpy(dec->info.load_none_intra_quantizer_matrix,default_none_quantizer_matrix,sizeof(default_none_quantizer_matrix));
	}
}

static int mpgdec_parse_gop_header(mpgdec_t* dec)
{
	int i;
	int timecode;
	mpgdec_picture_t* pic;
	bitstream_t* bs = &dec->bs;


	bs_skip(bs,32);
	timecode = bs_get(bs,25,1);
	dec->gop.hour = (timecode >> 19) & 0x1f;
	dec->gop.minute = (timecode >> 13) & 0x3f;
	dec->gop.second = (timecode >> 6) & 0x3f;
	dec->gop.closed_gop = bs_get(bs,1,1);
	dec->gop.broken_link = bs_get(bs,1,1);
	dec->gop.idx = 0;

	pic = dec->p;
	if(dec->gop.closed_gop)
	{
		dec->i = (mpgdec_picture_t*)NULL;
		dec->p = (mpgdec_picture_t*)NULL;
	}
	else
	{
		dec->i = pic;
		if(pic)
		{
			pic->alone = 1;
		}
		dec->p = (mpgdec_picture_t*)NULL;
	}
	for(i = 0; i < 1024; i ++)
	{
		pic = dec->gop.output[i];
		if(pic && (pic != dec->i) && (pic != dec->p) )
		{
			if(pic->frame)
			{
				img_destroy(pic->frame);
			}
			osFree(pic);
		}
		dec->gop.output[i] = (mpgdec_picture_t*)NULL;
	}
	return 0;
}


static int mpgdec_parse_picture(mpgdec_t* dec,double dts,double pts)
{
	int code;
	int tr;
	int pct;
	bitstream_t* bs = &dec->bs;
	mpgdec_picture_t* pic;

	bs_skip(bs,32);
	tr = bs_get(bs,10,1);
	pct = bs_get(bs,3,1);

	pic = mpgdec_alloc_picture(dec,tr,pct,dts,pts);
	if(!pic)
	{
		return -1;
	}
	pic->vbv_delay = bs_get(bs,16,1);
	if((pct == MPG_TYPE_P) || (pct == MPG_TYPE_B))
	{
		pic->full_pel_forward_vector = bs_get(bs,1,1);
		pic->forward_f_code = bs_get(bs,3,1);

		if(pct == MPG_TYPE_B)
		{
			pic->full_pel_backward_vector = bs_get(bs,1,1);
			pic->backward_f_code = bs_get(bs,3,1);
		}

	}
	while(bs_size(bs) > 4)
	{
		bs_align(bs);
		while(bs_size(bs) > 4)
		{
			code = bs_get(bs,32,0);
			if((code & 0xffffff00) == 0x00000100)
			{
				break;
			}
			bs_skip(bs,8);
		}
		if((code >= 0x00000101) && (code <= 0x000001af) )
		{
			mpgdec_parse_slice(dec);
		}
		else
		{
			break;
		}
	}

	if((pct == MPG_TYPE_I) || (pct == MPG_TYPE_P) )
	{
		if(!dec->i)
		{
			dec->i = pic;
		}
		else if(!dec->p)
		{
			dec->p = pic;
		}
		else
		{
			if(dec->i->alone)
			{
				img_destroy(dec->i->frame);
				osFree(dec->i);
			}
			dec->i = dec->p;
			dec->p = pic;
		}
	}

	return 0;
}



static int mpgdec_decode_frame(mpgdec_t* dec,double dts,double pts)
{
	int code;
	int ret = -1;
	int ok = 0;
	bitstream_t* bs = &dec->bs;
	while((bs_size(bs) > 4) && !ok)
	{
		bs_align(bs);
		code = bs_get(bs,32,0);
		switch(code)
		{
			case 0x000001b3:
				mpgdec_parse_sequence_header(dec);
				break;
			case 0x000001b8:
				mpgdec_parse_gop_header(dec);
				break;
			case 0x00000100:
				mpgdec_parse_picture(dec,dts,pts);
				ok = 1;
				break;
			default:
				bs_skip(bs,8);
				break;
		}
	}
	return bs_consumed(bs);
}


void* mpgdec_create()
{
	mpgdec_t* dec = (mpgdec_t*)osMalloc(sizeof(mpgdec_t));
	if(dec)
	{
		memset(dec,0,sizeof(mpgdec_t));
	}
	idct_init();
	return dec;
}

int mpgdec_decode(void* dec,char* es,int size,double dts,double pts)
{
	mpgdec_t* m1v = (mpgdec_t*)dec;
	bs_reload(&m1v->bs,es,size);
	return mpgdec_decode_frame(m1v,dts,pts);
}

img_frame_t* mpgdec_getframe(void* dec,int idx,mpegdec_info_t* info)
{
	mpgdec_t* m1v = (mpgdec_t*)dec;
	mpgdec_picture_t* pic;
	if(idx < 0)
	{
		idx = m1v->gop.idx;
	}
	else if(idx >= 1024)
	{
		return NULL;
	}
	pic = m1v->gop.output[idx];
	if(!pic)
	{
		return NULL;
	}
	m1v->gop.idx = (m1v->gop.idx + 1) & 0x3ff;
	if(pic->error < 10)
	{
		if(info)
		{
			info->bps = m1v->info.bitrate;
			info->fps = m1v->info.fps;
			info->width = m1v->info.width;
			info->height = m1v->info.height;
			info->idx = idx;
			info->pct = pic->pct;
		}
		return pic->frame;
	}
	else
	{
		info = info;
	}
	return NULL;
}

int mpgdec_destroy(void* dec)
{
	int i;
	mpgdec_picture_t* pic;
	mpgdec_t* m1v = (mpgdec_t*)dec;
	for(i = 0; i < 1024; i ++)
	{
		pic = m1v->gop.output[i];
		if(pic)
		{
			if(pic->frame)
			{
				img_destroy(pic->frame);
			}
			osFree(pic);
		}
	}
	osFree(dec);
	return 0;
}
