#ifndef __ICCARD_READER_SERIAL_H__
#define __ICCARD_READER_SERIAL_H__

class ICCardReaderSerial: public CommandProcess
{
public:
	ICCardReaderSerial();
	virtual ~ICCardReaderSerial();

	static ICCardReaderSerial* getInstance();

	virtual int process();

	virtual int getSerialType() {
		return SERIALTYPE_IC_CARD_READ;
	}

	// IC Card Reader APIs
	virtual int handShake();
	virtual int enableCardReader(bool bOn);
	virtual int setSectorKey(int secId, int type, char key[6]);
	virtual int writeBlock(int blkId, char data[16]);
	virtual int readBlock(int blkId);
	// ~

private:
	char m_chksum;
	void addcheck(char c);
	int  getchecksum();
	char checksum(const char*d, int s);

private:
	static ICCardReaderSerial* pThis;

	unsigned char m_SendId;

	//Command* m_pCmd;
	Command* createCmd(int c);

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

	class RSPEnableCardReader: public Command
	{
	public:
		RSPEnableCardReader(CommandProcess* p);
		virtual ~RSPEnableCardReader();

		virtual int parse();
		virtual int exec();
	private:
		char bOn;
	};

	class CMDCardDetect: public Command
	{
	public:
		CMDCardDetect(CommandProcess* p);
		virtual ~CMDCardDetect();

		virtual int parse();
		virtual int exec();
	private:
		int cardId;
		char cardStatus;
	};

	class RSPSetSectorKey: public Command
	{
	public:
		RSPSetSectorKey(CommandProcess* p);
		virtual ~RSPSetSectorKey();

		virtual int parse();
		virtual int exec();
	private:
		char secId;
		char type;
		char key[6];
	};

	class RSPWriteBlock: public Command
	{
	public:
		RSPWriteBlock(CommandProcess* p);
		virtual ~RSPWriteBlock();

		virtual int parse();
		virtual int exec();
	private:
		char status;
		char data[16];
	};

	class RSPReadBlock: public Command
	{
	public:
		RSPReadBlock(CommandProcess* p);
		virtual ~RSPReadBlock();

		virtual int parse();
		virtual int exec();
	private:
		char status;
		char data[16];
	};
};

#endif //__ICCARD_READER_SERIAL_H__
