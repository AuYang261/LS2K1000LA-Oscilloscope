#include <bits/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "types.h"

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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./pwm Freq [-d Duty_cycle=50(%)] [-c Channel=1]\n");
        return 1;
    }
    int freq;
    sscanf(argv[1], "%d", &freq);
    int channel = 1;
    int duty_cycle = 50;
    for (int i = 2; i < argc - 1; i++) {
        if (!strcmp(argv[i], "-d")) {
            sscanf(argv[++i], "%d", &duty_cycle);
            if (duty_cycle < 0 || duty_cycle > 100) {
                printf("duty_cycle error: %d%%\n", duty_cycle);
                return -1;
            }
        } else if (!strcmp(argv[i], "-c")) {
            sscanf(argv[++i], "%d", &channel);
            if (channel < 0 || channel > 3) {
                printf("channel error: %d\n", channel);
                return -1;
            }
        } else {
            printf("Unknow option: %s\n", argv[i]);
            return -1;
        }
    }
    int i;
    int dev_fd, offset, gpio_move;
    dev_fd = open("/dev/mem", O_RDWR | O_SYNC);

    if (dev_fd < 0) {
        printf("open(/dev/mem) failed.");
        return -1;
    }
    unsigned char *map_base =
        (unsigned char *)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                              dev_fd, MAP_BASE - MAP_BASE % PAGE_SIZE) +
        MAP_BASE % PAGE_SIZE;
    printf("map_base=%p\n", map_base);
    // 启用PWM1
    *(uint64_t *)map_base |= 1 << PWM_BIT(channel);

    map_base =
        (unsigned char *)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                              dev_fd, APB_BASE - APB_BASE % PAGE_SIZE) +
        APB_BASE % PAGE_SIZE;
    printf("map_base=%p\n", map_base);
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
