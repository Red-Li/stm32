/**
 * @file utils.h
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-06-09
 */


#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>
#include "printf.h"
#include "bit_field.h"
#define logger_printf tfp_printf
#include "logger.h"


#define UNUSED(x) 

//Flags operation
#define FLAG_SET(flags, flag) ((flags) |= (flag))
#define FLAG_CLR(flags, flag) ((flags) &= ~(flag))
#define FLAG_CHECK(flags, flag) ((flags) & (flag))


void global_init();







#endif
