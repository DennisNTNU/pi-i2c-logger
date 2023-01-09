#include "log_system.h"

#include <stdio.h>
#include <fcntl.h> // for open()
#include <unistd.h> // for close()
#include <string.h> // for memset(), strlen(), strcpy()
#include <stdlib.h> // for calloc()
#include <sys/time.h> // for gettimeofday()
#include <time.h> // for nanosleep()
#include <signal.h> // for SIGINT, signal()
#include <math.h> // for NAN

#include "si7021.h"
#include "tmp102.h"
#include "debug_macros.h"
#include "buffered_writer.h"

static struct logger_options lopts =
{
    .ip_port_str = "",
    .do_network_logging = 0,
    .do_local_logging = 0,
    .period_ms = 10000,
    .sensors_to_log = NULL,
    .sensor_count = 0,
    .max_sensor_count = 0,
};


static int running = 1;

static void signalHandler(int signal)
{
    running = 0;
}

int log_system_init(int max_sensor_count, unsigned short period_ms)
{
    memset(lopts.ip_port_str, 0, 32);
    lopts.do_network_logging = 0;
    lopts.do_local_logging = 0;

    lopts.period_ms = period_ms;
    if (period_ms < 5000)
    {
        lopts.period_ms = 5000;
    }
    lopts.max_sensor_count = max_sensor_count;
    lopts.sensor_count = 0;

    if (lopts.sensors_to_log != NULL)
    {
        free(lopts.sensors_to_log);
    }
    lopts.sensors_to_log = calloc(1, max_sensor_count*sizeof(struct sensor_info_float));

    return 0;
}

int log_system_deinit()
{
    memset(lopts.ip_port_str, 0, 32);
    lopts.do_network_logging = 0;
    lopts.do_local_logging = 0;

    lopts.period_ms = 10000;
    lopts.max_sensor_count = 0;
    lopts.sensor_count = 0;
    if (lopts.sensors_to_log != NULL)
    {
        free(lopts.sensors_to_log);
    }

    return 0;
}

int log_system_add_sensor(int (*sensor_read_func)(int, float*), int float_count, char* header_part)
{
    if (lopts.sensor_count < lopts.max_sensor_count && float_count > 0 && header_part != NULL)
    {
        lopts.sensors_to_log[lopts.sensor_count].sensor_read_func = sensor_read_func;
        lopts.sensors_to_log[lopts.sensor_count].float_count = float_count;
        unsigned int header_part_size = sizeof(lopts.sensors_to_log[lopts.sensor_count].header_part);
        strncpy(lopts.sensors_to_log[lopts.sensor_count].header_part, header_part, header_part_size);
        lopts.sensor_count++;
        return 0;
    }
    return -1;
}

int log_system_enable_file_logging(char* log_name)
{
    printf("Initing local log buffer & path\n");

    struct timeval t;
    memset(&t, 0, sizeof(struct timeval));
    gettimeofday(&t, NULL);

    char log_file_name[256] = "";
    snprintf(log_file_name, 256, "%li_%s.csv", t.tv_sec, log_name);

    int header_buffer_offset = 0;
    char header[1024];

    header_buffer_offset += snprintf(header, 512, "time");

    for (int i = 0; i < lopts.sensor_count; i++)
    {
        header_buffer_offset += snprintf(&(header[header_buffer_offset]),
            1024-header_buffer_offset, ", %s",
            lopts.sensors_to_log[i].header_part);
    }
    header[header_buffer_offset] = '\n';
    header[header_buffer_offset+1] = 0;

    printf("Full log file name: %s\n", log_file_name);
    printf("File log header: %s", header);

    //printf("Initing file log buffer\n");
    int local_logging_ok = br_alloc_buffer(1024*4, log_file_name); // 4KiB
    if (local_logging_ok == 0)
    {
        int headerlen = strlen(header);
        if (br_data(header, headerlen) != 0)
        {
            printf("Error logging header\n");
            return -2;
        }
    }
    else
    {
        printf("local logging to file not ok: %i\n", local_logging_ok);
        return -1;
    }

    return 0;
}

int log_system_enable_network_logging(char* ip_port_str)
{

    return 0;
}

int log_loop()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    int fd_i2c = open("/dev/i2c-1", O_RDWR);
    if (fd_i2c < 0)
    {
        printf("Error Opening i2c peripheral\n");
        return -1;
    }

    // Add up how many floats each sensor requires
    int float_count_total = 0;
    for (int i = 0; i < lopts.sensor_count; i++)
    {
        float_count_total += lopts.sensors_to_log[i].float_count;
    }

    float* data_buffer = malloc(float_count_total*sizeof(float));

    while (running)
    {
        struct timespec t0;
        clock_gettime(CLOCK_REALTIME_COARSE, &t0);

        // 1. get sensor data

        int current_float_count = 0;
        // iterate over all registered sensors
        for (int i = 0; i < lopts.sensor_count; i++)
        {
            // call the sensors read function
            int ret = (*(lopts.sensors_to_log[i].sensor_read_func))(fd_i2c, &(data_buffer[current_float_count]));
            if (ret < 0)
            {
                // on error set the measuement values to NaN
                for (int j = 0; j < lopts.sensors_to_log[i].float_count; j++)
                {
                    data_buffer[current_float_count + j] = NAN;
                }
            }
            current_float_count += lopts.sensors_to_log[i].float_count;
        }

        // 2. write data to log
        int data_buffer_offset = 0;
        char buffer[512];

        // timestamp
        data_buffer_offset += snprintf(buffer, 511, "%li", t0.tv_sec);

        // copy sensor values into buffer
        for (int i = 0; i < float_count_total; i++)
        {
            data_buffer_offset += sprintf(&(buffer[data_buffer_offset]), ", %f", data_buffer[i]);
        }

        buffer[data_buffer_offset] = '\n';
        buffer[data_buffer_offset+1] = 0;

        int len = strlen(buffer);
        // use the buffered reader module to sate data into file in chunks
        if (br_data(buffer, len) != 0)
        {
            printf("%li %s:%i: Error logging data to buffered reader\n", t0.tv_sec, __func__, __LINE__);
        }
        struct timespec t1;
        clock_gettime(CLOCK_REALTIME_COARSE, &t1);


        int circa_period_s = lopts.period_ms/1000 - (t1.tv_sec - t0.tv_sec);
        if (circa_period_s < 0)
        {
            circa_period_s = 1;
        }

        //const struct timespec delay = {
        //    .tv_sec = lopts.period_ms/1000,
        //    .tv_nsec = (lopts.period_ms % 1000) * 1000000
        //};
        const struct timespec delay = { .tv_sec = circa_period_s, .tv_nsec = 0 };
        nanosleep(&delay, 0);
    }
    printf("Flushing data to file before exiting\n");
    br_flush_buffer();
    br_dealloc_buffer();
    close(fd_i2c);
    return 0;
}
