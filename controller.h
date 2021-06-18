#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__
#define CP_PULSE_PER_LITRE 0
#define CP_HOT_PULSE_PER_LITRE 1
#define CP_COLD_PULSE_PER_LITRE 2
#define CP_ICE_PULSE_PER_LITRE 3
#define CP_PULSE_PER_LITRE_5 4
#define CP_PULSE_PER_LITRE_6 5

#define DEVICE_TYPE_SK_I                        0x00000001
#define DEVICE_TYPE_SK_II                       0x00000002
#define DEVICE_TYPE_SK_III                      0x00000003
#define DEVICE_TYPE_MASK                        0x000000FF

#define DEVICE_TYPE_RESTRICT_HOTWATER           0x00000100//command:147 --- like school
#define DEVICE_TYPE_KIDSAFETY_ENABLED           0x00000200
#define DEVICE_TYPE_HOLD_TO_UNLOCK_HOTWATER     0x00000400//command:149
#define DEVICE_TYPE_PROPERTY_MASK               0xFFFFFF00

#define DEVICE_RINSE_BACKSUP_SIZES              (5)
#define DEVICE_RINSE_MAX_NUM                    (21)

class Controller
{
public:
	Controller();
	virtual ~Controller();

	static Controller* getInstance();

	virtual int boot();
	virtual int shedule_loop(time_t seconds);
	virtual int action_loop();

    int test();

	virtual int startAppAccountEjectCountdown(int sec);
	virtual int stopAppAccountEjectCountdown();

	virtual int checkAccountForColdWaterSelling();
	virtual int checkAccountForHotWaterSelling();
    virtual int checkAccountForIceWaterSelling();

	virtual int updateAccountInfo(bool bfinished=false);

	virtual int setMaxLoopCountOfFilterRinse(int count);

    virtual bool isSKIDevice() { return (vodasDeviceType&DEVICE_TYPE_MASK)==DEVICE_TYPE_SK_I?true:false; }
    virtual bool isSKIIDevice() { return (vodasDeviceType&DEVICE_TYPE_MASK)==DEVICE_TYPE_SK_II?true:false; }
    virtual bool isSKIIIDevice() { return (vodasDeviceType&DEVICE_TYPE_MASK)==DEVICE_TYPE_SK_III?true:false; }
    virtual bool isRestrictHotWaterDevice() { return vodasDeviceType&DEVICE_TYPE_RESTRICT_HOTWATER?true:false; }
    virtual bool isKidSafetyEnabledDevice() { return vodasDeviceType&DEVICE_TYPE_KIDSAFETY_ENABLED?true:false; }
    virtual bool isHoldToUnlockHotWaterDevice() { return vodasDeviceType&DEVICE_TYPE_HOLD_TO_UNLOCK_HOTWATER?true:false; }

