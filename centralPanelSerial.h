#ifndef __CENTRALPANELSERIAL_H__
#define __CENTRALPANELSERIAL_H__

class CentralPanelSerial: public CommandProcess
{
public:
	CentralPanelSerial(RawSerial* ps=NULL);
	virtual ~CentralPanelSerial();

	static CentralPanelSerial* getInstance();

	virtual int process();

	virtual int getSerialType() {
		return SERIALTYPE_CENTRAL_PANEL;
	}

	//virtual int getRingBufferSize() {
	//	return 256;
	//}

	virtual int sendFormatMsg(const char* fmt, ...);
	virtual int uploadAppWaterInfo(int coldcost, int coldcap, int hotcost, int hotcap, int bOn);
	virtual int uploadCardBalance(int cardId, int balance, int region, int bLocked, int cardType, int index);
	virtual int uploadCardWaterInfo(int coldcost, int coldcap, int hotcost, int hotcap, int balance, int bOn);
	virtual int uploadWaterQuality(int type, int value);
	virtual int uploadWarningType(int type, int bOn);
	virtual int uploadFilterInfo(int curton, int curlitre, int limitton, int limitlitre);
	virtual int uploadFilterRinseStartEvent();
	virtual int uploadFilterRinseEndEvent();
	virtual int uploadDrainingEvent();
	virtual int uploadDrainEndEvent();
	virtual int uploadWaterTankInfo(int temp, int cur, int limit);
	virtual int uploadWaterStatus(int status);
	virtual int uploadCountdown(int cnt);
	virtual int uploadFilterWorkableDays(int days);
	virtual int uploadVersion(int major, int minor, int build);
	virtual int uploadMembraneRinseStartEvent();
	virtual int uploadMembraneRinseEndEvent();
	virtual int uploadDeviceId(const char* devId);
	virtual int uploadQRCode(const char* code, int length);
	virtual int uploadInfraredSensorWorksEvent();
	virtual int uploadInfraredSensorIdleEvent();
	virtual int uploadFirmwareInstallStatus(int status);
	virtual int uploadOperationLogStatus(int status);
	virtual int uploadOperationLog(int tm, int mod, int p1, int p2);
	virtual int uploadProductTestStatus(int status);
	virtual int uploadProductTestCountdown(int cnt);
	virtual int uploadDisinfectStatus(int status);
	virtual int uploadRequestSyncSysTime();
	virtual int uploadValveState(int B3,int cold,int hot,int ice);
    virtual int uploadConfigParamSecret(const char* secret);
    virtual int uploadConfigParam(int bHP, int bQR, int bDH, int bIC, int bSKII);
    virtual int uploadWarnings(void);
	virtual int uploadSteeringAngle(int angle);
	virtual int uploadMD5Value(const char* md5Value);
	// reporting settings
	virtual int uploadTemperature(int temp);
	virtual int uploadWaterPrice(int hot, int cold, int freeze);
	virtual int uploadFilterLimit(int litre);
	virtual int uploadPlanDelayTime(int min);
	virtual int uploadFilterCur(int litre);
	virtual int uploadFilterProduceDate(int y, int m, int d);
	virtual int uploadFilterRinseDur(int backmin, int fastmin);
	virtual int uploadChargeWay(int model, int unit);
	virtual int uploadMembraneRinseDur(int min);
	virtual int uploadServiceTime(int day, short start[4], short end[4]);
	virtual int uploadFilterShelflive(int month);
	virtual int uploadCompensationTemp(int start, int comp);
	virtual int uploadHeaterWorkTime(int day, short start[4], short end[4]);
	virtual int uploadDisinfectDur(int min);
	virtual int uploadPulsePerLitre(int totalpulse, int hotpulse, int coldpulse, int icepulse, int pulse5, int pulse6);
	virtual int uploadFilterRinseFixtime(int day, int hour, int min);
	virtual int uploadDrainFixtime(int day, int hour, int min);
	virtual int uploadMembraneRinseFixtime(int day, int hour, int min);
	virtual int uploadDisinfectFixtime(int day, int hour, int min);
	virtual int uploadFilterRinsePlan(int y, int m, int d, int h, int min);
	virtual int uploadDrainPlan(int y, int m, int d, int h, int min);
	virtual int uploadMembraneRinsePlan(int y, int m, int d, int h, int min);
	virtual int uploadDisinfectPlan(int y, int m, int d, int h, int min);
	virtual int uploadFilterCheckMode(int mode);
	virtual int uploadDisinfectDelayTime(int min);
	virtual int uploadFilterRinseFixtimes(int index, int day, int hour, int min);
	virtual int uploadMembraneRinseFixtimes(int index, int day, int hour, int min);
	virtual int uploadDisinfectFixtimes(int index, int day, int hour, int min);
	virtual int uploadSettinsFinished();

private:
	virtual int popInteger(int& val);
	virtual int popString(char* str, int size);
	int addcheck(const char* c, int size=1);
	char m_chksum;

	unsigned char m_sendId;

	int sendResponse(int cmd, char status, char sendId);

private:
	static CentralPanelSerial* pThis;

	Command* m_pCmd;
	Command* createCmd(int c);

private:
#define DEF_COMMAND_BEGIN(name) \
	class name: public Command \
	{ \
	public: \
		name(CommandProcess* p) : Command(p) { } \
		virtual ~name() { } \
		virtual int parse(); \
		virtual int exec()
#define DEF_COMMAND_END() }

