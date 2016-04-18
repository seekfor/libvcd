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

#define ENC_STATE_SEQ			0
#define ENC_STATE_GOP			1
#define ENC_STATE_PIC			2


#define MAX_GOP_NUM				32

#define DEFAULT_QP				7

#define FORWARD_F_CODE			4

typedef struct
{
	mpgenc_info_t opt;
	
	bitstream_t* bs;
	short int default_none_quantizer_matrix[64];
	short int default_quantizer_matrix[64];
	void* dec;
	img_frame_t* frm;
	img_frame_t* ref;

	int state;
	int pic_type;
	int quantizer_scale;
	int hour;
	int minute;
	int second;
	int tr;

	int dct_dc_y_past;
	int dct_dc_cb_past;
	int dct_dc_cr_past;

	int mb_x;
	int mb_y;
	int mb_w;
	int mb_h;

	int mv_x;
	int mv_y;


	unsigned int bitcnts;
	unsigned int delay;
}mpgenc_t;

static void mpgenc_init_default_opt(mpgenc_info_t* opt)
{
	opt->min_qp = CFG_MIN_QP;
	opt->max_qp = CFG_MAX_QP;
	opt->max_bitrate = CFG_MAX_BITRATE;
	opt->avg_bitrate = CFG_AVG_BITRATE;

	opt->cbr = 0;
	opt->width = 352;
	opt->height = 288;
	opt->fps = 25;
	opt->gop = 25;
	opt->cs = 7;
}

static int mpgenc_encode_seq(mpgenc_t* enc)
{
	int par = 4;
	int fps = (enc->opt.fps == 30)?5:3;
	bitstream_t* bs = enc->bs;
	bs_align(bs);
	bs_put(bs,32,0x000001b3);
	bs_put(bs,12,enc->opt.width);
	bs_put(bs,12,enc->opt.height);
	bs_put(bs,4,par);
	bs_put(bs,4,fps);
	bs_put(bs,18,enc->opt.avg_bitrate?enc->opt.avg_bitrate / 400:0x3ffff);
	bs_put(bs,1,1);
	bs_put(bs,10,64);
	bs_put(bs,1,1);
	bs_put(bs,1,0);
	bs_put(bs,1,0);
	bs_align(bs);
	enc->state = ENC_STATE_GOP;
	return 0;
}

static int mpgenc_encode_gop(mpgenc_t* enc)
{
	bitstream_t* bs = enc->bs;
	bs_align(bs);
	bs_put(bs,32,0x000001b8);
	bs_put(bs,1,0);
	bs_put(bs,5,enc->hour);
	bs_put(bs,6,enc->minute);
	bs_put(bs,1,1);
	bs_put(bs,6,enc->second);
	bs_put(bs,6,enc->opt.num_pic_encoded % enc->opt.gop);
	bs_put(bs,1,1);
	bs_put(bs,1,0);
	bs_align(bs);
	enc->tr = 0;
	enc->state = ENC_STATE_PIC;
	return 0;
}

static void mpgenc_reset_dct_params(mpgenc_t* enc)
{
	enc->dct_dc_y_past = 1024;
	enc->dct_dc_cb_past = 1024;
	enc->dct_dc_cr_past = 1024;
}

static void mpgenc_reset_mv_params(mpgenc_t* enc)
{
	enc->mv_x = 0;
	enc->mv_y = 0;
}

