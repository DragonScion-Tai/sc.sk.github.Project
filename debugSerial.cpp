#include "utils.h"
#include "ICCard.h"

#define LOG_MODULE 0x06

#define SYNC_BYTE_1				0xaa
#define SYNC_BYTE_2				0x56
#define FRAME_END_1				0xcc
#define FRAME_END_2				0x33
#define FRAME_END_3				0xc3
#define FRAME_END_4				0x3c

#define CMD_COMMON_COMMAND		0x00
#define CMD_SET_WATER_PRICE		0x01
#define CMD_LOGIN				0x02
#define CMD_HOTWATER			0x03
#define CMD_HEATOR_PLAN			0x04
#define CMD_FILTER				0x05
#define CMD_EMPTY				0x06
#define CMD_MEMBRANE			0x07
#define CMD_DISFECT				0x08
#define CMD_ID					0x09
#define CMD_PRINT_PS_LOG		0x0a
#define CMD_SET_DEVICE_TYPE     0x0b

DebugSerial* DebugSerial::pThis = NULL;
DebugSerial* DebugSerial::getInstance()
{
	return pThis;
}

DebugSerial::DebugSerial()
{
	pThis = this;

	m_pPort = new RawSerial(DEBUG_SERIAL_TX, DEBUG_SERIAL_RX);
	if(m_pPort)
	{
		m_pPort->baud(115200);//
		m_pPort->format(8, SerialBase::None, 1);
		m_pPort->attach(Callback<void()>(pThis, &DebugSerial::recv_interrupt), SerialBase::RxIrq);
	}

    nCount = 0;
    nLastSecond = time(NULL);
}

DebugSerial::~DebugSerial()
{
	
}

int DebugSerial::puts(const char *str)
{
	if(m_pPort)
		return m_pPort->puts(str);
	sendData(str, strlen(str));

	return 0;
}

int DebugSerial::testHelmet(time_t sec)
{
    if(sec > nLastSecond + 5)
    {
        if(0 == nCount%2)
        {
            DigitalSwitches::getInstance()->openDustHelmet(true);
            FTRACE("###Helmet Auto Test: open at %d times\r\n", nCount/2);
        }
        else
        {
            DigitalSwitches::getInstance()->openDustHelmet(false);
            FTRACE("###Helmet Auto Test: close at %d times\r\n", nCount/2);
        }

        nLastSecond = sec;
        nCount ++;
    }

    return 0;
}

