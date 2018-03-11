#include "Common.h"

int computeRollingAverage(int rollingAverage, int index, int newValue, int scale)
{
  return (( rollingAverage * (index-1) ) / index) + (( scale * newValue ) / index);
}

int sign(float number)
{
  return (number < 0.0) ? -1 : 1;
}

int sign(int number)
{
  return (number < 0.0) ? -1 : 1;
}
