#include "utils.h"
#include "ICCard.h"

#define LOG_MODULE 0x08

ActionBase* g_pActionList = NULL;

ActionBase::ActionBase()
{
	status = PENDING;
	sendCount = 0;

	//FTRACE("###ActionBase::ActionBase() 0x%p\r\n", this);
}

ActionBase::~ActionBase()
{
	//FTRACE("###ActionBase::~ActionBase() 0x%p\r\n", this);
}

int ActionBase::action()
{
	startTime = time(NULL);
	status = EXECUTED;
	sendCount ++;

	return 0;
}

int ActionBase::append(ActionBase* newItem)
{
	int count = 1;
	int sCnt = 0;
	if(!newItem)
		return -1;

	if(newItem->getActionId() == Action::AID_CP_WARNINGINFO) // multi-warning maybe triggered at same time
		sCnt = ActionWarningInfo::removeActonIfWithSameType((ActionWarningInfo*)newItem);
	else
		sCnt = removeActionIfHas(newItem->getActionId());

	newItem->sendCount = sCnt; // remember the send failed count if remove same type action;

	ActionBase* next = g_pActionList;
	if(!next)
	{
		g_pActionList = newItem;
		newItem->next = NULL;

		return 0;
	}

	while(next->next)
	{
		next = next->next;
		count ++;
	}

	if(count > 20) //max action command pending
	{
		LOG_ERROR("###ActionBase::append() too much action appended, aId(%d)\r\n", newItem->getActionId());
		delete newItem;
		return -1;
	}

	next->next = newItem;
	newItem->next = NULL;

	return 0;
}

int ActionBase::pop()
{
	ActionBase* next = g_pActionList;
	if(!next)
		return -1;

	if(next)
	{
		g_pActionList = next->next;

		delete next;
		next = NULL;
	}

	return 0;
}

int ActionBase::removeActionIfHas(int aid)
{
	int sCnt = 0;
	if(!g_pActionList)
		return 0;

	// remove header if has
	if(g_pActionList && g_pActionList->getActionId() == aid)
	{
		ActionBase* removed = g_pActionList;
		g_pActionList = g_pActionList->next;
		sCnt = removed->sendCount;
		delete removed;
		removed = NULL;

		return sCnt;
	}

	// remove not header if has
	ActionBase* prev = g_pActionList;
	ActionBase* next = prev->next;
	while(next)
	{
		if(next->getActionId() == aid)
		{
			prev->next = next->next;
			sCnt = next->sendCount;
			delete next;
			next = NULL;

			return sCnt;
		}

		prev = next;
		next = prev->next;
	}

	return 0;
}

ActionBase* ActionBase::header()
{
	return g_pActionList;
}

Action::Action(int id, Controller* pCtrl)
{
	actionId = id;
	ctrl = pCtrl;
}

Action::~Action()
{

}

int Action::getActionId()
{
	return actionId;
}

