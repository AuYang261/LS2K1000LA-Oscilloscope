#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <string>
//  #include <sys/stat.h>
//  #include <sys/types.h>

int main() {
    int GPIO_PIN;
    printf("input GPIO_PIN:");
    scanf("%d", &GPIO_PIN);
    FILE* exportFile = fopen("/sys/class/gpio/export", "a");
    if (exportFile < 0) {
        printf("open export error\n");
        return 0;
    }
    fprintf(exportFile, "%d", GPIO_PIN);
    fclose(exportFile);
    std::string gpiodir =
        "/sys/class/gpio/gpio" + std::to_string(GPIO_PIN) + "/";

    // char gpiodir[100];
    // sscanf(gpiodir, "/sys/class/gpio/gpio%d/", GPIO_PIN);
    FILE* in_out = fopen((gpiodir + "direction").c_str(), "a");
    FILE* value = fopen((gpiodir + "value").c_str(), "a");
    int io;
    printf("input function:1(in), 2(out):");
    scanf("%d", &io);
    int32_t data;
    if (io == 1) {
        fwrite("in", 1, 2, in_out);
        while (1) {
            fread(&data, sizeof(data), 1, value);
            printf("in %d\n", data);
        }
    } else if (io == 2) {
        fwrite("out", 1, 3, in_out);
        while (1) {
            data = 0;
            fwrite(&data, sizeof(data), 1, value);
            printf("out %d\n", data);
            data = 1;
            fwrite(&data, sizeof(data), 1, value);
            printf("out %d\n", data);
        }

    } else {
        printf("unknown function %d\n", io);
        return 0;
    }

    fclose(value);
    fclose(in_out);
    return 0;
}
