/**
 * @file logger.h
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-06-09
 */


#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdlib.h>

#define abort(code) exit(code)

/************* LOGGER *******************/
#define PRINT(fmt, ...) logger_printf(fmt "\n", ##__VA_ARGS__)
#define LOG(_label, fmt, ...) PRINT("[" _label "] " fmt, ##__VA_ARGS__) 
#define LOG_IF(_cond, _label, fmt, ...) do{\
    if(_cond) LOG(_label, fmt, ##__VA_ARGS__);\
}while(0)

#ifdef DEBUG
#define DBG(fmt, ...) LOG("DBG", fmt, ##__VA_ARGS__)
#define DBG_IF(_cond, fmt, ...) LOG_IF((_cond), "DBG", fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...) 
#define DBG_IF(_cond, fmt, ...) 
#endif
#define INF(fmt, ...) LOG("INF", fmt, ##__VA_ARGS__)
#define INF_IF(_cond, fmt, ...) LOG_IF((_cond), "INF", fmt, ##__VA_ARGS__)
#define WRN(fmt, ...) LOG("WRN", fmt, ##__VA_ARGS__)
#define WRN_IF(_cond, fmt, ...) LOG_IF((_cond), "WRN", fmt, ##__VA_ARGS__)
#define ERR(fmt, ...) LOG("ERR", fmt, ##__VA_ARGS__)
#define ERR_IF(_cond, fmt, ...) LOG_IF((_cond), "ERR", fmt, ##__VA_ARGS__)
#define FTL(fmt, ...) (LOG("FTL", fmt, ##__VA_ARGS__), abort(1))
#define FTL_IF(_cond, fmt, ...) while((_cond)){\
    LOG_IF((_cond), "FTL", fmt, ##__VA_ARGS__);\
    abort(1);\
}

#define ASSERT_MSG(_cond, _fmt, ...) \
    FTL_IF(!(_cond), "\"" #_cond "\" Assert Failed! " __FILE__ ":%d"  " \"" _fmt " \"", __LINE__, ##__VA_ARGS__)
#define ASSERT(_cond) ASSERT_MSG((_cond), "");



#endif