int Action::action()
{
	int retVal = 0;
	ActionBase::action();

	switch(actionId)
	{
	case AID_WH_HANDSHAKE:
		if(WaterHeaterSerial::getInstance())
			retVal = WaterHeaterSerial::getInstance()->handShake();
		break;
	case AID_WH_GETSTATUS:
		if(WaterHeaterSerial::getInstance())
			retVal = WaterHeaterSerial::getInstance()->getStatus();
		break;
	case AID_WH_SET_DRAIN_DUR:
		if(WaterHeaterSerial::getInstance() && ctrl)
			WaterHeaterSerial::getInstance()->setDrainDuration(ctrl->CPDrainMinS, ctrl->CPDrainMinE);
		break;
	//
	case AID_CP_DEVICEID:
		if(CentralPanelSerial::getInstance() && ctrl)
        {
			retVal = CentralPanelSerial::getInstance()->uploadDeviceId(ctrl->CPDeviceId);
        }
		break;
    case AID_CP_WARNINGS:
        if(CentralPanelSerial::getInstance())
			retVal = CentralPanelSerial::getInstance()->uploadWarnings();
		break;
	case AID_CP_FILTERRINSESTART:
		if(CentralPanelSerial::getInstance())
			retVal = CentralPanelSerial::getInstance()->uploadFilterRinseStartEvent();
		break;
	case AID_CP_FILTERRINSEEND:
		if(CentralPanelSerial::getInstance())
			retVal = CentralPanelSerial::getInstance()->uploadFilterRinseEndEvent();
		break;
	case AID_CP_DRAINING:
		if(CentralPanelSerial::getInstance())
			retVal = CentralPanelSerial::getInstance()->uploadDrainingEvent();
		break;
	case AID_CP_DRAINEND:
		if(CentralPanelSerial::getInstance())
			retVal = CentralPanelSerial::getInstance()->uploadDrainEndEvent();
		break;
	//case AID_CP_WATERTANKSTATUS:
	//	if(CentralPanelSerial::getInstance() && ctrl)
	//		retVal = CentralPanelSerial::getInstance()->uploadWaterTankInfo(ctrl->WHCurTemp,
	//					ctrl->CPFilterWaterCur, ctrl->CPFilterWaterMax);
	//	break;
	case AID_CP_COUNTDOWN:
		if(CentralPanelSerial::getInstance() && ctrl)
			retVal = CentralPanelSerial::getInstance()->uploadCountdown(ctrl->CPAppEjectCountdown);
		break;
	case AID_CP_FILTERWORKABLEDAYS:
		if(CentralPanelSerial::getInstance() && ctrl)
		{
			retVal = CentralPanelSerial::getInstance()->uploadFilterWorkableDays(ctrl->CPRemainWorkableDays);
		}
		break;
	case AID_CP_VERSION:
		if(CentralPanelSerial::getInstance())
			retVal = CentralPanelSerial::getInstance()->uploadVersion(VER_MAJOR, VER_MINOR, VER_BUILD);
		break;
	case AID_CP_MEMBRANERINSESTART:
		if(CentralPanelSerial::getInstance())
			retVal = CentralPanelSerial::getInstance()->uploadMembraneRinseStartEvent();
		break;
	case AID_CP_MEMBRANERINSEEND:
		if(CentralPanelSerial::getInstance())
			retVal = CentralPanelSerial::getInstance()->uploadMembraneRinseEndEvent();
		break;
	case AID_CP_INFRAREDSENSORWORKS:
		if(CentralPanelSerial::getInstance())
			retVal = CentralPanelSerial::getInstance()->uploadInfraredSensorWorksEvent();
		break;
	case AID_CP_INFRAREDSENSORIDLE:
		if(CentralPanelSerial::getInstance())
			retVal = CentralPanelSerial::getInstance()->uploadInfraredSensorIdleEvent();
		break;
	case AID_CP_REQUESTSYNCSYSTIME:
		if(CentralPanelSerial::getInstance())
			retVal = CentralPanelSerial::getInstance()->uploadRequestSyncSysTime();
		break;
	case AID_CP_VALVE_STATE:
		if(CentralPanelSerial::getInstance())
			retVal = Controller::getInstance()->updateOutletValveMark(Controller::getInstance()->valveStatus);
		break;
	// case AID_CP_STEERING_ANGLE:
	// 	if(CentralPanelSerial::getInstance())
	// 		retVal = CentralPanelSerial::getInstance()->uploadSteeringAngle(CWaterMixerValve::getInstance()->angle);
	// 	break;
	//
	case AID_CR_HANDSHAKE:
		if(ICCardReaderSerial::getInstance())
			retVal = ICCardReaderSerial::getInstance()->handShake();
		break;
	case AID_CR_ENABLEREADER:
		if(ICCardReaderSerial::getInstance())
			retVal = ICCardReaderSerial::getInstance()->enableCardReader(true);
		break;
	case AID_CR_SETSECTORKEY:
		if(ICCardReaderSerial::getInstance())
		{
			char key_type = 0x01; //
			char key[6];
			iccard_get_key(key_type, key);
			retVal = ICCardReaderSerial::getInstance()->setSectorKey(iccard_get_block_num(), key_type, key);
		}
		break;
	case AID_CR_READCARD:
		if(ICCardReaderSerial::getInstance())
			retVal = ICCardReaderSerial::getInstance()->readBlock(iccard_get_block_num());
		break;
	default:
		break;
	}

	return retVal;
}