	// APIs for Central Panel
	virtual int setFilterRinseFixTime(int day, int hour, int min);
	virtual int rebootCentralPanel();
	virtual int setHeatTemperature(int temp);
	virtual int setUnitPrice(int hot, int cold, int freeze);
	virtual int setWaterLimit(int litre);
	virtual int setDelayTime(int min);
	virtual int setWaterCurrent(int litre);
	virtual int setFilterProduceDate(int year, int month, int day);
	virtual int setFilterRinseDur(int backmin, int fastmin);
	virtual int setChargeWay(int model, int unit);
	virtual int setMembraneRinseDur(int min);
	virtual int setServiceTime(int day, int start[4], int end[4]);
	virtual int setFilterShelflife(int month);
	virtual int setCompensationTemp(int offset, int temp);
	virtual int setHeaterWorkTime(int day, int start[4], int end[4]);
	virtual int setDisinfectDur(int min);
	virtual int setPulsePerLitre(int totalpulse, int hotpulse, int coldpulse, int icepulse, int pulse5, int pulse6);
	virtual int setFilterCheckMode(int mode);
	virtual int setDisinfectDelayTime(int min);
	virtual int setSysTime(int y, int m, int d, int h, int min, int s);
	virtual int setUserCardState(int state);
	virtual int setAppBalance(int cents);
	virtual int setSwichLock(int bUnlock); // set back door is locked or not
	virtual int enableWaterSwitch(int type, int bDisable); // enable/disable water switch to be opened
	virtual int openFeedWaterSwitch(int type, int bOff); // open/close feed water switch directly
	virtual int setDrainFixTime(int day, int hour, int min);
	virtual int setFilterRinsePlan(int y, int m, int d, int h, int min);
	virtual int setDrainPlan(int y, int m, int d, int h, int min);
	virtual int getVersion();
	virtual int getConfiguration();
	virtual int setDeviceId(const char* deviceId);
	virtual int enableBLEDebug(int bOn);
	virtual int setMembraneRinseFixTime(int day, int hour, int min);	
    virtual int setMembraneRinsePlan(int y, int m, int d, int h, int min);
	virtual int drain();
	virtual int membraneRinse();
	virtual int filterRinse();
	virtual int getDeviceId();
	virtual int wechatAppPay(int cents, int ml);
	virtual int setCentralPanelVer(const char* ver);
	virtual int writeDLFirmware(int offset, const char* data, int size);
	virtual int installFirmware(unsigned int hashcode);
	virtual int getLogFile();
	virtual int startProdTest();
	virtual int setDisinfectFixTime(int day, int hour, int min);
	virtual int setDisinfectPlan(int y, int m, int d, int h, int min);
	virtual int disinfect();
	virtual int lockUsercard(int card, int bLocked);
	virtual int functionCancel(int funcType);
	virtual int getStatus();
	virtual int setICCardBalance(int cardId, int balance);
    virtual int getConfigParam(int kind);
    virtual int setConfigSecret(char* secret);
    virtual int setConfigParam(int bHP, int bQR, int bDH, int bIC, int bSKII);
    virtual int setICCardRegion(int cardId, int region, int cardType);
	virtual int response(int cmd, int actId, int result);
    virtual int setFilterRinseFixTimes(int index, int day, int hour, int min);
    virtual int setMembraneRinseFixTimes(int index, int day, int hour, int min);
    virtual int setDisinfectFixTimes(int index, int day, int hour, int min);	
    virtual int getWarnings(void);
	virtual int getMD5Value(void);
	virtual int getValue_1041(char* order_1041);
	virtual int getValue_1044(char* order_1044);
	virtual int getValue_1062(char* order_1062);
	virtual int getValue_1063(char* order_1063);
	virtual int getValue_1064(char* order_1064);

	// ~

	// APIs for water heater
	virtual int handShakeResponse(char v1, char v2, char v3);
	virtual int startHeatingResponse(bool bOn);
	virtual int setBoilTempResponse(char temp, char offset);
	virtual int startRinseResponse(bool bOn);
	virtual int startDrainResponse(bool bOn);
	virtual int setWarnTempResponse(char temp);
	virtual int startFeedbackPeriodicallyResponse(bool bOn, char sec);
	virtual int getStatusResponse(char status, char temp, char level, bool bHeat, bool bWater);
	virtual int completeNotify(char type);
	virtual int warningNotify(char index);
	virtual int setRinseDurResponse(char times);
	virtual int setDrainDurResponse(char minS, char minE);
	virtual int setTempRestrictResponse(char temp);
	// ~

	// APIs for IC card reader
	virtual int CRHandShakeResponse(char v1, char v2, char v3);
	virtual int CREnableReaderResponse(bool bOn);
	virtual int CRICCardDetected(int cardId, char status);
	virtual int CRSetSectorKeyResponse(char secId, char type, char key[6]);
	virtual int CRWriteBlockResponse(char status);
	virtual int CRReadBlockResponse(char status, char data[16]);
	// ~

	// APIs for QR code scanner
	virtual int QRCodeScanned(const char* code, int length);
	// ~

    virtual int setDeviceType(unsigned int type, bool bOnlyType=true);
    virtual int setDeviceRestrictHotWater(int bRestrict);
    virtual int setKidSafety(int bKidSafety);

    virtual int  GetCurRinseFixTimesindex();
    virtual void SetCurRinseFixTimesindex(int index);
    virtual int  GetCurMembraneFixTimesindex();
    virtual void SetCurMembraneFixTimesindex(int index);
    virtual int  GetCurDisinfectFixTimesindex();
    virtual void SetCurDisinfectFixTimesindex(int index);

    virtual int  IsHasFixTimesValidIndex(void);
    virtual int  IsHasFollowingFixTimesValidIndex(void);//Invalid default current index
    virtual int  GetFixTimesIndex(void);
    virtual void SetFixTimesIndex(int index);

    virtual int  IsHasMembraneFixTimesValidIndex(void);
    virtual int  IsHasFollowingMembraneFixTimesValidIndex(void);//Invalid default current index
    virtual int  GetMembraneFixTimesIndex(void);
    virtual void SetMembraneFixTimesIndex(int index);

