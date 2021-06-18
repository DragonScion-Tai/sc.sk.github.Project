#include <stdint.h>
#include "utils.h"

#include "SPIFBlockDevice.h"
#include "SpiFlash25.h"

#define SIZE_DEVICEID			32
#define SIZE_HOTPRICE			4
#define SIZE_COLDPRICE			4
#define SIZE_ICEPRICE			4
#define SIZE_FILTERWATERMAX		4
#define SIZE_FILTERWATERCUR		4
#define SIZE_FILTERPRODDATE		12
#define SIZE_FILTERBACKRINSEDUR	4
#define SIZE_FILTERFASTRINSEDUR	4
#define SIZE_MEMBRANERINSEDUR	4
#define SIZE_DRAINMINS			4
#define SIZE_DRAINMINE			4
#define SIZE_SERVICETIME		112
#define SIZE_HEATERWORKTIME		112
#define SIZE_FILTERSHELFLIFE	4
#define SIZE_DISINFECTDUR		4
#define SIZE_PULSEPERLITRE		4
#define SIZE_BOILINGTEMP		4
#define SIZE_BOILINGTEMPOFFSET	4
#define SIZE_WARNINGTEMPOFFSET	4
#define SIZE_HEATERFBDUR		4
#define SIZE_FILTERRINSEFIXTIME	4
#define SIZE_DRAINFIXTIME		4
#define SIZE_MEMBRANERINSEFIXTIME 4
#define SIZE_DISINFECTFIXTIME	4
#define SIZE_FILTERRINSEPLAN	8
#define SIZE_DRAINPLAN			8
#define SIZE_MEMBRANERINSEPLAN	8
#define SIZE_DISINFECTPLAN		8
#define SIZE_MACHINE_STATE		4
#define SIZE_PRODTEST_STEP		4
#define SIZE_FILTER_CHECK_MODE	4
#define SIZE_DISINFECT_DELAY_MAX 4
#define SIZE_INVASION_FLAG		4
#define SIZE_TEMP_COMPENSATION_PARAM 8
#define SIZE_HOTPULSEPERLITRE	4
#define SIZE_COLDPULSEPERLITRE	4
#define SIZE_ICEPULSEPERLITRE	4
#define SIZE_PULSEPERLITRE5		4
#define SIZE_PULSEPERLITRE6		4
#define SIZE_CONFIGPARAM_SECRET 64
#define SIZE_CONFIGPARAM        4
#define SIZE_DEVICETYPE         4

#define SIZE_FILTERRINSEFIXTIMES    DEVICE_RINSE_MAX_NUM*DEVICE_RINSE_BACKSUP_SIZES
#define SIZE_MEMBRANERINSEFIXTIMES  DEVICE_RINSE_MAX_NUM*DEVICE_RINSE_BACKSUP_SIZES
#define SIZE_DISINFECTFIXTIMES      DEVICE_RINSE_MAX_NUM*DEVICE_RINSE_BACKSUP_SIZES

#define SIZE_TDS_ON_OFF         1
#define SIZE_TDS_VALUE			4
#define SIZE_PULSE_WIDTH		4

#define OFFSET_CONFIG_START			0x00
#define OFFSET_DEVICEID				OFFSET_CONFIG_START
#define OFFSET_HOTPRICE				OFFSET_DEVICEID+SIZE_DEVICEID
#define OFFSET_COLDPRICE			OFFSET_HOTPRICE+SIZE_HOTPRICE
#define OFFSET_ICEPRICE				OFFSET_COLDPRICE+SIZE_COLDPRICE
#define OFFSET_FILTERWATERMAX		OFFSET_ICEPRICE+SIZE_ICEPRICE
#define OFFSET_FILTERWATERCUR		OFFSET_FILTERWATERMAX+SIZE_FILTERWATERMAX
#define OFFSET_FILTERPRODDATE		OFFSET_FILTERWATERCUR+SIZE_FILTERWATERCUR
#define OFFSET_FILTERBACKRINSEDUR 	OFFSET_FILTERPRODDATE+SIZE_FILTERPRODDATE
#define OFFSET_FILTERFASTRINSEDUR 	OFFSET_FILTERBACKRINSEDUR+SIZE_FILTERBACKRINSEDUR
#define OFFSET_MEMBRANERINSEDUR		OFFSET_FILTERFASTRINSEDUR+SIZE_FILTERFASTRINSEDUR
#define OFFSET_DRAINMINS			OFFSET_MEMBRANERINSEDUR+SIZE_MEMBRANERINSEDUR
#define OFFSET_DRAINMINE			OFFSET_DRAINMINS+SIZE_DRAINMINS
#define OFFSET_SERVICETIME			OFFSET_DRAINMINE+SIZE_DRAINMINE
#define OFFSET_HEATERWORKTIME		OFFSET_SERVICETIME+SIZE_SERVICETIME
#define OFFSET_FILTERSHELFLIFE		OFFSET_HEATERWORKTIME+SIZE_HEATERWORKTIME
#define OFFSET_DISINFECTDUR			OFFSET_FILTERSHELFLIFE+SIZE_FILTERSHELFLIFE
#define OFFSET_PULSEPERLITRE		OFFSET_DISINFECTDUR+SIZE_DISINFECTDUR
#define OFFSET_BOILINGTEMP			OFFSET_PULSEPERLITRE+SIZE_PULSEPERLITRE
#define OFFSET_BOILINGTEMPOFFSET 	OFFSET_BOILINGTEMP+SIZE_BOILINGTEMP
#define OFFSET_WARNINGTEMPOFFSET	OFFSET_BOILINGTEMPOFFSET+SIZE_BOILINGTEMPOFFSET
#define OFFSET_HEATERFBDUR			OFFSET_WARNINGTEMPOFFSET+SIZE_WARNINGTEMPOFFSET
#define OFFSET_FILTERRINSEFIXTIME	OFFSET_HEATERFBDUR+SIZE_HEATERFBDUR
#define OFFSET_DRAINFIXTIME			OFFSET_FILTERRINSEFIXTIME+SIZE_FILTERRINSEFIXTIME
#define OFFSET_MEMBRANERINSEFIXTIME OFFSET_DRAINFIXTIME+SIZE_DRAINFIXTIME
#define OFFSET_DISINFECTFIXTIME		OFFSET_MEMBRANERINSEFIXTIME+SIZE_MEMBRANERINSEFIXTIME
#define OFFSET_FILTERRINSEPLAN		OFFSET_DISINFECTFIXTIME+SIZE_DISINFECTFIXTIME
#define OFFSET_DRAINPLAN			OFFSET_FILTERRINSEPLAN+SIZE_FILTERRINSEPLAN
#define OFFSET_MEMBRANERINSEPLAN	OFFSET_DRAINPLAN+SIZE_DRAINPLAN
#define OFFSET_DISINFECTPLAN		OFFSET_MEMBRANERINSEPLAN+SIZE_MEMBRANERINSEPLAN
#define OFFSET_MACHINE_STATE		OFFSET_DISINFECTPLAN+SIZE_DISINFECTPLAN
#define OFFSET_PRODTEST_STEP		OFFSET_MACHINE_STATE+SIZE_MACHINE_STATE
#define OFFSET_FILTER_CHECK_MODE	OFFSET_PRODTEST_STEP+SIZE_PRODTEST_STEP
#define OFFSET_DISINFECT_DELAY_MAX	OFFSET_FILTER_CHECK_MODE+SIZE_FILTER_CHECK_MODE
#define OFFSET_INVASION_FLAG		OFFSET_DISINFECT_DELAY_MAX+SIZE_DISINFECT_DELAY_MAX
#define OFFSET_TEMP_COMPENSATION_PARAM OFFSET_INVASION_FLAG+SIZE_INVASION_FLAG
#define OFFSET_HOTPULSEPERLITRE		OFFSET_TEMP_COMPENSATION_PARAM+SIZE_TEMP_COMPENSATION_PARAM
#define OFFSET_COLDPULSEPERLITRE	OFFSET_HOTPULSEPERLITRE+SIZE_HOTPULSEPERLITRE
#define OFFSET_ICEPULSEPERLITRE		OFFSET_COLDPULSEPERLITRE+SIZE_COLDPULSEPERLITRE
#define OFFSET_PULSEPERLITRE5		OFFSET_ICEPULSEPERLITRE+SIZE_ICEPULSEPERLITRE
#define OFFSET_PULSEPERLITRE6		OFFSET_PULSEPERLITRE5+SIZE_PULSEPERLITRE5
#define OFFSET_CONFIGPARAM_SECRET   OFFSET_PULSEPERLITRE6+SIZE_PULSEPERLITRE6
#define OFFSET_CONFIGPARAM          OFFSET_CONFIGPARAM_SECRET+SIZE_CONFIGPARAM_SECRET
#define OFFSET_DEVICETYPE           OFFSET_CONFIGPARAM+SIZE_CONFIGPARAM

