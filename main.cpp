#include <stdio.h>
#include <string.h>

#include <iostream>

#include "slide_window.h"
#include "spi.h"

int main(int argc, char* argv[]) {
    __uint16_t buf[512];
    if (spi_init() < 0) {
        printf("spi_init error.\n");
        return -1;
    }
    Slide_Window& sw = Slide_Window::get_instance();
    sw.init();
    clock_t start = clock();
    int64_t cnt = 0;
    while (1) {
        // memset(buf, 0, sizeof(buf));
        // printf("%d ", cnt++);
        spi_read(buf, sizeof(buf) / sizeof(buf[0]));
        for (int i = 0; i < sizeof(buf) / sizeof(buf[0]); i++) {
            if (argc >= 2) {
                printf("0x%x ", buf[i]);
            }
            sw.add_data(buf[i]);
        }
        cnt += sizeof(buf) / sizeof(buf[0]);
        // sw.display();
        int ms = (clock() - start) * 1000 / CLOCKS_PER_SEC;
        printf(
            "%ld samples in %dms, %ld per second, "
            "error_cnt=%ld, error_rate=%ld/10000\r",
            cnt, ms, 1000 * cnt / (ms == 0 ? 1 : ms), sw.get_error_cnt(),
            sw.get_error_cnt() * 10000 / sw.size());
        // printf("\n");
    }
}