#include "utils.h"
#include "ICCard.h"
#include "stdio.h"
#include "stdlib.h"

#define LOG_MODULE 0x01

// flash APIs
extern void upgrade_init();
extern uint32_t upgrade_get_base_addr();
extern void upgrade_erase(uint32_t base_addr);
extern int upgrade_flash_copy(uint32_t dest, const uint8_t * src, uint32_t size);
extern void upgrade_commit();
extern int upgrade_check_hash_code(unsigned int size, unsigned int hashcode);
unsigned int upgrade_firmware_size = 0;

// IC Card Insert Serial Number
unsigned int gICCardInsertSerialNumber = 0;

Controller Controller::ctrler;
Controller* Controller::getInstance()
{
	return &Controller::ctrler;
}

Controller::Controller()
{
	int i,j;
	memset(WHVer, 0, 3);
	WHBoilingTemp = 50;
	WHTempOffset = 0;
	WHWarningTempOffset = 5;
	WHFBOnOff = true;
	WHFBDur = WH_STATUS_UPDATE_INTERVAL; // seconds
	WHStatus = 0;
	WHHeater = false;
	WHWater = false;
	WHInService = true;
	SYSInService = true;

	// settings
	//CPTemperature = 80;
	CPHotPrice = 30; // cents per litre
	CPColdPrice = 15;
	CPIcePrice = 30;
	CPFilterWaterMax = 120000; // litre
	CPFilterWaterCur = 0;
	CPPlanDelayTime = 0;
	CPDisinfectDelayTime = 40;
	CPFilterProductionDate.year = 2017;
	CPFilterProductionDate.month = 12;
	CPFilterProductionDate.day = 1;
	CPFilterBackRinseDur = 10;
	CPFilterFastRinseDur = 5;
	CPMembraneRinseDur = 5;
	CPDrainMinS = 2;
	CPDrainMinE = 3;
	CPChargeMode = 1;
	CPChargeUnit = 1000;
	for(i=0; i<7; i++)
	{
		CPServiceTime[i].start[0] = 0;
		CPServiceTime[i].end[0] = 2400;

		CPHeaterWorkTime[i].start[0] = 0;
		CPHeaterWorkTime[i].end[0] = 2400;

		for(j=1; j<4; j++)
		{
			CPServiceTime[i].start[j] = 0;
			CPServiceTime[i].end[j] = 0;

			CPHeaterWorkTime[i].start[j] = 0;
			CPHeaterWorkTime[i].end[j] = 0;
		}
	}
	CPFilterShelfLife = 12;
	CPFilterCheckMode = 0;
	CPCompensation.startTemp = 5;
	CPCompensation.compensation = 9;
	CPDisinfectDur = 5;
	CPDisinfectTemp = 93;
	CPPulsePerLitreArr[CP_PULSE_PER_LITRE] = 480;		//CPPulsePerLitre
	CPPulsePerLitreArr[CP_HOT_PULSE_PER_LITRE] = 480;		//CPHotPulsePerLitre
	CPPulsePerLitreArr[CP_COLD_PULSE_PER_LITRE] = 480;		//CPColdPulsePerLitre
	CPPulsePerLitreArr[CP_ICE_PULSE_PER_LITRE] = 480;		//CPIcePulsePerLitre
	CPPulsePerLitreArr[CP_PULSE_PER_LITRE_5] = 480;		//CPPulsePerLitre5
	CPPulsePerLitreArr[CP_PULSE_PER_LITRE_6] = 480;		//CPPulsePerLitre6
	memset(&CPUserInfo, 0, sizeof(CPUserInfo));
	CPDoorOpenForbidden = true;
	CPInvasionDetected = 0;
	memset(CPDeviceId, 0, 33);
	CPDrainDur = 10;
	CPHotWaterSwitchEnabled = true;
	CPColdWaterSwitchEnabled = true;

    CPFilterRinseFixTime.valid = false;
    CPFilterRinseFixTime.day   = 0;//2
    CPFilterRinseFixTime.hour  = 0;//2
    CPFilterRinseFixTime.min   = 0;//0
	// Tue. 2:00
    for(i=0; i<DEVICE_RINSE_MAX_NUM; i++)
    {
	    CPFilterRinseFixTimes[i].valid = false;
        CPFilterRinseFixTimes[i].index = i;
	    CPFilterRinseFixTimes[i].day   = 0;
	    CPFilterRinseFixTimes[i].hour  = 0;
	    CPFilterRinseFixTimes[i].min   = 0;
    }
    CurRinseFixTimesindex = 0xFF;    

	// Wed. 2:00
	CPDrainFixTime.valid = false;
	CPDrainFixTime.day   = 0;
	CPDrainFixTime.hour  = 0;
	CPDrainFixTime.min   = 0;

    // Thu. 2:00
    CPMembraneRinseFixTime.valid = false;
    CPMembraneRinseFixTime.day   = 0;//4
    CPMembraneRinseFixTime.hour  = 0;//2
    CPMembraneRinseFixTime.min   = 0;//0

    for(i=0; i<DEVICE_RINSE_MAX_NUM; i++)
    {
	    CPMembraneRinseFixTimes[i].valid = false;
        CPMembraneRinseFixTimes[i].index = i;
	    CPMembraneRinseFixTimes[i].day   = 0;
	    CPMembraneRinseFixTimes[i].hour  = 0;
	    CPMembraneRinseFixTimes[i].min   = 0;
    }
    CurMembraneFixTimesindex = 0xFF;    

    CPDisinfectFixTime.valid = false;
    CPDisinfectFixTime.day   = 0;//5
    CPDisinfectFixTime.hour  = 0;//2
    CPDisinfectFixTime.min   = 0;//0
	// Fri. 2:00
    for(i=0; i<DEVICE_RINSE_MAX_NUM; i++)
    {
	    CPDisinfectFixTimes[i].valid = false;
        CPDisinfectFixTimes[i].index = i;
	    CPDisinfectFixTimes[i].day   = 0;
	    CPDisinfectFixTimes[i].hour  = 0;
	    CPDisinfectFixTimes[i].min   = 0;
    }
    CurDisinfectFixTimesindex = 0xFF; 

	memset(&CPFilterRinsePlan, 0, sizeof(CPFilterRinsePlan));
	memset(&CPDrainPlan, 0, sizeof(CPDrainPlan));
	memset(&CPMembraneRinsePlan, 0, sizeof(CPMembraneRinsePlan));
	memset(&CPDisinfectPlan, 0, sizeof(CPDisinfectPlan));
	CPFilterRinseLast = 0;
	CPDrainLast = 0;
	CPDrainStartTime = 0;
	CPMembraneRinseLast = 0;
	CPDisinfectLast = 0;
	CPIsAlive = false;
	CPDisinfectStarted = false;
	//

	appEjectTimeoutTriggered = false;
	disinfectTimeoutTriggered = false;
	disinfectMembraneTimeoutTriggered = false;
	productionTestTimeoutTriggered = false;
	CPUserInfo.accountState = 0x00; // no activated card
    CPUserInfo.card_moving_flag = 0x00;

	memset(CRVer, 0, 3);
	CREnabled = true;
	CRCheckTime = 0;
	CRIsAlive = false;
	bICCardInserted = false;

	//
	filterRinseMaxCount = 1; //default once

	//
	CPRemainWorkableDays = 365; // 12 month
	CPFilterExpiredInformed = false;
	calcFilterExpiredNS(true);

    //
    vodasDeviceType = DEVICE_TYPE_SK_I;

    //
    UVPowerOffDelaySec = 30;
    DustHelmetCloseDelaySec = 10;
    AppPayEjectDelaySec = 5; // after operated
    AppPayEjectDelaySec2 = 10; // second time delay after operated
    AppPayEjectNoActionDelaySec = 15; // never operated
    WechatPayEjectDelaySec = 60; // after operated
    WechatPayNoActionEjectDelaySec = 60; // never operated

    waterReleaseTimes = 0;

    memset(configParamSecret, 0, 64);
    configParamHeatPreservation = 1;
    configParamQRScanner = 1;
    configParamDustHelmet = 1;
    configParamICCardReader = 1;
    configParamSK_II = 0;

	CPInvasionDetectedClosTime = 0;

	preValveStatus = 0;
	valveStatus = 0;

	//
	upgrade_init();
}

Controller::~Controller()
{

}

int Controller::boot()
{
	Action* pAct = NULL;

	// load configs
	int retVal = PersistentStorage::getInstance()->loadAllConfig();
	if(0 != retVal)
	{
		// goto failed
		FTRACE("###Controller::boot() PersistentStorage loadAllConfig failed retVal: %d\r\n", retVal);
	}

	// turn on feed water(B3) valve
	//if(DigitalSwitches::getInstance())
	//	DigitalSwitches::getInstance()->turnOnFeedWaterValve(true);

	// check water heater
	pAct = new Action(Action::AID_WH_HANDSHAKE);
	ActionBase::append(pAct);                                  

	// check IC Card Reader
	pAct = new Action(Action::AID_CR_HANDSHAKE);
	ActionBase::append(pAct);

	// update version to Central Panel
	//pAct = new Action(Action::AID_CP_VERSION);
	//ActionBase::append(pAct);

	// check reboot state
	if(StateMachine::getInstance()->inProductionTestState())
	{
		LOG_INFO("###Controller::boot() boot from Product test state. testStep: %d\r\n", productionTestStep);
		// continue doing product test
		if(productionTestStep < 4) // filter rinse is not finished;
		{
			DigitalSwitches::getInstance()->startFilterRinse(false);
			productionTestStep = 0;
		}
		else
			productionTestStep--; // return to last step;
		startProductionTestTimeout(30); //
	}
	//else if(StateMachine::getInstance()->inFilterRinseState())
	//{
		// move 631 valve to In-serv position
	//	DigitalSwitches::getInstance()->startFilterRinse(false);
	//}
	else
		StateMachine::getInstance()->transferToStandby();

	// check invasion flag when booted
	if(Controller::getInstance()->CPInvasionDetected)
	{
		WarningManager::getInstance()->triggerWarning(WarningManager::WARNING_TYPE_INVASION);
	}

#ifdef TDS_CONTROL_SWITCH
	//Whether to open the mixed water valve
	if(CWaterMixerValve::getInstance()->getEnabled())
	{
    	CWaterMixerValve::getInstance()->outputRotationalPulse(CWaterMixerValve::getInstance()->mPulseWidth);
	    CWaterMixerValve::getInstance()->setWantedTDS(CWaterMixerValve::getInstance()->mWantedTDS);
	}
#endif
	// reject IC card if exist
	//pAct = new ActionWaterInfo(0, 0, 0, 0, 0, 12345678);
	//ActionBase::append(pAct);
	return 0;
}

int Controller::shedule_loop(time_t seconds)
{
	// it is a loop for every one second

	// check and execute fixedtime and planed action
	checkPlanActions(seconds);

	// check service time
	//if(checkInService())
	//{
	//	if(!SYSInService)
	//	{
	//		SYSInService = true;
			// transfer to standby state; in service
	//	}
	//}
	//else
	//{
	//	if(SYSInService)
	//	{
	//		SYSInService = false;
			// transfer to Hibernate state; out of service
	//	}
	//}

	//valve Status
	getValveState();
	if(valveStatus != preValveStatus)
	{
		FTRACE("\n-----valveStatus : 0x%d%d%d%d   ------preValveStatus : 0x%d%d%d%d\r\n",(valveStatus & 0x08) >> 3,(valveStatus & 0x04) >> 2,(valveStatus & 0x02) >> 1,(valveStatus & 0x01),(preValveStatus & 0x08) >> 3,(preValveStatus & 0x04) >> 2,(preValveStatus & 0x02) >> 1,(preValveStatus & 0x01));

		preValveStatus = valveStatus;
		Controller::getInstance()->updateOutletValveMark(valveStatus);
	}

	// check heater work time
	if(StateMachine::getInstance()->inStandbyState())
	{
		if(checkWHWorkTime())
		{
			if(!WHInService)
			{
				// start water heater working
				WHInService = true;
				ActionStartHeating* pAct = new ActionStartHeating(true);
				ActionBase::append(pAct);
			}
		}
		else
		{
			if(WHInService)
			{
				// stop water heater working
				WHInService = false;
				ActionStartHeating* pAct = new ActionStartHeating(false);
				ActionBase::append(pAct);
			}
		}
	}

	// action time out
	if(ActionBase::header() && (ActionBase::header()->status == ActionBase::EXECUTED))
	{
		if(seconds > ActionBase::header()->startTime+ACTION_TIMEOUT || 
			seconds < ActionBase::header()->startTime) // system time reset
		{
			FTRACE("###Controller::shedule_loop() RE-SEND %d(%s)\r\n", ActionBase::header()->getActionId(),
                        ActionBase::header()->getActionName());
			if(ActionBase::header()->sendCount >= ACTION_RESEND_LIMIT || ActionBase::header()->action() != 0) // failed
			{
				//
				if(ActionBase::header()->getActionId() < Action::AID_WH_LAST)
				{
					// water heater no response
					LOG_WARN("###Controller::shedule_loop() Water Heater no response\r\n");
				}
				else if(ActionBase::header()->getActionId() < Action::AID_CP_LAST)
				{
					// Central Panel no response
					LOG_WARN("###Controller::shedule_loop() Central Panel no response\r\n");
					if(CPIsAlive)
					{
						CPIsAlive = false;
						rebootCentralPanel();
					}
				}
				else if(ActionBase::header()->getActionId() < Action::AID_CR_LAST)
				{
					// IC Card Reader no response
					LOG_WARN("###Controller::shedule_loop() IC Card Reader no response\r\n");
					CRIsAlive = false;
					WarningManager::getInstance()->triggerWarning(WarningManager::WARNING_TYPE_IC_CARD_READER_ERR);
				}

				ActionBase::pop();
			}
		}
	}

	// production Test
	if(StateMachine::getInstance()->inProductionTestState())
		productionTestProcess();

	// process timeout functions
	timeoutProcess();

	// Filter remain days changed
	if((CPFilterExPiredNS - seconds) / 86400 != CPRemainWorkableDays) //  seconds of a day; 86400 = 24 * 60 * 60;
	{
		CPRemainWorkableDays = (CPFilterExPiredNS - seconds) / 86400;
		LOG_INFO("###Controller::shedule_loop() Filter ramain days: %d\r\n", CPRemainWorkableDays);

		Action* pAct = new Action(Action::AID_CP_FILTERWORKABLEDAYS, this);
		ActionBase::append(pAct);
	}
	// check filter shelf life
	if(((CPFilterCheckMode == 0) && ((CPRemainWorkableDays <= 0) || (CPFilterWaterMax < CPFilterWaterCur))) // both
		|| ((CPFilterCheckMode == 1) && (CPFilterWaterMax < CPFilterWaterCur)) // flowmeter
		|| ((CPFilterCheckMode == 2) && (CPRemainWorkableDays <= 0))) // filter shelf life
	{
		if(!CPFilterExpiredInformed)
		{
			CPFilterExpiredInformed = true;
			WarningManager::getInstance()->triggerWarning(WarningManager::WARNING_TYPE_FILTER_EXPIRED);
		}
	}
	else
	{
		// filter reset case
		if(CPFilterExpiredInformed)
		{
			CPFilterExpiredInformed = false;
			WarningManager::getInstance()->resolveWarning(WarningManager::WARNING_TYPE_FILTER_EXPIRED);
		}
	}

	// check IC card is alive
	if(CRIsAlive)
	{
		CRCheckTime ++;
		if(CRCheckTime > CHECK_INTERVAL_IC_CARD_READER)
		{
			CRCheckTime = 0;

			Action* pAct = new Action(Action::AID_CR_ENABLEREADER);
			ActionBase::append(pAct);
		}
	}

	return 0;
}

