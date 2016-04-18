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
#include <math.h>

static double C[8][8];
static double Ct[8][8];
static int inited = 0;

void idct_init()
{
	int i;
    int j;
	if(inited)
	{
		return;
	}
	inited = 1;
	for(j = 0;j < 8; j++)
	{
        C[0][j] = 1.0 / sqrt(8.0);
        Ct[j][0] = C[0][j];
    }
    for(i = 1; i < 8; i++) 
	{
        for(j = 0;j < 8;j ++)
		{
            C[i][j] = sqrt(2.00000000000 / 8.0000000) * cos(3.14159265358 * (2.00 * j + 1.00) * i / 16.0000000000);
            Ct[j][i] = C[i][j];
        }
    }
}

void idct(short int* src, short int* dst)
{
    double temp[8][8];
    double temp1;
    int i;
    int j;
    int k;
    for (i = 0; i < 8; i++) 
	{
        for (j = 0; j < 8; j++) 
		{
            temp[i][j] = 0;
            for (k = 0; k < 8; k++)
			{
				temp[i][j] += (double)((src[i * 8 + k]) * C[k][j]);
			}
        }
    }
	
    for (i = 0; i < 8; i++) 
	{
        for (j = 0; j < 8; j++) 
		{
            temp1 = 0;
            for (k = 0; k < 8; k++)
            {    
				temp1 += (double)(Ct[i][k] * temp[k][j]);
			}
			if(temp1 < -256)
			{
				temp1 = -256;
			}
			else if(temp1 > 255)
			{
				temp1 = 255;
			}
			*(dst + 8 * i + j)=(short int)temp1;
        }
    }
	
}

