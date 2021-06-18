#include "utils.h"

#define LOG_MODULE 0x04

#define SYNC_BYTE_1				0xaa
#define SYNC_BYTE_2				0x56
#define FRAME_END_1				0xcc
#define FRAME_END_2				0x33
#define FRAME_END_3				0xc3
#define FRAME_END_4				0x3c

#define CMD_HAND_SHAKE			0x00
#define CMD_POWER_ON			0x01
#define CMD_CARD_INFO			0x02
#define CMD_SET_SECTOR_KEY		0x03
#define CMD_WRITE_BLK			0x04
#define CMD_READ_BLK			0x05

CommandProcess::Command* m_pCmd = NULL;

ICCardReaderSerial* ICCardReaderSerial::pThis = NULL;
ICCardReaderSerial* ICCardReaderSerial::getInstance()
{
	return pThis;
}

ICCardReaderSerial::ICCardReaderSerial()
	: m_SendId(0)
	//, m_pCmd(NULL)
{
	pThis = this;

	m_pPort = new RawSerial(ICCARDREADER_SERIAL_TX, ICCARDREADER_SERIAL_RX);
	if(m_pPort)
	{
		m_pPort->baud(9600);
		m_pPort->format(8, SerialBase::None, 1);
		m_pPort->attach(Callback<void()>(pThis, &ICCardReaderSerial::recv_interrupt), SerialBase::RxIrq);
	}
}

ICCardReaderSerial::~ICCardReaderSerial()
{

}

int ICCardReaderSerial::process()
{
	int retVal = 0;
#if DUMP_COMMAND_PACKET
	if(dataSize())
	{
		FTRACE("###ICCardReaderSerial::parse() dataSize: %d\r\n", dataSize());
		dumpData();
	}
#endif
	while(dataSize() > 0)
	{
		switch(m_status)
		{
			case C_PARSE_SYNC_START:
				{
					char s1, s2;
					if(dataSize() < 2)
					{
						// very little data
						peek(s1);
						if((unsigned char)s1 != SYNC_BYTE_1)
							pop(s1); //drop
						retVal = Command::CMD_PARSE_NEED_DATA;
						break;
					}
					pop(s1);
					if((unsigned char)s1 != SYNC_BYTE_1)
					{
						FTRACE("###ICCardReaderSerial::process() '%02x' is not Sync byte 'aa'\r\n", (unsigned char)s1);
						continue;
					}

					pop(s2);
					if((unsigned char)s2 != SYNC_BYTE_2)
					{
						FTRACE("###ICCardReaderSerial::process() '%02x' is not Sync byte '56'\r\n", (unsigned char)s2);
						continue;
					}
					m_chksum = s1;
					m_chksum ^= s2;
					m_status = C_PARSE_CMD;
				}
				break;
			case C_PARSE_CMD:
				{
					char c, sId;
					if(dataSize() < 2)
					{
						// very little data
						retVal = Command::CMD_PARSE_NEED_DATA;
						break;
					}
					//sendId
					pop(sId);
					m_chksum ^= sId;
					pop(c);
					m_chksum ^= c;

					m_pCmd = createCmd(c);
					if(!m_pCmd)
					{
						// out of memory
						goto WRONG_CMD;
					}
					// save the sendId
					m_pCmd->sId = sId;
					m_status = C_PARSE_PARAM;
				}
				break;
			case C_PARSE_PARAM:
				{
					if(!m_pCmd)
						goto WRONG_CMD;
					
					retVal = m_pCmd->parse();
					if(retVal == Command::CMD_PARSE_NEED_DATA)
						break;
					else if(retVal == Command::CMD_PARSE_FAIL)
						goto WRONG_CMD;
					m_status = C_PARSE_SYNC_END;
				}
				break;
			case C_PARSE_SYNC_END:
				{
					char c1, c2, c3, c4;
					if(dataSize() < 4)
					{
						// very little data
						retVal = Command::CMD_PARSE_NEED_DATA;
						break;
					}
					pop(c1); pop(c2); pop(c3); pop(c4);
					if((unsigned char)c1!=FRAME_END_1 || (unsigned char)c2!=FRAME_END_2 ||
						(unsigned char)c3!=FRAME_END_3 || (unsigned char)c4!=FRAME_END_4)
					{
						FTRACE("###ICCardReaderSerial::process() frame end check failed\r\n");
						// failed
					}
					m_chksum ^= c1 ^ c2 ^ c3 ^c4;
					
					m_status = C_PARSE_CHECKSUM;
				}
				break;
			case C_PARSE_CHECKSUM:
				{
					char c;
					pop(c);
					if(m_chksum != c)
					{
						FTRACE("###ICCardReaderSerial::process() checksum failed. 0x%x != 0x%x\r\n", 
									(unsigned char)m_chksum, (unsigned char)c);
					}
					m_status = C_PARSE_FINISH;
				}
				//break; //no break; go through
			case C_PARSE_FINISH:
				{
					if(m_pCmd)
					{
						retVal = m_pCmd->exec();
						if(retVal != Command::CMD_EXEC_OK)
						{
							//response failed?
						}
					
						retVal = 0;
						delete m_pCmd;
						m_pCmd = NULL;
					}
					m_status = C_PARSE_SYNC_START;
					m_SendId ++;

					// finish one command; process in next loop
					return 0;
				}
				break;
			default:
				//some goes wrong
				break;	
		}

		if(retVal == Command::CMD_PARSE_NEED_DATA)
			break; // wait more data
	}

	processEnd();
	return 0;
WRONG_CMD:
	m_status = C_PARSE_SYNC_START;
	if(m_pCmd)
	{
		delete m_pCmd;
		m_pCmd = NULL;
	}
	//flush data
	return -1;
}

