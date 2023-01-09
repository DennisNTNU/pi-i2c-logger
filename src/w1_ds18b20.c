#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


/*
 * Makes an array of N strings, with each string containing a path to a w1
 * device's temperature file, with N being the number of discovered w1 devices
 *
 * @param count returns the w1 device count
 * @param list returns the array of strings
 */
int make_filelist(char*** list, int* count)
{

  // Looking up device count
  char dev_count_file_path[] = "/sys/bus/w1/devices/w1_bus_master1/w1_master_slave_count";

  FILE* dev_count_fp = fopen(dev_count_file_path, "r");
  if (dev_count_fp != NULL)
  {
    int ret = fscanf(dev_count_fp, "%i", count);
    if (ret != 1)
    {
      printf("Error getting device/slave/sensor count: %i: %s\n", errno, strerror(errno));
      return 126;
    }
    //printf("Dev count: %i\n", *count);
  }
  fclose(dev_count_fp);


  // Allocating list
  *list = malloc(*count*sizeof(char**));

  size_t list_string_size = 512;
  for (int i = 0; i < *count; i++)
  {
    (*list)[i] = malloc(list_string_size*sizeof(char));
  }

  char base_path[] = "/sys/bus/w1/devices";

  // Getting deivce names
  struct dirent** namelist = NULL;
  int ret = scandir(base_path, &namelist, NULL, alphasort);
  //int ret = scandir(".", &namelist, NULL, alphasort);
  if (ret == -1)
  {
    printf("Error scandir() %s: %i, %s\n", base_path, errno, strerror(errno));
    return 127;
  }

  // Making paths to w1 device temperature files
  //printf("The directory '%s' contains:\n", base_path);
  int j = 0;
  for (int i = 0; i < ret; i++)
  {
    //printf("%s, %i\n", namelist[i]->d_name, namelist[i]->d_type);

    /*
    printf("dirent types:\n");
    printf("DT_BLK: %i\n", DT_BLK);
    printf("DT_CHR: %i\n", DT_CHR);
    printf("DT_DIR: %i\n", DT_DIR);
    printf("DT_FIFO: %i\n", DT_FIFO);
    printf("DT_LNK: %i\n", DT_LNK);
    printf("DT_REG: %i\n", DT_REG);
    printf("DT_SOCK: %i\n", DT_SOCK);
    printf("DT_UNKNOWN: %i\n", DT_UNKNOWN);
    */


    if (namelist[i]->d_type == DT_LNK)
    {
      if (strcmp(namelist[i]->d_name, "w1_bus_master1") != 0)
      {
        //printf("    Shall open %s/%s/temperature\n", base_path, namelist[i]->d_name);
        snprintf((*list)[j++], list_string_size-1, "%s/%s/temperature", base_path, namelist[i]->d_name);
        //printf("Dev %i: %s\n", i, (*list)[j-1]);
      }
    }

    free(namelist[i]);
  }
  free(namelist);

  return 0;
}

struct w1_device
{
  char name[512];
  float temperature;
};

int get_w1_temp(struct w1_device** devs, int* count)
{
  char** list = NULL;
  *count = 0;
  int ret = make_filelist(&list, count);

  *devs = malloc(*count*sizeof(struct w1_device));

  for (int i = 0; i < *count; i++)
  {
    snprintf((*devs)[i].name, 511, "%s", list[i]);

    FILE* sensor_fp = fopen(list[i], "r");
    if (sensor_fp == NULL)
    {
      printf("Error opening %s: %i, %s\n", list[i], errno, strerror(errno));
      (*devs)[i].temperature = -1.0;
      continue;
    }

    int temp = 0;
    int parsed = fscanf(sensor_fp, "%i", &temp);
    if (parsed != 1)
    {
      printf("Error (ret: %i) reading temp from %s: %i, %s\n", parsed, list[i], errno, strerror(errno));
      (*devs)[i].temperature = -1.0;
    }

    (*devs)[i].temperature = ((float)temp)/1000.0;
    fclose(sensor_fp);
  }

  return 0;
}


int get_w1_temp_logsys(int unused, float* temperature_data)
{
  char** list = NULL;
  int count = 0;
  int ret = make_filelist(&list, &count);

  for (int i = 0; i < count; i++)
  {

    FILE* sensor_fp = fopen(list[i], "r");
    if (sensor_fp == NULL)
    {
      printf("Error opening %s: %i, %s\n", list[i], errno, strerror(errno));
      continue;
    }

    int temp = 0;
    int parsed = fscanf(sensor_fp, "%i", &temp);
    if (parsed != 1)
    {
      printf("Error fscanf() reading temp (ret: %i) from %s. errno: %i, %s\n", parsed, list[i], errno, strerror(errno));
    }
    fclose(sensor_fp);

    temperature_data[i] = ((float)temp)/1000.0;
  }

  return 0;
}
