# STM32G4_CORDIC与定点带符号整数数据格式

> 2019年ST推出的G4系列芯片是STM32系列第一款带有CORDIC协同处理器的芯片。CORDIC协同处理器提供某些数学函数的硬件加速，尤其是三角函数。它能加快这些函数的运算，释放处理器以执行其他任务。通常用于电机控制、测量、信号处理和许多其他应用。



### 1. 关于CORDIC

CORDIC(coordinate rotation digital computer坐标旋转数字计算机)是一种用于计算三角函数和双曲线函数的低成本逐次逼近算法。最初由Jack Volder在1959年提出，它被广泛用于早期计算器当中。CORDIC算法通过基本的加和移位运算代替乘法运算，具体原理不在此赘述。<u>*（可查阅ST官方文档：AN5325）*</u>

<img src="https://github.com/WaterFishJ/Data_type_conversion/blob/master/STM32G4_CORDIC%E4%B8%8EQ%E6%A0%BC%E5%BC%8F/%E6%89%B9%E6%B3%A8%202020-03-26%20152057.png" style="zoom:67%;" />

<center>坐标旋转算法示意图</center>

### 2. STM32G4中使用CORDIC

#### 2.1 初始配置

使用STM32CubeMX激活CORDIC，再按需选择配置NVIC或者DMA。生成代码支持HAL库和LL库。此时代码包含了CORDIC的初始化（CORDIC_Initialize），不包括CORDIC的配置（CORDIC_Configure）。需要用户自行实例化结构体CORDIC_ConfigTypeDef：

<img src="/批注 2020-03-27 012200.png" alt="批注 2020-03-27 012200" style="zoom:80%;" />

<center>结构体成员变量</center>

**Function**包含共10种函数：余弦、正弦、方位角、取模、反正切、双曲余弦、双曲正弦、双曲反正切、自然对数、开平方。

**Scale**指缩放因子；CORDIC要求输入数值在[-1,1]区间内；设输入值为x，缩放因子为n，则会先对输入数值做运算x=x·2^-n，CORDIC计算完成后输出结果y再做运算y=y·2^n；缩放因子取值范围视所选函数规定，在[0,7]区间内；缩放因子可能会导致运算精度的丢失。<u>*（可查阅ST官方文档：RM0440）*</u>

**InSize**与**OutSize**：提供两种输入输出数据格式Q1.31和Q1.15；每次向CORDIC输入数据时，会发送一个32位的数据，当选择Q1.31数据格式时，CORDIC一次运算一个数据；当选择Q1.15数据格式时，将两个数据封装在一个32位数据中，CORDIC一次运算两个数据；选择Q1.15数据格式能提高运算速度，但牺牲了运算精度；有关Q1.31和Q1.15数据格式的内容下文会讲到。

**NbWrite**与**NbRead**：写入写出的参数数量；有些函数，比如求方位角、取模，需要输入x，y两个参数才能进行运算；有些函数，比如余弦、正弦，CORDIC运算后可以输出两个结果，例如进行余弦函数运算时，可以输出一个余弦结果和一个正弦结果；两个数据以交替的形式进行输入输出；输入时若次要参数一直保持不变，可在第一次计算开始后将NbWrite设置为1个参数输入模式。<u>*（可查阅ST官方文档：RM0440）*</u>

**Precision**指迭代周期；取值范围为1到13；迭代周期越多，运算精度越高，运算速度越低；当运算精度到达数据格式所能表达的精度极限时，继续增加迭代周期毫无意义，例如迭代6周期运算已经达到Q1.31数据格式正弦函数运算所能表达的精度极限，继续增加迭代周期的运算结果与迭代6周期的运算结果无异；对于大多数函数，迭代周期推荐3到6周期。

配置完结构体变量，后使用CORDIC_Configure函数将数据写入CORDIC_CSR寄存器，CORDIC的初始化和配置完成。

![批注 2020-03-27 145722](/批注 2020-03-27 145722.png)
<center>CORDIC_CSR寄存器</center>




#### 2.2 CORDIC的读写操作步骤

- #####  零开销模式（Zero-overhead mode）

  > 该模式下CORDIC运算效率最高，上一个结果被读取后即刻开始下一次运算，期间没有空闲时间；该模式中CPU的占用率为100%。

1. 配置CORDIC_CSR寄存器，也就是初始化和配置CORDIC；
2. 向CORDIC_WDATA寄存器写入参数，写入完成后CORDIC将会开始第一次计算；
3. 如果需要，为下一次计算重新配置CORDIC_CSR寄存器，此时不论上一个计算是否完成，重新配置CSR寄存器不会对上一次计算结果产生影响；
4. 向CORDIC_WDATA寄存器写入下一次计算所需参数；
5. 从CORDIC_RDATA寄存器读取上一次计算的结果，读取结果的操作完成后会触发下一次计算的开始；一旦开始计算，读取CORDIC_RDATA寄存器的操作都会插入AHB总线等待状态，直到计算结束才返回结果；因此，即使CORDIC未运算出结果，也可以进行读操作，当计算结果返回时读操作完成；
6. 重复第3至第6步骤；
7. 从CORDIC_RDATA寄存器读取最后一个结果，完成计算。

![批注 2020-03-27 144708](/批注 2020-03-27 144708.png)

<center>零开销模式示意图</center>



- ##### 轮询模式（Polling mode）

  > 该模式会轮询CORDIC_CSR寄存器的RRDY标志位以判断运算完成；该模式不会使CPU处于100%的占用率，使CPU可以处理其他任务；轮询模式会比零开销模式消耗稍长时间，因为计算完成后结果不会立即被读取，需要等待下一个轮询周期到来，且读取RRDY标志位后再读取结果会产生延迟。