int ICCardReaderSerial::handShake()
{
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_HAND_SHAKE;
	//data
	//frame end
	msg[pos++] = FRAME_END_1;
	msg[pos++] = FRAME_END_2;
	msg[pos++] = FRAME_END_3;
	msg[pos++] = FRAME_END_4;
	//check sum
	msg[pos] = checksum(msg, pos);
	
	//send out
	sendData(msg, ++pos);

	return 0;
}

int ICCardReaderSerial::enableCardReader(bool bOn)
{
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_POWER_ON;
	//ctrl: 0: off; 1: on;
	msg[pos++] = bOn;
	//frame end
	msg[pos++] = FRAME_END_1;
	msg[pos++] = FRAME_END_2;
	msg[pos++] = FRAME_END_3;
	msg[pos++] = FRAME_END_4;
	//check sum
	msg[pos] = checksum(msg, pos);
	
	//send out
	sendData(msg, ++pos);
	return 0;
}

int ICCardReaderSerial::setSectorKey(int secId, int type, char key[6])
{
	char msg[32];
	int pos = 0, i = 0;
	FTRACE("###ICCardReaderSerial::setSectorKey() secId: %d\r\n", secId);
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_SET_SECTOR_KEY;
	//sector id[0~15]
	msg[pos++] = secId;
	//key type: 1: Key A; 2: Key B;
	msg[pos++] = type;
	//key data
	for(i=0; i<6; i++)
		msg[pos++] = key[i];
	//frame end
	msg[pos++] = FRAME_END_1;
	msg[pos++] = FRAME_END_2;
	msg[pos++] = FRAME_END_3;
	msg[pos++] = FRAME_END_4;
	//check sum
	msg[pos] = checksum(msg, pos);

	//send out
	sendData(msg, ++pos);
	return 0;
}

int ICCardReaderSerial::writeBlock(int blkId, char data[16])
{
	char msg[32];
	int pos = 0, i = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_WRITE_BLK;
	//block id
	msg[pos++] = blkId;
	//data
	for(i=0; i<16; i++)
		msg[pos++] = data[i];
	//frame end
	msg[pos++] = FRAME_END_1;
	msg[pos++] = FRAME_END_2;
	msg[pos++] = FRAME_END_3;
	msg[pos++] = FRAME_END_4;
	//check sum
	msg[pos] = checksum(msg, pos);

	//send out
	sendData(msg, ++pos);
	return 0;
}

