#ifndef __SPI_H__
#define __SPI_H__

#include "types.h"

#ifndef _WIN32

#include <fcntl.h>
#include <sys/mman.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

int spi_init();
void spi_read(__uint16_t *buf, int len);
void spi_write(__uint16_t *buf, int len);

#endif
