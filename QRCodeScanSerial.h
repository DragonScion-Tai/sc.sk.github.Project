#ifndef __QRCODE_SCAN_H__
#define __QRCODE_SCAN_H__

class QRCodeScanSerial: public CommandProcess
{
public:
	QRCodeScanSerial();
	virtual ~QRCodeScanSerial();

	static QRCodeScanSerial* getInstance();

	virtual int process();

	virtual int getSerialType() {
		return SERIALTYPE_QR_CODE_SCAN;
	}

	//virtual int getRingBufferSize() {
	//	return 256;
	//}

	//virtual void qr_serial_interrupt(void);

private:
	static QRCodeScanSerial* pThis;

	char code[256];
	int length;
};

#endif //__QRCODE_SCAN_H__