#define OFFSET_FILTERRINSEFIXTIMES	  OFFSET_DEVICETYPE+SIZE_DEVICETYPE
#define OFFSET_MEMBRANERINSEFIXTIMES  OFFSET_FILTERRINSEFIXTIMES+SIZE_FILTERRINSEFIXTIMES
#define OFFSET_DISINFECTFIXTIMES      OFFSET_MEMBRANERINSEFIXTIMES+SIZE_MEMBRANERINSEFIXTIMES
#define OFFSET_TDS_ON_OFF             OFFSET_DISINFECTFIXTIMES+SIZE_DISINFECTFIXTIMES
#define OFFSET_TDS_VALUE			  OFFSET_TDS_ON_OFF+SIZE_TDS_ON_OFF
#define OFFSET_PULSE_WIDTH			  OFFSET_TDS_VALUE+SIZE_TDS_VALUE

#define CONFIG_TOTAL_SIZE		      OFFSET_PULSE_WIDTH+SIZE_PULSE_WIDTH

// CONFIG
#define CONFIG_INFO_MAGIC_CODE	(0xDEADBEEF)
#define INVALID_INFO_INDEX		(UINT32_MAX)
#define FLASH_SECTOR_SIZE		(0x1000)	//4KB
#define CONFIG_INFO_FLASH_SIZE	(0x2000)	//8KB
#define CONFIG_INFO_START_ADDR 	(0x10000) 	//offset 64KB
#define CONFIG_INFO_END_ADDR 	(CONFIG_INFO_START_ADDR + CONFIG_INFO_FLASH_SIZE)
#define CONFIG_INFO_BLOCK_SIZE	(0x400)		//every block 1KB
#define CONFIG_INFO_BLOCK_COUNT (CONFIG_INFO_FLASH_SIZE / CONFIG_INFO_BLOCK_SIZE)
#define IS_SECTOR_BOUNDARY(x) 	(((x) % FLASH_SECTOR_SIZE) == 0)
#define CONFIG_INFO_VERSION_ADDR	(CONFIG_INFO_BLOCK_SIZE-12) //
#define CONFIG_INFO_CURRENT_VERSION 0

#if PERSISTENT_STORAGE_VERIFY_CONFIGS
char VerifyConfigBuffer[CONFIG_INFO_BLOCK_SIZE] = {0};
#endif

// LOG
#define LOG_WRITELINE_START_ADDR (0x12000)  //offset 64+8KB
#define LOG_WRITELINE_SIZE		(0x2000)	//8KB
#define LOG_WRITELINE_COUNT		(LOG_WRITELINE_SIZE / sizeof(unsigned int))
#define LOG_START_ADDR			(0x20000)	//offset 128KB
#define LOG_FLASH_SIZE			(0x10000)	//64KB
#define LOG_BYTE_LENGTH			(0x10)		//16B
#define LOG_STORAGE_CAPACITY	(0x1000)	//4096 lines


typedef struct _ConfigInfoHeader
{
	uint32_t wMagicCode;
	uint32_t wConfigIndex;
}ConfigInfoHeader;

SpiFlash25 flash_chip(PA_7, PA_6, PA_5, PA_4);

void flash_revert_all(){
	flash_chip.clear_sector(CONFIG_INFO_START_ADDR);
	flash_chip.clear_sector(CONFIG_INFO_START_ADDR + FLASH_SECTOR_SIZE);
}

uint32_t flash_find_config_index(void)
{
	uint32_t index = INVALID_INFO_INDEX;
	uint32_t bindex = CONFIG_INFO_BLOCK_COUNT;
	uint32_t cindex = INVALID_INFO_INDEX;
	for (uint32_t addr = CONFIG_INFO_END_ADDR - CONFIG_INFO_BLOCK_SIZE; addr >= CONFIG_INFO_START_ADDR; addr -= CONFIG_INFO_BLOCK_SIZE)
	{
		ConfigInfoHeader header;
		bindex--;

		memset(&header,0,sizeof(header));
		if(flash_chip.read(addr +  CONFIG_INFO_BLOCK_SIZE - sizeof(header), sizeof(header), (char*)&header))
		{
			if (header.wMagicCode == CONFIG_INFO_MAGIC_CODE)
			{
				if (header.wConfigIndex != INVALID_INFO_INDEX)
				{
					if (index != INVALID_INFO_INDEX)
					{
						if(header.wConfigIndex > index)
						{
							index =  header.wConfigIndex;
							cindex = bindex;
						}
					}
					else
					{
						index = header.wConfigIndex;
						cindex = bindex;
					}
				}
			}
		}else
		{

		}
	}

	FTRACE("###flash_find_config_index() current config block index %u, %u\r\n", cindex, index);
	return cindex;
}

int flash_config_init(void)
{
	const char *chipid = flash_chip.read_id();
	if (chipid[0] != 0xEF ||
		chipid[1] != 0x30 ||
		chipid[2] != 0x12)
	{
		FTRACE("###flash_config_init() failed\r\n");
		return -1;
	}
	return 0;
}

PersistentStorage ps;
PersistentStorage* PersistentStorage::pThis = NULL;
PersistentStorage* PersistentStorage::getInstance()
{
	return pThis;
}

PersistentStorage::PersistentStorage()
{
	pThis = this;

	bInitialized = false;

	writeLine = 0;
	writeLinePosition = 0;
	bUploadLogs = false;
	readLine = 0;
	totalReadLines = LOG_STORAGE_CAPACITY;
	lastSavedSecond = 0;
}

PersistentStorage::~PersistentStorage()
{

}

void PersistentStorage::ClearAllConfig()
{
    flash_revert_all();
}

int PersistentStorage::init()
{
	int retVal = flash_config_init();
	if (retVal)
	{
		FTRACE("###PersistentStorage::init() flash_config_init failed\r\n");
		return -1;
	}
	
	bInitialized = true;

	// load LOG WRITE LINE
#if ENABLE_SAVE_PERSISTENT_LOG
	char *temp = new char[1024];
	if (!temp)
		return 0;
	uint32_t index = 0;
	writeLine = 0;
	writeLinePosition = 0;
	for(index=0; index<LOG_WRITELINE_SIZE/1024; index++)
	{
		uint32_t readAddr = LOG_WRITELINE_START_ADDR + index * 1024;
		flash_chip.read(readAddr, 1024, temp);

		uint32_t* pos = (uint32_t*)temp;
		if(pos[1024/sizeof(unsigned int) - 1] == 0xFFFFFFFF)
		{
			uint32_t j = 0;
			for(j=0; j<1024/sizeof(unsigned int); j++)
			{
				if(pos[j] == 0xFFFFFFFF)
				{
					break;
				}
			}

			if(j < 1024/sizeof(unsigned int))
			{
				writeLinePosition += j;
				if(j > 0)
				{
					writeLine = pos[j-1];
				}
				break;
			}
			else
			{
				// not happen
			}
		}
		else
		{
			if(pos[1024/sizeof(unsigned int) - 1] > writeLine)
			{
				writeLinePosition += 1024 / sizeof(unsigned int);
				writeLine = pos[1024/sizeof(unsigned int) - 1];
			}
			else
			{
				// only at 0x13000
				break;
			}
		}
	}
	if(writeLinePosition >= LOG_WRITELINE_COUNT)
	{
		writeLinePosition = 0;
	}
	delete[] temp;
	temp = 0;
#endif //ENABLE_SAVE_PERSISTENT_LOG

	// revert all configs
#if PERSISTENT_STORAGE_REVERT_ALL_CONFIGS
	saveAllConfig();
#endif

	return 0;
}

