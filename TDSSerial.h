#ifndef __TDS_SERIAL_H__
#if ENABLE_DTS_SERIAL_IMPL

class TDSSerial: public CommandProcess
{
public:
	TDSSerial();
	virtual ~TDSSerial();

	static TDSSerial* getInstance();

	virtual int process();

	virtual int getSerialType() {
		return SERIALTYPE_TDS;
	}

private:
	static TDSSerial* pThis;

};

#endif //ENABLE_DTS_SERIAL_IMPL
#endif //__TDS_SERIAL_H__
