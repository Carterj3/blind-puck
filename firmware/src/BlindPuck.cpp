#include "Blindpuck.h"

#include "Wire.h"
#include "Common.h"
#include "Constants.h"

int getAdc(){
  int average = 0;
  int count = 1000;
  int scale = 1000;

  for(int i=1; i <= count; i++){
    average = computeRollingAverage(average, i, analogRead(A0), scale);
  }

  return average / scale;
}
