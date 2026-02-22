#ifndef _LUT_H_
#define _LUT_H_

#include <stdint.h>

struct lut {
        const uint8_t * data;
        uint8_t size;
        };

struct lut_pack {
        struct lut vcom;
        struct lut ww;
        struct lut bw;
        struct lut wb;
        struct lut bb;
        };

extern struct lut_pack lut_kwr;
extern struct lut_pack lut_kw;

#endif /* #ifndef _LUT_H_ */
