# Description:
#     8051 MCU project makefile.
#     Code main source file with sdcc C syntax.
#
# Depend:
#     sdcc 4.2.0 #13081 (Mac OS X x86_64)
#         sdas8051 V02.00 + NoICE + SDCC mods  (Intel 8051)
#         sdld V03.00 + NoICE + sdld
#         packihx unknown
#         makebin unknown
#
#     stcgal 1.10
#
# Author:
#     cddwx525 <cddwx525@163.com>
#
# Changelog:
#     * 2025-11-18
#         * Modify: Add BUILD_DIR target.
#         * Fix: Header file not add
#     * 2025-11-08
#         * Add: Add command to use Raspberry Pi to flash CC2530.
#     * 2025-11-06
#         * Modify: Style according stm32 C makefile.
#         * Modify: Use main sdcc to call linker.
#         * Doc: Change sdcc version from "Mac OS ppc" to "Mac OS X x86_64".
#                 Before, macports sdcc 4.2.0, now manual download
#                 4.2.0 release for Mac OS X x86_64.
#     * 2024-10-05
#         * Update: Add xxx_SIZE and xxx_LOC
#     * 2024-10-05
#         * Done

.POSIX:
.SUFFIXES:

################################################################################
#
# Values
#
################################################################################
TARGET = esl

PORT = /dev/tty.usbserial-2130
REMOTE_HOST = pi@192.168.10.7
REMOTE_DIR = tmp/esl

BUILD_DIR = build
SDK_DIR = sdk


C_INCLUDES_SDK =
C_INCLUDES =
C_INCLUDES += .
C_INCLUDES += $(C_INCLUDES_SDK)

AS_INCLUDES_SDK =
AS_INCLUDES =


CC = sdcc
AS = sdas8051
LD = sdld

OPT =

IRAM_SIZE   = 0x0100  # data + idata: 256 B
XRAM_SIZE   = 0x2000  # xdata: 8 KB SRAM
#CODE_SIZE   = 0x10000  # code: 64 KB
CODE_SIZE   = 0x10000  # code: 64 KB
STACK_SIZE  = 0x0080  # 128 B

DATA_LOC    = 0x30
BIT_LOC     = 0x20
IDATA_LOC   = 0x80
XRAM_LOC    = 0x0000
CODE_LOC    = 0x0000
STACK_LOC   = 0x80
XSTACK_LOC  = 0x0000



#CPU = -mmcs51 --model-small
CPU = -mmcs51 --model-large
FPU =
FLOAT-ABI =
MCU = $(CPU)

C_DEFS =
CFLAGS = -c \
        --std-c99 \
        $(OPT) \
        $(MCU) \
        $(C_DEFS) \
        $(C_INCLUDES:%=-I%)

#--xstack \
#--stack-auto \

AS_DEFS =
ASFLAGS = -c \
        $(OPT) \
        $(MCU) \
        $(AS_DEFS) \
        $(AS_INCLUDES)

LIBS =
LIBDIR =
LDFLAGS = $(MCU) \
        -Wl "$(LIBDIR:%=-k %)" \
        -Wl "$(LIBS:%=-l %)" \
        -Wl "-I $(IRAM_SIZE)" \
        -Wl "-X $(XRAM_SIZE)" \
        -Wl "-C $(CODE_SIZE)" \
        -Wl "-b BSEG=$(BIT_LOC)" \
        -Wl "-b XSEG=$(XRAM_LOC)" \
        -Wl "-b PSEG=$(XSTACK_LOC)" \
        -Wl "-b HOME=$(CODE_LOC)"


        #
        # Use -Wl.
        #
        ## NOTE:
        ##     1. When use sdcc --code-loc, HOME is set instead of CSEG
        ##             , -b CSEG not take effect.
        ##     2. Use -b SSEG to set stack loction is not working
        ##             , stack loction is always at ISEG loction.
        ##     3. When not use "-b DSEG $(DATA_LOC)", sdcc do it.
        ##     4. When not use "-b ISEG $(IDATA_LOC)", sdcc do it.
        ##     5. When not use "-b SSEG $(STACK_LOC)", sdcc do it.
        ##     6. When not use "-S $(STACK_SIZE)", sdcc do it.
        ##
        #-Wl "-b DSEG=$(DATA_LOC)" \
        #-Wl "-b ISEG=$(IDATA_LOC)" \
        #-Wl "-S $(STACK_SIZE)" \
        #-Wl "-b SSEG=$(STACK_LOC)" \


        #
        # Use sdcc option.
        #
        ## NOTE:
        ##     1. SDCC say "deprecated compiler option '--stack-loc'".
        ##     2. --bit-loc Not implemented, use -Wl.
        ##     3. When not use "--data-loc $(DATA_LOC)", sdcc do it.
        ##     4. When not use "--idata-loc $(IDATA_LOC)", sdcc do it.
        ##     5. When not use "--stack-size $(STACK_SIZE)", sdcc do it.
        ##
        #$(LIBDIR:%=-L %) \
        #--iram-size $(IRAM_SIZE) \
        #--xram-size $(XRAM_SIZE) \
        #--stack-size $(STACK_SIZE) \
        #--code-size $(CODE_SIZE) \
        #-Wl "-b BSEG=$(BIT_LOC)" \
        #--data-loc $(DATA_LOC) \
        #--idata-loc $(IDATA_LOC) \
        #--stack-loc $(STACK_LOC) \
        #--xram-loc $(XRAM_LOC) \
        ##--xstack-loc $(XSTACK_LOC) \
        #--code-loc $(CODE_LOC)