int PersistentStorage::loadAllConfig()
{
	if(!bInitialized)
		return -1;

    uint8_t i;

	//flash_revert_all();
	//Flash has not initialize
	if (flash_find_config_index() == INVALID_INFO_INDEX)
	{
		//set to default config
		saveAllConfig();
	}
	
	Controller *ctrl = Controller::getInstance();

	char *info_buf = new char[CONFIG_INFO_BLOCK_SIZE];
	if (!info_buf)
		return -5;
	memset(info_buf, 0, CONFIG_INFO_BLOCK_SIZE);

	char *temp = info_buf;
	uint32_t index = flash_find_config_index();
	if (index != INVALID_INFO_INDEX)
	{
		uint32_t readAddr = CONFIG_INFO_START_ADDR + (index % CONFIG_INFO_BLOCK_COUNT) * CONFIG_INFO_BLOCK_SIZE;
		flash_chip.read(readAddr, CONFIG_INFO_BLOCK_SIZE, info_buf);
		uint32_t ver = info_buf[CONFIG_INFO_VERSION_ADDR];
		FTRACE("###PersistentStorage::init() load configs version: %d(CURRENT_VERSION=%d)\r\n", ver, CONFIG_INFO_CURRENT_VERSION);

		loadConfig_ver_0(temp);//flash data is changed real param

		if(ver != CONFIG_INFO_CURRENT_VERSION)
		{
			//modify and save upgrade items
		}
	}

	if(!(*(temp+OFFSET_CONFIGPARAM) & 0x08))
	{
    	WarningManager::getInstance()->resolveWarning(WarningManager::WARNING_TYPE_IC_CARD_READER_ERR);
	}
	
	// set valid flag and calc time sec
	process_plan_time();

	FTRACE("###PersistentStorage::init() load configs total size: %d\r\n", CONFIG_TOTAL_SIZE);
	FTRACE("###PersistentStorage::init() load CPDeviceId: %s\r\n", ctrl->CPDeviceId);
	FTRACE("###PersistentStorage::init() load CPHotPrice: %d\r\n", ctrl->CPHotPrice);
	FTRACE("###PersistentStorage::init() load CPColdPrice: %d\r\n", ctrl->CPColdPrice);
	FTRACE("###PersistentStorage::init() load CPIcePrice: %d\r\r\n", ctrl->CPIcePrice);
	FTRACE("###PersistentStorage::init() load CPFilterWaterMax: %d\r\r\n", ctrl->CPFilterWaterMax);
	FTRACE("###PersistentStorage::init() load CPFilterWaterCur: %d\r\r\n", ctrl->CPFilterWaterCur);
	FTRACE("###PersistentStorage::init() load CPFilterProductionDate: %d/%d/%d\r\r\n",
		   ctrl->CPFilterProductionDate.year, ctrl->CPFilterProductionDate.month, ctrl->CPFilterProductionDate.day);
	FTRACE("###PersistentStorage::init() load CPFilterBackRinseDur: %d\r\r\n", ctrl->CPFilterBackRinseDur);
	FTRACE("###PersistentStorage::init() load CPFilterFastRinseDur: %d\r\r\n", ctrl->CPFilterFastRinseDur);
	FTRACE("###PersistentStorage::init() load CPMembraneRinseDur: %d\r\r\n", ctrl->CPMembraneRinseDur);
	FTRACE("###PersistentStorage::init() load CPDrainMinS: %d\r\r\n", ctrl->CPDrainMinS);
	FTRACE("###PersistentStorage::init() load CPDrainMinE: %d\r\r\n", ctrl->CPDrainMinE);
	FTRACE("###PersistentStorage::init() load CPFilterShelfLife: %d\r\r\n", ctrl->CPFilterShelfLife);
	FTRACE("###PersistentStorage::init() load CPDisinfectDur: %d\r\r\n", ctrl->CPDisinfectDur);
	FTRACE("###PersistentStorage::init() load CPPulsePerLitre: %d, CPHotPulsePerLitre: %d, CPColdPulsePerLitre: %d, CPIcePulsePerLitre: %d, CPPulsePerLitre5: %d, CPPulsePerLitre6: %d\r\r\n", ctrl->CPPulsePerLitreArr[CP_PULSE_PER_LITRE], ctrl->CPPulsePerLitreArr[CP_HOT_PULSE_PER_LITRE], ctrl->CPPulsePerLitreArr[CP_COLD_PULSE_PER_LITRE], ctrl->CPPulsePerLitreArr[CP_ICE_PULSE_PER_LITRE], ctrl->CPPulsePerLitreArr[CP_PULSE_PER_LITRE_5], ctrl->CPPulsePerLitreArr[CP_PULSE_PER_LITRE_6]);
	FTRACE("###PersistentStorage::init() load WHBoilingTemp: %d\r\r\n", ctrl->WHBoilingTemp);
	FTRACE("###PersistentStorage::init() load WHTempOffset: %d\r\r\n", ctrl->WHTempOffset);
	FTRACE("###PersistentStorage::init() load WHWarningTempOffset: %d\r\r\n", ctrl->WHWarningTempOffset);
	FTRACE("###PersistentStorage::init() load WHFBDur: %d\r\r\n", ctrl->WHFBDur);
    
    FTRACE("###PersistentStorage::init() load CPFilterRinseFixTime: %d %02d:%02d\r\n", ctrl->CPFilterRinseFixTime.day, ctrl->CPFilterRinseFixTime.hour, ctrl->CPFilterRinseFixTime.min);
	
	FTRACE("###PersistentStorage::init() load CPDrainFixTime: %d %02d:%02d\r\n", ctrl->CPDrainFixTime.day,
		   ctrl->CPDrainFixTime.hour, ctrl->CPDrainFixTime.min);
    
    FTRACE("###PersistentStorage::init() load CPMembraneRinseFixTime: %d %02d:%02d\r\n", ctrl->CPMembraneRinseFixTime.day, ctrl->CPMembraneRinseFixTime.hour, ctrl->CPMembraneRinseFixTime.min);

    FTRACE("###PersistentStorage::init() load CPDisinfectFixTime: %d %02d:%02d\r\n",  ctrl->CPDisinfectFixTime.day, ctrl->CPDisinfectFixTime.hour, ctrl->CPDisinfectFixTime.min);
    
	FTRACE("###PersistentStorage::init() load CPFilterRinsePlan: %d/%d/%d %02d:%02d\r\n", ctrl->CPFilterRinsePlan.year,
		   ctrl->CPFilterRinsePlan.month, ctrl->CPFilterRinsePlan.day, ctrl->CPFilterRinsePlan.hour, ctrl->CPFilterRinsePlan.min);
	FTRACE("###PersistentStorage::init() load CPDrainPlan: %d/%d/%d %02d:%02d\r\n", ctrl->CPDrainPlan.year,
		   ctrl->CPDrainPlan.month, ctrl->CPDrainPlan.day, ctrl->CPDrainPlan.hour, ctrl->CPDrainPlan.min);
	FTRACE("###PersistentStorage::init() load CPMembraneRinsePlan: %d/%d/%d %02d:%02d\r\n", ctrl->CPMembraneRinsePlan.year,
		   ctrl->CPMembraneRinsePlan.month, ctrl->CPMembraneRinsePlan.day,
		   ctrl->CPMembraneRinsePlan.hour, ctrl->CPMembraneRinsePlan.min);
	FTRACE("###PersistentStorage::init() load CPDisinfectPlan: %d/%d/%d %02d:%02d\r\n", ctrl->CPDisinfectPlan.year,
		   ctrl->CPDisinfectPlan.month, ctrl->CPDisinfectPlan.day, ctrl->CPDisinfectPlan.hour, ctrl->CPDisinfectPlan.min);
	FTRACE("###PersistentStorage::init() load StateMachine state: %s\r\n", StateMachine::getInstance()->currentState());
	FTRACE("###PersistentStorage::init() load productionTestStep: %d\r\n", ctrl->productionTestStep);
	FTRACE("###PersistentStorage::init() load CPFilterCheckMode: %d\r\n", ctrl->CPFilterCheckMode);
	FTRACE("###PersistentStorage::init() load CPDisinfectDelayTime: %d\r\n", ctrl->CPDisinfectDelayTime);
	FTRACE("###PersistentStorage::init() load CPInvasionDetected: %d\r\n", ctrl->CPInvasionDetected);
	FTRACE("###PersistentStorage::init() load CPCompensation: [%d, %d]\r\n",
			ctrl->CPCompensation.startTemp, ctrl->CPCompensation.compensation);
    FTRACE("###PersistentStorage::init() load configParamSecret: %s\r\n", ctrl->configParamSecret);
    FTRACE("###PersistentStorage::init() load bCR: %d bQR: %d\r\n", ctrl->configParamICCardReader, ctrl->configParamQRScanner);
    FTRACE("###PersistentStorage::init() load vodasDeviceType: 0x%08X\r\n", ctrl->vodasDeviceType);
    FTRACE("###PersistentStorage::init() load Enabled: %d\r\n", CWaterMixerValve::getInstance()->mEnabled);
	FTRACE("###PersistentStorage::init() load WantedTDS: %d\r\n", CWaterMixerValve::getInstance()->mWantedTDS);
    FTRACE("###PersistentStorage::init() load PulseWidth: %d\r\n", CWaterMixerValve::getInstance()->mPulseWidth);

    for(i=0; i<DEVICE_RINSE_MAX_NUM; i++)
    {
        if(ctrl->CPFilterRinseFixTimes[i].valid)
        FTRACE("###PersistentStorage::init() load CPFilterRinseFixTimes: %d %d %02d:%02d\r\n", 
                ctrl->CPFilterRinseFixTimes[i].index, ctrl->CPFilterRinseFixTimes[i].day, 
                ctrl->CPFilterRinseFixTimes[i].hour, ctrl->CPFilterRinseFixTimes[i].min);
    }
    
    for(i=0; i<DEVICE_RINSE_MAX_NUM; i++)
    {
        if(ctrl->CPMembraneRinseFixTimes[i].valid)
	    FTRACE("###PersistentStorage::init() load CPMembraneRinseFixTimes: %d %d %02d:%02d\r\n", 
            ctrl->CPMembraneRinseFixTimes[i].index, ctrl->CPMembraneRinseFixTimes[i].day,
            ctrl->CPMembraneRinseFixTimes[i].hour, ctrl->CPMembraneRinseFixTimes[i].min);
    }

    for(i=0; i<DEVICE_RINSE_MAX_NUM; i++)
    {
        if(ctrl->CPDisinfectFixTimes[i].valid)
	    FTRACE("###PersistentStorage::init() load CPDisinfectFixTimes: %d %d %02d:%02d\r\n", 
            ctrl->CPDisinfectFixTimes[i].index, ctrl->CPDisinfectFixTimes[i].day,
            ctrl->CPDisinfectFixTimes[i].hour, ctrl->CPDisinfectFixTimes[i].min);
    }    

	//CentralPanelSerial::getInstance()->dumpData(temp, len);

    //dump worktime
    for(i=0; i<7; i++)
    {
        FTRACE("###PersistentStorage::init() load HeaterWorkTime day[%d], [%d~%d] [%d~%d] [%d~%d] [%d~%d]\r\n", i,
            ctrl->CPHeaterWorkTime[i].start[0], ctrl->CPHeaterWorkTime[i].end[0],
            ctrl->CPHeaterWorkTime[i].start[1], ctrl->CPHeaterWorkTime[i].end[1],
            ctrl->CPHeaterWorkTime[i].start[2], ctrl->CPHeaterWorkTime[i].end[2],
            ctrl->CPHeaterWorkTime[i].start[3], ctrl->CPHeaterWorkTime[i].end[3]);
    }

	//
	Controller::getInstance()->calcFilterExpiredNS(true);

	delete[] info_buf;
	info_buf = NULL;

	return 0;
}

