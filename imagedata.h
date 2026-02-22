#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <stdint.h>

struct imagedata {
        const uint8_t * bw;
        const uint8_t * r;
        };

extern struct imagedata imagedata_sample;

#else

#endif /* #ifndef IMAGEDATA_H */
