#include "utils.h"

#define LOG_MODULE 0x05

QRCodeScanSerial* QRCodeScanSerial::pThis = NULL;
QRCodeScanSerial* QRCodeScanSerial::getInstance()
{
	return pThis;
}

QRCodeScanSerial::QRCodeScanSerial()
{
	pThis = this;

	m_pPort = new RawSerial(QRCODE_SCAN_SERIAL_TX, QRCODE_SCAN_SERIAL_RX);
	if(m_pPort)
	{
		m_pPort->baud(115200);
		m_pPort->format(8, SerialBase::None, 1);
		m_pPort->attach(Callback<void()>(pThis, &QRCodeScanSerial::recv_interrupt), SerialBase::RxIrq);
		//m_pPort->attach(Callback<void()>(pThis, &QRCodeScanSerial::qr_serial_interrupt), SerialBase::RxIrq);
	}

	length = 0;
}

QRCodeScanSerial::~QRCodeScanSerial()
{
	length = 0;
}

int QRCodeScanSerial::process()
{
	int retVal = 0;
	char c;
	while(dataSize() > 0)
	{
		pop(c);
		if((unsigned char)c == 0x00) continue;
		if((unsigned char)c != 0x0D)
		{
			code[length] = c;
			length ++;
			if(length > 255)
			{
				FTRACE("###QRCodeScanSerial::process() fatal error! QR code is too long!\r\n");
				length = 0;
			}
		}
		else
		{
			code[length] = '\0';
			if(Controller::getInstance())
				Controller::getInstance()->QRCodeScanned(code, length);
			//dumpData(code, length);

			// reset
			length = 0;
		}
	}
	processEnd();
	return retVal;
}
/*
void QRCodeScanSerial::qr_serial_interrupt(void)
{
	char c = m_pPort->getc();
	if((unsigned char)c != 0x0D)
	{
		if(length+1 >= capacity)
		{
			if(capacity == 0)
				capacity = 256;
			else
				capacity += 256;

			char* newBuf = new char[capacity];
			if(newBuf)
			{
				if(code)
				{
					memcpy(newBuf, code, length);
					delete[] code;
					code = NULL;
				}

				code = newBuf;
			}
		}

		code[length] = c;
		length ++;
	}
	else
	{
		code[length] = '\0';
		if(Controller::getInstance())
			Controller::getInstance()->QRCodeScanned(code, length);

		dumpData(code, length);

		// reset
		length = 0;
	}
}*/