	DEF_COMMAND_BEGIN(CMDFilterRinseFixTime);
		int day, hour, minute;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDRebootCentralPanel);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDSetting);
		virtual int parseSubCmd();
		int subcmd;
		union PRAME{
			struct S1{ int temp; }s1; // set temperature
			struct S2{ int hotprice, coldprice, freezeprice; }s2; // set price
			struct S3{ int ton, litre; }s3; // set limit/current of filte water
			struct S4{ int minutes; }s4; // set plan/disinfect delay time; // set membrane/disinfect rinse duration
			struct S5{ int year, month, day; }s5; // set filter production date
			struct S6{ int model, unit; }s6; // set charge way
			struct S7{ int day, start[4], end[4]; }s7; // set service time
			struct S8{ int month; }s8; // set shelf life of the filetr
			struct S9{ int offset, temp; }s9; // set compensation temperature
			struct Sa{ int totalpulse,hotpulse,coldpulse,icepulse,pulse5,pulse6; }sa; // set pulse per litre
			struct Sb{ int backdur, fastdur; }sb; // set filter rinse duration
			struct Sc{ int day, hour, min; }sc; //  filter rinse/membrane rinse/disinfect fix time/drain        
			struct Sd{ int year, month, day, hour, min; }sd; // filter rinse/membrane rinse/drain/disinfect plan
			struct Se{ int mode; }se; // filter check mode
            struct Sf{ int index, day, hour, min; }sf; // filter rinses/membrane rinses/disinfect fix times  
		}param;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDSetTime);
		int year, month, day, hour, minute, second;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDUserCardState);
		int state, card_id, card_index;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDAppBalance);
		int cents;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDSwitchLock);
		int bUnlocked;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDWaterSwitch);
		int type, bDisable;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDFeedWaterSwitch);
		int switchType, bOff;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDDrainFixtime);
		int day, hour;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDFilterRinsePlan);
		int year, month, day, hour, minute;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDDrainPlan);
		int year, month, day, hour, minute;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDGetVersion);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDGetSettings);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDSetDeviceId);
		char deviceId[32];
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDEnableBLEDebug);
		int bOn;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDWrongCommand);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDMembraneRinseFixTime);
		int day, hour, minute;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDMembraneRinsePlan);
		int year, month, day, hour, minute;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDDrain);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDMembraneRinse);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDFilterRinse);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDGetDeviceId);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDWechatAppPay);
		int price, ml;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDSetCPVersion);
		char ver[16];
		int offset;
	DEF_COMMAND_END();

	//DEF_COMMAND_BEGIN(CMDDownloadFw);
	//	int sec, len, hashcode;
	//	char data[128];
	//	int pos;
	//DEF_COMMAND_END();
	class CMDDownloadFw: public Command
	{
	public:
		CMDDownloadFw(CommandProcess* p);
		virtual ~CMDDownloadFw();

		virtual int parse();
		virtual int exec();

		int sec, len, hashcode;
		char data[512];
		int pos;
	};


	DEF_COMMAND_BEGIN(CMDInstallFw);
		int hashcode;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDRevertFw);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDGetLogFile);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDStartProdTest);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDStopProdTest);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDDisinfectFixTime);
		int day, hour, minute;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDDisinfectPlan);
		int year, month, day, hour, minute;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDDisinfect);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDLockUsercard);
		int cardId;
		int bLocked;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDFunctionCancel);
		int funcType;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDGetStatus);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDSetICCardBalance);
		int cardId;
		int balance;
	DEF_COMMAND_END();

    DEF_COMMAND_BEGIN(CMDGetConfigParam);
        int kind;
    DEF_COMMAND_END();

    DEF_COMMAND_BEGIN(CMDSetConfigParam);
        virtual int parseSubCmd();
        int kind;
        union PARAM
        {
            struct S1{char secret[65];}s1;
            struct S2{int bHeatPreservation, bQRScanner, bDustHelmet, bICReader, bSKI;}s2;
        }param;
    DEF_COMMAND_END();

    DEF_COMMAND_BEGIN(CMDSetCardRegion);
        int cardId, region;
	int cardType;
    DEF_COMMAND_END();

    DEF_COMMAND_BEGIN(CMDSetDeviceType);
        int type;
    DEF_COMMAND_END();

    DEF_COMMAND_BEGIN(CMDSetHotWaterRestrict);
        int bRestrict;
    DEF_COMMAND_END();

    DEF_COMMAND_BEGIN(CMDSetKidSafety);
        int bKidSafety;
    DEF_COMMAND_END();

    DEF_COMMAND_BEGIN(CMDClearSettings);
    DEF_COMMAND_END();

    DEF_COMMAND_BEGIN(CMDGetWarnings);
        int warn[27];
    DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDResponse);
		int cmd, result;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDFilterRinseFixTimes);
		int index, day, hour, minute;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDMembraneRinseFixTimes);
		int index, day, hour, minute;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDDisinfectFixTimes);
		int index, day, hour, minute;
	DEF_COMMAND_END();

#ifdef TDS_CONTROL_SWITCH
	DEF_COMMAND_BEGIN(CMDSetWantedTDS);
        int bOn, tds;
	DEF_COMMAND_END();
	
	DEF_COMMAND_BEGIN(CMDRawWaterTDS);
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDMixedWaterTDS);
	DEF_COMMAND_END();

#endif

	DEF_COMMAND_BEGIN(CMDGetMD5Value);
	DEF_COMMAND_END();

#undef DEF_COMMAND_BEGIN
#undef DEF_COMMAND_END
};

#endif
