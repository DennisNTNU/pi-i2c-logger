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
#include "log_local.h"
#include "log_system.h"

////////////////////////////////////////////////////////////////////////////////

static int running = 1;

void signalHandler(int signal)
{
    if (signal == SIGINT)
    {
        running = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////

int writeADC(unsigned short val);

////////////////////////////////////////////////////////////////////////////////

struct logger_options
{
    char ip_port_str[32];
    unsigned short period_ms;
    int do_local_logging;
    int do_network_logging;
};

////////////////////////////////////////////////////////////////////////////////

void print_args(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
    {
        printf("%s | ", argv[i]);
    }
    printf("\n");
}

int parse_args(int argc, char** argv, struct logger_options* optns)
{
    int c = 0;
    opterr = 0;
    optind = 1;

    // setting default options
    strncpy(optns->ip_port_str, "10.0.0.3:3001", 32);
    optns->period_ms = 10000;
    optns->do_network_logging = 0;
    optns->do_local_logging = 0;
    /*
    -h: si7021 sensor: temp & humidity
    -t: tmp102 sensor: temp only
    -s <ip>:<port>: specify server to send data to
    */
    const char* options_string = ":htlLa:p:";

    while ((c = getopt(argc, argv, options_string)) != -1)
    {
        switch (c)
        {
        case 'h':
            printf("Si7021 option \"%c\"\n", c);
            float hum_temp_buffer[2];
            readSi7021(hum_temp_buffer);
            printf("Si7021: Humidity %f, Temp %f\n", hum_temp_buffer[0], hum_temp_buffer[1]);
            break;
        case 't':
            printf("TMP102 option \"%c\"\n", c);
            float temp;
            readTMP102(&temp);
            printf("TMP102: Temp %f\n", temp);
            break;

        case 'l':
            printf("Logging server option \"%c\"\n", c);
            optns->do_network_logging = 1;
            break;
        case 'L':
            printf("local logging option (log to file) \"%c\"\n", c);
            optns->do_local_logging = 1;
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
    return optns->do_network_logging | optns->do_local_logging;
}














void logging(struct logger_options* optns)
{
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
    }

    int local_logging_ok = 1;
    if (optns->do_local_logging)
    {
        printf("Initing local log buffer & path\n");
        struct timeval t;
        gettimeofday(&t, NULL);
        char logfilename[32];
        snprintf(logfilename, 32, "%li_data.log", t.tv_sec);

        local_logging_ok = log_alloc_buffer(1024*4, logfilename); // 4KiB
        if (local_logging_ok == 0)
        {
            char header[] = "time, humidity, temp si7021, temp tmp102\n";
            int headerlen = strlen(header);
            if (log_data(header, headerlen) != 0)
            {
                printf("Error logging header\n");
            }
        }
        else
        {
            printf("local logging not ok: %i\n", local_logging_ok);
        }
    }

    const struct timespec delay = {.tv_sec = optns->period_ms/1000, .tv_nsec = (optns->period_ms % 1000) * 1000000};
    char buffer_network[64];
    char buffer_local[64];
    int error = 0;
    while (running)
    {
        error = 0;
        if (readSi7021(humTemp) < 0)
        {
            error = 1;
        }
        if (readTMP102(&temp) < 0)
        {
            error = 1;
        }
        if (error == 1)
        {
            sprintf(buffer_network, "mHumidity: %f\nTemp1: %f Temp2: %f\nError %i\n", humTemp[0], humTemp[1], temp, errno);
            sprintf(buffer_local, "%i, %f, %f, %f\n", -1, humTemp[0], humTemp[1], temp);
        }
        else
        {
            sprintf(buffer_network, "mHumidity: %f\nTemp1: %f Temp2: %f\n", humTemp[0], humTemp[1], temp);
            struct timeval t;
            gettimeofday(&t, NULL);
            sprintf(buffer_local, "%li, %f, %f, %f\n", t.tv_sec, humTemp[0], humTemp[1], temp);
        }


        if (optns->do_network_logging)
        {
            int len = strlen(buffer_network);
            int bytes_sendt = sendto(sock, buffer_network, len+1, 0, (struct sockaddr*)&sa, sizeof(sa));
            printf("sendt bytes: %i\n%s\n", bytes_sendt, buffer_network);
        }

        if (optns->do_local_logging)
        {
            int len = strlen(buffer_local);
            if (local_logging_ok == 0)
            {
                if (log_data(buffer_local, len) != 0)
                {
                    printf("Error logging data\n");
                }
            }
        }
        nanosleep(&delay, 0);
    }
    printf("Flushing data to file before exiting\n");
    log_flush_buffer();
    log_dealloc_buffer();
    close(sock);
}



////////////////////////////////////////////////////////////////////////////////





int main(int argc, char** argv)
{
    signal(SIGINT, signalHandler);

    struct logger_options optns;
    print_args(argc, argv);
    int ret = parse_args(argc, argv, &optns);
    print_args(argc, argv);

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
    }

    if (ret)
    {
        logging(&optns);
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

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


