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
  * @brief  ������ת����CORDICҪ���Q1.31�������ݸ�ʽ��
  * @param  ��Ҫת�������ݣ�ȡֵ����Ϊ������
  * @param  ����ϵ�������ݳ��Ա���ϵ��֮����ת����ʽ��
	*         CORDIC�����������ֵ��[-1,1]֮�䣬����Ҫ�ȳ���һ������ϵ����
  *            @ref STM32G4 Series advanced Arm-based 32-bit MCUs - Reference Manual
  * @retval Q1.31��������
  */
int Value_To_CORDIC31(float Value, float Coefficient)
{
	int CORDIC31;
	CORDIC31 = (int)((Value/Coefficient)*0x80000000);
	return CORDIC31;
}


/**
  * @brief  ������ת����CORDICҪ���Q1.15�������ݸ�ʽ��
  * @param  ��Ҫת�������ݣ�ȡֵ����Ϊ����;�����ݴ���ڸ�16λ��
  * @param  ��Ҫת�������ݣ�ȡֵ����Ϊ����;�����ݴ���ڵ�16λ��
  * @param  ����ϵ�������ݳ��Ա���ϵ��֮����ת����ʽ��
	*         CORDIC�����������ֵ��[-1,1]֮�䣬����Ҫ�ȳ���һ������ϵ����
  *            @ref STM32G4 Series advanced Arm-based 32-bit MCUs - Reference Manual
  * @retval Q1.15��������
  */
int Value_To_CORDIC15(float ValueA, float ValueB, float Coefficient)
{
	int CORDIC15;
	CORDIC15 = (int)((ValueA/Coefficient)*0x8000) << 16;
	CORDIC15 = CORDIC15|(int)((ValueB/Coefficient)*0x8000);
	return CORDIC15;
}



/**
  * @brief  ��CORDIC�����Q1.31��������ת���ɴ����ŵĸ�����ֵ��ʽ��
  * @param  ��Ҫת�������ݡ�
  * @param  ���������ݵĵ�ַ��
  * @note   ת���޾�����ʧ��
  *         ����Ҫ�󲻸�ʱ�����Խ�double���ͻ���float�����Խ�ԼRAM�ռ䣻��ʱ���Ƚ��½���1/10000000��
  */
void CORDIC31_To_Value(int CORDIC31, double	*RES)
{
	if (CORDIC31&0x80000000)
	{ /*Ϊ����*/
		CORDIC31 = CORDIC31&0x7FFFFFFF;
		*RES = (((double)(CORDIC31)-0x80000000)/0x80000000);
	}
	else
	{/*Ϊ����*/
		*RES = (double)(CORDIC31)/0x80000000;
	}
}


/**
  * @brief  ��CORDIC�����Q1.15��������ת�������������ŵĸ�����ֵ��ʽ��
  * @param  ��Ҫת�������ݡ�
  * @param  ���������ݵĵ�ַ�������λ���ݡ�
  * @param  ���������ݵĵ�ַ�������λ���ݡ�
  */
void CORDIC15_To_Value(int CORDIC15, float *REA, float *REB)
{
	if (CORDIC15&0x80000000)//�����16λ
	{/*Ϊ����*/
		*REA = ((float)((CORDIC15>>16)&0x7FFF)-0x8000)/0x8000;
	}
	else
	{/*Ϊ����*/
		*REA = (float)((CORDIC15>>16)&0xFFFF)/0x8000;
	}
	if (CORDIC15&0x8000)//�����16λ
	{/*Ϊ����*/
		*REB = ((float)(CORDIC15&0x7FFF)-0x8000)/0x8000;
	}
	else
	{/*Ϊ����*/
		*REB = (float)(CORDIC15&0xFFFF)/0x8000;
	}
}



