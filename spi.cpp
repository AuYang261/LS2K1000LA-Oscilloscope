#include "spi.h"

static unsigned char* map_base;

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

#define KSEG1_STORE8(addr, val) (*(volatile char*)(addr) = val)
#define KSEG1_LOAD8(addr) (*(volatile char*)(addr))

#define SET_SPI(addr, val) KSEG1_STORE8(addr, val)
#define GET_SPI(addr) KSEG1_LOAD8(addr)

int spi_init() {
#ifndef _WIN32
    printf("%s\n", __FUNCTION__);
    int dev_fd;
    dev_fd = open("/dev/mem", O_RDWR | O_SYNC);

    if (dev_fd < 0) {
        printf("open(/dev/mem) failed.\n");
        return -1;
    }

    map_base = (unsigned char*)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, dev_fd, MAP_BASE);

    map_base += SPI_BASE_OFFSET;

    *SPCR &= ~SPCR_SPE;  // clr

    *SPSR = 0xc0;  // 8'b1100_0000

    // 原始时钟频率为100MHz
    // 分频系数为16
    // SPRE_2 SPRE_1 SPR_2 SPR_1
    uint16_t time_ef = 0x0010;
    if (time_ef & 1) {
        *SPCR |= SPCR_SPR_1;
    } else {
        *SPCR &= ~SPCR_SPR_1;
    }
    if ((time_ef >> 4) & 1) {
        *SPCR |= SPCR_SPR_2;
    } else {
        *SPCR &= ~SPCR_SPR_2;
    }
    if ((time_ef >> 8) & 1) {
        *SPER |= SPER_SPRE_1;
    } else {
        *SPER &= ~SPER_SPRE_1;
    }
    if ((time_ef >> 12) & 1) {
        *SPER |= SPER_SPRE_2;
    } else {
        *SPER &= ~SPER_SPRE_2;
    }

    *SPCR &= ~SPCR_CPOL;  // 时钟极性
    *SPCR &= ~SPCR_CPHA;  // 时钟相位
    *SPER |= SPER_MODE;   // 1模式,标准模式

    *SPCR &= ~SPCR_SPIE;  // disable interuption

    *SPCR |= SPCR_SPE;  // set
    // 取消片选所有
    SET_SPI(SFC_SOFTCS, 0xff);
    printf("%s done\n", __FUNCTION__);
#endif
    return 0;
}

static __uint32_t spi_send_4bytes(__uint32_t val) {
#ifndef _WIN32
    // 片选CSN1
    SET_SPI(SFC_SOFTCS, 0x02);

    while ((GET_SPI(SPSR)) & WFFULL)
        ;
    SET_SPI(TXRX_FIFO, (__uint8_t)((val >> 24) & 0xff));
    while ((GET_SPI(SPSR)) & WFFULL)
        ;
    SET_SPI(TXRX_FIFO, (__uint8_t)((val >> 16) & 0xff));
    // 读
    while ((GET_SPI(SPSR) & RFEMPTY))
        ;  // 等待读寄存器满
    __uint8_t ret1 = GET_SPI(TXRX_FIFO);
    while ((GET_SPI(SPSR) & RFEMPTY))
        ;
    __uint8_t ret2 = GET_SPI(TXRX_FIFO);

    SET_SPI(SFC_SOFTCS, 0x22);
    SET_SPI(SFC_SOFTCS, 0x02);

    while ((GET_SPI(SPSR)) & WFFULL)
        ;
    SET_SPI(TXRX_FIFO, (__uint8_t)((val >> 8) & 0xff));
    while ((GET_SPI(SPSR)) & WFFULL)
        ;  // 发送缓存满，等待
    SET_SPI(TXRX_FIFO, (__uint8_t)(val & 0xff));

    while ((GET_SPI(SPSR) & RFEMPTY))
        ;
    __uint8_t ret3 = GET_SPI(TXRX_FIFO);
    while ((GET_SPI(SPSR) & RFEMPTY))
        ;
    __uint8_t ret4 = GET_SPI(TXRX_FIFO);

    // 取消片选CSN1
    SET_SPI(SFC_SOFTCS, 0x22);

    return (ret1 << 24) | (ret2 << 16) | (ret3 << 8) | ret4;
#endif
}
static __uint16_t spi_send_2bytes(__uint16_t val) {
#ifndef _WIN32
    // 片选CSN1
    SET_SPI(SFC_SOFTCS, 0x02);

    // 高字节
    while ((GET_SPI(SPSR)) & WFFULL)
        ;
    SET_SPI(TXRX_FIFO, (__uint8_t)((val >> 8) & 0xff));
    // 低字节
    while ((GET_SPI(SPSR)) & WFFULL)
        ;  // 发送缓存满，等待
    SET_SPI(TXRX_FIFO, (__uint8_t)(val & 0xff));

    // 读
    while ((GET_SPI(SPSR) & RFEMPTY))
        ;  // 等待读寄存器满
    // 高字节
    __uint8_t ret1 = GET_SPI(TXRX_FIFO);
    while ((GET_SPI(SPSR) & RFEMPTY))
        ;
    // 低字节
    __uint8_t ret2 = GET_SPI(TXRX_FIFO);

    // 取消片选CSN1
    SET_SPI(SFC_SOFTCS, 0x22);

    return (ret1 << 8) | ret2;
#endif
}

void spi_read(__uint16_t* buf, int len) {
#ifndef _WIN32
    int cnt;
    // printf("%s\n", __FUNCTION__);
    for (cnt = 0; cnt < len; cnt++) {
        buf[cnt] = spi_send_2bytes(0x1234);
        // ((__uint32_t*)buf)[cnt] = spi_send_4bytes(0x12345678);
    }
    // printf("%s done\n", __FUNCTION__);
#endif
}
void spi_write(__uint16_t* buf, int len) {
#ifndef _WIN32
    int cnt;
    printf("%s\n", __FUNCTION__);
    for (cnt = 0; cnt < len; cnt++) {
        spi_send_2bytes(buf[cnt]);
    }
    printf("%s done\n", __FUNCTION__);
#endif
}
