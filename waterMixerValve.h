#ifndef __WATER_MIXER_VALVE_H__
#define __WATER_MIXER_VALVE_H__

class CWaterMixerValve
{
public:
    CWaterMixerValve();
    virtual ~CWaterMixerValve();

    static CWaterMixerValve* getInstance();

    int initialize();
    int process_one_second();

    int setWantedTDS(unsigned int tds);
    unsigned int getWantedTDS() { return mWantedTDS; }
    int enableAdjustTDS(bool bEnable);

    int increaseTDS();
    int decreaseTDS();

    int setStep(int step);

    int waterValveState(unsigned char  outletId, int state);
    bool isMixerValveAdjusting() { return mColdWaterOpened > 0;}

    int test();

    // MixingValve Param
    unsigned int getCurrentPWMAngle();
    int setCurrentPWMAngle(unsigned int angle);

    unsigned int getPulseWidth() {return mPulseWidth;}
    bool getEnabled() {return mEnabled;}
    int outputRotationalPulse(unsigned int PulseWidth);
    int getSteeringAngle();
    int getMixedWaterTDS();

private:
    friend class CWaterQualityDetection;
    PwmOut mServSignal;
    DigitalOut mElectricityControl;

    static const unsigned int PULSE_WIDTH_MAX = 1700;
    static const unsigned int PULSE_WIDTH_MIN = 800;
    static const unsigned int TDS_COMPARE_TOLERANCE = 10;

    unsigned int mStepDelta;  // micro seconds
    unsigned int mColdWaterOpened;
    int count;
public:
    unsigned int currentTDS;
    unsigned int mWantedTDS;
    unsigned int mPulseWidth; // micro seconds
    bool mEnabled;
    int angle;

    bool ServoStartFlag;
    bool RawWaterFlag;
};

#endif //__WATER_MIXER_VALVE_H__
