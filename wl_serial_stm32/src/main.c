#include <stdio.h>
#include <string.h>

#include "hal_stm32.h"
#include "utils.h"

int main(void)
{
    local_irq_disable();
    global_init();
    local_irq_enable();
    


    while(1);
    return 0;
}
