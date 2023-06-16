#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "slide_window.h"
#include "spi.h"

int main(int argc, char* argv[]) {
    __uint16_t buf[512];
    struct timeval dwStart, dwEnd;
    if (spi_init() < 0) {
        printf("spi_init error.\n");
        return -1;
    }
    Slide_Window& sw = Slide_Window::get_instance();
    sw.init();
    int64_t cnt = 0;
    gettimeofday(&dwStart, NULL);
    while (1) {
        // memset(buf, 0, sizeof(buf));
        // printf("%d ", cnt++);
        spi_read(buf, sizeof(buf) / sizeof(buf[0]));
        for (int i = 0; i < sizeof(buf) / sizeof(buf[0]); i++) {
            if (argc >= 2) {
                printf("0x%x\n", buf[i]);
            }
            sw.add_data(buf[i]);
        }
        cnt += sizeof(buf) / sizeof(buf[0]);
        // sw.display();
        gettimeofday(&dwEnd, NULL);
        int64_t us = 1000 * 1000 * (dwEnd.tv_sec - dwStart.tv_sec) +
                     (dwEnd.tv_usec - dwStart.tv_usec);
        // usleep(1);
        if (argc < 2) {
            printf(
                "%ld samples in %dms, %ld per second, "
                "error_cnt=%ld, error_rate=%ld/10000\r",
                cnt, us / 1000, 1000 * 1000 * cnt / (us == 0 ? 1 : us),
                sw.get_error_cnt(),
                sw.get_error_cnt() * 10000 / (sw.size() == 0 ? 1 : sw.size()));
        }
        // printf("\n");
    }
}