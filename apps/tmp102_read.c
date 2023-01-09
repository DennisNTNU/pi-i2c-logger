#include "tmp102.h"

#include <stdio.h>

int main()
{
  float temp = 0.0;

  int ret = readTMP102(&temp);

  printf("Tmp102: %f\n", temp);

  return 0;
}
