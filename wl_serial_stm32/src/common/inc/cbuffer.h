/******************************************************************
 * Author: Red Li
 * Date: 2013-07-24
 * ****************************************************************
 */

#include <stdint.h>

#ifndef __CBUFFER_H__
#define __CBUFFER_H__

#ifndef CBUFFER_INLINE
#define CBUFFER_INLINE inline
#endif

#ifndef CBUFFER_MEM_BARRIER
#define CBUFFER_MEM_BARRIER
#endif

#ifndef CBUFFER_MEMCPY
#include <string.h>
#define CBUFFER_MEMCPY(dst, src, size) memcpy((dst), (src), (size))
#endif


#ifndef cbuffer_value_t
typedef uint8_t cbuffer_value_t;
#endif

#ifndef cbuffer_size_t
typedef uint32_t cbuffer_size_t;
#endif

typedef struct cbuffer_s{
    volatile cbuffer_size_t head, tail;
    cbuffer_size_t size;
    cbuffer_size_t gap;
    cbuffer_value_t *buffer;
}cbuffer_t;


#define CBUFFER(_name,  _size, _gap)\
    cbuffer_value_t __buffer_##_name[(_size)];\
    cbuffer_t _name = {\
        .head = 0,\
        .tail = 0,\
        .size = (_size) - 2 * (_gap),\
        .gap = (_gap),\
        .buffer = (__buffer_##_name) + (_gap)\
    }


#define CBUFFER_INIT(_cb, _buffer, _size, _gap) do{\
    (_cb)->tail = (_cb)->head = 0;\
    (_cb)->size = (_size) - 2 * (_gap);\
    (_cb)->gap = (_gap);\
    (_cb)->buffer = (_buffer) + (_cb)->gap;\
}while(0)


#define CBUFFER_EMPTY(_cb) ((_cb)->head == (_cb)->tail)
#define CBUFFER_FULL(_cb) \
    ((((_cb)->tail + (_cb)->gap + 1) % (_cb)->size) == (_cb)->head)

#define CBUFFER_DATA_LENGTH(_cb) (((_cb)->tail + (_cb)->size - (_cb)->head) % (_cb)->size)

#define CBUFFER_CFREE_LENGTH(_cb) \
    ((_cb)->tail < (_cb)->head ?\
        (_cb)->head - (_cb)->gap - (_cb)->tail - 1 :\
        (_cb)->head <= (_cb)->gap ? (_cb)->size - (_cb)->tail - 1 : (_cb)->size - (_cb)->tail)

#define CBUFFER_CDATA_LENGTH(_cb) ((_cb)->head < (_cb)->tail ? (_cb)->tail - (_cb)->head : (_cb)->size - (_cb)->head)

#define CBUFFER_OCCUPY(_cb, _size) ((_cb)->tail = ((_cb)->tail + (_size)) % (_cb)->size)
#define CBUFFER_FREE(_cb, _size) ((_cb)->head = ((_cb)->head + (_size)) % (_cb)->size)

#define CBUFFER_DATA(_cb) ((_cb)->buffer + (_cb)->head)


static CBUFFER_INLINE cbuffer_size_t CBUFFER_WRITE(
        cbuffer_t *cb,
        cbuffer_value_t *data,
        cbuffer_size_t size
        )
{
    cbuffer_size_t n2copy = size, ncopy, csize;

    while(!CBUFFER_FULL(cb) && n2copy > 0){
        //calc continous size
        do{
            csize = CBUFFER_CFREE_LENGTH(cb);
        }while(csize != CBUFFER_CFREE_LENGTH(cb));

        //calc how many data to copy
        ncopy = csize > n2copy ? n2copy : csize;
        CBUFFER_MEMCPY(cb->buffer + cb->tail, data, ncopy);
        
        //update context
        n2copy -= ncopy;
        data += ncopy;

        //Make sure all data have been written
        CBUFFER_MEM_BARRIER;

        CBUFFER_OCCUPY(cb, ncopy);

    }

    return size - n2copy;
}


static CBUFFER_INLINE cbuffer_size_t CBUFFER_READ(
        cbuffer_t *cb,
        cbuffer_value_t *buffer,
        cbuffer_size_t size)
{
    cbuffer_size_t n2copy = size, ncopy, csize;

    while(!CBUFFER_EMPTY(cb) && n2copy > 0){
        //calc continous size
        do{
            csize = CBUFFER_CDATA_LENGTH(cb);
        }while(csize != CBUFFER_CDATA_LENGTH(cb));

        //calc how many data to copy
        ncopy = csize > n2copy ? n2copy : csize;
        CBUFFER_MEMCPY(buffer, cb->buffer + cb->head, ncopy);
        CBUFFER_MEMCPY(buffer, cb->buffer + cb->head, ncopy);
        
        //update context
        n2copy -= ncopy;
        buffer += ncopy;
        
        //Make sure all data have been read
        CBUFFER_MEM_BARRIER;

        CBUFFER_FREE(cb, ncopy);
    }

    return size - n2copy;
}


#endif