1. 配置CORDIC_CSR寄存器，也就是初始化和配置CORDIC；
2. 向CORDIC_WDATA寄存器写入参数，写入完成后CORDIC将会开始第一次计算；
3. 如果需要，为下一次计算重新配置CORDIC_CSR寄存器，此时不论上一个计算是否完成，重新配置CSR寄存器不会对上一次计算结果产生影响；
4. 向CORDIC_WDATA寄存器写入下一次计算所需参数；
5. 轮询CORDIC_CSR寄存器的RRDY标志位，直到该位被置1；
6. RRDY标志位置1时，从CORDIC_RDATA寄存器读取上一次计算的结果，读取结果的操作完成后会触发下一次计算的开始；
7. 重复第3至第7步骤；
8. 从CORDIC_RDATA寄存器读取最后一个结果，完成计算。



- ##### 中断模式（Interrupt mode）

  > 当RRDY标志位被置1时产生中断信号，该位被置0时会清除中断标志位；该模式下使得结果的读取具有优先级；因此会比零开销模式和轮询模式消耗更长时间。

1. 配置CORDIC_CSR寄存器，也就是初始化和配置CORDIC，并且设置IEN位为1；可以直接在STM32CubeMX中设置为中断模式；
2. 向CORDIC_WDATA寄存器写入参数，写入完成后CORDIC将会开始第一次计算；
3. 如果需要，为下一次计算重新配置CORDIC_CSR寄存器，此时不论上一个计算是否完成，重新配置CSR寄存器不会对上一次计算结果产生影响；
4. 向CORDIC_WDATA寄存器写入下一次计算所需参数；
5. 当计算完成，RRDY位被置1，产生中断信号；在中断服务函数中读取CORDIC_RDATA寄存器上一次计算的结果，读取结果的操作完成后会触发下一次计算的开始；读取CORDIC_RDATA的操作会清除RRDY位和中断信号；；
6. 从CORDIC_RDATA寄存器读取最后一个结果，完成计算。



- ##### 直接存储器访问模式（DMA mode）

  > 该模式下CPU占用率为0%；DMA请求不能对CORDIC_CSR寄存器进行读写操作，因此DMA模式只适用于相同模式下的运算，比如使用相同的函数、相同的缩放因子、相同的迭代周期等；DMA模式下，数据的来源与输出目的地不一定是片上内存，可以是其他外设，比如DAC和ADC；

1. 配置CORDIC_CSR寄存器，也就是初始化和配置CORDIC；在STM32CubeMX中设置DMA模式；

2. 使用CORDIC_Calculate_DMA函数启动DMA运算，入口参数中配置数据源的地址以及输出地址，并且设置DMAWEN位和DMAREN位；

   <u>*注意事项：当DMAWEN置1时会产生DMA写请求，当DMAREN置1且RRDY置1时会产生DMA读请求，因此要暂停DMA读写只需将DMAWEN和DMAREN置0即可；在DMA模式运行中应避免对CORDIC_WDATA寄存器做写操作和CORDIC_RDATA寄存器做读操作，否则可能会产生DMA阻塞。*</u>



[关于在G4中使用CORDIC的详细内容，请查阅ST官方文档RM0440中424页-442页内容。](https://www.st.com/content/st_com/en/search.html#q=RM0440-t=resources-page=1)

https://www.st.com/content/st_com/en/search.html#q=RM0440-t=resources-page=1



### 3. 定点带符号整数数据格式（Q1.31,Q1.15）

#### 3.1 定义

在q1.31格式中，数字由一个符号位和31个小数位(二进制位)表示；数值范围是-1(0x80000000)到1 - 2^-

31(0 x7fffffff)；精度是2^-31(大约5 x 10^-10)。

![批注 2020-03-27 165016](/批注 2020-03-27 165016.png)
$$
Num=(-1)^s\sum_{k=0}^{30} \frac{1}{2^{31-k}}C_k
$$


在q1.15格式中，数字由一个符号位和15个小数位(二进制位)表示;数值范围是-1(0x8000)到1 - 2^-15(0 x7fff)；

![批注 2020-03-27 165036](/批注 2020-03-27 165036.png)
$$
Num=(-1)^s\sum_{k=0}^{14} \frac{1}{2^{15-k}}C_k
$$
这种格式的优点是可以将两个输入参数打包到一个32位的数据中，并且可以在一个32位的读操作中获取两个结果；但精度降低到2^-15(大约3 × 10^-5)。

![批注 2020-03-27 165051](/批注 2020-03-27 165051.png)



#### 3.2 带符号浮点格式的转换

Q1.31或Q1.15格式对开发者而言不够直观，以下提供四条函数，可将Q1.31和Q1.15格式转换成带符号浮点数。

**将带符号浮点数转换成Q1.31格式：**

```c
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
```



**将带符号浮点数转换成Q1.15格式：**

```c
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
```



**将Q1.31格式转换成带符号浮点数：**

```c
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
```



**将Q1.15格式转换成带符号浮点数：**

```c
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
```



### 4. 参考

1. Getting started with the CORDIC accelerator using STM32CubeG4 MCU Package - Application note（AN5325）
2. STM32G4xx advanced Arm-based 32-bit MCUs - Reference manual（RM0440）
3. en.STM32G4-Peripheral-Cordic_Coprocessor_CORDIC
4. 0x80000000为什么等于-2147483648和负数如何在内存上储存（https://blog.csdn.net/youyou362/article/details/72667951）



<center>转载请注明出处，万分感谢！</center>
