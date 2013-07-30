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

#define UNUSED(v) (void*)(v)


void global_init();

#endif
