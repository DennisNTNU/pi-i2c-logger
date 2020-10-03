#include <stdio.h>
#include <fcntl.h> // for open()
#include <sys/ioctl.h> // for ioctl()
#include <sys/time.h> // for gettimeofday()
#include <unistd.h> // for read(), write(), close(), getopt()

#include <signal.h>

#include <string.h> // for strcmp(), strtok()
#include <stdlib.h> // for atoi()
#include <time.h> // for nanosleep()

#include <sys/socket.h> // defines AF_INET, SOCK_DGRAM and socket()
#include <netinet/in.h> // defines IPPROTO_UDP
#include <arpa/inet.h> // for inet_addr()

#include <errno.h>

#include <linux/i2c-dev.h> // for I2C_SLAVE

#include "si7021.h"
#include "tmp102.h"
#include "buffered_writer.h"
#include "log_system.h"

////////////////////////////////////////////////////////////////////////////////

struct args_options
{
    unsigned char do_file_logging;
    unsigned char do_network_logging;
    unsigned short period_ms;
    char ip_port_str[32];

    unsigned char log_si7021;
    unsigned char log_tmp102;

    int total_sensor_count;
};

////////////////////////////////////////////////////////////////////////////////

int writeADC(unsigned short val);

////////////////////////////////////////////////////////////////////////////////

void print_args(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
    {
        printf("%s | ", argv[i]);
    }
    printf("\n");
}

int parse_args(int argc, char** argv, struct args_options* optns)
{
    int c = 0;
    opterr = 0;
    optind = 1;

    // setting default options
    strncpy(optns->ip_port_str, "10.0.0.3:3001", 32);
    optns->period_ms = 10000;
    optns->do_network_logging = 0;
    optns->do_file_logging = 0;
    optns->log_si7021 = 0;
    optns->log_tmp102 = 0;
    optns->total_sensor_count = 0;

    const char* options_string = ":htlLa:p:";

    while ((c = getopt(argc, argv, options_string)) != -1)
    {
        switch (c)
        {
            // TODO let these flags define which sensor are going to be logged
        case 'h':
            printf("Si7021 option \"%c\"\n", c);
            float hum_temp_buffer[2];
            readSi7021(hum_temp_buffer);
            printf("Si7021: Humidity %f, Temp %f\n", hum_temp_buffer[0], hum_temp_buffer[1]);
            optns->log_si7021 = 1;
            optns->total_sensor_count++;
            break;
        case 't':
            printf("TMP102 option \"%c\"\n", c);
            float temp;
            readTMP102(&temp);
            printf("TMP102: Temp %f\n", temp);
            optns->log_tmp102 = 1;
            optns->total_sensor_count++;
            break;

        case 'l':
            printf("Logging server option \"%c\"\n", c);
            optns->do_network_logging = 1;
            break;
        case 'L':
            printf("local logging option (log to file) \"%c\"\n", c);
            optns->do_file_logging = 1;
            break;
        case 'a':
            strncpy(optns->ip_port_str, optarg, 32);
            printf("Custom <ip>:<port> option \"%c\"; %s\n", c, optarg);
            break;
        case 'p':
            optns->period_ms = atoi(optarg);
            if (optns->period_ms < 2000)
            {
                optns->period_ms = 2000;
            }
            printf("Custom sample period option \"%c\"; %i: atoi(%s) -> %i\n", c, optind, optarg, optns->period_ms);
            break;
        //------------------------------------------------------------
        case '?':
            printf("Unknown option: \"%c\"\n", optopt);
            return -1;
            break;
        case ':':
            printf("Option \"%c\" is missing its argument\n", optopt);
            return -2;
            break;
        default:
            printf("Unhandled option \"%c\" ? (default case)\n", c);
            return -3;
            break;
        }
    }
    return optns->do_network_logging | optns->do_file_logging;
}



void configure_logger(struct args_options* optns)
{
/*
    float humTemp[2];
    float temp;

    char* ip = strtok(optns->ip_port_str, ":");
    char* port = strtok(NULL, ":");

    printf("given ip: %s | and port: %i\n", ip, atoi(port));

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(ip);
    sa.sin_port = htons(atoi(port));

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        printf("Could not create socket\n");
        return;
    }*/

    log_system_init(optns->total_sensor_count, optns->period_ms);
    if (optns->log_si7021)
    {
        log_system_add_sensor(readSi7021_fd, 2, "si7021_humidity, si7021_temperature");
    }
    if (optns->log_tmp102)
    {
        log_system_add_sensor(readTMP102_fd, 1, "tmp102_temperature");
    }
    if (optns->do_file_logging)
    {
        log_system_enable_file_logging("fridge_temperature_humidity");
    }
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    struct args_options optns;
    int ret = parse_args(argc, argv, &optns);

    if (ret)
    {
        configure_logger(&optns);
        log_loop();
    }

    return 0;
}





////////////////////////////////////////////////////////////////////////////////

/*
    if (argc >= 2)
    {
        if (strcmp(argv[1], "adc") == 0)
        {
            if (argc == 3)
            {
                printf("writing adc\n");
                unsigned short val = atoi(argv[2]);
                writeADC(val);
            }
        }
    }*/

int writeADC(unsigned short val)
{
    int fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0)
    {
        printf("Error Opening i2c peripheral\n");
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, 0x62) < 0)
    {
        printf("Error setting slave address\n");
        close(fd);
        return -1;
    }

    unsigned char data[2];
    data[0] = 0x0f & (val >> 8);
    data[1] = val;

    if (write(fd, data, 2) != 2)
    {
        printf("Error writing bytes\n");
        close(fd);
        return -1;
    }

    close(fd);


    return 0;
}