int PersistentStorage::saveAllConfig()
{
	// this function will override all configuration data
	// if call successfully
	if(!bInitialized)
		return -4;

	char *info_buf = new char[CONFIG_INFO_BLOCK_SIZE];
	if (!info_buf)
		return -5;
	memset(info_buf, 0, CONFIG_INFO_BLOCK_SIZE);

	ConfigInfoHeader *pInfoHeader = (ConfigInfoHeader *)(info_buf+CONFIG_INFO_BLOCK_SIZE -sizeof(ConfigInfoHeader));
	char *temp = info_buf;
	uint32_t index = flash_find_config_index();
	if (index != INVALID_INFO_INDEX)
	{
		uint32_t readAddr = CONFIG_INFO_START_ADDR + (index % CONFIG_INFO_BLOCK_COUNT) * CONFIG_INFO_BLOCK_SIZE;
		flash_chip.read(readAddr, CONFIG_INFO_BLOCK_SIZE, info_buf);
	}
	else
	{
		pInfoHeader->wMagicCode   = CONFIG_INFO_MAGIC_CODE;
		pInfoHeader->wConfigIndex = INVALID_INFO_INDEX;
	}

	FTRACE("###PersistentStorage::saveAllConfig() revert all configs\r\n");
	Controller *ctrl = Controller::getInstance();

	memcpy(temp+OFFSET_DEVICEID, ctrl->CPDeviceId, SIZE_DEVICEID);
	memcpy(temp+OFFSET_HOTPRICE, &ctrl->CPHotPrice, SIZE_HOTPRICE);
	memcpy(temp+OFFSET_COLDPRICE, &ctrl->CPColdPrice, SIZE_COLDPRICE);
	memcpy(temp+OFFSET_ICEPRICE, &ctrl->CPIcePrice, SIZE_ICEPRICE);
	memcpy(temp+OFFSET_FILTERWATERMAX, &ctrl->CPFilterWaterMax, SIZE_FILTERWATERMAX);
	memcpy(temp+OFFSET_FILTERWATERCUR, &ctrl->CPFilterWaterCur, SIZE_FILTERWATERCUR);
	memcpy(temp+OFFSET_FILTERPRODDATE, &ctrl->CPFilterProductionDate, SIZE_FILTERPRODDATE);
	memcpy(temp+OFFSET_FILTERBACKRINSEDUR, &ctrl->CPFilterBackRinseDur, SIZE_FILTERBACKRINSEDUR);
	memcpy(temp+OFFSET_FILTERFASTRINSEDUR, &ctrl->CPFilterFastRinseDur, SIZE_FILTERFASTRINSEDUR);
	memcpy(temp+OFFSET_MEMBRANERINSEDUR, &ctrl->CPMembraneRinseDur, SIZE_MEMBRANERINSEDUR);
	memcpy(temp+OFFSET_DRAINMINS, &ctrl->CPDrainMinS, SIZE_DRAINMINS);
	memcpy(temp+OFFSET_DRAINMINE, &ctrl->CPDrainMinE, SIZE_DRAINMINE);
	memcpy(temp+OFFSET_SERVICETIME, ctrl->CPServiceTime, SIZE_SERVICETIME);
	memcpy(temp+OFFSET_HEATERWORKTIME, ctrl->CPHeaterWorkTime, SIZE_HEATERWORKTIME);
	memcpy(temp+OFFSET_FILTERSHELFLIFE, &ctrl->CPFilterShelfLife, SIZE_FILTERSHELFLIFE);
	memcpy(temp+OFFSET_DISINFECTDUR, &ctrl->CPDisinfectDur, SIZE_DISINFECTDUR);
	memcpy(temp+OFFSET_PULSEPERLITRE, &ctrl->CPPulsePerLitreArr[CP_PULSE_PER_LITRE], SIZE_PULSEPERLITRE);
	memcpy(temp+OFFSET_BOILINGTEMP, &ctrl->WHBoilingTemp, SIZE_BOILINGTEMP);
	memcpy(temp+OFFSET_BOILINGTEMPOFFSET, &ctrl->WHTempOffset, SIZE_BOILINGTEMPOFFSET);
	memcpy(temp+OFFSET_WARNINGTEMPOFFSET, &ctrl->WHWarningTempOffset, SIZE_WARNINGTEMPOFFSET);
	memcpy(temp+OFFSET_HEATERFBDUR, &ctrl->WHFBDur, SIZE_HEATERFBDUR);
	memcpy(temp+OFFSET_FILTERRINSEFIXTIME, &ctrl->CPFilterRinseFixTime, SIZE_FILTERRINSEFIXTIME);
	memcpy(temp+OFFSET_DRAINFIXTIME, &ctrl->CPDrainFixTime, SIZE_DRAINFIXTIME);
	memcpy(temp+OFFSET_MEMBRANERINSEFIXTIME, &ctrl->CPMembraneRinseFixTime, SIZE_MEMBRANERINSEFIXTIME);
	memcpy(temp+OFFSET_DISINFECTFIXTIME, &ctrl->CPDisinfectFixTime, SIZE_DISINFECTFIXTIME);
	memcpy(temp+OFFSET_FILTERRINSEPLAN, &ctrl->CPFilterRinsePlan, SIZE_FILTERRINSEPLAN);
	memcpy(temp+OFFSET_DRAINPLAN, &ctrl->CPDrainPlan, SIZE_DRAINPLAN);
	memcpy(temp+OFFSET_MEMBRANERINSEPLAN, &ctrl->CPMembraneRinsePlan, SIZE_MEMBRANERINSEPLAN);
	memcpy(temp+OFFSET_DISINFECTPLAN, &ctrl->CPDisinfectPlan, SIZE_DISINFECTPLAN);
	memset(temp+OFFSET_MACHINE_STATE, 0, SIZE_MACHINE_STATE);
	memcpy(temp+OFFSET_PRODTEST_STEP, &ctrl->productionTestStep, SIZE_PRODTEST_STEP);
	memcpy(temp+OFFSET_FILTER_CHECK_MODE, &ctrl->CPFilterCheckMode, SIZE_FILTER_CHECK_MODE);
	memcpy(temp+OFFSET_DISINFECT_DELAY_MAX, &ctrl->CPDisinfectDelayTime, SIZE_DISINFECT_DELAY_MAX);
	memcpy(temp+OFFSET_INVASION_FLAG, &ctrl->CPInvasionDetected, SIZE_INVASION_FLAG);
	memcpy(temp+OFFSET_TEMP_COMPENSATION_PARAM, &ctrl->CPCompensation, SIZE_TEMP_COMPENSATION_PARAM);
	memcpy(temp+OFFSET_HOTPULSEPERLITRE, &ctrl->CPPulsePerLitreArr[CP_HOT_PULSE_PER_LITRE], SIZE_HOTPULSEPERLITRE);
	memcpy(temp+OFFSET_COLDPULSEPERLITRE, &ctrl->CPPulsePerLitreArr[CP_COLD_PULSE_PER_LITRE], SIZE_COLDPULSEPERLITRE);
	memcpy(temp+OFFSET_ICEPULSEPERLITRE, &ctrl->CPPulsePerLitreArr[CP_ICE_PULSE_PER_LITRE], SIZE_ICEPULSEPERLITRE);
	memcpy(temp+OFFSET_PULSEPERLITRE5, &ctrl->CPPulsePerLitreArr[CP_PULSE_PER_LITRE_5], SIZE_PULSEPERLITRE5);
	memcpy(temp+OFFSET_PULSEPERLITRE6, &ctrl->CPPulsePerLitreArr[CP_PULSE_PER_LITRE_6], SIZE_PULSEPERLITRE6);
    memcpy(temp+OFFSET_CONFIGPARAM_SECRET, ctrl->configParamSecret, SIZE_CONFIGPARAM_SECRET);
    {
        unsigned int param = 0;
        param = ctrl->configParamHeatPreservation;
        param |= ctrl->configParamQRScanner << 1;
        param |= ctrl->configParamDustHelmet << 2;
        param |= ctrl->configParamICCardReader << 3;
        param |= ctrl->configParamSK_II << 4;
        memcpy(temp+OFFSET_CONFIGPARAM, &param, SIZE_CONFIGPARAM);
    }
    memcpy(temp+OFFSET_DEVICETYPE, &ctrl->vodasDeviceType, SIZE_DEVICETYPE);
    memcpy(temp+OFFSET_FILTERRINSEFIXTIMES, &ctrl->CPFilterRinseFixTimes, SIZE_FILTERRINSEFIXTIMES);
    memcpy(temp+OFFSET_MEMBRANERINSEFIXTIMES, &ctrl->CPMembraneRinseFixTimes, SIZE_MEMBRANERINSEFIXTIMES);
    memcpy(temp+OFFSET_DISINFECTFIXTIMES, &ctrl->CPDisinfectFixTimes, SIZE_DISINFECTFIXTIMES);
	memcpy(temp+OFFSET_TDS_ON_OFF, &CWaterMixerValve::getInstance()->mEnabled, SIZE_TDS_ON_OFF);
    memcpy(temp+OFFSET_TDS_VALUE, &CWaterMixerValve::getInstance()->mWantedTDS, SIZE_TDS_VALUE);
    memcpy(temp+OFFSET_PULSE_WIDTH, &CWaterMixerValve::getInstance()->mPulseWidth, SIZE_PULSE_WIDTH);

	//save version number
	info_buf[CONFIG_INFO_VERSION_ADDR] = CONFIG_INFO_CURRENT_VERSION;

	uint32_t writeAddr = CONFIG_INFO_START_ADDR + ((index + 1) % CONFIG_INFO_BLOCK_COUNT) * CONFIG_INFO_BLOCK_SIZE;
	//Next write is sector boundary
	if (IS_SECTOR_BOUNDARY(writeAddr))
	{
		flash_chip.clear_sector(writeAddr);
	}
	
	pInfoHeader->wMagicCode = CONFIG_INFO_MAGIC_CODE;
	pInfoHeader->wConfigIndex++;
	flash_chip.write(writeAddr, CONFIG_INFO_BLOCK_SIZE, info_buf);

	delete[] info_buf;
	info_buf = NULL;

	return 0;
}

