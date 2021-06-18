#include "utils.h"

#define LOG_MODULE 0x07

DigitalSwitches* DigitalSwitches::pThis = NULL;
DigitalSwitches* DigitalSwitches::getInstance()
{
	if(!pThis)
		pThis = new DigitalSwitches();

	return pThis;
}

DigitalSwitches::DigitalSwitches()
	: redLED(LED_LIGHT_RED, DIGITAL_SWITCH_ON)
	, greenLED(LED_LIGHT_GREEN, DIGITAL_SWITCH_OFF)
	, beep(BEEP_PIN_ADDR, DIGITAL_SWITCH_OFF)
	, CPPowerControl(CENTRLPANEL_POWER_CONTROL, DIGITAL_SWITCH_OFF)
	, QRPowerControl(QRCODE_SCAN_POWER_CONTROL, DIGITAL_SWITCH_ON)
	, infraredSensor0(INFRARED_SENSOR_PIN_ADDR_0)
	, infraredSensor1(INFRARED_SENSOR_PIN_ADDR_1)
	, hotWaterSwitch(ICE_WATER_SWITCH)
	, coldWaterSwitch(COLD_WATER_SWITCH)
#if ENABLE_ICE_WATER
    , iceWaterSwitch(HOT_WATER_SWITCH)
#endif
	, hotWaterValve(ICE_WATER_VALVE, DIGITAL_SWITCH_OFF)
	, coldWaterValve(COLD_WATER_VALVE, DIGITAL_SWITCH_OFF)
#ifndef TDS_CONTROL_SWITCH
    , iceWaterValve(HOT_WATER_VALVE, DIGITAL_SWITCH_OFF)
#endif
	, hotWaterFlowMeter(ICE_WATER_FLOW_METER)
	, coldWaterFlowMeter(COLD_WATER_FLOW_METER)
#if ENABLE_ICE_WATER
    , iceWaterFlowMeter(HOT_WATER_FLOW_METER)
#endif
	, feedWaterFlowMeter(FEED_WATER_FLOW_METER)
	, filterRinseMotor(FILTER_RINSE_MOTOR, DIGITAL_SWITCH_OFF)
	, filterRinsePosA(FILTER_RINSE_POS_A)
	, filterRinsePosB(FILTER_RINSE_POS_B)
	, hotWaterSwitchLED(ICE_WATER_SWITCH_LED, DIGITAL_SWITCH_OFF)
	, coldWaterSwitchLED(COLD_WATER_SWITCH_LED, DIGITAL_SWITCH_OFF)
#ifndef TDS_CONTROL_SWITCH
    , iceWaterSwitchLED(HOT_WATER_SWITCH_LED, DIGITAL_SWITCH_OFF)
#endif
	, feedWaterGreen(FEED_WATER_GREEN_OUT, DIGITAL_SWITCH_ON)
	, feedWaterRed(FEED_WATER_RED_OUT, DIGITAL_SWITCH_OFF)
	, kidSafetyButton(KID_SAFETY_BUTTON)
	, invasionDetect0(INVASION_DETECT_0)
	//, invasionDetect1(INVASION_DETECT_1)
	, waterLeak0(DEVICE_WATER_LEAK_0)
	, waterLeak1(DEVICE_WATER_LEAK_1)
	, UVFault(UV_DEVICE_FAULT)
    , UVPowerControl(UV_POWER_CONTROL, DIGITAL_SWITCH_OFF)
	, dustHelmet(STEP_MOTOR_ORANGE, STEP_MOTOR_YELLOW, STEP_MOTOR_PINK, STEP_MOTOR_BLUE)
	, dustHelmetCloseSwitch(DUST_HELMET_CLOSED)
	, dustHelmetOpenSwitch(DUST_HELMET_OPENED)
	, TDSDetect(TDS_IN)
{
	pThis = this;

	filterRinsePosition = 4;
	bFilterRinsePositionATriggered = false;
	bFilterRinsePositionBTriggered = false;
	filterRinseLastTime = 0;
	filterRinseLoopCount = 0;

	bWaterLeakDetected = false;
	bInfraredDetectEnabled = true;
	bInfraredDetected = false;
	infraredDetectVanishedSec = 0;
	bUVFaultInterrupt = false;
	bUVFaultDetected = false;
	UVFaultDetectedSec = 0;
	//bInvasionDetected = false;

	coldWaterSwitchPressed = false;
	hotWaterSwitchPressed = false;
    iceWaterSwitchPressed = false;
    bSwitchStateChanged = false;
	bHotWaterSwitchTimeoutTriggered = false;
    bColdWaterSwitchTimeoutTriggered = false;
    bIceWaterSwitchTimeoutTriggered = false;
	bCalculateColdCost = false;
	bCalculateHotCost = false;
    bCalculateIceCost = false;

	coldWaterPulseRisen = false;
	hotWaterPulseRisen = false;
    iceWaterPulseRisen = false;
	feedWaterPulseRisen = false;

	dustHelmetStep = 0;
	bDustHelmetOpen = 0;
	dustHelmetAllowedSteps = 0;
    helmetTryCount = 0;
    intentClosed = true;
    ticksToNextRetry = 0;
    helmetErrorRetryCount = 0;

	TDSDetectSec = TDS_DETECT_INTERVAL;

#if ENABLE_KID_SAFETY_FUNCTION
    KidSafetyProtected = true; // default hot water is forbidden by kid safty
    HoldToUnlockHotWaterProtected = true;
    HoldToUnlockIceWaterProtected = true;
#endif

//#if USING_BUTTON_SCANNER
//	ticker.attach_us(Callback<void()>(this, &DigitalSwitches::scan), 5000); // 5 ms
//#endif
    timer.start();
}

DigitalSwitches::~DigitalSwitches()
{

}

int DigitalSwitches::turnOnBeep(float sec)
{
	// start timer
	beepTimeout.attach(Callback<void()>(this, &DigitalSwitches::turnOffBeep), sec);
	// turn on beep
	beep = DIGITAL_SWITCH_ON;

	return 0;
}

void DigitalSwitches::turnOffBeep(void)
{
	// turn off beep
	beep = DIGITAL_SWITCH_OFF;

	beepTimeout.detach();
}

int DigitalSwitches::powerOffCentralPanel(bool bOff)
{
	if(bOff)
		CPPowerControl = DIGITAL_SWITCH_ON;
	else
		CPPowerControl = DIGITAL_SWITCH_OFF;

	return 0;
}

int DigitalSwitches::turnOnInfraredSensor(bool bOn)
{
	bInfraredDetectEnabled = bOn;
	if(!bInfraredDetectEnabled)
	{
		bInfraredDetected = false;
		infraredDetectVanishedSec = 0;
	}

	return 0;
}

int DigitalSwitches::openHotWater()
{
	if(DIGITAL_SWITCH_OFF == hotWaterValve)
		hotWaterValve = DIGITAL_SWITCH_ON;

	return 0;
}

int DigitalSwitches::closeHotWater()
{
	if(DIGITAL_SWITCH_ON == hotWaterValve)
		hotWaterValve = DIGITAL_SWITCH_OFF;

	return 0;
}

bool DigitalSwitches::hotWaterOpened()
{
    return DIGITAL_SWITCH_ON == hotWaterValve;
}

int DigitalSwitches::openColdWater()
{
	if(DIGITAL_SWITCH_OFF == coldWaterValve)
		coldWaterValve = DIGITAL_SWITCH_ON;

	return 0;
}

int DigitalSwitches::closeColdWater()
{
	if(DIGITAL_SWITCH_ON == coldWaterValve)
		coldWaterValve = DIGITAL_SWITCH_OFF;

	return 0;
}

bool DigitalSwitches::coldWaterOpened()
{
    return DIGITAL_SWITCH_ON == coldWaterValve;
}

int DigitalSwitches::openIceWater()
{
#ifndef TDS_CONTROL_SWITCH
     if(DIGITAL_SWITCH_OFF == iceWaterValve)
         iceWaterValve = DIGITAL_SWITCH_ON;
#endif
    return 0;
}

int DigitalSwitches::closeIceWater()
{
#ifndef TDS_CONTROL_SWITCH
     if(DIGITAL_SWITCH_ON == iceWaterValve)
         iceWaterValve = DIGITAL_SWITCH_OFF;
#endif
    return 0;
}

bool DigitalSwitches::iceWaterOpened()
{
#ifndef TDS_CONTROL_SWITCH
     return DIGITAL_SWITCH_ON == iceWaterValve;
#endif
}