    virtual int  IsHasDisinfectFixTimesValidIndex(void);
    virtual int  IsHasFollowingDisinfectFixTimesValidIndex(void);//Invalid default current index
    virtual int  GetDisinfectFixTimesIndex(void);
    virtual void SetDisinfectFixTimesIndex(int index);

    virtual int  GetCPUserInfoIcCardId(void);
    virtual int  GetCPUserInfoCardMovingFlag(void);

public:
	virtual int currentFilterWaterLitres() { return CPFilterWaterCur; }
	virtual int remainFilterWorkableDays() { return CPRemainWorkableDays; }
	virtual int currentWaterHeaterStatus() { return WHStatus; }
	virtual int currentBoilingTemperature() { return WHBoilingTemp; }

private:
	virtual bool checkInService();
	virtual bool checkWHWorkTime();
	virtual int waterHeaterConnected(); // executed after handshake successfull
	virtual int calcFilterExpiredNS(bool bSetRemainDays);
	virtual int checkPlanActions(time_t seconds);
	virtual int startDisinfect();
	virtual int suspendDisinfect();
	virtual int resumeDisinfect();
	virtual int setInvasionDetectedFlag(bool bDetected);

private:
	friend class StateMachine;
	friend class DigitalSwitches;
	friend class DebugSerial;
	friend class PersistentStorage;

	char WHVer[3];
	int WHBoilingTemp, WHTempOffset; // altitude compensation
	int WHWarningTempOffset;
	bool WHFBOnOff; int WHFBDur; // feedback
	char WHStatus; /**< 0: heater off; 1: heater on; 2: rinse; 3: alarm; 4: drain;*/
	char WHCurTemp;
	char WHLevel; /**< 0: below low water; 1: low water; 2: high water; 3: over high water;*/
	bool WHHeater;
	bool WHWater;
	bool WHInService; // work state; true: heating enabled; false: not enabled;
	bool SYSInService; // System state; true: system in service; false: system out of service;

private:
	friend class Action;
	friend class ActionConfigInfo;
	friend class ActionICCardWrite;
	friend class WarningManager;
	friend class WaterHeaterSerial;
	// Settings
	//char CPTemperature; // use WHBoilingTemp;
	int CPHotPrice, CPColdPrice, CPIcePrice; // cents per Litre
	int CPFilterWaterMax; // Litre
	int CPPlanDelayTime; // Minute
	int CPDisinfectDelayTime; // Minute
	int CPFilterWaterCur; // Litre
	struct {
		int year, month, day;
	}CPFilterProductionDate;
	int CPFilterBackRinseDur, CPFilterFastRinseDur; // minute
	int CPMembraneRinseDur; // minute
	int CPDrainMinS, CPDrainMinE; // minute
	int CPChargeMode; // 
	int CPChargeUnit;
	struct {
		short start[4];
		short end[4];
	}CPServiceTime[7], CPHeaterWorkTime[7];
	int CPFilterShelfLife; // month
	time_t CPFilterExPiredNS;
	int CPRemainWorkableDays;
	bool CPFilterExpiredInformed;
	int CPFilterCheckMode; // 0: Both; 1: Flowmeter; 2: Shelflife;
	struct {
		int startTemp;
		int compensation;
	}CPCompensation;
	int CPDisinfectDur; // Minute
	int CPDisinfectTemp;
	int CPPulsePerLitreArr[6];
	//int CPPulsePerLitre;
	//int CPHotPulsePerLitre;
	//int CPColdPulsePerLitre;
	//int CPIcePulsePerLitre;
	//int CPPulsePerLitre5;
	//int CPPulsePerLitre6;
	// ~Settings
	struct {
		int icCardId;
		int accountState; // 0x00: not active; 0x01: activated; 
		int balance; // cents
		int hotWaterPulse;
		int coldWaterPulse;
		int waterLimitBalance; // for wechat only
		int waterLimitMilliLitre; // for wechat only
		char accountType; // 0: IC Card; 1: App; 2: WeChat pay
		char cardType; //1: normal; 2: corp card; 3: VIP card;
		int  regionCode;
		char vyear, vday;
        char card_moving_flag;
	}CPUserInfo;
	bool CPDoorOpenForbidden;// true: open the door will trigger INVASION warning; false: allowed to open;
	int CPInvasionDetected; // 0: no invasion detected; 1: invasion detected;
	char CPDeviceId[33]; // 32 bytes device id and one byte '\0'
	bool CPBLEDebugEnabled;
	int CPDrainDur; // Minute
	char CPAppEjectCountdown; // seconds
	bool CPHotWaterSwitchEnabled;
	bool CPColdWaterSwitchEnabled;
	struct {
		char valid;
		char day;
		char hour;
		char min;
	}CPDrainFixTime,CPFilterRinseFixTime,CPMembraneRinseFixTime, CPDisinfectFixTime;
	struct {
        char index;
		char valid;
		char day;
		char hour;
		char min;
	}CPFilterRinseFixTimes[DEVICE_RINSE_MAX_NUM],CPMembraneRinseFixTimes[DEVICE_RINSE_MAX_NUM], CPDisinfectFixTimes[DEVICE_RINSE_MAX_NUM];
	struct {
		int  year;
		char month;
		char day;
		char hour;
		char min;
		char valid;
		time_t plan_time;
	}CPFilterRinsePlan, CPDrainPlan, CPMembraneRinsePlan, CPDisinfectPlan;
	time_t CPFilterRinseLast, CPDrainLast, CPMembraneRinseLast, CPDisinfectLast,CPDrainStartTime;
	time_t CPDisinfectStartTime;
	int CPDisinfectCostSec;
	bool CPIsAlive;
	bool CPDisinfectStarted;

