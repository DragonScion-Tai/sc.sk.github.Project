#ifndef __DIGITAL_SWITCH_H__
#define __DIGITAL_SWITCH_H__
#include "inputScanner.h"

class DigitalSwitches
{
public:
	static DigitalSwitches* getInstance();

	virtual int turnOnBeep(float sec);
	virtual void turnOffBeep(void);
	virtual int powerOffCentralPanel(bool bOff);
	virtual int turnOnInfraredSensor(bool bOn);
	virtual void hotWaterSwitchOpen(void);
	virtual void hotWaterSwitchClose(void);
	virtual void coldWaterSwitchOpen(void);
	virtual void coldWaterSwitchClose(void);
    virtual void iceWaterSwitchOpen(void);
    virtual void iceWaterSwitchClose(void);
	virtual void hotWaterPulseRise(void);
	virtual void hotWaterPulseFall(void);
	virtual void coldWaterPulseRise(void);
	virtual void coldWaterPulseFall(void);
    virtual void iceWaterPulseRise(void);
    virtual void iceWaterPulseFall(void);
	virtual void feedWaterPulseRise(void);
	virtual void feedWaterPulseFall(void);
	virtual void filterRinseInterrupt_posA(void);
	virtual void filterRinseInterrupt_posB(void);
	virtual void filterRinseInterrupt_posB_timeout();
	virtual int startFilterRinse(bool bOn);
	virtual int turnOnFeedWaterValve(bool bOn);
	virtual void kidSafetyButtonCallback(void);
	virtual void UVFaultCallback(void);
	virtual int openHotWater();
	virtual int closeHotWater();
    virtual bool hotWaterOpened();
	virtual int openColdWater();
	virtual int closeColdWater();
    virtual bool coldWaterOpened();
    virtual int openIceWater();
    virtual int closeIceWater();
    virtual bool iceWaterOpened();
	virtual int lightWarningLED(bool bOn);
	virtual int openDustHelmet(bool bOpen);

	virtual int initializeDigitalSwitches();

	virtual int process();
	virtual int process_one_second(time_t seconds);

	virtual bool infraredDetectedWorks();
	virtual bool feedWaterSwitchOpened();
    virtual bool dustHelmetIsOpened();

	virtual int getHotWaterValveState();
	virtual int getColdWaterValveState();
	virtual int getIceWaterValveState();

    // UV device power control
    virtual void powerOnUVDevice(bool bOn);

private:
	int startFilterRinseMotor(bool bOn);
	int stepDustHelmetMotor(char bType); //0: do nothing; 1: Open; 2: Close;

private:
	DigitalSwitches();
	virtual ~DigitalSwitches();
    friend class DebugSerial;

	static DigitalSwitches* pThis;

	// LED
	DigitalOut redLED;
	DigitalOut greenLED;
	// beep
	DigitalOut beep;
	// contral panel power control
	DigitalOut CPPowerControl;
	// qr control
	DigitalOut QRPowerControl;
	// infrared sensor
	DigitalIn infraredSensor0;
	DigitalIn infraredSensor1;
	// water switch and valve
#if USING_BUTTON_SCANNER
    InputScanner hotWaterSwitch;
    InputScanner coldWaterSwitch;
    InputScanner iceWaterSwitch;
#else
	InterruptIn hotWaterSwitch; //PB_0
    InterruptIn coldWaterSwitch; //PD_9
#if ENABLE_ICE_WATER
    InterruptIn iceWaterSwitch; //PD_8
#endif
#endif
	DigitalOut hotWaterValve;
	DigitalOut coldWaterValve;
#ifndef TDS_CONTROL_SWITCH
	DigitalOut iceWaterValve;
#endif
	// flow meter
	InterruptIn hotWaterFlowMeter; //PE_14
	InterruptIn coldWaterFlowMeter; //PE_13
#if ENABLE_ICE_WATER
    InterruptIn iceWaterFlowMeter; //PE_11
#endif
	InterruptIn feedWaterFlowMeter; //PC_8
	// filter rinse valve
	DigitalOut filterRinseMotor;
#if USING_BUTTON_SCANNER
    InputScanner filterRinsePosA;
    InputScanner filterRinsePosB;
#else
	InterruptIn filterRinsePosA; //PD_11
	InterruptIn filterRinsePosB; //PD_10
#endif
	// LED for water switch
	DigitalOut hotWaterSwitchLED;
	DigitalOut coldWaterSwitchLED;
#ifndef TDS_CONTROL_SWITCH
    DigitalOut iceWaterSwitchLED;
#endif
	// Feed Water(B3) valve
	DigitalOut feedWaterGreen;
	DigitalOut feedWaterRed;
	// Kid Safety button
#if USING_BUTTON_SCANNER
    InputScanner kidSafetyButton; //PB_1
#else
	InterruptIn kidSafetyButton; //PB_1
#endif
	// invasion detect
	DigitalIn invasionDetect0;
	//DigitalIn invasionDetect1;
	// Device water leak detect
	AnalogIn waterLeak0;
	AnalogIn waterLeak1;
	// UV device fault detect
	InterruptIn UVFault; //PD_3
    // UV device power control
    DigitalOut UVPowerControl;
	// Dust Helmit step motor and position switch
	BusOut dustHelmet;
#if USING_BUTTON_SCANNER
    InputScanner dustHelmetCloseSwitch;
    InputScanner dustHelmetOpenSwitch;
#else
	DigitalIn dustHelmetCloseSwitch;
	DigitalIn dustHelmetOpenSwitch;
#endif
	// TDS detect
	AnalogIn TDSDetect;