void DigitalSwitches::hotWaterSwitchOpen(void)
{
	//FTRACE("###DigitalSwitches::hotWaterSwitchOpen() \r\n");
#if ENABLE_KID_SAFETY_FUNCTION
    if(Controller::getInstance()->isKidSafetyEnabledDevice())
    {
        if(KidSafetyProtected)
        {
            return;
        }
        else
        {
            // unlock
            KidSafetyDelayTimeout.detach();
            KidSafetyProtected = false; // reset;
        }
    }
    else if(Controller::getInstance()->isHoldToUnlockHotWaterDevice())
    {
        if(HoldToUnlockHotWaterProtected)
        {
            // protected
            //FTRACE("###DigitalSwitches::hotWaterSwitchOpen() Hold To Unlock: protected\r\n");
            HoldToUnlockHotWaterTimeout.attach(Callback<void()>(this, &DigitalSwitches::HoldToUnlockHotWaterTimeoutFun),
                                                HOLD_TO_UNLOCK_DURATION);
            return;
        }
        else
        {
            //FTRACE("###DigitalSwitches::hotWaterSwitchOpen() Hold To Unlock: unprotected\r\n");
            HoldToUnlockHotWaterTimeout.detach(); // cancel count down
            HoldToUnlockHotWaterProtected = false;
        }
    }
#endif

    // only one delivery port for SK-I
#ifdef TDS_CONTROL_SWITCH
	if(Controller::getInstance()->isSKIDevice() && coldWaterValve.read())
		return ;
#else
	if(Controller::getInstance()->isSKIDevice() && (coldWaterValve.read() || iceWaterValve.read()))
		return ;
#endif
	// check account
	if(Controller::getInstance()->checkAccountForHotWaterSelling())
    {
        FTRACE("###DigitalSwitches::hotWaterSwitchOpen() failed, reason: %d\r\n",
            Controller::getInstance()->checkAccountForHotWaterSelling());
		return;
    }

#if ENABLE_CHECK_HELTMET_OPENED
    if(Controller::getInstance()->CPHotPrice == 0) // free mode
    {
        // cancel close procedure
        closeHelmetDelayTimeout.detach();
        bDustHelmetOpen = 0;
    }
    if(dustHelmetOpenSwitch.read() != 0) // not opened
    {
        // notify Open Dust Helmet first
        return;
    }
#endif

    FTRACE("-------- ###DigitalSwitches::hotWaterSwitchOpen() \r\n");
	// turn on LED
	hotWaterSwitchLED = DIGITAL_SWITCH_ON;

	// relese hot water
	hotWaterValve = DIGITAL_SWITCH_ON;

	//
	bCalculateHotCost = true;
// #ifdef TDS_CONTROL_SWITCH
// 	CWaterMixerValve::getInstance()->waterValveState(1, 1);
// #endif
}

#if ENABLE_KID_SAFETY_FUNCTION
void DigitalSwitches::HoldToUnlockHotWaterTimeoutFun(void)
{
    //FTRACE("###DigitalSwitches::HoldToUnlockHotWaterTimeoutFun() HoldToUnlockHotWaterProtected: %d\r\n", HoldToUnlockHotWaterProtected);
    if(HoldToUnlockHotWaterProtected)
    {
        // hold to unlock
        HoldToUnlockHotWaterProtected = false;
	hotWaterSwitchLED = DIGITAL_SWITCH_ON;
    }
    else
    {
        // count down to lock
        HoldToUnlockHotWaterProtected = true;
    }
}
#endif

void DigitalSwitches::hotWaterSwitchClose(void)
{
	//FTRACE("###DigitalSwitches::hotWaterSwitchClose() \r\n");

	// stop hot water
	hotWaterValve = DIGITAL_SWITCH_OFF;

	// turn off LED
	hotWaterSwitchLED = DIGITAL_SWITCH_OFF;
// #ifdef TDS_CONTROL_SWITCH
// 	CWaterMixerValve::getInstance()->waterValveState(1, 0);
// #endif

#if ENABLE_KID_SAFETY_FUNCTION
    if(Controller::getInstance()->isHoldToUnlockHotWaterDevice())
    {
        if(HoldToUnlockHotWaterProtected)
        {
            //FTRACE("###DigitalSwitches::hotWaterSwitchClose() Hold To Unlock: protected\r\n");
            HoldToUnlockHotWaterTimeout.detach();
            HoldToUnlockHotWaterProtected = true;
        }
        else
        {
            //FTRACE("###DigitalSwitches::hotWaterSwitchClose() Hold To Unlock: unprotected\r\n");
            // start to count down
            HoldToUnlockHotWaterTimeout.attach(Callback<void()>(this, &DigitalSwitches::HoldToUnlockHotWaterTimeoutFun),
                                                KID_SAFETY_DELAY_SECOND);
        }
    }
#endif

#if ENABLE_CHECK_HELTMET_OPENED
    if(Controller::getInstance()->CPHotPrice == 0)
    {
        if(Controller::getInstance()->isRestrictHotWaterDevice() &&
            (Controller::getInstance()->CPUserInfo.accountState & 0x01) == 0x01)
            return ; // not close if account is activated for hot water restrict device

        // close dust helmet
        openDustHelmet(false);
    }
#endif
}

void DigitalSwitches::coldWaterSwitchOpen(void)
{
	//FTRACE("###DigitalSwitches::coldWaterSwitchOpen() \r\n");

    // only one delivery port for SKI
#ifdef TDS_CONTROL_SWITCH
	if(Controller::getInstance()->isSKIDevice() && hotWaterValve.read())
		return ;
#else
	if(Controller::getInstance()->isSKIDevice() && (hotWaterValve.read() || iceWaterValve.read()))
		return ;
#endif

	// check account 
	if(Controller::getInstance()->checkAccountForColdWaterSelling())
	{
#if ENABLE_CHECK_HELTMET_OPENED
        // if cold water if not allowed but hot water allowed
        // open dust helmet if press cold water button
		if(!Controller::getInstance()->checkAccountForHotWaterSelling())
        {
            closeHelmetDelayTimeout.detach();
            bDustHelmetOpen = 0;
            if(dustHelmetOpenSwitch.read() != 0)
            {
                openDustHelmet(true);
            }
        }
#endif

        FTRACE("###DigitalSwitches::coldWaterSwitchOpen() failed, reason: %d\r\n",
            Controller::getInstance()->checkAccountForColdWaterSelling());

		return ;
	}

#if ENABLE_CHECK_HELTMET_OPENED
    if(Controller::getInstance()->CPColdPrice == 0) // free mode
    {
        closeHelmetDelayTimeout.detach();
        bDustHelmetOpen = 0;
        if(dustHelmetOpenSwitch.read() != 0)
        {
            openDustHelmet(true);
            coldBtnPressedTimeout.attach(Callback<void()>(this, &DigitalSwitches::coldBtnPressedTimeoutFun), 3); // 3sec
            return ;
        }
    }
    else if(dustHelmetOpenSwitch.read() != 0) // not opened
    {
        return ;
    }
#endif

	// turn on LED
	coldWaterSwitchLED = DIGITAL_SWITCH_ON;

	// relese cold water
	coldWaterValve = DIGITAL_SWITCH_ON;

	//
	bCalculateColdCost = true;

#ifdef TDS_CONTROL_SWITCH
	CWaterMixerValve::getInstance()->waterValveState(1, 1);
#endif
}

void DigitalSwitches::coldBtnPressedTimeoutFun(void)
{
    // turn on LED
	coldWaterSwitchLED = DIGITAL_SWITCH_ON;

	// relese cold water
	coldWaterValve = DIGITAL_SWITCH_ON;

	//
	bCalculateColdCost = true;
}

void DigitalSwitches::coldWaterSwitchClose(void)
{
	//FTRACE("###DigitalSwitches::coldWaterSwitchClose() \r\n");
	// stop cold water
	coldWaterValve = DIGITAL_SWITCH_OFF;

	// turn off LED
	coldWaterSwitchLED = DIGITAL_SWITCH_OFF;
	
#ifdef TDS_CONTROL_SWITCH
	CWaterMixerValve::getInstance()->waterValveState(1, 0);
#endif

#if ENABLE_CHECK_HELTMET_OPENED
    if(Controller::getInstance()->CPColdPrice == 0)
    {
        coldBtnPressedTimeout.detach();
        if(Controller::getInstance()->isRestrictHotWaterDevice() &&
            (Controller::getInstance()->CPUserInfo.accountState & 0x01) == 0x01)
            return ; // not close if account is activated for hot water restrict device

        // close dust helmet
        openDustHelmet(false);
    }
#endif
}

