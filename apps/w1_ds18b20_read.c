#include <stdio.h>
#include "w1_ds18b20.h"

int main()
{
  printf("Hi w1\n");
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


  int count = 0;
  struct w1_device* w1_devs = NULL;
  int ret = get_w1_temp(&w1_devs, &count);

  for (int i = 0; i < count; i++)
  {
    printf("%i: ", i);
    printf("%.15s: ", &(w1_devs[i].name[20]));
    printf("%f\n", w1_devs[i].temperature);
    //printf("%i: %.15s: %f\n", i, &(w1_devs[i].name[20]), w1_devs[i].temperature);
  }
  return 0;
}


