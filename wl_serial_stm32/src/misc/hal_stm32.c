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

//Assign value to _estack in LD script

uint8_t *g_stack_lowest = (uint8_t*)-1;

static void NVIC_Configration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 

    //DMA for uart
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    //uart RX irq
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    //NRF RX interrupt
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    //hal timer irq
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0xf;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_SetPriority(SysTick_IRQn, 0);
}


static void USART_Configration(void)
{
    //HAL_DEBUG_UART
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = HAL_DEFAULT_UART_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    /* USART configuration */
    USART_Init(HAL_DEBUG_UART, &USART_InitStructure);
    /* Enable USART */
    USART_Cmd(HAL_DEBUG_UART, ENABLE);

}

static void RCC_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//DMA1

	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);
}


static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure USART3 Tx as push-pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Configure USART3 Rx as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
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
    uint8_t tmp, *p = &tmp;
    __hal_time += 1000000 / systick_freq();
    if(p < g_stack_lowest)
        g_stack_lowest = p;
}

void _SysTick_Config()
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    SysTick_Config(systick_counter());
}


extern void __hal_timer_tick();
void hal_timer_interrupt_handler()
{
    __hal_timer_tick();
    TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);
}


void hal_timer_config()
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_DeInit(TIM3);

    TIM_TimeBaseStructure.TIM_Period = 1000;//ARR鐨勫�
    TIM_TimeBaseStructure.TIM_Prescaler = 72; //sysclk_freq() / 1000000; //Every 1ms
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //鍚戜笂璁℃暟妯″紡
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE); //寮�惎鏃堕挓

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
    
    hal_timer_config();

    NVIC_Configration();

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