int Controller::action_loop()
{
	if(ActionBase::header() && (ActionBase::header()->status == ActionBase::PENDING))
	{
#if !PERIPHERAL_DEVICE_WATER_HEATER
		// drop water heater actions if no water heater configed
		if(ActionBase::header()->getActionId() < Action::AID_WH_LAST)
		{
			ActionBase::pop();
			return 0;
		}
#endif
#if !PERIPHERAL_DEVICE_CENTRAL_PANEL
		// drop Central Panel actions if no Central Panel configed
		if((ActionBase::header()->getActionId() > Action::AID_WH_LAST) &&
			(ActionBase::header()->getActionId() < Action::AID_CP_LAST))
		{
			ActionBase::pop();
			return 0;
		}
#endif

		// drop IC Card Reader actions if no IC Card Reader configed
		if((ActionBase::header()->getActionId() > Action::AID_CP_LAST) &&
			(ActionBase::header()->getActionId() < Action::AID_CR_LAST))
		{
#if !PERIPHERAL_DEVICE_IC_CARD_READER
			ActionBase::pop();
			return 0;
#endif
            if(configParamICCardReader == 0)
            {
                ActionBase::pop();
			    return 0;
            }
		}

		if(ActionBase::header()->action() != 0) // failed
		{
			ActionBase::pop();
		}
	}

	// for ic card insert action
	if(bICCardInserted)
	{
		bICCardInserted = false;
		ICCardInsertedProc();
	}

    //const char* buf = "";    
    //setDeviceId(buf);
    //char* temp = "";    
    //setConfigSecret(temp);

	return 0;
}

int Controller::startAppAccountEjectCountdown(int sec)
{
    //FTRACE("###Controller::startAppAccountEjectCountdown() sec: %d\r\n", sec);
	stopAppAccountEjectCountdown();

	CPAppEjectCountdown = sec;
	//Action *pAct = new Action(Action::AID_CP_COUNTDOWN, this);
	//ActionBase::append(pAct);
	//CPAppEjectTimeout.attach(Callback<void()>(this, &Controller::appEjectTimeoutFun), sec);

    appEjectTimeoutTriggered = true;

	return 0;
}

int Controller::stopAppAccountEjectCountdown()
{
	CPAppEjectCountdown = 0;
	CPAppEjectTimeout.detach();
	appEjectTimeoutTriggered = false;

	return 0;
}

int Controller::checkAccountForColdWaterSelling()
{
	// warning restrict
	if(WarningManager::getInstance()->isColdWaterForbidden())
		return 1;

	// Central Panel Forbid the Cold water release
	if(!CPColdWaterSwitchEnabled)
		return 2;

	// if it is free
	if(CPColdPrice <= 0 && (StateMachine::getInstance()->inStandbyState() || StateMachine::getInstance()->inSoldState()))
    {
        //
		return 0;
    }

	// not in sold state
	if(!StateMachine::getInstance()->inSoldState())
		return 3;

	// no activated account
	if((CPUserInfo.accountState & 0x01) == 0x00)
		return 4;

	// no balance
	if(CPUserInfo.balance <= 0)
		return 5;

	return 0;
}

int Controller::checkAccountForHotWaterSelling()
{
	// warning restrict
	if(WarningManager::getInstance()->isHotWaterForbidden())
		return 1;

	// Central Panel Forbid the Hot water release
	if(!CPHotWaterSwitchEnabled)
		return 2;

	// water heater is out of service
	if(!WHInService)
		return 6;

	// if it is free
	if(CPHotPrice <= 0 && (StateMachine::getInstance()->inStandbyState() || StateMachine::getInstance()->inSoldState()))
    {
        //
        if(isRestrictHotWaterDevice())
        {
            if((CPUserInfo.accountState & 0x01) == 0x00) // not activated
		        return 7;
        }
		return 0;
    }

	// not in sold state
	if(!StateMachine::getInstance()->inSoldState())
		return 3;

	// no activated account
	if((CPUserInfo.accountState & 0x01) == 0x00)
		return 4;

	// no balance
	if(CPUserInfo.balance <= 0)
		return 5;

	return 0;
}

int Controller::checkAccountForIceWaterSelling()
{
    return checkAccountForHotWaterSelling();
}

int Controller::updateAccountInfo(bool bfinished)
{
	// milli litre
	int coldcap = 1000*CPUserInfo.coldWaterPulse / CPPulsePerLitreArr[CP_COLD_PULSE_PER_LITRE];
	int hotcap = 1000*CPUserInfo.hotWaterPulse / CPPulsePerLitreArr[CP_HOT_PULSE_PER_LITRE];
	// cents
	int coldcost = coldcap * CPColdPrice / 1000;
	int hotcost = hotcap * CPHotPrice / 1000;
	if(CPUserInfo.accountState && CPUserInfo.accountType == 2) // wechat pay
	{
		if(CPUserInfo.waterLimitMilliLitre > 0)
		{
			coldcost = CPUserInfo.waterLimitBalance * coldcap / CPUserInfo.waterLimitMilliLitre;
			hotcost = CPUserInfo.waterLimitBalance * hotcap / CPUserInfo.waterLimitMilliLitre;
		}
	}
	int balance = CPUserInfo.balance - coldcost - hotcost;
	bool bSellingWater = bfinished ? false : true;
	if(CPUserInfo.accountState && (CPColdPrice > 0 || CPHotPrice > 0))
	{
	    if(balance <= 0)
	    {
		    // out of balance
            if(CPColdPrice > 0)
		        DigitalSwitches::getInstance()->coldWaterSwitchClose();
            if(CPHotPrice > 0)
		        DigitalSwitches::getInstance()->hotWaterSwitchClose();
		    balance = 0;
		    CPUserInfo.balance = 0; // can't release water
		    //bSellingWater = false;
	    }
	}
	else
	{
		// free case
		balance = 0;
		if(!bSellingWater){
			CPUserInfo.coldWaterPulse = 0;
			CPUserInfo.hotWaterPulse = 0;
			coldWaterFlowPulse = 0;
			hotWaterFlowPulse = 0;
            iceWaterFlowPulse = 0;
		}
	}

	Action* pAct = NULL;
	if(CPUserInfo.accountState && (CPColdPrice > 0 || CPHotPrice > 0))
	{
		if(CPUserInfo.accountType) // app/wechat
		{
			pAct = new ActionWaterInfo(coldcost, coldcap, hotcost, hotcap, bSellingWater);
			ActionBase::append(pAct);
		}
		else // ic card
		{
			// save IC Card balance
			if(!bfinished)
			{
				pAct = new ActionICCardWrite(this, balance);
				ActionBase::append(pAct);
			}

			pAct = new ActionWaterInfo(coldcost, coldcap, hotcost, hotcap, bSellingWater, balance);
			ActionBase::append(pAct);
		}
	}
	else
	{
		// free case
		pAct = new ActionWaterInfo(coldcost, coldcap, hotcost, hotcap, bSellingWater);
		ActionBase::append(pAct);
	}

	return 0;
}

int Controller::setMaxLoopCountOfFilterRinse(int count)
{
	filterRinseMaxCount = count;
	return 0;
}

//
int Controller::setFilterRinseFixTime(int day, int hour, int min)
{
	CPFilterRinseFixTime.day   = day;
	CPFilterRinseFixTime.hour  = hour;
	CPFilterRinseFixTime.min   = min;

	// save to PS
	LOG_INFO("###Controller::setFilterRinseFixTime() day: %d  hour|min: 0x%08X\r\n", day, hour<<16 | min);
	return PersistentStorage::getInstance()->saveFilterRinseFixtime();
	//return 0;
}

int Controller::rebootCentralPanel()
{
	LOG_INFO("###Controller::rebootCentralPanel()\r\n");

	DigitalSwitches::getInstance()->powerOffCentralPanel(true);
	rebootDeviceType = 1; //Central Panel
	CPRebootTimeout.attach(Callback<void()>(this, &Controller::rebootTimeoutFun), 2); // reboot in 2 sec
	return 0;
}

int Controller::setHeatTemperature(int temp)
{
	LOG_INFO("###Controller::setHeatTemperature() boiling temperature: %d\r\n", temp);
	WHBoilingTemp = temp;

#if ENABLE_TEMPERATURE_COMPENSATION
	ActionSetBoilTemp* pAct = new ActionSetBoilTemp(WHBoilingTemp-CPCompensation.compensation, WHTempOffset);
#else
	ActionSetBoilTemp* pAct = new ActionSetBoilTemp(WHBoilingTemp, WHTempOffset);
#endif
	ActionBase::append(pAct);

	// save to PS
	return PersistentStorage::getInstance()->saveTemperature();
	//return 0;
}

int Controller::setUnitPrice(int hot, int cold, int freeze)
{
	LOG_INFO("###Controller::setUnitPrice() hot: %d cold: %d freeze: %d  cents per litre\r\n", hot, cold, freeze);
	CPHotPrice = hot;
	CPColdPrice = cold;
    CPIcePrice = freeze;
    
	int value = 0;

	// save to PS
	value = PersistentStorage::getInstance()->saveHotWaterPrice();
	if(value == 0)
	{
		value = PersistentStorage::getInstance()->saveColdWaterPrice();
		//PersistentStorage::getInstance()->saveIceWaterPrice();
	}
	
	return value;
}

int Controller::setWaterLimit(int litre)
{
	LOG_INFO("###Controller::setWaterLimit() litre: %d\r\n", litre);
	CPFilterWaterMax = litre;

	// save to ps
	return PersistentStorage::getInstance()->saveFilterWaterMax();
	//return 0;
}

int Controller::setDelayTime(int min)
{
	LOG_INFO("###Controller::setDelayTime() min: %d\r\n", min);
	CPPlanDelayTime = min;

	return 0;
}

int Controller::setWaterCurrent(int litre)
{
	LOG_INFO("###Controller::setWaterCurrent() litre: %d\r\n", litre);
	CPFilterWaterCur = litre;

	// save to ps
	return PersistentStorage::getInstance()->saveFilterWaterCur();
	//return 0;
}

int Controller::setFilterProduceDate(int year, int month, int day)
{
	LOG_INFO("###Controller::setFilterProduceDate() year: %d month|day: 0x%08X\r\n", year, month<<16 | day);
	CPFilterProductionDate.year = year;
	CPFilterProductionDate.month = month;
	CPFilterProductionDate.day = day;

	//
	calcFilterExpiredNS(false);

	// save to ps
	return PersistentStorage::getInstance()->saveFilterProductionDate();
	//return 0;
}

int Controller::setFilterRinseDur(int backmin, int fastmin)
{
	LOG_INFO("###Controller::setFilterRinseDur() back rinse duration: %d  fast rinse duration: %d\r\n", backmin, fastmin);
	CPFilterBackRinseDur = backmin;
	CPFilterFastRinseDur = fastmin;

	// save to ps
	return PersistentStorage::getInstance()->saveFilterRinseDur();
	//return 0;
}

int Controller::setChargeWay(int model, int unit)
{
	LOG_INFO("###Controller::setChargeWay() model: %d unit: %d\r\n", model, unit);
	CPChargeMode = model;
	CPChargeUnit = unit;

	return 0;
}

int Controller::setMembraneRinseDur(int min)
{
	LOG_INFO("###Controller::setMembraneRinseDur() duration: %d minutes\r\n", min);
	CPMembraneRinseDur = min;

	// save to ps
	return PersistentStorage::getInstance()->saveMembraneRinseDur();
	//return 0;
}

int Controller::setServiceTime(int day, int start[4], int end[4])
{
	int i;
	FTRACE("###Controller::setServiceTime() day: %d [%d,%d] [%d,%d] [%d,%d] [%d,%d]\r\n", day, 
			start[0], end[0], start[1], end[1], start[2], end[2], start[3], end[3]);
	if(day<1 || day>7)
		return -1; // bad parameter

	day -= 1;
	for(i=0; i<4; i++)
	{
		CPServiceTime[day].start[i] = start[i];
		CPServiceTime[day].end[i] = end[i];
	}

	// store to PS
	return PersistentStorage::getInstance()->saveServiceTime();
	//return 0;
}

int Controller::setFilterShelflife(int month)
{
	LOG_INFO("###Controller::setFilterShelflife() month: %d\r\n", month);
	CPFilterShelfLife = month;

	//
	calcFilterExpiredNS(false);

	// save to PS
	return PersistentStorage::getInstance()->saveFilterShelfLife();
	//return 0;
}

int Controller::setCompensationTemp(int offset, int temp)
{
	FTRACE("###Controller::setCompensationTemp() offset: %d  temp: %d\n", offset, temp);
	CPCompensation.startTemp = temp;
	CPCompensation.compensation = offset;

#if ENABLE_TEMPERATURE_COMPENSATION
	ActionSetBoilTemp* pAct = new ActionSetBoilTemp(WHBoilingTemp-CPCompensation.compensation, WHTempOffset);
	ActionBase::append(pAct);
#endif

	// save to PS
	return PersistentStorage::getInstance()->saveTempCompensationParam();
	//return 0;
}

int Controller::setHeaterWorkTime(int day, int start[4], int end[4])
{
	int i;
	FTRACE("###Controller::setHeaterWorkTime() day: %d [%d,%d] [%d,%d] [%d,%d] [%d,%d]\r\n", day, 
			start[0], end[0], start[1], end[1], start[2], end[2], start[3], end[3]);
	if(day<1 || day>7)
		return -1; // bad parameter

	day -= 1;
	for(i=0; i<4; i++)
	{
		CPHeaterWorkTime[day].start[i] = start[i];
		CPHeaterWorkTime[day].end[i] = end[i];
	}

	// store to PS
	return PersistentStorage::getInstance()->saveHeaterWorkTime();

	//return 0;
}

int Controller::setDisinfectDur(int min)
{
	LOG_INFO("###Controller::setDisinfectDur() duration: %d minutes\r\n", min);
	CPDisinfectDur = min;

	// save to PS
	return PersistentStorage::getInstance()->saveDisinfectDur();
	//return 0;
}

