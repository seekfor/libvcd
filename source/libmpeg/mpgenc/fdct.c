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

#define USE_ACCURATE_ROUNDING

#define RIGHT_SHIFT(x, shft)  (short int)((x) >> (shft))

#ifdef USE_ACCURATE_ROUNDING
#define ONE ((int) 1)
#define DESCALE(x, n)  RIGHT_SHIFT((x) + (ONE << ((n) - 1)), n)
#else
#define DESCALE(x, n)  RIGHT_SHIFT(x, n)
#endif

#define CONST_BITS  13
#define PASS1_BITS  2

#define FIX_0_298631336  ((int)  2446) /* FIX(0.298631336) */
#define FIX_0_390180644  ((int)  3196) /* FIX(0.390180644) */
#define FIX_0_541196100  ((int)  4433) /* FIX(0.541196100) */
#define FIX_0_765366865  ((int)  6270) /* FIX(0.765366865) */
#define FIX_0_899976223  ((int)  7373) /* FIX(0.899976223) */
#define FIX_1_175875602  ((int)  9633) /* FIX(1.175875602) */
#define FIX_1_501321110  ((int) 12299) /* FIX(1.501321110) */
#define FIX_1_847759065  ((int) 15137) /* FIX(1.847759065) */
#define FIX_1_961570560  ((int) 16069) /* FIX(1.961570560) */
#define FIX_2_053119869  ((int) 16819) /* FIX(2.053119869) */
#define FIX_2_562915447  ((int) 20995) /* FIX(2.562915447) */
#define FIX_3_072711026  ((int) 25172) /* FIX(3.072711026) */

