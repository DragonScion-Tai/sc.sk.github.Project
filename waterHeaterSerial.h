#ifndef __WATERHEATERSERIAL_H__
#define __WATERHEATERSERIAL_H__

class WaterHeaterSerial: public CommandProcess
{
public:
	WaterHeaterSerial();
	virtual ~WaterHeaterSerial();

	static WaterHeaterSerial* getInstance();

	virtual int process();

	virtual int getSerialType() {
		return SERIALTYPE_WATER_HEATER;
	}

	// Water Heater APIs
	virtual int handShake();
	virtual int startHeating(bool bOn);
	virtual int setBoilTemp(char temp, char offset);
	virtual int startRinse(bool bOn);
	virtual int startDrain(bool bOn);
	virtual int setWarnTempOffset(char offset);
	virtual int resetWaterHeater();
	virtual int startFeedbackPeriodically(bool bOn, char sec);
	virtual int getStatus();
	virtual int setRinseDuration(int min);
	virtual int setDrainDuration(int minS, int minE);
	virtual int setBoilingTempRestrict(int temp);
	// ~

private:
	char m_chksum;
	void addcheck(char c);
	int  getchecksum();
	char checksum(const char*d, int s);

private:
	static WaterHeaterSerial* pThis;

	unsigned char m_SendId;

	Command* m_pCmd;
	Command* createCmd(int c);

private:
	static const int WATER_HEATER_BOILING_TEMP_OFFSET = 7;

private:
	class RSPHandShake: public Command
	{
	public:
		RSPHandShake(CommandProcess* p);
		virtual ~RSPHandShake();

		virtual int parse();
		virtual int exec();
	private:
		char v1, v2, v3;
	};

	class RSPStartHeating: public Command
	{
	public:
		RSPStartHeating(CommandProcess* p);
		virtual ~RSPStartHeating();

		virtual int parse();
		virtual int exec();
	private:
		char onOff;
	};

	class RSPSetBoilTemp: public Command
	{
	public:
		RSPSetBoilTemp(CommandProcess* p);
		virtual ~RSPSetBoilTemp();

		virtual int parse();
		virtual int exec();
	private:
		char temp, offset;
	};

	class RSPStartRinse: public Command
	{
	public:
		RSPStartRinse(CommandProcess* p);
		virtual ~RSPStartRinse();

		virtual int parse();
		virtual int exec();
	private:
		char onOff;
	};

	class RSPStartDrain: public Command
	{
	public:
		RSPStartDrain(CommandProcess* p);
		virtual ~RSPStartDrain();

		virtual int parse();
		virtual int exec();
	private:
		char onOff;
	};

	class RSPSetWarnTemp: public Command
	{
	public:
		RSPSetWarnTemp(CommandProcess* p);
		virtual ~RSPSetWarnTemp();

		virtual int parse();
		virtual int exec();
	private:
		char temp;
	};

	class RSPStartFeedbackPer: public Command
	{
	public:
		RSPStartFeedbackPer(CommandProcess* p);
		virtual ~RSPStartFeedbackPer();

		virtual int parse();
		virtual int exec();
	private:
		char bOn, sec;
	};

	class RSPGetStatus: public Command
	{
	public:
		RSPGetStatus(CommandProcess* p);
		virtual ~RSPGetStatus();

		virtual int parse();
		virtual int exec();
	private:
		char status, temp, level, heat, water;
	};

	class NotifyCompleteType: public Command
	{
	public:
		NotifyCompleteType(CommandProcess* p);
		virtual ~NotifyCompleteType();

		virtual int parse();
		virtual int exec();

	private:
		char type;
	};

	class NotifyWarning: public Command
	{
	public:
		NotifyWarning(CommandProcess* p);
		virtual ~NotifyWarning();

		virtual int parse();
		virtual int exec();

	private:
		char index;
	};

	class RSPSetRinseDur: public Command
	{
	public:
		RSPSetRinseDur(CommandProcess* p);
		virtual ~RSPSetRinseDur();

		virtual int parse();
		virtual int exec();

	private:
		char times;
	};

	class RSPSetDrainDur: public Command
	{
	public:
		RSPSetDrainDur(CommandProcess* p);
		virtual ~RSPSetDrainDur();

		virtual int parse();
		virtual int exec();

	private:
		char minS, minE;
	};

	class RSPSetTempRestrict: public Command
	{
	public:
		RSPSetTempRestrict(CommandProcess* p);
		virtual ~RSPSetTempRestrict();

		virtual int parse();
		virtual int exec();

	private:
		char temp;
	};
};

#endif //__WATERHEATERSERIAL_H__