int DebugSerial::process()
{
	int retVal = 0;
#if DUMP_COMMAND_PACKET
	if(dataSize())
	{
		FTRACE("###DebugSerial::parse() dataSize: %d\r\n", dataSize());
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
						FTRACE("###DebugSerial::process() '%02x' is not Sync byte 'aa'\r\n", (unsigned char)s1);
                        // process one byte on one loop, avoid WD timeout
						//continue;
                        retVal = Command::CMD_PARSE_NEED_DATA;
						break;
					}

					pop(s2);
					if((unsigned char)s2 != SYNC_BYTE_2)
					{
						FTRACE("###DebugSerial::process() '%02x' is not Sync byte '56'\r\n", (unsigned char)s2);
                        // process one byte on one loop, avoid WD timeout
						//continue;
                        retVal = Command::CMD_PARSE_NEED_DATA;
						break;
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
						FTRACE("###DebugSerial::process() frame end check failed\r\n");
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
						//FTRACE("###DebugSerial::process() checksum failed. 0x%x != 0x%x\r\n", 
						//			(unsigned char)m_chksum, (unsigned char)c);
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
					//m_SendId ++;

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

CommandProcess::Command* DebugSerial::createCmd(int c)
{
	switch(c)
	{
		case CMD_COMMON_COMMAND:
			return new CMDCommon(this);
		case CMD_SET_WATER_PRICE:
			return new CMDSetWaterPrice(this);
		case CMD_LOGIN:
			return new CMDLogin(this);
		case CMD_HOTWATER:
			return new CMDHotwater(this);
		case CMD_HEATOR_PLAN:
			return new CMDHeatorPlan(this);
		case CMD_FILTER:
			return new CMDFilter(this);
		case CMD_EMPTY:
			return new CMDEmpty(this);
		case CMD_MEMBRANE:
			return new CMDMembrane(this);
		case CMD_DISFECT:
			return new CMDDisfect(this);
		case CMD_ID:
			return new CMDIDSet(this);
		case CMD_PRINT_PS_LOG:
			return new CMDPrintPSLog(this);
        case CMD_SET_DEVICE_TYPE:
            return new CMDSetDeviceType(this);
		default:
			break;
	}

	return NULL;
}

//CMDCommon
int DebugSerial::CMDCommon::parse()
{
	if(proc->dataSize() < 1)
		return CMD_PARSE_NEED_DATA;

	proc->pop(subId);
	((DebugSerial*)proc)->m_chksum ^= subId;

	return 0;
}

int setCurrentTime()
{
	int year=2017, month=11, day=15, hour=15, min=30;
	struct tm nt;
	memset(&nt, 0, sizeof(struct tm));
	nt.tm_sec = 0;
	nt.tm_min = min;
	nt.tm_hour = hour;
	nt.tm_mday = day;
	nt.tm_mon = month - 1;
	nt.tm_year = year - 1900;
	time_t ns = mktime(&nt);
	set_time(ns);

	Controller::getInstance()->setSysTime(year, month, day, hour, min, 0);

	return 0;
}

int DebugSerial::CMDCommon::exec()
{
	if(subId == 1) // start heating
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() start heating\r\n");
		Action* pAct = new ActionStartHeating(true);
		ActionBase::append(pAct);
	}
	else if(subId == 2) // stop heating
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() stop heating\r\n");
		Action* pAct = new ActionStartHeating(false);
		ActionBase::append(pAct);
	}
	else if(subId == 3) // start drainage
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() start drain\r\n");
		StateMachine::getInstance()->transferToDrainage();
	}
	else if(subId == 4) // stop drainage
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() stop drain\r\n");
		StateMachine::getInstance()->stopDrainageManually();
	}
	else if(subId == 5) // start membrane rinse
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() start membrane rinse\r\n");
		StateMachine::getInstance()->transferToMembraneRinse();
	}
	else if(subId == 6) // stop membrane rinse
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() stop membrane rinse\r\n");
		StateMachine::getInstance()->stopMembraneRinseManually();
	}
	else if(subId == 7) // feed water(B3) open
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() open feed water(B3) valve\r\n");
		DigitalSwitches::getInstance()->turnOnFeedWaterValve(true);
	}
	else if(subId == 8) // feed water(B3) close
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() close feed water(B3) valve\r\n");
		DigitalSwitches::getInstance()->turnOnFeedWaterValve(false);
	}
	else if(subId == 9) // start filter rinse
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() start filter rinse\r\n");
		StateMachine::getInstance()->transferToFilterRinse();
	}
	else if(subId == 10) // stop filter rinse
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() stop filter rinse\r\n");
		StateMachine::getInstance()->stopFilterRinseManually();
	}
	else if(subId == 11)
	{
		//Controller::getInstance()->setFilterShelflife(2);
		//Controller::getInstance()->setFilterProduceDate(2017, 10, 10);

		//Controller::getInstance()->setPulsePerLitre(950);

		//Controller::getInstance()->setWaterCurrent(10000);
		//Controller::getInstance()->setMembraneRinsePlan(2017, 12, 23, 2, 0);
		//setCurrentTime();

		//bool water_release = true;
		//bool ret = __sync_bool_compare_and_swap(&water_release, false, true);
		//FTRACE("###DigitalSwitches::DigitalSwitches() ret=%d water_release=%d\r\n", ret, water_release);

		//DigitalSwitches::getInstance()->startFilterRinse(false);

		//NVIC_SystemReset();

		//Controller::getInstance()->setFilterRinseDur(2, 2);

		//PersistentStorage::getInstance()->dumpFlash(0x20000, 16*10);

		//PersistentStorage::getInstance()->dumpFlash(0x12000, 16*10);

		//Controller::getInstance()->setDisinfectDur(2);
		//Controller::getInstance()->CPDisinfectTemp = 90;
		//Controller::getInstance()->setDisinfectFixTime(6, 11, 42);
		//Controller::getInstance()->setUnitPrice(50, 30, 50);

		//DigitalSwitches::getInstance()->openDustHelmit(true);

		//Controller::getInstance()->WHWarningTempOffset = 5;
		//PersistentStorage::getInstance()->saveWarningTempOffset();

		//PersistentStorage::getInstance()->readSavedLog(20);
        //char dId[32] = {0};
        //Controller::getInstance()->setDeviceId(dId);

		//ActionSetWarningTemp* pAct = new ActionSetWarningTemp(5);
		//ActionBase::append(pAct);
		//Controller::getInstance()->setHeatTemperature(97);

		//Controller::getInstance()->setSwichLock(1);

		//Controller::getInstance()->exitProdTest();

		FTRACE("###DebugSerial::CMDCommon::exec()\n");
        Controller::getInstance()->test();
		
        //int start[4] = {0, 0, 0, 0};
        //int end[4] = {2400, 0, 0, 0};
        //Controller::getInstance()->setHeaterWorkTime(4, start, end);

        //Controller::getInstance()->setAppBalance(5000);
        //if(Controller::getInstance()->isSKIDevice())
        //    FTRACE("###DebugSerial::CMDCommon::exec() SK-I\r\n");
        //if(Controller::getInstance()->isRestrictHotWaterDevice())
        //    FTRACE("###DebugSerial::CMDCommon::exec() Restrict Hot water for free device\r\n");
	}
	else if(subId == 12) // stop central panel
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() power off central panel\r\n");
		DigitalSwitches::getInstance()->powerOffCentralPanel(true);
	}
	else if(subId == 13) // start central panel
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() power on central panel\r\n");
		DigitalSwitches::getInstance()->powerOffCentralPanel(false);
	}
	else if(subId == 14) // start disinfect
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() start disinfect\r\n");
		StateMachine::getInstance()->transferToDisinfect();
	}
	else if(subId == 15) // stop disinfect
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() stop disinfect\r\n");
		StateMachine::getInstance()->stopDisinfectManually();
	}
	else if(subId == 16) // run product setup test
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() start product test\r\n");
		Controller::getInstance()->startProdTest();
	}
	else if(subId == 17) // logout
	{
		
	}
	else if(subId == 20) // start cold water valve
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() open cold water valve\r\n");
		DigitalSwitches::getInstance()->openColdWater();
	}
	else if(subId == 21) // stop cold water valve
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() close cold water valve\r\n");
		DigitalSwitches::getInstance()->closeColdWater();
	}
	else if(subId == 22) // start hot water valve
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() open hot water valve\r\n");
		DigitalSwitches::getInstance()->openHotWater();
	}
	else if(subId == 23) // stop hot water valve
	{
		LOG_INFO("###DebugSerial::CMDCommon::exec() close hot water valve\r\n");
		DigitalSwitches::getInstance()->closeHotWater();
	}
	else if(subId == 24) // read ID
	{
        //char msg[128];

		LOG_INFO("###DebugSerial::CMDCommon::exec() read device id\r\n");
		((DebugSerial*)proc)->puts("APPNotice:Config:ID:");
		((DebugSerial*)proc)->puts(Controller::getInstance()->CPDeviceId);
		((DebugSerial*)proc)->puts("\r\n");

        //snprintf(msg, 128, "APPNotice:Config:ID:%d\r\n", Controller::getInstance()->CPDeviceId);
		//((DebugSerial*)proc)->puts(msg);
	}
	else if(subId == 25) // read Heator
	{
		char msg[128];
		snprintf(msg, 128, "APPNotice:Config:Heator:%d:%d:%d\r\n", Controller::getInstance()->WHBoilingTemp,
			Controller::getInstance()->CPCompensation.compensation,Controller::getInstance()->CPCompensation.startTemp);
		LOG_INFO("###DebugSerial::CMDCommon::exec() read heator setting\r\n");
		((DebugSerial*)proc)->puts(msg);

		int i,j;
		for(i=0; i<7; i++){
			for(j=0; j<3; j++){

				if(!Controller::getInstance()->CPHeaterWorkTime[i].start[j] && !Controller::getInstance()->CPHeaterWorkTime[i].end[j])
					continue;

				snprintf(msg, 128, "APPNotice:Config:HeatorPlan:%d:%d:%d:%d:%d:%d\r\n", i+1, j,
					Controller::getInstance()->CPHeaterWorkTime[i].start[j]/100,Controller::getInstance()->CPHeaterWorkTime[i].start[j]%100,
					Controller::getInstance()->CPHeaterWorkTime[i].end[j]/100,Controller::getInstance()->CPHeaterWorkTime[i].end[j]%100);
				
				((DebugSerial*)proc)->puts(msg);
			}
		}
	}
	else if((subId == 26)) // read Brush --- cannot be used
	{
		char msg[128];
        
		snprintf(msg, 128, "APPNotice:Config:Brush:%d:%d:%d:%d:%d\r\n", 
                Controller::getInstance()->CPFilterFastRinseDur,	
                Controller::getInstance()->CPFilterBackRinseDur, 
                Controller::getInstance()->CPFilterRinseFixTime.day, 
                Controller::getInstance()->CPFilterRinseFixTime.hour, 
                Controller::getInstance()->CPFilterRinseFixTime.min);
		LOG_INFO("###DebugSerial::CMDCommon::exec() read brush setting\r\n");
		((DebugSerial*)proc)->puts(msg);
	}
	else if((subId == 27)) // read clean --- cannot be used
	{
		char msg[128];

		snprintf(msg, 128, "APPNotice:Config:Clean:%d:%d:%d:%d\r\n",
			Controller::getInstance()->CPMembraneRinseDur,
			Controller::getInstance()->CPMembraneRinseFixTime.day,
			Controller::getInstance()->CPMembraneRinseFixTime.hour,
            Controller::getInstance()->CPMembraneRinseFixTime.min);
		LOG_INFO("###DebugSerial::CMDCommon::exec() read clean setting\r\n");
		((DebugSerial*)proc)->puts(msg);
	}
	else if((subId == 28)) // read empty
	{
		char msg[128];
		snprintf(msg, 128, "APPNotice:Config:Empty:%d:%d:%d:%d\r\n",
			Controller::getInstance()->CPDrainDur,
			Controller::getInstance()->CPDrainFixTime.day,
			Controller::getInstance()->CPDrainFixTime.hour,Controller::getInstance()->CPDrainFixTime.min);
		LOG_INFO("###DebugSerial::CMDCommon::exec() read drain setting\r\n");
		((DebugSerial*)proc)->puts(msg);
	}
	else if((subId == 29)) // read disfect --- cannot be used
	{
		char msg[128];

		snprintf(msg, 128, "APPNotice:Config:Disfect:%d:%d:%d:%d\r\n",
			Controller::getInstance()->CPDisinfectDur,
			Controller::getInstance()->CPDisinfectFixTime.day,
			Controller::getInstance()->CPDisinfectFixTime.hour,
            Controller::getInstance()->CPDisinfectFixTime.min);
		LOG_INFO("###DebugSerial::CMDCommon::exec() read disfect setting\r\n");
		((DebugSerial*)proc)->puts(msg);
	}
    else if(subId == 30) // open dust helmet
    {
        DigitalSwitches::getInstance()->openDustHelmet(true);
    }
    else if(subId == 31) // close dust helmet
    {
        //DigitalSwitches::getInstance()->openDustHelmet(false);
        DigitalSwitches::getInstance()->closeHelmetTimeoutFun(); // close immediately
    }
    else if((subId == 32))
    {
        //Controller::getInstance()->setUserCardState(0);
        Controller::getInstance()->exitProdTest();
    }
    else if(subId == 33)
    {
        Controller::getInstance()->setSwichLock(1);
    }
	else if (subId == 34) //open invasion detect
	{
    	Controller::getInstance()->setSwichLock(true);
		LOG_INFO("###DebugSerial::CMDCommon::exec() open invasion detect\r\n");
	}
	else if(subId == 35) //clear all config
	{
		PersistentStorage::getInstance()->ClearAllConfig();

	}
	
	return 0;
}