void DigitalSwitches::iceWaterSwitchOpen(void)
{
    //FTRACE("###DigitalSwitches::iceWaterSwitchOpen() \r\n");

#if ENABLE_KID_SAFETY_FUNCTION
    if(Controller::getInstance()->isKidSafetyEnabledDevice())
    {
        if(KidSafetyProtected)
        {
            return;
        }
        else
        {
            // unlock
            KidSafetyDelayTimeout.detach();
            KidSafetyProtected = false; // reset;
        }
    }
    else if(Controller::getInstance()->isHoldToUnlockHotWaterDevice())
    {
        if(HoldToUnlockIceWaterProtected)
        {
            // protected
            HoldToUnlockIceWaterTimeout.attach(Callback<void()>(this, &DigitalSwitches::HoldToUnlockIceWaterTimeoutFun),
                                                HOLD_TO_UNLOCK_DURATION);
            return;
        }
        else
        {
            HoldToUnlockIceWaterTimeout.detach(); // cancel count down
            HoldToUnlockIceWaterProtected = false;
        }
    }
#endif

    // only one delivery port for SKI
	if(Controller::getInstance()->isSKIDevice() && (hotWaterValve.read() || coldWaterValve.read()))
		return ;

	// check account 
	if(Controller::getInstance()->checkAccountForIceWaterSelling())
    {
        FTRACE("###DigitalSwitches::iceWaterSwitchOpen() failed, reason: %d\r\n",
            Controller::getInstance()->checkAccountForIceWaterSelling());

		return ;
    }

#if ENABLE_CHECK_HELTMET_OPENED
    if(Controller::getInstance()->CPHotPrice == 0) // free mode
    {
        // cancel close procedure
        closeHelmetDelayTimeout.detach();
        bDustHelmetOpen = 0;
    }

    if(dustHelmetOpenSwitch.read() != 0) // not opened
    {
        // notify Press cold water button to open dust helmet first
        return ;
    }
#endif

#ifndef TDS_CONTROL_SWITCH
	// turn on LED
	iceWaterSwitchLED = DIGITAL_SWITCH_ON;

	// relese ice water
	iceWaterValve = DIGITAL_SWITCH_ON;
#endif
	//
	bCalculateIceCost = true;
}

#if ENABLE_KID_SAFETY_FUNCTION
void DigitalSwitches::HoldToUnlockIceWaterTimeoutFun(void)
{
    if(HoldToUnlockIceWaterProtected)
    {
        // hold to unlock
        HoldToUnlockIceWaterProtected = false;
	    // turn on LED
		#ifndef TDS_CONTROL_SWITCH
	    iceWaterSwitchLED = DIGITAL_SWITCH_ON;
		#endif
    }
    else
    {
        // count down to lock
        HoldToUnlockIceWaterProtected = true;
    }
}
#endif

void DigitalSwitches::iceWaterSwitchClose(void)
{
    //FTRACE("###DigitalSwitches::iceWaterSwitchClose() \r\n");

#ifndef TDS_CONTROL_SWITCH
    // stop ice water
	iceWaterValve = DIGITAL_SWITCH_OFF;

	// turn off LED
	iceWaterSwitchLED = DIGITAL_SWITCH_OFF;
#endif

#if ENABLE_KID_SAFETY_FUNCTION
    if(Controller::getInstance()->isHoldToUnlockHotWaterDevice())
    {
        if(HoldToUnlockIceWaterProtected)
        {
            HoldToUnlockIceWaterTimeout.detach();
            HoldToUnlockIceWaterProtected = true;
        }
        else
        {
            // start to count down
            HoldToUnlockIceWaterTimeout.attach(Callback<void()>(this, &DigitalSwitches::HoldToUnlockIceWaterTimeoutFun),
                                                KID_SAFETY_DELAY_SECOND);
        }
    }
#endif

#if ENABLE_CHECK_HELTMET_OPENED
    if(Controller::getInstance()->CPHotPrice == 0)
    {
        if(Controller::getInstance()->isRestrictHotWaterDevice() &&
            (Controller::getInstance()->CPUserInfo.accountState & 0x01) == 0x01)
            return ; // not close if account is activated for hot water restrict device

        // close dust helmet
        openDustHelmet(false);
    }
#endif
}

void DigitalSwitches::hotWaterPulseRise(void)
{
	//FTRACE("###DigitalSwitches::hotWaterPulseRise()\r\n");
	if(bCalculateHotCost) // acculate pulse only when release hot water
		hotWaterPulseRisen = true;
}

void DigitalSwitches::hotWaterPulseFall(void)
{
	//FTRACE("###DigitalSwitches::hotWaterPulseFall()\r\n");
	if(hotWaterPulseRisen)
	{
		hotWaterPulseRisen = false;
		Controller::getInstance()->hotWaterFlowPulse ++;
	}
}

void DigitalSwitches::coldWaterPulseRise(void)
{
	//FTRACE("###DigitalSwitches::coldWaterPulseRise()\r\n");
	if(bCalculateColdCost) // acculate pulse only when release cold water
		coldWaterPulseRisen = true;
}

void DigitalSwitches::coldWaterPulseFall(void)
{
	//FTRACE("###DigitalSwitches::coldWaterPulseFall()\r\n");
	if(coldWaterPulseRisen)
	{
		coldWaterPulseRisen = false;
		Controller::getInstance()->coldWaterFlowPulse ++;
	}
}

void DigitalSwitches::iceWaterPulseRise(void)
{
    //FTRACE("###DigitalSwitches::iceWaterPulseRise()\r\n");
    if(bCalculateIceCost) // acculate pulse only when release ice water
		hotWaterPulseRisen = true;
}

void DigitalSwitches::iceWaterPulseFall(void)
{
    //FTRACE("###DigitalSwitches::iceWaterPulseFall()\r\n");
    if(hotWaterPulseRisen)
	{
		hotWaterPulseRisen = false;
		Controller::getInstance()->iceWaterFlowPulse ++;
	}
}

void DigitalSwitches::feedWaterPulseRise(void)
{
	//FTRACE("###DigitalSwitches::feedWaterPulseRise()\r\n");
	feedWaterPulseRisen = true;
}

void DigitalSwitches::feedWaterPulseFall(void)
{
	//FTRACE("###DigitalSwitches::feedWaterPulseFall()\r\n");
	if(feedWaterPulseRisen)
	{
		feedWaterPulseRisen = false;
		Controller::getInstance()->feedWaterFlowPulse ++;
	}
}

void DigitalSwitches::filterRinseInterrupt_posA(void)
{
	//FTRACE("###DigitalSwitches::filterRinseInterrupt_posA() filterRinsePosition: %d\r\n", filterRinsePosition);
	// reach In service position;
	if(filterRinseTimer.read_ms() > filterRinseLastTime)
	{
		filterRinseLastTime = filterRinseTimer.read_ms() + 1000;

		bFilterRinsePositionATriggered = true;
		// stop motor
		startFilterRinseMotor(false);
	}
}

void DigitalSwitches::filterRinseInterrupt_posB(void)
{
	//FTRACE("###DigitalSwitches::filterRinseInterrupt_posB() filterRinsePosition: %d\r\n", filterRinsePosition);
	if(filterRinseTimer.read_ms()-filterRinseLastTime > 2000) // 2s
	{
		bFilterRinsePositionBTriggered = true;

		filterRinsePosition ++;
		filterRinseLastTime = filterRinseTimer.read_ms();

		if(filterRinsePosition < 3)
		{
			// stop motor for filter rinse duration;
			startFilterRinseMotor(false);
		}
	}
}

void DigitalSwitches::filterRinseInterrupt_posB_timeout()
{
	//
	filterRinseLastTime = filterRinseTimer.read_ms() + 1000; // 1 sec to avoid start interrupt

	startFilterRinseMotor(true);
}

int DigitalSwitches::startFilterRinse(bool bOn)
{
	LOG_INFO("###DigitalSwitches::startFilterRinse() bOn: %d \r\n", bOn);

	filterRinseTimer.start();
	filterRinseLastTime = filterRinseTimer.read_ms() + 1000; // 1 sec to avoid start interrupt

	if(bOn)
	{
		filterRinsePosition = 0; // assume it is on in-serv
		startFilterRinseMotor(true);
	}
	else
	{
		filterRinsePosition = 5; // forced to stop; set bigger than 4, no stopped position;
		filterRinseLoopCount = Controller::getInstance()->filterRinseMaxCount; // forced to stop; set to be MAX count;
		filterRinseTimeout.detach();
		startFilterRinseMotor(true); // go back to in-serv position

        Controller::getInstance()->SetCurRinseFixTimesindex(0xFF);
	}

	return 0;
}

