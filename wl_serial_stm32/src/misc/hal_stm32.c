/**
 * @file hal_stm32.c
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-09
 */


#include <stdint.h>

#include "hal_stm32.h"

volatile static hal_time_t __hal_time = 0;

static void USART_Configration(void)
{
    //USART1
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    /* USART configuration */
    USART_Init(USART1, &USART_InitStructure);
    /* Enable USART */
    USART_Cmd(USART1, ENABLE);

}

static void RCC_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
}


static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure USART Tx as push-pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* Configure SPI2 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    //SCK, MISO, MOSI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    //NSS and CE
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    //Configure IRQ PIN
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//input
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    //Enable extern interrupt
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0); //interrupt 0 

}

hal_time_t hal_time()
{
    uint32_t elapse = systick_counter() - systick_current();
    hal_time_t t0;
    do{
        t0 = __hal_time;
    }while(t0 != __hal_time); //Trick

    return t0 + elapse / (sysclk_freq() / 1000000); 
}


static uint32_t s_rand_seed = 0;
void hal_srand(uint32_t seed)
{
    if(seed == (uint32_t)-1)
        s_rand_seed = hal_time();
    else
        s_rand_seed = seed;
}

//http://stackoverflow.com/questions/6275593/how-to-write-you-own-random-number-algorithm
uint32_t hal_rand()
{
    return s_rand_seed = (s_rand_seed * 214013UL + 2531011UL)
        % 2147483647UL;
}


void systick_interrupt_handler()
{
    __hal_time += 1000000 / systick_freq();

}

void _SysTick_Config()
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    SysTick_Config(systick_counter());
}


void hal_init(void) 
{
    SystemInit();
    
    SystemCoreClockUpdate();

    _SysTick_Config();
    //
    RCC_Configuration();

    GPIO_Configuration();

    USART_Configration();

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 
}

void udelay(uint32_t us)
{
    hal_time_t now = hal_time();
    while(hal_time() < now + us);
}

void mdelay(uint32_t ms)
{
    hal_time_t now = hal_time();
    while(hal_time() < now + 1000UL * ms);
}




