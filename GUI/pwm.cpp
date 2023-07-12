#include <bits/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "../types.h"

#define PAGE_SIZE 0x4000
#define MAP_SIZE PAGE_SIZE
#define MAP_BASE 0x1fe00420
// #define MAP_BASE1 0x1fe20000
// #define MAP_BASE_offset 0x2000
#define APB_BASE 0x1fe22000
#define PWM_REG_ADDR(BASE, x, channel) (BASE + x + (channel << 4) + (0 << 8))
#define PWM_LOWBUF_ADDR(BASE, channel) \
    ((uint32_t *)PWM_REG_ADDR(BASE, 0x4, channel))
#define PWM_FULLBUF_ADDR(BASE, channel) \
    ((uint32_t *)PWM_REG_ADDR(BASE, 0x8, channel))
#define PWM_CTL_ADDR(BASE, channel) \
    ((uint32_t *)PWM_REG_ADDR(BASE, 0xC, channel))

#define PWM_BIT(channel) (12 + channel)

#define GPIO_EN 0x500
#define GPIO_OUT 0x510
#define GPIO_PWM(channel) (0x20 + channel)

static int channel = 1;
static unsigned char *map_base;

int PWM_init(int _channel) {
    printf("%s\n", __FUNCTION__);
    channel = _channel;
    int dev_fd;
    dev_fd = open("/dev/mem", O_RDWR | O_SYNC);

    if (dev_fd < 0) {
        printf("open(/dev/mem) failed.");
        return -1;
    }
    map_base =
        (unsigned char *)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                              dev_fd, MAP_BASE - MAP_BASE % PAGE_SIZE) +
        MAP_BASE % PAGE_SIZE;
    // 启用PWM1
    *(uint64_t *)map_base |= 1 << PWM_BIT(channel);

    map_base =
        (unsigned char *)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                              dev_fd, APB_BASE - APB_BASE % PAGE_SIZE) +
        APB_BASE % PAGE_SIZE;
    // 低11位清零
    *PWM_CTL_ADDR(map_base, channel) &= ~0x7ff;
    printf("%s done!\n", __FUNCTION__);
    return 0;
}

int PWM_set(int freq, int duty_cycle) {
    // 低11位清零
    *PWM_CTL_ADDR(map_base, channel) &= ~0x7ff;
    // 写入计数器以控制占空比和频率
    // 占空比=(FULLBUF-LOWBUF)/FULLBUF
    // 频率=125M/FULLBUF Hz
    *PWM_LOWBUF_ADDR(map_base, channel) =
        125 * 1000 * 1000 / freq - 125 * 1000 * 1000 * duty_cycle / 100 / freq;
    *PWM_FULLBUF_ADDR(map_base, channel) = 125 * 1000 * 1000 / freq;
    // 使能
    *PWM_CTL_ADDR(map_base, channel) |= 0x1;
    return 0;
}