const char* Action::getActionName()
{
    switch(actionId)
    {
    case AID_WH_HANDSHAKE:
        return "AID_WH_HANDSHAKE";
	case AID_WH_STARTHEATING:
        return "AID_WH_STARTHEATING";
	case AID_WH_BOILINGTEMP:
        return "AID_WH_BOILINGTEMP";
	case AID_WH_RINSE:
        return "AID_WH_RINSE";
	case AID_WH_DRAIN:
        return "AID_WH_DRAIN";
	case AID_WH_WARNINGTEMP:
        return "AID_WH_WARNINGTEMP";
	case AID_WH_SOFTRESET:
        return "AID_WH_SOFTRESET";
	case AID_WH_FEEDBACK:
        return "AID_WH_FEEDBACK";
	case AID_WH_GETSTATUS:
        return "AID_WH_GETSTATUS";
	case AID_WH_SET_RINSE_DUR:
        return "AID_WH_SET_RINSE_DUR";
	case AID_WH_SET_DRAIN_DUR:
        return "AID_WH_SET_DRAIN_DUR";
	case AID_WH_SET_TEMP_RESTRICT:
        return "AID_WH_SET_TEMP_RESTRICT";
	case AID_CP_WATERINFO:
        return "AID_CP_WATERINFO";
	case AID_CP_CARDINFO:
        return "AID_CP_CARDINFO";
	case AID_CP_CARDWATERINFO:
        return "AID_CP_CARDWATERINFO";
	case AID_CP_WATERQUALITYINFO:
        return "AID_CP_WATERQUALITYINFO";
	case AID_CP_WARNINGINFO:
        return "AID_CP_WARNINGINFO";
	case AID_CP_FILTERINFO:
        return "AID_CP_FILTERINFO";
	case AID_CP_CONFIGINFO:
        return "AID_CP_CONFIGINFO";
	case AID_CP_FILTERRINSESTART:
        return "AID_CP_FILTERRINSESTART";
	case AID_CP_FILTERRINSEEND:
        return "AID_CP_FILTERRINSEEND";
	case AID_CP_DRAINING:
        return "AID_CP_DRAINING";
	case AID_CP_DRAINEND:
        return "AID_CP_DRAINEND";
	case AID_CP_WATERTANKSTATUS:
        return "AID_CP_WATERTANKSTATUS";
	case AID_CP_WATERSTATUS:
        return "AID_CP_WATERSTATUS";
	case AID_CP_COUNTDOWN:
        return "AID_CP_COUNTDOWN";
	case AID_CP_FILTERWORKABLEDAYS:
        return "AID_CP_FILTERWORKABLEDAYS";
	case AID_CP_VERSION:
        return "AID_CP_VERSION";
	case AID_CP_MEMBRANERINSESTART:
        return "AID_CP_MEMBRANERINSESTART";
	case AID_CP_MEMBRANERINSEEND:
        return "AID_CP_MEMBRANERINSEEND";
	case AID_CP_DEVICEID:
        return "AID_CP_DEVICEID";
    case AID_CP_WARNINGS:
        return "AID_CP_WARNINGS";
	case AID_CP_QRCODE:
        return "AID_CP_QRCODE";
	case AID_CP_INFRAREDSENSORWORKS:
        return "AID_CP_INFRAREDSENSORWORKS";
	case AID_CP_INFRAREDSENSORIDLE:
        return "AID_CP_INFRAREDSENSORIDLE";
	case AID_CP_FIRMWAREUPDATESTATUS:
        return "AID_CP_FIRMWAREUPDATESTATUS";
	case AID_CP_OPERATIONLOGSTATUS:
        return "AID_CP_OPERATIONLOGSTATUS";
	case AID_CP_OPERATIONLOG:
        return "AID_CP_OPERATIONLOG";
	case AID_CP_PRODUCTTESTSTATUS:
        return "AID_CP_PRODUCTTESTSTATUS";
	case AID_CP_PRODUCTTESTCOUNTDOWN:
        return "AID_CP_PRODUCTTESTCOUNTDOWN";
	case AID_CP_DISINFECTSTATUS:
        return "AID_CP_DISINFECTSTATUS";
	case AID_CP_REQUESTSYNCSYSTIME:
        return "AID_CP_REQUESTSYNCSYSTIME";
	case AID_CP_VALVE_STATE:
		return "AID_CP_VALVE_STATE";
	case AID_CP_STEERING_ANGLE:
		return "AID_CP_STEERING_ANGLE";
	case AID_CR_HANDSHAKE:
        return "AID_CR_HANDSHAKE";
	case AID_CR_ENABLEREADER:
        return "AID_CR_ENABLEREADER";
	case AID_CR_SETSECTORKEY:
        return "AID_CR_SETSECTORKEY";
	case AID_CR_WRITECARD:
        return "AID_CR_WRITECARD";
	case AID_CR_READCARD:
        return "AID_CR_READCARD";
    }

    return "Unknown Action";
}

ActionStartHeating::ActionStartHeating(bool bOn): Action(AID_WH_STARTHEATING)
{
	bOnOff = bOn;
}

int ActionStartHeating::action()
{
	ActionBase::action();

	return WaterHeaterSerial::getInstance()->startHeating(bOnOff);
}

ActionSetBoilTemp::ActionSetBoilTemp(char temp, char offset): Action(AID_WH_BOILINGTEMP)
{
	this->temp = temp;
	this->offset = offset;
}

int ActionSetBoilTemp::action()
{
	ActionBase::action();

	return WaterHeaterSerial::getInstance()->setBoilTemp(temp, offset);
}

