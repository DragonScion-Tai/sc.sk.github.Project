#include "utils.h"

#ifdef TDS_CONTROL_SWITCH

#define LOG_MODULE 0x10
#define BKUPREG_PWM_ANGLE_DURATION_ADDR (BKPSRAM_BASE+64)

#define MAX_SET_TDS 1000
#define MIN_SET_TDS 0

#define CONFIRM_NUMBER 5

CWaterMixerValve gWaterMixerValve;
CWaterMixerValve* CWaterMixerValve::getInstance()
{
    return &gWaterMixerValve;
}

CWaterMixerValve::CWaterMixerValve()
    : mServSignal(HOT_WATER_VALVE)
    , mElectricityControl(HOT_WATER_SWITCH_LED, DIGITAL_SWITCH_OFF)
{
    //mPulseWidth = 1200;
    mPulseWidth = PULSE_WIDTH_MAX;

    mStepDelta = 50;

    //mWantedTDS = 150;
    mWantedTDS = 100;
    mEnabled = true;

    mColdWaterOpened = 0x00;
    ServoStartFlag = true;
    RawWaterFlag = false;
    count = 0;
}

CWaterMixerValve::~CWaterMixerValve()
{

}

int CWaterMixerValve::initialize()
{
    FTRACE("###CWaterMixerValve::initialize() mPulseWidth : %d\r\n",mPulseWidth);
    mServSignal.period_ms(20); // 200Hz

    //mPulseWidth = CWaterMixerValve::getInstance()->getCurrentPWMAngle();
    if(mPulseWidth < PULSE_WIDTH_MIN || mPulseWidth > PULSE_WIDTH_MAX)
    {
        mPulseWidth = PULSE_WIDTH_MAX;
        //CWaterMixerValve::getInstance()->setCurrentPWMAngle(mPulseWidth);
    }

    //mServSignal.pulsewidth_us(mPulseWidth);//200000-mPulseWidth
    return 0;
}

int CWaterMixerValve::process_one_second()
{
    FTRACE("------------------------------------------------------------------------------\n");
    FTRACE("###CWaterMixerValve::process_one_second() mEnabled: %d mColdWaterOpened: %d  pulseWidth: %d wantTDS: %d currentTDS: %d\r\n", mEnabled, mColdWaterOpened, mPulseWidth, mWantedTDS, currentTDS);
 #if 1
    mEnabled = true;
    if(mEnabled)
    {
        FTRACE("###CWaterMixerValve::process_one_second() ServoStartFlag: %d \n",ServoStartFlag);
        FTRACE("###CWaterMixerValve::process_one_second() RawWaterFlag: %d \n",RawWaterFlag);

        if(mColdWaterOpened && ServoStartFlag) // water is live
        {
            mElectricityControl = DIGITAL_SWITCH_ON;

            if(RawWaterFlag == true)
            {
                if(mPulseWidth > PULSE_WIDTH_MIN)
                {
                    mPulseWidth = PULSE_WIDTH_MIN;
                    CWaterMixerValve::getInstance()->outputRotationalPulse(mPulseWidth);
                }

                currentTDS += DigitalSwitches::getInstance()->getPurifiedWaterTDS();
                count++;
                if(count >= 5)
                {
                    currentTDS /= count;
                    
                    // upload water quality
                    Action* pAct = new ActionWaterQualityInfo(0, currentTDS);
                    ActionBase::append(pAct);
                    FTRACE("###CWaterMixerValve::process_one_second() yuanshui currentTDS %d\n",currentTDS);
                    count = 0;
                    RawWaterFlag = false;
                }
            }
            else{
                currentTDS = DigitalSwitches::getInstance()->getPurifiedWaterTDS();

                if(currentTDS > mWantedTDS + 4 * TDS_COMPARE_TOLERANCE || currentTDS < mWantedTDS - 4 * TDS_COMPARE_TOLERANCE)
                {
                    mStepDelta = 100;
                }
                else if(currentTDS > mWantedTDS + 2 * TDS_COMPARE_TOLERANCE || currentTDS < mWantedTDS - 2 * TDS_COMPARE_TOLERANCE)
                {
                    mStepDelta = 50;
                }
                else
                {
                    mStepDelta = 20;
                }
                
                if(currentTDS > mWantedTDS + TDS_COMPARE_TOLERANCE)
                {
                    increaseTDS();
                    count = 0;
                }
                else if(currentTDS < mWantedTDS - TDS_COMPARE_TOLERANCE)
                {
                    decreaseTDS();
                    count = 0;
                }
                else{
                    count++;
                }

                if(count > CONFIRM_NUMBER)
                {
                    currentTDS = DigitalSwitches::getInstance()->getPurifiedWaterTDS();
                    angle = CWaterMixerValve::getInstance()->getSteeringAngle();

                    FTRACE("###CWaterMixerValve::process_one_second() currentTDS %d     angle: %d \n",currentTDS,angle);
                    // upload water quality
                    Action* pAct = new ActionWaterQualityInfo(1, currentTDS);
                    ActionBase::append(pAct);

                    // // Action* pAct = new ActionSteeringAngle(angle);
                    // // ActionBase::append(pAct);
                    CentralPanelSerial::getInstance()->uploadSteeringAngle(angle);
                    
                    mElectricityControl = DIGITAL_SWITCH_OFF;
                    ServoStartFlag = false;
                    count = 0;
                }
            }
        }
    }
    if(!mEnabled || !mColdWaterOpened || !ServoStartFlag 
        || (mPulseWidth == PULSE_WIDTH_MAX || mPulseWidth == PULSE_WIDTH_MIN))
    {
        if(RawWaterFlag == false)
            mElectricityControl = DIGITAL_SWITCH_OFF;
    }
    return 0;
#else    
    mElectricityControl = DIGITAL_SWITCH_ON;
    if(mPulseWidth <= PULSE_WIDTH_MAX && mEnabled)
    {
        increaseTDS();
        if(mPulseWidth == PULSE_WIDTH_MAX)
            mEnabled = false;
    }
    else if(mPulseWidth >= PULSE_WIDTH_MIN && !mEnabled)
    {
        decreaseTDS();
        if(mPulseWidth == PULSE_WIDTH_MIN)
            mEnabled = true;
    }
    return 0;
#endif
}