int PersistentStorage::loadConfig_ver_0(char* temp)
{
	Controller *ctrl = Controller::getInstance();

	memcpy(ctrl->CPDeviceId, temp + OFFSET_DEVICEID, SIZE_DEVICEID);
	memcpy(&ctrl->CPHotPrice, temp + OFFSET_HOTPRICE, SIZE_HOTPRICE);
	memcpy(&ctrl->CPColdPrice, temp + OFFSET_COLDPRICE, SIZE_COLDPRICE);
	memcpy(&ctrl->CPIcePrice, temp + OFFSET_ICEPRICE, SIZE_ICEPRICE);
	memcpy(&ctrl->CPFilterWaterMax, temp + OFFSET_FILTERWATERMAX, SIZE_FILTERWATERMAX);
	memcpy(&ctrl->CPFilterWaterCur, temp + OFFSET_FILTERWATERCUR, SIZE_FILTERWATERCUR);
	memcpy(&ctrl->CPFilterProductionDate, temp + OFFSET_FILTERPRODDATE, SIZE_FILTERPRODDATE);
	memcpy(&ctrl->CPFilterBackRinseDur, temp + OFFSET_FILTERBACKRINSEDUR, SIZE_FILTERBACKRINSEDUR);
	memcpy(&ctrl->CPFilterFastRinseDur, temp + OFFSET_FILTERFASTRINSEDUR, SIZE_FILTERFASTRINSEDUR);
	memcpy(&ctrl->CPMembraneRinseDur, temp + OFFSET_MEMBRANERINSEDUR, SIZE_MEMBRANERINSEDUR);
	memcpy(&ctrl->CPDrainMinS, temp + OFFSET_DRAINMINS, SIZE_DRAINMINS);
	memcpy(&ctrl->CPDrainMinE, temp + OFFSET_DRAINMINE, SIZE_DRAINMINE);
	memcpy(ctrl->CPServiceTime, temp + OFFSET_SERVICETIME, SIZE_SERVICETIME);
	memcpy(ctrl->CPHeaterWorkTime, temp + OFFSET_HEATERWORKTIME, SIZE_HEATERWORKTIME);
	memcpy(&ctrl->CPFilterShelfLife, temp + OFFSET_FILTERSHELFLIFE, SIZE_FILTERSHELFLIFE);
	memcpy(&ctrl->CPDisinfectDur, temp + OFFSET_DISINFECTDUR, SIZE_DISINFECTDUR);
	memcpy(&ctrl->CPPulsePerLitreArr[CP_PULSE_PER_LITRE], temp + OFFSET_PULSEPERLITRE, SIZE_PULSEPERLITRE);
	memcpy(&ctrl->WHBoilingTemp, temp + OFFSET_BOILINGTEMP, SIZE_BOILINGTEMP);
	memcpy(&ctrl->WHTempOffset, temp + OFFSET_BOILINGTEMPOFFSET, SIZE_BOILINGTEMPOFFSET);
	memcpy(&ctrl->WHWarningTempOffset, temp + OFFSET_WARNINGTEMPOFFSET, SIZE_WARNINGTEMPOFFSET);
	memcpy(&ctrl->WHFBDur, temp + OFFSET_HEATERFBDUR, SIZE_HEATERFBDUR);
	memcpy(&ctrl->CPFilterRinseFixTime, temp + OFFSET_FILTERRINSEFIXTIME, SIZE_FILTERRINSEFIXTIME);
	memcpy(&ctrl->CPDrainFixTime, temp + OFFSET_DRAINFIXTIME, SIZE_DRAINFIXTIME);
	memcpy(&ctrl->CPMembraneRinseFixTime, temp + OFFSET_MEMBRANERINSEFIXTIME, SIZE_MEMBRANERINSEFIXTIME);
	memcpy(&ctrl->CPDisinfectFixTime, temp + OFFSET_DISINFECTFIXTIME, SIZE_DISINFECTFIXTIME);
	memcpy(&ctrl->CPFilterRinsePlan, temp + OFFSET_FILTERRINSEPLAN, SIZE_FILTERRINSEPLAN);
	memcpy(&ctrl->CPDrainPlan, temp + OFFSET_DRAINPLAN, SIZE_DRAINPLAN);
	memcpy(&ctrl->CPMembraneRinsePlan, temp + OFFSET_MEMBRANERINSEPLAN, SIZE_MEMBRANERINSEPLAN);
	memcpy(&ctrl->CPDisinfectPlan, temp + OFFSET_DISINFECTPLAN, SIZE_DISINFECTPLAN);
	memcpy(&StateMachine::getInstance()->m_state, temp + OFFSET_MACHINE_STATE, SIZE_MACHINE_STATE);
	memcpy(&ctrl->productionTestStep, temp + OFFSET_PRODTEST_STEP, SIZE_PRODTEST_STEP);
	memcpy(&ctrl->CPFilterCheckMode, temp + OFFSET_FILTER_CHECK_MODE, SIZE_FILTER_CHECK_MODE);
	memcpy(&ctrl->CPDisinfectDelayTime, temp + OFFSET_DISINFECT_DELAY_MAX, SIZE_DISINFECT_DELAY_MAX);
	memcpy(&ctrl->CPInvasionDetected, temp + OFFSET_INVASION_FLAG, SIZE_INVASION_FLAG);
	memcpy(&ctrl->CPCompensation, temp + OFFSET_TEMP_COMPENSATION_PARAM, SIZE_TEMP_COMPENSATION_PARAM);
	memcpy(&ctrl->CPPulsePerLitreArr[CP_HOT_PULSE_PER_LITRE], temp + OFFSET_HOTPULSEPERLITRE, SIZE_HOTPULSEPERLITRE);
	memcpy(&ctrl->CPPulsePerLitreArr[CP_COLD_PULSE_PER_LITRE], temp + OFFSET_COLDPULSEPERLITRE, SIZE_COLDPULSEPERLITRE);
	memcpy(&ctrl->CPPulsePerLitreArr[CP_ICE_PULSE_PER_LITRE], temp + OFFSET_ICEPULSEPERLITRE, SIZE_ICEPULSEPERLITRE);
	memcpy(&ctrl->CPPulsePerLitreArr[CP_PULSE_PER_LITRE_5], temp + OFFSET_PULSEPERLITRE5, SIZE_PULSEPERLITRE5);
	memcpy(&ctrl->CPPulsePerLitreArr[CP_PULSE_PER_LITRE_6], temp + OFFSET_PULSEPERLITRE6, SIZE_PULSEPERLITRE6);
    memcpy(ctrl->configParamSecret, temp + OFFSET_CONFIGPARAM_SECRET, SIZE_CONFIGPARAM_SECRET);
    {
        unsigned int param = 0;
        memcpy(&param, temp + OFFSET_CONFIGPARAM, SIZE_CONFIGPARAM);
        ctrl->configParamHeatPreservation = param & 0x01;
        ctrl->configParamQRScanner = (param >> 1) & 0x01;
        ctrl->configParamDustHelmet = (param >> 2) & 0x01;
        ctrl->configParamICCardReader = (param >> 3) & 0x01;
        ctrl->configParamSK_II = (param >> 4) & 0x01;
    }
    {
        unsigned int tempType = 0;
        memcpy(&tempType, temp + OFFSET_DEVICETYPE, SIZE_DEVICETYPE);
        //if(tempType != ctrl->vodasDeviceType) // forced saving current device type
        //{
        //    // never saved
        //    saveDeviceType();
        //}
        //else
        {
            ctrl->vodasDeviceType = tempType;
        }
        //memcpy(&ctrl->vodasDeviceType, temp + OFFSET_DEVICETYPE, SIZE_DEVICETYPE); // can't load if has not set
    }
    memcpy(&ctrl->CPFilterRinseFixTimes, temp + OFFSET_FILTERRINSEFIXTIMES, SIZE_FILTERRINSEFIXTIMES);
	memcpy(&ctrl->CPMembraneRinseFixTimes, temp + OFFSET_MEMBRANERINSEFIXTIMES, SIZE_MEMBRANERINSEFIXTIMES);
	memcpy(&ctrl->CPDisinfectFixTimes, temp + OFFSET_DISINFECTFIXTIMES, SIZE_DISINFECTFIXTIMES);

	memcpy(&CWaterMixerValve::getInstance()->mEnabled, temp + OFFSET_TDS_ON_OFF, SIZE_TDS_ON_OFF);
	memcpy(&CWaterMixerValve::getInstance()->mWantedTDS, temp + OFFSET_TDS_VALUE, SIZE_TDS_VALUE);
	memcpy(&CWaterMixerValve::getInstance()->mPulseWidth, temp + OFFSET_PULSE_WIDTH, SIZE_PULSE_WIDTH);

	return 0;
}

