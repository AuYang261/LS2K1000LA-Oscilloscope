#include <stdio.h>
#include <string.h>

#include <iostream>

#include "slide_window.h"
#include "spi.h"

int main() {
    __uint16_t buf[512];
    if (spi_init() < 0) {
        printf("spi_init error.\n");
        return -1;
    }
    Slide_Window::get_instance().init();
    Sequence_t& seq = Slide_Window::get_instance().get_seq();
    clock_t start = clock();
    int64_t cnt = 0;
    while (1) {
        // memset(buf, 0, sizeof(buf));
        // printf("%d ", cnt++);
        spi_read(buf, sizeof(buf) / sizeof(buf[0]));
        for (int i = 0; i < sizeof(buf) / sizeof(buf[0]); i++) {
            // printf("0x%x ", buf[i]);
            Slide_Window::get_instance().add_data(buf[i]);
            cnt++;
        }
        int ms = (clock() - start) * 1000 / CLOCKS_PER_SEC;
        printf(
            "%ld samples in %dms, %ld per second, "
            "error_cnt=%ld,error_rate=%ld/10000\r",
            cnt, ms, 1000 * cnt / (ms == 0 ? 1 : ms),
            Slide_Window::get_instance().get_error_cnt(),
            Slide_Window::get_instance().get_error_cnt() * 10000 / cnt);
        // printf("\n");
    }
}