//CMDSetWaterPrice
int DebugSerial::CMDSetWaterPrice::parse()
{
	if(proc->dataSize() < 2)
		return CMD_PARSE_NEED_DATA;

	proc->pop(hot);
	proc->pop(cold);
	((DebugSerial*)proc)->m_chksum ^= hot ^ cold;

	return 0;
}

int DebugSerial::CMDSetWaterPrice::exec()
{
	Controller::getInstance()->CPHotPrice = hot;
	Controller::getInstance()->CPColdPrice = cold;
	PersistentStorage::getInstance()->saveHotWaterPrice();
	PersistentStorage::getInstance()->saveColdWaterPrice();

	// upload configs
	Controller::getInstance()->getConfiguration();

	return 0;
}


//CMDLogin
int DebugSerial::CMDLogin::parse()
{
	if(proc->dataSize() < 4)
		return CMD_PARSE_NEED_DATA;

	proc->pop(pass0);
	proc->pop(pass1);
	proc->pop(pass2);
	proc->pop(pass3);
	((DebugSerial*)proc)->m_chksum ^= (pass0 ^ pass1 ^ pass2 ^ pass3);

	return 0;
}

int DebugSerial::CMDLogin::exec()
{
	if((pass0 == 1) && (pass1 == 2) && (pass2 == 3) && (pass3 == 4)){
		((DebugSerial*)proc)->puts("APPNotice:Login:OK\r\n");
	} else {
		((DebugSerial*)proc)->puts("APPNotice:Login:Fail\r\n");
	}
	return 0;
}