ActionSetRinseDur::ActionSetRinseDur(int min): Action(AID_WH_SET_RINSE_DUR)
{
	duration = min;
}

int ActionSetRinseDur::action()
{
	ActionBase::action();

	return WaterHeaterSerial::getInstance()->setRinseDuration(duration);
}

ActionStartRinse::ActionStartRinse(bool bOn): Action(AID_WH_RINSE)
{
	bOnOff = bOn;
}

int ActionStartRinse::action()
{
	ActionBase::action();

	return WaterHeaterSerial::getInstance()->startRinse(bOnOff);
}

ActionStartDrain::ActionStartDrain(bool bOn): Action(AID_WH_DRAIN)
{
	bOnOff = bOn;
}

int ActionStartDrain::action()
{
	ActionBase::action();

	return WaterHeaterSerial::getInstance()->startDrain(bOnOff);
}

ActionSetWarningTemp::ActionSetWarningTemp(char temp): Action(AID_WH_WARNINGTEMP)
{
	warningTemp = temp;
}

int ActionSetWarningTemp::action()
{
	ActionBase::action();

	return WaterHeaterSerial::getInstance()->setWarnTempOffset(warningTemp);
}

ActionStartFeedbackPeriodically::ActionStartFeedbackPeriodically(bool bOn, char sec): Action(AID_WH_FEEDBACK)
{
	bOnOff = bOn;
	seconds = sec;
}

int ActionStartFeedbackPeriodically::action()
{
	ActionBase::action();

	return WaterHeaterSerial::getInstance()->startFeedbackPeriodically(bOnOff, seconds);
}

ActionSetTempRestrict::ActionSetTempRestrict(char temp): Action(AID_WH_SET_TEMP_RESTRICT)
{
	restrictTemp = temp;
}

int ActionSetTempRestrict::action()
{
	ActionBase::action();
	return WaterHeaterSerial::getInstance()->setBoilingTempRestrict(restrictTemp);
}

//
ActionWaterInfo::ActionWaterInfo(int coldcost, int coldcap, int hotcost, int hotcap, int bOn, int balance): Action(AID_CP_WATERINFO)
{
	this->coldcost = coldcost;
	this->coldcap = coldcap;
	this->hotcost = hotcost;
	this->hotcap = hotcap;
	this->bOn = bOn;
	this->balance = balance;
	if(this->balance >= 0)
		actionId = AID_CP_CARDWATERINFO;
}

int ActionWaterInfo::action()
{
	int retVal = 0;
	ActionBase::action();
	if(actionId == AID_CP_WATERINFO)
		retVal = CentralPanelSerial::getInstance()->uploadAppWaterInfo(coldcost, coldcap, hotcost, hotcap, bOn);
	if(actionId == AID_CP_CARDWATERINFO)
		retVal = CentralPanelSerial::getInstance()->uploadCardWaterInfo(coldcost, coldcap, hotcost, hotcap, balance, bOn);

	return retVal;
}

ActionWaterTankStatus::ActionWaterTankStatus(int curTemp, int curWater, int maxWater): Action(AID_CP_WATERTANKSTATUS)
{
	this->curTemp = curTemp;
	this->curWater = curWater;
	this->maxWater = maxWater;
}

int ActionWaterTankStatus::action()
{
	ActionBase::action();

	return CentralPanelSerial::getInstance()->uploadWaterTankInfo(curTemp, curWater, maxWater);
}

int ActionCardInfo::index = 0x00;
ActionCardInfo::ActionCardInfo(int id, int balance, int region, int flag, int cardType): Action(AID_CP_CARDINFO)
{
	this->cardId   = id;
	this->balance  = balance;
	this->region   = region;
	this->flag     = flag;
	this->cardType = cardType;    
}

int ActionCardInfo::action()
{    
	ActionBase::action();
    
	return CentralPanelSerial::getInstance()->uploadCardBalance(cardId, balance, region, flag, cardType, ++index);
}

ActionWaterQualityInfo::ActionWaterQualityInfo(int type, int value): Action(AID_CP_WATERQUALITYINFO)
{
	this->type = type;
	this->value = value;
}

int ActionWaterQualityInfo::action()
{
	ActionBase::action();

	return CentralPanelSerial::getInstance()->uploadWaterQuality(type, value);
}

ActionWarningInfo::ActionWarningInfo(int type, int bOn): Action(AID_CP_WARNINGINFO)
{
	warningType = type;
	bWarning = bOn;
}

int ActionWarningInfo::action()
{
	ActionBase::action();

	return CentralPanelSerial::getInstance()->uploadWarningType(warningType, bWarning);
}

