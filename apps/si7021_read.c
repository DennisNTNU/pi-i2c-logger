#include "si7021.h"

#include <stdio.h>

int main()
{
  float data[8] = {0.0};

  int ret = readSi7021(data);

  printf("Si7021: Hum: %f | Tmp: %f\n", data[0], data[1]);

  return 0;
}
