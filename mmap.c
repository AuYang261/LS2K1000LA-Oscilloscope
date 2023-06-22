#include <bits/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define MAP_SIZE 0x10000
#define AUDIO_REG_BASE 0x1fe00000

#define GPIO_EN 0x500
#define GPIO_OUT 0x510
#define GPIO_IN 0x520

// 控制GPIO39
// #define GPIO_PIN        39

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: ./mmap PIN 1(IN)/2(OUT) FREQ");
        return -1;
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
                              dev_fd, AUDIO_REG_BASE);
    printf("map_base=%p\n", map_base);

    //	printf("%x \n", *(volatile unsigned int *)(map_base + GPIO_EN));
    ////打印该寄存器地址的value

    int GPIO_PIN;
    sscanf(argv[1], "%d", &GPIO_PIN);

    if (GPIO_PIN > 31) {
        offset = 4;
        gpio_move = GPIO_PIN - 32;
    } else {
        offset = 0;
        gpio_move = GPIO_PIN;
    }

    int func;
    sscanf(argv[2], "%d", &func);
    int freq;
    sscanf(argv[3], "%d", &freq);
    struct timespec last, cur;
    clock_gettime(CLOCK_REALTIME, &last);

    if (func == 2) {
        *(volatile unsigned int *)(map_base + GPIO_EN + offset) &=
            ~(1 << gpio_move);  // GPIO输出使能
        int value = 0;
        while (1) {
            if (value) {
                *(volatile unsigned int *)(map_base + GPIO_OUT + offset) |=
                    (1 << gpio_move);  // 输出高
            } else {
                *(volatile unsigned int *)(map_base + GPIO_OUT + offset) &=
                    ~(1 << gpio_move);  // 输出底
            }
            while (1) {
                clock_gettime(CLOCK_REALTIME, &cur);
                if ((cur.tv_sec - last.tv_sec) * 1000 * 1000 * 1000 +
                        cur.tv_nsec - last.tv_nsec >
                    1000 * 1000 * 1000 / freq / 2) {
                    break;
                }
            }
            last = cur;
            // printf("out %d\r", value);
            // usleep(1000 * 1000 / freq / 2);
            // getchar();
            value = !value;
        }
    } else if (func == 1) {
        *(volatile unsigned int *)(map_base + GPIO_EN + offset) |=
            (1 << gpio_move);
        while (1) {
            printf("in %u\r",
                   ((*(volatile unsigned int *)(map_base + GPIO_IN + offset)) >>
                    gpio_move) &
                       1);
            usleep(1);
        }
    } else {
        printf("invalid func: %d\n", func);
    }

    munmap(map_base, MAP_SIZE);  // 解除映射关系

    if (dev_fd) close(dev_fd);

    return 0;
}
