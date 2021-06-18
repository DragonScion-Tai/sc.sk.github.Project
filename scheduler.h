#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

class Controller;
class ActionBase
{
public:
	ActionBase();
	virtual ~ActionBase();

	enum _STATUS {
		PENDING = 0, /**< action is pending, ready to be executed*/
		EXECUTED,    /**< action is executed, wait for response*/
		FINISHED     /**< action is finised, to be removed*/
	} status;

	virtual int getActionId() = 0;
	virtual int action();
    virtual const char* getActionName() = 0;

	ActionBase* next;
	time_t startTime;
	int sendCount;

	static int append(ActionBase* newItem);
	static int pop();
	static int removeActionIfHas(int aid);
	static ActionBase* header();
};

class Action: public ActionBase
{
public:
	enum _ACTION_ID
	{
		AID_WH_HANDSHAKE = 0,
		AID_WH_STARTHEATING,
		AID_WH_BOILINGTEMP,
		AID_WH_RINSE,
		AID_WH_DRAIN,
		AID_WH_WARNINGTEMP,
		AID_WH_SOFTRESET,
		AID_WH_FEEDBACK,
		AID_WH_GETSTATUS,
		AID_WH_SET_RINSE_DUR,
		AID_WH_SET_DRAIN_DUR,
		AID_WH_SET_TEMP_RESTRICT,
		AID_WH_LAST,
		//
		AID_CP_WATERINFO,
		AID_CP_CARDINFO,
		AID_CP_CARDWATERINFO,
		AID_CP_WATERQUALITYINFO,
		AID_CP_WARNINGINFO,
		AID_CP_FILTERINFO,
		AID_CP_CONFIGINFO,
		AID_CP_FILTERRINSESTART,
		AID_CP_FILTERRINSEEND,
		AID_CP_DRAINING,
		AID_CP_DRAINEND,
		AID_CP_WATERTANKSTATUS,
		AID_CP_WATERSTATUS, // 0: ready for drinking; 1: heating
		AID_CP_COUNTDOWN, // time countdown for App account eject 
		AID_CP_FILTERWORKABLEDAYS,
		AID_CP_VERSION,
		AID_CP_MEMBRANERINSESTART,
		AID_CP_MEMBRANERINSEEND,
		AID_CP_DEVICEID,
        AID_CP_WARNINGS,
		AID_CP_QRCODE,
		AID_CP_INFRAREDSENSORWORKS,
		AID_CP_INFRAREDSENSORIDLE,
		AID_CP_FIRMWAREUPDATESTATUS,
		AID_CP_OPERATIONLOGSTATUS,
		AID_CP_OPERATIONLOG,
		AID_CP_PRODUCTTESTSTATUS,
		AID_CP_PRODUCTTESTCOUNTDOWN,
		AID_CP_DISINFECTSTATUS,
		AID_CP_REQUESTSYNCSYSTIME,
		AID_CP_VALVE_STATE,
        AID_CP_CONFIGPARAMETER,
		AID_CP_STEERING_ANGLE,
		AID_CP_LAST,
		//
		AID_CR_HANDSHAKE,
		AID_CR_ENABLEREADER,
		AID_CR_SETSECTORKEY,
		AID_CR_WRITECARD,
		AID_CR_READCARD,
		AID_CR_LAST
	};

	Action(int id, Controller* pCtrl=NULL);
	virtual ~Action();

	virtual int getActionId();
	virtual int action();

    // for debug
    virtual const char* getActionName();

	int actionId;
	Controller* ctrl;
};

class ActionStartHeating: public Action
{
public:
	ActionStartHeating(bool bOn);
	virtual ~ActionStartHeating() { }

	virtual int action();

	bool bOnOff;
};

class ActionSetBoilTemp: public Action
{
public:
	ActionSetBoilTemp(char temp, char offset);
	virtual ~ActionSetBoilTemp() { }

	virtual int action();

	char temp, offset;
};

class ActionSetRinseDur: public Action
{
public:
	ActionSetRinseDur(int min);
	virtual ~ActionSetRinseDur() { }

	virtual int action();

	int duration; // minutes
};

class ActionStartRinse: public Action
{
public:
	ActionStartRinse(bool bOn);
	virtual ~ActionStartRinse() { }

	virtual int action();

	bool bOnOff;
};

class ActionStartDrain: public Action
{
public:
	ActionStartDrain(bool bOn);
	virtual ~ActionStartDrain() { }

	virtual int action();

	bool bOnOff;
	char minutes;
};

class ActionSetWarningTemp: public Action
{
public:
	ActionSetWarningTemp(char temp);
	virtual ~ActionSetWarningTemp() { }

	virtual int action();

	char warningTemp;
};

class ActionStartFeedbackPeriodically: public Action
{
public:
	ActionStartFeedbackPeriodically(bool bOn, char sec);
	virtual ~ActionStartFeedbackPeriodically() { }

	virtual int action();

	bool bOnOff;
	char seconds;
};

class ActionSetTempRestrict: public Action
{
public:
	ActionSetTempRestrict(char temp);
	virtual ~ActionSetTempRestrict() { }

	virtual int action();

	char restrictTemp;
};

//
class ActionWaterInfo: public Action
{
public:
	ActionWaterInfo(int coldcost, int coldcap, int hotcost, int hotcap, int bOn, int balance=-1);
	virtual ~ActionWaterInfo() { }

	virtual int action();

	int coldcost, coldcap, hotcost, hotcap, bOn, balance;
};

class ActionWaterTankStatus: public Action
{
public:
	ActionWaterTankStatus(int curTemp, int curWater, int maxWater);
	virtual ~ActionWaterTankStatus() { }

	virtual int action();

