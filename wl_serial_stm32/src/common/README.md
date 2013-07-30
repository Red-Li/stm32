
- bit_filed.h  
    - Desc: Bit and Bits filed operation
    - Porting: None

- buffer_fix.h, buffer_fix.c
    - Desc: Fix Circle Queue buffer implementation
    - Porting: None

- printf.h, printf.c
    - Desc: TFP printf impl
    - Porting: void (*putcf)(void *, char)

- hal_table.h
    - Desc: Save struct to specified section
    - Porting: Need change link script to add table section name

- logger.h
    - Desc: Debug and logger micro
    - Porting: define logger_printf before include this file

- dev.h, dev.c
    - Desc: Device abstract layer, depends on hal_table.h
    - Porting: None

- atomic.h
    - Desc: define aotmic operation and data type
    - Porting: Need define ARCH micro, current only support STM32

- list.h
    - Desc: List ops from linux kernel
    - Porting: None


