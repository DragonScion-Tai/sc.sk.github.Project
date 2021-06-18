#include "utils.h"

#define LOG_MODULE 0x0A

#define TEST_TRIGGERED_FLAG(x) (x&0x01)
#define SET_TRIGGERED_FLAG(x) x|=0x01
#define TEST_RESOLVED_FLAG(x) (x&0x02)
#define SET_RESOLVED_FLAG(x) x|=0x02

#define WATERHEATER_WARNING_MASK 0x001807E0

WarningManager* WarningManager::pThis = NULL;
WarningManager* WarningManager::getInstance()
{
	if(!pThis)
		pThis = new WarningManager();

	return pThis;
}

WarningManager::WarningManager()
{
	memset(warningArray, 0, sizeof(SWarningRecord)*WARNING_TYPE_COUNT);
	warningIndexFlags = 0;

	bOutOfService = false;
}

WarningManager::~WarningManager()
{

}

int WarningManager::warning_loop()
{
	// one second loop

	int i = 0; 
	for(i=0; i<WARNING_TYPE_COUNT; i++)
	{
		if(TEST_TRIGGERED_FLAG(warningArray[i].flag))
		{
			warningArray[i].lastNotifyTime ++;

			if(TEST_RESOLVED_FLAG(warningArray[i].flag))
			{
				// clear flag
				warningIndexFlags &= ~(0x1 << i);

				// notify
				ActionWarningInfo* pAct = new ActionWarningInfo(getWerningTypeFromIndex(i), false);
				ActionBase::append(pAct);

				// clear warning
				warningArray[i].flag = 0;
				warningArray[i].lastTriggerTime = 0;
				warningArray[i].lastNotifyTime = 0;
			}
			else
			{
				// set flag
				warningIndexFlags |= 0x1 << i;

				if(warningArray[i].lastNotifyTime > WARNING_REPEAT_INTERVAL)
				{
					if(isRetriggeredWarningType(getWerningTypeFromIndex(i)))
					{
						if(warningArray[i].lastTriggerTime <= 0) // no triggered during last interval
						{
							// treat as resolved
							SET_RESOLVED_FLAG(warningArray[i].flag);

							// check next warning type
							continue;
						}
						else
						{
							// reset trigger times
							warningArray[i].lastTriggerTime = 0;
						}
					}

					// notify
					ActionWarningInfo* pAct = new ActionWarningInfo(getWerningTypeFromIndex(i), true);
					ActionBase::append(pAct);

					// reset
					warningArray[i].lastNotifyTime = 0;
				}
			}
		}
	}

	// close hot water if ...
	if(warningIndexFlags & getHotWaterForbiddenWarningMask())
	{
		DigitalSwitches::getInstance()->closeHotWater();
	}

	// close cold water if ...
	if(warningIndexFlags & getColdWaterForbiddenWarningMask())
	{
		DigitalSwitches::getInstance()->closeColdWater();
	}

	// light Warning LED
	if(DigitalSwitches::getInstance())
		DigitalSwitches::getInstance()->lightWarningLED(warningIndexFlags);

	// warning action
	if(warningIndexFlags & getOutOfServiceWarningMask())
	{
		// out of service
		if(!bOutOfService)
		{
			bOutOfService = true;

			StateMachine::getInstance()->transferToFatalErrorState();
		}
	}
	else
	{
		// back to In-Service
		if(bOutOfService)
		{
			bOutOfService = false;

			DigitalSwitches::getInstance()->turnOnFeedWaterValve(true);
			DigitalSwitches::getInstance()->turnOffBeep();

			StateMachine::getInstance()->transferToStandby();
		}
	}

	return 0;
}

unsigned int WarningManager::warningFlags()
{
	return warningIndexFlags;
}

bool WarningManager::isOutOfServiceWarningTriggered()
{
	return warningIndexFlags & getOutOfServiceWarningMask();
}

bool WarningManager::isColdWaterForbidden()
{
	return warningIndexFlags & getColdWaterForbiddenWarningMask();
}

bool WarningManager::isHotWaterForbidden()
{
	return warningIndexFlags & getHotWaterForbiddenWarningMask();
}

