#include "utils.h"

#define LOG_MODULE 0x03

#define SYNC_BYTE_1				0xaa
#define SYNC_BYTE_2				0x55
#define FRAME_END_1				0xcc
#define FRAME_END_2				0x33
#define FRAME_END_3				0xc3
#define FRAME_END_4				0x3c

#define CMD_HAND_SHAKE			0x00
#define CMD_START_HEATING		0x01
#define CMD_SET_BOIL_TEMP		0x02
#define CMD_START_RINSE			0x03
#define CMD_START_DRAIN			0x04
#define CMD_SET_WARN_TEMP		0x05
#define CMD_RESET				0x06 //same as hand shake response
#define CMD_START_FEEDBACK_PER	0x07
#define CMD_GET_STATUS			0x08
#define CMD_RETURN_COMP_TYPE	0x09
#define CMD_RETURN_WARNING		0x0A
#define CMD_SET_RINSE_DUR		0x13
#define CMD_SET_DRAIN_DUR		0x14
#define CMD_SET_TEMP_RESTRICT	0x12

WaterHeaterSerial* WaterHeaterSerial::pThis = NULL;
WaterHeaterSerial* WaterHeaterSerial::getInstance()
{
	return pThis;
}

WaterHeaterSerial::WaterHeaterSerial()
	: m_SendId(0)
	, m_pCmd(NULL)
{
	pThis = this;

	m_pPort = new RawSerial(WATERHEATER_SERIAL_TX, WATERHEATER_SERIAL_RX);
	if(m_pPort)
	{
		m_pPort->baud(9600);
		m_pPort->format(8, SerialBase::None, 1);
		m_pPort->attach(Callback<void()>(pThis, &WaterHeaterSerial::recv_interrupt), SerialBase::RxIrq);
	}
}

WaterHeaterSerial::~WaterHeaterSerial()
{

}

