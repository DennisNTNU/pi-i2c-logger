#include "si7021.h"

#include <stdio.h>
#include <fcntl.h> // for open()
#include <sys/ioctl.h> // for ioctl()
#include <unistd.h> // for read(), write(), close()
#include <time.h> // for nanosleep()
#include <linux/i2c-dev.h> // for I2C_SLAVE

int readSi7021(float * data)
{
    int fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0)
    {
        printf("Error Opening i2c peripheral\n");
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, 0x40) < 0)
    {
        printf("Error setting slave address\n");
        close(fd);
        return -1;
    }

    char buffer[2];
    buffer[0] = 0xF5; // measure humidity command

    int nacks = 1;
    //int bread;

    if (write(fd, buffer, 1) != 1)
    {
        printf("Error writing humidity measurement command\n");
        close(fd);
        return -1;
    }


    struct timespec _200us = {.tv_sec = 0, .tv_nsec = 200000};
    while (read(fd, buffer, 2) < 0)
    {
        nanosleep(&_200us, 0);
        nacks++;
        if (nacks > 100)
        {
            printf("reading humidity. timeout\n");
            close(fd);
            return -1;
        }
    }

    float rel = 125*(buffer[0] << 8 | buffer[1]);
    rel = rel/65536.0 - 6.0;

    buffer[0] = 0xE0; // read temperature from last humidity measurement command

    if (write(fd, buffer, 1) != 1)
    {
        printf("Error writing temperature read command\n");
        close(fd);
        return -1;
    }

    if (read(fd, buffer, 2) < 0)
    {
        printf("Error reading temperature\n");
        close(fd);
        return -1;
    }

    close(fd);

    float temp = 175.25*(buffer[0] << 8 | buffer[1]);
    temp = temp/65536.0 - 46.85;

    //printf("ackked. nacks: %i\n", nacks);

    data[0] = rel;
    data[1] = temp;


    return 0;
}

int readSi7021_fd(int fd_i2c, float* data)
{
    if (ioctl(fd_i2c, I2C_SLAVE, 0x40) < 0)
    {
        printf("Error setting slave address\n");
        return -1;
    }

    char buffer[2];
    buffer[0] = 0xF5; // measure humidity command

    int nacks = 1;
    if (write(fd_i2c, buffer, 1) != 1)
    {
        printf("Error writing humidity measurement command\n");
        return -1;
    }

    struct timespec _200us = {.tv_sec = 0, .tv_nsec = 200000};
    while (read(fd_i2c, buffer, 2) < 0)
    {
        nanosleep(&_200us, 0);
        nacks++;
        if (nacks > 100)
        {
            printf("reading humidity timeout\n");
            return -1;
        }
    }

    float rel = 125*(buffer[0] << 8 | buffer[1]);
    rel = rel/65536.0 - 6.0;

    buffer[0] = 0xE0; // read temperature from last humidity measurement command

    if (write(fd_i2c, buffer, 1) != 1)
    {
        printf("Error writing temperature read command\n");
        return -1;
    }

    if (read(fd_i2c, buffer, 2) < 0)
    {
        printf("Error reading temperature\n");
        return -1;
    }

    float temp = 175.25*(buffer[0] << 8 | buffer[1]);
    temp = temp/65536.0 - 46.85;

    //printf("ackked. nacks: %i\n", nacks);

    data[0] = rel;
    data[1] = temp;


    return 0;
}
