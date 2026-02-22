//
// NOTE: The display quality of thes LUT is not good, do not use it.
//

#include "lut.h"

#include <stdint.h>


/*
__code static const uint8_t kwr_vcom[] = {
        0x00,   80, 80, 80, 0,  1,  // ----
        0x00,   20, 20, 0,  0,  5,  // ----
        0x84,   10, 1,  10, 1,  10, // L-H-
        0x00,   20, 1,  20, 5,  5,  // ----
        0x00,   6,  2,  35, 20, 5,  // ----
        0x00,   5,  2,  30, 20, 10, // ----
        0x00,   10, 10, 0,  0,  1,  // ----
        0x00,   0,
        };
__code static const uint8_t kwr_ww[] = {
        0x90,   80, 80, 80, 0,  1,  // LH--
        0x40,   20, 20, 0,  0,  5,  // H---
        0x48,   10, 1,  10, 1,  10, // H-L-
        0x80,   20, 1,  20, 5,  5,  // L---
        0x00,   6,  2,  35, 20, 5,  // ----
        0x00,   5,  2,  30, 20, 10, // ----
        0x00,   10, 10, 0,  0,  1,  // ----
        };
__code static const uint8_t kwr_bw[] = {
        0x88,   80, 80, 80, 0,  1,  // L-L-
        0x80,   20, 20, 0,  0,  5,  // L---
        0x48,   10, 1,  10, 1,  10, // H-L-
        0x06,   20, 1,  20, 5,  5,  // --HL
        0x8C,   6,  2,  35, 20, 5,  // L-h-
        0x8C,   5,  2,  30, 20, 10, // L-h-
        0xF0,   10, 10, 0,  0,  1,  // hh--
        };
__code static const uint8_t kwr_wb[] = {
        0x90,   80, 80, 80, 0,  1,  // LH--
        0x40,   20, 20, 0,  0,  5,  // H---
        0x48,   10, 1,  10, 1,  10, // H-L-
        0x80,   20, 1,  20, 5,  5,  // L---
        0x00,   6,  2,  35, 20, 5,  // ----
        0x00,   5,  2,  30, 20, 10, // ----
        0x00,   10, 10, 0,  0,  1,  // ----
        };
__code static const uint8_t kwr_bb[] = {
        0x90,   80, 80, 80, 0,  1,  // LH--
        0x20,   20, 20, 0,  0,  5,  // -L--
        0x48,   10, 1,  10, 1,  10, // H-L-
        0x04,   20, 1,  20, 5,  5,  // --H-
        0x00,   6,  2,  35, 20, 5,  // ----
        0x00,   5,  2,  30, 20, 10, // ----
        0x00,   10, 10, 0,  0,  1,  // ----
        };

struct lut_pack lut_kwr = {
        .vcom = {
                .data = kwr_vcom,
                .size = sizeof(kwr_vcom),
                },
        .ww = {
                .data = kwr_ww,
                .size = sizeof(kwr_ww),
                },
        .bw = {
                .data = kwr_bw,
                .size = sizeof(kwr_bw),
                },
        .wb = {
                .data = kwr_wb,
                .size = sizeof(kwr_wb),
                },
        .bb = {
                .data = kwr_bb,
                .size = sizeof(kwr_bb),
                },
        };

__code static const uint8_t kw_vcom[] = {
        0x00,   64, 64, 0,  0,  1,  // ----
        0x00,   16, 16, 0,  0,  8,  // ----
        0x00,   8,  1,  8,  1,  8,  // ----
        0x00,   16, 16, 0,  0,  8,  // ----
        0x00,   0,  0,  0,  0,  0,  // ----
        0x00,   0,  0,  0,  0,  0,  // ----
        0x00,   0,  0,  0,  0,  0,  // ----
        0x00,   0,
        };
__code static const uint8_t kw_ww[] = {
        0x90,   64, 64, 0,  0,  1,  // LH--
        0x40,   16, 16, 0,  0,  8,  // H---
        0x48,   8,  1,  8,  1,  8,  // H-L-
        0x80,   16, 16, 0,  0,  8,  // L---
        0x00,   0,  0,  0,  0,  0,  // ----
        0x00,   0,  0,  0,  0,  0,  // ----
        0x00,   0,  0,  0,  0,  0,  // ----
        };
__code static const uint8_t kw_bw[] = {
        0x90,   64, 64, 0,  0,  1,  // LH--
        0x40,   16, 16, 0,  0,  8,  // H---
        0x48,   8,  1,  8,  1,  8,  // H-L-
        0x80,   16, 16, 0,  0,  8,  // L---
        0x00,   0,  0,  0,  0,  0,  // ----
        0x00,   0,  0,  0,  0,  0,  // ----
        0x00,   0,  0,  0,  0,  0,  // ----
        };
__code static const uint8_t kw_wb[] = {
        0x90,   64, 64, 0,  0,  1,  // LH--
        0x20,   16, 16, 0,  0,  8,  // -L--
        0x48,   8,  1,  8,  1,  8,  // H-L-
        0x10,   16, 16, 0,  0,  8,  // -H--
        0x00,   0,  0,  0,  0,  0,  // ----
        0x00,   0,  0,  0,  0,  0,  // ----
        0x00,   0,  0,  0,  0,  0,  // ----
        };
__code static const uint8_t kw_bb[] = {
        0x90,   64, 64, 0,  0,  1,  // LH--
        0x20,   16, 16, 0,  0,  8,  // -L--
        0x48,   8,  1,  8,  1,  8,  // H-L-
        0x10,   16, 16, 0,  0,  8,  // -H--
        0x00,   0,  0,  0,  0,  0,  // ----
        0x00,   0,  0,  0,  0,  0,  // ----
        0x00,   0,  0,  0,  0,  0,  // ----
        };

struct lut_pack lut_kw = {
        .vcom = {
                .data = kw_vcom,
                .size = sizeof(kw_vcom),
                },
        .ww = {
                .data = kw_ww,
                .size = sizeof(kw_ww),
                },
        .bw = {
                .data = kw_bw,
                .size = sizeof(kw_bw),
                },
        .wb = {
                .data = kw_wb,
                .size = sizeof(kw_wb),
                },
        .bb = {
                .data = kw_bb,
                .size = sizeof(kw_bb),
                },
        };
*/

//
// Dummy data.
//
struct lut_pack lut_kwr = {
        .vcom = {
                .data = {0},
                .size = 1,
                },
        .ww = {
                .data = {0},
                .size = 1,
                },
        .bw = {
                .data = {0},
                .size = 1,
                },
        .wb = {
                .data = {0},
                .size = 1,
                },
        .bb = {
                .data = {0},
                .size = 1,
                },
        };

struct lut_pack lut_kw = {
        .vcom = {
                .data = {0},
                .size = 1,
                },
        .ww = {
                .data = {0},
                .size = 1,
                },
        .bw = {
                .data = {0},
                .size = 1,
                },
        .wb = {
                .data = {0},
                .size = 1,
                },
        .bb = {
                .data = {0},
                .size = 1,
                },
        };