static void mpgenc_copy_pel(mpgenc_t* enc,img_frame_t* frm,int x,int y,int blk,short int* dst)
{
	unsigned char* src;
	int i,j;
	int w;
	mpgenc_info_t* opt = &enc->opt;
	if(blk >= 4)
	{
		if((blk == 4) && !(opt->cs & 0x02))
		{
			for(i = 0; i < 64; i ++)
			{
				dst[i] = 0x80;
			}
			return;
		}
		if((blk == 5) && !(opt->cs & 0x04))
		{
			for(i = 0; i < 64; i ++)
			{
				dst[i] = 0x80;
			}
			return;
		}
		w = frm->width / 2;
		src = (blk == 4)?frm->u:frm->v;
	}
	else
	{
		if(!(opt->cs & 0x01))
		{
			for(i = 0; i < 64; i ++)
			{
				dst[i] = 0x10;
			}
			return;
		}
		src = frm->y;
		w = frm->width;
	}
	
	src += y * w + x;
	if(blk < 4)
	{
		if(blk > 1)
		{
			src += 8 * w;
		}
		if(blk & 0x01)
		{
			src += 8;
		}
	}
	for(i = 0; i < 8; i ++)
	{
		for(j = 0; j < 8; j ++)
		{
			dst[i * 8 + j] = src[i * w + j];
		}
	}

}

static int calc_dc_size(int value)
{
	int i;
	if(!value)
	{
		return 0;
	}
	for(i = 1; i < 9; i ++)
	{
		if(value < (1 << i))
		{
			return i;
		}
	}
	return 8;
}

static int calc_rle(short int* zz,int* i,int* r)
{
	int j = *i;
	short int level = 0;
	short int run = 0;
	for(;j < 64; j ++)
	{
		if(zz[j])
		{
			level = zz[j];
			break;
		}
		++run;
	}
	*i = (j + 1);
	*r = run;
	return level;
}

static int mpgenc_encode_i_block(mpgenc_t* enc,int blk)
{
	short int src[8][8];
	short int dct[8][8];
	short int zz[64];
	int m,n;
	int i;
	int size;
	int r,l;
	short int diff;
	vld_t* vld;
	bitstream_t* bs = enc->bs;
	int ratio = 16;
	if(blk > 3)
	{
		ratio = 8;
	}
	mpgenc_copy_pel(enc,enc->frm,enc->mb_x * ratio,enc->mb_y * ratio,blk,(short int*)src);
	fdct((short int*)src,(short int*)dct);
	for(m = 0; m < 8;m ++)
	{
		for(n = 0; n < 8; n ++)
		{
			if(dct[m][n] < -2048)
			{
				dct[m][n] = -2048;
			}
			else if(dct[m][n] > 2047)
			{
				dct[m][n] = 2047;
			}
			if(!(dct[m][n] & 0x01))
			{
				dct[m][n] += Sign(dct[m][n]);
			}
			i = default_scan[m][n];
			zz[i] = (dct[m][n] * 8)  / (enc->quantizer_scale * enc->default_quantizer_matrix[i]);
		}
	}

	switch(blk)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			diff = (dct[0][0] / 8 - enc->dct_dc_y_past / 8);
			enc->dct_dc_y_past = dct[0][0];
			size = calc_dc_size(abs(diff));
			vld = vlc_get(m1vtbl_dct_ysize,sizeof(m1vtbl_dct_ysize) / sizeof(vld_t),size);
			bs_put(bs,vld->bit,vld->value);
			if(size)
			{
				if(diff < 0)
				{
					diff = diff + (1 << size) - 1;
				}
				bs_put(bs,size,diff);
			}
			break;
		case 4:
			diff = dct[0][0] / 8 - enc->dct_dc_cb_past / 8;
			enc->dct_dc_cb_past = dct[0][0];
			size = calc_dc_size(abs(diff));
			vld = vlc_get(m1vtbl_dct_csize,sizeof(m1vtbl_dct_csize) / sizeof(vld_t),size);
			bs_put(bs,vld->bit,vld->value);
			if(size)
			{
				if(diff < 0)
				{
					diff = diff + (1 << size) - 1;
				}
				bs_put(bs,size,diff);
			}
			break;
		case 5:
			diff = dct[0][0] / 8 - enc->dct_dc_cr_past / 8;
			enc->dct_dc_cr_past = dct[0][0];
			size = calc_dc_size(abs(diff));
			vld = vlc_get(m1vtbl_dct_csize,sizeof(m1vtbl_dct_csize) / sizeof(vld_t),size);
			bs_put(bs,vld->bit,vld->value);
			if(size)
			{
				if(diff < 0)
				{
					diff = diff + (1 << size) - 1;
				}
				bs_put(bs,size,diff);
			}
			break;
	}

	for(i = 1; i < 64; )
	{
		l = calc_rle(zz,&i,&r);
		vld = vlc_get(m1vtbl_dct + 1,sizeof(m1vtbl_dct) / sizeof(vld_t) - 1,(r << 16) | abs(l));
		if(vld)
		{
			bs_put(bs,vld->bit,vld->value);
			bs_put(bs,1,(l < 0)?1:0);
		}
		else if(l)
		{
			bs_put(bs,6,1);
			bs_put(bs,6,r);
			if(l < -127)
			{
				bs_put(bs,8,0x80);
				bs_put(bs,8,(l + 256) & 0xff);			
			}
			else if(l < 128)
			{
				bs_put(bs,8,l & 0xff);
			}
			else
			{
				bs_put(bs,8,0);
				bs_put(bs,8,l & 0xff);
			}
		}
	}
	bs_put(bs,2,0x02);
	return 0;
}


