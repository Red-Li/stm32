/**
 * @file atomic.h
 * @brief 
 * @author Li Hong H
 * @version 
 * @date 2013-06-09
 */

#ifndef ATOMIC_H
#define ATOMIC_H

#ifdef PLT_STM32

typedef int atomic_t;

#else
#error "Unspported platform"
#endif

#endif

