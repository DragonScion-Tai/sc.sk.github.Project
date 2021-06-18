#include "utils.h"

#define LOG_MODULE 0x09

StateMachine g_StateMachine;
StateMachine* StateMachine::getInstance()
{
	return &g_StateMachine;
}

StateMachine::StateMachine()
{
	m_state = S1;

	m_bMembraneRinseState = false;
}

StateMachine::~StateMachine()
{

}

const char* StateMachine::toString(State s)
{
	static char state[4];
	snprintf(state, 4, "S%d", s);
	return state;
}

const char* StateMachine::currentState()
{
	return toString(m_state);
}

int StateMachine::transferTo(State s)
{
	bool bNeedSaveState = false;

	//State changes save the current state
	if(m_state != s)
	{
		bNeedSaveState = true;
	}

	// Exit from ...
	if(m_state == S12) // product test
    {
		bNeedSaveState = true;
    }
    else if(m_state == S5) // transfer from Sold State
    {
        //FTRACE("###StateMachine::transferTo() from SoldState; close Dust Helmet\r\n");
        DigitalSwitches::getInstance()->openDustHelmet(false);
    }

	m_state = s;

	// Enter into ...
	if(m_state == S12) // product test
		bNeedSaveState = true;

	// save to PS
	// reduce save times. Only used when in product test state now;
	if(bNeedSaveState)
	{
		PersistentStorage::getInstance()->saveStateMachineState();
	}
	return TR_OK;
}

int StateMachine::transferToStandby()
{
	Controller::getInstance()->CPUserInfo.accountState = 0; // deactivated

	transferTo(S2);
	return TR_OK;
}

int StateMachine::transferToSoldState()
{
	// initialize flow Pulse
	if(m_state != S6)
	{
		Controller::getInstance()->coldWaterFlowPulse = 0;
		Controller::getInstance()->hotWaterFlowPulse = 0;
        Controller::getInstance()->iceWaterFlowPulse = 0;

        // Open Helmet
        //FTRACE("###StateMachine::transferToSoldState() open Dust Helmet\r\n");
        DigitalSwitches::getInstance()->openDustHelmet(true);
	}

	transferTo(S5);
	return TR_OK;
}

int StateMachine::transferToWaterRelease()
{
	transferTo(S6);
	return TR_OK;
}

int StateMachine::transferToFilterRinse()
{
	//if((m_state != S2 && m_state != S4 && m_state != S8) || m_bMembraneRinseState)  // not Standby or Hibernate state
    if(!isFreeState())
		return TR_INVALID;

	// step 1: start filter rinse motor
	if(DigitalSwitches::getInstance())
		DigitalSwitches::getInstance()->startFilterRinse(true);

	// step 2: upload filter rinse start event to Central Panel
	Action* pAct = new Action(Action::AID_CP_FILTERRINSESTART);
	ActionBase::append(pAct);

	// from now on, we are in FILTER RINSE state
	transferTo(S7);
	return TR_OK;
}

int StateMachine::transferToMembraneRinse()
{
	FTRACE("###StateMachine::transferToMembraneRinse()\r\n");
	//if((m_state != S2 && m_state != S4 && m_state != S8) || m_bMembraneRinseState) // not Standby or Hibernate state
    if(!isFreeState())
		return TR_INVALID;

	m_bMembraneRinseState = true;

	Action* pAct = NULL;
	// step 1: set drain duration
	pAct = new ActionSetRinseDur(Controller::getInstance()->CPMembraneRinseDur);
	ActionBase::append(pAct);

	// step 2: trigger membrane rinse function
	pAct = new ActionStartRinse(true);
	ActionBase::append(pAct);

	// step 3: upload membrane rinse start event to Central Panel
	pAct = new Action(Action::AID_CP_MEMBRANERINSESTART);
	ActionBase::append(pAct);

	// from now on, we are in MEMBRANE RINSE state
	transferTo(S8);
	return TR_OK;
}

int StateMachine::transferToDrainage()
{
	//if((m_state != S2 && m_state != S4 && m_state != S8) || m_bMembraneRinseState) // not Standby or Hibernate state
    if(!isFreeState())
		return TR_INVALID;

	Action* pAct = NULL;
	// step 1: set drain duration
	pAct = new Action(Action::AID_WH_SET_DRAIN_DUR, Controller::getInstance());
	ActionBase::append(pAct);

	// step 2: trigger drain function
	pAct = new ActionStartDrain(true);
	ActionBase::append(pAct);

	// step 3: upload drain start event to Central Panel
	pAct = new Action(Action::AID_CP_DRAINING);
	ActionBase::append(pAct);

	// from now on, we are in DRAIN state
	transferTo(S9);
	return TR_OK;
}

int StateMachine::transferToDisinfect()
{
	//if((m_state != S2 && m_state != S4 && m_state != S8) || m_bMembraneRinseState) // not Standby or Hibernate state
    if(!isFreeState())
		return TR_INVALID;

	// do not disinfect if infrared detected
	if(DigitalSwitches::getInstance()->infraredDetectedWorks())
		return 0;

	// initialize the cost record; none ZERO means in disinfect pending
	Controller::getInstance()->CPDisinfectCostSec = 1;
	Controller::getInstance()->CPDisinfectStarted = false;

	// start disinfect
	Controller::getInstance()->resumeDisinfect();

	// from now on, we are in DISINFECT state
	transferTo(S10);
	return 0;
}

