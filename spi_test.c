#include "spi.h"

#include <stdio.h>
#include <string.h>

int main() {
    __uint16_t buf[512];
    __int64_t cnt = 0;
    if (spi_init() < 0) {
        printf("spi_init error.\n");
        return -1;
    }
    int i;
    FILE* f = fopen("output.txt", "w");
    if (!f) {
        printf("open error.\n");
    }
    clock_t start = clock();
    __uint16_t last_value;
    __uint16_t last_value2;
    __uint16_t last_value3;
    __uint16_t last_value4;
    __int64_t error_cnt = 0;
    __int64_t repeat_err_cnt = 0;
    while (1) {
        // memset(buf, 0, sizeof(buf));
        // printf("%d ", cnt++);
        spi_read(buf, sizeof(buf) / sizeof(buf[0]));
        for (i = 0; i < sizeof(buf) / sizeof(buf[0]); i++) {
            printf("0x%x ", buf[i]);
            // fprintf(f, "0x%x ", buf[i]);
            if (cnt != 0 && buf[i] != last_value2 && buf[i] != last_value &&
                (((last_value >> 12) + 1) & 0xf) != (buf[i] >> 12) &&
                (((last_value >> 12) + 2) & 0xf) != (buf[i] >> 12) &&
                (((last_value >> 12) + 3) & 0xf) != (buf[i] >> 12) &&
                (((last_value >> 12) + 4) & 0xf) != (buf[i] >> 12) &&
                (((last_value2 >> 12) + 1) & 0xf) != (buf[i] >> 12)) {
                error_cnt++;
                // printf("0x%x, 0x%x, 0x%x\n", last_value2, last_value,
                // buf[i]);
            }
            if (cnt != 0 && (buf[i] == last_value || buf[i] == last_value2 ||
                             buf[i] == last_value3 || buf[i] == last_value4)) {
                repeat_err_cnt++;
            }
            last_value4 = last_value3;
            last_value3 = last_value2;
            last_value2 = last_value;
            last_value = buf[i];
            cnt++;
            // if ((buf[i] & 0xfff) > 0x600) {
            //     printf("#####wraning.\n");
            //     // usleep(1000 * 100);
            // }
            // usleep(1);
        }
        if ((cnt % sizeof(buf)) == 0) {
            int ms = (clock() - start) * 1000 / CLOCKS_PER_SEC;
            printf(
                "%d samples in %dms, %d per second, "
                "error_cnt=%d,error_rate=%d/10000, rpt_rate=%d/10000\r",
                cnt, ms, 1000 * cnt / (ms == 0 ? 1 : ms), error_cnt,
                error_cnt * 10000 / cnt, repeat_err_cnt * 10000 / cnt);
        }
        // printf("\n");
        // fprintf(f, "\n");
    }
}