static int mpgenc_encode_i_macroblock(mpgenc_t* enc)
{
	int i;
	bitstream_t* bs = enc->bs;
	bs_put(bs,1,0x01);
	if(enc->pic_type == MPG_TYPE_I)
	{
		bs_put(bs,1,0x01);
	}
	else
	{
		bs_put(bs,5,0x03);
	}
	for(i = 0; i < 6; i ++)
	{
		mpgenc_encode_i_block(enc,i);
	}
	return 0;
}

static int mpgenc_encode_i_slices(mpgenc_t* enc)
{
	bitstream_t* bs = enc->bs;
	for(enc->mb_y = 0; enc->mb_y < enc->mb_h; enc->mb_y ++)
	{
		mpgenc_reset_dct_params(enc);
		bs_align(bs);
		bs_put(bs,32,0x000101 + enc->mb_y);
		bs_put(bs,5,enc->quantizer_scale);
		bs_put(bs,1,0);
		for(enc->mb_x = 0; enc->mb_x < enc->mb_w; enc->mb_x ++)
		{
			mpgenc_encode_i_macroblock(enc);
		}
	}
	return 0;
}


static int mpgenc_encode_p_block(mpgenc_t* enc,short int* zz)
{
	int i;
	int l,r;
	bitstream_t* bs = enc->bs;
	vld_t* vld;

	for(i = 0; i < 64; )
	{
		int offset = !!i;
		l = calc_rle(zz,&i,&r);
		vld = vlc_get(m1vtbl_dct + offset,(sizeof(m1vtbl_dct) / sizeof(vld_t)) - offset,(r << 16) | abs(l));
		if(vld)
		{
			bs_put(bs,vld->bit,vld->value);
			bs_put(bs,1,(l < 0)?1:0);
		}
		else if(l)
		{
			bs_put(bs,6,1);
			bs_put(bs,6,r);
			if(l < -127)
			{
				bs_put(bs,8,0x80);
				bs_put(bs,8,(l + 256) & 0xff);			
			}
			else if(l < 128)
			{
				bs_put(bs,8,l & 0xff);
			}
			else
			{
				bs_put(bs,8,0);
				bs_put(bs,8,l & 0xff);
			}
		}
	}
	bs_put(bs,2,0x02);
	return 0;
}

