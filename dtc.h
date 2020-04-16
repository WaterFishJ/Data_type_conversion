/**
  ******************************************************************************
  * @author  Au Wenggee
  * @brief   Header for dtc.c module
  ******************************************************************************
  * @note
	*
	* This file include four data type conversion functions.
	* Used in conjunction with, for example, the CORDIC module 
	* and the FMAC module.
  ******************************************************************************
  */



#ifndef __DTC_H
#define __DTC_H

int Value_To_CORDIC31(float Value, float Coefficient);
void CORDIC31_To_Value(int CORDIC31, double *RES);
int Value_To_CORDIC15(float ValueA , float ValueB, float Coefficient);
void CORDIC15_To_Value(int CORDIC15, float *REA, float *REB);

#endif


