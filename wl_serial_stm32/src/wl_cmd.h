/**
 * @file wl_cmd.h
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-07-18
 */


#ifndef WL_CMD_H
#define WL_CMD_H
#include "utils.h"

#define WL_CMD_BUF_SIZE 32

typedef int (*wl_cmd_callback_t)(void *, uint8_t *, uint8_t);

void wl_cmd_set_handle_remote_cmd(wl_cmd_callback_t handle_cmd, void *data);
void wl_cmd_set_handle_return(wl_cmd_callback_t handle_return, void *data);
int wl_cmd_recv(uint8_t *data, uint8_t len);
int wl_cmd_reset();


#endif //