int Controller::setPulsePerLitre(int totalpulse, int hotpulse, int coldpulse, int icepulse, int pulse5, int pulse6)
{
	LOG_INFO("###Controller::setPulsePerLitre() totalpulse: %d ,hotpulse: %d ,coldpulse: %d ,icepulse: %d ,pulse5: %d ,pulse6: %d \r\n", totalpulse, hotpulse, coldpulse, icepulse, pulse5, pulse6);

	CPPulsePerLitreArr[CP_PULSE_PER_LITRE] = totalpulse;
	CPPulsePerLitreArr[CP_HOT_PULSE_PER_LITRE] = hotpulse;
	CPPulsePerLitreArr[CP_COLD_PULSE_PER_LITRE] = coldpulse;
	CPPulsePerLitreArr[CP_ICE_PULSE_PER_LITRE] = icepulse;
	CPPulsePerLitreArr[CP_PULSE_PER_LITRE_5] = pulse5;
	CPPulsePerLitreArr[CP_PULSE_PER_LITRE_6] = pulse6;
	// save to PS
	return PersistentStorage::getInstance()->savePulsePerLitre();
	//return 0;
}

int Controller::setFilterCheckMode(int mode)
{
	FTRACE("###Controller::setFilterCheckMode() mode: %d\r\n", mode);
	CPFilterCheckMode = mode;

	// save to PS
	return PersistentStorage::getInstance()->saveFilterCheckMode();
	//return 0;
}

int Controller::setDisinfectDelayTime(int min)
{
	FTRACE("###Controller::setFilterCheckMode() min: %d\r\n", min);
	CPDisinfectDelayTime = min;

	// save to PS
	return PersistentStorage::getInstance()->saveDisinfectDelayMax();
	//return 0;
}

int Controller::setSysTime(int y, int m, int d, int h, int min, int s)
{
	struct tm nt = {0};
	FTRACE("###Controller::setSysTime() %d.%d.%d %d:%d:%d\r\n", y, m, d, h, min, s);	

	nt.tm_sec = s;
	nt.tm_min = min;
	nt.tm_hour = h;
	nt.tm_mday = d;
	nt.tm_mon = m - 1;
	nt.tm_year = y - 1900;
	time_t seconds = mktime(&nt);
	set_time(seconds);

#if ENABLE_EXTERNAL_RTC && USING_EXTERNAL_RTC_AS_SYNC_TIME
	FTRACE("###Controller::setSysTime() sync extern RTC\r\n");
	extern void pcf8563_write_rtc(time_t time);
	pcf8563_write_rtc(seconds);
#endif

	return 0;
}

int Controller::setUserCardState(int state)
{
	LOG_INFO("###Controller::setUserCardState() %d\r\n", state);
    
//	if(CRCardStatus != 0x01) // card insert status --- CRCardStatus(1: legal; 2: illegeal; 3: card removed)
//	{
//		// if remove card happen during upload card info and set card state
//		// central panel will dead locked
//		// send water sold finished event
//		if(state == 0)
//		{
//			//Action* pAct = new ActionWaterInfo(0, 0, 0, 0, 0, 12345678);
//			Action* pAct = new ActionWaterInfo(0, 0, 0, 0, 0, 0);
//			ActionBase::append(pAct);
//		}
//		return 0;
//	}

	//0: normal card
	//1: locked card
	//2: region error
	if(state == 1) // locked card
	{
		CPUserInfo.accountState = 0; // deactived
		// write lock flag
		CPUserInfo.cardType |= 0x80; // set lock flag

		ActionICCardWrite* pActIcCard = new ActionICCardWrite(this, CPUserInfo.balance);
		ActionBase::append(pActIcCard);

		StateMachine::getInstance()->transferToStandby();
        
        ActionCardInfo* pAct = new ActionCardInfo(CPUserInfo.icCardId, 0, 0, 1, CPUserInfo.cardType);
		ActionBase::append(pAct);
	}
	else if(state == 2) // region error
	{
		CPUserInfo.accountState = 0; // not actived

		StateMachine::getInstance()->transferToStandby();

		//static int regionId = 0;
		//ActionCardInfo* pAct = new ActionCardInfo(CPUserInfo.icCardId, CPUserInfo.balance, regionId, false);
		//ActionBase::append(pAct);
	}
	else if(state == 0) // OK
	{
		CPUserInfo.accountState = 0x01; // activated

		StateMachine::getInstance()->transferToSoldState();
	}
	else
	{
		LOG_ERROR("###Controller::setUserCardState() unknown state\r\n");
		CPUserInfo.accountState = 0; // not actived
		StateMachine::getInstance()->transferToStandby();
	}

	return 0;
}

int Controller::setAppBalance(int cents)
{
	LOG_INFO("###Controller::setAppBalance() balance: %d\r\n", cents);
	if(!StateMachine::getInstance()->inStandbyState())
		return 0;

	CPUserInfo.balance = cents;
	CPUserInfo.accountState = 0x01;
	CPUserInfo.accountType = 1; // App
	CPUserInfo.hotWaterPulse = 0;
	CPUserInfo.coldWaterPulse = 0;

    waterReleaseTimes = 0; // reset water release times

	StateMachine::getInstance()->transferToSoldState();

	startAppAccountEjectCountdown(AppPayEjectNoActionDelaySec);

	return 0;
}

int Controller::setSwichLock(int bUnlock)
{
	LOG_INFO("###Controller::setSwichLock() bUnlock(0: locked; 1: unlock): %d\r\n", bUnlock);
	CPDoorOpenForbidden = !bUnlock;

	return 0;
}

int Controller::enableWaterSwitch(int type, int bDisable)
{
	LOG_INFO("###Controller::enableWaterSwitch() type: %d  bDisable: %d\r\n", type, bDisable);
	// type: 0: cold; 1: hot; 2: ice;
	if(type == 1)
	{
		CPHotWaterSwitchEnabled = !bDisable;
	}
	else if(type == 0)
	{
		CPColdWaterSwitchEnabled = !bDisable;
	}

	return 0;
}

int Controller::openFeedWaterSwitch(int type, int bOff)
{
	LOG_INFO("###Controller::openFeedWaterSwitch() type: %d bOff: %d\r\n", type, bOff);
	if(type == 0)
	{
		DigitalSwitches::getInstance()->turnOnFeedWaterValve(!bOff);
	}

	return 0;
}

int Controller::setDrainFixTime(int day, int hour, int min)
{
	LOG_INFO("###Controller::setDrainFixTime() day: %d  hour: %d\r\n", day, hour);
	CPDrainFixTime.valid = true;
	CPDrainFixTime.day = day;
	CPDrainFixTime.hour = hour;
	CPDrainFixTime.min = min;

	// save to PS
	return PersistentStorage::getInstance()->saveDrainFixtime();
	//return 0;
}

int Controller::setFilterRinsePlan(int y, int m, int d, int h, int min)
{
	FTRACE("###Controller::setFilterRinsePlan() %d.%d.%d %d:%d\r\n", y, m, d, h, min);
	CPFilterRinsePlan.year = y;
	CPFilterRinsePlan.month = m;
	CPFilterRinsePlan.day = d;
	CPFilterRinsePlan.hour = h;
	CPFilterRinsePlan.min = min;
	if(y==0 && m==0 && d==0 && h==0 && min==0)
	{
		CPFilterRinsePlan.plan_time = 0;
		CPFilterRinsePlan.valid = false;
	}
	else
	{
		struct tm nt;
		memset(&nt, 0, sizeof(struct tm));
		//nt.tm_sec = 0;
		nt.tm_min = min;
		nt.tm_hour = h;
		nt.tm_mday = d;
		nt.tm_mon = m - 1;
		nt.tm_year = y - 1900;
		CPFilterRinsePlan.plan_time = mktime(&nt);
		CPFilterRinsePlan.valid = true;
	}

	// save to PS
	return PersistentStorage::getInstance()->saveFilterRinsePlan();
	//return 0;
}

int Controller::setDrainPlan(int y, int m, int d, int h, int min)
{
	FTRACE("###Controller::setDrainPlan() %d.%d.%d %d:%d\r\n", y, m, d, h, min);
	CPDrainPlan.year = y;
	CPDrainPlan.month = m;
	CPDrainPlan.day = d;
	CPDrainPlan.hour = h;
	CPDrainPlan.min = min;
	if(y==0 && m==0 && d==0 && h==0 && min==0)
	{
		CPDrainPlan.plan_time = 0;
		CPDrainPlan.valid = false;
	}
	else
	{
		struct tm nt;
		memset(&nt, 0, sizeof(struct tm));
		//nt.tm_sec = 0;
		nt.tm_min = min;
		nt.tm_hour = h;
		nt.tm_mday = d;
		nt.tm_mon = m - 1;
		nt.tm_year = y - 1900;
		CPDrainPlan.plan_time = mktime(&nt);
		CPDrainPlan.valid = true;
	}

	// save to PS
	return PersistentStorage::getInstance()->saveDrainPlan();
	//return 0;
}

int Controller::getVersion()
{
	LOG_INFO("###Controller::getVersion()\r\n");

	// update version to Central Panel
	Action* pAct = new Action(Action::AID_CP_VERSION);
	ActionBase::append(pAct);

	// trick code; upload the remain days of filter
	pAct = new Action(Action::AID_CP_FILTERWORKABLEDAYS, this);
	ActionBase::append(pAct);

	return 0;
}

int Controller::getConfiguration()
{
	FTRACE("###Controller::getConfiguration()\r\n");
	if(ActionBase::header() && ActionBase::header()->getActionId() == Action::AID_CP_CONFIGINFO)
	{
		// current is reporting
		
	}
	else
	{
		ActionConfigInfo* pAct = new ActionConfigInfo(this);
		ActionBase::append(pAct);
	}

	return 0;
}

int Controller::setDeviceId(const char* deviceId)
{
	if(deviceId)
		memcpy(CPDeviceId, deviceId, 32);
	CPDeviceId[32] = '\0';

	FTRACE("###Controller::setDeviceId() %s\r\n", CPDeviceId);

	PersistentStorage::getInstance()->saveDeviceId();

	return 0;
}

int Controller::enableBLEDebug(int bOn)
{
	FTRACE("###Controller::enableBLEDebug() bOn: %d\n", bOn);
	CPBLEDebugEnabled = bOn;

	return 0;
}

int Controller::setMembraneRinseFixTime(int day, int hour, int min)
{
	CPMembraneRinseFixTime.day   = day;
	CPMembraneRinseFixTime.hour  = hour;
	CPMembraneRinseFixTime.min   = min;

	// save to PS
    LOG_INFO("###Controller::setMembraneRinseFixTime() day: %d  hour|min: 0x%08X\r\n", day, hour<<16 | min);
	return PersistentStorage::getInstance()->saveMembraneRinseFixtime();
	//return 0;
}

int Controller::setMembraneRinsePlan(int y, int m, int d, int h, int min)
{
	FTRACE("###Controller::setMembraneRinsePlan() %d.%d.%d %d:%d\r\n", y, m, d, h, min);
	CPMembraneRinsePlan.year = y;
	CPMembraneRinsePlan.month = m;
	CPMembraneRinsePlan.day = d;
	CPMembraneRinsePlan.hour = h;
	CPMembraneRinsePlan.min = min;
	if(y==0 && m==0 && d==0 && h==0 && min==0)
	{
		CPMembraneRinsePlan.plan_time = 0;
		CPMembraneRinsePlan.valid = false;
	}
	else
	{
		struct tm nt;
		memset(&nt, 0, sizeof(struct tm));
		//nt.tm_sec = 0;
		nt.tm_min = min;
		nt.tm_hour = h;
		nt.tm_mday = d;
		nt.tm_mon = m - 1;
		nt.tm_year = y - 1900;
		CPMembraneRinsePlan.plan_time = mktime(&nt);
		CPMembraneRinsePlan.valid = true;
	}

	// save to PS
	return PersistentStorage::getInstance()->saveMembraneRinsePlan();
}

int Controller::drain()
{
	LOG_INFO("###Controller::drain()\r\n");
	
	return StateMachine::getInstance()->transferToDrainage();
}

int Controller::membraneRinse()
{
	LOG_INFO("###Controller::membraneRinse()\r\n");

	return StateMachine::getInstance()->transferToMembraneRinse();
}

int Controller::filterRinse()
{
	LOG_INFO("###Controller::filterRinse()\r\n");

	return StateMachine::getInstance()->transferToFilterRinse();
}

int Controller::getDeviceId()
{
	FTRACE("###Controller::getDeviceId()\r\n");
	Action* pAct = new Action(Action::AID_CP_DEVICEID, this);
	ActionBase::append(pAct);

	return 0;
}

int Controller::wechatAppPay(int cents, int ml)
{
	LOG_INFO("###Controller::wechatAppPay() ballance: %d cents  water: %d ml\r\n", cents, ml);
	if(!StateMachine::getInstance()->inStandbyState())
		return 0;

	CPUserInfo.accountType = 2; //wechat
	CPUserInfo.accountState = 1; //activated
	CPUserInfo.balance = cents;
	CPUserInfo.waterLimitBalance = cents;
	CPUserInfo.waterLimitMilliLitre = ml;
	CPUserInfo.hotWaterPulse = 0;
	CPUserInfo.coldWaterPulse = 0;

    waterReleaseTimes = 0; // reset water release times

	StateMachine::getInstance()->transferToSoldState();

	startAppAccountEjectCountdown(WechatPayNoActionEjectDelaySec);
	return 0;
}

int Controller::setCentralPanelVer(const char* ver)
{
	FTRACE("###Controller::setCentralPanelVer() version: %s\r\n", ver);
	return 0;
}

int Controller::writeDLFirmware(int offset, const char* data, int size)
{
	FTRACE("###Controller::writeDLFirmware() offset: %d size: %d\r\n", offset, size);

	// write to flash
	uint32_t base_addr = upgrade_get_base_addr();
	if(offset == 0)
	{
		upgrade_erase(base_addr);
        upgrade_firmware_size = 0;

        //FTRACE("###Controller::writeDLFirmware eraser: base_addr: 0x%08X\r\n", base_addr);
        LOG_INFO("###Controller::writeDLFirmware() baseAddr: 0x%X\r\n", base_addr);
	}

	if(upgrade_flash_copy(base_addr+FIRMWARE_DL_SECTOR_SIZE*offset, (const uint8_t*)data, size))
	{
		FTRACE("###Controller::writeDLFirmware() offset: %d flash copy failed\r\n", offset);
		return -1;
	}

    upgrade_firmware_size += size;

	return 0;
}

int Controller::installFirmware(unsigned int hashcode)
{
	LOG_INFO("###Controller::installFirmware() hashcode: 0x%08X\r\n", hashcode);

	// upload Firmware upgrade start event
	//ActionFirmwareUpgradeStatus* pAct = new ActionFirmwareUpgradeStatus(0);
	//ActionBase::append(pAct);

    int result = upgrade_check_hash_code(upgrade_firmware_size, hashcode);
    upgrade_firmware_size = 0;
    if(result < 0)
    {
        // failed
        LOG_ERROR("###Controller::installFirmware() check hash  failed\r\n");
        return -1;
    }

	upgrade_commit();

	// set reboot timer; reboot in 2 sec
	rebootDeviceType = 1; //Mbed
	CPRebootTimeout.attach(Callback<void()>(this, &Controller::rebootTimeoutFun), 2); // reboot in 2 sec

	// upload Firmware upgrade finished event
	ActionFirmwareUpgradeStatus* pAct = new ActionFirmwareUpgradeStatus(1);
	ActionBase::append(pAct);

	return 0;
}