################################################################################
#
# Targets
#
################################################################################
.PHONY: all ihx hex bin flash flashbin clean debugger_shutdown

all: $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/fonts:
	mkdir -p $@


$(BUILD_DIR)/cc2530_ioctl.rel: $(BUILD_DIR)
$(BUILD_DIR)/cc2530_ioctl.rel: cc2530_ioctl.c cc2530_ioctl.h
	$(CC) $(CFLAGS) -o $@ cc2530_ioctl.c

$(BUILD_DIR)/uart.rel: $(BUILD_DIR)
$(BUILD_DIR)/uart.rel: uart.c uart.h
	$(CC) $(CFLAGS) -o $@ uart.c

$(BUILD_DIR)/delay.rel: $(BUILD_DIR)
$(BUILD_DIR)/delay.rel: delay.c delay.h
	$(CC) $(CFLAGS) -o $@ delay.c

$(BUILD_DIR)/string.rel: $(BUILD_DIR)
$(BUILD_DIR)/string.rel: string.c string.h
	$(CC) $(CFLAGS) -o $@ string.c

$(BUILD_DIR)/buffer.rel: $(BUILD_DIR)
$(BUILD_DIR)/buffer.rel: $(BUILD_DIR)/fonts/font_en.rel
$(BUILD_DIR)/buffer.rel: $(BUILD_DIR)/fonts/font_zh.rel
$(BUILD_DIR)/buffer.rel: buffer.c buffer.h
	$(CC) $(CFLAGS) -o $@ buffer.c

$(BUILD_DIR)/lut.rel: $(BUILD_DIR)
$(BUILD_DIR)/lut.rel: lut.c lut.h
	$(CC) $(CFLAGS) -o $@ lut.c

$(BUILD_DIR)/epd.rel: $(BUILD_DIR)
$(BUILD_DIR)/epd.rel: $(BUILD_DIR)/delay.rel
$(BUILD_DIR)/epd.rel: $(BUILD_DIR)/string.rel
$(BUILD_DIR)/epd.rel: $(BUILD_DIR)/cc2530_ioctl.rel
$(BUILD_DIR)/epd.rel: $(BUILD_DIR)/uart.rel
$(BUILD_DIR)/epd.rel: $(BUILD_DIR)/buffer.rel
$(BUILD_DIR)/epd.rel: $(BUILD_DIR)/lut.rel
$(BUILD_DIR)/epd.rel: epd.c epd.h
	$(CC) $(CFLAGS) -o $@ epd.c

$(BUILD_DIR)/fonts/font_en.rel: $(BUILD_DIR)/fonts
$(BUILD_DIR)/fonts/font_en.rel: fonts/font_en.c fonts/font_en.h fonts/fonts.h
	$(CC) $(CFLAGS) -o $@ fonts/font_en.c

$(BUILD_DIR)/fonts/font_zh.rel: $(BUILD_DIR)/fonts
$(BUILD_DIR)/fonts/font_zh.rel: fonts/font_zh.c fonts/font_zh.h fonts/fonts.h
	$(CC) $(CFLAGS) -o $@ fonts/font_zh.c

$(BUILD_DIR)/fonts/font8.rel: $(BUILD_DIR)/fonts
$(BUILD_DIR)/fonts/font8.rel: $(BUILD_DIR)/fonts/font_en.rel
$(BUILD_DIR)/fonts/font8.rel: fonts/font8.c
	$(CC) $(CFLAGS) -o $@ fonts/font8.c

$(BUILD_DIR)/fonts/font12.rel: $(BUILD_DIR)/fonts
$(BUILD_DIR)/fonts/font12.rel: $(BUILD_DIR)/fonts/font_en.rel
$(BUILD_DIR)/fonts/font12.rel: fonts/font12.c
	$(CC) $(CFLAGS) -o $@ fonts/font12.c

