#ifndef W1_DS18B20_H
#define W1_DS18B20_H

struct w1_device
{
  char name[512];
  float temperature;
};

int make_w1_filelist(char*** list, int* count);
int get_w1_temp(struct w1_device** devs, int* count);

int get_w1_temp_logsys(int unused, float* temperature_data);

#endif /* W1_DS18B20_H */