//CMDHotwater
int DebugSerial::CMDHotwater::parse()
{
	if(proc->dataSize() < 3)
		return CMD_PARSE_NEED_DATA;

	proc->pop(hot_temp);
	proc->pop(hot_temp_adj);
	proc->pop(hot_temp_adj_start);
	((DebugSerial*)proc)->m_chksum ^= hot_temp ^ hot_temp_adj ^ hot_temp_adj_start;

	return 0;
}

int DebugSerial::CMDHotwater::exec()
{
	Controller::getInstance()->setHeatTemperature(hot_temp);
	Controller::getInstance()->setCompensationTemp(hot_temp_adj, hot_temp_adj_start);

	// upload configs
	Controller::getInstance()->getConfiguration();

	return 0;
}


//CMDFilter
int DebugSerial::CMDFilter::parse()
{
	if(proc->dataSize() < 6)
		return CMD_PARSE_NEED_DATA;

	proc->pop(time);
	proc->pop(r_time);
	proc->pop(weekday);
	proc->pop(hour);
	proc->pop(minite);
	proc->pop(times);
	
	((DebugSerial*)proc)->m_chksum ^= time ^ r_time ^ weekday ^ hour ^ minite ^ times;
    
	if(times>20) times = 1;
    
	if(weekday==0){
		hour = 0;
		minite = 0;
	}

	return 0;
}

