#ifndef __WARNING_MANAGER_H__

class WarningManager
{
public:
	static WarningManager* getInstance();

	virtual int warning_loop();

	virtual unsigned int warningFlags();
	virtual bool isOutOfServiceWarningTriggered();
	virtual bool isColdWaterForbidden();
	virtual bool isHotWaterForbidden();

	typedef enum {
		WARNING_TYPE_WATER_LEAK = 2040,				//漏水告警
		WARNING_TYPE_INVASION = 2042,				//非法入侵
		WARNING_TYPE_FILTER_EXPIRED = 2043,			//滤芯到期
		WARNING_TYPE_COLD_WATER_ERR = 2044,			//冷水故障
		WARNING_TYPE_HOT_WATER_ERR = 2045,			//热水故障
		WARNING_TYPE_WH_ELECTRIC_LEAK = 2046,		//热水器漏电
		WARNING_TYPE_WH_HIGH_TEMPERATURE = 2047, 	//热水器高温 // temperature doesn't go down with feeding cold water 
		WARNING_TYPE_WH_LOW_LEVEL_ERR = 2048,		//热水器下电机故障
		WARNING_TYPE_WH_HIGH_LEVEL_ERR = 2049,		//热水器上电机故障
		WARNING_TYPE_WH_LEVEL_ERR = 2050,			//热水器上下水位故障
		WARNING_TYPE_WH_OVER_FLOW = 2051,			//热水器溢流故障
		WARNING_TYPE_631_VALVE_ERR = 2052,			//631阀故障 //没有逻辑触发
		WARNING_TYPE_UV_DEVICE_ERR = 2053,			//紫外线故障
		WARNING_TYPE_WATER_RELEASE_ERR = 2054,		//连续出水故障 //没有逻辑触发
		WARNING_TYPE_ELECTRIC_LEAK = 2055,			//漏电 //没有逻辑触发
		WARNING_TYPE_LOW_BATTERY = 2056,			//电池电量低 //没有逻辑触发
		WARNING_TYPE_COLD_VALVE_ERR = 2060,			//冷水阀故障
		WARNING_TYPE_HOT_VALVE_ERR = 2061,			//热水阀故障
		WARNING_TYPE_FEED_VALVE_ERR = 2063,			//总进水阀故障 //没有逻辑触发
		WARNING_TYPE_WH_OVER_TEMPERATURE = 2064,	//热水器超高温
		WARNING_TYPE_WH_TEMPERATURE_ERR = 2065,		//热水器温感故障 //加热板没有发送该告警
		WARNING_TYPE_INFRARED_SENSOR_A_ERR = 2067,	//红外(上)故障 //没有逻辑触发
		WARNING_TYPE_INFRARED_SENSOR_B_ERR = 2068,	//红外(下)故障 //没有逻辑触发
		WARNING_TYPE_QR_CODE_SCANNER_ERR = 2069,	//读码器故障 //没有逻辑触发
		WARNING_TYPE_IC_CARD_READER_ERR = 2070,		//刷卡器故障
		WARNING_TYPE_DUST_HELMET_ERR = 2071,		//防尘罩故障
		WARNING_TYPE_MBED_REBOOT = 2072				//下位机异常重启 //没有逻辑触发
	}EWarningType;

	virtual int triggerWarning(EWarningType warningType);
	virtual int resolveWarning(EWarningType warningType);
    virtual int getWarnings(int warn[27]);

private:
	WarningManager();
	virtual ~WarningManager();

	static WarningManager* pThis;

private:
	static const int WARNING_TYPE_COUNT = 27;
	typedef struct _warningRecord
	{
		//EWarningType type;
		char flag;
		char reserved;
		char lastTriggerTime;
		char lastNotifyTime;
	}SWarningRecord;
	SWarningRecord warningArray[WARNING_TYPE_COUNT];
	unsigned int warningIndexFlags;

	bool bOutOfService;

private:
	int getWarningTypeIndex(EWarningType warningType);
	EWarningType getWerningTypeFromIndex(int index);
	bool isRetriggeredWarningType(EWarningType warningType);
	int warningAction(EWarningType type, bool bOn);
	unsigned int getOutOfServiceWarningMask();
	unsigned int getColdWaterForbiddenWarningMask();
	unsigned int getHotWaterForbiddenWarningMask();
};

#endif