void fdct(short int *block,short int* dst)
{
	int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	int tmp10, tmp11, tmp12, tmp13;
	int z1, z2, z3, z4, z5;
	short int* blkptr;
	short int *dataptr;
	short int data[64];
	int i;
	
	/* Pass 1: process rows. */
	/* Note results are scaled up by sqrt(8) compared to a true DCT; */
	/* furthermore, we scale the results by 2**PASS1_BITS. */
	
	dataptr = data;
	blkptr = block;
	for (i = 0; i < 8; i++) 
	{
		tmp0 = (int)(blkptr[0] + blkptr[7]);
		tmp7 = (int)(blkptr[0] - blkptr[7]);
		tmp1 = (int)(blkptr[1] + blkptr[6]);
		tmp6 = (int)(blkptr[1] - blkptr[6]);
		tmp2 = (int)(blkptr[2] + blkptr[5]);
		tmp5 = (int)(blkptr[2] - blkptr[5]);
		tmp3 = (int)(blkptr[3] + blkptr[4]);
		tmp4 = (int)(blkptr[3] - blkptr[4]);
		
		
		
		tmp10 = tmp0 + tmp3;
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;
		
		dataptr[0] = (tmp10 + tmp11) << PASS1_BITS;
		dataptr[4] = (tmp10 - tmp11) << PASS1_BITS;
		
		z1 = (tmp12 + tmp13) * FIX_0_541196100;
		dataptr[2] = DESCALE(z1 + tmp13 * FIX_0_765366865, CONST_BITS - PASS1_BITS);
		dataptr[6] = DESCALE(z1 + tmp12 * (-FIX_1_847759065), CONST_BITS - PASS1_BITS);
		
		
		
		z1 = tmp4 + tmp7;
		z2 = tmp5 + tmp6;
		z3 = tmp4 + tmp6;
		z4 = tmp5 + tmp7;
		z5 = (z3 + z4) * FIX_1_175875602; /* sqrt(2) * c3 */
		
		tmp4 *= FIX_0_298631336; /* sqrt(2) * (-c1+c3+c5-c7) */
		tmp5 *= FIX_2_053119869; /* sqrt(2) * ( c1+c3-c5+c7) */
		tmp6 *= FIX_3_072711026; /* sqrt(2) * ( c1+c3+c5-c7) */
		tmp7 *= FIX_1_501321110; /* sqrt(2) * ( c1+c3-c5-c7) */
		z1 *= -FIX_0_899976223;  /* sqrt(2) * (c7-c3) */
		z2 *= -FIX_2_562915447;  /* sqrt(2) * (-c1-c3) */
		z3 *= -FIX_1_961570560;  /* sqrt(2) * (-c3-c5) */
		z4 *= -FIX_0_390180644;  /* sqrt(2) * (c5-c3) */
		
		z3 += z5;
		z4 += z5;
		
		dataptr[7] = DESCALE(tmp4 + z1 + z3, CONST_BITS - PASS1_BITS);
		dataptr[5] = DESCALE(tmp5 + z2 + z4, CONST_BITS - PASS1_BITS);
		dataptr[3] = DESCALE(tmp6 + z2 + z3, CONST_BITS - PASS1_BITS);
		dataptr[1] = DESCALE(tmp7 + z1 + z4, CONST_BITS - PASS1_BITS);
		
		dataptr += 8;   /* advance pointer to next row */
		blkptr += 8;
	}
	
	/* Pass 2: process columns.
	* We remove the PASS1_BITS scaling, but leave the results scaled up
	* by an overall factor of 8.
	*/
	
	dataptr = data;
	for (i = 0; i < 8; i++) 
	{
		tmp0 = dataptr[0] + dataptr[56];
		tmp7 = dataptr[0] - dataptr[56];
		tmp1 = dataptr[8] + dataptr[48];
		tmp6 = dataptr[8] - dataptr[48];
		tmp2 = dataptr[16] + dataptr[40];
		tmp5 = dataptr[16] - dataptr[40];
		tmp3 = dataptr[24] + dataptr[32];
		tmp4 = dataptr[24] - dataptr[32];
		
		
		
		tmp10 = tmp0 + tmp3;
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;
		
		dataptr[0] = DESCALE(tmp10 + tmp11, PASS1_BITS);
		dataptr[32] = DESCALE(tmp10 - tmp11, PASS1_BITS);
		
		z1 = (tmp12 + tmp13) * FIX_0_541196100;
		dataptr[16] = DESCALE(z1 + tmp13 * FIX_0_765366865, CONST_BITS + PASS1_BITS);
		dataptr[48] = DESCALE(z1 + tmp12 * (-FIX_1_847759065), CONST_BITS + PASS1_BITS);
		
		
		
		z1 = tmp4 + tmp7;
		z2 = tmp5 + tmp6;
		z3 = tmp4 + tmp6;
		z4 = tmp5 + tmp7;
		z5 = (z3 + z4) * FIX_1_175875602; /* sqrt(2) * c3 */
		
		tmp4 *= FIX_0_298631336; /* sqrt(2) * (-c1+c3+c5-c7) */
		tmp5 *= FIX_2_053119869; /* sqrt(2) * ( c1+c3-c5+c7) */
		tmp6 *= FIX_3_072711026; /* sqrt(2) * ( c1+c3+c5-c7) */
		tmp7 *= FIX_1_501321110; /* sqrt(2) * ( c1+c3-c5-c7) */
		z1 *= -FIX_0_899976223;  /* sqrt(2) * (c7-c3) */
		z2 *= -FIX_2_562915447;  /* sqrt(2) * (-c1-c3) */
		z3 *= -FIX_1_961570560;  /* sqrt(2) * (-c3-c5) */
		z4 *= -FIX_0_390180644;  /* sqrt(2) * (c5-c3) */
		
		z3 += z5;
		z4 += z5;
		
		dataptr[56] = DESCALE(tmp4 + z1 + z3, CONST_BITS + PASS1_BITS);
		dataptr[40] = DESCALE(tmp5 + z2 + z4, CONST_BITS + PASS1_BITS);
		dataptr[24] = DESCALE(tmp6 + z2 + z3, CONST_BITS + PASS1_BITS);
		dataptr[8] = DESCALE(tmp7 + z1 + z4, CONST_BITS + PASS1_BITS);
		
		dataptr++;    /* advance pointer to next column */
	}
	/* descale */
	for (i = 0; i < 64; i++)
	{
		dst[i] = DESCALE(data[i], 3);
	}
}