int WarningManager::triggerWarning(EWarningType warningType)
{
	int index = getWarningTypeIndex(warningType);
	if(index >= WARNING_TYPE_COUNT)
		return -1;

	if(TEST_TRIGGERED_FLAG(warningArray[index].flag))
	{
		// re-triggered
		warningArray[index].lastTriggerTime++;
	}
	else
	{
		LOG_WARN("###WarningManager::triggerWarning() warningType: %d\r\n", warningType);
		SET_TRIGGERED_FLAG(warningArray[index].flag);
		warningArray[index].lastTriggerTime++;
		warningArray[index].lastNotifyTime = WARNING_REPEAT_INTERVAL; // notify first time;
	}

	return 0;
}

int WarningManager::resolveWarning(EWarningType warningType)
{
	int index = getWarningTypeIndex(warningType);
	if(index >= WARNING_TYPE_COUNT)
		return -1;

	if(TEST_TRIGGERED_FLAG(warningArray[index].flag))
	{
		LOG_WARN("###WarningManager::resolveWarning() warningType: %d\r\n", warningType);
		SET_RESOLVED_FLAG(warningArray[index].flag); // wait notify resolved
	}

	return 0;
}

int WarningManager::getWarnings(int warn[27])
{
    uint8_t i,j;
    int temp;

    for(i=0; i<WARNING_TYPE_COUNT; i++)
    {
        warn[i] = 0xFFFF;
    }

    j = 0x00;
    for(i=0; i<WARNING_TYPE_COUNT; i++)
    {
        if(TEST_TRIGGERED_FLAG(warningArray[i].flag))
        {
            temp = getWerningTypeFromIndex(i);
            if(WARNING_TYPE_COUNT > temp)
            {
                warn[j++] = temp;
            }
        }        
    }
	return 0;
}


int WarningManager::getWarningTypeIndex(EWarningType warningType)
{
	switch(warningType)
	{
		case WARNING_TYPE_WATER_LEAK:
			return 0;
		case WARNING_TYPE_INVASION:
			return 1;
		case WARNING_TYPE_FILTER_EXPIRED:
			return 2;
		case WARNING_TYPE_COLD_WATER_ERR:
			return 3;
		case WARNING_TYPE_HOT_WATER_ERR:
			return 4;
		case WARNING_TYPE_WH_ELECTRIC_LEAK:
			return 5;
		case WARNING_TYPE_WH_HIGH_TEMPERATURE:
			return 6;
		case WARNING_TYPE_WH_LOW_LEVEL_ERR:
			return 7;
		case WARNING_TYPE_WH_HIGH_LEVEL_ERR:
			return 8;
		case WARNING_TYPE_WH_LEVEL_ERR:
			return 9;
		case WARNING_TYPE_WH_OVER_FLOW:
			return 10;
		case WARNING_TYPE_631_VALVE_ERR:
			return 11;
		case WARNING_TYPE_UV_DEVICE_ERR:
			return 12;
		case WARNING_TYPE_WATER_RELEASE_ERR:
			return 13;
		case WARNING_TYPE_ELECTRIC_LEAK:
			return 14;
		case WARNING_TYPE_LOW_BATTERY:
			return 15;
		case WARNING_TYPE_COLD_VALVE_ERR:
			return 16;
		case WARNING_TYPE_HOT_VALVE_ERR:
			return 17;
		case WARNING_TYPE_FEED_VALVE_ERR:
			return 18;
		case WARNING_TYPE_WH_OVER_TEMPERATURE:
			return 19;
		case WARNING_TYPE_WH_TEMPERATURE_ERR:
			return 20;
		case WARNING_TYPE_INFRARED_SENSOR_A_ERR:
			return 21;
		case WARNING_TYPE_INFRARED_SENSOR_B_ERR:
			return 22;
		case WARNING_TYPE_QR_CODE_SCANNER_ERR:
			return 23;
		case WARNING_TYPE_IC_CARD_READER_ERR:
			return 24;
		case WARNING_TYPE_DUST_HELMET_ERR:
			return 25;
		case WARNING_TYPE_MBED_REBOOT:
			return 26;
		default:
			FTRACE("###WarningManager::getWarningTypeIndex Unknown WARNING_TYPE(%d)\r\n", warningType);
			break;
	}

	return WARNING_TYPE_COUNT;
}

