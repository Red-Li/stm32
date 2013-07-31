#include <stdio.h>
#include <string.h>

#include "hal_stm32.h"
#include "utils.h"
#include "wl_serial.h"

int main(void)
{
    local_irq_disable();
    global_init();

    ASSERT(wls_init(WLS) == 0);

    local_irq_enable();
    


    while(1);
    return 0;
}