int WaterHeaterSerial::process()
{
	int retVal = 0;
#if DUMP_COMMAND_PACKET
	if(dataSize())
	{
		FTRACE("###WaterHeaterSerial::parse() dataSize: %d\r\n", dataSize());
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
						FTRACE("###WaterHeaterSerial::process() '%02x' is not Sync byte 'aa'\r\n", (unsigned char)s1);
						continue;
					}

					pop(s2);
					if((unsigned char)s2 != SYNC_BYTE_2)
					{
						FTRACE("###WaterHeaterSerial::process() '%02x' is not Sync byte '55'\r\n", (unsigned char)s2);
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
						FTRACE("###WaterHeaterSerial::process() frame end check failed\r\n");
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
						FTRACE("###WaterHeaterSerial::process() checksum failed. 0x%x != 0x%x\r\n", 
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

//
int WaterHeaterSerial::handShake()
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

int WaterHeaterSerial::startHeating(bool bOn)
{
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_START_HEATING;
	//data
	msg[pos++] = bOn ? 1 : 0;
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

int WaterHeaterSerial::setBoilTemp(char temp, char offset)
{
	char msg[32];
	int pos = 0;
	if((unsigned char)temp > WATER_HEATER_HIGHEST_TEMPERATURE)
		temp = WATER_HEATER_HIGHEST_TEMPERATURE;
	if((unsigned char)temp <= WATER_HEATER_BOILING_TEMP_OFFSET)
		temp = WATER_HEATER_BOILING_TEMP_OFFSET + 1;
	
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_SET_BOIL_TEMP;
	//temp
	msg[pos++] = temp;
	//offset
	msg[pos++] = offset;
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

int WaterHeaterSerial::startRinse(bool bOn)
{
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_START_RINSE;
	//data
	msg[pos++] = bOn ? 1 : 0;
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

int WaterHeaterSerial::startDrain(bool bOn)
{
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_START_DRAIN;
	//ctrl
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

int WaterHeaterSerial::setWarnTempOffset(char offset)
{
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_SET_WARN_TEMP;
	//warning temperature offset
	msg[pos++] = offset;
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

int WaterHeaterSerial::resetWaterHeater()
{
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_RESET;
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

int WaterHeaterSerial::startFeedbackPeriodically(bool bOn, char sec)
{
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_START_FEEDBACK_PER;
	//ctrl
	msg[pos++] = bOn ? 1 : 0;
	//time
	msg[pos++] = sec;
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

int WaterHeaterSerial::getStatus()
{
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_GET_STATUS;
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

int WaterHeaterSerial::setRinseDuration(int min)
{
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_SET_RINSE_DUR;
	//times
	msg[pos++] = min;
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

int WaterHeaterSerial::setDrainDuration(int minS, int minE)
{
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_SET_DRAIN_DUR;
	//minS
	msg[pos++] = minS;
	//minE
	msg[pos++] = minE;
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

int WaterHeaterSerial::setBoilingTempRestrict(int temp)
{
	char msg[32];
	int pos = 0;
	//sync
	msg[pos++] = SYNC_BYTE_1;
	msg[pos++] = SYNC_BYTE_2;
	//send id
	msg[pos++] = m_SendId;
	//cmd if
	msg[pos++] = CMD_SET_TEMP_RESTRICT;
	//Temp
	msg[pos++] = temp;
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
CommandProcess::Command* WaterHeaterSerial::createCmd(int c)
{
	//FTRACE("###WaterHeaterSerial::createCmd() cmd: 0x%X\r\n", c);
	switch(c)
	{
		case CMD_HAND_SHAKE:
			return new RSPHandShake(this);
		case CMD_START_HEATING:
			return new RSPStartHeating(this);
		case CMD_SET_BOIL_TEMP:
			return new RSPSetBoilTemp(this);
		case CMD_START_RINSE:
			return new RSPStartRinse(this);
		case CMD_START_DRAIN:
			return new RSPStartDrain(this);
		case CMD_SET_WARN_TEMP:
			return new RSPSetWarnTemp(this);
		case CMD_START_FEEDBACK_PER:
			return new RSPStartFeedbackPer(this);
		case CMD_GET_STATUS:
			return new RSPGetStatus(this);
		case CMD_RETURN_COMP_TYPE:
			return new NotifyCompleteType(this);
		case CMD_RETURN_WARNING:
			return new NotifyWarning(this);
		case CMD_SET_RINSE_DUR:
			return new RSPSetRinseDur(this);
		case CMD_SET_DRAIN_DUR:
			return new RSPSetDrainDur(this);
		case CMD_SET_TEMP_RESTRICT:
			return new RSPSetTempRestrict(this);
		default:
			FTRACE("###ICCardReaderSerial::createCmd() unknown cmd: 0x%X\r\n", c);
			break;
	}

	return NULL;
}

void WaterHeaterSerial::addcheck(char c)
{

}

int WaterHeaterSerial::getchecksum()
{

	return 0;
}

char WaterHeaterSerial::checksum(const char*d, int s)
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
WaterHeaterSerial::RSPHandShake::RSPHandShake(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::RSPHandShake::~RSPHandShake()
{

}

int WaterHeaterSerial::RSPHandShake::parse()
{
	if(proc->dataSize() < 3)
		return CMD_PARSE_NEED_DATA;

	proc->pop(v1);
	proc->pop(v2);
	proc->pop(v3);
	((WaterHeaterSerial*)proc)->m_chksum ^= v1 ^ v2 ^ v3;

	return 0;
}

int WaterHeaterSerial::RSPHandShake::exec()
{
	Controller::getInstance()->handShakeResponse(v1, v2, v3);
	return 0;
}

//RSPStartHeating
WaterHeaterSerial::RSPStartHeating::RSPStartHeating(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::RSPStartHeating::~RSPStartHeating()
{

}

int WaterHeaterSerial::RSPStartHeating::parse()
{
	if(proc->dataSize() < 1)
		return CMD_PARSE_NEED_DATA;

	proc->pop(onOff);
	((WaterHeaterSerial*)proc)->m_chksum ^= onOff;

	return 0;
}

int WaterHeaterSerial::RSPStartHeating::exec()
{
	Controller::getInstance()->startHeatingResponse(onOff);
	return 0;
}

//RSPSetBoilTemp
WaterHeaterSerial::RSPSetBoilTemp::RSPSetBoilTemp(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::RSPSetBoilTemp::~RSPSetBoilTemp()
{

}

int WaterHeaterSerial::RSPSetBoilTemp::parse()
{
	if(proc->dataSize() < 2)
		return CMD_PARSE_NEED_DATA;

	proc->pop(temp);
	proc->pop(offset);
	((WaterHeaterSerial*)proc)->m_chksum ^= temp ^ offset;

	return 0;
}

int WaterHeaterSerial::RSPSetBoilTemp::exec()
{
	Controller::getInstance()->setBoilTempResponse(temp, offset);
	return 0;
}

//RSPStartRinse
WaterHeaterSerial::RSPStartRinse::RSPStartRinse(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::RSPStartRinse::~RSPStartRinse()
{

}

int WaterHeaterSerial::RSPStartRinse::parse()
{
	if(proc->dataSize() < 1)
		return CMD_PARSE_NEED_DATA;

	proc->pop(onOff);
	((WaterHeaterSerial*)proc)->m_chksum ^= onOff;

	return 0;
}

int WaterHeaterSerial::RSPStartRinse::exec()
{
	Controller::getInstance()->startRinseResponse(onOff);
	return 0;
}

//RSPStartDrain
WaterHeaterSerial::RSPStartDrain::RSPStartDrain(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::RSPStartDrain::~RSPStartDrain()
{

}

int WaterHeaterSerial::RSPStartDrain::parse()
{
	if(proc->dataSize() < 1)
		return CMD_PARSE_NEED_DATA;

	proc->pop(onOff);
	((WaterHeaterSerial*)proc)->m_chksum ^= onOff;

	return 0;
}

int WaterHeaterSerial::RSPStartDrain::exec()
{
	Controller::getInstance()->startDrainResponse(onOff);
	return 0;
}

//RSPSetWarnTemp
WaterHeaterSerial::RSPSetWarnTemp::RSPSetWarnTemp(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::RSPSetWarnTemp::~RSPSetWarnTemp()
{

}

int WaterHeaterSerial::RSPSetWarnTemp::parse()
{
	if(proc->dataSize() < 1)
		return CMD_PARSE_NEED_DATA;

	proc->pop(temp);
	((WaterHeaterSerial*)proc)->m_chksum ^= temp;

	return 0;
}

int WaterHeaterSerial::RSPSetWarnTemp::exec()
{
	Controller::getInstance()->setWarnTempResponse(temp);
	return 0;
}

//RSPSetWarnTemp
WaterHeaterSerial::RSPStartFeedbackPer::RSPStartFeedbackPer(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::RSPStartFeedbackPer::~RSPStartFeedbackPer()
{

}

int WaterHeaterSerial::RSPStartFeedbackPer::parse()
{
	if(proc->dataSize() < 2)
		return CMD_PARSE_NEED_DATA;

	proc->pop(bOn);
	proc->pop(sec);
	((WaterHeaterSerial*)proc)->m_chksum ^= bOn ^ sec;

	return 0;
}

int WaterHeaterSerial::RSPStartFeedbackPer::exec()
{
	Controller::getInstance()->startFeedbackPeriodicallyResponse(bOn, sec);
	return 0;
}

//RSPGetStatus
WaterHeaterSerial::RSPGetStatus::RSPGetStatus(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::RSPGetStatus::~RSPGetStatus()
{

}

int WaterHeaterSerial::RSPGetStatus::parse()
{
	if(proc->dataSize() < 5)
		return CMD_PARSE_NEED_DATA;

	proc->pop(status);
	proc->pop(temp);
	proc->pop(level);
	proc->pop(heat);
	proc->pop(water);
	((WaterHeaterSerial*)proc)->m_chksum ^= status ^ temp ^ level ^ heat ^ water;

	return 0;
}

int WaterHeaterSerial::RSPGetStatus::exec()
{
	Controller::getInstance()->getStatusResponse(status, temp, level, heat, water);
	return 0;
}

//NotifyCompleteType
WaterHeaterSerial::NotifyCompleteType::NotifyCompleteType(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::NotifyCompleteType::~NotifyCompleteType()
{

}

int WaterHeaterSerial::NotifyCompleteType::parse()
{
	if(proc->dataSize() < 1)
		return CMD_PARSE_NEED_DATA;

	proc->pop(type);
	((WaterHeaterSerial*)proc)->m_chksum ^= type;

	return 0;
}

int WaterHeaterSerial::NotifyCompleteType::exec()
{
	Controller::getInstance()->completeNotify(type);

	// send response
	{
		char msg[32];
		int pos = 0;
		//sync
		msg[pos++] = SYNC_BYTE_1;
		msg[pos++] = SYNC_BYTE_2;
		//send id
		msg[pos++] = sId;
		//cmd if
		msg[pos++] = CMD_RETURN_COMP_TYPE;
		//type
		msg[pos++] = type;
		//frame end
		msg[pos++] = FRAME_END_1;
		msg[pos++] = FRAME_END_2;
		msg[pos++] = FRAME_END_3;
		msg[pos++] = FRAME_END_4;
		//check sum
		msg[pos] = ((WaterHeaterSerial*)proc)->checksum(msg, pos);
	
		//send out
		((WaterHeaterSerial*)proc)->sendData(msg, ++pos);
	}

	return 0;
}

//NotifyWarning
WaterHeaterSerial::NotifyWarning::NotifyWarning(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::NotifyWarning::~NotifyWarning()
{

}

int WaterHeaterSerial::NotifyWarning::parse()
{
	if(proc->dataSize() < 1)
		return CMD_PARSE_NEED_DATA;

	proc->pop(index);
	((WaterHeaterSerial*)proc)->m_chksum ^= index;

	return 0;
}

int WaterHeaterSerial::NotifyWarning::exec()
{
	Controller::getInstance()->warningNotify(index);

	// can't send response for warning notify
	return 0;
}

//RSPSetRinseDur
WaterHeaterSerial::RSPSetRinseDur::RSPSetRinseDur(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::RSPSetRinseDur::~RSPSetRinseDur()
{

}

int WaterHeaterSerial::RSPSetRinseDur::parse()
{
	if(proc->dataSize() < 1)
		return CMD_PARSE_NEED_DATA;

	proc->pop(times);
	((WaterHeaterSerial*)proc)->m_chksum ^= times;

	return 0;
}

int WaterHeaterSerial::RSPSetRinseDur::exec()
{
	Controller::getInstance()->setRinseDurResponse(times);
	return 0;
}

//RSPSetDrainDur
WaterHeaterSerial::RSPSetDrainDur::RSPSetDrainDur(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::RSPSetDrainDur::~RSPSetDrainDur()
{

}

int WaterHeaterSerial::RSPSetDrainDur::parse()
{
	if(proc->dataSize() < 2)
		return CMD_PARSE_NEED_DATA;

	proc->pop(minS);
	proc->pop(minE);
	((WaterHeaterSerial*)proc)->m_chksum ^= minS ^ minE;

	return 0;
}

int WaterHeaterSerial::RSPSetDrainDur::exec()
{
	Controller::getInstance()->setDrainDurResponse(minS, minE);
	return 0;
}

//RSPSetTempRestrict
WaterHeaterSerial::RSPSetTempRestrict::RSPSetTempRestrict(CommandProcess* p): Command(p)
{

}

WaterHeaterSerial::RSPSetTempRestrict::~RSPSetTempRestrict()
{

}

int WaterHeaterSerial::RSPSetTempRestrict::parse()
{
	if(proc->dataSize() < 1)
		return CMD_PARSE_NEED_DATA;

	proc->pop(temp);
	((WaterHeaterSerial*)proc)->m_chksum ^= temp;

	return 0;
}

int WaterHeaterSerial::RSPSetTempRestrict::exec()
{
	Controller::getInstance()->setTempRestrictResponse(temp);
	return 0;
}
