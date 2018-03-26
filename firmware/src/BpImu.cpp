#include "BpImu.h"

// IMS
#include "I2Cdev.h"
#include "MPU6050.h"

#include "Common.h"

const int GYRO_FULL_CIRCLE_RESET = 5904; // 16.4 * 360
const float GYRO_DEGRESS_TO_RADIAN = 939.650784015; // Pi / (180 * 16.4)

BpImu::BpImu()
{
  mpu_.setFullScaleGyroRange(MPU6050_GYRO_FS_2000); // x/16.4 degrees
  mpu_.setFullScaleAccelRange(MPU6050_ACCEL_FS_16); // x/1024 mg

  reset();
}

void BpImu::reset()
{
  comulativeDistance_[0] = 0.0;
  comulativeDistance_[1] = 0.0;
  comulativeDistance_[2] = 0.0;

  comulativeVelocity_[0] = 0.0;
  comulativeVelocity_[1] = 0.0;
  comulativeVelocity_[2] = 0.0;

  cumulativeRotation_[0] = 0;
  cumulativeRotation_[1] = 0;
  cumulativeRotation_[2] = 0;

  lastReadTime_ = millis();
  overrun_[0] = false;
  overrun_[1] = false;
  overrun_[2] = false;
}

void BpImu::calibrate()
{
  // Do Nothing for now
}

void BpImu::getPositionDelta(float *xDistance, float *yDistance, float *zDistance, bool *xValid, bool *yValid, bool *zValid)
{
  // Undo 1024 LSB to 1 milligravity and answer is in millis
  *xDistance = comulativeDistance_[0] / 1024.0;
  *yDistance = comulativeDistance_[1] / 1024.0;
  *zDistance = comulativeDistance_[2] / 1024.0;

  *xValid = overrun_[0];
  *yValid = overrun_[1];
  *zValid = overrun_[2];
}

void BpImu::setGravityOrientation(int x, int y, int z)
{
  gravity_[0] = x;
  gravity_[1] = y;
  gravity_[2] = z;
}

void BpImu::debugGetValues(float *distance, float *velocity, float *gravity, int *rotation, bool *overrun, long *lastReadTime)
{
  for(int i=0;i<3;i++)
  {
    distance[i] = comulativeDistance_[i];
    velocity[i] = comulativeVelocity_[i];
    gravity[i] = gravity_[i];
    rotation[i] = cumulativeRotation_[i];
    overrun[i] = overrun_[i];
    *lastReadTime = lastReadTime_;
  }
}

long BpImu::tick()
{
  if(!mpu_.getIntDataReadyStatus())
  {
    return -1;
  }

  int tickStart = micros();

  int16_t accelXRaw, accelYRaw, accelZRaw,
          gyroXRaw , gyroYRaw , gyroZRaw;
  mpu_.getMotion6(&accelXRaw, &accelYRaw, &accelZRaw, &gyroXRaw, &gyroYRaw, &gyroZRaw);
  int durationUs = micros() - lastReadTime_;
  lastReadTime_ = lastReadTime_ + durationUs;

  cumulativeRotation_[0] = (cumulativeRotation_[0] + gyroXRaw) % GYRO_FULL_CIRCLE_RESET;
  cumulativeRotation_[1] = (cumulativeRotation_[1] + gyroYRaw) % GYRO_FULL_CIRCLE_RESET;
  cumulativeRotation_[2] = (cumulativeRotation_[2] + gyroZRaw) % GYRO_FULL_CIRCLE_RESET;

  float xRadians = cumulativeRotation_[0] / GYRO_DEGRESS_TO_RADIAN;
  float yRadians = cumulativeRotation_[1] / GYRO_DEGRESS_TO_RADIAN;
  float zRadians = cumulativeRotation_[2] / GYRO_DEGRESS_TO_RADIAN;

  float cosX = cos(xRadians);
  float cosY = cos(yRadians);
  float cosZ = cos(zRadians);

  float sinX = sin(xRadians);
  float sinY = sin(yRadians);
  float sinZ = sin(zRadians);

  float xAccel =
                ( accelXRaw * ( cosY * cosZ)
                + accelYRaw * ( (cosZ * sinX * sinY) + (cosX * sinZ))
                + accelZRaw * ( (sinX * sinZ) - (cosX * cosZ * sinY)))
                + gravity_[0];

  float yAccel =
                ( accelXRaw * ( -1.0 * cosY * sinZ)
                + accelYRaw * ( (cosX * cosZ) - (sinX * sinY * sinZ))
                + accelZRaw * ( (cosZ * sinX) + (cosX * sinY * sinZ)))
                + gravity_[1];

  float zAccel =
                ( accelXRaw * ( sinY)
                + accelYRaw * ( -1.0 * cosY * sinX )
                + accelZRaw * ( cosX * cosY ))
                - gravity_[2];

  // d = v0 * t + (1/2) a * t^2
  incrementComulativeDistance(0, durationUs, xAccel);
  incrementComulativeDistance(1, durationUs, yAccel);
  incrementComulativeDistance(2, durationUs, zAccel);

  // v = v0 + a * t
  incrementComulativeVelocity(0, durationUs, xAccel);
  incrementComulativeVelocity(1, durationUs, yAccel);
  incrementComulativeVelocity(2, durationUs, zAccel);

  return micros() - tickStart;
}

