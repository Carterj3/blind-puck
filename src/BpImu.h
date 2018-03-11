#ifndef BpImu_h
#define BpImu_h

#include "I2Cdev.h"
#include "MPU6050.h"


class BpImu
{
  public:
    BpImu();

    void reset();

    void calibrate();

    long tick();

    void getPositionDelta(float *xDistance, float *yDistance, float *zMDistance, bool *xValid, bool *yValid, bool *zValid);

    void setGravityOrientation(int x, int y, int z);

    void debugGetValues(float *distance, float *velocity, float *gravity, int *rotation, bool *overrun, long *lastReadTime);
  private:
      void incrementComulativeDistance(int index, int durationMs, float accel);
      void incrementComulativeVelocity(int index, int durationMs, float accel);

      MPU6050 mpu_;
      float comulativeDistance_[3] = { 0.0, 0.0, 0.0 };
      float comulativeVelocity_[3] = { 0.0, 0.0, 0.0 };

      float gravity_[3] = { 0.0, 0.0, -1.024e6 };
      int cumulativeRotation_[3] = { 0, 0, 0 };
      long lastReadTime_;
      bool overrun_[3] = { false, false, false };
};

#endif
