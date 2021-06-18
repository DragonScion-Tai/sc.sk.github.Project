#include <cstdarg> //va_list

#include "iDrink.h"
#include "utils.h"

#define LOG_MODULE 0x0B

#if ENABLE_EXTERNAL_RTC
#include "PCF8563.h"

PCF8563 pcf8563(PB_7, PB_6);
time_t pcf8563_read_rtc(void)
{
	return pcf8563.now();
}
void pcf8563_write_rtc(time_t time)
{
	pcf8563.set_time(time);
}
void pcf8563_init_rtc(void)
{
}
int pcf8563_isenabled_rtc(void)
{
	return true;
}
int setExternalRTC()
{
	attach_rtc(pcf8563_read_rtc, pcf8563_write_rtc, pcf8563_init_rtc, pcf8563_isenabled_rtc);
	return 0;
}
#endif

void debug_trace(const char* fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	debug_trace_va_list(fmt, arg);
	va_end(arg);
}

void debug_trace_va_list(const char* fmt, va_list arg)
{
	char msg[256];

	vsnprintf(msg, 256, fmt, arg);
	DebugSerial::getInstance()->puts(msg);
}

const char* getVersionString()
{
	static char ver[16];
	snprintf(ver, 16, "%d.%d.%04d", VER_MAJOR, VER_MINOR, VER_BUILD);

	return ver;
}

void logBoot()
{
    int ver = VER_MAJOR;
    ver <<= 8;
    ver |= VER_MINOR;
    ver <<= 16;
    ver |= VER_BUILD;

    LOG_INFO("###main_loop() iDrink boots log. Ver: 0x%08x  RCC->CSR: 0x%08X\r\n", ver, RCC->CSR);
    RCC->CSR |= 0x01000000; //RMVF
    //RCC->CSR
    //PowerOn: 0x0E000000
    //Reset:   0x04000000
    //SWReset: 0x14000000(Jlink Start Application)
    //IWD TO:  0x24000000
}

int main_loop(void)
{
#if WATCH_DOG_ENABLED
	IWDG_HandleTypeDef iwdg;
	iwdg.Instance = IWDG;
	iwdg.Init.Prescaler = 6;
    iwdg.Init.Reload = WATCH_DOG_REFRESH_DUR;
	//iwdg.Init.Reload = WATCH_DOG_REFRESH_DUR / 8; // in uint of 8 ms
	HAL_IWDG_Init(&iwdg);
#endif

	time_t seconds, last_log_seconds = 0, last_one_seconds = 0;
	Timer debugTimer;
	int lastMiliSec = 0, curMiliSec = 0, fastLoopCost = 4000, slowLoopCost = 0;

	DebugSerial ds;
	CentralPanelSerial cps;
	WaterHeaterSerial whs;
	ICCardReaderSerial iccrs;
	QRCodeScanSerial qrcss;

	// Init Persistent Storage
	if(PersistentStorage::getInstance()->init() < 0)
	{
		// Failed
	}

#if ENABLE_EXTERNAL_RTC
#if USING_EXTERNAL_RTC_AS_SYNC_TIME
	{
		time_t now = pcf8563_read_rtc();
		if(now > 0)
		{
			set_time(now);
		}
		else
		{
			LOG_ERROR("###main_loop() PCF8563 read rtc failed\r\n");
		}
	}
#else
	setExternalRTC();
#endif //USING_EXTERNAL_RTC_AS_SYNC_TIME
#endif //ENABLE_EXTERNAL_RTC

    // for LSE failed case; Switch to LSI costs some seconds
#if WATCH_DOG_ENABLED
    HAL_IWDG_Refresh(&iwdg);
#endif

	// log boot
	logBoot();

	DigitalSwitches::getInstance()->initializeDigitalSwitches();
	DigitalSwitches::getInstance()->turnOnBeep(POWERON_BEEP_DURATION);

#ifdef TDS_CONTROL_SWITCH
    CWaterMixerValve::getInstance()->initialize();
#endif

	Controller::getInstance()->boot();

	debugTimer.start();
	while (true) 
	{
		lastMiliSec = debugTimer.read_ms();

		// process Central Panel Serial
		CentralPanelSerial::getInstance()->process();

		// process Water Heater Serial
		WaterHeaterSerial::getInstance()->process();

		// process QR Code Scan Serial
		QRCodeScanSerial::getInstance()->process();

		// process IC Card Reader Serial
		ICCardReaderSerial::getInstance()->process();

		// process Debug serial
		DebugSerial::getInstance()->process();

		// execute action command
		Controller::getInstance()->action_loop();

		// digital switches process
		DigitalSwitches::getInstance()->process();

		//
		seconds = time(NULL);
		if(seconds > last_one_seconds || seconds < last_one_seconds)
		{
			last_one_seconds = seconds;

			// process scheduled action
			Controller::getInstance()->shedule_loop(seconds);

			// process per one second
			DigitalSwitches::getInstance()->process_one_second(seconds);

			// process warnings
			WarningManager::getInstance()->warning_loop();

#ifdef TDS_CONTROL_SWITCH
			// water mixer valve
            CWaterMixerValve::getInstance()->process_one_second();
#endif

			// process persistent storage
			PersistentStorage::getInstance()->process(seconds);
		}
		if(seconds >= last_log_seconds + MAIN_LOOP_LOG_INTERVAL || seconds < last_log_seconds)
		{
			last_log_seconds = seconds;

		    FTRACE("iDrink(v%s)main loop. Loop cost [%d, %d] ms\r\n", VER_STRING, fastLoopCost, slowLoopCost);
			FTRACE("[DT%X][%s][%dL][%dd][W%08X][I%d][B3%d][T%d][DHO%d][L%d]\r\n",
                    Controller::getInstance()->vodasDeviceType,
					StateMachine::getInstance()->currentState(),
					Controller::getInstance()->currentFilterWaterLitres(),
					Controller::getInstance()->remainFilterWorkableDays(),
					WarningManager::getInstance()->warningFlags(),
					DigitalSwitches::getInstance()->infraredDetectedWorks(),
					DigitalSwitches::getInstance()->feedWaterSwitchOpened(),
					Controller::getInstance()->currentBoilingTemperature(),
                    DigitalSwitches::getInstance()->dustHelmetIsOpened(),
					PersistentStorage::getInstance()->getWriteLine());
			FTRACE("build at %s %s  System time: %s\r\n", __DATE__, __TIME__, asctime(localtime(&seconds)));

			FTRACE("\r\n");
		}

        // for test helmet
        //DebugSerial::getInstance()->testHelmet(seconds);

		//wait(1.0);
		//wait_ms(10);
		wait_us(1000);

		curMiliSec = debugTimer.read_ms();
		curMiliSec -= lastMiliSec;
		if(curMiliSec > slowLoopCost)
			slowLoopCost = curMiliSec;
		if(curMiliSec < fastLoopCost)
			fastLoopCost = curMiliSec;

		//
#if WATCH_DOG_ENABLED
		HAL_IWDG_Refresh(&iwdg);
#endif
    }

	return 0;
}