int CWaterMixerValve::setWantedTDS(unsigned int tds)
{
    if(MIN_SET_TDS > tds)
    {
        mWantedTDS = MIN_SET_TDS;
    }
    else if(MAX_SET_TDS < tds)
    {
        mWantedTDS = MAX_SET_TDS;
    }
    else
    {
        mWantedTDS = tds;
    }
    ServoStartFlag = true;
    PersistentStorage::getInstance()->saveTDSValue();
    return 0;
}

int CWaterMixerValve::enableAdjustTDS(bool bEnable)
{
    mEnabled = bEnable;
    PersistentStorage::getInstance()->saveTDS_On_Off();
    return 0;
}

int CWaterMixerValve::decreaseTDS()//往超滤方向转
{
     FTRACE("###CWaterMixerValve::decreaseTDS()\r\n");
    if(mPulseWidth > PULSE_WIDTH_MIN)
    {
        mPulseWidth -= mStepDelta;
        mPulseWidth = (mPulseWidth / 10) * 10;
        if(mPulseWidth < PULSE_WIDTH_MIN)
            mPulseWidth = PULSE_WIDTH_MIN;
        CWaterMixerValve::getInstance()->outputRotationalPulse(mPulseWidth);
        FTRACE("mPulseWidth: %d\r\n",mPulseWidth);
        PersistentStorage::getInstance()->savePulseWidth();
    }
    return 0;
}

int CWaterMixerValve::increaseTDS()//往纳滤方向转
{
     FTRACE("###CWaterMixerValve::increaseTDS()\r\n");
    if(mPulseWidth < PULSE_WIDTH_MAX)
    {
        mPulseWidth += mStepDelta;
        mPulseWidth = (mPulseWidth / 10) * 10;
        if(mPulseWidth > PULSE_WIDTH_MAX)
            mPulseWidth = PULSE_WIDTH_MAX;
        CWaterMixerValve::getInstance()->outputRotationalPulse(mPulseWidth);
        FTRACE("mPulseWidth: %d\r\n",mPulseWidth);
        PersistentStorage::getInstance()->savePulseWidth();
    }
    return 0;
}

int CWaterMixerValve::setStep(int step)
{
    mStepDelta = step;
    return 0;
}

int CWaterMixerValve::waterValveState(unsigned char  outletId, int state)
{
    if(state == 0x01) // pressed
    {
        mColdWaterOpened |= 0x01 << outletId;
    }
    else
    {
        mColdWaterOpened &= ~(0x01 << outletId);
    }

    return 0;
}

int CWaterMixerValve::test()
{
    mPulseWidth = 1000;
    CWaterMixerValve::getInstance()->setCurrentPWMAngle(mPulseWidth);
    mServSignal.pulsewidth_us(mPulseWidth);
    return 0;
}

unsigned int CWaterMixerValve::getCurrentPWMAngle()
{
    return *(unsigned int*)BKUPREG_PWM_ANGLE_DURATION_ADDR;
}

int CWaterMixerValve::setCurrentPWMAngle(unsigned int angle)
{
    *(unsigned int*)BKUPREG_PWM_ANGLE_DURATION_ADDR = angle;
    return 0;
}

int CWaterMixerValve::outputRotationalPulse(unsigned int PulseWidth)
{
    CWaterMixerValve::getInstance()->setCurrentPWMAngle(PulseWidth);
    mServSignal.pulsewidth_us(PulseWidth);
    return 0;
}

int CWaterMixerValve::getSteeringAngle()
{
    int getAngle,PulseWidth;
    
    PulseWidth = CWaterMixerValve::getInstance()->mPulseWidth;

    getAngle = (PulseWidth - PULSE_WIDTH_MIN) / ((PULSE_WIDTH_MAX - PULSE_WIDTH_MIN) / 90);

    return getAngle;
}

int CWaterMixerValve::getMixedWaterTDS()
{
    unsigned int mixedTDS;

    mElectricityControl = DIGITAL_SWITCH_ON;

    mixedTDS = DigitalSwitches::getInstance()->getPurifiedWaterTDS();

    // upload water quality
    Action* pAct = new ActionWaterQualityInfo(1, mixedTDS);
    ActionBase::append(pAct);

    mElectricityControl = DIGITAL_SWITCH_OFF;

    return 0;
}

#endif