int StateMachine::transferToFatalErrorState()
{
	// step 1: finish current job
	// deactivate account and stop water releasing if...
	if(inSoldState())
	{
		// stop water release
		DigitalSwitches::getInstance()->closeHotWater();
		DigitalSwitches::getInstance()->closeColdWater();

		// exit account
		Controller::getInstance()->updateAccountInfo(true);

		// deactivate account
		Controller::getInstance()->CPUserInfo.accountState = 0x0;
	}
	// exit product test procedure
	else if(inProductionTestState())
		Controller::getInstance()->exitProdTest();
	// stop disinfect
	else if(inDisinfectState())
		stopDisinfectManually();
	// stop filter rinse
	else if(inFilterRinseState())
	{
		stopFilterRinseManually();
		// B631 goes back In-Serv will cost some time
		// at that time the state has changed to FATALERROR
		// upload filter rinse END event to Central Panel
		Action* pAct = new Action(Action::AID_CP_FILTERRINSEEND);
		ActionBase::append(pAct);
	}
	// stop drain
	else if(inDrainageState())
		stopDrainageManually();
	// stop membrane rinse; membrane rinse may happen same time with water sold state
	/*else */if(inMembraneRinseState())
	{
		stopMembraneRinseManually();
		NotifyMembraneRinseComplete();
	}

	// step 2: close B3 valve and beep
	DigitalSwitches::getInstance()->turnOnFeedWaterValve(false);
	DigitalSwitches::getInstance()->turnOnBeep(10);

	// step 3: stop water heater working
	ActionStartHeating* pAct = new ActionStartHeating(false);
	ActionBase::append(pAct);

	// from now on, we are in FATAL ERROR state
	transferTo(S11);
	return 0;
}

int StateMachine::transferToProductionTest()
{
	if(m_state != S2 && m_state != S4) // not Standby or Hibernate state
		return TR_INVALID;

	// start product test process in 30 sec; wait 631 valve goes back to In-serv
	Controller::getInstance()->startProductionTestTimeout(30);

	// close B3;
	DigitalSwitches::getInstance()->turnOnFeedWaterValve(false);

	// stop heater
	Action* pAct = new ActionStartHeating(false);
	ActionBase::append(pAct);

	// notify the product test is started
	pAct = new ActionProductTestStatus(0);
	ActionBase::append(pAct);

	// from now on, we are in PRODUCT TEST state
	transferTo(S12);
	return TR_OK;
}

//
int StateMachine::stopFilterRinseManually()
{
	if(!inFilterRinseState())
		return TR_OK;

	// stop filter rinse, and move to In-serv position
	if(DigitalSwitches::getInstance())
		DigitalSwitches::getInstance()->startFilterRinse(false);

	// upload complete event and tranfer to standby state
	// done after move to In-serv position

	return TR_OK;
}

int StateMachine::stopMembraneRinseManually()
{
	if(!inMembraneRinseState())
		return TR_OK;

	m_bMembraneRinseState = false;

	// stop membrane rinse
	Action* pAct = new ActionStartRinse(false);
	ActionBase::append(pAct);

	// upload membrane rinse complete event
	pAct = new Action(Action::AID_CP_MEMBRANERINSEEND);
	ActionBase::append(pAct);

	// transfer to standby state
	transferToStandby();

	return TR_OK;
}

int StateMachine::NotifyMembraneRinseComplete()
{
	m_bMembraneRinseState = false;

	return 0;
}

int StateMachine::stopDrainageManually()
{
	if(!inDrainageState())
		return TR_OK;

	// stop drain
	Action* pAct = new ActionStartDrain(false);
	ActionBase::append(pAct);

	// upload drain complete event
	pAct = new Action(Action::AID_CP_DRAINEND);
	ActionBase::append(pAct);

	// tranfer to standby state here
	transferToStandby();

	return TR_OK;
}

int StateMachine::stopDisinfectManually()
{
	if(!inDisinfectState())
		return TR_OK;

	// transfer to standby state first
	transferToStandby();

	// stop timeout; call timeout function manually
	Controller::getInstance()->CPDisinfectTimeout.detach();
	Controller::getInstance()->disinfectTimeoutTriggered = false;
	Controller::getInstance()->CPDisinfectMembraneTimeout.detach();
	Controller::getInstance()->disinfectMembraneTimeoutTriggered = false;

	Controller::getInstance()->disinfectFinished();

	return TR_OK;
}

// private
bool StateMachine::isFreeState()
{
    if((m_state != S2 && m_state != S4 && m_state != S8) || m_bMembraneRinseState) // not Standby or Hibernate state
        return false;

    // cold water free case
    if(m_state == S2 && Controller::getInstance()->CPColdPrice == 0 && DigitalSwitches::getInstance()->coldWaterOpened())
        return false;

    // hot water free case
    if(m_state == S2 && Controller::getInstance()->CPHotPrice == 0 && DigitalSwitches::getInstance()->hotWaterOpened())
        return false;

    // ice water free case
#ifndef TDS_CONTROL_SWITCH
     if(m_state == S2 && Controller::getInstance()->CPHotPrice == 0 && DigitalSwitches::getInstance()->iceWaterOpened())
         return false;
#endif

    return true;
}

//Upload current status
int StateMachine::uploadStatus()
{
	if(m_state == S2)
}