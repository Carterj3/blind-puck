#include "BpSpeaker.h"

#include "Arduino.h"

BpSpeaker::BpSpeaker(int* pins, int length, int onDurationMs, int offDurationMs)
{
  pins_ = pins;
  length_ = length;
  onDurationMs_ = onDurationMs;
  offDurationMs_ = offDurationMs;

  enabled_ = false;
  nextOnTimeMs_ = 0;
  nextOffTimeMs_ = 0;
  lastTime_ = 0;

  for(int i=0;i<length_;i++)
  {
    pinMode(pins_[i], OUTPUT);
  }

  togglePins(false);
}

void BpSpeaker::start()
{
  enabled_ = true;

  lastTime_ = millis();
  nextOffTimeMs_ = lastTime_ + onDurationMs_;
  nextOnTimeMs_ = nextOffTimeMs_ + offDurationMs_;

  togglePins(true);
}

void BpSpeaker::stop()
{
  enabled_ = false;

  togglePins(false);
}

void BpSpeaker::togglePins(bool on){

  for(int i=0;i<length_;i++)
  {
    digitalWrite(pins_[i], on?HIGH:LOW);
  }
}

void BpSpeaker::tick()
{
  if(!enabled_){
    return;
  }

  if(lastTime_ > millis())
  {
    // Clock overflow
    togglePins(true);

    nextOffTimeMs_ = millis() + onDurationMs_;
    nextOnTimeMs_ = nextOffTimeMs_ + offDurationMs_;
  }

  lastTime_ = millis();

  if(millis() > nextOnTimeMs_)
  {
    togglePins(true);
    nextOnTimeMs_ = nextOffTimeMs_ + offDurationMs_;
  }else if(millis() > nextOffTimeMs_)
  {
    togglePins(false);
    nextOffTimeMs_ = nextOnTimeMs_ + onDurationMs_;
  }
}
