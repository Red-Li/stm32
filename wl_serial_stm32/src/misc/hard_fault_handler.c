// From Joseph Yiu, minor edits by FVH
// hard fault handler in C,
// with stack frame location as input parameter
// called from HardFault_Handler in file xxx.s

#ifdef DEBUG

#include "utils.h"
#include "hal_stm32.h"
void hard_fault_handler_c (unsigned int * hardfault_args)
{
  unsigned int stacked_r0;
  unsigned int stacked_r1;
  unsigned int stacked_r2;
  unsigned int stacked_r3;
  unsigned int stacked_r12;
  unsigned int stacked_lr;
  unsigned int stacked_pc;
  unsigned int stacked_psr;
 
  stacked_r0 = ((unsigned long) hardfault_args[0]);
  stacked_r1 = ((unsigned long) hardfault_args[1]);
  stacked_r2 = ((unsigned long) hardfault_args[2]);
  stacked_r3 = ((unsigned long) hardfault_args[3]);
 
  stacked_r12 = ((unsigned long) hardfault_args[4]);
  stacked_lr = ((unsigned long) hardfault_args[5]);
  stacked_pc = ((unsigned long) hardfault_args[6]);
  stacked_psr = ((unsigned long) hardfault_args[7]);
 
  tfp_printf ("\n\n[Hard fault handler - all numbers in hex]\n");
  tfp_printf ("R0 = %x\n", stacked_r0);
  tfp_printf ("R1 = %x\n", stacked_r1);
  tfp_printf ("R2 = %x\n", stacked_r2);
  tfp_printf ("R3 = %x\n", stacked_r3);
  tfp_printf ("R12 = %x\n", stacked_r12);
  tfp_printf ("LR [R14] = %x  subroutine call return address\n", stacked_lr);
  tfp_printf ("PC [R15] = %x  program counter\n", stacked_pc);
  tfp_printf ("PSR = %x\n", stacked_psr);
  tfp_printf ("BFAR = %x\n", (*((volatile unsigned long *)(0xE000ED38))));
  tfp_printf ("CFSR = %x\n", (*((volatile unsigned long *)(0xE000ED28))));
  tfp_printf ("HFSR = %x\n", (*((volatile unsigned long *)(0xE000ED2C))));
  tfp_printf ("DFSR = %x\n", (*((volatile unsigned long *)(0xE000ED30))));
  tfp_printf ("AFSR = %x\n", (*((volatile unsigned long *)(0xE000ED3C))));
  tfp_printf ("SCB_SHCSR = %x\n", SCB->SHCSR);
 
  while (1);
}


#else

void hard_fault_handler_c (unsigned int * hardfault_args)
{
	while(1);
}

#endif