int Controller::getLogFile()
{
	FTRACE("###Controller::getLogFile()\r\n");

	// upload log start event
	ActionOperateLogStatus* pAct = new ActionOperateLogStatus(0);
	ActionBase::append(pAct);

	// start upload LOGs
	PersistentStorage::getInstance()->readSavedLog();

	return 0;
}

int Controller::startProdTest()
{
	LOG_INFO("###Controller::startProdTest() \r\n");
	productionTestTimeoutTriggered = false;
	productionTestStep = 0; // initial state

	return StateMachine::getInstance()->transferToProductionTest();
}

void Controller::exitProdTest()
{
	if(StateMachine::getInstance()->inProductionTestState())
	{
		if(productionTestStep <= 3) // filter rinse
		{
			DigitalSwitches::getInstance()->startFilterRinse(false);
		}
		else if(productionTestStep == 5) // membrane rinse
		{
			ActionStartRinse* pAct = new ActionStartRinse(false);
			ActionBase::append(pAct);
		}
		else if(productionTestStep == 9) // drain
		{
			ActionStartDrain* pAct = new ActionStartDrain(false);
			ActionBase::append(pAct);
		}

		ActionProductTestStatus* pAct = new ActionProductTestStatus(1);
		ActionBase::append(pAct);

		StateMachine::getInstance()->transferToStandby();
	}
}

void Controller::productionTestTimeoutFun(void)
{
	productionTestTimeoutTriggered = true;
}

void Controller::startProductionTestTimeout(int sec)
{
	ProductionTestTimeout.attach(Callback<void()>(this, &Controller::productionTestTimeoutFun), sec);
}

void Controller::productionTestProcess()
{
	// productionTestStep: variable for record current step of product test procedure, it will be stored in PS for restore;
	// 0: start filter rinse;
	// 1: open B3 for filter back wash;
	// 2: open B3 for filter fast rinse;
	// 3: wait 30 seconds during filter rinse and membrane rinse;
	// 4: start membrane rinse;
	// 5: wait 60 seconds during membrane rinse and heater start heating;
	// 6: start heating;
	// 7: wait 60 seconds during heating complete and start drain;
	// 8: start drain;
	// 9: wait 10 minutes durting drainage and heating again;
	// 10: start heating again;
	// 11: product test finished;

	if(productionTestTimeoutTriggered) // move to next step if timeout triggered
	{
		LOG_INFO("###Controller::productionTestProcess() productionTestStep: %d\r\n", productionTestStep);
		productionTestTimeoutTriggered = false; // reset flag

		if(productionTestStep == 0)
		{
			// step 1: start filter rinse
			DigitalSwitches::getInstance()->startFilterRinse(true);

			// event to Central Panel
			Action* pAct = new Action(Action::AID_CP_FILTERRINSESTART);
			ActionBase::append(pAct);
		}
		else if(productionTestStep == 1)
		{
			// open feed water valve(B3) for back wash filter
			DigitalSwitches::getInstance()->turnOnFeedWaterValve(true);
		}
		else if(productionTestStep == 2)
		{
			// open feed water valve(B3) for fast rinse filter
			DigitalSwitches::getInstance()->turnOnFeedWaterValve(true);
		}
		else if(productionTestStep == 3)
		{
			// filter rinse finished
			// wait 30 sec to start membrane rinse
			startProductionTestTimeout(30);
		}
		else if(productionTestStep == 4)
		{
			// start membrane rinse
			Action* pAct = new ActionSetRinseDur(PRODUCT_TEST_MEMBRANE_RINSE_DUR);
			ActionBase::append(pAct);
			pAct = new ActionStartRinse(true);
			ActionBase::append(pAct);

			//
			pAct = new Action(Action::AID_CP_MEMBRANERINSESTART);
			ActionBase::append(pAct);
		}
		else if(productionTestStep == 5)
		{
			// membrane rinse finished
			// wait 60 sec to start heat
			startProductionTestTimeout(60);
		}
		else if(productionTestStep == 6)
		{
			// start heat
			ActionStartHeating* pAct = new ActionStartHeating(true);
			ActionBase::append(pAct);
		}
		else if(productionTestStep == 7)
		{
			// heat finished
			// wait 60 sec to start drain
			startProductionTestTimeout(PRODUCT_TEST_WAIT_TIME_BEFORE_DRAIN);
		}
		else if(productionTestStep == 8)
		{
			// start drain
			//
			Action* pAct = new Action(Action::AID_WH_SET_DRAIN_DUR, this);
			ActionBase::append(pAct);

			pAct = new ActionStartDrain(true);
			ActionBase::append(pAct);

			//
			pAct = new Action(Action::AID_CP_DRAINING);
			ActionBase::append(pAct);
		}
		else if(productionTestStep == 9)
		{
			// drain finished
			// stop water heater
			Action* pAct = new ActionStartHeating(false);
			ActionBase::append(pAct);
			// wait 10 min to start heat again
			startProductionTestTimeout(PRODUCT_TEST_WAIT_TIME_AFTER_DRAIN);
		}
		else if(productionTestStep == 10)
		{
			// start heating
			Action* pAct = new ActionStartHeating(true);
			ActionBase::append(pAct);
		}
		else if(productionTestStep == 11)
		{
			// product test finished
			//
			ActionProductTestStatus* pAct = new ActionProductTestStatus(1);
			ActionBase::append(pAct);

			StateMachine::getInstance()->transferToStandby();
		}

		productionTestStep++; // move to next step;
		PersistentStorage::getInstance()->saveProductTestStep();
	}
}

int Controller::setDisinfectFixTime(int day, int hour, int min)
{
	CPDisinfectFixTime.day   = day;
	CPDisinfectFixTime.hour  = hour;
	CPDisinfectFixTime.min   = min;

	// save to PS
	FTRACE("###Controller::setDisinfectFixTime() day: %d  hour: %d  min: %d\r\n", day, hour, min);
	return PersistentStorage::getInstance()->saveDisinfectFixtime();

	//return 0;
}

int Controller::setDisinfectPlan(int y, int m, int d, int h, int min)
{
	FTRACE("###Controller::setDisinfectPlan() %d.%d.%d %d:%d\r\n", y, m, d, h, min);
	CPDisinfectPlan.year = y;
	CPDisinfectPlan.month = m;
	CPDisinfectPlan.day = d;
	CPDisinfectPlan.hour = h;
	CPDisinfectPlan.min = min;
	if(y==0 && m==0 && d==0 && h==0 && min==0)
		CPDisinfectPlan.valid = false;
	else
	{
		struct tm nt;
		memset(&nt, 0, sizeof(struct tm));
		//nt.tm_sec = 0;
		nt.tm_min = min;
		nt.tm_hour = h;
		nt.tm_mday = d;
		nt.tm_mon = m - 1;
		nt.tm_year = y - 1900;
		CPDisinfectPlan.plan_time = mktime(&nt);
		CPDisinfectPlan.valid = true;
	}

	// save to PS
	return PersistentStorage::getInstance()->saveDisinfectPlan();
	//return 0;
}

int Controller::disinfect()
{
	LOG_INFO("###Controller::disinfect()\r\n");

	return StateMachine::getInstance()->transferToDisinfect();
}

int Controller::lockUsercard(int card, int bLocked)
{
	LOG_INFO("###Controller::lockUsercard() card: %d bLocked: %d\r\n", card, bLocked);
	// write lock flag
	if(card == CPUserInfo.icCardId)
	{
		CPUserInfo.cardType |= 0x80;

		// write card
		ActionICCardWrite* pAct = new ActionICCardWrite(this, CPUserInfo.balance);
		ActionBase::append(pAct);

		// deactivate the account
		CPUserInfo.accountState = 0x00;

		//
		StateMachine::getInstance()->transferToStandby();
	}

	return 0;
}

int Controller::functionCancel(int funcType)
{
	LOG_INFO("###Controller::functionCancel() type: %d\r\n", funcType);
	// 0: all if
	// 1: drainage
	// 2: membrane rinse
	// 3: filter rinse
	// 4: disinfect

	if((funcType == 0 || funcType == 1) && StateMachine::getInstance()->inDrainageState())
	{
		// stop drain
		StateMachine::getInstance()->stopDrainageManually();
	}
	else if((funcType == 0 || funcType == 2) && StateMachine::getInstance()->inMembraneRinseState())
	{
		// stop membrane rinse
		StateMachine::getInstance()->stopMembraneRinseManually();
	}
	else if((funcType == 0 || funcType == 3) && StateMachine::getInstance()->inFilterRinseState())
	{
		// stop filter rinse
		StateMachine::getInstance()->stopFilterRinseManually();
	}
	else if((funcType == 0 || funcType == 4) && StateMachine::getInstance()->inDisinfectState())
	{
		// stop disinfect
		StateMachine::getInstance()->stopDisinfectManually();
	}

	return 0;
}

int Controller::getStatus()
{
	FTRACE("###Controller::getStatus() %s\r\n", StateMachine::getInstance()->currentState());

	if(StateMachine::getInstance()->inProductionTestState())
	{
		// notify the product test is started
		ActionProductTestStatus* pAct = new ActionProductTestStatus(0);
		ActionBase::append(pAct);
	}

	Controller::getInstance()->updateOutletValveMark(valveStatus);

	return 0;
}

int Controller::setICCardBalance(int cardId, int balance)
{
	LOG_INFO("###Controller::setICCardBalance() cardId: %d  balance: %d\r\n", cardId, balance);

	if(StateMachine::getInstance()->inSoldState() && CPUserInfo.accountType == 0)
	{
		if(cardId == CPUserInfo.icCardId)
		{
			if(balance < CPUserInfo.balance)
			{
				CPUserInfo.balance = balance;

				// write balance back to IC card
				ActionICCardWrite* pAct = new ActionICCardWrite(this, balance);
				ActionBase::append(pAct);
			}
		}
	}

	return 0;
}

int Controller::getConfigParam(int kind)
{
    FTRACE("###Controller::getConfigParam() kind: %d\r\n", kind);
    if(kind == 0 || kind == 1)
    {
        ActionBase* pAct = new ActionConfigParam(kind, this);
        ActionBase::append(pAct);
    }
    return 0;
}

int Controller::setConfigSecret(char* secret)
{
    int i=0;
    FTRACE("###Controller::setConfigSecret() secret: %s\r\n", secret);
    memset(configParamSecret, 0, 64);
    for(i=0; i<64; i++)
    {
        if(secret && secret[i] != '\0')
        {
            configParamSecret[i] = secret[i];
        }
    }

    // save to PS
    PersistentStorage::getInstance()->saveConfigParamSecret();
    return 0;
}

int Controller::setConfigParam(int bHP, int bQR, int bDH, int bIC, int bSKII)
{
    FTRACE("###Controller::setConfigParam() bHP: %d bQR: %d bDH: %d bIC: %d bSKII: %d\r\n", bHP, bQR, bDH, bIC, bSKII);
    configParamHeatPreservation = bHP;
    configParamQRScanner = bQR;
    configParamDustHelmet = bDH;
    configParamICCardReader = bIC;
    configParamSK_II = bSKII;

    if(!configParamICCardReader)
    {
        WarningManager::getInstance()->resolveWarning(WarningManager::WARNING_TYPE_IC_CARD_READER_ERR);
    }

    // save to PS
    PersistentStorage::getInstance()->saveConfigParam();
    return 0;
}

int Controller::setICCardRegion(int cardId, int region, int cardType)
{
    LOG_INFO("###Controller::setICCardRegion() cardId: %u  regionCode: %d cardType: %d\r\n", cardId, region, cardType);

	if(StateMachine::getInstance()->inSoldState() && CPUserInfo.accountType == 0)
	{
		if(cardId == CPUserInfo.icCardId)
		{
			CPUserInfo.regionCode  = region;
			CPUserInfo.cardType   &= 0x80;
			CPUserInfo.cardType   |= cardType & 0x7F;

			// write region code back to IC card
			ActionICCardWrite* pAct = new ActionICCardWrite(this, CPUserInfo.balance);
			ActionBase::append(pAct);
			return 0;
		}
	}
    return 1;
}

int Controller::response(int cmd, int actId, int result)
{
	//FTRACE("###Controller::response() command: %d action id: %d result: %d\r\n", cmd, actId, result);
	if(result == 1) // failed; resend;
		return 0;

	// Set Central Panel is alive when get reponse
	if(!CPIsAlive)
	{
		CPIsAlive = true;
	}

	if(ActionBase::header() && ActionBase::header()->getActionId() == actId)
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			// special work
			if(ActionBase::header()->getActionId() == Action::AID_CP_CONFIGINFO)
			{
				if(((ActionConfigInfo*)ActionBase::header())->index < ActionConfigInfo::SETID_LAST)
				{
                    if(ActionConfigInfo::SETID_FILTER_RINSE_FIXTIMES-1 == ((ActionConfigInfo*)ActionBase::header())->index)
                    {
                        fix_times_index = IsHasFixTimesValidIndex();
                        membrane_fix_times_index = IsHasMembraneFixTimesValidIndex();
                        disinfect_fix_times_index = IsHasDisinfectFixTimesValidIndex();

                        if(0xFF == fix_times_index)
                        {
                            if(0xFF == membrane_fix_times_index)
                            {
                                if(0xFF == disinfect_fix_times_index)
                                {
                                    ((ActionConfigInfo*)ActionBase::header())->index  += 4; // move to next next setting
                                }
                                else ((ActionConfigInfo*)ActionBase::header())->index += 3; // move to next next setting
                            }
                            else ((ActionConfigInfo*)ActionBase::header())->index     += 2; // move to next next setting
                        }
                        else ((ActionConfigInfo*)ActionBase::header())->index         += 1; // move to next setting
                    }
                    else if((ActionConfigInfo::SETID_FILTER_RINSE_FIXTIMES == ((ActionConfigInfo*)ActionBase::header())->index) &&
                            (IsHasFollowingFixTimesValidIndex() != 0xFF))
                    {
                        fix_times_index = IsHasFollowingFixTimesValidIndex();
                    }
                    else if((ActionConfigInfo::SETID_MEMBRANE_RINSE_FIXTIMES == ((ActionConfigInfo*)ActionBase::header())->index) && 
                            (IsHasFollowingMembraneFixTimesValidIndex() != 0xFF))
                    {
                        membrane_fix_times_index = IsHasFollowingMembraneFixTimesValidIndex();
                    }
                    else if((ActionConfigInfo::SETID_DISINFECT_FIXTIMES == ((ActionConfigInfo*)ActionBase::header())->index) &&
                            (IsHasFollowingDisinfectFixTimesValidIndex() != 0xFF))
                    {
                        disinfect_fix_times_index = IsHasFollowingDisinfectFixTimesValidIndex();
                    }
                    else
                    {
                        if(ActionConfigInfo::SETID_MEMBRANE_RINSE_FIXTIMES-1 == ((ActionConfigInfo*)ActionBase::header())->index)
                        {
                            membrane_fix_times_index = IsHasMembraneFixTimesValidIndex();
                            disinfect_fix_times_index = IsHasDisinfectFixTimesValidIndex();

                            if(0xFF == membrane_fix_times_index)
                            {
                                if(0xFF == disinfect_fix_times_index)
                                {
                                    ((ActionConfigInfo*)ActionBase::header())->index  += 3; // move to next next setting
                                }
                                else ((ActionConfigInfo*)ActionBase::header())->index += 2; // move to next next setting
                            }
                            else ((ActionConfigInfo*)ActionBase::header())->index     += 1; // move to next setting
                        }
                        else if(ActionConfigInfo::SETID_DISINFECT_FIXTIMES-1 == ((ActionConfigInfo*)ActionBase::header())->index)
                        {
                            disinfect_fix_times_index = IsHasDisinfectFixTimesValidIndex();
                            
                            if(0xFF == disinfect_fix_times_index)
                            {
                                ((ActionConfigInfo*)ActionBase::header())->index  += 2; // move to next next setting
                            }
                            else ((ActionConfigInfo*)ActionBase::header())->index += 1; // move to next setting
                        }
                        else ((ActionConfigInfo*)ActionBase::header())->index     += 1; // move to next setting
                    }
                    
					((ActionConfigInfo*)ActionBase::header())->sendCount = 0; // reset sendCount
					((ActionConfigInfo*)ActionBase::header())->status = ActionBase::PENDING;
					return 0; // continue to report other settings
				}
			}
			//else if(ActionBase::header()->getActionId() == Action::AID_CP_COUNTDOWN)
			//{
				//if(CPAppEjectCountdown > 0)
			//		CPAppEjectTimeout.attach(Callback<void()>(this, &Controller::appEjectTimeoutFun), 1.0);
				//else
				//{
				//	// upload app water selling finished
				//	ActionWaterInfo* pAct = new ActionWaterInfo(0, 0, 0, 0, 0);
				//	ActionBase::append(pAct);
				//}
			//}

			ActionBase::pop();
		}
	}

	return 0;
}

