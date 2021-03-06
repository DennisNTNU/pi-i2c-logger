#include "tmp102.h"

#include <stdio.h>
#include <fcntl.h> // for open()
#include <sys/ioctl.h> // for ioctl()
#include <unistd.h> // for read(), write(), close()
#include <time.h> // for nanosleep()
#include <linux/i2c-dev.h> // for I2C_SLAVE

int readTMP102(float * temp)
{
    int fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0)
    {
        printf("Error Opening i2c peripheral\n");
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, 0x48) < 0)
    {
        printf("Error setting slave address\n");
        close(fd);
        return -1;
    }

    signed char buffer[4];
    char data = 0x00;
    if (write(fd, &data, 1) < 0)
    {
        printf("error writing temp reg address\n");
    }

    if (read(fd, buffer, 2) < 0)
    {
        printf("error reading temp reg\n");
    }
    close(fd);

    //printf("%x %x\n", buffer[0], buffer[1]);

    *temp = ((buffer[0] << 8) | (unsigned char)buffer[1]) >> 4;
    *temp = *temp *0.0625;
    
    return 0;
}

int readTMP102_fd(int fd_i2c, float* temp)
{
    if (ioctl(fd_i2c, I2C_SLAVE, 0x48) < 0)
    {
        printf("Error setting slave address\n");
        return -1;
    }

    signed char buffer[4];
    char data = 0x00;
    if (write(fd_i2c, &data, 1) < 0)
    {
        printf("error writing temp reg address\n");
    }

    if (read(fd_i2c, buffer, 2) < 0)
    {
        printf("error reading temp reg\n");
    }

    //printf("%x %x\n", buffer[0], buffer[1]);

    *temp = ((buffer[0] << 8) | (unsigned char)buffer[1]) >> 4;
    *temp = *temp *0.0625;
    
    return 0;
}