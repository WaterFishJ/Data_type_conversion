/**
  ******************************************************************************
  * @author  Au Wenggee
  * @brief   Data type conversion function prototypes.
  ******************************************************************************
  * @note
	*
	* This file include four data type conversion function prototypes.
	* Used in conjunction with, for example, the CORDIC module 
	* and the FMAC module.
  ******************************************************************************
  */



#include "dtc.h"



/**
  * @brief  将数据转换成CORDIC要求的Q1.31整型数据格式。
  * @param  需要转换的数据，取值可以为负数。
  * @param  比例系数；数据除以比例系数之后再转换格式；
	*         CORDIC的输入参数数值在[-1,1]之间，故需要先除以一个比例系数。
  *            @ref STM32G4 Series advanced Arm-based 32-bit MCUs - Reference Manual
  * @retval Q1.31整型数据
  */
int Value_To_CORDIC31(float Value, float Coefficient)
{
	int CORDIC31;
	CORDIC31 = (int)((Value/Coefficient)*0x80000000);
	return CORDIC31;
}


/**
  * @brief  将数据转换成CORDIC要求的Q1.15整型数据格式。
  * @param  需要转换的数据，取值可以为负数;该数据存放在高16位。
  * @param  需要转换的数据，取值可以为负数;该数据存放在低16位。
  * @param  比例系数；数据除以比例系数之后再转换格式；
	*         CORDIC的输入参数数值在[-1,1]之间，故需要先除以一个比例系数。
  *            @ref STM32G4 Series advanced Arm-based 32-bit MCUs - Reference Manual
  * @retval Q1.15整型数据
  */
int Value_To_CORDIC15(float ValueA, float ValueB, float Coefficient)
{
	int CORDIC15;
	CORDIC15 = (int)((ValueA/Coefficient)*0x8000) << 16;
	CORDIC15 = CORDIC15|(int)((ValueB/Coefficient)*0x8000);
	return CORDIC15;
}



/**
  * @brief  将CORDIC输出的Q1.31整型数据转换成带符号的浮点数值格式。
  * @param  需要转换的数据。
  * @param  存放输出数据的地址。
  * @note   转换无精度损失；
  *         精度要求不高时，可以将double类型换成float类型以节约RAM空间；此时精度将下降至1/10000000。
  */
void CORDIC31_To_Value(int CORDIC31, double	*RES)
{
	if (CORDIC31&0x80000000)
	{ /*为负数*/
		CORDIC31 = CORDIC31&0x7FFFFFFF;
		*RES = (((double)(CORDIC31)-0x80000000)/0x80000000);
	}
	else
	{/*为正数*/
		*RES = (double)(CORDIC31)/0x80000000;
	}
}


/**
  * @brief  将CORDIC输出的Q1.15整型数据转换成两个带符号的浮点数值格式。
  * @param  需要转换的数据。
  * @param  存放输出数据的地址；输出高位数据。
  * @param  存放输出数据的地址；输出低位数据。
  */
void CORDIC15_To_Value(int CORDIC15, float *REA, float *REB)
{
	if (CORDIC15&0x80000000)//处理高16位
	{/*为负数*/
		*REA = ((float)((CORDIC15>>16)&0x7FFF)-0x8000)/0x8000;
	}
	else
	{/*为正数*/
		*REA = (float)((CORDIC15>>16)&0xFFFF)/0x8000;
	}
	if (CORDIC15&0x8000)//处理低16位
	{/*为负数*/
		*REB = ((float)(CORDIC15&0x7FFF)-0x8000)/0x8000;
	}
	else
	{/*为正数*/
		*REB = (float)(CORDIC15&0xFFFF)/0x8000;
	}
}