int Controller::setFilterRinseFixTimes(int index, int day, int hour, int min)
{    
    if((0x00 == day) && (0x00 == hour) && (0x00 == min))
    {
        CPFilterRinseFixTimes[index].valid = false;
    }
    else
    {
	    CPFilterRinseFixTimes[index].valid = true;
    }

    CPFilterRinseFixTimes[index].index = index;
	CPFilterRinseFixTimes[index].day   = day;
	CPFilterRinseFixTimes[index].hour  = hour;
	CPFilterRinseFixTimes[index].min   = min;
    
	// save to PS
	LOG_INFO("###Controller::setFilterRinseFixTimes() index: %d valid: %d day: %d  hour|min: 0x%08X\r\n", index, CPFilterRinseFixTimes[index].valid, day, hour<<16 | min);
	return PersistentStorage::getInstance()->saveFilterRinseFixtimes(index);

	//return 0;
}

int Controller::setMembraneRinseFixTimes(int index, int day, int hour, int min)
{
    if((0x00 == day) && (0x00 == hour) && (0x00 == min))
    {
        CPMembraneRinseFixTimes[index].valid = false;
    }
    else
    {
	    CPMembraneRinseFixTimes[index].valid = true;
    }
	
    CPMembraneRinseFixTimes[index].index = index;
	CPMembraneRinseFixTimes[index].day   = day;
	CPMembraneRinseFixTimes[index].hour  = hour;
	CPMembraneRinseFixTimes[index].min   = min;
    
	// save to PS
    LOG_INFO("###Controller::setMembraneRinseFixTimes() index: %d valid: %d day: %d  hour|min: 0x%08X\r\n", index, CPMembraneRinseFixTimes[index].valid, day, hour<<16 | min);
	return PersistentStorage::getInstance()->saveMembraneRinseFixtimes(index);

	//return 0;
}

int Controller::setDisinfectFixTimes(int index, int day, int hour, int min)
{
    if((0x00 == day) && (0x00 == hour) && (0x00 == min))
    {
        CPDisinfectFixTimes[index].valid = false;
    }
    else
    {
	    CPDisinfectFixTimes[index].valid = true;
    }

    CPDisinfectFixTimes[index].index = index;
	CPDisinfectFixTimes[index].day   = day;
	CPDisinfectFixTimes[index].hour  = hour;
	CPDisinfectFixTimes[index].min   = min;
    
	// save to PS
	FTRACE("###Controller::setDisinfectFixTimes() index: %d valid: %d day: %d  hour: %d  min: %d\r\n", index, CPDisinfectFixTimes[index].valid, day, hour, min);
	return PersistentStorage::getInstance()->saveDisinfectFixtimes(index);

	//return 0;
}

int Controller::getWarnings(void)
{
	Action* pAct = new Action(Action::AID_CP_WARNINGS);
	ActionBase::append(pAct);

	return 0;
}

int Controller::getValue_1041(char* order_1041)
{
	int i,j,val;
	int value_1041[64];
	
	val = 0;
	for(i = 0;i < 7;i++)
	{
		value_1041[val++] = i + 1;
		for(j = 0;j < 4;j++)
		{
			if(j != 0 && CPServiceTime[i].start[j] == 0 && CPServiceTime[i].end[j] == 0)
			{
				break;
			}
			value_1041[val++] = CPServiceTime[i].start[j];
			value_1041[val++] = CPServiceTime[i].end[j];
		}
	}
	int valueLen_1041 = sizeof(value_1041) / sizeof(value_1041[0]);
	j = 0;
	int len = 0;
	for(i = 0;i < valueLen_1041;i++)
	{
		len = sprintf(order_1041 + j,"%d#",value_1041[i]);
		j += len;
	}
	order_1041[j-1] = '\0';

	return j;
}

int Controller::getValue_1044(char* order_1044)
{
	int i,j,val;
	int value_1044[64];
	
	val = 0;
	for(i = 0;i < 7;i++)
	{
		value_1044[val++] = i + 1;
		for(j = 0;j < 4;j++)
		{
			if(j != 0 && CPHeaterWorkTime[i].start[j] == 0 && CPHeaterWorkTime[i].end[j] == 0)
			{
				break;
			}
			value_1044[val++] = CPHeaterWorkTime[i].start[j];
			value_1044[val++] = CPHeaterWorkTime[i].end[j];
		}
	}
	int valueLen_1044 = sizeof(value_1044) / sizeof(value_1044[0]);
	j = 0;
	int len = 0;
	for(i = 0;i < valueLen_1044;i++)
	{
		len = sprintf(order_1044 + j,"%d#",value_1044[i]);
		j += len;
	}
	order_1044[j-1] = '\0';

	return j;
}

int Controller::getValue_1062(char* order_1062)
{
	int i,j,val;
	int value_1062[100];
	
	val = 0;
	for(i = 0;i < DEVICE_RINSE_MAX_NUM;i++)
	{
		if(i != 0 && CPFilterRinseFixTimes[i].valid == false) continue;

		value_1062[val++] = CPFilterRinseFixTimes[i].index;
		value_1062[val++] = CPFilterRinseFixTimes[i].day;
		value_1062[val++] = CPFilterRinseFixTimes[i].hour;
		value_1062[val++] = CPFilterRinseFixTimes[i].min;
	}
	int valueLen_1062 = sizeof(value_1062) / sizeof(value_1062[0]);
	j = 0;
	int len = 0;
	for(i = 0;i < valueLen_1062;i++)
	{
		len = sprintf(order_1062 + j,"%d#",value_1062[i]);
		j += len;
	}
	order_1062[j-1] = '\0';

	return j;
}

int Controller::getValue_1063(char* order_1063)
{
	int i,j,val;
	int value_1063[100];
	
	val = 0;
	for(i = 0;i < DEVICE_RINSE_MAX_NUM;i++)
	{
		if(i != 0 && CPMembraneRinseFixTimes[i].valid == false) continue;

		value_1063[val++] = CPMembraneRinseFixTimes[i].index;
		value_1063[val++] = CPMembraneRinseFixTimes[i].day;
		value_1063[val++] = CPMembraneRinseFixTimes[i].hour;
		value_1063[val++] = CPMembraneRinseFixTimes[i].min;
	}
	int valueLen_1063 = sizeof(value_1063) / sizeof(value_1063[0]);
	j = 0;
	int len = 0;
	for(i = 0;i < valueLen_1063;i++)
	{
		len = sprintf(order_1063 + j,"%d#",value_1063[i]);
		j += len;
	}
	order_1063[j-1] = '\0';

	return j;
}

int Controller::getValue_1064(char* order_1064)
{
	int i,j,val;
	int value_1064[100];
	
	val = 0;
	for(i = 0;i < DEVICE_RINSE_MAX_NUM;i++)
	{
		if(i != 0 && CPDisinfectFixTimes[i].valid == false) continue;

		value_1064[val++] = CPDisinfectFixTimes[i].index;
		value_1064[val++] = CPDisinfectFixTimes[i].day;
		value_1064[val++] = CPDisinfectFixTimes[i].hour;
		value_1064[val++] = CPDisinfectFixTimes[i].min;
	}
	int valueLen_1064 = sizeof(value_1064) / sizeof(value_1064[0]);
	j = 0;
	int len = 0;
	for(i = 0;i < valueLen_1064;i++)
	{
		len = sprintf(order_1064 + j,"%d#",value_1064[i]);
		j += len;
	}
	order_1064[j-1] = '\0';

	return j;
}


int Controller::getMD5Value()
{
	char buffer[2048];

	char order_1041[256];
	getValue_1041(order_1041);

	char order_1044[256];
	getValue_1044(order_1044);

	char order_1062[320];
	getValue_1062(order_1062);

	char order_1063[320];
	getValue_1063(order_1063);

	char order_1064[320];
	getValue_1064(order_1064);
	
	/*			1030 &  1033     &  1034 & 1035 &1036 & 1037      & 1038 & 1039	& 1040 & 1041 &	1042 & 1043	 & 1044 & 1045 & 1050     & 1051           & 1052     & 1053           & 1054     & 1055           & 1056     & 1057           & 1058 & 1059 & 1061              & 1062 & 1063 & 1064*/
	sprintf(buffer,"%d&%d#%d#%d&%d&%d&%d&%d#%d#%d&%d#%d&%d#%d&%d&%s&%d&%d#%d&%s&%d&%d#%d#%d&%d#%d#%d#%d#%d&%d#%d#%d&%d#%d#%d#%d#%d&%d#%d#%d&%d#%d#%d#%d#%d&%d#%d#%d&%d#%d#%d#%d#%d&%d&%d&%d#%d#%d#%d#%d#%d&%s&%s&%s",
				WHBoilingTemp,	//1030
				CPHotPrice,CPColdPrice,CPIcePrice,	//1033
				CPFilterWaterMax,	//1034
				CPPlanDelayTime,	//1035
				CPFilterWaterCur,	//1036
				CPFilterProductionDate.year,CPFilterProductionDate.month,CPFilterProductionDate.day,	//1037
				CPFilterBackRinseDur,CPFilterFastRinseDur,	//1038
				CPChargeMode,CPChargeUnit,	//1039
				CPMembraneRinseDur,	//1040
				order_1041,		//1041
				CPFilterShelfLife,	//1042
				CPCompensation.compensation,CPCompensation.startTemp,	//1043
				order_1044,		//1044
				CPDisinfectDur,		//1045
				CPFilterRinseFixTime.day,CPFilterRinseFixTime.hour,CPFilterRinseFixTime.min,	//1050
				CPFilterRinsePlan.year,CPFilterRinsePlan.month,CPFilterRinsePlan.day,CPFilterRinsePlan.hour,CPFilterRinsePlan.min,	//1051
				CPDrainFixTime.day,CPDrainFixTime.hour,CPDrainFixTime.min,	//1052
				CPDrainPlan.year,CPDrainPlan.month,CPDrainPlan.day,CPDrainPlan.hour,CPDrainPlan.min,	//1053
				CPMembraneRinseFixTime.day,CPMembraneRinseFixTime.hour,CPMembraneRinseFixTime.min,	//1054
				CPMembraneRinsePlan.year,CPMembraneRinsePlan.month,CPMembraneRinsePlan.day,CPMembraneRinsePlan.hour,CPMembraneRinsePlan.min,	//1055
				CPDisinfectFixTime.day,CPDisinfectFixTime.hour,CPDisinfectFixTime.min,	//1056
				CPDisinfectPlan.year,CPDisinfectPlan.month,CPDisinfectPlan.day,CPDisinfectPlan.hour,CPDisinfectPlan.min,	//1057
				CPFilterCheckMode,		//1058
				CPDisinfectDelayTime,	//1059
				CPPulsePerLitreArr[CP_PULSE_PER_LITRE],CPPulsePerLitreArr[CP_HOT_PULSE_PER_LITRE],CPPulsePerLitreArr[CP_COLD_PULSE_PER_LITRE],CPPulsePerLitreArr[CP_ICE_PULSE_PER_LITRE],CPPulsePerLitreArr[CP_PULSE_PER_LITRE_5],CPPulsePerLitreArr[CP_PULSE_PER_LITRE_6],		//1061	
				order_1062,		//1062
				order_1063,		//1063
				order_1064		//1064
				);

	FTRACE("###Controller::getMD5Value [ %s ]\n",buffer);

	char md5Value[33];

	strcpy(md5Value,MD5(buffer).toStr().c_str());
	FTRACE("###Controller::getMD5Value md5 [ %s ]\n",md5Value);

	CentralPanelSerial::getInstance()->uploadMD5Value(md5Value);

	return 0;
}

//APIs for water heater
int Controller::handShakeResponse(char v1, char v2, char v3)
{
	FTRACE("###Controller::handShakeResponse() water heater version: %d.%d.%d\r\n", v1, v2, v3);
	WHVer[0] = v1;
	WHVer[1] = v2;
	WHVer[2] = v3;

	//if(ActionBase::header() &&
	//	((ActionBase::header()->getActionId() == Action::AID_WH_HANDSHAKE) ||
	//		(ActionBase::header()->getActionId() == Action::AID_WH_SOFTRESET)))
	//{
	//	if(ActionBase::header()->status == ActionBase::EXECUTED)
	//	{
	//		ActionBase::pop();
	//	}
	//}
	ActionBase::removeActionIfHas(Action::AID_WH_HANDSHAKE);
	ActionBase::removeActionIfHas(Action::AID_WH_SOFTRESET);

	// Now, water heater is connected
	waterHeaterConnected();

	return 0;
}

int Controller::startHeatingResponse(bool bOn)
{
	FTRACE("###Controller::startHeatingResponse() bOn=%d\r\n", bOn);
	WHInService = bOn;

	if(ActionBase::header() && (ActionBase::header()->getActionId() == Action::AID_WH_STARTHEATING))
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}

	return 0;
}