int ActionWarningInfo::removeActonIfWithSameType(ActionWarningInfo* warning)
{
	int sCnt = 0;
	if(!g_pActionList || !warning)
		return 0;

	// remove header if has
	if(g_pActionList->getActionId() == AID_CP_WARNINGINFO)
	{
		if(((ActionWarningInfo*)g_pActionList)->warningType == warning->warningType)
		{
			ActionBase* removed = g_pActionList;
			g_pActionList = g_pActionList->next;
			sCnt = removed->sendCount;
			delete removed;
			removed = NULL;

			return sCnt;
		}
	}

	// remove not header if has
	ActionBase* prev = g_pActionList;
	ActionBase* next = prev->next;
	while(next)
	{
		if(next->getActionId() == AID_CP_WARNINGINFO)
		{
			if(((ActionWarningInfo*)next)->warningType == warning->warningType)
			{
				prev->next = next->next;
				sCnt = next->sendCount;
				delete next;
				next = NULL;

				// only one
				return sCnt;
			}
		}

		prev = next;
		next = prev->next;
	}

	return 0;
}

ActionFilterInfo::ActionFilterInfo(int cur, int limit): Action(AID_CP_FILTERINFO)
{
	filterWaterCur = cur;
	filterWaterLimit = limit;
}

int ActionFilterInfo::action()
{
	ActionBase::action();

	return CentralPanelSerial::getInstance()->uploadFilterInfo(filterWaterCur/1000, filterWaterCur%1000, 
																filterWaterLimit/1000, filterWaterLimit%1000);
}

ActionConfigInfo::ActionConfigInfo(Controller* pCtrl): Action(AID_CP_CONFIGINFO)
{
	ctrl = pCtrl;
	index = SETID_FIRST;
}