	Timeout beepTimeout;
	Timeout filterRinseTimeout;

	char filterRinsePosition; // 0: in-serv; 1: back wash; 2: fast wash;
	bool bFilterRinsePositionATriggered;
	bool bFilterRinsePositionBTriggered;
	Timer filterRinseTimer;
	int filterRinseLastTime;
	int filterRinseLoopCount;

	bool bWaterLeakDetected;
	bool bInfraredDetectEnabled;
	bool bInfraredDetected;
	int infraredDetectVanishedSec;
	bool bUVFaultInterrupt;
	bool bUVFaultDetected;
	int UVFaultDetectedSec;

	// invasion
	//void invasionSwitchClosed(void);
	//bool bInvasionDetected;

	// switch flags
	bool coldWaterSwitchPressed;
	bool hotWaterSwitchPressed;
    bool iceWaterSwitchPressed;
    bool bSwitchStateChanged;
	Timeout hotWaterSwitchTimeout;
	void hotWaterSwitchTimeoutFun(void);
	bool bHotWaterSwitchTimeoutTriggered;
    Timeout coldWaterSwitchTimeout;
	void coldWaterSwitchTimeoutFun(void);
	bool bColdWaterSwitchTimeoutTriggered;
    Timeout iceWaterSwitchTimeout;
    void iceWaterSwitchTimeoutFun(void);
    bool bIceWaterSwitchTimeoutTriggered;
	bool bCalculateColdCost;
	bool bCalculateHotCost;
    bool bCalculateIceCost;

	// flowmeter flags
	bool coldWaterPulseRisen;
	bool hotWaterPulseRisen;
    bool iceWaterPulseRisen;
	bool feedWaterPulseRisen;

	// dusthelmet
	int dustHelmetStep;
	int dustHelmetAllowedSteps;
	char bDustHelmetOpen; // 0: do nothing; 1: open; 2: close;
    char helmetTryCount;
    void setStepMotorSteps();
	void dustHelmetOpened(void);
	void dustHelmetClosed(void);
    Timeout closeHelmetDelayTimeout;
    void closeHelmetTimeoutFun(void);
    bool intentClosed;
    int ticksToNextRetry;
    char helmetErrorRetryCount;
    Timeout coldBtnPressedTimeout;
    void coldBtnPressedTimeoutFun(void);

	// TDS detect
	unsigned int TDSDetectSec;

    // Ticker; used for scan button/switch status;
    // instead of interrupt
    void scan(void);
    Ticker ticker;
    Timer timer;

    // UV Power control
#if ENABLE_UV_POWER_CONTROL
    Timeout UVPowerControlTimeout;
    void UVPowerControlTimeoutFun(void);
#endif

    // Kid Safety function
#if ENABLE_KID_SAFETY_FUNCTION
    bool KidSafetyProtected; //
    Timeout KidSafetyDelayTimeout;
    void KidSafetyDelayTimeoutFun(void);
    bool HoldToUnlockHotWaterProtected;
    Timeout HoldToUnlockHotWaterTimeout;
    void HoldToUnlockHotWaterTimeoutFun(void);
    bool HoldToUnlockIceWaterProtected;
    Timeout HoldToUnlockIceWaterTimeout;
    void HoldToUnlockIceWaterTimeoutFun(void);
#endif

#ifdef TDS_CONTROL_SWITCH
public:
	int getPurifiedWaterTDS();
#endif
};

#endif //__DIGITAL_SWITCH_H__