int PersistentStorage::process(time_t seconds)
{
	// updateConfigArea
	//if(seconds - lastSavedSecond < 30 || seconds < lastSavedSecond)
	//{
	//	lastSavedSecond = seconds;
	//}

	// uploadLogs
	if(bUploadLogs)
	{
		if(readLine >= totalReadLines)
		{
			bUploadLogs = false;
			readLine = 0;

			// if finished
			ActionOperateLogStatus* pAct = new ActionOperateLogStatus(1);
			ActionBase::append(pAct);
		}
		else
		{
			char buff[LOG_BYTE_LENGTH];
			uint32_t* pInt32 = (uint32_t*)buff;
			flash_chip.read(LOG_START_ADDR+readLine*LOG_BYTE_LENGTH, LOG_BYTE_LENGTH, buff);
			readLine ++;

			ActionOperateLog* pAct = new ActionOperateLog(pInt32[0], pInt32[1], pInt32[2], pInt32[3]);
			ActionBase::append(pAct);
		}
	}

	return 0;
}

int PersistentStorage::save(int offset, void* data, int size)
{
	if(!bInitialized)
		return -1;

	ConfigInfoHeader *pInfoHeader = NULL;
	char *info_buf = new char[CONFIG_INFO_BLOCK_SIZE];
	if (!info_buf)
		return -5;
	memset(info_buf, 0, CONFIG_INFO_BLOCK_SIZE);

	char *temp = info_buf;
	uint32_t index = flash_find_config_index();
	if (index != INVALID_INFO_INDEX)
	{
		uint32_t readAddr = CONFIG_INFO_START_ADDR + (index % CONFIG_INFO_BLOCK_COUNT) * CONFIG_INFO_BLOCK_SIZE;
		flash_chip.read(readAddr, CONFIG_INFO_BLOCK_SIZE, info_buf);
	}
	else
	{
		FTRACE("###PersistentStorage::save() revert all configs\r\n");
		//TODO:
	}

    // check data is changed or not
    if(memcmp(temp+offset, data, size) == 0)
    {
        // not changed
        delete[] info_buf;
	    return 0;
    }

	uint32_t writeAddr = CONFIG_INFO_START_ADDR + ((index + 1) % CONFIG_INFO_BLOCK_COUNT) * CONFIG_INFO_BLOCK_SIZE;
	memcpy(temp + offset, data, size);
	//Next write is sector boundary
	if (IS_SECTOR_BOUNDARY(writeAddr))
	{
		flash_chip.clear_sector(writeAddr);
	}
	pInfoHeader = (ConfigInfoHeader *)(info_buf+CONFIG_INFO_BLOCK_SIZE -sizeof(ConfigInfoHeader));
	pInfoHeader->wMagicCode = CONFIG_INFO_MAGIC_CODE;
	pInfoHeader->wConfigIndex++;
	flash_chip.write(writeAddr, CONFIG_INFO_BLOCK_SIZE, info_buf);

#if PERSISTENT_STORAGE_VERIFY_CONFIGS
    int tryBlocks = 0;
    while(tryBlocks < CONFIG_INFO_BLOCK_COUNT)
    {
        flash_chip.read(writeAddr, CONFIG_INFO_BLOCK_SIZE, VerifyConfigBuffer);
        if(memcmp(info_buf, VerifyConfigBuffer, CONFIG_INFO_BLOCK_SIZE) != 0)
        {
            // flash write error
            index ++; // move to next block
            FTRACE("###PersistentStorage::save() verify failed at block %d\r\n", index);

            writeAddr = CONFIG_INFO_START_ADDR + ((index + 1) % CONFIG_INFO_BLOCK_COUNT) * CONFIG_INFO_BLOCK_SIZE;
            //Next write is sector boundary
	        if (IS_SECTOR_BOUNDARY(writeAddr))
	        {
		        flash_chip.clear_sector(writeAddr);
	        }
	        pInfoHeader = (ConfigInfoHeader *)(info_buf+CONFIG_INFO_BLOCK_SIZE -sizeof(ConfigInfoHeader));
	        pInfoHeader->wMagicCode = CONFIG_INFO_MAGIC_CODE;
	        pInfoHeader->wConfigIndex++;
	        flash_chip.write(writeAddr, CONFIG_INFO_BLOCK_SIZE, info_buf);
        }
        else
        {
            break;
        }

        tryBlocks ++;
    }
#endif

	delete[] info_buf;

	return 0;
}