int ICCardReaderSerial::readBlock(int blkId)
{
	char msg[32];
	int pos = 0;
	FTRACE("###ICCardReaderSerial::readBlock() blkId: %d\r\n", blkId);
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_READ_BLK;
	//block id
	msg[pos++] = blkId;
	//frame end
	msg[pos++] = FRAME_END_1;
	msg[pos++] = FRAME_END_2;
	msg[pos++] = FRAME_END_3;
	msg[pos++] = FRAME_END_4;
	//check sum
	msg[pos] = checksum(msg, pos);

	//send out
	sendData(msg, ++pos);
	return 0;
}

//private
CommandProcess::Command* ICCardReaderSerial::createCmd(int c)
{
	switch(c)
	{
		case CMD_HAND_SHAKE:
			return new RSPHandShake(this);
		case CMD_POWER_ON:
			return new RSPEnableCardReader(this);
		case CMD_CARD_INFO:
			return new CMDCardDetect(this);
		case CMD_SET_SECTOR_KEY:
			return new RSPSetSectorKey(this);
		case CMD_WRITE_BLK:
			return new RSPWriteBlock(this);
		case CMD_READ_BLK:
			return new RSPReadBlock(this);
		default:
			FTRACE("###ICCardReaderSerial::createCmd() unknown cmd: 0x%X\r\n", c);
			break;
	}

	return NULL;
}

void ICCardReaderSerial::addcheck(char c)
{

}

int ICCardReaderSerial::getchecksum()
{

	return 0;
}

char ICCardReaderSerial::checksum(const char*d, int s)
{
	char sum = 0;
	int i = 0;
	if(!d || s < 1)
		return sum;

	sum = d[0];
	for(i=1; i<s; i++)
	{
		sum ^= d[i];
	}

	return sum;
}

//RSPHandShake
ICCardReaderSerial::RSPHandShake::RSPHandShake(CommandProcess* p): Command(p)
{

}

ICCardReaderSerial::RSPHandShake::~RSPHandShake()
{

}

int ICCardReaderSerial::RSPHandShake::parse()
{
	if(proc->dataSize() < 3)
		return CMD_PARSE_NEED_DATA;

	proc->pop(v1);
	proc->pop(v2);
	proc->pop(v3);
	((ICCardReaderSerial*)proc)->m_chksum ^= v1 ^ v2 ^ v3;

	return 0;
}

int ICCardReaderSerial::RSPHandShake::exec()
{
	Controller::getInstance()->CRHandShakeResponse(v1, v2, v3);
	return 0;
}

//RSPEnableCardReader
ICCardReaderSerial::RSPEnableCardReader::RSPEnableCardReader(CommandProcess* p): Command(p)
{

}

ICCardReaderSerial::RSPEnableCardReader::~RSPEnableCardReader()
{

}

int ICCardReaderSerial::RSPEnableCardReader::parse()
{
	if(proc->dataSize() < 1)
		return CMD_PARSE_NEED_DATA;

	proc->pop(bOn);
	((ICCardReaderSerial*)proc)->m_chksum ^= bOn;

	return 0;
}

int ICCardReaderSerial::RSPEnableCardReader::exec()
{
	Controller::getInstance()->CREnableReaderResponse(bOn);
	return 0;
}

//CMDCardDetect
ICCardReaderSerial::CMDCardDetect::CMDCardDetect(CommandProcess* p): Command(p)
{

}

ICCardReaderSerial::CMDCardDetect::~CMDCardDetect()
{

}

int ICCardReaderSerial::CMDCardDetect::parse()
{
	if(proc->dataSize() < 5)
		return CMD_PARSE_NEED_DATA;

	char c1, c2, c3, c4;
	proc->pop(c1);
	proc->pop(c2);
	proc->pop(c3);
	proc->pop(c4);
	proc->pop(cardStatus);
	((ICCardReaderSerial*)proc)->m_chksum ^= c1 ^ c2 ^ c3 ^ c4 ^ cardStatus;

	// card id; LE
	cardId = c4;
	cardId <<= 8;
	cardId |= c3;
	cardId <<= 8;
	cardId |= c2;
	cardId <<= 8;
	cardId |= c1;

	return 0;
}

