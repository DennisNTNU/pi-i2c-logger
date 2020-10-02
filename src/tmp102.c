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

    char buffer[4];
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

    *temp = ((buffer[0] << 8) | buffer[1]) >> 4;
    *temp = *temp *0.0625;
    
    return 0;
}