    int CurRinseFixTimesindex;//CPFilterRinseFixTime[DEVICE_RINSE_MAX_NUM] of index
    int CurMembraneFixTimesindex;//CPMembraneRinseFixTime[DEVICE_RINSE_MAX_NUM] of index
    int CurDisinfectFixTimesindex;//CPDisinfectFixTime[DEVICE_RINSE_MAX_NUM] of index
    
public:
	Timeout CPAppEjectTimeout;
	void appEjectTimeoutFun();
	bool appEjectTimeoutTriggered;
	Timeout CPRebootTimeout;
	void rebootTimeoutFun();
	int rebootDeviceType; // 1: Central Panel; 2: Mbed;
	Timeout CPDisinfectTimeout;
	void disinfectTimeoutFun();
	void disinfectFinished();
	bool disinfectTimeoutTriggered;
	Timeout CPDisinfectMembraneTimeout;
	void disinfectMembraneTimeoutFun();
	bool disinfectMembraneTimeoutTriggered;
	void timeoutProcess();
	// production test
	int productionTestStep;
	bool productionTestTimeoutTriggered;
	Timeout ProductionTestTimeout;
	void startProductionTestTimeout(int sec);
	void productionTestTimeoutFun(void);
	void productionTestProcess();
	void exitProdTest();

	int getValveState();
	int updateOutletValveMark(unsigned char status);

    int fix_times_index;
    int membrane_fix_times_index;
    int disinfect_fix_times_index;

private:
	char CRVer[3];
	bool CREnabled;
	int CRCardStatus; // 1: legal; 2: illegeal; 3: card removed;
	int CRCheckTime;
	bool CRIsAlive;

	// avoid issue when insert/remove in very short duration(200ms)
	bool bICCardInserted;
	Timeout ICCardInsertedTimeout;
	void ICCardInsertedTimeoutFun();
	int ICCardInsertedProc();

private:
	static Controller ctrler;
	int hotWaterFlowPulse;
	int coldWaterFlowPulse;
    int iceWaterFlowPulse;
	int feedWaterFlowPulse;

	int filterRinseMaxCount; //redo times when start filter rinse

    int UVPowerOffDelaySec;
    int DustHelmetCloseDelaySec;
    int AppPayEjectDelaySec;
    int AppPayEjectDelaySec2;
    int AppPayEjectNoActionDelaySec;
    int WechatPayEjectDelaySec;
    int WechatPayNoActionEjectDelaySec;

    int waterReleaseTimes;   

public:
    unsigned int vodasDeviceType;

    char configParamSecret[64];
    char configParamHeatPreservation;
    char configParamQRScanner;
    char configParamDustHelmet;
    char configParamICCardReader;
    char configParamSK_II;

	unsigned char valveStatus; //Store valve condition
	unsigned char preValveStatus;

	time_t CPInvasionDetectedClosTime; //Invasion Detected Clos Time
	int CPSteeringGearTime;
};

#endif //__CONTROLLER_H__