int PersistentStorage::process_plan_time()
{
	Controller* ctrl = Controller::getInstance();
	time_t seconds = time(NULL);

	if(ctrl->CPFilterRinsePlan.year==0 && ctrl->CPFilterRinsePlan.month==0 && ctrl->CPFilterRinsePlan.day==0 &&
		ctrl->CPFilterRinsePlan.hour==0 && ctrl->CPFilterRinsePlan.min==0)
	{
		ctrl->CPFilterRinsePlan.plan_time = 0;
		ctrl->CPFilterRinsePlan.valid = false;
	}
	else
	{
		struct tm nt;
		memset(&nt, 0, sizeof(struct tm));
		//nt.tm_sec = 0;
		nt.tm_min = ctrl->CPFilterRinsePlan.min;
		nt.tm_hour = ctrl->CPFilterRinsePlan.hour;
		nt.tm_mday = ctrl->CPFilterRinsePlan.day;
		nt.tm_mon = ctrl->CPFilterRinsePlan.month - 1;
		nt.tm_year = ctrl->CPFilterRinsePlan.year - 1900;
		ctrl->CPFilterRinsePlan.plan_time = mktime(&nt);
		if(ctrl->CPFilterRinsePlan.plan_time > seconds)
			ctrl->CPFilterRinsePlan.valid = true;
		else
			ctrl->CPFilterRinsePlan.valid = false;
	}

	if(ctrl->CPDrainPlan.year==0 && ctrl->CPDrainPlan.month==0 && ctrl->CPDrainPlan.day==0 &&
		ctrl->CPDrainPlan.hour==0 && ctrl->CPDrainPlan.min==0)
	{
		ctrl->CPDrainPlan.plan_time = 0;
		ctrl->CPDrainPlan.valid = false;
	}
	else
	{
		struct tm nt;
		memset(&nt, 0, sizeof(struct tm));
		//nt.tm_sec = 0;
		nt.tm_min = ctrl->CPDrainPlan.min;
		nt.tm_hour = ctrl->CPDrainPlan.hour;
		nt.tm_mday = ctrl->CPDrainPlan.day;
		nt.tm_mon = ctrl->CPDrainPlan.month - 1;
		nt.tm_year = ctrl->CPDrainPlan.year - 1900;
		ctrl->CPDrainPlan.plan_time = mktime(&nt);
		if(ctrl->CPDrainPlan.plan_time > seconds)
			ctrl->CPDrainPlan.valid = true;
		else
			ctrl->CPDrainPlan.valid = false;
	}

	if(ctrl->CPMembraneRinsePlan.year==0 && ctrl->CPMembraneRinsePlan.month==0 && ctrl->CPMembraneRinsePlan.day==0 &&
		ctrl->CPMembraneRinsePlan.hour==0 && ctrl->CPMembraneRinsePlan.min==0)
	{
		ctrl->CPMembraneRinsePlan.plan_time = 0;
		ctrl->CPMembraneRinsePlan.valid = false;
	}
	else
	{
		struct tm nt;
		memset(&nt, 0, sizeof(struct tm));
		//nt.tm_sec = 0;
		nt.tm_min = ctrl->CPMembraneRinsePlan.min;
		nt.tm_hour = ctrl->CPMembraneRinsePlan.hour;
		nt.tm_mday = ctrl->CPMembraneRinsePlan.day;
		nt.tm_mon = ctrl->CPMembraneRinsePlan.month - 1;
		nt.tm_year = ctrl->CPMembraneRinsePlan.year - 1900;
		ctrl->CPMembraneRinsePlan.plan_time = mktime(&nt);
		if(ctrl->CPMembraneRinsePlan.plan_time > seconds)
			ctrl->CPMembraneRinsePlan.valid = true;
		else
			ctrl->CPMembraneRinsePlan.valid = false;
	}

	if(ctrl->CPDisinfectPlan.year==0 && ctrl->CPDisinfectPlan.month==0 && ctrl->CPDisinfectPlan.day==0 &&
		ctrl->CPDisinfectPlan.hour==0 && ctrl->CPDisinfectPlan.min==0)
	{
		ctrl->CPDisinfectPlan.plan_time = 0;
		ctrl->CPDisinfectPlan.valid = false;
	}
	else
	{
		struct tm nt;
		memset(&nt, 0, sizeof(struct tm));
		//nt.tm_sec = 0;
		nt.tm_min = ctrl->CPDisinfectPlan.min;
		nt.tm_hour = ctrl->CPDisinfectPlan.hour;
		nt.tm_mday = ctrl->CPDisinfectPlan.day;
		nt.tm_mon = ctrl->CPDisinfectPlan.month - 1;
		nt.tm_year = ctrl->CPDisinfectPlan.year - 1900;
		ctrl->CPDisinfectPlan.plan_time = mktime(&nt);
		if(ctrl->CPDisinfectPlan.plan_time > seconds)
			ctrl->CPDisinfectPlan.valid = true;
		else
			ctrl->CPDisinfectPlan.valid = false;
	}

	return 0;
}

int PersistentStorage::saveDeviceId()
{
	return save(OFFSET_DEVICEID, Controller::getInstance()->CPDeviceId, SIZE_DEVICEID);
}

int PersistentStorage::saveHotWaterPrice()
{
	return save(OFFSET_HOTPRICE, &Controller::getInstance()->CPHotPrice, SIZE_HOTPRICE);
}

int PersistentStorage::saveColdWaterPrice()
{
	return save(OFFSET_COLDPRICE, &Controller::getInstance()->CPColdPrice, SIZE_COLDPRICE);
}

int PersistentStorage::saveIceWaterPrice()
{
	return save(OFFSET_ICEPRICE, &Controller::getInstance()->CPIcePrice, SIZE_ICEPRICE);
}

int PersistentStorage::saveFilterWaterCur()
{
	return save(OFFSET_FILTERWATERCUR, &Controller::getInstance()->CPFilterWaterCur, SIZE_FILTERWATERCUR);
}

int PersistentStorage::saveFilterWaterMax()
{
	return save(OFFSET_FILTERWATERMAX, &Controller::getInstance()->CPFilterWaterMax, SIZE_FILTERWATERMAX);
}

int PersistentStorage::saveFilterProductionDate()
{
	return save(OFFSET_FILTERPRODDATE, &Controller::getInstance()->CPFilterProductionDate, SIZE_FILTERPRODDATE);
}

int PersistentStorage::saveFilterRinseDur()
{
	save(OFFSET_FILTERBACKRINSEDUR, &Controller::getInstance()->CPFilterBackRinseDur, SIZE_FILTERBACKRINSEDUR);
	save(OFFSET_FILTERFASTRINSEDUR, &Controller::getInstance()->CPFilterFastRinseDur, SIZE_FILTERFASTRINSEDUR);
	return 0;
}

int PersistentStorage::saveMembraneRinseDur()
{
	return save(OFFSET_MEMBRANERINSEDUR, &Controller::getInstance()->CPMembraneRinseDur, SIZE_MEMBRANERINSEDUR);
}

int PersistentStorage::saveDrainageDur()
{
	save(OFFSET_DRAINMINS, &Controller::getInstance()->CPDrainMinS, SIZE_DRAINMINS);
	save(OFFSET_DRAINMINE, &Controller::getInstance()->CPDrainMinE, SIZE_DRAINMINE);
	return 0;
}

int PersistentStorage::saveServiceTime()
{
	return save(OFFSET_SERVICETIME, Controller::getInstance()->CPServiceTime, SIZE_SERVICETIME);
}

int PersistentStorage::saveHeaterWorkTime()
{
	return save(OFFSET_HEATERWORKTIME, Controller::getInstance()->CPHeaterWorkTime, SIZE_HEATERWORKTIME);
}

int PersistentStorage::saveFilterShelfLife()
{
	return save(OFFSET_FILTERSHELFLIFE, &Controller::getInstance()->CPFilterShelfLife, SIZE_FILTERSHELFLIFE);
}

int PersistentStorage::saveDisinfectDur()
{
	return save(OFFSET_DISINFECTDUR, &Controller::getInstance()->CPDisinfectDur, SIZE_DISINFECTDUR);
}

int PersistentStorage::savePulsePerLitre()
{
	save(OFFSET_PULSEPERLITRE, &Controller::getInstance()->CPPulsePerLitreArr[CP_PULSE_PER_LITRE] , SIZE_PULSEPERLITRE);
	save(OFFSET_HOTPULSEPERLITRE, &Controller::getInstance()->CPPulsePerLitreArr[CP_HOT_PULSE_PER_LITRE] , SIZE_HOTPULSEPERLITRE + SIZE_COLDPULSEPERLITRE + SIZE_ICEPULSEPERLITRE + SIZE_PULSEPERLITRE5 + SIZE_PULSEPERLITRE6);
	return 0;
}

int PersistentStorage::saveTemperature()
{
	return save(OFFSET_BOILINGTEMP, &Controller::getInstance()->WHBoilingTemp, SIZE_BOILINGTEMP);
}

int PersistentStorage::saveWarningTempOffset()
{
	return save(OFFSET_WARNINGTEMPOFFSET, &Controller::getInstance()->WHWarningTempOffset, SIZE_WARNINGTEMPOFFSET);
}

int PersistentStorage::saveHeaterFBDur()
{
	return save(OFFSET_HEATERFBDUR, &Controller::getInstance()->WHFBDur, SIZE_HEATERFBDUR);
}

int PersistentStorage::saveFilterRinseFixtime()
{
	return save(OFFSET_FILTERRINSEFIXTIME, &Controller::getInstance()->CPFilterRinseFixTime, SIZE_FILTERRINSEFIXTIME);
}

int PersistentStorage::saveFilterRinsePlan()
{
	return save(OFFSET_FILTERRINSEPLAN, &Controller::getInstance()->CPFilterRinsePlan, SIZE_FILTERRINSEPLAN);
}

int PersistentStorage::saveMembraneRinseFixtime()
{
	return save(OFFSET_MEMBRANERINSEFIXTIME, &Controller::getInstance()->CPMembraneRinseFixTime, SIZE_MEMBRANERINSEFIXTIME);
}

int PersistentStorage::saveMembraneRinsePlan()
{
	return save(OFFSET_MEMBRANERINSEPLAN, &Controller::getInstance()->CPMembraneRinsePlan, SIZE_MEMBRANERINSEPLAN);
}

int PersistentStorage::saveDrainFixtime()
{
	return save(OFFSET_DRAINFIXTIME, &Controller::getInstance()->CPDrainFixTime, SIZE_DRAINFIXTIME);
}

int PersistentStorage::saveDrainPlan()
{
	return save(OFFSET_DRAINPLAN, &Controller::getInstance()->CPDrainPlan, SIZE_DRAINPLAN);
}

int PersistentStorage::saveDisinfectFixtime()
{
	return save(OFFSET_DISINFECTFIXTIME, &Controller::getInstance()->CPDisinfectFixTime, SIZE_DISINFECTFIXTIME);
}

int PersistentStorage::saveDisinfectPlan()
{
	return save(OFFSET_DISINFECTPLAN, &Controller::getInstance()->CPDisinfectPlan, SIZE_DISINFECTPLAN);
}

int PersistentStorage::saveStateMachineState()
{
	return save(OFFSET_MACHINE_STATE, &StateMachine::getInstance()->m_state, SIZE_MACHINE_STATE);
}

int PersistentStorage::saveProductTestStep()
{
	return save(OFFSET_PRODTEST_STEP, &Controller::getInstance()->productionTestStep, SIZE_PRODTEST_STEP);
}