static char* mpgenc_pel_locate(img_frame_t* frm,int blk,int y,int x)
{
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


static void mpgenc_copy_mb_pel(img_frame_t* ref,int mb_x,int mb_y,int blk,int right,int down,int right_half,int down_half,short int* dst)
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
		pad_w = ref->pad_width;
		pad_h = ref->pad_height;
	}
	else
	{
		w = 8;
		h = 8;
		pad_w = ref->pad_width / 2;
		pad_h = ref->pad_height / 2;
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

	A = mpgenc_pel_locate(ref,blk,y,x);
	B = mpgenc_pel_locate(ref,blk,y + 1,x);
	C = mpgenc_pel_locate(ref,blk,y,x + 1);
	D = mpgenc_pel_locate(ref,blk,y + 1,x + 1);


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


static int mpgenc_get_p_macroblock_info(mpgenc_t* enc,int mv_x,int mv_y,short int zz[6][64])
{
	short int y[16][16];
	short int u[8][8];
	short int v[8][8];

	short int src[8][8];
	short int diff[8][8];
	short int dct[8][8];
	short int zero[64];
	int i,j;
	int m,n;
	int cbp = 0x00;
	int right[2];
	int right_half[2];
	int down[2];
	int down_half[2];
	memset(zero,0,sizeof(zero));

	right[0] = mv_x >> 1;
	right_half[0] = mv_x - (2 * right[0]);
	down[0] = mv_y >> 1;
	down_half[0] = mv_y - (2 * down[0]);
	
	right[1] = (mv_x / 2) >> 1;
	right_half[1] = (mv_x / 2) - (2 * right[1]);
	down[1] = (mv_y / 2) >> 1;
	down_half[1] = mv_y / 2 - (2 * down[1]);

	mpgenc_copy_mb_pel(enc->ref,enc->mb_x,enc->mb_y,0,right[0],down[0],right_half[0],down_half[0],(short int*)y);
	mpgenc_copy_mb_pel(enc->ref,enc->mb_x,enc->mb_y,4,right[1],down[1],right_half[1],down_half[1],(short int*)u);
	mpgenc_copy_mb_pel(enc->ref,enc->mb_x,enc->mb_y,5,right[1],down[1],right_half[1],down_half[1],(short int*)v);

	for(i = 0; i < 6; i ++)
	{
		if(i < 4)
		{
			short int* pel;
			mpgenc_copy_pel(enc,enc->frm,enc->mb_x * 16,enc->mb_y * 16,i,(short int*)src);
			pel = (short int*)y + (i >> 1) * 8 * 16 + (i & 0x01 ) * 8;
			for(m = 0; m < 8; m ++)
			{
				for(n = 0; n < 8; n ++)
				{
					diff[m][n] = src[m][n] - pel[n];
				}
				pel += 16;
			}
		}
		else
		{
			mpgenc_copy_pel(enc,enc->frm,enc->mb_x * 8,enc->mb_y * 8,i,(short int*)src);
			for(m = 0; m < 8; m ++)
			{
				for(n = 0; n < 8; n ++)
				{
					if(i == 4)
					{
						diff[m][n] = src[m][n] - u[m][n];
					}
					else
					{
						diff[m][n] = src[m][n] - v[m][n];
					}
				}
			}
		}
		fdct((short int*)diff,(short int*)dct);
		for(m = 0; m < 8;m ++)
		{
			for(n = 0; n < 8; n ++)
			{
				j = default_scan[m][n];
				if(!(dct[m][n] & 0x01))
				{
					dct[m][n] += Sign(dct[m][n]);
				}
				if(dct[m][n] < -2048)
				{
					dct[m][n] = -2048;
				}
				else if(dct[m][n] > 2047)
				{
					dct[m][n] = 2047;
				}
				zz[i][j] = ((dct[m][n] * 16) / (enc->quantizer_scale * enc->default_none_quantizer_matrix[j]) - Sign(dct[m][n])) / 2;

			}
		}

		if(memcmp(zz[i],zero,128))
		{
			cbp |= (1 << (5 - i));
		}
	}
	return cbp;
}

static int mpgenc_encode_p_macroblock(mpgenc_t* enc,int skip,int mbt,int mv_x,int mv_y,int cbp,short int zz[6][64])
{
	bitstream_t* bs = enc->bs;
	vld_t* vld;
	int i;

	while(skip > 33)
	{
		bs_put(bs,11,0x08);
		skip -= 33;
	}

	vld = vlc_get(m1vtbl_mba,sizeof(m1vtbl_mba) / sizeof(vld_t),skip);
	bs_put(bs,vld->bit,vld->value);
	vld = vlc_get(m1vtbl_mbtp,sizeof(m1vtbl_mbtp) / sizeof(vld_t),mbt);
	bs_put(bs,vld->bit,vld->value);

	if(mbt & MPG_MBT_QUANT)
	{
		bs_put(bs,5,enc->quantizer_scale);
	}

	if(mbt & MPG_MBT_MVF)
	{
		int M,R;
		enc->mv_x = mv_calc_mr(FORWARD_F_CODE,mv_x,enc->mv_x,&M,&R);
		vld = vlc_get(m1vtbl_mv,sizeof(m1vtbl_mv) / sizeof(vld_t),M);
		bs_put(bs,vld->bit,vld->value);
		if(M)
		{
			bs_put(bs,FORWARD_F_CODE - 1,R);
		}
		enc->mv_y = mv_calc_mr(FORWARD_F_CODE,mv_y,enc->mv_y,&M,&R);
		vld = vlc_get(m1vtbl_mv,sizeof(m1vtbl_mv) / sizeof(vld_t),M);
		bs_put(bs,vld->bit,vld->value);
		if(M)
		{
			bs_put(bs,FORWARD_F_CODE - 1,R);
		}
	}

	if(mbt & MPG_MBT_CBP)
	{
		vld = vlc_get(m1vtbl_cbp,sizeof(m1vtbl_cbp) / sizeof(vld_t),cbp);
		bs_put(bs,vld->bit,vld->value);
	}


	for(i = 0; i < 6; i ++)
	{
		if(cbp & (1 << (5 - i)) )
		{
			mpgenc_encode_p_block(enc,zz[i]);
		}
	}


	return 0;
}

static int mpgenc_get_next_macroblock(mpgenc_t* enc,int x[],int y[],short int zz[6][64],int* cbp)
{
	int skip = 1;
	int c;
	*cbp = 0;
	for(;enc->mb_x < enc->mb_w;enc->mb_x ++)
	{
		if(skip > 1)
		{
			mpgenc_reset_mv_params(enc);
		}
		c = mpgenc_get_p_macroblock_info(enc,x[enc->mb_x],y[enc->mb_x],zz);
		if(c || !enc->mb_x || (enc->mb_x == enc->mb_w - 1) || x[enc->mb_x] || y[enc->mb_x])
		{
			*cbp = c;
			break;
		}
		skip ++;
	}
	return skip;
}

static int mpgenc_encode_p_slices(mpgenc_t* enc)
{
	int i;
	int mbt[4096 / 16];
	int x[4096 / 16];
	int y[4096 / 16];
	int sad[4096 / 16];
	short int zz[6][64];
	int cbp;
	int skip;
	bitstream_t* bs = enc->bs;
	for(enc->mb_y = 0; enc->mb_y < enc->mb_h; enc->mb_y ++)
	{
		mpgenc_reset_dct_params(enc);
		mpgenc_reset_mv_params(enc);
		bs_align(bs);
		bs_put(bs,32,0x000101 + enc->mb_y);
		bs_put(bs,5,enc->quantizer_scale);
		bs_put(bs,1,0);
		for(i = 0; i < enc->mb_w; i ++)
		{
			mv_calc(enc->ref->y,enc->frm->y,i * 16,enc->mb_y * 16,enc->mb_w * 16,enc->mb_h * 16,&sad[i],&x[i],&y[i]);
			mbt[i] = MPG_MBT_MVF;
		}
		for(enc->mb_x = 0; enc->mb_x < enc->mb_w;enc->mb_x++)
		{
			skip = mpgenc_get_next_macroblock(enc,x,y,zz,&cbp);
			i = enc->mb_x;
			if(skip > 1)
			{
				mpgenc_reset_mv_params(enc);
			}
			if(cbp)
			{
				mbt[i] |= MPG_MBT_CBP;
			}
			mpgenc_encode_p_macroblock(enc,skip,mbt[i],x[i],y[i],cbp,zz);
		}

	}
	return 0;
}

static int mpgenc_encode_slices(mpgenc_t* enc,int type)
{
	bitstream_t* bs = enc->bs;
	switch(type)
	{
	case MPG_TYPE_I:
		mpgenc_encode_i_slices(enc);
		break;
	case MPG_TYPE_P:
		mpgenc_encode_p_slices(enc);
		break;
	default:
		return -1;
	}
	bs_align(bs);
	bs_flush(bs);
	return 0;
}


static int mpgenc_encode_pic(mpgenc_t* enc,char* y,char* u,char* v,int type)
{
	mpegdec_info_t info;
	bitstream_t* bs = enc->bs;
	if(!enc->frm)
	{
		enc->frm = img_create(IMG_TYPE_YUV420,enc->opt.width,enc->opt.height,0,0);
		if(!enc->frm)
		{
			return -1;
		}
	}
	if(y)
	{
		memcpy(enc->frm->y,y,enc->opt.width * enc->opt.height);
	}
	if(u)
	{
		memcpy(enc->frm->u,u,enc->opt.width * enc->opt.height / 4);
	}
	if(v)
	{
		memcpy(enc->frm->v,v,enc->opt.width * enc->opt.height / 4);
	}
	bs_align(bs);
	bs_put(bs,32,0x00000100);
	bs_put(bs,10,enc->tr);
	bs_put(bs,3,type);
	bs_put(bs,16,9000);

	if(type == MPG_TYPE_P)
	{
		bs_put(bs,1,CFG_FULLPEL_SUPPORT);
		bs_put(bs,3,FORWARD_F_CODE);
	}
	bs_put(bs,1,0);

	mpgenc_encode_slices(enc,type);

	++enc->tr;
	++enc->opt.num_pic_encoded;
	if(!(enc->opt.num_pic_encoded % enc->opt.fps))
	{
		++enc->second;
		if(enc->second == 60)
		{
			enc->second = 0;
			++enc->minute;
			if(enc->minute == 60)
			{
				enc->minute = 0;
				++enc->hour;
				if(enc->hour == 24)
				{
					enc->hour = 0;
				}
			}
		}
	}

	if(!(enc->opt.num_pic_encoded % enc->opt.gop))
	{
		++enc->opt.num_gop_encoded;
		if(enc->opt.num_gop_encoded > MAX_GOP_NUM)
		{
			enc->state = ENC_STATE_SEQ;
		}
		else
		{
			enc->state = ENC_STATE_GOP;
		}
		enc->tr = 0;
	}
	enc->pic_type = (enc->opt.num_pic_encoded % enc->opt.gop)?MPG_TYPE_P:MPG_TYPE_I;
	if(enc->dec && (enc->pic_type != MPG_TYPE_I))
	{
		mpgdec_decode(enc->dec,enc->bs->base,bs_consumed(enc->bs),0,0);
		enc->ref = mpgdec_getframe(enc->dec,-1,&info);
	}

	return 0;
}


void* mpgenc_create(int bufsize)
{
	mpgenc_t* enc;
	enc = (mpgenc_t*)osMalloc(sizeof(mpgenc_t));
	if(!enc)
	{
		return NULL;
	}
	memset(enc,0,sizeof(mpgenc_t));
	memcpy(enc->default_quantizer_matrix,default_quantizer_matrix,sizeof(default_quantizer_matrix));
	memcpy(enc->default_none_quantizer_matrix,default_none_quantizer_matrix,sizeof(default_none_quantizer_matrix));
	enc->dec = mpgdec_create();
	enc->bs = bs_create(bufsize?bufsize:128 * 1024);
	mpgenc_init_default_opt(&enc->opt);
	enc->state = ENC_STATE_SEQ;
	enc->quantizer_scale = DEFAULT_QP;
	enc->pic_type = MPG_TYPE_I;
	enc->delay = 2;
	return enc;
}

int mpgenc_encode(void* hdl,char* y,char* u,char* v)
{
	mpgenc_t* enc = (mpgenc_t*)hdl;


	if(!hdl)
	{
		return -1;
	}
	if(!enc->ref)
	{
		enc->pic_type = MPG_TYPE_I;
	}
	if(enc->opt.cs & 0x01)
	{
		if(!y)
		{
			return -1;
		}
	}
	else
	{
		y = NULL;
	}

	if(enc->opt.cs & 0x02)
	{
		if(!u)
		{
			return -1;
		}
	}
	else
	{
		u = NULL;
	}
	
	if(enc->opt.cs & 0x04)
	{
		if(!v)
		{
			return -1;
		}
	}
	else
	{
		v = NULL;
	}

	bs_reset(enc->bs);

	switch(enc->state)
	{
	case ENC_STATE_SEQ:
		mpgenc_encode_seq(enc);
	case ENC_STATE_GOP:
		mpgenc_encode_gop(enc);
	case ENC_STATE_PIC:
		if(enc->pic_type == MPG_TYPE_I)
		{
			enc->quantizer_scale = enc->opt.min_qp;
		}
		mpgenc_encode_pic(enc,y,u,v,enc->pic_type);
		break;
	}
	enc->bitcnts += bs_consumed(enc->bs) * 8;
	if(enc->delay)
	{
		--enc->delay;
	}
	if(enc->opt.cbr && enc->opt.num_pic_encoded)
	{
		int delta;
		enc->opt.bitrate = (unsigned int)((double)enc->bitcnts / ((double)enc->opt.num_pic_encoded / (double)enc->opt.fps));
		delta = enc->opt.bitrate - enc->opt.avg_bitrate;
		enc->opt.qp = enc->quantizer_scale;
		if(delta > 1000000)
		{
			if(enc->quantizer_scale <= enc->opt.max_qp - 2)
			{
				enc->quantizer_scale +=2;
			}
			else
			{
				enc->quantizer_scale = enc->opt.max_qp;
			}
		}
		else if(delta > 200000)
		{
			if(enc->quantizer_scale <= enc->opt.max_qp - 1)
			{
				enc->quantizer_scale ++;
			}
			else
			{
				enc->quantizer_scale = enc->opt.max_qp;
			}
		}
		else if(delta < -1000000)
		{
			if(enc->quantizer_scale >= enc->opt.min_qp + 2)
			{
				enc->quantizer_scale -= 2;
			}
			else
			{
				enc->quantizer_scale = enc->opt.min_qp;
			}
		}
		else if(delta < -200000)
		{
			if(enc->quantizer_scale >= enc->opt.min_qp + 1)
			{
				enc->quantizer_scale --;
			}
			else
			{
				enc->quantizer_scale = enc->opt.min_qp;
			}
		}
		
	}

	return 0;
}

int mpgenc_getbitstream(void* hdl,char** es,mpgenc_info_t* info)
{
	int size;
	mpgenc_t* enc = (mpgenc_t*)hdl;
	size = bs_consumed(enc->bs);
	if(es)
	{
		*es = enc->bs->base;
	}
	if(info)
	{
		mpgenc_getinfo(hdl,info);
	}
	return size;
}

int mpgenc_getinfo(void* hdl,mpgenc_info_t* info)
{
	mpgenc_t* enc = (mpgenc_t*)hdl;
	if(info)
	{
		*info = enc->opt;
	}
	return 0;
}

int mpgenc_setinfo(void* hdl,mpgenc_info_t* info)
{
	mpgenc_t* enc = (mpgenc_t*)hdl;
	if(info)
	{
		if((info->avg_bitrate > info->max_bitrate) || !info->min_qp || !info->max_qp || !info->cs || (info->gop <= 0) )
		{
			return -1;
		}

		enc->opt = *info;
		
		enc->mb_w = info->width / 16;
		enc->mb_h = info->height / 16;
	}
	return 0;
}

int mpgenc_destroy(void* hdl)
{
	mpgenc_t* enc = (mpgenc_t*)hdl;
	if(enc->dec)
	{
		mpgdec_destroy(enc->dec);
	}
	bs_destroy(enc->bs);
	osFree(enc);
	return 0;
}