int Controller::setBoilTempResponse(char temp, char offset)
{
	FTRACE("###Controller::setBoilTempResponse() temp=%d offset=%d\r\n", temp, offset);
	//WHBoilingTemp = temp;
	//WHTempOffset = offset;

	if(ActionBase::header() && (ActionBase::header()->getActionId() == Action::AID_WH_BOILINGTEMP))
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}
	return 0;
}

int Controller::startRinseResponse(bool bOn)
{
	FTRACE("###Controller::startRinseResponse() bOn=%d\r\n", bOn);

	if(ActionBase::header() && (ActionBase::header()->getActionId() == Action::AID_WH_RINSE))
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}
	return 0;
}

int Controller::startDrainResponse(bool bOn)
{
	FTRACE("###Controller::startDrainResponse() bOn=%d\r\n", bOn);

	if(ActionBase::header() && (ActionBase::header()->getActionId() == Action::AID_WH_DRAIN))
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}
	return 0;
}

int Controller::setWarnTempResponse(char temp)
{
	FTRACE("###Controller::setWarnTempResponse() temp=%d\r\n", temp);
	WHWarningTempOffset = temp;

	if(ActionBase::header() && (ActionBase::header()->getActionId() == Action::AID_WH_WARNINGTEMP))
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}
	return 0;
}

int Controller::startFeedbackPeriodicallyResponse(bool bOn, char sec)
{
	FTRACE("###Controller::startFeedbackPeriodicallyResponse() bOn: %d sec: %d\r\n", bOn, sec);
	WHFBOnOff = bOn;
	WHFBDur = sec;

	if(ActionBase::header() && (ActionBase::header()->getActionId() == Action::AID_WH_FEEDBACK))
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}
	return 0;
}

int Controller::getStatusResponse(char status, char temp, char level, bool bHeat, bool bWater)
{
	FTRACE("###Controller::getStatusResponse() status: %d temp: %d level: %d bHeat: %d bWater: %d\r\n",
				status, temp, level, bHeat, bWater);
	// status: 0: out of service; 1: in service; 2: rinse; 3: warning; 4: drain;
	WHStatus = status;
	WHCurTemp = temp;
	WHLevel = level;
	WHHeater = bHeat;
	WHWater = bWater;

	if(ActionBase::header() && (ActionBase::header()->getActionId() == Action::AID_WH_GETSTATUS))
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}

	// upload tank info
	int N = temp;
#if ENABLE_TEMPERATURE_COMPENSATION
	//if(!StateMachine::getInstance()->inDisinfectState())
	//{
	//	int n = WHBoilingTemp - CPCompensation.compensation;
	//	int a = CPCompensation.compensation;
	//	int b = CPCompensation.startTemp;
	//	int d = n - b;
	//	if(N > d)
	//	{
	//		N = d + (a + b) * (N - d) / b;
	//	}
	//}
#endif
	Action* pAct = new ActionWaterTankStatus(N, CPFilterWaterCur, CPFilterWaterMax);
	ActionBase::append(pAct);

	if(bHeat)
	{
		// heating
		ActionWaterStatus* pAct = new ActionWaterStatus(1);
		ActionBase::append(pAct);
	}
	else
	{
		ActionWaterStatus* pAct = new ActionWaterStatus(0);
		ActionBase::append(pAct);
	}

	// sync status; make sure has same status as heater
	if(status == 0 && WHInService)
		WHInService = false;
	if(status == 1 && !WHInService)
		WHInService = true;

	return 0;
}

int Controller::completeNotify(char type)
{
	// 0x01: heating finished
	// 0x02: membrane rinse finished
	// 0x04: drain finished
	if(type == 1)
	{
        FTRACE("###Controller::completeNotify() Heat Finished\r\n");
		if(StateMachine::getInstance()->inDisinfectState())
		{
			// start disinfect
			if(!DigitalSwitches::getInstance()->infraredDetectedWorks())
				startDisinfect();
		}
		else if(StateMachine::getInstance()->inProductionTestState())
		{
			//productionTestStep = 7;
			//productionTestStep = 11;
			startProductionTestTimeout(1);
		}
		else
		{
			// heat finished, ready to drink
			//ActionWaterStatus* pAct = new ActionWaterStatus(0);
			//ActionBase::append(pAct);
		}
	}
	else if(type == 0x02)
	{
        LOG_INFO("###Controller::completeNotify() Membrane Rinse Finished\r\n");

		if(!StateMachine::getInstance()->inDisinfectState())
		{
			Action* pAct = new Action(Action::AID_CP_MEMBRANERINSEEND);
			ActionBase::append(pAct);
		}
		else
		{
			// TODO: restart membrane rinse during disinfect?
		}

		if(StateMachine::getInstance()->inProductionTestState())
		{
			// start heat
			//productionTestStep = 5;
			startProductionTestTimeout(1);
		}
		else if(StateMachine::getInstance()->inMembraneRinseState())
		{
			//WHInService = false; // water heater is out of service after membrane rinse
			StateMachine::getInstance()->NotifyMembraneRinseComplete();
		}
	}
	else if(type == 0x04)
	{
        LOG_INFO("###Controller::completeNotify() Drain Finished\r\n");
		Action* pAct = new Action(Action::AID_CP_DRAINEND);
		ActionBase::append(pAct);

		if(StateMachine::getInstance()->inProductionTestState())
		{
			// start heat
			//productionTestStep = 9;
			startProductionTestTimeout(1);
		}
		else
		{
			//WHInService = false; // water heater is out of service after drainage
			StateMachine::getInstance()->transferToStandby();
		}
	}

	return 0;
}

int Controller::warningNotify(char index)
{
	FTRACE("###Controller::warningNotify() index: %d\r\n", index);
	// 0x01: electric leak
	// 0x02: high temperature alarm
	// 0x03: over flow alarm
	// 0x04: no temperature reducation with cold water, high temperature alarm
	// 0x05: no low water trigger
	// 0x06: no high water trigger, but over flow trigger
	// 0x07: no low water and high water trigger
    // 0x08: over high temperature alarm

	WarningManager::EWarningType WHWarningType;
	if(index == 0x01)
		WHWarningType = WarningManager::WARNING_TYPE_WH_ELECTRIC_LEAK;
	else if(index == 0x02)
		WHWarningType = WarningManager::WARNING_TYPE_WH_HIGH_TEMPERATURE;
	else if(index == 0x03)
		WHWarningType = WarningManager::WARNING_TYPE_WH_OVER_FLOW;
	else if(index == 0x04)
	{
		return 0;
		//WHWarningType = WarningManager::WARNING_TYPE_WH_HIGH_TEMPERATURE;
	}
	else if(index == 0x05)
		WHWarningType = WarningManager::WARNING_TYPE_WH_LOW_LEVEL_ERR;
	else if(index == 0x06)
		WHWarningType = WarningManager::WARNING_TYPE_WH_HIGH_LEVEL_ERR;
	else if(index == 0x07)
		WHWarningType = WarningManager::WARNING_TYPE_WH_LEVEL_ERR;
	else if(index == 0x08)
		WHWarningType = WarningManager::WARNING_TYPE_WH_OVER_TEMPERATURE;
	else
		return 0;

	WarningManager::getInstance()->triggerWarning(WHWarningType);

	return 0;
}

int Controller::setRinseDurResponse(char times)
{
	FTRACE("###Controller::setRinseDurResponse() times: %d\r\n", times);
	if(ActionBase::header() && (ActionBase::header()->getActionId() == Action::AID_WH_SET_RINSE_DUR))
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}

	return 0;
}

int Controller::setDrainDurResponse(char minS, char minE)
{
	FTRACE("###Controller::setDrainDurResponse() minS: %d minE: %d\r\n", minS, minE);
	if(ActionBase::header() && (ActionBase::header()->getActionId() == Action::AID_WH_SET_DRAIN_DUR))
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}

	return 0;
}

int Controller::setTempRestrictResponse(char temp)
{
	FTRACE("###Controller::setTempRestrictResponse() temp: %d\r\n", temp);
	if(ActionBase::header() && (ActionBase::header()->getActionId() == Action::AID_WH_SET_TEMP_RESTRICT))
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}

	return 0;
}

// APIs for IC card reader
int Controller::CRHandShakeResponse(char v1, char v2, char v3)
{
	FTRACE("###Controller::CRHandShakeResponse() IC Card Reader version: %d.%d.%d\r\n", v1, v2, v3);
	CRVer[0] = v1;
	CRVer[1] = v2;
	CRVer[2] = v3;

	//if(ActionBase::header() && ActionBase::header()->getActionId() == Action::AID_CR_HANDSHAKE)
	//{
	//	if(ActionBase::header()->status == ActionBase::EXECUTED)
	//	{
	//		ActionBase::pop();
	//	}
	//}
	ActionBase::removeActionIfHas(Action::AID_CR_HANDSHAKE);

	// enable reader
	Action* pAct = new Action(Action::AID_CR_ENABLEREADER);
	ActionBase::append(pAct);

	// resolve IC card reader warning
	WarningManager::getInstance()->resolveWarning(WarningManager::WARNING_TYPE_IC_CARD_READER_ERR);
	CRCheckTime = 0;
	CRIsAlive = true;

	return 0;
}

int Controller::CREnableReaderResponse(bool bOn)
{
	FTRACE("###Controller::CREnableReaderResponse() IC Card Reader enabled: %d\r\n", bOn);
	CREnabled = bOn;

	if(ActionBase::header() && ActionBase::header()->getActionId() == Action::AID_CR_ENABLEREADER)
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}

	CRCheckTime = 0;
	CRIsAlive = true;
	return 0;
}

void Controller::ICCardInsertedTimeoutFun()
{
	bICCardInserted = true;
}

int Controller::ICCardInsertedProc()
{
	FTRACE("###Controller::ICCardInsertedProc()\r\n");
	Action* pAct = NULL;

	// set key first
	// set sector key
	pAct = new Action(Action::AID_CR_SETSECTORKEY, this);
	ActionBase::append(pAct);

	// try to read balance
	pAct = new Action(Action::AID_CR_READCARD, this);
	ActionBase::append(pAct);
	return 0;
}

int Controller::CRICCardDetected(int cardId, char status)
{
	LOG_INFO("###Controller::CRICCardDetected() IC Card ID: %u  status: %d\r\n", cardId, status);
	//check state if allowed read IC card
	if(!StateMachine::getInstance()->inStandbyState() && (status == 1 || status == 2)) // insert card not in standby state
		return 0;

	CPUserInfo.icCardId = cardId;
	CRCardStatus = status;
	if(status == 1) // insert normal card
	{
        gICCardInsertSerialNumber ++;

		CPUserInfo.accountType = 0; // IC card
		CPUserInfo.accountState = 0x00; // no activated card
		//CPUserInfo.balance = 0;

		ICCardInsertedTimeout.attach(Callback<void()>(this, &Controller::ICCardInsertedTimeoutFun), 0.2); // 200ms
	}
	else if(status == 2) // insert illegal card
	{
		CPUserInfo.accountState = 0x00; // no activated card
		// report the illegal card to Central Panel
		ActionCardInfo* pAct = new ActionCardInfo(CPUserInfo.icCardId, 0, 0, 3, CPUserInfo.cardType);
		ActionBase::append(pAct);
	}
	else // if(status == 3) //  card removed
	{
        CPUserInfo.card_moving_flag = 0x00;
		ICCardInsertedTimeout.detach();

		// stop water selling and report card removed
		if(CPUserInfo.accountState && CPUserInfo.accountType==0)
		{
			DigitalSwitches::getInstance()->hotWaterSwitchClose();
			DigitalSwitches::getInstance()->coldWaterSwitchClose();

			//TODO: update later after all flow pulse finished?
			updateAccountInfo(true);

			// deactivate later
			CPUserInfo.accountState = 0x00; // inactivated
		}

		//
		ActionBase::removeActionIfHas(Action::AID_CR_SETSECTORKEY);
		ActionBase::removeActionIfHas(Action::AID_CR_READCARD);

		// always send finish selling water
		//ActionWaterInfo* pAct = new ActionWaterInfo(0, 0, 0, 0, 0, 0);
		//ActionBase::append(pAct);

		// only IC Card inserted and in sold state
		if(StateMachine::getInstance()->inSoldState() && CPUserInfo.accountType==0)
		{
			StateMachine::getInstance()->transferToStandby();
		}
	}

	CRCheckTime = 0;
	CRIsAlive = true;
	return 0;
}

int Controller::CRSetSectorKeyResponse(char secId, char type, char key[6])
{
	FTRACE("###Controller::CRSetSectorKeyResponse() secId: %d type: %d key: 0x%02X%02X%02X%02X%02X%02X\r\n",
			secId, type, key[0], key[1], key[2], key[3], key[4], key[5]);

	if(ActionBase::header() && ActionBase::header()->getActionId() == Action::AID_CR_SETSECTORKEY)
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}

	CRCheckTime = 0;
	CRIsAlive = true;
	return 0;
}

int Controller::CRWriteBlockResponse(char status)
{
	FTRACE("###Controller::CRWriteBlockResponse() status: %d\r\n", status);

	if(ActionBase::header() && ActionBase::header()->getActionId() == Action::AID_CR_WRITECARD)
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}

	CRCheckTime = 0;
	CRIsAlive = true;
	return 0;
}

int Controller::test()
{
    FTRACE("###Controller::test()\r\n");
    NVIC_SystemReset();

    return 0;
}