int DigitalSwitches::startFilterRinseMotor(bool bOn)
{
	//FTRACE("###DigitalSwitches::startFilterRinseMotor() bOn=%d\r\n", bOn);

	if(bOn)
		filterRinseMotor = DIGITAL_SWITCH_ON;
	else
		filterRinseMotor = DIGITAL_SWITCH_OFF;
	
	return 0;
}

int DigitalSwitches::turnOnFeedWaterValve(bool bOn)
{
	if(bOn)
	{
		feedWaterRed = DIGITAL_SWITCH_OFF;
		feedWaterGreen = DIGITAL_SWITCH_ON;
	}
	else
	{
		feedWaterGreen = DIGITAL_SWITCH_OFF;
		feedWaterRed = DIGITAL_SWITCH_ON;
	}

	return 0;
}

void DigitalSwitches::kidSafetyButtonCallback(void)
{
#if ENABLE_KID_SAFETY_FUNCTION
	//FTRACE("###DigitalSwitches::kidSafetyButtonCallback() \r\n");
    KidSafetyProtected = false;
    KidSafetyDelayTimeout.attach(Callback<void()>(this, &DigitalSwitches::KidSafetyDelayTimeoutFun), KID_SAFETY_DELAY_SECOND);
#endif
}

void DigitalSwitches::UVFaultCallback(void)
{
	//FTRACE("###DigitalSwitches::UVFaultCallback() \r\n");
	bUVFaultInterrupt = true;
}

//void DigitalSwitches::invasionSwitchClosed(void)
//{
	//FTRACE("###DigitalSwitches::invasionSwitchClosed() %d\r\n", invasionDetect0.read());
//	if(!Controller::getInstance()->CPDoorOpenForbidden) // authorized to open
//	{
//		if(invasionDetect0.read() == 0) // should be closed
//		{
//			Controller::getInstance()->CPDoorOpenForbidden = true; // unauthorized
//		}
//	}
//}

int DigitalSwitches::initializeDigitalSwitches()
{
	//
#if USING_BUTTON_SCANNER
    hotWaterSwitch.setHandler(Callback<void()>(this, &DigitalSwitches::hotWaterSwitchOpen), 
                                Callback<void()>(this, &DigitalSwitches::hotWaterSwitchClose));
    coldWaterSwitch.setHandler(Callback<void()>(this, &DigitalSwitches::coldWaterSwitchOpen),
                                Callback<void()>(this, &DigitalSwitches::coldWaterSwitchClose));
    iceWaterSwitch.setHandler(Callback<void()>(this, &DigitalSwitches::iceWaterSwitchOpen),
                                Callback<void()>(this, &DigitalSwitches::iceWaterSwitchClose));
#else
	hotWaterSwitch.rise(Callback<void()>(this, &DigitalSwitches::hotWaterSwitchOpen));
	hotWaterSwitch.fall(Callback<void()>(this, &DigitalSwitches::hotWaterSwitchClose));
	coldWaterSwitch.rise(Callback<void()>(this, &DigitalSwitches::coldWaterSwitchOpen));
	coldWaterSwitch.fall(Callback<void()>(this, &DigitalSwitches::coldWaterSwitchClose));
#if ENABLE_ICE_WATER
    iceWaterSwitch.rise(Callback<void()>(this, &DigitalSwitches::iceWaterSwitchOpen));
	iceWaterSwitch.fall(Callback<void()>(this, &DigitalSwitches::iceWaterSwitchClose));
#endif
#endif

	//
	hotWaterFlowMeter.rise(Callback<void()>(this, &DigitalSwitches::hotWaterPulseRise));
	coldWaterFlowMeter.rise(Callback<void()>(this, &DigitalSwitches::coldWaterPulseRise));
#if ENABLE_ICE_WATER
    iceWaterFlowMeter.rise(Callback<void()>(this, &DigitalSwitches::iceWaterPulseRise));
#endif
	feedWaterFlowMeter.rise(Callback<void()>(this, &DigitalSwitches::feedWaterPulseRise));
	hotWaterFlowMeter.fall(Callback<void()>(this, &DigitalSwitches::hotWaterPulseFall));
	coldWaterFlowMeter.fall(Callback<void()>(this, &DigitalSwitches::coldWaterPulseFall));
#if ENABLE_ICE_WATER
    iceWaterFlowMeter.fall(Callback<void()>(this, &DigitalSwitches::iceWaterPulseFall));
#endif
	feedWaterFlowMeter.fall(Callback<void()>(this, &DigitalSwitches::feedWaterPulseFall));

	//
#if USING_BUTTON_SCANNER
    filterRinsePosA.setHandler(Callback<void()>(this, &DigitalSwitches::filterRinseInterrupt_posA), NULL);
    filterRinsePosB.setHandler(Callback<void()>(this, &DigitalSwitches::filterRinseInterrupt_posB), NULL);
#else
	filterRinsePosA.rise(Callback<void()>(this, &DigitalSwitches::filterRinseInterrupt_posA));
	filterRinsePosB.rise(Callback<void()>(this, &DigitalSwitches::filterRinseInterrupt_posB));
#endif

	//
#if USING_BUTTON_SCANNER
    kidSafetyButton.setHandler(Callback<void()>(this, &DigitalSwitches::kidSafetyButtonCallback), NULL);
#else
	kidSafetyButton.rise(Callback<void()>(this, &DigitalSwitches::kidSafetyButtonCallback));
#endif
	//
	UVFault.rise(Callback<void()>(this, &DigitalSwitches::UVFaultCallback));

	//
#if USING_BUTTON_SCANNER
    dustHelmetCloseSwitch.setHandler(NULL, Callback<void()>(this, &DigitalSwitches::dustHelmetClosed));
    dustHelmetOpenSwitch.setHandler(NULL, Callback<void()>(this, &DigitalSwitches::dustHelmetOpened));
#else
	//dustHelmetCloseSwitch.fall(Callback<void()>(this, &DigitalSwitches::dustHelmetClosed));
	//dustHelmetOpenSwitch.fall(Callback<void()>(this, &DigitalSwitches::dustHelmetOpened));
#endif

	// reset 631 valve
	if(!filterRinsePosA.read())
	{
		FTRACE("###DigitalSwitches::initializeDigitalSwitches() reset filter rinse to In-serv Position\r\n");
		startFilterRinse(false);

		//TODO: main process need wait untill 631 valve back in-serv postion
	}

    // reset Dust Helmet
    if(dustHelmetCloseSwitch.read() != 0) // not closed
    {
        openDustHelmet(false);
    }

	return 0;
}

void DigitalSwitches::hotWaterSwitchTimeoutFun(void)
{
	bHotWaterSwitchTimeoutTriggered = true;
}

void DigitalSwitches::coldWaterSwitchTimeoutFun(void)
{
	bColdWaterSwitchTimeoutTriggered = true;
}

void DigitalSwitches::iceWaterSwitchTimeoutFun(void)
{
	bIceWaterSwitchTimeoutTriggered = true;
}

