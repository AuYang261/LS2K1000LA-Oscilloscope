#ifndef __SPI_H__
#define __SPI_H__

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

extern unsigned char *map_base;

#define MAP_SIZE 0x4000
#define MAP_BASE 0x1fff0000

#define SPI_BASE_OFFSET (0x1fff0220 - MAP_BASE)

#define SPCR (map_base + 0)
#define SPCR_SPIE (1 << 7)
#define SPCR_SPE (1 << 6)
#define SPCR_CPOL (1 << 3)
#define SPCR_CPHA (1 << 2)
#define SPCR_SPR_2 (1 << 1)
#define SPCR_SPR_1 (1)

#define SPSR (map_base + 1)

#define TXRX_FIFO (map_base + 2)

#define SPER (map_base + 3)
#define SPER_MODE (1 << 2)
#define SPER_SPRE_2 (1 << 1)
#define SPER_SPRE_1 (1)

#define SFC_PARAM (map_base + 4)

#define SFC_SOFTCS (map_base + 5)

#define SFC_TIMING (map_base + 6)

#define WFFULL (1 << 3)  // 发送缓存满
#define RFFULL (1 << 2)  // 读寄存器满
#define RFEMPTY 1

#define KSEG1_STORE8(addr, val) \
    (*(volatile char *)(addr) = val)
#define KSEG1_LOAD8(addr) \
    (*(volatile char *)(addr))

#define SET_SPI(addr, val) KSEG1_STORE8(addr, val)
#define GET_SPI(addr) KSEG1_LOAD8(addr)

int spi_init();
void spi_read(__uint16_t *buf, int len);
void spi_write(__uint16_t *buf, int len);

#endif
