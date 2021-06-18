#include "utils.h"

#if ENABLE_DTS_SERIAL_IMPL

#define OP_GET 		0x00
#define OP_GET_ACK	0x01
#define OP_SET		0x02
#define OP_SET_ACK	0x03
#define OP_EVT		0x04
#define OP_ERR		0xFF

#define OP_TYPE_GET_WORK_STATUS		0x04
#define OP_TYPE_GET_WATER_QUALITY	0x05
#define OP_TYPE_SET_WORK_STATUS		0x10
#define OP_TYPE_EVT_WATER_QUALITY	0x2A

TDSSerial* TDSSerial::pThis = NULL;
TDSSerial* TDSSerial::getInstance()
{
	return pThis;
}

TDSSerial::TDSSerial()
{
	pThis = this;

	m_pPort = new RawSerial(RESERVED_SERIAL_TX, RESERVED_SERIAL_RX);
	if(m_pPort)
	{
		m_pPort->baud(9600);
		m_pPort->format(8, SerialBase::None, 1);
		m_pPort->attach(Callback<void()>(pThis, &TDSSerial::recv_interrupt), SerialBase::RxIrq);
	}
}

TDSSerial::~TDSSerial()
{

}

int TDSSerial::process()
{

	return 0;
}

#endif //ENABLE_DTS_SERIAL_IMPL
