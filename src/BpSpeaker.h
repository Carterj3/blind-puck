#ifndef BpSpeaker_h
#define BpSpeaker_h


class BpSpeaker
{
  public:
    BpSpeaker(int* pins, int length, int onDurationMs, int offDurationMs);

    void start();
    void stop();

    void tick();
  private:
    void togglePins(bool on);

    int* pins_;
    int length_;
    int onDurationMs_;
    int offDurationMs_;

    bool enabled_;
    int nextOnTimeMs_;
    int nextOffTimeMs_;
    int lastTime_;
};

#endif