int ActionConfigInfo::action()
{
	ActionBase::action();
	if(!ctrl)
		return -1;

	switch(index)
	{
		case SETID_TEMPERATURE:
			CentralPanelSerial::getInstance()->uploadTemperature(ctrl->WHBoilingTemp);
			break;
		case SETID_PRICE:
			CentralPanelSerial::getInstance()->uploadWaterPrice(ctrl->CPHotPrice, ctrl->CPColdPrice, ctrl->CPIcePrice);
			break;
		case SETID_FILTER_LIMIT:
			CentralPanelSerial::getInstance()->uploadFilterLimit(ctrl->CPFilterWaterMax);
			break;
		case SETIF_PLAN_DELAY:
			CentralPanelSerial::getInstance()->uploadPlanDelayTime(ctrl->CPPlanDelayTime);
			break;
		case SETID_FILTER_CUR:
			CentralPanelSerial::getInstance()->uploadFilterCur(ctrl->CPFilterWaterCur);
			break;
		case SETID_FILTER_PRODUCE_DATE:
			CentralPanelSerial::getInstance()->uploadFilterProduceDate(ctrl->CPFilterProductionDate.year, 
								ctrl->CPFilterProductionDate.month, ctrl->CPFilterProductionDate.day);
			break;
		case SETID_FILTER_RINSE_DUR:
			CentralPanelSerial::getInstance()->uploadFilterRinseDur(ctrl->CPFilterBackRinseDur, ctrl->CPFilterFastRinseDur);
			break;
		case SETID_CHARGE_WAY:
			CentralPanelSerial::getInstance()->uploadChargeWay(ctrl->CPChargeMode, ctrl->CPChargeUnit);
			break;
		case SETID_MEMBRANE_RINSE_DUR:
			CentralPanelSerial::getInstance()->uploadMembraneRinseDur(ctrl->CPMembraneRinseDur);
			break;
		case SETID_SERVICE_TIME_D1:
			CentralPanelSerial::getInstance()->uploadServiceTime(1, ctrl->CPServiceTime[0].start, ctrl->CPServiceTime[0].end);
			break;
		case SETID_SERVICE_TIME_D2:
			CentralPanelSerial::getInstance()->uploadServiceTime(2, ctrl->CPServiceTime[1].start, ctrl->CPServiceTime[1].end);
			break;
		case SETID_SERVICE_TIME_D3:
			CentralPanelSerial::getInstance()->uploadServiceTime(3, ctrl->CPServiceTime[2].start, ctrl->CPServiceTime[2].end);
			break;
		case SETID_SERVICE_TIME_D4:
			CentralPanelSerial::getInstance()->uploadServiceTime(4, ctrl->CPServiceTime[3].start, ctrl->CPServiceTime[3].end);
			break;
		case SETID_SERVICE_TIME_D5:
			CentralPanelSerial::getInstance()->uploadServiceTime(5, ctrl->CPServiceTime[4].start, ctrl->CPServiceTime[4].end);
			break;
		case SETID_SERVICE_TIME_D6:
			CentralPanelSerial::getInstance()->uploadServiceTime(6, ctrl->CPServiceTime[5].start, ctrl->CPServiceTime[5].end);
			break;
		case SETID_SERVICE_TIME_D7:
			CentralPanelSerial::getInstance()->uploadServiceTime(7, ctrl->CPServiceTime[6].start, ctrl->CPServiceTime[6].end);
			break;
		case SETID_FILTER_SHELFLIFE:
			CentralPanelSerial::getInstance()->uploadFilterShelflive(ctrl->CPFilterShelfLife);
			break;
		case SETID_COMPENSATION_TEMP:
			CentralPanelSerial::getInstance()->uploadCompensationTemp(ctrl->CPCompensation.startTemp, ctrl->CPCompensation.compensation);
			break;
		case SETID_HEATER_WORK_TIME_D1:
			CentralPanelSerial::getInstance()->uploadHeaterWorkTime(1, ctrl->CPHeaterWorkTime[0].start, ctrl->CPHeaterWorkTime[0].end);
			break;
		case SETID_HEATER_WORK_TIME_D2:
			CentralPanelSerial::getInstance()->uploadHeaterWorkTime(2, ctrl->CPHeaterWorkTime[1].start, ctrl->CPHeaterWorkTime[1].end);
			break;
		case SETID_HEATER_WORK_TIME_D3:
			CentralPanelSerial::getInstance()->uploadHeaterWorkTime(3, ctrl->CPHeaterWorkTime[2].start, ctrl->CPHeaterWorkTime[2].end);
			break;
		case SETID_HEATER_WORK_TIME_D4:
			CentralPanelSerial::getInstance()->uploadHeaterWorkTime(4, ctrl->CPHeaterWorkTime[3].start, ctrl->CPHeaterWorkTime[3].end);
			break;
		case SETID_HEATER_WORK_TIME_D5:
			CentralPanelSerial::getInstance()->uploadHeaterWorkTime(5, ctrl->CPHeaterWorkTime[4].start, ctrl->CPHeaterWorkTime[4].end);
			break;
		case SETID_HEATER_WORK_TIME_D6:
			CentralPanelSerial::getInstance()->uploadHeaterWorkTime(6, ctrl->CPHeaterWorkTime[5].start, ctrl->CPHeaterWorkTime[5].end);
			break;
		case SETID_HEATER_WORK_TIME_D7:
			CentralPanelSerial::getInstance()->uploadHeaterWorkTime(7, ctrl->CPHeaterWorkTime[6].start, ctrl->CPHeaterWorkTime[6].end);
			break;
		case SETID_DISINFECT_DUR:
			CentralPanelSerial::getInstance()->uploadDisinfectDur(ctrl->CPDisinfectDur);
			break;
		case SETID_PULSE_PER_LITRE:
			CentralPanelSerial::getInstance()->uploadPulsePerLitre(ctrl->CPPulsePerLitreArr[CP_PULSE_PER_LITRE], ctrl->CPPulsePerLitreArr[CP_HOT_PULSE_PER_LITRE], ctrl->CPPulsePerLitreArr[CP_COLD_PULSE_PER_LITRE], ctrl->CPPulsePerLitreArr[CP_ICE_PULSE_PER_LITRE], ctrl->CPPulsePerLitreArr[CP_PULSE_PER_LITRE_5], ctrl->CPPulsePerLitreArr[CP_PULSE_PER_LITRE_6]);
			break;
		case SETID_FILTER_RINSE_FIXTIME:
			CentralPanelSerial::getInstance()->uploadFilterRinseFixtime(ctrl->CPFilterRinseFixTime.day, ctrl->CPFilterRinseFixTime.hour, ctrl->CPFilterRinseFixTime.min);
			break;
		case SETID_FILTER_RINSE_PLAN:
			CentralPanelSerial::getInstance()->uploadFilterRinsePlan(ctrl->CPFilterRinsePlan.year, ctrl->CPFilterRinsePlan.month,
				ctrl->CPFilterRinsePlan.day, ctrl->CPFilterRinsePlan.hour, ctrl->CPFilterRinsePlan.min);
			break;
		case SETID_DRAIN_FIXTIME:
			CentralPanelSerial::getInstance()->uploadDrainFixtime(ctrl->CPDrainFixTime.day,
				ctrl->CPDrainFixTime.hour, ctrl->CPDrainFixTime.min);
			break;
		case SETID_DRAIN_PLAN:
			CentralPanelSerial::getInstance()->uploadDrainPlan(ctrl->CPDrainPlan.year, ctrl->CPDrainPlan.month,
				ctrl->CPDrainPlan.day, ctrl->CPDrainPlan.hour, ctrl->CPDrainPlan.min);
			break;
		case SETID_MEMBRANE_RINSE_FIXTIME:
			CentralPanelSerial::getInstance()->uploadMembraneRinseFixtime(ctrl->CPMembraneRinseFixTime.day, ctrl->CPMembraneRinseFixTime.hour, ctrl->CPMembraneRinseFixTime.min);
			break;
		case SETID_MEMBRANE_RINSE_PLAN:
			CentralPanelSerial::getInstance()->uploadMembraneRinsePlan(ctrl->CPMembraneRinsePlan.year, ctrl->CPMembraneRinsePlan.month,
				ctrl->CPMembraneRinsePlan.day, ctrl->CPMembraneRinsePlan.hour, ctrl->CPMembraneRinsePlan.min);
			break;
		case SETID_DISINFECT_FIXTIME:
			CentralPanelSerial::getInstance()->uploadDisinfectFixtime(ctrl->CPDisinfectFixTime.day, ctrl->CPDisinfectFixTime.hour, ctrl->CPDisinfectFixTime.min);
			break;
		case SETID_DISINFECT_PLAN:
			CentralPanelSerial::getInstance()->uploadDisinfectPlan(ctrl->CPDisinfectPlan.year, ctrl->CPDisinfectPlan.month,
				ctrl->CPDisinfectPlan.day, ctrl->CPDisinfectPlan.hour, ctrl->CPDisinfectPlan.min);
			break;
		case SETID_FILTERCHECK_MODE:
			CentralPanelSerial::getInstance()->uploadFilterCheckMode(ctrl->CPFilterCheckMode);
			break;
		case SETID_DISINFECT_DELAY_TIME:
			CentralPanelSerial::getInstance()->uploadDisinfectDelayTime(ctrl->CPDisinfectDelayTime);
			break;
		case SETID_FILTER_RINSE_FIXTIMES:
			CentralPanelSerial::getInstance()->uploadFilterRinseFixtimes(Controller::getInstance()->fix_times_index, ctrl->CPFilterRinseFixTimes[Controller::getInstance()->fix_times_index].day, ctrl->CPFilterRinseFixTimes[Controller::getInstance()->fix_times_index].hour, ctrl->CPFilterRinseFixTimes[Controller::getInstance()->fix_times_index].min);
			break;
		case SETID_MEMBRANE_RINSE_FIXTIMES:
			CentralPanelSerial::getInstance()->uploadMembraneRinseFixtimes(Controller::getInstance()->membrane_fix_times_index, ctrl->CPMembraneRinseFixTimes[Controller::getInstance()->membrane_fix_times_index].day, ctrl->CPMembraneRinseFixTimes[Controller::getInstance()->membrane_fix_times_index].hour, ctrl->CPMembraneRinseFixTimes[Controller::getInstance()->membrane_fix_times_index].min);
			break;
		case SETID_DISINFECT_FIXTIMES:
			CentralPanelSerial::getInstance()->uploadDisinfectFixtimes(Controller::getInstance()->disinfect_fix_times_index, ctrl->CPDisinfectFixTimes[Controller::getInstance()->disinfect_fix_times_index].day, ctrl->CPDisinfectFixTimes[Controller::getInstance()->disinfect_fix_times_index].hour, ctrl->CPDisinfectFixTimes[Controller::getInstance()->disinfect_fix_times_index].min);
			break;
		case SETID_LAST:
			CentralPanelSerial::getInstance()->uploadSettinsFinished();
			break;
		default:
			break;
	}

	return 0;
}

