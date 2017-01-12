/**
 * @file      common.hpp
 * @brief     Provides backwards kernel compatibility and several other common data types
 * @author    Lukáš Ručka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * @date      2011-03-29 00:28 UTC+2
 * @copyright BSD
 */

#ifndef MWTOUCH_COMMON_HPP
#define MWTOUCH_COMMON_HPP

#include <linux/input.h>
#include <list>
#include <vector>
#include <stdint.h>
#include <limits>

// compatibility hacks based on 2.6.38's input.h
#ifndef ABS_MT_SLOT
    #define ABS_MT_SLOT     0x2f    /* MT slot being modified */
#endif

#ifndef ABS_MT_DISTANCE
    #define ABS_MT_DISTANCE     0x3b    /* Contact hover distance */
#endif

#ifndef ABS_CNT
    #error "input.h seem to be missing or damaged"
#endif

// compatibility and incomplete input.h hack
#ifndef SYN_DROPPED
    #define SYN_DROPPED 3
#endif

#if ABS_CNT <= (ABS_MT_DISTATANCE + 1)
    #undef ABS_CNT
    #define ABS_CNT (ABS_MT_DISTANCE + 1)
#endif

typedef uint16_t ev_code_t;
typedef int32_t ev_value_t;
typedef uint16_t priority_t;
typedef int32_t session_t;


static unsigned int const DIMENSIONS = 3;
static ev_code_t const MAPPING_IGNORE_CODE = -1;

inline bool bitfield_test_bit(int bit, const uint8_t * data){
    int byte_offset = bit/8;
    int bit_offset = bit%8;
    const uint8_t mask = (1 << bit_offset);
    return ((data[byte_offset] & mask) == mask);
}

inline int timeval_delta(struct timeval & tv1, struct timeval & tv2){
    int d = ((tv2.tv_sec - tv1.tv_sec)*1000000)+(tv2.tv_usec - tv1.tv_usec);
    if (d < 0){ d = -d; }
    return d;
}




#endif // MWTOUCH_COMMON_HPP