int ICCardReaderSerial::CMDCardDetect::exec()
{
	Controller::getInstance()->CRICCardDetected(cardId, cardStatus);

	// response
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = sId;
	//cmd if
	msg[pos++] = CMD_CARD_INFO;
	//card id
	msg[pos++] = (cardId      ) & 0xFF;
	msg[pos++] = (cardId >>  8) & 0xFF;
	msg[pos++] = (cardId >> 16) & 0xFF;
	msg[pos++] = (cardId >> 24) & 0xFF;
	//cmd; 0x01: start Read/Write; 0x02: stop Read/Write;
	msg[pos++] = 0x01;
	//frame end
	msg[pos++] = FRAME_END_1;
	msg[pos++] = FRAME_END_2;
	msg[pos++] = FRAME_END_3;
	msg[pos++] = FRAME_END_4;
	//check sum
	msg[pos] = ((ICCardReaderSerial*)proc)->checksum(msg, pos);

	//send out
	((ICCardReaderSerial*)proc)->sendData(msg, ++pos);

	// sleep 200 ms for IC Card reader process continous command
	//if(cardStatus == 1)
	//	wait_ms(200);
	return 0;
}

//RSPSetSectorKey
ICCardReaderSerial::RSPSetSectorKey::RSPSetSectorKey(CommandProcess* p): Command(p)
{

}

ICCardReaderSerial::RSPSetSectorKey::~RSPSetSectorKey()
{

}

int ICCardReaderSerial::RSPSetSectorKey::parse()
{
	int i = 0;
	if(proc->dataSize() < 8)
		return CMD_PARSE_NEED_DATA;

	proc->pop(secId);
	proc->pop(type);
	((ICCardReaderSerial*)proc)->m_chksum ^= secId ^ type;
	for(i=0; i<6; i++)
	{
		proc->pop(key[i]);
		((ICCardReaderSerial*)proc)->m_chksum ^= key[i];
	}

	return 0;
}

int ICCardReaderSerial::RSPSetSectorKey::exec()
{
	Controller::getInstance()->CRSetSectorKeyResponse(secId, type, key);
	return 0;
}

//RSPWriteBlock
ICCardReaderSerial::RSPWriteBlock::RSPWriteBlock(CommandProcess* p): Command(p)
{

}

ICCardReaderSerial::RSPWriteBlock::~RSPWriteBlock()
{

}

int ICCardReaderSerial::RSPWriteBlock::parse()
{
	int i = 0;
	if(proc->dataSize() < 17)
		return CMD_PARSE_NEED_DATA;

	proc->pop(status);
	((ICCardReaderSerial*)proc)->m_chksum ^= status;
	for(i=0; i<16; i++)
	{
		proc->pop(data[i]);
		((ICCardReaderSerial*)proc)->m_chksum ^= data[i];
	}

	return 0;
}

int ICCardReaderSerial::RSPWriteBlock::exec()
{
	Controller::getInstance()->CRWriteBlockResponse(status);
	return 0;
}

//RSPReadBlock
ICCardReaderSerial::RSPReadBlock::RSPReadBlock(CommandProcess* p): Command(p)
{

}

ICCardReaderSerial::RSPReadBlock::~RSPReadBlock()
{

}

int ICCardReaderSerial::RSPReadBlock::parse()
{
	int i = 0;
	if(proc->dataSize() < 17)
		return CMD_PARSE_NEED_DATA;

	proc->pop(status);
	((ICCardReaderSerial*)proc)->m_chksum ^= status;
    //FTRACE("###ICCardReaderSerial::RSPReadBlock::parse() status: %d data:", status);
	for(i=0; i<16; i++)
	{
		proc->pop(data[i]);
		((ICCardReaderSerial*)proc)->m_chksum ^= data[i];
        //FTRACE(" 0x%08X\r\n", data[i]);
	}
    //FTRACE("\r\n");

	return 0;
}

int ICCardReaderSerial::RSPReadBlock::exec()
{
	Controller::getInstance()->CRReadBlockResponse(status, data);
	return 0;
}