int DigitalSwitches::process()
{
    // check button state
    scan();

	if(StateMachine::getInstance()->inSoldState() || bCalculateColdCost || bCalculateHotCost || bCalculateIceCost)
	{
		// cold water
		// Pressed action
		if(!coldWaterSwitchPressed && DIGITAL_SWITCH_ON == coldWaterValve.read())
		{
			// stop App eject timeout
			if(Controller::getInstance()->CPUserInfo.accountType) // app/wechat
				Controller::getInstance()->stopAppAccountEjectCountdown();

			// start timeout to check water flow
            bColdWaterSwitchTimeoutTriggered = false;
			coldWaterSwitchTimeout.attach(Callback<void()>(this, &DigitalSwitches::coldWaterSwitchTimeoutFun), 2.0);

			coldWaterSwitchPressed = true;
            bSwitchStateChanged = true;

#if ENABLE_UV_POWER_CONTROL
            UVPowerControlTimeout.detach();
            powerOnUVDevice(true);
#endif
		}
		// Released action
		if(coldWaterSwitchPressed && DIGITAL_SWITCH_OFF == coldWaterValve.read())
		{
			// cold water is stopped after released
			//if(DIGITAL_SWITCH_ON == coldWaterValve.read()) // make sure the valve is closed
			//{
			//	coldWaterSwitchClose();
			//}

			// start timeout to update water info later
			//if(Controller::getInstance()->CPUserInfo.accountState || Controller::getInstance()->CPColdPrice == 0)
			//	coldWaterSwitchTimeout.attach(Callback<void()>(this, &DigitalSwitches::coldWaterSwitchTimeoutFun), 1.3);

            coldWaterSwitchTimeout.detach();
			coldWaterSwitchPressed = false;
#if ENABLE_UV_POWER_CONTROL
            UVPowerControlTimeout.attach(Callback<void()>(this, &DigitalSwitches::UVPowerControlTimeoutFun),
                Controller::getInstance()->UVPowerOffDelaySec);
#endif
		}

		// hot water
		// Pressed action
		if(!hotWaterSwitchPressed && DIGITAL_SWITCH_ON == hotWaterValve.read())
		{
			// stop App eject timeout
			if(Controller::getInstance()->CPUserInfo.accountType) // app/wechat
				Controller::getInstance()->stopAppAccountEjectCountdown();

			// start timeout to check water flow
            bHotWaterSwitchTimeoutTriggered = false;
			hotWaterSwitchTimeout.attach(Callback<void()>(this, &DigitalSwitches::hotWaterSwitchTimeoutFun), 2.0);

			hotWaterSwitchPressed = true;
            bSwitchStateChanged = true;

#if ENABLE_UV_POWER_CONTROL
            UVPowerControlTimeout.detach();
            powerOnUVDevice(true);
#endif
		}
		// Released action
		if(hotWaterSwitchPressed && DIGITAL_SWITCH_OFF == hotWaterValve.read())
		{
			// hot water is stopped after released
			//if(DIGITAL_SWITCH_ON == hotWaterValve.read()) // make sure the valve is closed
			//{
			//	hotWaterSwitchClose();
			//}

			// start timeout to update water info later
			//if(Controller::getInstance()->CPUserInfo.accountState || Controller::getInstance()->CPHotPrice  == 0)
			//	hotWaterSwitchTimeout.attach(Callback<void()>(this, &DigitalSwitches::hotWaterSwitchTimeoutFun), 1.3);

            hotWaterSwitchTimeout.detach();
			hotWaterSwitchPressed = false;
#if ENABLE_UV_POWER_CONTROL
            UVPowerControlTimeout.attach(Callback<void()>(this, &DigitalSwitches::UVPowerControlTimeoutFun),
                Controller::getInstance()->UVPowerOffDelaySec);
#endif

#if ENABLE_KID_SAFETY_FUNCTION
            // start count for lock KID safety button
            if(Controller::getInstance()->isKidSafetyEnabledDevice())
            {
			#ifndef TDS_CONTROL_SWITCH
                 if(DIGITAL_SWITCH_OFF == iceWaterValve.read()) // at meantime another hotwater is closed
                 {
                     KidSafetyDelayTimeout.attach(Callback<void()>(this, &DigitalSwitches::KidSafetyDelayTimeoutFun),
                                                 KID_SAFETY_DELAY_SECOND);
                 }
			#endif
            }
#endif
		}

        // ice water
		// Pressed action
#ifndef TDS_CONTROL_SWITCH
		if(!iceWaterSwitchPressed && DIGITAL_SWITCH_ON == iceWaterValve.read())
		{
			// stop App eject timeout
			if(Controller::getInstance()->CPUserInfo.accountType) // app/wechat
				Controller::getInstance()->stopAppAccountEjectCountdown();

			// start timeout to check water flow
            bIceWaterSwitchTimeoutTriggered = false;
			iceWaterSwitchTimeout.attach(Callback<void()>(this, &DigitalSwitches::iceWaterSwitchTimeoutFun), 2.0);

			iceWaterSwitchPressed = true;
            bSwitchStateChanged = true;

#if ENABLE_UV_POWER_CONTROL
            UVPowerControlTimeout.detach();
            powerOnUVDevice(true);
#endif
		}
		//Released action
		if(iceWaterSwitchPressed && DIGITAL_SWITCH_OFF == iceWaterValve.read())
		{
			// hot water is stopped after released
			//if(DIGITAL_SWITCH_ON == hotWaterValve.read()) // make sure the valve is closed
			//{
			//	hotWaterSwitchClose();
			//}

			// start timeout to update water info later
			//if(Controller::getInstance()->CPUserInfo.accountState || Controller::getInstance()->CPIcePrice  == 0)
			//	hotWaterSwitchTimeout.attach(Callback<void()>(this, &DigitalSwitches::hotWaterSwitchTimeoutFun), 1.3);

            hotWaterSwitchTimeout.detach();
			iceWaterSwitchPressed = false;
#if ENABLE_UV_POWER_CONTROL
            UVPowerControlTimeout.attach(Callback<void()>(this, &DigitalSwitches::UVPowerControlTimeoutFun),
                Controller::getInstance()->UVPowerOffDelaySec);
#endif

#if ENABLE_KID_SAFETY_FUNCTION
            // start count for lock KID safety button
            if(Controller::getInstance()->isKidSafetyEnabledDevice())
            {
                if(DIGITAL_SWITCH_OFF == hotWaterValve.read()) // at meantime another hotwater is closed
                {
                    KidSafetyDelayTimeout.attach(Callback<void()>(this, &DigitalSwitches::KidSafetyDelayTimeoutFun),
                                                KID_SAFETY_DELAY_SECOND);
                }
            }
#endif
		}
#endif
		// calculate cost
		if(bCalculateColdCost || bCalculateHotCost || bCalculateIceCost)
		{
			bool bUpdateAccountInfo = false;
			// cold water info
			if(Controller::getInstance()->coldWaterFlowPulse > Controller::getInstance()->CPUserInfo.coldWaterPulse + 10)
			{
				//FTRACE("###DigitalSwitches::process() coldWaterFlowPulse: %d\r\n", Controller::getInstance()->coldWaterFlowPulse);
				Controller::getInstance()->CPUserInfo.coldWaterPulse = Controller::getInstance()->coldWaterFlowPulse;
				bUpdateAccountInfo = true;
			}

#if ENABLE_ICE_WATER
             // hot&ice water info
			if(Controller::getInstance()->hotWaterFlowPulse + Controller::getInstance()->iceWaterFlowPulse  > 
                    Controller::getInstance()->CPUserInfo.hotWaterPulse + 10)
			{
				//FTRACE("###DigitalSwitches::process() hotWaterFlowPulse: %d\r\n", Controller::getInstance()->hotWaterFlowPulse);
				Controller::getInstance()->CPUserInfo.hotWaterPulse = Controller::getInstance()->hotWaterFlowPulse + 
                                                                        Controller::getInstance()->iceWaterFlowPulse;
				bUpdateAccountInfo = true;
			}

#else
			// hot water info
			if(Controller::getInstance()->hotWaterFlowPulse > Controller::getInstance()->CPUserInfo.hotWaterPulse + 10)
			{
				//FTRACE("###DigitalSwitches::process() hotWaterFlowPulse: %d\r\n", Controller::getInstance()->hotWaterFlowPulse);
				Controller::getInstance()->CPUserInfo.hotWaterPulse = Controller::getInstance()->hotWaterFlowPulse;
				bUpdateAccountInfo = true;
			}
#endif

			if(bUpdateAccountInfo)
            {
				Controller::getInstance()->updateAccountInfo();
			}
		}

		// switch timeout process
		if(bColdWaterSwitchTimeoutTriggered)
		{
			// start timeout
			if(coldWaterSwitchPressed && Controller::getInstance()->coldWaterFlowPulse==0)
			{
				// warning; Switch is Open but no flow pulse
				WarningManager::getInstance()->triggerWarning(WarningManager::WARNING_TYPE_COLD_VALVE_ERR);
				// try to close cold water, for flow meter error case
				// closeColdWater();
				//if(Controller::getInstance()->CPUserInfo.accountType) //app/wechat
				//{
				//	Controller::getInstance()->CPUserInfo.accountState = 0; // deactive if valve error
				//	StateMachine::getInstance()->transferToStandby();
				//}
			}
			if(coldWaterSwitchPressed && Controller::getInstance()->coldWaterFlowPulse>0)
			{
				WarningManager::getInstance()->resolveWarning(WarningManager::WARNING_TYPE_COLD_VALVE_ERR);
			}

            bColdWaterSwitchTimeoutTriggered = false;
        }
        if(bHotWaterSwitchTimeoutTriggered)
        {
			if(hotWaterSwitchPressed && Controller::getInstance()->hotWaterFlowPulse==0)
			{
				// warning; Switch is Open but no flow pulse
				WarningManager::getInstance()->triggerWarning(WarningManager::WARNING_TYPE_HOT_VALVE_ERR);
				// try to close hot water, for flow meter error case
				//closeHotWater();
				//if(Controller::getInstance()->CPUserInfo.accountType) //app/wechat
				//{
				//	Controller::getInstance()->CPUserInfo.accountState = 0; // deactive if valve error
				//	StateMachine::getInstance()->transferToStandby();
				//}
			}
			if(hotWaterSwitchPressed && Controller::getInstance()->hotWaterFlowPulse>0)
			{
				WarningManager::getInstance()->resolveWarning(WarningManager::WARNING_TYPE_HOT_VALVE_ERR);
			}

            bHotWaterSwitchTimeoutTriggered = false;
        }
        if(bIceWaterSwitchTimeoutTriggered)
        {
			if(iceWaterSwitchPressed && Controller::getInstance()->iceWaterFlowPulse==0)
			{
				// warning; Switch is Open but no flow pulse
				//WarningManager::getInstance()->triggerWarning(WarningManager::WARNING_TYPE_ICE_VALVE_ERR);
				// try to close hot water, for flow meter error case
				//closeHotWater();
				//if(Controller::getInstance()->CPUserInfo.accountType) //app/wechat
				//{
				//	Controller::getInstance()->CPUserInfo.accountState = 0; // deactive if valve error
				//	StateMachine::getInstance()->transferToStandby();
				//}
			}
			if(iceWaterSwitchPressed && Controller::getInstance()->iceWaterFlowPulse>0)
			{
				//WarningManager::getInstance()->resolveWarning(WarningManager::WARNING_TYPE_HOT_VALVE_ERR);
			}

            bIceWaterSwitchTimeoutTriggered = false;
        }

		// end timeout
		if(!coldWaterSwitchPressed && !hotWaterSwitchPressed && !iceWaterSwitchPressed && bSwitchStateChanged)
		{
			// upadte water info
		//	Controller::getInstance()->CPUserInfo.coldWaterPulse = Controller::getInstance()->coldWaterFlowPulse;
		//	Controller::getInstance()->CPUserInfo.hotWaterPulse = Controller::getInstance()->hotWaterFlowPulse;
		//	Controller::getInstance()->updateAccountInfo();

            bSwitchStateChanged = false;
            Controller::getInstance()->waterReleaseTimes ++;

            // start app eject timeout if app/wechat
			if(Controller::getInstance()->CPUserInfo.accountState)
            {
				if(Controller::getInstance()->CPUserInfo.accountType == 1) // app
			    {
                    if(Controller::getInstance()->waterReleaseTimes == 1) // during first time release to second time release
                        Controller::getInstance()->startAppAccountEjectCountdown(Controller::getInstance()->AppPayEjectDelaySec2);
                    else
				        Controller::getInstance()->startAppAccountEjectCountdown(Controller::getInstance()->AppPayEjectDelaySec);
			    }
                else if(Controller::getInstance()->CPUserInfo.accountType == 2) // wechat
                {
                    if(Controller::getInstance()->CPUserInfo.balance > 0)
                        Controller::getInstance()->startAppAccountEjectCountdown(Controller::getInstance()->WechatPayEjectDelaySec);
                    else
                        Controller::getInstance()->startAppAccountEjectCountdown(5); // no balance yet
                }
            }

			bCalculateColdCost = false;
			bCalculateHotCost = false;
            bCalculateIceCost = false;
			if(Controller::getInstance()->CPColdPrice == 0 && 
                Controller::getInstance()->CPHotPrice == 0 && 
                //Controller::getInstance()->CPIcePrice == 0 && 
                Controller::getInstance()->CPUserInfo.accountState == 0)
			{
                Controller::getInstance()->updateAccountInfo(true); // update the finish event
				//StateMachine::getInstance()->transferToStandby();
			}
		}
	}

	// process 631 valve interrupt
	if(bFilterRinsePositionATriggered)
	{
		bool bFromFilterRinseState = false;
		bFilterRinsePositionATriggered = false;
		LOG_INFO("###DigitalSwitches::process() 631 valve trigger A position. filterRinsePosition: %d\r\n", filterRinsePosition);

		if(StateMachine::getInstance()->inProductionTestState() && filterRinsePosition < 4)
		{
			//
			Action* pAct = new Action(Action::AID_CP_FILTERRINSEEND);
			ActionBase::append(pAct);

			//Controller::getInstance()->productionTestStep = 3;
			Controller::getInstance()->startProductionTestTimeout(1);
		}
		if(StateMachine::getInstance()->inFilterRinseState())
		{
			// upload filter rinse END event to Central Panel
			Action* pAct = new Action(Action::AID_CP_FILTERRINSEEND);
			ActionBase::append(pAct);

			bFromFilterRinseState = true;
			filterRinseLoopCount ++;
		
			//
			StateMachine::getInstance()->transferToStandby();
		}

		filterRinsePosition = 5;
		filterRinseTimeout.detach();

		filterRinseTimer.stop();
		filterRinseLastTime = 0;

		if(bFromFilterRinseState)
		{
			//FTRACE("###DigitalSwitches::process() %d/%d\r\n", filterRinseLoopCount, Controller::getInstance()->filterRinseMaxCount);
			if(filterRinseLoopCount < Controller::getInstance()->filterRinseMaxCount)
			{
				// do next filter rinse loop
				Controller::getInstance()->filterRinse();
			}
			else
			{
				// finished
				// reset count variable
				filterRinseLoopCount = 0;
				Controller::getInstance()->setMaxLoopCountOfFilterRinse(1);
			}
		}
	}
	if(bFilterRinsePositionBTriggered)
	{
		bFilterRinsePositionBTriggered = false;
		LOG_INFO("###DigitalSwitches::process() 631 valve trigger B position filterRinsePosition:%d\r\n", filterRinsePosition);

		// reach back wash/fast wash position; filterRinsePosition==1: back wash; filterRinsePosition==2: fast wash
		if((StateMachine::getInstance()->inFilterRinseState() || StateMachine::getInstance()->inProductionTestState()) &&
				(filterRinsePosition == 1 || filterRinsePosition == 2)) //
		{
			int duration = Controller::getInstance()->CPFilterBackRinseDur * 60;
			if(StateMachine::getInstance()->inProductionTestState() && filterRinsePosition == 1)
			{
				duration = PRODUCT_TEST_FILTER_BACK_WASH_DUR*60 + 30; // 15min
				//Controller::getInstance()->productionTestStep = 1;
				Controller::getInstance()->startProductionTestTimeout(30); // open B3 valve in 30 sec
			}
			if(filterRinsePosition == 2) // fast wash
			{
				duration = Controller::getInstance()->CPFilterFastRinseDur * 60;
				if(StateMachine::getInstance()->inProductionTestState())
				{
					duration = PRODUCT_TEST_FILTER_FAST_RINSE_DUR*60 + 30; // 5min
					//Controller::getInstance()->productionTestStep = 2;
					Controller::getInstance()->startProductionTestTimeout(30);
				}
			}

			// set time to restart motor to next position
			FTRACE("###DigitalSwitches::process() filterRinseTimeout: %d\r\n", duration);
			filterRinseTimeout.attach(Callback<void()>(this, &DigitalSwitches::filterRinseInterrupt_posB_timeout), 
										duration);
		}
	}

	//
	stepDustHelmetMotor(bDustHelmetOpen);

	return 0;
}