WarningManager::EWarningType WarningManager::getWerningTypeFromIndex(int index)
{
	switch(index)
	{
		case 0:
			return WARNING_TYPE_WATER_LEAK;
		case 1:
			return WARNING_TYPE_INVASION;
		case 2:
			return WARNING_TYPE_FILTER_EXPIRED;
		case 3:
			return WARNING_TYPE_COLD_WATER_ERR;
		case 4:
			return WARNING_TYPE_HOT_WATER_ERR;
		case 5:
			return WARNING_TYPE_WH_ELECTRIC_LEAK;
		case 6:
			return WARNING_TYPE_WH_HIGH_TEMPERATURE;
		case 7:
			return WARNING_TYPE_WH_LOW_LEVEL_ERR;
		case 8:
			return WARNING_TYPE_WH_HIGH_LEVEL_ERR;
		case 9:
			return WARNING_TYPE_WH_LEVEL_ERR;
		case 10:
			return WARNING_TYPE_WH_OVER_FLOW;
		case 11:
			return WARNING_TYPE_631_VALVE_ERR;
		case 12:
			return WARNING_TYPE_UV_DEVICE_ERR;
		case 13:
			return WARNING_TYPE_WATER_RELEASE_ERR;
		case 14:
			return WARNING_TYPE_ELECTRIC_LEAK;
		case 15:
			return WARNING_TYPE_LOW_BATTERY;
		case 16:
			return WARNING_TYPE_COLD_VALVE_ERR;
		case 17:
			return WARNING_TYPE_HOT_VALVE_ERR;
		case 18:
			return WARNING_TYPE_FEED_VALVE_ERR;
		case 19:
			return WARNING_TYPE_WH_OVER_TEMPERATURE;
		case 20:
			return WARNING_TYPE_WH_TEMPERATURE_ERR;
		case 21:
			return WARNING_TYPE_INFRARED_SENSOR_A_ERR;
		case 22:
			return WARNING_TYPE_INFRARED_SENSOR_B_ERR;
		case 23:
			return WARNING_TYPE_QR_CODE_SCANNER_ERR;
		case 24:
			return WARNING_TYPE_IC_CARD_READER_ERR;
		case 25:
			return WARNING_TYPE_DUST_HELMET_ERR;
		case 26:
			return WARNING_TYPE_MBED_REBOOT;
		default:
			break;
	}

	return WARNING_TYPE_WATER_LEAK;
}

bool WarningManager::isRetriggeredWarningType(EWarningType warningType)
{
	return (warningType == WARNING_TYPE_WH_ELECTRIC_LEAK) ||
			(warningType == WARNING_TYPE_WH_HIGH_TEMPERATURE) ||
			(warningType == WARNING_TYPE_WH_LOW_LEVEL_ERR) ||
			(warningType == WARNING_TYPE_WH_HIGH_LEVEL_ERR) ||
			(warningType == WARNING_TYPE_WH_LEVEL_ERR) ||
			(warningType == WARNING_TYPE_WH_OVER_FLOW) ||
			(warningType == WARNING_TYPE_WH_OVER_TEMPERATURE);
}

int WarningManager::warningAction(EWarningType type, bool bOn)
{
	switch(type)
	{
		case WARNING_TYPE_WATER_LEAK:
			if(bOn)
			{

			}
			else
			{

			}
			break;
		default:
			break;
	}

	return 0;
}

unsigned int WarningManager::getOutOfServiceWarningMask()
{
	static unsigned int mask = (0x1 << getWarningTypeIndex(WARNING_TYPE_WATER_LEAK)) |
								(0x1 << getWarningTypeIndex(WARNING_TYPE_INVASION)) |
								(0x1 << getWarningTypeIndex(WARNING_TYPE_WH_ELECTRIC_LEAK)) |
								(0x1 << getWarningTypeIndex(WARNING_TYPE_WATER_RELEASE_ERR));

	return mask;
}

unsigned int WarningManager::getColdWaterForbiddenWarningMask()
{
	static unsigned int mask = (0x01 << getWarningTypeIndex(WARNING_TYPE_FILTER_EXPIRED)) |
								(0x01 << getWarningTypeIndex(WARNING_TYPE_631_VALVE_ERR));

	return mask;
}

unsigned int WarningManager::getHotWaterForbiddenWarningMask()
{
	static unsigned int mask = (0x1 << getWarningTypeIndex(WARNING_TYPE_WH_HIGH_TEMPERATURE)) | 
								//(0x1 << getWarningTypeIndex(WARNING_TYPE_WH_LOW_LEVEL_ERR)) |	
								//(0x1 << getWarningTypeIndex(WARNING_TYPE_WH_HIGH_LEVEL_ERR)) |
								//(0x1 << getWarningTypeIndex(WARNING_TYPE_WH_LEVEL_ERR)) |
								(0x1 << getWarningTypeIndex(WARNING_TYPE_WH_OVER_TEMPERATURE));

	return mask;
}
