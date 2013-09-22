/******************************************************************************/
/* sram.c: 	library for the CY7C1071DV33 Static RAM on the MCBSTM32F200				*/
/*          development board																                  */
/******************************************************************************/
/* This file is based on the LCD driver in the uVision/ARM development tools. */
/******************************************************************************/

/* Usage: call SRAM_Init() before using any space allocated in the SRAM						*/
/* To manually place data in the SRAM, aim a pointer at mySRAM_BASE cast as the 	*/
/* appropriate pointer type. User is responsible for maintaining offsets.					*/
/* Example																																				*/
/*	#define ARRAY2_OFFSET 250*sizeof(int32_t)																			*/
/*	int32_t *array1 = (int32_t *)mySRAM_BASE;																			*/
/*	int16_t *array2 = (int16_t *)(mySRAM_BASE+ARRAY2_OFFSET);											*/
/*																																								*/
#include "sram.h"



void SRAM_Init(void)
{

	/* Configure the LCD Control pins --------------------------------------------*/
  RCC->AHB1ENR    |=((1UL <<  0) |      /* Enable GPIOA clock                 */
                     (1UL <<  3) |      /* Enable GPIOD clock                 */
                     (1UL <<  4) |      /* Enable GPIOE clock                 */
                     (1UL <<  5) |      /* Enable GPIOF clock                 */
                     (1UL <<  6));      /* Enable GPIOG clock                 */

  /* PD.00(IO2),  PD.01(IO3),  PD.04(N_OE), PD.05(N_WE)                       */
	/* PD.11(A16), PD.12(A17), PD.13(A18)																				*/
  /* PD.08(IO13), PD.09(IO14), PD.10(IO15), PD.14(IO0), PD.15(IO1)            */
  GPIOD->MODER    &= ~0xFFFF0F0F;       /* Clear Bits                         */
  GPIOD->MODER    |=  0xAAAA0A0A;       /* Alternate Function mode            */
  GPIOD->OSPEEDR  &= ~0xFFFF0F0F;       /* Clear Bits                         */
  GPIOD->OSPEEDR  |=  0xAAAA0A0A;       /* 50 MHz Fast speed                  */
  GPIOD->AFR[0]   &= ~0x00FF00FF;       /* Clear Bits                         */
  GPIOD->AFR[0]   |=  0x00CC00CC;       /* Alternate Function mode AF12       */
  GPIOD->AFR[1]   &= ~0xFFFFFFFF;       /* Clear Bits                         */
  GPIOD->AFR[1]   |=  0xCCCCCCCC;       /* Alternate Function mode AF12       */

  /* PE.00(N_BLE), PE.01(N_BHE), PE.03(A19), PE.04(A20)												*/
	/* PE.07(D4), PE.08(D5),  PE.09(D6),  PE.10(D7), PE.11(D8)                  */
  /* PE.12(D9), PE.13(D10), PE.14(D11), PE.15(D12)                            */
  GPIOE->MODER    &= ~0xFFFFC3CF;       /* Clear Bits                         */
  GPIOE->MODER    |=  0xAAAA828A;       /* Alternate Function mode            */
  GPIOE->OSPEEDR  &= ~0xFFFFC3CF;       /* Clear Bits                         */
  GPIOE->OSPEEDR  |=  0xAAAA828A;       /* 50 MHz Fast speed                  */
  GPIOE->AFR[0]   &= ~0xF00FF0FF;       /* Clear Bits                         */
  GPIOE->AFR[0]   |=  0xC00CC0CC;       /* Alternate Function mode AF12       */
  GPIOE->AFR[1]   &= ~0xFFFFFFFF;       /* Clear Bits                         */
  GPIOE->AFR[1]   |=  0xCCCCCCCC;       /* Alternate Function mode AF12       */

  /* PF.00(A0), PF.01(A1), PF.02(A2), PF.03(A3) , PF.04(A4), PF.05(A5)				*/ 
	/* PF.12(A6), PF.13(A7), PF.14(A8) , PF.15(A9)															*/ 
  GPIOF->MODER    &= ~0xFF000FFF;       /* Clear Bits                         */
  GPIOF->MODER    |=  0xAA000AAA;       /* Alternate Function mode            */
  GPIOF->OSPEEDR  &= ~0xFF000FFF;       /* Clear Bits                         */
  GPIOF->OSPEEDR  |=  0xAA000AAA;       /* 50 MHz Fast speed                  */
  GPIOF->AFR[0]   &= ~0x00FFFFFF;       /* Clear Bits                         */
	GPIOF->AFR[1]   &= ~0xFFF00000;       /* Clear Bits                         */
  GPIOF->AFR[0]   |=  0x00CCCCCC;       /* Alternate Function mode AF12       */
	GPIOF->AFR[1]   |=  0xCCC00000;       /* Alternate Function mode AF12       */


  /*  PG.00(A10), PG.01(A11), PG.02(A12), PG.03(A13), PG.04(A14), PG.05(A15)  */
	/*  PG.10(N_CE)																															*/
  GPIOG->MODER    &= ~0x00300FFF;       /* Clear Bits                         */
  GPIOG->MODER    |=  0x00200AAA;       /* Alternate Function mode            */
  GPIOG->OSPEEDR  &= ~0x00300FFF;       /* Clear Bits                         */
  GPIOG->OSPEEDR  |=  0x00200AAA;       /* 50 MHz Fast speed                  */
  GPIOG->AFR[0]   &= ~0x00FFFFFF;       /* Clear Bits                         */
  GPIOG->AFR[1]   &= ~0x00000F00;       /* Clear Bits                         */
  GPIOG->AFR[0]   |=  0x00CCCCCC;       /* Alternate Function mode AF12       */
	GPIOG->AFR[1]   |=  0x000C0C00;       /* Alternate Function mode AF12       */



	
/*-- FSMC Configuration ------------------------------------------------------*/
  RCC->AHB3ENR  |= (1UL << 0);          /* Enable FSMC clock                  */
                                 /* MCBSTM32F200 and MCBSTMF400 board  */
  FSMC_Bank1->BTCR[(3-1)*2 + 1] =       /* Bank3 NOR/SRAM timing register     */
                                        /* configuration                      */
                          (0 << 28) |   /* FSMC AccessMode A                  */
                          (0 << 24) |   /* Data Latency                       */
                          (0 << 20) |   /* CLK Division                       */
                          (0 << 16) |   /* Bus Turnaround Duration            */
                          (9 <<  8) |   /* Data SetUp Time                    */
                          (0 <<  4) |   /* Address Hold Time                  */
                          (1 <<  0);    /* Address SetUp Time                 */

                                /* MCBSTM32F200 and MCBSTMF400 board  */
  FSMC_Bank1->BTCR[(3-1)*2 + 0] =       /* Control register                   */
                          (0 << 19) |   /* Write burst disabled               */
                          (0 << 15) |   /* Async wait disabled                */
                          (0 << 14) |   /* Extended mode disabled             */
                          (0 << 13) |   /* NWAIT signal is disabled           */ 
                          (1 << 12) |   /* Write operation enabled            */
                          (0 << 11) |   /* NWAIT signal is active one data    */
                                        /* cycle before wait state            */
                          (0 << 10) |   /* Wrapped burst mode disabled        */
                          (0 <<  9) |   /* Wait signal polarity active low    */
                          (0 <<  8) |   /* Burst access mode disabled         */
                          (1 <<  4) |   /* Memory data  bus width is 16 bits  */
                          (0 <<  2) |   /* Memory type is SRAM                */
                          (0 <<  1) |   /* Address/Data Multiplexing disable  */
                          (1 <<  0);    /* Memory Bank enable                 */
	
	
	
}
