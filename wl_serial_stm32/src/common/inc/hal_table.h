/**
 * @file hal_table.h
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-06-09
 */


#ifndef HAL_TABLE_H
#define HAL_TABLE_H


#define __xstring(_name) #_name

#define HAL_ATTR_USED __attribute__((unused))

#define HAL_ATTR_SECTION(__sect__)\
    __attribute__((section(__sect__)))

#define HAL_TABLE_ENTRY(_name)\
    HAL_ATTR_SECTION(".table." __xstring(_name) ".data")\
    HAL_ATTR_USED

#endif