$(BUILD_DIR)/fonts/font16.rel: $(BUILD_DIR)/fonts
$(BUILD_DIR)/fonts/font16.rel: $(BUILD_DIR)/fonts/font_en.rel
$(BUILD_DIR)/fonts/font16.rel: fonts/font16.c
	$(CC) $(CFLAGS) -o $@ fonts/font16.c

$(BUILD_DIR)/fonts/font20.rel: $(BUILD_DIR)/fonts
$(BUILD_DIR)/fonts/font20.rel: $(BUILD_DIR)/fonts/font_en.rel
$(BUILD_DIR)/fonts/font20.rel: fonts/font20.c
	$(CC) $(CFLAGS) -o $@ fonts/font20.c

$(BUILD_DIR)/fonts/font24.rel: $(BUILD_DIR)/fonts
$(BUILD_DIR)/fonts/font24.rel: $(BUILD_DIR)/fonts/font_en.rel
$(BUILD_DIR)/fonts/font24.rel: fonts/font24.c
	$(CC) $(CFLAGS) -o $@ fonts/font24.c

$(BUILD_DIR)/fonts/font12zh.rel: $(BUILD_DIR)/fonts
$(BUILD_DIR)/fonts/font12zh.rel: $(BUILD_DIR)/fonts/font_zh.rel
$(BUILD_DIR)/fonts/font12zh.rel: fonts/font12zh.c
	$(CC) $(CFLAGS) -o $@ fonts/font12zh.c

$(BUILD_DIR)/fonts/font24zh.rel: $(BUILD_DIR)/fonts
$(BUILD_DIR)/fonts/font24zh.rel: $(BUILD_DIR)/fonts/font_zh.rel
$(BUILD_DIR)/fonts/font24zh.rel: fonts/font24zh.c
	$(CC) $(CFLAGS) -o $@ fonts/font24zh.c

$(BUILD_DIR)/imagedata.rel: $(BUILD_DIR)
$(BUILD_DIR)/imagedata.rel: $(BUILD_DIR)/epd.rel
$(BUILD_DIR)/imagedata.rel: imagedata.c imagedata.h
	$(CC) $(CFLAGS) -o $@ imagedata.c

$(BUILD_DIR)/main.rel: $(BUILD_DIR)
$(BUILD_DIR)/main.rel: $(BUILD_DIR)/cc2530_ioctl.rel
$(BUILD_DIR)/main.rel: $(BUILD_DIR)/uart.rel
$(BUILD_DIR)/main.rel: $(BUILD_DIR)/delay.rel
$(BUILD_DIR)/main.rel: $(BUILD_DIR)/string.rel
$(BUILD_DIR)/main.rel: $(BUILD_DIR)/epd.rel
#$(BUILD_DIR)/main.rel: $(BUILD_DIR)/imagedata.rel
$(BUILD_DIR)/main.rel: main.c
	$(CC) $(CFLAGS) -o $@ main.c

$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/cc2530_ioctl.rel
$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/uart.rel
$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/delay.rel
$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/string.rel
$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/buffer.rel
$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/lut.rel
$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/epd.rel
$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/fonts/font_en.rel
$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/fonts/font_zh.rel
#$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/fonts/font8.rel
$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/fonts/font12.rel
#$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/fonts/font16.rel
#$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/fonts/font20.rel
#$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/fonts/font24.rel
$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/fonts/font12zh.rel
#$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/fonts/font24zh.rel
#$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/imagedata.rel
$(BUILD_DIR)/$(TARGET).ihx: $(BUILD_DIR)/main.rel
	$(CC) $(LDFLAGS) -o $@ $^
	#$(LD) -n -i $^ $@
	cat $(BUILD_DIR)/$(TARGET).mem

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.ihx
	packihx $^ > $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.ihx
	makebin -p -s $(CODE_SIZE) $^ $@

flash: $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	#stcgal -b 115200 -p $(PORT) $(TARGET).hex
	scp $(BUILD_DIR)/$(TARGET).hex $(REMOTE_HOST):$(REMOTE_DIR) \
			&& ssh $(REMOTE_HOST) \
					". ~/.profile && cd $(REMOTE_DIR) \
					&& cc_erase -r 1 -c 0 -d 2 \
					&& cc_write -r 1 -c 0 -d 2 $(TARGET).hex"

flashbin: $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	#stcgal -b 115200 -p $(PORT) $(TARGET).bin

debugger_shutdown:
	ssh $(REMOTE_HOST) "sudo shutdown now"


clean:
	rm -rf $(BUILD_DIR)


