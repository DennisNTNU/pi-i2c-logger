#ifndef LOG_SYSTEM_H
#define LOG_SYSTEM_H

// log 'state' that holds flags about whether to log to file and to network
// and which sensors to log.
struct logger_options
{
    char ip_port_str[32];
    int do_network_logging;
    int do_local_logging;

    unsigned short period_ms;

    // pointer to an array of 'struct sensor_info_float's with max_sensor_count elements
    struct sensor_info_float* sensors_to_log;
    int sensor_count;
    int max_sensor_count;
};

// struct containing a function pointer to the function that reads the data from the i2c sensor
struct sensor_info_float
{
    // function pointer to the function that reads the data from the i2c sensor
    // expects i2c fd as first argument and a suitably allocated array of floats
    // also expects this funtion to put the readings into the float array
    int (*sensor_read_func)(int, float*);

    // how many floats the sensor_read function requries allocated
    int float_count;

    // name of this sensors log entry
    // e.g. for si7021, should be something like: "si7021_humidity, si7021_temperature"
    char header_part[128];
};







/**
 * Initialize the logging system with a upper bound of sensors and the logging period
 */
int log_system_init(int max_sensor_count, unsigned short period_ms);

// frees (struct sensor_info_float*) in the logger options struct
int log_system_deinit();

/**
 * sensor_read_func Function with first parameter being an open i2c file desciptor and
 *                  second parameter an array of floats that is being filled with one sensor reading
 * float_count How many floating point values one sensor reading consists of
 * header_part String containing a laber for this sensor's readings
 */
int log_system_add_sensor(int (*sensor_read_func)(int, float*), int float_count, char* header_part);

/**
 * name Ascii string with the log file name
 */
int log_system_enable_file_logging(char* name);
/**
 * CURRENTLY NOT IMPLEMENTED
 * ip_port_str Address to send data to. Of the form <ip>:<port> "111.222.101.100:12345"
 */
int log_system_enable_network_logging(char* ip_port_str);



/**
 * Start logging loop
 */
int log_loop();

#endif /* LOG_SYSTEM_H */
