#ifndef __DEBUG_SERIAL_H__
#define __DEBUG_SERIAL_H__

class DebugSerial: public CommandProcess
{
public:
	DebugSerial();
	virtual ~DebugSerial();

	static DebugSerial* getInstance();

	virtual int puts(const char *str);

	virtual int process();

	virtual int getSerialType() {
		return SERIALTYPE_DEBUG;
	}

    // for test helmet
    virtual int testHelmet(time_t sec);
    unsigned int nCount;
    time_t nLastSecond;

private:
	static DebugSerial* pThis;
	char m_chksum;
	Command* m_pCmd;
	Command* createCmd(int c);

#define DEF_COMMAND_BEGIN(name) \
	class name: public Command \
	{ \
	public: \
		name(CommandProcess* p) : Command(p) { } \
		virtual ~name() { } \
		virtual int parse(); \
		virtual int exec()
#define DEF_COMMAND_END() }

	DEF_COMMAND_BEGIN(CMDCommon);
		char subId;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDSetWaterPrice);
		char hot, cold;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDLogin);
		char pass0, pass1, pass2, pass3;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDHotwater);
		char hot_temp, hot_temp_adj, hot_temp_adj_start;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDHeatorPlan);
		char day;
		int start[4];
		int end[4];
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDFilter);
		char time;
		char r_time;
		char weekday;
		char hour;
		char minite;
		char times;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDEmpty);
		char time;
		char weekday;
		char hour;
		char minite;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDMembrane);
		char time;
		char weekday;
		char hour;
		char minite;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDDisfect);
		char time;
		char weekday;
		char hour;
		char minite;
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDIDSet);
		char id[33];
	DEF_COMMAND_END();

	DEF_COMMAND_BEGIN(CMDPrintPSLog);
		int start, lines;
	DEF_COMMAND_END();

    DEF_COMMAND_BEGIN(CMDSetDeviceType);
        int type;
    DEF_COMMAND_END();

#undef DEF_COMMAND_BEGIN
#undef DEF_COMMAND_END
};

#endif //__DEBUG_SERIAL_H__