int DigitalSwitches::process_one_second(time_t seconds)
{
	//FTRACE("###DigitalSwitches::process() kidSafetyButton: %u\r\n", kidSafetyButton.read());
	// blink GREEN LED
	greenLED = !greenLED;

	// feed water; low frequency
	if(Controller::getInstance()->feedWaterFlowPulse > Controller::getInstance()->CPPulsePerLitreArr[CP_PULSE_PER_LITRE])
	{
		Controller::getInstance()->CPFilterWaterCur += Controller::getInstance()->feedWaterFlowPulse /
														Controller::getInstance()->CPPulsePerLitreArr[CP_PULSE_PER_LITRE];
		Controller::getInstance()->feedWaterFlowPulse %= Controller::getInstance()->CPPulsePerLitreArr[CP_PULSE_PER_LITRE];

		// save the total litre of filter water
		PersistentStorage::getInstance()->saveFilterWaterCur();
	}

	// water leak test
	unsigned short l0,l1;
    {
        unsigned int temp_total0 = 0, temp_total1 = 0;
        int i = 0;
        for(i=0; i<10; i++)
        {
            temp_total0 += waterLeak0.read_u16();
            temp_total1 += waterLeak1.read_u16();
        }
	    l0 = temp_total0 / 10;
	    l1 = temp_total1 / 10;
    }
	//FTRACE("######DigitalSwitches::process() Leak: 0x%x 0x%x\r\n", l0, l1);
	if(l0<0x5000 && l1<0x5000)
	{
		if(!bWaterLeakDetected)
		{
			bWaterLeakDetected = true;
			WarningManager::getInstance()->triggerWarning(WarningManager::WARNING_TYPE_WATER_LEAK);
		}
	}
	else
	{
        if(l0>0x7000 && l1>0x7000)
        {
		    if(bWaterLeakDetected)
		    {
			    bWaterLeakDetected = false;
			    WarningManager::getInstance()->resolveWarning(WarningManager::WARNING_TYPE_WATER_LEAK);
		    }
        }
	}

#ifdef TDS_CONTROL_SWITCH
	// TDS detect
	if(CWaterMixerValve::getInstance()->getEnabled())
	{
	// 	TDSDetectSec ++;
	// 	if(TDSDetectSec > TDS_DETECT_INTERVAL)
	// 	{
	// 		// CWaterMixerValve::getInstance()->ServoStartFlag = true;
    // 		// CWaterMixerValve::getInstance()->RawWaterFlag = true;

	// 		unsigned short tds = getPurifiedWaterTDS();

	// 		FTRACE("###DigitalSwitches::process() tds 2: %d\r\n", tds);

	// 		// // upload water quality
	// 		// Action* pAct = new ActionWaterQualityInfo(1, tds);
	// 		// ActionBase::append(pAct);
			
	// 		// CWaterMixerValve::getInstance()->ServoStartFlag = true;
	// 		// reset
	// 		TDSDetectSec = 0;
	// 	}
		FTRACE("\n");
		FTRACE("###DigitalSwitches::process() Purified tds : %d\r\n", getPurifiedWaterTDS());
		FTRACE("\n");
	}
#endif

	// infrared dectection test
	if(bInfraredDetectEnabled)
	{
		if(infraredSensor0.read() == 0
#if ENABLE_DOUBLE_INFRARED_SENSOR
			|| infraredSensor1.read() == 0
#endif
			) // falll
		{
			// detected
			if(!bInfraredDetected)
			{
				//
				Action* pAct = new Action(Action::AID_CP_INFRAREDSENSORWORKS);
				ActionBase::append(pAct);

				FTRACE("Human detected\n");

				bInfraredDetected = true;

				//
				QRPowerControl = DIGITAL_SWITCH_ON;

				// open dust helmet if not opened
				//if(dustHelmetOpenSwitch.read() != 0)
				//	openDustHelmet(true);

				//suspend disinfect
				if(StateMachine::getInstance()->inDisinfectState())
					Controller::getInstance()->suspendDisinfect();
			}

			infraredDetectVanishedSec = 0;
		}
		else // rise
		{
			// vanished
			if(bInfraredDetected && (infraredDetectVanishedSec > INFRARED_DETECTED_DELAY))
			{
				FTRACE("Human undetected\n");
				Action* pAct = new Action(Action::AID_CP_INFRAREDSENSORIDLE);
				ActionBase::append(pAct);

				bInfraredDetected = false;
				//infraredDetectedSec = 0;

				//
				QRPowerControl = DIGITAL_SWITCH_OFF;

				// resume disinfect if suspended
				if(Controller::getInstance()->CPDisinfectCostSec)
					Controller::getInstance()->resumeDisinfect();
			}

			infraredDetectVanishedSec++;
		}
	}
	else
	{
		// increase the time as no human detected even if infrared is turned off;
		infraredDetectVanishedSec++;
	}

	// close dust helmet while no human detect for 1 hour
	//if(dustHelmetCloseSwitch.read() == 0 && infraredDetectVanishedSec > 3600) // 1 hour = 3600 seconds;
	//{
	//	openDustHelmet(false);
	//	infraredDetectVanishedSec = 0; // avoid close it again
	//}

	// UV device Error test
	if(bUVFaultInterrupt)
	{
		if(!bUVFaultDetected)
		{
			bUVFaultDetected = true;
			WarningManager::getInstance()->triggerWarning(WarningManager::WARNING_TYPE_UV_DEVICE_ERR);
		}

		bUVFaultInterrupt = false;
		UVFaultDetectedSec = 0;
	}
	else
	{
		if(bUVFaultDetected && UVFaultDetectedSec > UVFAULT_DETECTED_DELAY)
		{
			if(bUVFaultDetected)
			{
				bUVFaultDetected = false;
				WarningManager::getInstance()->resolveWarning(WarningManager::WARNING_TYPE_UV_DEVICE_ERR);
			}
		}

		if(bUVFaultDetected)
			UVFaultDetectedSec ++;
	}

    //FTRACE("###INVASION forbidden: %d detected: %d invasion: %d\r\n", Controller::getInstance()->CPDoorOpenForbidden,
    //    Controller::getInstance()->CPInvasionDetected, invasionDetect0.read());
	// invasion detect test
	if(Controller::getInstance()->CPDoorOpenForbidden) // unauthorized
	{
		if(invasionDetect0.read())
		{
			if(!Controller::getInstance()->CPInvasionDetected)
			{
				Controller::getInstance()->setInvasionDetectedFlag(true);
				WarningManager::getInstance()->triggerWarning(WarningManager::WARNING_TYPE_INVASION);
			}
		}
	}
	else // authorized
	{
		// if(Controller::getInstance()->CPInvasionDetected && invasionDetect0.read() == 0) // if closed
		// {
		// 	Controller::getInstance()->CPDoorOpenForbidden = true; // unauthorized

        //     if(Controller::getInstance()->CPInvasionDetected)
		//     {
		// 	    Controller::getInstance()->setInvasionDetectedFlag(false);
		// 	    //WarningManager::getInstance()->resolveWarning(WarningManager::WARNING_TYPE_INVASION);
		//     }
		// }
        // else if(!Controller::getInstance()->CPInvasionDetected && invasionDetect0.read())
        // {
        //     Controller::getInstance()->setInvasionDetectedFlag(true);
        // }

		// WarningManager::getInstance()->resolveWarning(WarningManager::WARNING_TYPE_INVASION);

		if(Controller::getInstance()->CPInvasionDetected)
		{
			Controller::getInstance()->setInvasionDetectedFlag(false);
			WarningManager::getInstance()->resolveWarning(WarningManager::WARNING_TYPE_INVASION);
		}

        if(300 < seconds - Controller::getInstance()->CPInvasionDetectedClosTime){
            if(Controller::getInstance()->CPInvasionDetectedClosTime > 0 && invasionDetect0.read() == 0) // if closed
            {
				Controller::getInstance()->CPInvasionDetectedClosTime = 0;
				Controller::getInstance()->CPDoorOpenForbidden = true; // unauthorized
            }
            Controller::getInstance()->CPInvasionDetectedClosTime = seconds;
        }
	}

#if ENABLE_CHECK_HELTMET_OPENED
    // dust helmet closed check
    if(intentClosed)
    {
        if(dustHelmetCloseSwitch.read() != 0)
        {
            if(helmetErrorRetryCount > 5)
            {
                // report dust helmet ERROR
                WarningManager::getInstance()->triggerWarning(WarningManager::WARNING_TYPE_DUST_HELMET_ERR);
            }
            else
            {
                ticksToNextRetry ++;
                if(ticksToNextRetry > 120) // sec
                {
                    // try to close
                    openDustHelmet(false);
                    ticksToNextRetry = 0; // reset
                }
            }
        }
    }
#endif

	return 0;
}