void BpImu::incrementComulativeDistance(int index, int durationUs, float accel)
{
  float durationS = durationUs / 1.0e6;
  float distanceFromExistingVelocity = comulativeVelocity_[index] * durationS;
  float distanceFromMeasuredAcceleration = ((accel * durationS * durationS) / 2.0);

  if(overrun_[index])
  {
     // Don't bother trying to avoid overflow if we've already overflowd'd.
     comulativeDistance_[index] =  comulativeDistance_[index] + distanceFromExistingVelocity + distanceFromMeasuredAcceleration;
     return;
  }


  if(sign(comulativeVelocity_[index]) != sign(distanceFromExistingVelocity))
  {
    overrun_[index] = true;
  }

  if(sign(accel) != sign(distanceFromMeasuredAcceleration))
  {
      overrun_[index] = true;
  }

  // Adjust the order the 3 numbers are added to avoid overflow (if possible)
  float nextComulativeDistance;
  if(sign(comulativeDistance_[index]) != sign(distanceFromExistingVelocity))
  {
    nextComulativeDistance = comulativeDistance_[index] + distanceFromExistingVelocity;

    if(sign(nextComulativeDistance) != sign(distanceFromMeasuredAcceleration))
    {
      nextComulativeDistance = nextComulativeDistance + distanceFromMeasuredAcceleration;
    }
    else
    {
      nextComulativeDistance = nextComulativeDistance + distanceFromMeasuredAcceleration;
      if(sign(nextComulativeDistance) != sign(distanceFromMeasuredAcceleration))
      {
        overrun_[index] = true;
      }
    }
  }
  else if(sign(comulativeDistance_[index]) != sign(distanceFromMeasuredAcceleration))
  {
    nextComulativeDistance = comulativeDistance_[index] + distanceFromMeasuredAcceleration;
    if(sign(nextComulativeDistance) != sign(distanceFromExistingVelocity))
    {
      nextComulativeDistance = nextComulativeDistance + distanceFromExistingVelocity;
    }
    else
    {
      nextComulativeDistance = nextComulativeDistance + distanceFromExistingVelocity;
      if(sign(nextComulativeDistance) != sign(distanceFromExistingVelocity))
      {
        overrun_[index] = true;
      }
    }
  }
  else if(sign(distanceFromMeasuredAcceleration) != sign(distanceFromExistingVelocity))
  {
    nextComulativeDistance = distanceFromMeasuredAcceleration + distanceFromExistingVelocity;
    if(sign(nextComulativeDistance) != sign(comulativeDistance_[index]))
    {
      nextComulativeDistance = nextComulativeDistance + comulativeDistance_[index];
    }
    else
    {
      nextComulativeDistance = nextComulativeDistance + comulativeDistance_[index];
      if(sign(nextComulativeDistance) != sign(comulativeDistance_[index]))
      {
        overrun_[index] = true;
      }
    }
  }
  else
  {
    // All 3 have the same sign
    nextComulativeDistance = comulativeDistance_[index] + distanceFromExistingVelocity;
    if(sign(nextComulativeDistance) != sign(distanceFromExistingVelocity))
    {
      overrun_[index] = true;
    }

    nextComulativeDistance = nextComulativeDistance + distanceFromMeasuredAcceleration;
    if(sign(nextComulativeDistance) != sign(distanceFromMeasuredAcceleration))
    {
      overrun_[index] = true;
    }
  }

  comulativeDistance_[index] = nextComulativeDistance;
}

void BpImu::incrementComulativeVelocity(int index, int durationUs, float accel)
{
  float durationS = durationUs / 1.0e6;
  float velocityFromMeasuredAcceleration = accel * durationS;
  float nextComulativeVelocity = comulativeVelocity_[index] + velocityFromMeasuredAcceleration;

  if(sign(velocityFromMeasuredAcceleration) == sign(comulativeVelocity_[index]))
  {
    if(sign(nextComulativeVelocity) != sign(velocityFromMeasuredAcceleration))
    {
      overrun_[index] = true;
    }
  }

  comulativeVelocity_[index] = nextComulativeVelocity;
}
