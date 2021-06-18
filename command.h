#ifndef __COMMAND_H__
#define __COMMAND_H__

class CommandProcess
{
public:
	CommandProcess();
	virtual ~CommandProcess();

	enum {
		SERIALTYPE_CENTRAL_PANEL = 1,
		SERIALTYPE_WATER_HEATER = 2,
		SERIALTYPE_QR_CODE_SCAN = 3,
		SERIALTYPE_IC_CARD_READ = 4,
		SERIALTYPE_DEBUG =5,
		SERIALTYPE_TDS = 6
	};

	virtual int getSerialType() = 0;
	virtual int process() = 0;

	virtual int freeSize();
	virtual int dataSize();
	virtual int dataArrived(); // only called when receive interrupt is triggered and buff is full
	virtual int append(char c);
	virtual int pop(char& c);
	virtual int pop(short& s);
	virtual int pop(int& i);

	virtual bool hasNext(const char c='#', int times=1);
	virtual int popInteger(int& val);
	virtual int popString(char* str, int size);
	virtual int peek(char& c);

	virtual int processEnd(); // hook for bytes consumed
	virtual int flush(); // drop all bytes

	virtual void recv_interrupt(void); // serial receives byte interrupt callback

	virtual int getRingBufferSize() {
		return COMMANDBUFFER_SIZE;
	}

public:
	enum {
		C_PARSE_SYNC_START = 0,
		C_PARSE_CMD,
		C_PARSE_SUB_CMD,
		C_PARSE_PARAM,
		C_PARSE_SYNC_END,
		C_PARSE_CHECKSUM,
		C_PARSE_FINISH,
		C_PARSE_SYNC_ERROR
	} m_status;

	class Command
	{
	public:
		Command(CommandProcess* p) { proc = p; }
		virtual ~Command() { proc = 0; }

		enum
		{
			CMD_PARSE_OK = 0,
			CMD_PARSE_FAIL = -1,
			CMD_PARSE_NEED_DATA = -2,

			CMD_EXEC_OK = 0,
			CMD_EXEC_FAIL = -1
		};

		virtual int parseSubCmd() { return 0; }
		virtual int getCommandId() { return 0; }
		virtual int parse() = 0;
		virtual int exec() = 0;

		char sId; // sendId
		CommandProcess* proc;
	};

protected:
	RawSerial* m_pPort;
	virtual int sendData(const char* buff, int size);

	virtual int dumpData();

public:
	static int dumpData(const char* d, int size);

public:

private:
	char buff[COMMANDBUFFER_SIZE];
	//char* buff;
	char* readPtr;
	char* writePtr;

	bool bMoreBytes;
};

#endif //__COMMAND_H__