int PersistentStorage::saveFilterCheckMode()
{
	return save(OFFSET_FILTER_CHECK_MODE, &Controller::getInstance()->CPFilterCheckMode, SIZE_FILTER_CHECK_MODE);
}

int PersistentStorage::saveDisinfectDelayMax()
{
	return save(OFFSET_DISINFECT_DELAY_MAX, &Controller::getInstance()->CPDisinfectDelayTime, SIZE_DISINFECT_DELAY_MAX);
}

int PersistentStorage::saveInvasionDetectedFlag()
{
	return save(OFFSET_INVASION_FLAG, &Controller::getInstance()->CPInvasionDetected, SIZE_INVASION_FLAG);
}

int PersistentStorage::saveTempCompensationParam()
{
	return save(OFFSET_TEMP_COMPENSATION_PARAM, &Controller::getInstance()->CPCompensation, SIZE_TEMP_COMPENSATION_PARAM);
}

int PersistentStorage::saveConfigParamSecret()
{
    return save(OFFSET_CONFIGPARAM_SECRET, &Controller::getInstance()->configParamSecret, SIZE_CONFIGPARAM_SECRET);
}

int PersistentStorage::saveDeviceType()
{
    return save(OFFSET_DEVICETYPE, &Controller::getInstance()->vodasDeviceType, SIZE_DEVICETYPE);
}

int PersistentStorage::saveConfigParam()
{
    unsigned int param = 0;
    param = Controller::getInstance()->configParamHeatPreservation;
    param |= Controller::getInstance()->configParamQRScanner << 1;
    param |= Controller::getInstance()->configParamDustHelmet << 2;
    param |= Controller::getInstance()->configParamICCardReader << 3;
    param |= Controller::getInstance()->configParamSK_II << 4;
    return save(OFFSET_CONFIGPARAM, &param, SIZE_CONFIGPARAM);
}

int PersistentStorage::saveFilterRinseFixtimes(int index)
{
	return save(OFFSET_FILTERRINSEFIXTIMES+index*DEVICE_RINSE_BACKSUP_SIZES, &Controller::getInstance()->CPFilterRinseFixTimes[index], DEVICE_RINSE_BACKSUP_SIZES);
}

int PersistentStorage::saveMembraneRinseFixtimes(int index)
{
	return save(OFFSET_MEMBRANERINSEFIXTIMES+index*DEVICE_RINSE_BACKSUP_SIZES, &Controller::getInstance()->CPMembraneRinseFixTimes[index], DEVICE_RINSE_BACKSUP_SIZES);
}

int PersistentStorage::saveDisinfectFixtimes(int index)
{
	return save(OFFSET_DISINFECTFIXTIMES+index*DEVICE_RINSE_BACKSUP_SIZES, &Controller::getInstance()->CPDisinfectFixTimes[index], DEVICE_RINSE_BACKSUP_SIZES);
}

int PersistentStorage::saveTDS_On_Off()
{
	return save(OFFSET_TDS_ON_OFF, &CWaterMixerValve::getInstance()->mEnabled, SIZE_TDS_ON_OFF);
}

int PersistentStorage::saveTDSValue()
{
	return save(OFFSET_TDS_VALUE, &CWaterMixerValve::getInstance()->mWantedTDS, SIZE_TDS_VALUE);
}

int PersistentStorage::savePulseWidth()
{
	return save(OFFSET_PULSE_WIDTH, &CWaterMixerValve::getInstance()->mPulseWidth, SIZE_PULSE_WIDTH);
}

int PersistentStorage::dumpFlash(unsigned int addr, unsigned int size)
{
	char buf[256];
	if(size > 256)
		return -1;

	flash_chip.read(addr, size, buf);
	CommandProcess::dumpData(buf, size);
	return 0;
}

int PersistentStorage::printLog(int line, int time, int info, int param1, int param2)
{
	const static char* modules[] = {
		"[Command           ]", //0
		"[Controller        ]", //1
		"[CentralPanelSerial]", //2
		"[WaterHeaterSerial ]", //3
		"[ICCardReaderSerial]", //4
		"[QRCodeScanSerial  ]", //5
		"[DebugSerial       ]", //6
		"[DigitalSwitches   ]", //7
		"[Scheduler         ]", //8
		"[StateMachine      ]", //9
		"[WarningManager    ]", //10
		"[iDrink            ]"  //11
	};
	const static char* severity[] = {
		"[FATAL]", //0
		"[ERROR]", //1
		"[WARN ]", //2
		"[INFO ]", //3
		"[DATA ]" //4
	};

	uint8_t m = (info >> 24) & 0xFF;
	uint8_t s = (info >> 16) & 0xFF;
	FTRACE("###PersistentStorage::printLog() %04d%s: %s:%04d param1: 0x%08X param2: 0x%08X  %s",
			line,
			s<=4?severity[s]:"[UNKNOWN]",
			m<=11?modules[m]:"[UNKNOWN]",
			info & 0xFFFF,
			param1,
			param2,
			asctime(localtime((time_t*)&time)));
	return 0;
}

// log
int PersistentStorage::readSavedLog(int lines)
{
	// setup upload log flags; uploaded in main loop
	readLine = 0;
	if(lines > 0 && lines < LOG_STORAGE_CAPACITY)
		totalReadLines = lines;
	else
		totalReadLines = LOG_STORAGE_CAPACITY;
	bUploadLogs = true;

	return 0;
}

int PersistentStorage::readSavedLog(int startline, int lines)
{
	int i = 0;
	char buff[LOG_BYTE_LENGTH];
	uint32_t* pInt32 = (uint32_t*)buff;

	for(i=0; i<lines; i++)
	{
		flash_chip.read(LOG_START_ADDR+(startline+i)*LOG_BYTE_LENGTH, LOG_BYTE_LENGTH, buff);

		printLog(startline+i, pInt32[0], pInt32[1], pInt32[2], pInt32[3]);
	}

	return 0;
}

int PersistentStorage::saveOneLog(const char log[16])
{
#if ENABLE_SAVE_PERSISTENT_LOG
	if(!bInitialized)
		return -1;

	uint32_t writeAddr = LOG_START_ADDR + (writeLine * LOG_BYTE_LENGTH);
	if(IS_SECTOR_BOUNDARY(writeAddr))
	{
		flash_chip.clear_sector(writeAddr);
	}
	// save log
	flash_chip.write(writeAddr, LOG_BYTE_LENGTH, log);
	// increase log position
	writeLine ++;
	if(writeLine >= LOG_STORAGE_CAPACITY)
		writeLine = 0;
	// save log position
	saveLogWriteLine();
#endif //ENABLE_SAVE_PERSISTENT_LOG

	return 0;
}

int PersistentStorage::log_common(char module, char severity, short id, int param1, int param2)
{
	//FTRACE("###PersistentStorage::logger() module(%d) severity(%d) id(%d) param1(%d) param2(%d)\r\n",
	//		module, severity, id, param1, param2);
	char log[16] = {0};
	uint32_t* p = (uint32_t*)log;
	// time
	*p++ = time(NULL);
	// log info
	*p++ = ((unsigned int)module << 24) | ((unsigned int)severity << 16) | ((unsigned int)id & 0xFFFF);
	// log param
	*p++ = param1;
	*p++ = param2;

	return saveOneLog(log);
}

int PersistentStorage::logger(char module, char severity, short id, const char* fmt, ...)
{
	int i = 0, param_count = 0;
	int param1 = 0, param2 = 0;
	bool percentSign = false;
	va_list arg;
	va_start(arg, fmt);
	while(fmt[i] != '\0')
	{
		if(!percentSign && fmt[i] == '%')
		{
			percentSign = true;
		}
		else if(percentSign && (fmt[i] >= '0' && fmt[i] <= '9'))
		{

		}
		else if(percentSign && (fmt[i] == 'd' || fmt[i] == 'u' || fmt[i] == 'x' || fmt[i] == 'X'))
		{
			if(param_count == 0)
				param1 = va_arg(arg, int);
			else if(param_count == 1)
				param2 = va_arg(arg, int);
			else
				break;

			param_count ++;
			percentSign = false;
		}
		else
		{
			percentSign = false;
		}

		i++;
	}
	va_end(arg);

	va_start(arg, fmt);
	debug_trace_va_list(fmt, arg);
	va_end(arg);

	return getInstance()->log_common(module, severity, id, param1, param2);
}

int PersistentStorage::saveLogWriteLine()
{
	uint32_t writeAddr = LOG_WRITELINE_START_ADDR + writeLinePosition * sizeof(unsigned int);
	if(IS_SECTOR_BOUNDARY(writeAddr))
	{
		flash_chip.clear_sector(writeAddr);
	}
	flash_chip.write(writeAddr, sizeof(unsigned int), (char*)&writeLine);
	writeLinePosition ++;
	if(writeLinePosition >= LOG_WRITELINE_COUNT)
	{
		writeLinePosition = 0;
	}
	return 0;
}