	int curTemp, curWater, maxWater;
};

class ActionCardInfo: public Action
{
public:
	ActionCardInfo(int id, int balance, int region, int flag, int cardType);
	virtual ~ActionCardInfo() { }

	virtual int action();

	int cardId;
	int balance;
	int region;
	int flag;
	int cardType;
    static int index;
};

class ActionWaterQualityInfo: public Action
{
public:
	ActionWaterQualityInfo(int type, int value);
	virtual ~ActionWaterQualityInfo() { }

	virtual int action();

	int type; // 0: PH; 1: TDS;
	int value;
};

class ActionWarningInfo: public Action
{
public:
	ActionWarningInfo(int type, int bOn);
	virtual ~ActionWarningInfo() { }

	virtual int action();

	static int removeActonIfWithSameType(ActionWarningInfo* warning);

	int warningType, bWarning;
};

class ActionFilterInfo: public Action
{
public:
	ActionFilterInfo(int cur, int limit);
	virtual ~ActionFilterInfo() { }

	virtual int action();

	int filterWaterCur, filterWaterLimit;
};

class ActionConfigInfo: public Action
{
public:
	ActionConfigInfo(Controller* pCtrl);
	virtual ~ActionConfigInfo() { }

	virtual int action();

	enum {
		SETID_FIRST = 0,
		SETID_TEMPERATURE = SETID_FIRST,
		SETID_PRICE,
		SETID_FILTER_LIMIT,
		SETIF_PLAN_DELAY,
		SETID_FILTER_CUR,
		SETID_FILTER_PRODUCE_DATE,
		SETID_FILTER_RINSE_DUR,
		SETID_CHARGE_WAY,
		SETID_MEMBRANE_RINSE_DUR,
		SETID_SERVICE_TIME_D1,
		SETID_SERVICE_TIME_D2,
		SETID_SERVICE_TIME_D3,
		SETID_SERVICE_TIME_D4,
		SETID_SERVICE_TIME_D5,
		SETID_SERVICE_TIME_D6,
		SETID_SERVICE_TIME_D7,
		SETID_FILTER_SHELFLIFE,
		SETID_COMPENSATION_TEMP,
		SETID_HEATER_WORK_TIME_D1,
		SETID_HEATER_WORK_TIME_D2,
		SETID_HEATER_WORK_TIME_D3,
		SETID_HEATER_WORK_TIME_D4,
		SETID_HEATER_WORK_TIME_D5,
		SETID_HEATER_WORK_TIME_D6,
		SETID_HEATER_WORK_TIME_D7,
		SETID_DISINFECT_DUR,
		SETID_PULSE_PER_LITRE,
		SETID_FILTER_RINSE_FIXTIME,
		SETID_FILTER_RINSE_PLAN,
		SETID_DRAIN_FIXTIME,
		SETID_DRAIN_PLAN,
		SETID_MEMBRANE_RINSE_FIXTIME,
		SETID_MEMBRANE_RINSE_PLAN,
		SETID_DISINFECT_FIXTIME,
		SETID_DISINFECT_PLAN,
		SETID_FILTERCHECK_MODE,
		SETID_DISINFECT_DELAY_TIME,
		SETID_FILTER_RINSE_FIXTIMES,
		SETID_MEMBRANE_RINSE_FIXTIMES,
		SETID_DISINFECT_FIXTIMES,
		SETID_LAST
	};
	int index;
};

class ActionQRCodeScanned: public Action
{
public:
	ActionQRCodeScanned(const char* code, int length);
	virtual ~ActionQRCodeScanned();

	virtual int action();

private:
	char* QRCode;
	int size;
};

class ActionICCardWrite: public Action
{
public:
	ActionICCardWrite(Controller* ctrl, int ballance);
	virtual ~ActionICCardWrite() { }

	virtual int action();
public:
	int ballance;
};

class ActionFirmwareUpgradeStatus: public Action
{
public:
	ActionFirmwareUpgradeStatus(int status); // 0: start; 1: succeeded; 2: failed;
	virtual ~ActionFirmwareUpgradeStatus() { }

	virtual int action();

private:
	int status;
};

class ActionOperateLogStatus: public Action
{
public:
	ActionOperateLogStatus(int status);
	virtual ~ActionOperateLogStatus() { }

	virtual int action();

	int status;
};

class ActionOperateLog: public Action
{
public:
	ActionOperateLog(uint32_t tm, uint32_t flag, uint32_t p1, uint32_t p2);
	virtual ~ActionOperateLog() { }

	virtual int action();

private:
	uint32_t time, module, param1, param2;
};

class ActionProductTestStatus: public Action
{
public:
	ActionProductTestStatus(int status);
	virtual ~ActionProductTestStatus() { }

	virtual int action();

	int status;
};

class ActionDisinfectStatus: public Action
{
public:
	ActionDisinfectStatus(int status);
	virtual ~ActionDisinfectStatus() { }

	virtual int action();

	int status;
};

class ActionWaterStatus: public Action
{
public:
	ActionWaterStatus(int status);
	virtual ~ActionWaterStatus() { }

	virtual int action();

	int status;
};

class ActionCountDown: public Action
{
public:
    ActionCountDown(int sec);
    virtual ~ActionCountDown();

    virtual int action();

    int remainSeconds;
};

class ActionConfigParam: public Action
{
public:
    ActionConfigParam(int kind, Controller* pCtrl);
    virtual ~ActionConfigParam();

    virtual int action();

    int kind;
};

class ActionSteeringAngle: public Action
{
public:
    ActionSteeringAngle(int angle);
    virtual ~ActionSteeringAngle();

    virtual int action();

    int angle;
};


#endif //__SCHEDULER_H__