int DigitalSwitches::lightWarningLED(bool bOn)
{
	redLED = !bOn;

	return 0;
}

int DigitalSwitches::openDustHelmet(bool bOpen)
{
	//FTRACE("###DigitalSwitches::openDustHelmet() bOpen: %d\r\n", bOpen);
    intentClosed = !bOpen;
    if(bOpen)
    {
        closeHelmetDelayTimeout.detach(); // cancel close
        bDustHelmetOpen = 0; // stop current action
        if(dustHelmetOpenSwitch.read() != 0) // not opened
        {
            helmetTryCount = 0;
	        bDustHelmetOpen = bOpen ? 1 : 2;
            setStepMotorSteps();
        }
    }
    else
    {
        closeHelmetDelayTimeout.attach(Callback<void()>(this, &DigitalSwitches::closeHelmetTimeoutFun),
										Controller::getInstance()->DustHelmetCloseDelaySec);
        //closeHelmetTimeoutFun();
    }

	return 0;
}

void DigitalSwitches::closeHelmetTimeoutFun(void)
{
#ifdef TDS_CONTROL_SWITCH
    if(hotWaterValve.read() == DIGITAL_SWITCH_ON ||
        coldWaterValve.read() == DIGITAL_SWITCH_ON) 
#else
    if(hotWaterValve.read() == DIGITAL_SWITCH_ON ||
        coldWaterValve.read() == DIGITAL_SWITCH_ON ||
        iceWaterValve.read() == DIGITAL_SWITCH_ON)
#endif
    {
        // not allowed
        FTRACE("###DigitalSwitches::closeHelmetTimeoutFun() at valve is open\r\n");
        return ;
    }

    helmetTryCount = 0;
    bDustHelmetOpen = 2; // close
    setStepMotorSteps();
}