ActionQRCodeScanned::ActionQRCodeScanned(const char* code, int length) : Action(AID_CP_QRCODE)
{
	QRCode = NULL;
	size = 0;

	if(code && length)
	{
		QRCode = new char[length+1];
		if(QRCode)
		{
			memcpy(QRCode, code, length);
			size = length;
			QRCode[length] = '\0';
		}
	}
}

ActionQRCodeScanned::~ActionQRCodeScanned()
{
	if(QRCode)
		delete[] QRCode;
	size = 0;
}

int ActionQRCodeScanned::action()
{
	ActionBase::action();
	if(CentralPanelSerial::getInstance())
		CentralPanelSerial::getInstance()->uploadQRCode(QRCode, size);
	return 0;
}

ActionICCardWrite::ActionICCardWrite(Controller* ctrl, int ballance): Action(AID_CR_WRITECARD, ctrl)
{
	this->ballance = ballance;
}

int ActionICCardWrite::action()
{
	ActionBase::action();

	char cardId[4];
	cardId[0] = (ctrl->CPUserInfo.icCardId      ) & 0xFF;
	cardId[1] = (ctrl->CPUserInfo.icCardId >>  8) & 0xFF;
	cardId[2] = (ctrl->CPUserInfo.icCardId >> 16) & 0xFF;
	cardId[3] = (ctrl->CPUserInfo.icCardId >> 24) & 0xFF;
	char data[16] = {0};
	ICCARD_DATA* iccard = (ICCARD_DATA*)data;
	iccard->version = 0x01;
	iccard->type = ctrl->CPUserInfo.cardType;
	iccard->region = ctrl->CPUserInfo.regionCode;
	iccard->value = ballance;
	iccard->crc = 0;
	if((iccard->type & 0x7F) == 0x03) // VIP card
	{
		iccard->vyear = ctrl->CPUserInfo.vyear;
		iccard->vday = ctrl->CPUserInfo.vday;
	}
	iccard_encode(iccard, cardId);
	if(ICCardReaderSerial::getInstance())
		ICCardReaderSerial::getInstance()->writeBlock(iccard_get_block_num(), data);
	return 0;
}

