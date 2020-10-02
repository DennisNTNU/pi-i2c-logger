#ifndef LOG_SYSTEM_H
#define LOG_SYSTEM_H

// log 'state' that holds flags about whether to log to file and to netwrok
// and which sensors to log.
struct logger_options
{
    char ip_port_str[32];
    int do_network_logging;
    int do_local_logging;

    unsigned short period_ms;
    struct sensor_info_float* sensors_to_log;
    int sensor_count;
    int max_sensor_count;
};

// struct containing a function pointer to the function that reads the data from the i2c sensor
struct sensor_info_float
{
    // function pointer to the function that reads the data from the i2c sensor
    // expects i2c fd as first argument and a suitably allocated array of floats
    int (*sensor_read_func)(int, float*);
    // how many floats the sensor_read function requries allocated
    int float_count;
    // e.g. for si7021, should be something like: "si7021_humidity, si7021_temperature"
    char header_part[128];
};


int log_system_init(int max_sensor_count);
 // frees (struct sensor_info_float*) in the logger options struct
int log_system_deinit();

int log_system_add_sensor(int (*sensor_read_func)(int, float*), int float_count, char* header_part);

int log_system_enable_file_logging(char* name);
int log_system_enable_network_logging(char* ip_port_str);


int log_start();

#endif /* LOG_SYSTEM_H */
