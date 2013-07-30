/**
 * @file bit_filed.h
 * @brief 
 * @author Red Li
 * @version 
 * @date 2013-06-29
 */


#ifndef BIT_FIELD_H
#define BIT_FIELD_H


#define BIT(n)                              (0x1 << (n))
#define BIT_MASK(n)                         (BIT(n) - 1)

//bit operation
#define BIT_SET(_v_, _pos_)                 ((_v_) |= (BIT(_pos_)))
#define BIT_CLR(_v_, _pos_)                 ((_v_) &= ~(BIT(_pos_)))
#define BIT_FLIP(_v_, _pos_)                ((_v_) ^= (BIT(_pos_)))
#define BIT_GET(_v_, _pos_)                 (((_v_) >> (_pos_)) & 0x1)

//bit fields operation
#define BF_MASK(s, l)                       (BIT_MASK(l) << (s))
#define BF_PREP(x, s, l)                    (((x) & BIT_MASK(l)) << (s))
#define BF_GET(_v_, _start_, _len_)         (((_v_) >> (_start_)) & BIT_MASK(_len_))
#define BF_SET(_v_, _v1_, _start_, _len_)   ((_v_) = (((_v_) &~ BF_MASK((_start_), (_len_))) | BF_PREP((_v1_), (_start_), (_len_)))) 
#define BFN_GET(_v_, _nm_)                  (BF_GET((_v_), _nm_##_START, _nm_##_LENGTH))
#define BFN_SET(_v_, _v1_, _nm_)            (BF_SET((_v_), (_v1_), _nm_##_START, _nm_##_LENGTH))


static inline int is_little_endian(void)
{
    union{
        int i;
        char c[sizeof(int)];
    }bint = {0x01020304};

    return bint.c[0] > bint.c[1];
}


#endif /* BIT_FIELD_H */