int Controller::CRReadBlockResponse(char status, char data[16])
{
	FTRACE("###Controller::CRReadBlockResponse() status: %d\r\n", status);

	if(ActionBase::header() && ActionBase::header()->getActionId() == Action::AID_CR_READCARD)
	{
		if(ActionBase::header()->status == ActionBase::EXECUTED)
		{
			ActionBase::pop();
		}
	}
	CRCheckTime = 0;
	CRIsAlive = true;

    if(status != 0x00 && status != 0x01) // read failed
    {
        return -1;
    }

	if(CRCardStatus != 1 || !StateMachine::getInstance()->inStandbyState()) // if card is removed or other account activated
		return 0;

	ICCARD_DATA* icdata = iccard_decode(data, (char*)&CPUserInfo.icCardId);
	if(icdata)
	{
		FTRACE("###Controller::CRReadBlockResponse() version: %d type: 0x%X\r\n", icdata->version, icdata->type);
		LOG_INFO("###Controller::CRReadBlockResponse() region: 0x%08X value: %d\r\n", icdata->region, icdata->value);
		if(icdata->version != 0x01)
		{
			// wrong icdata
			LOG_ERROR("###Controller::CRReadBlockResponse() Version error: %d\r\n", icdata->version);
			ActionCardInfo* pAct = new ActionCardInfo(CPUserInfo.icCardId, 0, 0, 3, CPUserInfo.cardType);
			ActionBase::append(pAct);
			return 0;
		}

		CPUserInfo.balance = icdata->value;
		CPUserInfo.regionCode = icdata->region;
		CPUserInfo.cardType = icdata->type;
		CPUserInfo.vyear = icdata->vyear;
		CPUserInfo.vday = icdata->vday;
		if((CPUserInfo.cardType & 0x7F) == 0x03) // VIP card
		{
			// check expire date
			int year = (CPUserInfo.vyear & 0x7F) + 2016;
			int month = (CPUserInfo.vday >> 4) & 0x0F;
			int day = ((CPUserInfo.vday & 0x0F) << 1) | ((CPUserInfo.vyear >> 7) & 0x01);
			if(month == 0) month = 1;
			if(month > 12) month = 12;
			if(day == 0) day = 1;
			FTRACE("###Controller::CRReadBlockResponse() VIP card expire date: %d/%d/%d\r\n", year, month, day);
			struct tm nt;
			memset(&nt, 0, sizeof(struct tm));
			nt.tm_sec = 59;
			nt.tm_min = 59;
			nt.tm_hour = 23;
			nt.tm_mday = day;
			nt.tm_mon = month - 1;
			nt.tm_year = year - 1900;
            unsigned int exp = mktime(&nt);
            unsigned int cur = time(NULL);
			if(exp < cur)
			{
				LOG_ERROR("###Controller::CRReadBlockResponse() VIP expired. vyear: %X vday: %X\r\n", CPUserInfo.vyear, CPUserInfo.vday);
				ActionCardInfo* pAct = new ActionCardInfo(CPUserInfo.icCardId, CPUserInfo.balance, icdata->region, 2, CPUserInfo.cardType);
				ActionBase::append(pAct);
				return 0;
			}
		}
        else if((CPUserInfo.cardType & 0x7F) == 0x04) // Customerized card
        {
            //
            LOG_INFO("###Controller::CRReadBlockResponse() Customerized Card detected\r\n");
            if(isRestrictHotWaterDevice())
            {
                CPUserInfo.accountState = 0x01; // activated
                StateMachine::getInstance()->transferToSoldState();
            }
            else
            {
                ActionCardInfo* pAct = new ActionCardInfo(CPUserInfo.icCardId, 0, 0, 3, CPUserInfo.cardType);
		        ActionBase::append(pAct);
            }

            return 0;
        }

		int bLocked = (CPUserInfo.cardType >> 7) & 0x01;
		CPUserInfo.hotWaterPulse = 0;
		CPUserInfo.coldWaterPulse = 0;

		//StateMachine::getInstance()->transferToSoldState();

		// update IC Card Infor to Central Panel
		ActionCardInfo* pAct = new ActionCardInfo(CPUserInfo.icCardId, CPUserInfo.balance, icdata->region, bLocked, CPUserInfo.cardType);
		ActionBase::append(pAct);
        
        if(!bLocked) CPUserInfo.card_moving_flag = 0x01;
	}
	else
	{
		// invalid
		LOG_ERROR("###Controller::CRReadBlockResponse() check failed\r\n");
		ActionCardInfo* pAct = new ActionCardInfo(CPUserInfo.icCardId, 0, 0, 3, CPUserInfo.cardType);
		ActionBase::append(pAct);
	}

	return 0;
}

// APIs for QR code scanner
int Controller::QRCodeScanned(const char* code, int length)
{
	if(!StateMachine::getInstance()->inStandbyState())
		return 0;

    if(configParamQRScanner)
    {
	    ActionQRCodeScanned* pAct = new ActionQRCodeScanned(code, length);
	    ActionBase::append(pAct);
    }
	return 0;
}

int Controller::setDeviceType(unsigned int type, bool bOnlyType)
{
    LOG_INFO("###Controller::setDeviceType() type: %d bOnlyType: %d\r\n", type, bOnlyType);
    if(bOnlyType)
    {
        type &= 0xFF;

        vodasDeviceType &= 0xFFFFFF00;
        vodasDeviceType |= type;
    }
    else
    {
        vodasDeviceType = type;
    }
    PersistentStorage::getInstance()->saveDeviceType();
    return 0;
}

int Controller::setDeviceRestrictHotWater(int bRestrict)
{
    LOG_INFO("###Controller::setDeviceRestrictHotWater() bRestrict: %d\r\n", bRestrict);
    if(bRestrict)
    {
        vodasDeviceType |= DEVICE_TYPE_RESTRICT_HOTWATER;
    }
    else
    {
        vodasDeviceType &= ~DEVICE_TYPE_RESTRICT_HOTWATER;
    }
    PersistentStorage::getInstance()->saveDeviceType();
    return 0;
}

int Controller::setKidSafety(int bKidSafety)
{
    LOG_INFO("###Controller::setKidSafety() bKidSafety: %d\r\n", bKidSafety);
    
    if(bKidSafety)
    {
        vodasDeviceType |= DEVICE_TYPE_HOLD_TO_UNLOCK_HOTWATER;
    }
    else
    {
        vodasDeviceType &= ~DEVICE_TYPE_HOLD_TO_UNLOCK_HOTWATER;
    }
    PersistentStorage::getInstance()->saveDeviceType();
    return 0;
}

int Controller::GetCurRinseFixTimesindex()
{
    return CurRinseFixTimesindex;
}

void Controller::SetCurRinseFixTimesindex(int index)
{
    CurRinseFixTimesindex = index;
}

int  Controller::GetCurMembraneFixTimesindex()
{
    return CurMembraneFixTimesindex;
}

void Controller::SetCurMembraneFixTimesindex(int index)
{
    CurMembraneFixTimesindex = index;
}

int  Controller::GetCurDisinfectFixTimesindex()
{
    return CurDisinfectFixTimesindex;
}

void Controller::SetCurDisinfectFixTimesindex(int index)
{
    CurDisinfectFixTimesindex = index;
}

int  Controller::IsHasFixTimesValidIndex(void)//the function is only for ActionConfigInfo()
{
    uint8_t i;
    
    for(i=fix_times_index; i<DEVICE_RINSE_MAX_NUM; i++)
    {
        if(CPFilterRinseFixTimes[i].valid)
        return i;
    }
    
    return 0xFF;
}

int  Controller::IsHasFollowingFixTimesValidIndex(void)//Invalid default current index
{
    uint8_t i;
    
    for(i=fix_times_index+1; i<DEVICE_RINSE_MAX_NUM; i++)
    {
        if(CPFilterRinseFixTimes[i].valid)
        return i;
    }
    
    return 0xFF;
}

int  Controller::GetFixTimesIndex(void)
{
    return fix_times_index;
}

void Controller::SetFixTimesIndex(int index)
{
    fix_times_index = index;
}

int  Controller::IsHasMembraneFixTimesValidIndex(void)//the function is only for ActionConfigInfo()
{
    uint8_t i;
    
    for(i=membrane_fix_times_index; i<DEVICE_RINSE_MAX_NUM; i++)
    {
        if(CPMembraneRinseFixTimes[i].valid)
        return i;
    }
    
    return 0xFF;
}

int  Controller::IsHasFollowingMembraneFixTimesValidIndex(void)//Invalid default current index
{
    uint8_t i;
    
    for(i=membrane_fix_times_index+1; i<DEVICE_RINSE_MAX_NUM; i++)
    {
        if(CPMembraneRinseFixTimes[i].valid)
        return i;
    }
    
    return 0xFF;
}

int  Controller::GetMembraneFixTimesIndex(void)
{
    return membrane_fix_times_index;
}

void Controller::SetMembraneFixTimesIndex(int index)
{
    membrane_fix_times_index = index;
}

int  Controller::IsHasDisinfectFixTimesValidIndex(void)//the function is only for ActionConfigInfo()
{
    uint8_t i;
    
    for(i=disinfect_fix_times_index; i<DEVICE_RINSE_MAX_NUM; i++)
    {
        if(CPDisinfectFixTimes[i].valid)
        return i;
    }
    
    return 0xFF;
}

int  Controller::IsHasFollowingDisinfectFixTimesValidIndex(void)//Invalid default current index
{
    uint8_t i;
    
    for(i=disinfect_fix_times_index+1; i<DEVICE_RINSE_MAX_NUM; i++)
    {
        if(CPDisinfectFixTimes[i].valid)
        return i;
    }
    
    return 0xFF;
}

int  Controller::GetDisinfectFixTimesIndex(void)
{
    return disinfect_fix_times_index;
}

void Controller::SetDisinfectFixTimesIndex(int index)
{
    disinfect_fix_times_index = index;
}

int Controller::GetCPUserInfoIcCardId(void)
{
    return CPUserInfo.icCardId;
}

int Controller::GetCPUserInfoCardMovingFlag(void)
{
    return CPUserInfo.card_moving_flag;
}

// private
bool Controller::checkInService()
{
	time_t seconds = time(NULL);
	struct tm *pt = localtime(&seconds);

	int i = 0;
	int sh, sm, eh, em;
	int wday = pt->tm_wday; // 0: Sunday; 1: Monday; 2: Tuesday; ...
	if(wday == 0)
		wday = 6;
	else
		wday -= 1;
	for(i=0; i<4; i++)
	{
		sh = CPServiceTime[wday].start[i] / 100;
		sm = CPServiceTime[wday].start[i] % 100;
		eh = CPServiceTime[wday].end[i] / 100;
		em = CPServiceTime[wday].end[i] % 100;

		if((pt->tm_hour>sh || (pt->tm_hour==sh && pt->tm_min>=sm)) &&
			(pt->tm_hour<eh || (pt->tm_hour==eh && pt->tm_min<=em)))
		{
			return true;
		}
	}

	return false;
}

bool Controller::checkWHWorkTime()
{
	time_t seconds = time(NULL);
	struct tm *pt = localtime(&seconds);

	int i = 0;
	int sh, sm, eh, em;
	int wday = pt->tm_wday; // 0: Sunday; 1: Monday; 2: Tuesday; ...
	if(wday == 0)
		wday = 6;
	else
		wday -= 1;
	for(i=0; i<4; i++)
	{
		sh = CPHeaterWorkTime[wday].start[i] / 100;
		sm = CPHeaterWorkTime[wday].start[i] % 100;
		eh = CPHeaterWorkTime[wday].end[i] / 100;
		em = CPHeaterWorkTime[wday].end[i] % 100;
		if(CPHeaterWorkTime[wday].start[i] && eh==0 && em==0){
			eh=24;
		}

		if((pt->tm_hour>sh || (pt->tm_hour==sh && pt->tm_min>=sm)) &&
			(pt->tm_hour<eh || (pt->tm_hour==eh && pt->tm_min<=em)))
		{
			return true;
		}
	}

	return false;
}

void Controller::appEjectTimeoutFun()
{
	//FTRACE("###Controller::appEjectTimeoutFun()\r\n");

	appEjectTimeoutTriggered = true;
}

void Controller::rebootTimeoutFun()
{
	if(rebootDeviceType == 1)
	{
		// power on central panel
		DigitalSwitches::getInstance()->powerOffCentralPanel(false);
		NVIC_SystemReset(); // reboot mbed
	}
	else if(rebootDeviceType == 2)
	{
		// reboot mbed system
		//wait_ms(100);
		NVIC_SystemReset();
	}
}

void Controller::disinfectTimeoutFun()
{
	//FTRACE("###Controller::disinfectTimeoutFun()\r\n");

	disinfectTimeoutTriggered = true;
}

void Controller::disinfectFinished()
{
	//LOG_INFO("###Controller::disinfectFinished()\r\n");
	// reset after finished
	CPDisinfectCostSec = 0;

	// step 1: stop hot water
	DigitalSwitches::getInstance()->closeHotWater();

	// step 2: stop membrane rinse
	Action* pAct = new ActionStartRinse(false);
	ActionBase::append(pAct);

	// step 2: re-set boiling temperature to heater
#if ENABLE_TEMPERATURE_COMPENSATION
	pAct = new ActionSetBoilTemp(WHBoilingTemp-CPCompensation.compensation, WHTempOffset);
#else
	pAct = new ActionSetBoilTemp(WHBoilingTemp, WHTempOffset);
#endif
	ActionBase::append(pAct);

	// step 3: inform Central Panel that disinfect is finished
	pAct = new ActionDisinfectStatus(1);
	ActionBase::append(pAct);

	// step 4: transfer to standby state
	StateMachine::getInstance()->transferToStandby();

	CPDisinfectStarted = false;
}

void Controller::disinfectMembraneTimeoutFun()
{
	//FTRACE("###Controller::disinfectMembraneTimeoutFun() CPDisinfectCostSec: %d\r\n", CPDisinfectCostSec);

	disinfectMembraneTimeoutTriggered = true;
}

void Controller::timeoutProcess()
{
	//
	if(appEjectTimeoutTriggered)
	{
        if(Controller::getInstance()->CPAppEjectCountdown > 0)
        {
            Action *pAct = new ActionCountDown(Controller::getInstance()->CPAppEjectCountdown);
		    ActionBase::append(pAct);

            Controller::getInstance()->CPAppEjectCountdown --;
        }
        else
        {
            waterReleaseTimes = 0; // reset
            Controller *ctrl = Controller::getInstance();
		    Controller::getInstance()->CPAppEjectCountdown = 0;
		    Action *pAct = NULL;

		    appEjectTimeoutTriggered = false;

		    StateMachine::getInstance()->transferToStandby();

		    // upload app water selling finished
		    // milli litre
		    int coldcap = 1000*ctrl->CPUserInfo.coldWaterPulse / ctrl->CPPulsePerLitreArr[CP_COLD_PULSE_PER_LITRE];
		    int hotcap = 1000*ctrl->CPUserInfo.hotWaterPulse / ctrl->CPPulsePerLitreArr[CP_HOT_PULSE_PER_LITRE];
		    // cents
		    int coldcost = coldcap * Controller::getInstance()->CPColdPrice / 1000;
		    int hotcost = hotcap * Controller::getInstance()->CPHotPrice / 1000;
		    if(Controller::getInstance()->CPUserInfo.accountType == 2) // wechat pay
		    {
			    coldcost = Controller::getInstance()->CPUserInfo.waterLimitBalance * coldcap /
						    Controller::getInstance()->CPUserInfo.waterLimitMilliLitre;
			    hotcost = Controller::getInstance()->CPUserInfo.waterLimitBalance * hotcap /
						    Controller::getInstance()->CPUserInfo.waterLimitMilliLitre;
		    }

		    pAct = new ActionWaterInfo(coldcost, coldcap, hotcost, hotcap, 0);
		    ActionBase::append(pAct);
        }
	}

	//
	if(disinfectTimeoutTriggered)
	{
		disinfectTimeoutTriggered = false;

		disinfectFinished();
	}

	//
	if(disinfectMembraneTimeoutTriggered)
	{
		disinfectMembraneTimeoutTriggered = false;

		// release hot water
		DigitalSwitches::getInstance()->openHotWater();
		// set timer to finish disinfect
		CPDisinfectTimeout.attach(Callback<void()>(this, &Controller::disinfectTimeoutFun), 60*CPDisinfectDur-CPDisinfectCostSec);
		// record discinfect start time
		CPDisinfectStartTime = time(NULL);
	}
}