bool DigitalSwitches::infraredDetectedWorks()
{
	return bInfraredDetected;
}

bool DigitalSwitches::feedWaterSwitchOpened()
{
	if(DIGITAL_SWITCH_OFF == feedWaterRed.read() && DIGITAL_SWITCH_ON == feedWaterGreen.read())
	{
		return true;
	}
	else if(DIGITAL_SWITCH_ON == feedWaterRed.read() && DIGITAL_SWITCH_OFF == feedWaterGreen.read())
    {
		return false; // CONF_DIGITAL_OFF
	}
	else
    {
        // At initialize state, treat as closed
        // turn off B3
        // set GREEN line to low
        // set RED line to high
		feedWaterGreen = DIGITAL_SWITCH_OFF;
		feedWaterRed = DIGITAL_SWITCH_ON;
        return false;
    }
}

int DigitalSwitches::getHotWaterValveState()
{
	if(DIGITAL_SWITCH_ON == hotWaterValve.read())
		return 1;
	else
		return 0;
}

int DigitalSwitches::getColdWaterValveState()
{
	if(DIGITAL_SWITCH_ON == coldWaterValve.read())
		return 1;
	else
		return 0;
}

int DigitalSwitches::getIceWaterValveState()
{
#ifndef TDS_CONTROL_SWITCH
	if(DIGITAL_SWITCH_ON == iceWaterValve.read())
		return 1;
	else
		return 0;
#else
	return 0;
#endif

}

bool DigitalSwitches::dustHelmetIsOpened()
{
    if(0 == dustHelmetOpenSwitch.read())
        return true;

    return false;
}

void DigitalSwitches::setStepMotorSteps()
{
    //dustHelmetAllowedSteps = 4096; // one circle
	dustHelmetAllowedSteps = 64*12;
}

int DigitalSwitches::stepDustHelmetMotor(char bType)
{
    //0: do nothing; 1: Open; 2: Close;
    if(bType == 0)
    {
        dustHelmetAllowedSteps --;
        return 0;
    }

	if(dustHelmetAllowedSteps <= 0)
    {
        //
        if(bType == 1) //open
        {
            if(0!=dustHelmetOpenSwitch.read()) //opened state switch is not on
            {
                // try to open again
                if(helmetTryCount < 1)
                {
                    helmetTryCount ++;
                    setStepMotorSteps();
                    return 0;
                }
                else
                {
                    // report Helmet ERROR
                    FTRACE("###DigitalSwitches::stepDustHelmetMotor() Open failed\r\n");
                    WarningManager::getInstance()->triggerWarning(WarningManager::WARNING_TYPE_DUST_HELMET_ERR);
                }
            }
        }
        else if(bType == 2)//close
        {
            if(0!=dustHelmetCloseSwitch.read()) //closed state switch is not on
            {
                // try to close again
                if(helmetTryCount < 1)
                {
                    helmetTryCount ++;
                    setStepMotorSteps();
                    return 0;
                }
                else
                {
                    // report Helmet ERROR
                    helmetErrorRetryCount ++;
                }
            }
            else
            {
                // close succeeded, reset
                helmetErrorRetryCount = 0;
                ticksToNextRetry = 0;
            }
        }

        // finished
        bDustHelmetOpen = 0;
		return 0;
    }

	dustHelmetAllowedSteps --;

	switch(dustHelmetStep)
	{
		case 0:
			dustHelmet = 0x01;
			break;
		case 1:
			dustHelmet = 0x03;
			break;
		case 2:
			dustHelmet = 0x02;
			break;
		case 3:
			dustHelmet = 0x06;
			break;
		case 4:
			dustHelmet = 0x04;
			break;
		case 5:
			dustHelmet = 0x0C;
			break;
		case 6:
			dustHelmet = 0x08;
			break;
		case 7:
			dustHelmet = 0x09;
			break;
		default:
			break;
	}

	// move to next step
	if(bType == 1)
	{
		dustHelmetStep += 1;
		if(dustHelmetStep > 7)
			dustHelmetStep = 0;
	}
	else if(bType == 2)
	{
		dustHelmetStep -= 1;
		if(dustHelmetStep < 0)
			dustHelmetStep = 7;
	}

	//wait_us(1000);
	return 0;
}

void DigitalSwitches::dustHelmetOpened(void)
{
	dustHelmetAllowedSteps = 64; // a little more steps
}

void DigitalSwitches::dustHelmetClosed(void)
{
	dustHelmetAllowedSteps = 64; // a little more steps
}

void DigitalSwitches::powerOnUVDevice(bool bOn)
{
    if(bOn)
    {
        UVPowerControl = DIGITAL_SWITCH_ON;
    }
    else
    {
        UVPowerControl = DIGITAL_SWITCH_OFF;
    }
}

#if ENABLE_UV_POWER_CONTROL
void DigitalSwitches::UVPowerControlTimeoutFun(void)
{
    powerOnUVDevice(false);
}
#endif

#if ENABLE_KID_SAFETY_FUNCTION
void DigitalSwitches::KidSafetyDelayTimeoutFun(void)
{
    KidSafetyProtected = true;
}
#endif

void DigitalSwitches::scan(void)
{
#if USING_BUTTON_SCANNER
    if(timer.read_ms() < 10)
    {
        return;
    }

    hotWaterSwitch.check();
    coldWaterSwitch.check();
    iceWaterSwitch.check();
    filterRinsePosA.check();
    filterRinsePosB.check();
    dustHelmetCloseSwitch.check();
    dustHelmetOpenSwitch.check();
#if ENABLE_KID_SAFETY_FUNCTION
    kidSafetyButton.check();
#endif

    timer.reset();
#endif
}

#ifdef TDS_CONTROL_SWITCH
int DigitalSwitches::getPurifiedWaterTDS()
{
    static const int COUNT = 10;
    static const float VREF = 3.3f;
    uint16_t temp[COUNT] = {0};
    uint16_t i = 0;
    uint32_t total = 0;
    float compensationVoltage = 0.0f, tdsValue = 0.0f;
    float temperature = 25.0f;

    // FTRACE("\n###getPurifiedTDS() ");
    for(i=0; i<COUNT; i++)
    {
        temp[i] = TDSDetect.read() * 0x0FFF;

        total += temp[i];
        // FTRACE(" %d", temp[i]);
    }
    // FTRACE("\r\n");

	// FTRACE("%d\n",total/COUNT);
    // compensationVoltage = (total / COUNT) * VREF / 4096;

    // compensationVoltage /= 1.0 + 0.02*(temperature - 25.0);

    // tdsValue = (133.42*compensationVoltage*compensationVoltage*compensationVoltage -
    //             255.86*compensationVoltage*compensationVoltage +
    //             857.39*compensationVoltage) * 0.5;
	total /= COUNT;
	if(700 > total)
		tdsValue = total * 0.44;
	else if(700 < total && 1150 > total)
		tdsValue = total * 0.5;
	else if(1150 < total && 1600 > total)
		tdsValue = total * 0.55;
	else
		tdsValue = total * 0.6;

    return (int)tdsValue;
}

#endif