int DebugSerial::CMDFilter::exec()
{
	FTRACE("###DebugSerial::CMDFilter::exec() (%d/%d) times: %d\r\n", r_time, time, times);
	Controller::getInstance()->setFilterRinseDur(r_time, time);
	Controller::getInstance()->setFilterRinseFixTime(weekday, hour, minite);
	Controller::getInstance()->setMaxLoopCountOfFilterRinse(times);

	// upload configs
	Controller::getInstance()->getConfiguration();

	return 0;
}

//CMDEmpty
int DebugSerial::CMDEmpty::parse()
{
	if(proc->dataSize() < 4)
		return CMD_PARSE_NEED_DATA;

	proc->pop(time);
	proc->pop(weekday);
	proc->pop(hour);
	proc->pop(minite);
	
	((DebugSerial*)proc)->m_chksum ^= time ^ weekday ^ hour ^ minite;

	if(weekday==0){
		hour = 0;
		minite = 0;
	}

	return 0;
}

int DebugSerial::CMDEmpty::exec()
{
	Controller::getInstance()->setDrainFixTime(weekday, hour, minite);
	//Controller::getInstance()->setMembraneRinseDur(time);

	// upload configs
	Controller::getInstance()->getConfiguration();

	return 0;
}

//CMDEMembrane
int DebugSerial::CMDMembrane::parse()
{
	if(proc->dataSize() < 4)
		return CMD_PARSE_NEED_DATA;
    
	proc->pop(time);
	proc->pop(weekday);
	proc->pop(hour);
	proc->pop(minite);
	((DebugSerial*)proc)->m_chksum ^= time ^ weekday ^ hour ^ minite;
    
	if(weekday==0){
		hour = 0;
		minite = 0;
	}

	return 0;
}