int Controller::waterHeaterConnected()
{
	Action *pAct = NULL;

	// step 0: set water boiling temperature restrict
	{
		pAct = new ActionSetTempRestrict(WATER_HEATER_HIGHEST_TEMPERATURE);
		ActionBase::append(pAct);
	}

	// step 1: set water boiling temperature
	{
#if ENABLE_TEMPERATURE_COMPENSATION
		pAct = new ActionSetBoilTemp(WHBoilingTemp-CPCompensation.compensation, WHTempOffset);
#else
		pAct = new ActionSetBoilTemp(WHBoilingTemp, WHTempOffset);
#endif
		ActionBase::append(pAct);
	}

	// step2: set water warning temperature offset; default is 5
	{
		pAct = new ActionSetWarningTemp(WHWarningTempOffset);
		ActionBase::append(pAct);
	}

	// step 3: set status update interval of water heater
	{
		Action* pAct = new ActionStartFeedbackPeriodically(true, WH_STATUS_UPDATE_INTERVAL);
		ActionBase::append(pAct);
	}

	// step 4: set water heater work state
	//{
	//	if(checkWHWorkTime())
	//	{
	//		WHInService = true;
	//		pAct = new ActionStartHeating(true);
	//	}
	//	else
	//	{
	//		WHInService = false;
	//		pAct = new ActionStartHeating(false);
	//	}

	//	ActionBase::append(pAct);
	//}
	WHInService = false;

	return 0;
}

int Controller::calcFilterExpiredNS(bool bSetRemainDays)
{
	struct tm nt;
	memset(&nt, 0, sizeof(struct tm));
	nt.tm_sec = 59;
	nt.tm_min = 59;
	nt.tm_hour = 23;
	nt.tm_mday = CPFilterProductionDate.day;
	nt.tm_mon = ((CPFilterProductionDate.month + CPFilterShelfLife) % 12) - 1;
	nt.tm_year = CPFilterProductionDate.year + ((CPFilterProductionDate.month + CPFilterShelfLife) / 12) - 1900;
	CPFilterExPiredNS = mktime(&nt);

	if(bSetRemainDays)
	{
		CPRemainWorkableDays = (CPFilterExPiredNS - time(NULL)) / 86400; //  seconds of a day; 86400 = 24 * 60 * 60;
	}

	return 0;
}

int Controller::checkPlanActions(time_t seconds)
{
    uint8_t i;
    uint8_t is_valib = 0x00;
    
	struct tm *ptm = localtime(&seconds);
	if(!ptm) return 0;
	if(ptm->tm_wday == 0) ptm->tm_wday = 7;

	// check fixed time action
    for(i=0; i<DEVICE_RINSE_MAX_NUM; i++)
    {
        if((CPFilterRinseFixTimes[i].valid == true) && (CPFilterRinseFixTimes[i].day == ptm->tm_wday)  && (CPFilterRinseFixTimes[i].hour == ptm->tm_hour)  && (CPFilterRinseFixTimes[i].min == ptm->tm_min))
        {
            is_valib = 0x01;
            CurRinseFixTimesindex = i;
            break;
        }
    }

    if((CPFilterRinseFixTime.day == ptm->tm_wday)  && (CPFilterRinseFixTime.hour == ptm->tm_hour)  && (CPFilterRinseFixTime.min == ptm->tm_min))
    {
        is_valib = 0x01;
    }

    if(is_valib)
	{
		if(seconds - CPFilterRinseLast > FUNCTIONS_MINIMAL_INTERVAL)
		{
			LOG_INFO("###Controller::checkPlanActions() start filter rinse at fixed time\r\n");
			if(StateMachine::TR_OK == StateMachine::getInstance()->transferToFilterRinse())
				CPFilterRinseLast = seconds;
		}
	}

	if(CPDrainFixTime.valid)
	{
		if((CPDrainFixTime.day == ptm->tm_wday) &&
				(CPDrainFixTime.hour == ptm->tm_hour) &&
				(CPDrainFixTime.min == ptm->tm_min))
		{
			if(seconds - CPDrainLast > FUNCTIONS_MINIMAL_INTERVAL)
			{
				LOG_INFO("###Controller::checkPlanActions() start drain at fixed time\r\n");
				if(StateMachine::TR_OK == StateMachine::getInstance()->transferToDrainage())
					CPDrainLast = seconds;
			}
		}
	}

    for(i=0; i<DEVICE_RINSE_MAX_NUM; i++)
    {
        if((CPMembraneRinseFixTimes[i].valid == true) && (CPMembraneRinseFixTimes[i].day == ptm->tm_wday) && (CPMembraneRinseFixTimes[i].hour == ptm->tm_hour) && (CPMembraneRinseFixTimes[i].min == ptm->tm_min))
		{
            is_valib = 0x01;
            CurMembraneFixTimesindex = i;
            break;
        }
    }

    if((CPMembraneRinseFixTime.day == ptm->tm_wday) && (CPMembraneRinseFixTime.hour == ptm->tm_hour) && (CPMembraneRinseFixTime.min == ptm->tm_min))
	{
        is_valib = 0x01;
    }

    if(is_valib)
    {
		if(seconds - CPMembraneRinseLast > FUNCTIONS_MINIMAL_INTERVAL)
		{
			LOG_INFO("###Controller::checkPlanActions() start membrane rinse at fixed time\r\n");
			if(StateMachine::TR_OK == StateMachine::getInstance()->transferToMembraneRinse())
				CPMembraneRinseLast = seconds;
		}
	}

    for(i=0; i<DEVICE_RINSE_MAX_NUM; i++)
    {
        if((CPDisinfectFixTimes[i].valid == true) && (CPDisinfectFixTimes[i].day == ptm->tm_wday) && (CPDisinfectFixTimes[i].hour == ptm->tm_hour) && (CPDisinfectFixTimes[i].min == ptm->tm_min))
	    {
            is_valib = 0x01;
            CurDisinfectFixTimesindex = i;
            break;
        }
    }

    if((CPDisinfectFixTime.day == ptm->tm_wday) && (CPDisinfectFixTime.hour == ptm->tm_hour) && (CPDisinfectFixTime.min == ptm->tm_min))
    {
        is_valib = 0x01;
    }

    if(is_valib)
	{
		if(seconds - CPDisinfectLast > FUNCTIONS_MINIMAL_INTERVAL)
		{
			LOG_INFO("###Controller::checkPlanActions() start disinfect at fixed time\r\n");
			if(StateMachine::TR_OK == StateMachine::getInstance()->transferToDisinfect())
				CPDisinfectLast = seconds;
		}
	}

	// check plan action
	if(CPFilterRinsePlan.valid)
	{
		// reach the time tick
		if(CPFilterRinsePlan.plan_time+60*CPPlanDelayTime == seconds)
		{
			// check closer enough to last action
			if(seconds - CPFilterRinseLast > FUNCTIONS_MINIMAL_INTERVAL)
			{
				LOG_INFO("###Controller::checkPlanActions() start filter rinse at plan time\r\n");
				// check current state allowed to start or not
				if(StateMachine::TR_OK == StateMachine::getInstance()->transferToFilterRinse())
				{
					// record action time and reset plan time if started
					CPFilterRinseLast = seconds;

					memset(&CPFilterRinsePlan, 0, sizeof(CPFilterRinsePlan));
					CPFilterRinsePlan.valid = false;
				}
			}
		}
	}
	if(CPDrainPlan.valid)
	{
		if(CPDrainPlan.plan_time+60*CPPlanDelayTime == seconds)
		{
			if(seconds - CPDrainLast > FUNCTIONS_MINIMAL_INTERVAL)
			{
				LOG_INFO("###Controller::checkPlanActions() start drain rinse at plan time\r\n");
				if(StateMachine::TR_OK == StateMachine::getInstance()->transferToDrainage())
				{
					CPDrainLast = seconds;

					memset(&CPDrainPlan, 0, sizeof(CPDrainPlan));
					CPDrainPlan.valid = false;
				}
			}
		}
	}
	if(CPMembraneRinsePlan.valid)
	{
		if(CPMembraneRinsePlan.plan_time+60*CPPlanDelayTime == seconds)
		{
			if(seconds - CPMembraneRinseLast > FUNCTIONS_MINIMAL_INTERVAL)
			{
				LOG_INFO("###Controller::checkPlanActions() start membrane rinse at plan time\r\n");
				if(StateMachine::TR_OK == StateMachine::getInstance()->transferToMembraneRinse())
				{
					CPMembraneRinseLast = seconds;

					memset(&CPMembraneRinsePlan, 0, sizeof(CPMembraneRinsePlan));
					CPMembraneRinsePlan.valid = false;
				}
			}
		}
	}
	if(CPDisinfectPlan.valid)
	{
		if(CPDisinfectPlan.plan_time+60*CPPlanDelayTime == seconds)
		{
			if(seconds - CPDisinfectLast > FUNCTIONS_MINIMAL_INTERVAL)
			{
				LOG_INFO("###Controller::checkPlanActions() start disinfect at plan time\r\n");
				if(StateMachine::TR_OK == StateMachine::getInstance()->transferToDisinfect())
				{
					CPDisinfectLast = seconds;

					memset(&CPDisinfectPlan, 0, sizeof(CPDisinfectPlan));
					CPDisinfectPlan.valid = false;
				}
			}
		}
	}

	//Force off after emptying for a certain time
	if(StateMachine::getInstance()->inDrainageState())
	{
		if(CPDrainStartTime == 0)
		{
			CPDrainStartTime = seconds;
		}
		if(DRAIN_DURATION_TIME < seconds - CPDrainStartTime)
		{
			StateMachine::getInstance()->stopDrainageManually();
			CPDrainStartTime = 0;
		}
	}
	else
	{
		CPDrainStartTime = 0;
	}

	return 0;
}

int Controller::startDisinfect()
{
	LOG_INFO("###Controller::startDisinfect() CPDisinfectCostSec: %ds CPDisinfectDur: %dm\r\n", CPDisinfectCostSec, CPDisinfectDur);
	// private function to start disinfect;
	// now assume the temperature is high enough to disinfect
	if(CPDisinfectCostSec >= 60*CPDisinfectDur)
	{
		CPDisinfectTimeout.detach();
		disinfectTimeoutTriggered = false;
		disinfectFinished();
		return -1;
	}

	// avoid restarted
	if(CPDisinfectStarted)
		return -1;

	// step 1: open membrane rinse before disinfect
	Action* pAct = new ActionSetRinseDur(Controller::getInstance()->CPDisinfectDur + 2);
	ActionBase::append(pAct);
	pAct = new ActionStartRinse(true);
	ActionBase::append(pAct);

	// step2: open dust helmet
	DigitalSwitches::getInstance()->openDustHelmet(true);

	// start disinfect in 2 minutes
	CPDisinfectMembraneTimeout.attach(Callback<void()>(this, &Controller::disinfectMembraneTimeoutFun), 120); // 2 mins

	CPDisinfectStarted = true;

	return 0;
}

int Controller::suspendDisinfect()
{
	LOG_INFO("###Controller::suspendDisinfect()\r\n");
	// step 1: detach timer
	CPDisinfectTimeout.detach();
	disinfectTimeoutTriggered = false;
	CPDisinfectMembraneTimeout.detach();
	disinfectMembraneTimeoutTriggered = false;

	// step 2: suspend if ifrared detected
	if(CPDisinfectStartTime > 0)
	{
		CPDisinfectCostSec += time(NULL) - CPDisinfectStartTime;

		CPDisinfectStartTime = 0;
	}

	// step 3: stop hot water
	DigitalSwitches::getInstance()->closeHotWater();

	// step 4: stop membrane rinse
	Action* pAct = new ActionStartRinse(false);
	ActionBase::append(pAct);

	// step 5: re-set boiling temperature to heater
#if ENABLE_TEMPERATURE_COMPENSATION
	pAct = new ActionSetBoilTemp(WHBoilingTemp-CPCompensation.compensation, WHTempOffset);
#else
	pAct = new ActionSetBoilTemp(WHBoilingTemp, WHTempOffset);
#endif
	ActionBase::append(pAct);

	// step 6: inform Central Panel that disinfect is finished
	pAct = new ActionDisinfectStatus(1);
	ActionBase::append(pAct);

	// step 7: transfer to standby state
	StateMachine::getInstance()->transferToStandby();

	CPDisinfectStarted = false;

	return 0;
}

int Controller::resumeDisinfect()
{
	LOG_INFO("###Controller::resumeDisinfect() CPDisinfectCostSec: %d\r\n", CPDisinfectCostSec);
	if(!StateMachine::getInstance()->inStandbyState())
		return -1;

	Action* pAct = NULL;
	// step 1: check currnt temperature
	if(WHCurTemp >= CPDisinfectTemp)
	{
		// temperature is enough to disinfect
		// step 1-1: start disinfect
		if(startDisinfect() < 0) // disinfect time exhausted
			return 0;
	}
	else
	{
		// step 1-2: set disinfect temperature to heater
		pAct = new ActionSetBoilTemp(Controller::getInstance()->CPDisinfectTemp, 0);
		ActionBase::append(pAct);
	}

	// step 3: upload disinfect start event to Central Panel
	pAct = new ActionDisinfectStatus(0);
	ActionBase::append(pAct);

	// step 4: transfer to disinfect state
	StateMachine::getInstance()->transferTo(StateMachine::S10);
	return 0;
}

int Controller::setInvasionDetectedFlag(bool bDetected)
{
	CPInvasionDetected = bDetected;

	// save to PS
	PersistentStorage::getInstance()->saveInvasionDetectedFlag();
	return 0;
}

int Controller::getValveState()
{
	if(DigitalSwitches::getInstance()->feedWaterSwitchOpened())
	{
		valveStatus |= 0x08;
	}
	else
	{
		if(valveStatus & 0x08)
			valveStatus ^= 0x08;
	}
	
	if(DigitalSwitches::getInstance()->getColdWaterValveState())
	{
		valveStatus |= 0x04;
	}
	else
	{
		if(valveStatus & 0x04)
			valveStatus ^= 0x04;
	}

	if(DigitalSwitches::getInstance()->getHotWaterValveState())
	{
		valveStatus |= 0x02;
	}
	else
	{
		if(valveStatus & 0x02)
			valveStatus ^= 0x02;
	}

	if(DigitalSwitches::getInstance()->getIceWaterValveState())
	{
		valveStatus |= 0x01;
	}
	else
	{
		if(valveStatus & 0x01)
			valveStatus ^= 0x01;
	}
}

int Controller::updateOutletValveMark(unsigned char status)
{
	int valve[4] = {0};
	valve[0] = (status & 0x08) >> 3;
	valve[1] = (status & 0x04) >> 2;
	valve[2] = (status & 0x02) >> 1;
	valve[3] = status & 0x01;
	
	CentralPanelSerial::getInstance()->uploadValveState(valve[0],valve[1],valve[2],valve[3]);
	return 0;
}