ActionFirmwareUpgradeStatus::ActionFirmwareUpgradeStatus(int status): Action(AID_CP_FIRMWAREUPDATESTATUS)
{
	this->status = status;
}

int ActionFirmwareUpgradeStatus::action()
{
	ActionBase::action();

	return CentralPanelSerial::getInstance()->uploadFirmwareInstallStatus(status);
}

ActionOperateLogStatus::ActionOperateLogStatus(int status): Action(AID_CP_OPERATIONLOGSTATUS)
{
	this->status = status;
}

int ActionOperateLogStatus::action()
{
	ActionBase::action();

	return CentralPanelSerial::getInstance()->uploadOperationLogStatus(status);
}

ActionOperateLog::ActionOperateLog(uint32_t tm, uint32_t flag, uint32_t p1, uint32_t p2): Action(AID_CP_OPERATIONLOG)
{
	time = tm;
	module = flag;
	param1 = p1;
	param2 = p2;
}

int ActionOperateLog::action()
{
	ActionBase::action();

	return CentralPanelSerial::getInstance()->uploadOperationLog(time, module, param1, param2);
}

ActionProductTestStatus::ActionProductTestStatus(int status): Action(AID_CP_PRODUCTTESTSTATUS)
{
	this->status = status;
}

int ActionProductTestStatus::action()
{
	ActionBase::action();

	return CentralPanelSerial::getInstance()->uploadProductTestStatus(status);
}

ActionDisinfectStatus::ActionDisinfectStatus(int status): Action(AID_CP_DISINFECTSTATUS)
{
	this->status = status;
}

int ActionDisinfectStatus::action()
{
	ActionBase::action();

	return CentralPanelSerial::getInstance()->uploadDisinfectStatus(status);
}

ActionWaterStatus::ActionWaterStatus(int status): Action(AID_CP_WATERSTATUS)
{
	this->status = status;
}

int ActionWaterStatus::action()
{
	ActionBase::action();

	return CentralPanelSerial::getInstance()->uploadWaterStatus(status);
}

ActionCountDown::ActionCountDown(int sec): Action(AID_CP_COUNTDOWN)
{
    remainSeconds = sec;
}

ActionCountDown::~ActionCountDown()
{

}

int ActionCountDown::action()
{
	ActionBase::action();

	return CentralPanelSerial::getInstance()->uploadCountdown(remainSeconds);
}

ActionConfigParam::ActionConfigParam(int kind, Controller* pCtrl) : Action(AID_CP_CONFIGPARAMETER, pCtrl)
{
    this->kind = kind;
}

ActionConfigParam::~ActionConfigParam()
{

}

int ActionConfigParam::action()
{
    ActionBase::action();

    if(kind == 0)
        return CentralPanelSerial::getInstance()->uploadConfigParamSecret(ctrl->configParamSecret);
    else if(kind == 1)
        return CentralPanelSerial::getInstance()->uploadConfigParam(ctrl->configParamHeatPreservation,
                ctrl->configParamQRScanner, ctrl->configParamDustHelmet, ctrl->configParamICCardReader, ctrl->configParamSK_II);
    else
    {
        // error
    }

    return 0;
}

// ActionSteeringAngle::ActionSteeringAngle(int angle): Action(AID_CP_STEERING_ANGLE)
// {
//     this->angle = angle;
// }

// ActionSteeringAngle::~ActionSteeringAngle()
// {

// }

// int ActionSteeringAngle::action()
// {
// 	ActionBase::action();

// 	return CentralPanelSerial::getInstance()->uploadSteeringAngle(angle);
// }