int DebugSerial::CMDMembrane::exec()
{
	Controller::getInstance()->setMembraneRinseFixTime(weekday, hour, minite);
	Controller::getInstance()->setMembraneRinseDur(time);

	// upload configs
	Controller::getInstance()->getConfiguration();

	return 0;
}

//CMDDisfect
int DebugSerial::CMDDisfect::parse()
{
	if(proc->dataSize() < 4)
		return CMD_PARSE_NEED_DATA;

	proc->pop(time);
	proc->pop(weekday);
	proc->pop(hour);
	proc->pop(minite);
	((DebugSerial*)proc)->m_chksum ^= time ^ weekday ^ hour ^ minite;
    
	if(weekday==0){
		hour = 0;
		minite = 0;
	}

	return 0;
}

int DebugSerial::CMDDisfect::exec()
{
	Controller::getInstance()->setDisinfectFixTime(weekday, hour, minite);
	Controller::getInstance()->setDisinfectDur(time);

	// upload configs
	Controller::getInstance()->getConfiguration();

	return 0;
}

//CMDIDSet
int DebugSerial::CMDIDSet::parse()
{
	if(proc->dataSize() < 33)
		return CMD_PARSE_NEED_DATA;
	int i;
	for(i=0; i<33; i++){
		proc->pop(id[i]);
		((DebugSerial*)proc)->m_chksum ^= id[i];
	}

	return 0;
}

int DebugSerial::CMDIDSet::exec()
{
	Controller::getInstance()->setDeviceId(id);
	return 0;
}

//CMDHeatorPlan
int DebugSerial::CMDHeatorPlan::parse()
{
	if(proc->dataSize() < 17)
		return CMD_PARSE_NEED_DATA;

	short v,i;
	proc->pop(day);

	for(i=0; i<4; i++){
		proc->pop(v);
		start[i] = v;
		proc->pop(v);
		end[i] = v;
	}
	
	return 0;
}

int DebugSerial::CMDHeatorPlan::exec()
{
	Controller::getInstance()->setHeaterWorkTime(day, start, end);

	// upload configs
	Controller::getInstance()->getConfiguration();

	return 0;
}

//CMDPrintPSLog
int DebugSerial::CMDPrintPSLog::parse()
{
	if(proc->dataSize() < 8)
		return CMD_PARSE_NEED_DATA;

	proc->pop(start);
	proc->pop(lines);

	return 0;
}

int DebugSerial::CMDPrintPSLog::exec()
{
	PersistentStorage::getInstance()->readSavedLog(start, lines);

	return 0;
}

//CMDSetDeviceType
int DebugSerial::CMDSetDeviceType::parse()
{
	if(proc->dataSize() < 4)
		return CMD_PARSE_NEED_DATA;

	proc->pop(type);
	return 0;
}

int DebugSerial::CMDSetDeviceType::exec()
{
    FTRACE("###DebugSerial::CMDSetDeviceType::exec() type: 0x%08X\r\n", type);
    Controller::getInstance()->setDeviceType((unsigned int)type, false);
	return 0;
}
