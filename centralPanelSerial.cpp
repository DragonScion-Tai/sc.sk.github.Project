#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

#define LOG_MODULE 0x02

#define COMMAND_DELIMETER           '#'

// dowload command
#define CMD_FILTER_RINSE_FIXTIME    101
#define CMD_REBOOT_CENTRAL_PANEL    102
#define CMD_SETTING                 103
#define CMD_SET_TIME                104
#define CMD_USER_CARD_STATE         105
#define CMD_APP_BALANCE             106
#define CMD_SWITCH_LOCK             107
#define CMD_WATER_SWITCH            110
#define CMD_FEED_WATER_SWITCH       111
#define CMD_DRAIN_FIXTIME           112
#define CMD_FILTER_RINSE_PLAN       114
#define CMD_DRAIN_PLAN              115
#define CMD_GET_VERSION             116
#define CMD_GET_SETTINGS            117
#define CMD_SET_DEVICE_ID           118
#define CMD_BLE_DEBUG               119
#define CMD_WRONG_COMMAND           120
#define CMD_MEMBRANE_RINSE_FIXTIME  121
#define CMD_MEMBRANE_RINSE_PLAN     122
#define CMD_DRAIN                   123
#define CMD_MEMBRANE_RINSE          124
#define CMD_FILTER_RINSE            125
#define CMD_GET_DEVICE_ID           126
#define CMD_WECHAT_PAY              127
#define CMD_SET_CP_VER              130
#define CMD_FIRMWARE_DL             131
#define CMD_FIRMWARE_INSTALL        132
#define CMD_FIRMWARE_REVERT         133
#define CMD_GET_LOGFILE             134
#define CMD_START_PROD_TEST         135
#define CMD_DISINFECT_FIXTIME       136
#define CMD_DISINFECT_PLAN          137
#define CMD_DISINFECT               138
#define CMD_LOCK_USERCARD           139
#define CMD_FUNCTION_CANCEL         140
#define CMD_GET_STATUS              141
#define CMD_SET_ICCARD_BALANCE      142
#define CMD_GET_CONFIG_PARAM        143
#define CMD_SET_CONFIG_PARAM        144
#define CMD_SET_CARD_REGION         145
#define CMD_SET_DEVICE_TYPE         146
#define CMD_SET_HOTWATER_RESTRICT   147
#define CMD_STOP_PROD_TEST  	    148
#define CMD_SET_KIDSAFETY  	        149
#define CMD_CLEAR_SETTINGS 	        150
#define CMD_GET_WARNINGS 	        151
#define CMD_SETWANTEDTDS            152
#define CMD_RAW_WATER_TDS           153
#define CMD_MIXED_WATER_TDS         154
#define CMD_GET_MD5                 155

//upload command
#define CMD_APP_WATER_INFO_UPLOAD   201
#define CMD_CARD_BALANCE_UPLOAD     202
#define CMD_CARD_WATER_INFO_UPLOAD  203
#define CMD_WATER_QUALITY_UPLOAD    204
#define CMD_WARNING_UPLOAD          205
#define CMD_FILTER_INFO_UPLOAD      206
#define CMD_SETTINGS_UPLOAD         207
#define CMD_FILTERRINSESTART        208
#define CMD_FILTERRINSEEND          209
#define CMD_DRAINING                210
#define CMD_DRAINEND                211
#define CMD_WATERTANKSTATUS         213
#define CMD_WATERSTATUS             214
#define CMD_COUNTDOWN               216
#define CMD_FILTERWORKABLEDAYS      217
#define CMD_MEMBRANERINSESTART      219
#define CMD_MEMBRANERINSEEND        220
#define CMD_DEVICEID_UPLOAD         221
#define CMD_VERSION                 230
#define CMD_QRCODE                  231
#define CMD_INFRAREDSENSORWORKS     232
#define CMD_INFRAREDSENSORIDLE      233
#define CMD_FIRMWAREUPDATESTATUS    234
#define CMD_OPERATIONLOGSTATUS      235
#define CMD_OPERATIONLOG            236
#define CMD_PRODUCTTESTSTATUS       237
#define CMD_PRODUCTTESTCOUNTDOWN    238
#define CMD_DISINFECTSTATUS         239
#define CMD_REQUESTSYNCSYSTIME      240
#define CMD_STATE_VALVE             241
#define CMD_CONFIGPARAMETER         243
#define CMD_WARNINGS 	            244
#define CMD_STEERING_ANGLE          245
#define CMD_MD5_VALUE_UPLOAD        246

//setting sub command
#define SUBCMD_TEMPERATURE			1030
#define SUBCMD_PRICE				1033
#define SUBCMD_FILTER_LIMIT			1034
#define SUBCMD_PLAN_DELAY_TIME		1035
#define SUBCMD_FILTER_CUR			1036
#define SUBCMD_FILTER_PRODUCE_DATE 	1037
#define SUBCMD_FILTER_RINSE_DUR 	1038
#define SUBCMD_CHARGE_WAY			1039
#define SUBCMD_MEMBRANE_RINSE_DUR 	1040
#define SUBCMD_SERVICE_TIME			1041
#define SUBCMD_FILTER_SHELFLIVE	 	1042
#define SUBCMD_COMPENSATION_TEMP	1043
#define SUBCMD_HEATER_WORK_TIME 	1044
#define SUBCMD_DISINFECT_DUR		1045
#define SUBCMD_FILTER_RINSE_FIXTIME	1050
#define SUBCMD_FILTER_RINSE_PLAN	1051
#define SUBCMD_DRAIN_FIXTIME		1052
#define SUBCMD_DRAIN_PLAN			1053
#define SUBCMD_MEMBRANE_RINSE_FIXTIME  1054
#define SUBCMD_MEMBRANE_RINSE_PLAN	   1055
#define SUBCMD_DISINFECT_FIXTIME	   1056
#define SUBCMD_DISINFECT_PLAN		   1057
#define SUBCMD_FILTER_CHECK_MODE	   1058
#define SUBCMD_DISINFECT_DELAY_TIME	   1059
#define SUBCMD_PULSE_PER_LITRE1		   1061
#define SUBCMD_FILTER_RINSE_FIXTIMES   1062
#define SUBCMD_MEMBRANE_RINSE_FIXTIMES 1063
#define SUBCMD_DISINFECT_FIXTIMES	   1064

CentralPanelSerial* CentralPanelSerial::pThis = NULL;
CentralPanelSerial* CentralPanelSerial::getInstance()
{
    return pThis;
}

CentralPanelSerial::CentralPanelSerial(RawSerial* ps)
    : m_sendId(128)
    ,m_pCmd(NULL)
{
    pThis = this;

    m_pPort = ps;

    if(!m_pPort)
    {
        m_pPort = new RawSerial(CENTRALPANEL_SERIAL_TX, CENTRALPANEL_SERIAL_RX);
    }

    if(m_pPort)
    {
        m_pPort->baud(115200);
        m_pPort->format(8, SerialBase::None, 1);

        m_pPort->attach(Callback<void()>(pThis, &CentralPanelSerial::recv_interrupt), SerialBase::RxIrq);
    }
}

CentralPanelSerial::~CentralPanelSerial()
{

}

int CentralPanelSerial::process()
{
    int retVal = 0;

#if DUMP_COMMAND_PACKET
    if(dataSize())
    {
        FTRACE("###CentralPanelSerial::parse() dataSize: %d\r\n", dataSize());
        dumpData();
    }
#endif
    while(dataSize() > 0)
    {
        switch(m_status)
        {
            case C_PARSE_SYNC_START:
                {
                    char c;
                    pop(c);

                    if(c != COMMAND_DELIMETER)
                    {
                        FTRACE("###CentralPanelSerial::process() 0x%02X is not Sync byte '#'\r\n", c);
                        m_status = C_PARSE_SYNC_ERROR;
                        continue;
                    }
                    m_chksum = 0; // start to calculate checksum
                    addcheck(&c);
                    m_status = C_PARSE_CMD;
                }
                break;
            case C_PARSE_CMD:
                {
                    int c;
                    retVal = popInteger(c);

                    //if(retVal < 0)
                    {
                        if(retVal == Command::CMD_PARSE_NEED_DATA)
                            break;
                        else  if(retVal == Command::CMD_PARSE_FAIL)
                            goto WRONG_CMD;
                    }
                    m_pCmd = createCmd(c);
                    if(!m_pCmd)
                    {
                        // out of memory
                        goto WRONG_CMD;
                    }
                    if(c == CMD_SETTING || c == CMD_SET_CONFIG_PARAM)
                        m_status = C_PARSE_SUB_CMD;
                    else
                        m_status = C_PARSE_PARAM;
                }
                break;
            case C_PARSE_SUB_CMD:
                {
                    if(m_pCmd)
                    {
                        retVal = m_pCmd->parseSubCmd();
                        if(retVal == Command::CMD_PARSE_NEED_DATA)
                            break;
                        else if(retVal == Command::CMD_PARSE_FAIL)
                            goto WRONG_CMD;
                        m_status = C_PARSE_PARAM;
                    }
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
                    m_status = C_PARSE_CHECKSUM;
                }
                break;
            case C_PARSE_CHECKSUM:
                {
                    char c;
                    if(!m_pCmd)
                        goto WRONG_CMD;

                    // SHOULD has three bytes at least
                    // one byte SEND_ID, one byte CHECK_SUM, one byte TERMINAL(0x0A)
                    if(dataSize() < 3)
                    {
                        retVal = Command::CMD_PARSE_NEED_DATA;
                        break;
                    }

                    pop(c);
                    addcheck(&c);
                    m_pCmd->sId = c;

                    pop(c);
                    if(m_chksum != c)
                    {
                        FTRACE("###CentralPanelSerial::parse() checksum failed 0x%X is not (0x%X)\r\n", c, m_chksum);

                        // checksum failed
                        /*sendResponse(m_pCmd->getCommandId(), 2, m_pCmd->sId);

                        retVal = 0;
                        delete m_pCmd;
                        m_pCmd = NULL;
                        m_status = C_PARSE_SYNC_START;

                        break;*/
                    }

                    // there should be a '\n' char at end; remove it
                    pop(c);

                    m_status = C_PARSE_FINISH;
                }
                //break; //no break; go through
            case C_PARSE_FINISH:
                {
                    if(!m_pCmd)
                        goto WRONG_CMD;

                    retVal = m_pCmd->exec();
                    if(retVal != Command::CMD_EXEC_OK)
                    {
                        //response failed?
                    }

                    retVal = 0;
                    delete m_pCmd;
                    m_pCmd = NULL;
                    m_status = C_PARSE_SYNC_START;

                    // next command processed in next loop;
                    processEnd();
                    return 0;
                }
                break;
            case C_PARSE_SYNC_ERROR:
                {
                    // miss synced; drop to next sync start;
                    char c;
                    pop(c);

                    if((unsigned char)c == 0x0A || (unsigned char)c == 0x0D)
                        m_status = C_PARSE_SYNC_START;
                    else
                        continue;
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

int CentralPanelSerial::sendFormatMsg(const char* fmt, ...)
{
    char msg[64];
    int len;
    va_list arg;

    va_start(arg, fmt);
    len = vsnprintf(msg, 64, fmt, arg);
    va_end(arg);

    msg[len++] = m_sendId;
    m_chksum = 0;
    msg[len] = addcheck(msg, len);
    len ++;
    msg[len++] = 0x0A; // '\n'

    sendData(msg, len);
    return 0;
}

int CentralPanelSerial::uploadAppWaterInfo(int coldcost, int coldcap, int hotcost, int hotcap, int bOn)
{
    char msg[64];
    int len = snprintf(msg, 64, "#%d#%d#%d#%d#%d#%d#",
                        CMD_APP_WATER_INFO_UPLOAD, coldcost, coldcap, hotcost, hotcap, bOn);
    if(len < 0)
        return -1;

    msg[len++] = m_sendId;
    m_chksum = 0;
    msg[len] = addcheck(msg, len);
    len ++;
    msg[len++] = 0x0A; // '\n'

    sendData(msg, len);

    return 0;
}

int CentralPanelSerial::uploadCardBalance(int cardId, int balance, int region, int bLocked, int cardType, int index)
{
	char msg[64];
	int len = snprintf(msg, 64, "#%d#%u#%d#%08u#%d#%d#%d#", CMD_CARD_BALANCE_UPLOAD, cardId, balance, region, bLocked, cardType, index);
	if(len < 0)
		return -1;

    msg[len++] = m_sendId;
    m_chksum = 0;
    msg[len] = addcheck(msg, len);
    len ++;
    msg[len++] = 0x0A; // '\n'

    sendData(msg, len);

    return 0;
}

int CentralPanelSerial::uploadCardWaterInfo(int coldcost, int coldcap, int hotcost, int hotcap, int balance, int bOn)
{
    char msg[64];
    int len = snprintf(msg, 64, "#%d#%d#%d#%d#%d#%d#%d#",
                        CMD_CARD_WATER_INFO_UPLOAD, coldcost, coldcap, hotcost, hotcap, balance, bOn);
    if(len < 0)
        return -1;

    msg[len++] = m_sendId;
    m_chksum = 0;
    msg[len] = addcheck(msg, len);
    len ++;
    msg[len++] = 0x0A; // '\n'

    sendData(msg, len);

    return 0;
}

int CentralPanelSerial::uploadWaterQuality(int type, int value)
{
    return sendFormatMsg("#%d#%d#%d#", CMD_WATER_QUALITY_UPLOAD, type, value);
}

int CentralPanelSerial::uploadWarningType(int type, int bOn)
{
    char msg[64];
    int len = snprintf(msg, 64, "#%d#%d#%d#", CMD_WARNING_UPLOAD, type, bOn);
    if(len < 0)
        return -1;

    msg[len++] = m_sendId;
    m_chksum = 0;
    msg[len] = addcheck(msg, len);
    len ++;
    msg[len++] = 0x0A; // '\n'

    sendData(msg, len);

    return 0;
}

int CentralPanelSerial::uploadFilterInfo(int curton, int curlitre, int limitton, int limitlitre)
{
    char msg[64];
    int len = snprintf(msg, 64, "#%d#%d#%d#%d#%d#",
                        CMD_FILTER_INFO_UPLOAD, curton, curlitre, limitton, limitlitre);
    if(len < 0)
        return -1;

    msg[len++] = m_sendId;
    m_chksum = 0;
    msg[len] = addcheck(msg, len);
    len ++;
    msg[len++] = 0x0A; // '\n'

    sendData(msg, len);

    return 0;
}

int CentralPanelSerial::uploadFilterRinseStartEvent()
{
    LOG_INFO("###CentralPanelSerial::uploadFilterRinseStartEvent()\r\n");
    return sendFormatMsg("#%d#", CMD_FILTERRINSESTART);
}

int CentralPanelSerial::uploadFilterRinseEndEvent()
{
    LOG_INFO("###CentralPanelSerial::uploadFilterRinseEndEvent()\r\n");
    return sendFormatMsg("#%d#", CMD_FILTERRINSEEND);
}

int CentralPanelSerial::uploadDrainingEvent()
{
    LOG_INFO("###CentralPanelSerial::uploadDrainingEvent()\r\n");
    return sendFormatMsg("#%d#", CMD_DRAINING);
}

int CentralPanelSerial::uploadDrainEndEvent()
{
    LOG_INFO("###CentralPanelSerial::uploadDrainEndEvent()\r\n");
    return sendFormatMsg("#%d#", CMD_DRAINEND);
}

int CentralPanelSerial::uploadWaterTankInfo(int temp, int cur, int limit)
{
    return sendFormatMsg("#%d#%d#%d#%d#%d#%d#", CMD_WATERTANKSTATUS, temp, cur/1000, cur%1000, limit/1000, limit%1000);
}

int CentralPanelSerial::uploadWaterStatus(int status)
{
    return sendFormatMsg("#%d#%d#", CMD_WATERSTATUS, status);
}

int CentralPanelSerial::uploadCountdown(int cnt)
{
    return sendFormatMsg("#%d#%d#", CMD_COUNTDOWN, cnt);
}

int CentralPanelSerial::uploadFilterWorkableDays(int days)
{
    return sendFormatMsg("#%d#%d#", CMD_FILTERWORKABLEDAYS, days);
}

int CentralPanelSerial::uploadVersion(int major, int minor, int build)
{
	return sendFormatMsg("#%d#%d.%d.%d#", CMD_VERSION, major, minor, build);
}

int CentralPanelSerial::uploadMembraneRinseStartEvent()
{
    LOG_INFO("###CentralPanelSerial::uploadMembraneRinseStartEvent()\r\n");
    return sendFormatMsg("#%d#", CMD_MEMBRANERINSESTART);
}

int CentralPanelSerial::uploadMembraneRinseEndEvent()
{
    LOG_INFO("###CentralPanelSerial::uploadMembraneRinseEndEvent()\r\n");
    return sendFormatMsg("#%d#", CMD_MEMBRANERINSEEND);
}

int CentralPanelSerial::uploadDeviceId(const char* devId)
{
	return sendFormatMsg("#%d#%s#", CMD_DEVICEID_UPLOAD, devId);
}

int CentralPanelSerial::uploadQRCode(const char* code, int length)
{
    (void)length;
    //return sendFormatMsg("#%d#%s#", CMD_QRCODE, code);

    char msg[268]; // 256+12
    int len = snprintf(msg, 268, "#%d#%s#", CMD_QRCODE, code);
    if(len < 0)
        return -1;

    msg[len++] = m_sendId;
    m_chksum = 0;
    msg[len] = addcheck(msg, len);
    len ++;
    msg[len++] = 0x0A; // '\n'
    
	sendData(msg, len);

	return 0;
}

int CentralPanelSerial::uploadInfraredSensorWorksEvent()
{
    return sendFormatMsg("#%d#", CMD_INFRAREDSENSORWORKS);
}

int CentralPanelSerial::uploadInfraredSensorIdleEvent()
{
    return sendFormatMsg("#%d#", CMD_INFRAREDSENSORIDLE);
}

int CentralPanelSerial::uploadFirmwareInstallStatus(int status)
{
    return sendFormatMsg("#%d#%d#", CMD_FIRMWAREUPDATESTATUS, status);
}

int CentralPanelSerial::uploadOperationLogStatus(int status)
{
    return sendFormatMsg("#%d#%d#", CMD_OPERATIONLOGSTATUS, status);
}

int CentralPanelSerial::uploadOperationLog(int tm, int mod, int p1, int p2)
{
    return sendFormatMsg("#%d#%X#%X#%X#%X#", CMD_OPERATIONLOG, tm, mod, p1, p2);
}

int CentralPanelSerial::uploadProductTestStatus(int status)
{
    return sendFormatMsg("#%d#%d#", CMD_PRODUCTTESTSTATUS, status);
}

int CentralPanelSerial::uploadProductTestCountdown(int cnt)
{
    return sendFormatMsg("#%d#%d#", CMD_PRODUCTTESTCOUNTDOWN, cnt);
}

int CentralPanelSerial::uploadDisinfectStatus(int status)
{
    LOG_INFO("###CentralPanelSerial::uploadDisinfectStatus() status: %d\r\n", status);
    return sendFormatMsg("#%d#%d#", CMD_DISINFECTSTATUS, status);
}

int CentralPanelSerial::uploadRequestSyncSysTime()
{
    return sendFormatMsg("#%d#", CMD_REQUESTSYNCSYSTIME);
}

int CentralPanelSerial::uploadValveState(int B3, int cold,int hot,int ice)
{
    return sendFormatMsg("#%d#%d#%d#%d#%d#",CMD_STATE_VALVE,B3,cold,hot,ice);
}

int CentralPanelSerial::uploadConfigParamSecret(const char* secret)
{
    int i=0;
    char secret_terminal[65] = {0};
    for(i=0; i<64; i++)
    {
        if(secret[i] != '\0')
            secret_terminal[i] = secret[i];
    }
    return sendFormatMsg("#%d#0#%s#", CMD_CONFIGPARAMETER, secret_terminal);
}

int CentralPanelSerial::uploadConfigParam(int bHP, int bQR, int bDH, int bIC, int bSKII)
{
    return sendFormatMsg("#%d#1#%d#%d#%d#%d#%d#", CMD_CONFIGPARAMETER, bHP, bQR, bDH, bIC, bSKII);
}

char new_temp[6];
char new_msg[150];
int CentralPanelSerial::uploadWarnings(void)
{
    int last_index;
    int len=0x00;
    int data_len=0x00;
    int new_buf[27];
    
    WarningManager::getInstance()->getWarnings(new_buf);
    
    new_msg[len++] = '#';
    data_len = snprintf(new_temp, 10, "%d", CMD_WARNINGS); 
    new_msg[len]   = '\0';
    strcat(new_msg, new_temp);
    len += data_len;
    new_msg[len++] = '#';
    
    for(last_index=0; last_index<27; last_index++)
    {
        if(0xFFFF == new_buf[last_index]) break;
        else
        {
            data_len = snprintf(new_temp, 10, "%d", new_buf[last_index]);            
            if(data_len)
            {
                new_msg[len] = '\0';
                strcat(new_msg, new_temp);
                len += data_len;
                new_msg[len++] = '#';
            }
        }
    }
    
    if(0xFFFF == new_buf[0])
    {
        new_msg[len++] = '0';
        new_msg[len++] = '#';
    }

    new_msg[len++] = m_sendId;
    m_chksum = 0;
    new_msg[len]   = addcheck(new_msg, len);
    len++;
    new_msg[len++] = 0x0A; // '\n'
    sendData(new_msg, len);

    return 0;
}

int CentralPanelSerial::uploadSteeringAngle(int angle)
{
    return sendFormatMsg("#%d#%d#", CMD_STEERING_ANGLE, angle);
}

int CentralPanelSerial::uploadMD5Value(const char* md5Value)
{
    return sendFormatMsg("#%d#%s#", CMD_MD5_VALUE_UPLOAD, md5Value);
}

int CentralPanelSerial::uploadTemperature(int temp)
{
    return sendFormatMsg("#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_TEMPERATURE, temp);
}

int CentralPanelSerial::uploadWaterPrice(int hot, int cold, int freeze)
{
    return sendFormatMsg("#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_PRICE, cold, hot, freeze);
}

int CentralPanelSerial::uploadFilterLimit(int litre)
{
    return sendFormatMsg("#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_FILTER_LIMIT, litre/1000, litre%1000);
}

int CentralPanelSerial::uploadPlanDelayTime(int min)
{
    return sendFormatMsg("#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_PLAN_DELAY_TIME, min);
}

int CentralPanelSerial::uploadFilterCur(int litre)
{
    return sendFormatMsg("#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_FILTER_CUR, litre/1000, litre%1000);
}

int CentralPanelSerial::uploadFilterProduceDate(int y, int m, int d)
{
    return sendFormatMsg("#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_FILTER_PRODUCE_DATE, y, m, d);
}

int CentralPanelSerial::uploadFilterRinseDur(int backmin, int fastmin)
{
    return sendFormatMsg("#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_FILTER_RINSE_DUR, fastmin, backmin);
}

int CentralPanelSerial::uploadChargeWay(int model, int unit)
{
    return sendFormatMsg("#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_CHARGE_WAY, model, unit);
}

int CentralPanelSerial::uploadMembraneRinseDur(int min)
{
    return sendFormatMsg("#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_MEMBRANE_RINSE_DUR, min);
}

int CentralPanelSerial::uploadServiceTime(int day, short start[4], short end[4])
{
	if(start[0] == 0 && end[0] == 2400)
    {
		sendFormatMsg("#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_SERVICE_TIME, day, 0, 0);
    }
	else if(start[0] == 0 && end[0] == 0)
    {
		sendFormatMsg("#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_SERVICE_TIME, day);
    }
	else
	{
		if(start[1] == 0 && end[1] == 0)
        {
			sendFormatMsg("#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_SERVICE_TIME, day, start[0], end[0]);
        }
		else if(start[2] == 0 && end[2] == 0)
        {
			sendFormatMsg("#%d#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_SERVICE_TIME, day, start[0], end[0], start[1], end[1]);
        }
		else if(start[3] == 0 && end[3] == 0)
        {
			sendFormatMsg("#%d#%d#%d#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_SERVICE_TIME, day, start[0], end[0],
							start[1], end[1], start[2], end[2]);
        }
		else
        {
			sendFormatMsg("#%d#%d#%d#%d#%d#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_SERVICE_TIME, day, start[0], end[0],
							start[1], end[1], start[2], end[2], start[3], end[3]);
        }
	}

	return 0;
}

int CentralPanelSerial::uploadFilterShelflive(int month)
{
    return sendFormatMsg("#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_FILTER_SHELFLIVE, month);
}

int CentralPanelSerial::uploadCompensationTemp(int start, int comp)
{
    return sendFormatMsg("#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_COMPENSATION_TEMP, comp, start);
}

int CentralPanelSerial::uploadHeaterWorkTime(int day, short start[4], short end[4])
{
	if(start[0] == 0 && end[0] == 2400)
    {
		sendFormatMsg("#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_HEATER_WORK_TIME, day, 0, 0);
    }
	else if(start[0] == 0 && end[0] == 0)
    {
		sendFormatMsg("#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_HEATER_WORK_TIME, day);
    }
	else
	{
		if(start[1] == 0 && end[1] == 0)
        {
			sendFormatMsg("#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_HEATER_WORK_TIME, day, start[0], end[0]);
        }
		else if(start[2] == 0 && end[2] == 0)
        {
			sendFormatMsg("#%d#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_HEATER_WORK_TIME, day, start[0], end[0], 
							start[1], end[1]);
        }
		else if(start[3] == 0 && end[3] == 0)
        {
			sendFormatMsg("#%d#%d#%d#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_HEATER_WORK_TIME, day, start[0], end[0],
							start[1], end[1], start[2], end[2]);
        }
		else
        {
			sendFormatMsg("#%d#%d#%d#%d#%d#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_HEATER_WORK_TIME, day, start[0], end[0],
							start[1], end[1], start[2], end[2], start[3], end[3]);
        }
	}

	return 0;
}

int CentralPanelSerial::uploadDisinfectDur(int min)
{
    return sendFormatMsg("#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_DISINFECT_DUR, min);
}

int CentralPanelSerial::uploadPulsePerLitre(int totalpulse, int hotpulse, int coldpulse, int icepulse, int pulse5, int pulse6)
{
    return sendFormatMsg("#%d#%d#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_PULSE_PER_LITRE1, totalpulse, hotpulse, coldpulse, icepulse, pulse5, pulse6);
}

int CentralPanelSerial::uploadFilterRinseFixtime(int day, int hour, int min)
{
    return sendFormatMsg("#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_FILTER_RINSE_FIXTIME, day, hour, min);
}

int CentralPanelSerial::uploadDrainFixtime(int day, int hour, int min)
{
    (void)min;
    return sendFormatMsg("#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_DRAIN_FIXTIME, day, hour, min);
}

int CentralPanelSerial::uploadMembraneRinseFixtime(int day, int hour, int min)
{
    return sendFormatMsg("#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_MEMBRANE_RINSE_FIXTIME, day, hour, min);
}

int CentralPanelSerial::uploadDisinfectFixtime(int day, int hour, int min)
{
    return sendFormatMsg("#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_DISINFECT_FIXTIME, day, hour, min);
}

int CentralPanelSerial::uploadFilterRinsePlan(int y, int m, int d, int h, int min)
{
    return sendFormatMsg("#%d#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_FILTER_RINSE_PLAN, y, m, d, h, min);
}

int CentralPanelSerial::uploadDrainPlan(int y, int m, int d, int h, int min)
{
    return sendFormatMsg("#%d#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_DRAIN_PLAN, y, m, d, h, min);
}

int CentralPanelSerial::uploadMembraneRinsePlan(int y, int m, int d, int h, int min)
{
    return sendFormatMsg("#%d#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_MEMBRANE_RINSE_PLAN, y, m, d, h, min);
}

int CentralPanelSerial::uploadDisinfectPlan(int y, int m, int d, int h, int min)
{
    return sendFormatMsg("#%d#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_DISINFECT_PLAN, y, m, d, h, min);
}

int CentralPanelSerial::uploadFilterCheckMode(int mode)
{
    return sendFormatMsg("#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_FILTER_CHECK_MODE, mode);
}

int CentralPanelSerial::uploadDisinfectDelayTime(int min)
{
    return sendFormatMsg("#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_DISINFECT_DELAY_TIME, min);
}

int CentralPanelSerial::uploadFilterRinseFixtimes(int index, int day, int hour, int min)
{
	return sendFormatMsg("#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_FILTER_RINSE_FIXTIMES, index, day, hour, min);
}

int CentralPanelSerial::uploadMembraneRinseFixtimes(int index, int day, int hour, int min)
{
	return sendFormatMsg("#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_MEMBRANE_RINSE_FIXTIMES, index, day, hour, min);
}

int CentralPanelSerial::uploadDisinfectFixtimes(int index, int day, int hour, int min)
{
	return sendFormatMsg("#%d#%d#%d#%d#%d#%d#", CMD_SETTINGS_UPLOAD, SUBCMD_DISINFECT_FIXTIMES, index, day, hour, min);
}

int CentralPanelSerial::uploadSettinsFinished()
{
    return sendFormatMsg("#%d#%d#", CMD_SETTINGS_UPLOAD, 0);
}

//private
CommandProcess::Command* CentralPanelSerial::createCmd(int c)
{
    switch(c)
    {
        case CMD_FILTER_RINSE_FIXTIME:
            return new CMDFilterRinseFixTime(this);
        case CMD_REBOOT_CENTRAL_PANEL:
            return new CMDRebootCentralPanel(this);
        case CMD_SETTING:
            return new CMDSetting(this);
        case CMD_SET_TIME:
            return new CMDSetTime(this);
        case CMD_USER_CARD_STATE:
            return new CMDUserCardState(this);
        case CMD_APP_BALANCE:
            return new CMDAppBalance(this);
        case CMD_SWITCH_LOCK:
            return new CMDSwitchLock(this);
        case CMD_WATER_SWITCH:
            return new CMDWaterSwitch(this);
        case CMD_FEED_WATER_SWITCH:
            return new CMDFeedWaterSwitch(this);
        case CMD_DRAIN_FIXTIME:
            return new CMDDrainFixtime(this);
        case CMD_FILTER_RINSE_PLAN:
            return new CMDFilterRinsePlan(this);
        case CMD_DRAIN_PLAN:
            return new CMDDrainPlan(this);
        case CMD_GET_VERSION:
            return new CMDGetVersion(this);
        case CMD_GET_SETTINGS:
            return new CMDGetSettings(this);
        case CMD_SET_DEVICE_ID:
            return new CMDSetDeviceId(this);
        case CMD_BLE_DEBUG:
            return new CMDEnableBLEDebug(this);
        case CMD_WRONG_COMMAND:
            return new CMDWrongCommand(this);
        case CMD_MEMBRANE_RINSE_FIXTIME:
            return new CMDMembraneRinseFixTime(this);
        case CMD_MEMBRANE_RINSE_PLAN:
            return new CMDMembraneRinsePlan(this);
        case CMD_DRAIN:
            return new CMDDrain(this);
        case CMD_MEMBRANE_RINSE:
            return new CMDMembraneRinse(this);
        case CMD_FILTER_RINSE:
            return new CMDFilterRinse(this);
        case CMD_GET_DEVICE_ID:
            return new CMDGetDeviceId(this);
        case CMD_WECHAT_PAY:
            return new CMDWechatAppPay(this);
        case CMD_SET_CP_VER:
            return new CMDSetCPVersion(this);
        case CMD_FIRMWARE_DL:
            return new CMDDownloadFw(this);
        case CMD_FIRMWARE_INSTALL:
            return new CMDInstallFw(this);
        case CMD_FIRMWARE_REVERT:
            return new CMDRevertFw(this);
        case CMD_GET_LOGFILE:
            return new CMDGetLogFile(this);
        case CMD_START_PROD_TEST:
            return new CMDStartProdTest(this);
        case CMD_STOP_PROD_TEST:
            return new CMDStopProdTest(this);
        case CMD_DISINFECT_FIXTIME:
            return new CMDDisinfectFixTime(this);
        case CMD_DISINFECT_PLAN:
            return new CMDDisinfectPlan(this);
        case CMD_DISINFECT:
            return new CMDDisinfect(this);
        case CMD_LOCK_USERCARD:
            return new CMDLockUsercard(this);
        case CMD_FUNCTION_CANCEL:
            return new CMDFunctionCancel(this);
        case CMD_GET_STATUS:
            return new CMDGetStatus(this);
        case CMD_SET_ICCARD_BALANCE:
            return new CMDSetICCardBalance(this);
        case CMD_GET_CONFIG_PARAM:
            return new CMDGetConfigParam(this);
        case CMD_SET_CONFIG_PARAM:
            return new CMDSetConfigParam(this);
        case CMD_SET_CARD_REGION:
            return new CMDSetCardRegion(this);
        case CMD_SET_DEVICE_TYPE:
            return new CMDSetDeviceType(this);
        case CMD_SET_HOTWATER_RESTRICT:
            return new CMDSetHotWaterRestrict(this);
	    case CMD_SET_KIDSAFETY:
            return new CMDSetKidSafety(this);
        case CMD_CLEAR_SETTINGS:
            return new CMDClearSettings(this);
        case CMD_GET_WARNINGS:
            return new CMDGetWarnings(this);
#ifdef TDS_CONTROL_SWITCH
        case CMD_SETWANTEDTDS:
            return new CMDSetWantedTDS(this);
        case CMD_RAW_WATER_TDS:
            return new CMDRawWaterTDS(this);
        case CMD_MIXED_WATER_TDS:
            return new CMDMixedWaterTDS(this);
#endif
        case CMD_GET_MD5:
            return new CMDGetMD5Value(this);
		case CMD_APP_WATER_INFO_UPLOAD:
		case CMD_CARD_BALANCE_UPLOAD:
		case CMD_CARD_WATER_INFO_UPLOAD:
		case CMD_WATER_QUALITY_UPLOAD:
		case CMD_WARNING_UPLOAD:
		case CMD_FILTER_INFO_UPLOAD:
		case CMD_SETTINGS_UPLOAD:
		case CMD_FILTERRINSESTART:
		case CMD_FILTERRINSEEND:
		case CMD_DRAINING:
		case CMD_DRAINEND:
		case CMD_WATERTANKSTATUS:
		case CMD_WATERSTATUS:
		case CMD_COUNTDOWN:
		case CMD_FILTERWORKABLEDAYS:
		case CMD_VERSION:
		case CMD_MEMBRANERINSESTART:
		case CMD_MEMBRANERINSEEND:
		case CMD_DEVICEID_UPLOAD:
		case CMD_QRCODE:
		case CMD_INFRAREDSENSORWORKS:
		case CMD_INFRAREDSENSORIDLE:
		case CMD_FIRMWAREUPDATESTATUS:
		case CMD_OPERATIONLOGSTATUS:
		case CMD_OPERATIONLOG:
		case CMD_PRODUCTTESTSTATUS:
		case CMD_PRODUCTTESTCOUNTDOWN:
		case CMD_DISINFECTSTATUS:
		case CMD_REQUESTSYNCSYSTIME:
        case CMD_STATE_VALVE:
        case CMD_CONFIGPARAMETER:
        case CMD_WARNINGS:
        case CMD_STEERING_ANGLE:
        case CMD_MD5_VALUE_UPLOAD:
            {
                CMDResponse* pCmd = new CMDResponse(this);
                if(!pCmd)
                    return NULL;
                pCmd->cmd = c;

                return pCmd;
            }
        default:
            FTRACE("###CentralPanelSerial::createCmd() unknown cmd: 0x%X\r\n", c);
            break;
    }

    return NULL;
}

int CentralPanelSerial::popInteger(int& val)
{
    if(hasNext())
    {
        char c;
        val = 0;
        pop(c);
        addcheck(&c);
        while(c != COMMAND_DELIMETER)
        {
            if(c > '9' || c < '0')
            {
                FTRACE("###CentralPanelSerial::popInteger() WRONG COMMAND 0x%X\r\n", c);
                val = 0; // reset if error occur
                return Command::CMD_PARSE_FAIL; // not integer
            }

            val *= 10;
            val += c - '0';

            pop(c);
            addcheck(&c);
        }

        return Command::CMD_PARSE_OK;
    }

    return Command::CMD_PARSE_NEED_DATA;
}

int CentralPanelSerial::popString(char* str, int size)
{
    char c;
    int pos;
    if(!str || size <= 0)
        return -1;

    //pop string
    pos = 0;
    while(pop(c) == 0)
    {
        addcheck(&c);
        if(c == COMMAND_DELIMETER)
            break;

        str[pos++] = c;
        if(pos >= size)
            break;
    }

    if(pos < size)
        str[pos] = '\0';

    return pos;
}

int CentralPanelSerial::addcheck(const char* c, int size)
{
    int i;
    if(!c) return m_chksum;

    for(i=0; i<size; i++)
        m_chksum ^= c[i];

    return m_chksum;
}

int CentralPanelSerial::sendResponse(int cmd, char status, char sendId)
{
    char msg[32];
    int len = snprintf(msg, 32, "#%d#%d#", cmd, status);

    msg[len++] = sendId;
    m_chksum = 0;
    msg[len] = addcheck(msg, len);
    len ++;
    msg[len++] = 0x0A; // '\n'

    sendData(msg, len);

    return 0;
}

//CMDFilterRinseFixTime
int CentralPanelSerial::CMDFilterRinseFixTime::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 3))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(day);
    proc->popInteger(hour);
    proc->popInteger(minute);

    return 0;
}

int CentralPanelSerial::CMDFilterRinseFixTime::exec()
{
    Controller::getInstance()->setFilterRinseFixTime(day, hour, minute);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_FILTER_RINSE_FIXTIME, 0, sId);
    return 0;
}

//CMDRebootCentralPanel
int CentralPanelSerial::CMDRebootCentralPanel::parse()
{
    int temp = 0;
    proc->popInteger(temp);
    return 0;
}

int CentralPanelSerial::CMDRebootCentralPanel::exec()
{
    FTRACE("###CentralPanelSerial::CMDRebootCentralPanel::exec()\r\n");
    Controller::getInstance()->rebootCentralPanel();

    //((CentralPanelSerial*)proc)->sendResponse(CMD_FILTER_RINSE_FIXTIME, 0, sId);
    return 0;
}

//CMDSetting
int CentralPanelSerial::CMDSetting::parseSubCmd()
{
    return proc->popInteger(subcmd);
}

int CentralPanelSerial::CMDSetting::parse()
{
    switch(subcmd)
    {
    case SUBCMD_TEMPERATURE: // set temperature
        {
            if(!proc->hasNext())
                return Command::CMD_PARSE_NEED_DATA;

            proc->popInteger(param.s1.temp);
        }
        break;
    case SUBCMD_PRICE: // set unit price
        {
            if(!proc->hasNext(COMMAND_DELIMETER, 3))
                return Command::CMD_PARSE_NEED_DATA;

            proc->popInteger(param.s2.coldprice);
            proc->popInteger(param.s2.hotprice);
            proc->popInteger(param.s2.freezeprice);
        }
        break;
    case SUBCMD_FILTER_LIMIT: // set limit of filte water
    case SUBCMD_FILTER_CUR: // set current of filte water
        {
            if(!proc->hasNext(COMMAND_DELIMETER, 2))
                return Command::CMD_PARSE_NEED_DATA;

            proc->popInteger(param.s3.ton);
            proc->popInteger(param.s3.litre);
        }
        break;
    case SUBCMD_PLAN_DELAY_TIME: // set plan delay time
    case SUBCMD_DISINFECT_DELAY_TIME: // set disinfect delay time
    case SUBCMD_MEMBRANE_RINSE_DUR: // set membrane rinse duration
    case SUBCMD_DISINFECT_DUR: // set disinfect duration
        {
            if(!proc->hasNext())
                return Command::CMD_PARSE_NEED_DATA;

            proc->popInteger(param.s4.minutes);
        }
        break;
    case SUBCMD_FILTER_RINSE_DUR: // set filter rinse duration
        {
            if(!proc->hasNext(COMMAND_DELIMETER, 2))
                return Command::CMD_PARSE_NEED_DATA;

            proc->popInteger(param.sb.fastdur);
            proc->popInteger(param.sb.backdur);
        }
        break;
    case SUBCMD_FILTER_PRODUCE_DATE: // set filter produnction date
        {
            if(!proc->hasNext(COMMAND_DELIMETER, 3))
                return Command::CMD_PARSE_NEED_DATA;

            proc->popInteger(param.s5.year);
            proc->popInteger(param.s5.month);
            proc->popInteger(param.s5.day);
        }
        break;
    case SUBCMD_CHARGE_WAY: // set charge way
        {
            if(!proc->hasNext(COMMAND_DELIMETER, 2))
                return Command::CMD_PARSE_NEED_DATA;

            proc->popInteger(param.s6.model);
            proc->popInteger(param.s6.unit);
        }
        break;
    case SUBCMD_SERVICE_TIME: // set service time
    case SUBCMD_HEATER_WORK_TIME: // set water heater service time
        {
            int i = 0;
            // need whole COMMAND data to process
            if(!proc->hasNext('\r', 1) && !proc->hasNext('\n', 1))
                return Command::CMD_PARSE_NEED_DATA;

            proc->popInteger(param.s7.day);

            for(i=0; i<4; i++)
            {
                param.s7.start[i] = 2400;
                param.s7.end[i] = 2400;

                if(proc->hasNext(COMMAND_DELIMETER, 2))
                {
                    proc->popInteger(param.s7.start[i]);
                    proc->popInteger(param.s7.end[i]);
                }
            }

            for(i=0; i<4; i++)
            {
                if(param.s7.start[i]==0 && param.s7.end[i]==0)
                {
                    param.s7.end[i] = 2400;
                }
                if(param.s7.start[i]==2400 && param.s7.end[i]==2400)
                {
                    param.s7.start[i] = 0;
                    param.s7.end[i] = 0;
                }
                //workaround; for error case
                if(param.s7.start[i] > param.s7.end[i] || param.s7.end[i] > 2400)
                {
                    param.s7.start[i] = 0;
                    param.s7.end[i] = 0;
                }
			}
		}
		break;
	case SUBCMD_FILTER_SHELFLIVE: // set shelf life of the filter
		{
			if(!proc->hasNext())
				return Command::CMD_PARSE_NEED_DATA;

			proc->popInteger(param.s8.month);
		}
		break;
	case SUBCMD_COMPENSATION_TEMP: // set compensation temperature
		{
			if(!proc->hasNext(COMMAND_DELIMETER, 2))
				return Command::CMD_PARSE_NEED_DATA;

			proc->popInteger(param.s9.offset);
			proc->popInteger(param.s9.temp);
		}
		break;
	case SUBCMD_PULSE_PER_LITRE1: // set pulse per litre
		{
			if(!proc->hasNext(COMMAND_DELIMETER, 6))
				return Command::CMD_PARSE_NEED_DATA;

			proc->popInteger(param.sa.totalpulse);
			proc->popInteger(param.sa.hotpulse);
			proc->popInteger(param.sa.coldpulse);
			proc->popInteger(param.sa.icepulse);
			proc->popInteger(param.sa.pulse5);
			proc->popInteger(param.sa.pulse6);
		}
		break;
	case SUBCMD_FILTER_RINSE_FIXTIME:
    case SUBCMD_MEMBRANE_RINSE_FIXTIME:
    case SUBCMD_DISINFECT_FIXTIME:
	{
		if(!proc->hasNext(COMMAND_DELIMETER, 3))
			return Command::CMD_PARSE_NEED_DATA;
        
		proc->popInteger(param.sc.day);
		proc->popInteger(param.sc.hour);
		proc->popInteger(param.sc.min);
	}
	break;
	case SUBCMD_FILTER_RINSE_FIXTIMES:
    case SUBCMD_MEMBRANE_RINSE_FIXTIMES:
    case SUBCMD_DISINFECT_FIXTIMES:
	{
		if(!proc->hasNext(COMMAND_DELIMETER, 4))
			return Command::CMD_PARSE_NEED_DATA;

        proc->popInteger(param.sf.index);
		proc->popInteger(param.sf.day);
		proc->popInteger(param.sf.hour);
		proc->popInteger(param.sf.min);
	}
	break;
	case SUBCMD_DRAIN_FIXTIME:
		{
			if(!proc->hasNext(COMMAND_DELIMETER, 3))
				return Command::CMD_PARSE_NEED_DATA;

			proc->popInteger(param.sc.day);
			proc->popInteger(param.sc.hour);
			proc->popInteger(param.sc.min);
		}
		break;
	//case SUBCMD_DRAIN_FIXTIME:
	//	{
	//		if(!proc->hasNext(COMMAND_DELIMETER, 2))
	//			return Command::CMD_PARSE_NEED_DATA;

	//		proc->popInteger(param.sc.day);
	//		proc->popInteger(param.sc.hour);
	//		param.sc.min = 0;
	//	}
	//	break;
	case SUBCMD_FILTER_RINSE_PLAN:
	case SUBCMD_DRAIN_PLAN:
	case SUBCMD_MEMBRANE_RINSE_PLAN:
	case SUBCMD_DISINFECT_PLAN:
		{
			if(!proc->hasNext(COMMAND_DELIMETER, 5))
				return Command::CMD_PARSE_NEED_DATA;

			proc->popInteger(param.sd.year);
			proc->popInteger(param.sd.month);
			proc->popInteger(param.sd.day);
			proc->popInteger(param.sd.hour);
			proc->popInteger(param.sd.min);
		}
		break;
	case SUBCMD_FILTER_CHECK_MODE:
		{
			if(!proc->hasNext(COMMAND_DELIMETER, 1))
				return Command::CMD_PARSE_NEED_DATA;

			proc->popInteger(param.se.mode);
		}
		break;
	default:
		// error
		break;
	}

	return 0;
}

int CentralPanelSerial::CMDSetting::exec()
{
	int retval = 0;
	switch(subcmd)
	{
	case SUBCMD_TEMPERATURE: // set temperature
		retval = Controller::getInstance()->setHeatTemperature(param.s1.temp);
		break;
	case SUBCMD_PRICE: // set unit price
		retval = Controller::getInstance()->setUnitPrice(param.s2.hotprice, param.s2.coldprice, param.s2.freezeprice);
		break;
	case SUBCMD_FILTER_LIMIT: // set limit of filte water
		retval = Controller::getInstance()->setWaterLimit(1000*param.s3.ton + param.s3.litre);
		break;
	case SUBCMD_PLAN_DELAY_TIME: // set plan delay time
		retval = Controller::getInstance()->setDelayTime(param.s4.minutes);
		break;
	case SUBCMD_FILTER_CUR: // set current of filte water
		retval = Controller::getInstance()->setWaterCurrent(1000*param.s3.ton + param.s3.litre);
		break;
	case SUBCMD_FILTER_PRODUCE_DATE: // set filter produnction date
		retval = Controller::getInstance()->setFilterProduceDate(param.s5.year, param.s5.month, param.s5.day);
		break;
	case SUBCMD_FILTER_RINSE_DUR: // set filter rinse duration
		retval = Controller::getInstance()->setFilterRinseDur(param.sb.backdur, param.sb.fastdur);
		break;
	case SUBCMD_CHARGE_WAY: // set charge way
		retval = Controller::getInstance()->setChargeWay(param.s6.model, param.s6.unit);
		break;
	case SUBCMD_MEMBRANE_RINSE_DUR: // set filter rinse duration
		retval = Controller::getInstance()->setMembraneRinseDur(param.s4.minutes);
		break;
	case SUBCMD_SERVICE_TIME: // set service time
		retval = Controller::getInstance()->setServiceTime(param.s7.day, param.s7.start, param.s7.end);
		break;
	case SUBCMD_FILTER_SHELFLIVE: // set shelf life of the filter
		retval = Controller::getInstance()->setFilterShelflife(param.s8.month);
		break;
	case SUBCMD_COMPENSATION_TEMP: // set compensation temperature
		retval = Controller::getInstance()->setCompensationTemp(param.s9.offset, param.s9.temp);
		break;
	case SUBCMD_HEATER_WORK_TIME: // set water heater work time
		retval = Controller::getInstance()->setHeaterWorkTime(param.s7.day, param.s7.start, param.s7.end);
		break;
	case SUBCMD_DISINFECT_DUR: // set disinfect duration
		retval = Controller::getInstance()->setDisinfectDur(param.s4.minutes);
		break;
	case SUBCMD_PULSE_PER_LITRE1: // set pulse per litre
		retval = Controller::getInstance()->setPulsePerLitre(param.sa.totalpulse, param.sa.hotpulse, param.sa.coldpulse, param.sa.icepulse, param.sa.pulse5, param.sa.pulse6);
		break;
	case SUBCMD_FILTER_RINSE_FIXTIME:
		retval = Controller::getInstance()->setFilterRinseFixTime(param.sc.day, param.sc.hour, param.sc.min);
		break;
	case SUBCMD_DRAIN_FIXTIME:
		retval = Controller::getInstance()->setDrainFixTime(param.sc.day, param.sc.hour, param.sc.min);
		break;
	case SUBCMD_MEMBRANE_RINSE_FIXTIME:
		retval = Controller::getInstance()->setMembraneRinseFixTime(param.sc.day, param.sc.hour, param.sc.min);
		break;
	case SUBCMD_DISINFECT_FIXTIME:
		retval = Controller::getInstance()->setDisinfectFixTime(param.sc.day, param.sc.hour, param.sc.min);
		break;
	case SUBCMD_FILTER_RINSE_PLAN:
		retval = Controller::getInstance()->setFilterRinsePlan(param.sd.year, param.sd.month, param.sd.day, param.sd.hour, param.sd.min);
		break;
	case SUBCMD_DRAIN_PLAN:
		retval = Controller::getInstance()->setDrainPlan(param.sd.year, param.sd.month, param.sd.day, param.sd.hour, param.sd.min);
		break;
	case SUBCMD_MEMBRANE_RINSE_PLAN:
		retval = Controller::getInstance()->setMembraneRinsePlan(param.sd.year, param.sd.month, param.sd.day, param.sd.hour, param.sd.min);
		break;
	case SUBCMD_DISINFECT_PLAN:
		retval = Controller::getInstance()->setDisinfectPlan(param.sd.year, param.sd.month, param.sd.day, param.sd.hour, param.sd.min);
		break;
	case SUBCMD_FILTER_CHECK_MODE:
		retval = Controller::getInstance()->setFilterCheckMode(param.se.mode);
		break;
	case SUBCMD_DISINFECT_DELAY_TIME:
		retval = Controller::getInstance()->setDisinfectDelayTime(param.s4.minutes);
		break;
	case SUBCMD_FILTER_RINSE_FIXTIMES:
		retval = Controller::getInstance()->setFilterRinseFixTimes(param.sf.index, param.sf.day, param.sf.hour, param.sf.min);
		break;
	case SUBCMD_MEMBRANE_RINSE_FIXTIMES:
		retval = Controller::getInstance()->setMembraneRinseFixTimes(param.sf.index, param.sf.day, param.sf.hour, param.sf.min);
		break;
	case SUBCMD_DISINFECT_FIXTIMES:
		retval = Controller::getInstance()->setDisinfectFixTimes(param.sf.index, param.sf.day, param.sf.hour, param.sf.min);
		break;
	default:
		// error
		break;
	}

	if(retval == 0)
		((CentralPanelSerial*)proc)->sendResponse(CMD_SETTING, 0, sId); //succeeded
	else
		((CentralPanelSerial*)proc)->sendResponse(CMD_SETTING, 1, sId); //failed
	return 0;
}

//CMDSetTime
int CentralPanelSerial::CMDSetTime::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 6))
        return Command::CMD_PARSE_NEED_DATA;
    
	proc->popInteger(year);
	proc->popInteger(month);
	proc->popInteger(day);
	proc->popInteger(hour);
	proc->popInteger(minute);
	proc->popInteger(second);
    
	return 0;
}

int CentralPanelSerial::CMDSetTime::exec()
{
    Controller::getInstance()->setSysTime(year, month, day, hour, minute, second);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_SET_TIME, 0, sId);
    return 0;
}

//CMDUserCardState
int CentralPanelSerial::CMDUserCardState::parse()
{
	if(!proc->hasNext(COMMAND_DELIMETER, 3))
		return Command::CMD_PARSE_NEED_DATA;

	proc->popInteger(state);
    proc->popInteger(card_id);
    proc->popInteger(card_index);

    return 0;
}

int CentralPanelSerial::CMDUserCardState::exec()
{
    if((!Controller::getInstance()->GetCPUserInfoCardMovingFlag()) 
        || (card_id != Controller::getInstance()->GetCPUserInfoIcCardId()) 
        || (card_index != ActionCardInfo::index)) 
    {
	    ((CentralPanelSerial*)proc)->sendResponse(CMD_USER_CARD_STATE, 1, sId);
    
	    return 1;
    }
    else
    {
        Controller::getInstance()->setUserCardState(state);
	    ((CentralPanelSerial*)proc)->sendResponse(CMD_USER_CARD_STATE, 0, sId);
    
	    return 0;
    }    
}

//CMDAppBalance
int CentralPanelSerial::CMDAppBalance::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(cents);

    return 0;
}

int CentralPanelSerial::CMDAppBalance::exec()
{
    Controller::getInstance()->setAppBalance(cents);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_APP_BALANCE, 0, sId);
    return 0;
}

//CMDSwitchLock
int CentralPanelSerial::CMDSwitchLock::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(bUnlocked);

    return 0;
}

int CentralPanelSerial::CMDSwitchLock::exec()
{
    Controller::getInstance()->setSwichLock(bUnlocked);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_SWITCH_LOCK, 0, sId);
    return 0;
}

//CMDWaterSwitch
int CentralPanelSerial::CMDWaterSwitch::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 2))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(type);
    proc->popInteger(bDisable);

    return 0;
}

int CentralPanelSerial::CMDWaterSwitch::exec()
{
    Controller::getInstance()->enableWaterSwitch(type, bDisable);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_WATER_SWITCH, 0, sId);
    return 0;
}

//CMDFeedWaterSwitch
int CentralPanelSerial::CMDFeedWaterSwitch::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 2))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(switchType);
    proc->popInteger(bOff);

    return 0;
}

int CentralPanelSerial::CMDFeedWaterSwitch::exec()
{
    Controller::getInstance()->openFeedWaterSwitch(switchType, bOff);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_FEED_WATER_SWITCH, 0, sId);
    return 0;
}

//CMDDrainFixtime
int CentralPanelSerial::CMDDrainFixtime::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 2))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(day);
    proc->popInteger(hour);

    return 0;
}

int CentralPanelSerial::CMDDrainFixtime::exec()
{
    Controller::getInstance()->setDrainFixTime(day, hour, 0);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_DRAIN_FIXTIME, 0, sId);
    return 0;
}

//CMDFilterRinsePlan
int CentralPanelSerial::CMDFilterRinsePlan::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 5))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(year);
    proc->popInteger(month);
    proc->popInteger(day);
    proc->popInteger(hour);
    proc->popInteger(minute);

    return 0;
}

int CentralPanelSerial::CMDFilterRinsePlan::exec()
{
    Controller::getInstance()->setFilterRinsePlan(year, month, day, hour, minute);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_FILTER_RINSE_PLAN, 0, sId);
    return 0;
}

//CMDGetVersion
int CentralPanelSerial::CMDGetVersion::parse()
{
    return 0;
}

int CentralPanelSerial::CMDGetVersion::exec()
{
    Controller::getInstance()->getVersion();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_GET_VERSION, 0, sId);
    return 0;
}

//CMDDrainPlan
int CentralPanelSerial::CMDDrainPlan::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 5))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(year);
    proc->popInteger(month);
    proc->popInteger(day);
    proc->popInteger(hour);
    proc->popInteger(minute);

    return 0;
}

int CentralPanelSerial::CMDDrainPlan::exec()
{
    Controller::getInstance()->setDrainPlan(year, month, day, hour, minute);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_DRAIN_PLAN, 0, sId);
    return 0;
}

//CMDGetSettings
int CentralPanelSerial::CMDGetSettings::parse()
{
    return 0;
}

int CentralPanelSerial::CMDGetSettings::exec()
{
    Controller::getInstance()->getConfiguration();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_GET_SETTINGS, 0, sId);
    return 0;
}

//CMDSetDeviceId
int CentralPanelSerial::CMDSetDeviceId::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    //pop string
    proc->popString(deviceId, 32);

    //FTRACE("###deviceId: %s\r\n", deviceId);
    return 0;
}

int CentralPanelSerial::CMDSetDeviceId::exec()
{
    Controller::getInstance()->setDeviceId(deviceId);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_SET_DEVICE_ID, 0, sId);
    return 0;
}

//CMDEnableBLEDebug
int CentralPanelSerial::CMDEnableBLEDebug::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(bOn);

    return 0;
}

int CentralPanelSerial::CMDEnableBLEDebug::exec()
{
    Controller::getInstance()->enableBLEDebug(bOn);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_BLE_DEBUG, 0, sId);
    return 0;
}

//CMDWrongCommand
int CentralPanelSerial::CMDWrongCommand::parse()
{
    //if(!proc->hasNext(COMMAND_DELIMETER, 1))
    //    return Command::CMD_PARSE_NEED_DATA;

    //proc->popInteger(bOn);

    //FTRACE("###Enable BLE debug: %d\r\n", bOn);
    return 0;
}

int CentralPanelSerial::CMDWrongCommand::exec()
{
    //Controller::getInstance()->

    ((CentralPanelSerial*)proc)->sendResponse(CMD_WRONG_COMMAND, 0, sId);
    return 0;
}

//CMDMembraneRinseFixTime
int CentralPanelSerial::CMDMembraneRinseFixTime::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 3))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(day);
    proc->popInteger(hour);
    proc->popInteger(minute);

    return 0;
}

int CentralPanelSerial::CMDMembraneRinseFixTime::exec()
{
    Controller::getInstance()->setMembraneRinseFixTime(day, hour, minute);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_MEMBRANE_RINSE_FIXTIME, 0, sId);
    return 0;
}

//CMDMembraneRinsePlan
int CentralPanelSerial::CMDMembraneRinsePlan::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 5))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(year);
    proc->popInteger(month);
    proc->popInteger(day);
    proc->popInteger(hour);
    proc->popInteger(minute);

    return 0;
}

int CentralPanelSerial::CMDMembraneRinsePlan::exec()
{
    Controller::getInstance()->setMembraneRinsePlan(year, month, day, hour, minute);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_MEMBRANE_RINSE_PLAN, 0, sId);
    return 0;
}

//CMDDrain
int CentralPanelSerial::CMDDrain::parse()
{
    return 0;
}

int CentralPanelSerial::CMDDrain::exec()
{
    int retVal = Controller::getInstance()->drain();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_DRAIN, retVal<0?1:0, sId);
    return 0;
}

//CMDMembraneRinse
int CentralPanelSerial::CMDMembraneRinse::parse()
{
    return 0;
}

int CentralPanelSerial::CMDMembraneRinse::exec()
{
    int retVal = Controller::getInstance()->membraneRinse();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_MEMBRANE_RINSE, retVal<0?1:0, sId);
    return 0;
}

//CMDFilterRinse
int CentralPanelSerial::CMDFilterRinse::parse()
{
    return 0;
}

int CentralPanelSerial::CMDFilterRinse::exec()
{
    Controller::getInstance()->filterRinse();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_FILTER_RINSE, 0, sId);
    return 0;
}

//CMDGetDeviceId
int CentralPanelSerial::CMDGetDeviceId::parse()
{
    return 0;
}

int CentralPanelSerial::CMDGetDeviceId::exec()
{
    Controller::getInstance()->getDeviceId();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_GET_DEVICE_ID, 0, sId);
    return 0;
}

//CMDWechatAppPay
int CentralPanelSerial::CMDWechatAppPay::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 2))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(price);
    proc->popInteger(ml);

    return 0;
}

int CentralPanelSerial::CMDWechatAppPay::exec()
{
    Controller::getInstance()->wechatAppPay(price, ml);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_WECHAT_PAY, 0, sId);
    return 0;
}

//CMDSetCPVersion
int CentralPanelSerial::CMDSetCPVersion::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    //pop string
    proc->popString(ver, 16);
    return 0;
}

int CentralPanelSerial::CMDSetCPVersion::exec()
{
    Controller::getInstance()->setCentralPanelVer(ver);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_SET_CP_VER, 0, sId);
    return 0;
}

//CMDDownloadFw
CentralPanelSerial::CMDDownloadFw::CMDDownloadFw(CommandProcess* p): Command(p)
{
    sec = 0;
    len = 0;
    pos = 0;
}

CentralPanelSerial::CMDDownloadFw::~CMDDownloadFw()
{

}

int CentralPanelSerial::CMDDownloadFw::parse()
{
    // first time enter parse
    if(len <= 0)
    {
        if(!proc->hasNext(COMMAND_DELIMETER, 3))
            return Command::CMD_PARSE_NEED_DATA;

        proc->popInteger(sec);
        proc->popInteger(len);
        proc->popInteger(hashcode);

        //
        if(len <= 0 || len > FIRMWARE_DL_SECTOR_SIZE)
        {
            return 0;
        }

        FTRACE("###CentralPanelSerial::CMDDownloadFw::parse() sec: %d len:%d hash: 0x%08X\r\n", sec, len, (uint32_t)hashcode);
    }

    while(pos < len)
    {
        if(proc->dataSize())
        {
            proc->pop(data[pos]);
            pos ++;
        }
        else
        {
            return Command::CMD_PARSE_NEED_DATA;
        }
    }

    if(proc->dataSize() >= 2) // delimeter '#' and terminal byte '\0A'
    {
        // update checksum
        ((CentralPanelSerial*)proc)->addcheck(data, len);

        // pop the delimeter '#'
        char sharp;
        proc->pop(sharp);
        ((CentralPanelSerial*)proc)->addcheck(&sharp);
    }
    else
    {
        return Command::CMD_PARSE_NEED_DATA;
    }

    return 0;
}

#if ENABLE_CRC32_CHECKSUM
extern uint32_t make_crc(uint32_t crc, unsigned char *string, uint32_t size);
#endif //ENABLE_CRC32_CHECKSUM

int CentralPanelSerial::CMDDownloadFw::exec()
{
    // checksum
#if ENABLE_CRC32_CHECKSUM
    {
        uint32_t crc32 = 0xFFFFFFFF;
        crc32 = make_crc(crc32, (unsigned char*)data, len);
        crc32 ^= 0xFFFFFFFF;
        if(crc32 != (uint32_t)hashcode)
        {
            FTRACE("###CentralPanelSerial::CMDDownloadFw::exec() hashcode[0x%08X] check failed; [0x%08X]\r\n", hashcode, crc32);
            ((CentralPanelSerial*)proc)->sendResponse(CMD_FIRMWARE_DL, 1, sId);
            return 0;
        }
    }
#endif //ENABLE_CRC32_CHECKSUM

    if(len > 0 && len <= FIRMWARE_DL_SECTOR_SIZE)
    {
        if(Controller::getInstance()->writeDLFirmware(sec-1, data, len))
        {
            ((CentralPanelSerial*)proc)->sendResponse(CMD_FIRMWARE_DL, 1, sId); // failed
        }
        else
        {
            FTRACE("###CentralPanelSerial::CMDDownloadFw::exec() succeeded\r\n");
            ((CentralPanelSerial*)proc)->sendResponse(CMD_FIRMWARE_DL, 0, sId); // succeeded
        }
    }
    else
    {
        FTRACE("###CentralPanelSerial::CMDDownloadFw::exec() WRONG LENGTH, drop whole command data\r\n");
        proc->flush();
        ((CentralPanelSerial*)proc)->sendResponse(CMD_FIRMWARE_DL, 1, sId);
    }

    return 0;
}

//CMDInstallFw
int CentralPanelSerial::CMDInstallFw::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(hashcode);

    return 0;
}

int CentralPanelSerial::CMDInstallFw::exec()
{
    if(Controller::getInstance()->installFirmware(hashcode) < 0)
    {
        return ((CentralPanelSerial*)proc)->sendResponse(CMD_FIRMWARE_INSTALL, 1, sId);
    }

    ((CentralPanelSerial*)proc)->sendResponse(CMD_FIRMWARE_INSTALL, 0, sId);
    return 0;
}

//CMDRevertFw
int CentralPanelSerial::CMDRevertFw::parse()
{
    return 0;
}

int CentralPanelSerial::CMDRevertFw::exec()
{
    //Controller::getInstance()->

    ((CentralPanelSerial*)proc)->sendResponse(CMD_FIRMWARE_REVERT, 0, sId);
    return 0;
}

//CMDGetLogFile
int CentralPanelSerial::CMDGetLogFile::parse()
{
    return 0;
}

int CentralPanelSerial::CMDGetLogFile::exec()
{
    Controller::getInstance()->getLogFile();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_GET_LOGFILE, 0, sId);
    return 0;
}

//CMDStartProdTest
int CentralPanelSerial::CMDStartProdTest::parse()
{
    return 0;
}

int CentralPanelSerial::CMDStartProdTest::exec()
{
    Controller::getInstance()->startProdTest();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_START_PROD_TEST, 0, sId);
    return 0;
}

//CMDStopProdTest
int CentralPanelSerial::CMDStopProdTest::parse()
{
    return 0;
}

int CentralPanelSerial::CMDStopProdTest::exec()
{
    Controller::getInstance()->exitProdTest();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_STOP_PROD_TEST, 0, sId);
    return 0;
}

//CMDDisinfectFixTime
int CentralPanelSerial::CMDDisinfectFixTime::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 3))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(day);
    proc->popInteger(hour);
    proc->popInteger(minute);

    return 0;
}

int CentralPanelSerial::CMDDisinfectFixTime::exec()
{
    Controller::getInstance()->setDisinfectFixTime(day, hour, minute);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_DISINFECT_FIXTIME, 0, sId);
    return 0;
}

//CMDDisinfectPlan
int CentralPanelSerial::CMDDisinfectPlan::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 5))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(year);
    proc->popInteger(month);
    proc->popInteger(day);
    proc->popInteger(hour);
    proc->popInteger(minute);

    return 0;
}

int CentralPanelSerial::CMDDisinfectPlan::exec()
{
    Controller::getInstance()->setDisinfectPlan(year, month, day, hour, minute);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_DISINFECT_PLAN, 0, sId);
    return 0;
}

//CMDDisinfect
int CentralPanelSerial::CMDDisinfect::parse()
{
    return 0;
}

int CentralPanelSerial::CMDDisinfect::exec()
{
    Controller::getInstance()->disinfect();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_DISINFECT, 0, sId);
    return 0;
}

//CMDLockUsercard
int CentralPanelSerial::CMDLockUsercard::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 2))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(cardId);
    proc->popInteger(bLocked);

    return 0;
}

int CentralPanelSerial::CMDLockUsercard::exec()
{
    Controller::getInstance()->lockUsercard(cardId, bLocked);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_LOCK_USERCARD, 0, sId);
    return 0;
}

//CMDFunctionCancel
int CentralPanelSerial::CMDFunctionCancel::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(funcType);

    return 0;
}

int CentralPanelSerial::CMDFunctionCancel::exec()
{
    Controller::getInstance()->functionCancel(funcType);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_FUNCTION_CANCEL, 0, sId);
    return 0;
}

//CMDGetStatus
int CentralPanelSerial::CMDGetStatus::parse()
{

    return 0;
}

int CentralPanelSerial::CMDGetStatus::exec()
{
    Controller::getInstance()->getStatus();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_GET_STATUS, 0, sId);
    return 0;
}

//CMDSetICCardBalance
int CentralPanelSerial::CMDSetICCardBalance::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 2))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(cardId);
    proc->popInteger(balance);
    return 0;
}

int CentralPanelSerial::CMDSetICCardBalance::exec()
{
    Controller::getInstance()->setICCardBalance(cardId, balance);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_SET_ICCARD_BALANCE, 0, sId);
    return 0;
}

//CMDGetConfigParam
int CentralPanelSerial::CMDGetConfigParam::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(kind);
    return 0;
}

int CentralPanelSerial::CMDGetConfigParam::exec()
{
    Controller::getInstance()->getConfigParam(kind);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_GET_CONFIG_PARAM, 0, sId);
    return 0;
}

//CMDSetConfigParam
int CentralPanelSerial::CMDSetConfigParam::parseSubCmd()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(kind);
    return 0;
}

int CentralPanelSerial::CMDSetConfigParam::parse()
{
    if(kind == 0)
    {
        if(!proc->hasNext(COMMAND_DELIMETER, 1))
            return Command::CMD_PARSE_NEED_DATA;

        proc->popString(param.s1.secret, 65);
    }
    else if(kind == 1)
    {
        if(!proc->hasNext(COMMAND_DELIMETER, 5))
            return Command::CMD_PARSE_NEED_DATA;

        proc->popInteger(param.s2.bHeatPreservation);
        proc->popInteger(param.s2.bQRScanner);
        proc->popInteger(param.s2.bDustHelmet);
        proc->popInteger(param.s2.bICReader);
        proc->popInteger(param.s2.bSKI);
    }
    else
    {
        // error
    }

    return 0;
}

int CentralPanelSerial::CMDSetConfigParam::exec()
{
    if(kind == 0)
    {
        Controller::getInstance()->setConfigSecret(param.s1.secret);
    }
    else if(kind == 1)
    {
        Controller::getInstance()->setConfigParam(param.s2.bHeatPreservation, param.s2.bQRScanner,
            param.s2.bDustHelmet, param.s2.bICReader, param.s2.bSKI);
    }

    ((CentralPanelSerial*)proc)->sendResponse(CMD_SET_CONFIG_PARAM, 0, sId);
    return 0;
}

//CMDSetCardRegion
int CentralPanelSerial::CMDSetCardRegion::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 3))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(cardId);
    proc->popInteger(region);
    proc->popInteger(cardType);

    return 0;
}

int CentralPanelSerial::CMDSetCardRegion::exec()
{
    int status;
    status = Controller::getInstance()->setICCardRegion(cardId, region, cardType);

    if(0x00 == status)
    {
    ((CentralPanelSerial*)proc)->sendResponse(CMD_SET_CARD_REGION, 0, sId);
    }
    else
    {
        ((CentralPanelSerial*)proc)->sendResponse(CMD_SET_CARD_REGION, 1, sId);
    }
    
    return 0;
}

//CMDSetDeviceType
int CentralPanelSerial::CMDSetDeviceType::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(type);
    return 0;
}

int CentralPanelSerial::CMDSetDeviceType::exec()
{
    Controller::getInstance()->setDeviceType((unsigned int)type);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_SET_DEVICE_TYPE, 0, sId);
    return 0;
}

//CMDSetHotWaterRestrict
int CentralPanelSerial::CMDSetHotWaterRestrict::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(bRestrict);
    return 0;
}

int CentralPanelSerial::CMDSetHotWaterRestrict::exec()
{
    Controller::getInstance()->setDeviceRestrictHotWater(bRestrict);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_SET_HOTWATER_RESTRICT, 0, sId);
    return 0;
}

//CMDSetKidSafety
int CentralPanelSerial::CMDSetKidSafety::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(bKidSafety);
    return 0;
}

int CentralPanelSerial::CMDSetKidSafety::exec()
{
    Controller::getInstance()->setKidSafety(bKidSafety);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_SET_KIDSAFETY, 0, sId);
    return 0;
}

//CMD_CLEAR_SETTINGS
int CentralPanelSerial::CMDClearSettings::parse()
{
    return 0;
}

int CentralPanelSerial::CMDClearSettings::exec()
{
    ((CentralPanelSerial*)proc)->sendResponse(CMD_CLEAR_SETTINGS, 0, sId);
    PersistentStorage::getInstance()->ClearAllConfig();
    
    wait(3.0);
    NVIC_SystemReset();

    return 0;
}

//CMD_GET_WARNINGS
int CentralPanelSerial::CMDGetWarnings::parse()
{
    return 0;
}

int CentralPanelSerial::CMDGetWarnings::exec()
{
    Controller::getInstance()->getWarnings();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_GET_WARNINGS, 0, sId);
    return 0;
}

#ifdef TDS_CONTROL_SWITCH
//CMDSetWantedTDS
int CentralPanelSerial::CMDSetWantedTDS::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 2))
        return 1;

    proc->popInteger(bOn);
    proc->popInteger(tds);
    return 0;
}

int CentralPanelSerial::CMDSetWantedTDS::exec()
{
    CWaterMixerValve::getInstance()->enableAdjustTDS(bOn);
    CWaterMixerValve::getInstance()->setWantedTDS(tds);

    ((CentralPanelSerial*)proc)->sendResponse(CMD_SETWANTEDTDS, 0, sId);
    return 0;
}

//CMDRawWaterTDS
int CentralPanelSerial::CMDRawWaterTDS::parse()
{
    return 0;
}

int CentralPanelSerial::CMDRawWaterTDS::exec()
{
    CWaterMixerValve::getInstance()->ServoStartFlag = true;
    CWaterMixerValve::getInstance()->RawWaterFlag = true;

    ((CentralPanelSerial*)proc)->sendResponse(CMD_RAW_WATER_TDS, 0, sId);
    return 0;
}

//CMDMixedWaterTDS
int CentralPanelSerial::CMDMixedWaterTDS::parse()
{
    return 0;
}

int CentralPanelSerial::CMDMixedWaterTDS::exec()
{
    CWaterMixerValve::getInstance()->getMixedWaterTDS();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_MIXED_WATER_TDS, 0, sId);
    return 0;
}

#endif

//CMDGetMD5Value
int CentralPanelSerial::CMDGetMD5Value::parse()
{
    return 0;
}

int CentralPanelSerial::CMDGetMD5Value::exec()
{
    Controller::getInstance()->getMD5Value();

    ((CentralPanelSerial*)proc)->sendResponse(CMD_GET_MD5, 0, sId);
    return 0;
}

//CMDResponse
int CentralPanelSerial::CMDResponse::parse()
{
    if(!proc->hasNext(COMMAND_DELIMETER, 1))
        return Command::CMD_PARSE_NEED_DATA;

    proc->popInteger(result);

    return 0;
}

int CentralPanelSerial::CMDResponse::exec()
{
    int actId = -1;
    switch(cmd)
    {
        case CMD_APP_WATER_INFO_UPLOAD:
            actId = Action::AID_CP_WATERINFO;
            break;
        case CMD_CARD_BALANCE_UPLOAD:
            actId = Action::AID_CP_CARDINFO;
            break;
        case CMD_CARD_WATER_INFO_UPLOAD:
            actId = Action::AID_CP_CARDWATERINFO;
            break;
        case CMD_WATER_QUALITY_UPLOAD:
            actId = Action::AID_CP_WATERQUALITYINFO;
            break;
        case CMD_WARNING_UPLOAD:
            actId = Action::AID_CP_WARNINGINFO;
            break;
        case CMD_FILTER_INFO_UPLOAD:
            actId = Action::AID_CP_FILTERINFO;
            break;
        case CMD_SETTINGS_UPLOAD:
            actId = Action::AID_CP_CONFIGINFO;
            break;
        case CMD_FILTERRINSESTART:
            actId = Action::AID_CP_FILTERRINSESTART;
            break;
        case CMD_FILTERRINSEEND:
            actId = Action::AID_CP_FILTERRINSEEND;
            break;
        case CMD_DRAINING:
            actId = Action::AID_CP_DRAINING;
            break;
        case CMD_DRAINEND:
            actId = Action::AID_CP_DRAINEND;
            break;
        case CMD_WATERTANKSTATUS:
            actId = Action::AID_CP_WATERTANKSTATUS;
            break;
        case CMD_WATERSTATUS:
            actId = Action::AID_CP_WATERSTATUS;
            break;
        case CMD_COUNTDOWN:
            actId = Action::AID_CP_COUNTDOWN;
            break;
        case CMD_FILTERWORKABLEDAYS:
            actId = Action::AID_CP_FILTERWORKABLEDAYS;
            break;
        case CMD_VERSION:
            actId = Action::AID_CP_VERSION;
            break;
        case CMD_MEMBRANERINSESTART:
            actId = Action::AID_CP_MEMBRANERINSESTART;
            break;
        case CMD_MEMBRANERINSEEND:
            actId = Action::AID_CP_MEMBRANERINSEEND;
            break;
        case CMD_DEVICEID_UPLOAD:
            actId = Action::AID_CP_DEVICEID;
            break;
        case CMD_QRCODE:
            actId = Action::AID_CP_QRCODE;
            break;
        case CMD_INFRAREDSENSORWORKS:
            actId = Action::AID_CP_INFRAREDSENSORWORKS;
            break;
        case CMD_INFRAREDSENSORIDLE:
            actId = Action::AID_CP_INFRAREDSENSORIDLE;
            break;
        case CMD_FIRMWAREUPDATESTATUS:
            actId = Action::AID_CP_FIRMWAREUPDATESTATUS;
            break;
        case CMD_OPERATIONLOGSTATUS:
            actId = Action::AID_CP_OPERATIONLOGSTATUS;
            break;
        case CMD_OPERATIONLOG:
            actId = Action::AID_CP_OPERATIONLOG;
            break;
        case CMD_PRODUCTTESTSTATUS:
            actId = Action::AID_CP_PRODUCTTESTSTATUS;
            break;
        case CMD_PRODUCTTESTCOUNTDOWN:
            actId = Action::AID_CP_PRODUCTTESTCOUNTDOWN;
            break;
        case CMD_DISINFECTSTATUS:
            actId = Action::AID_CP_DISINFECTSTATUS;
            break;
        case CMD_REQUESTSYNCSYSTIME:
            actId = Action::AID_CP_REQUESTSYNCSYSTIME;
            break;
        case CMD_STATE_VALVE:
            actId = Action::AID_CP_VALVE_STATE;
            break;
        case CMD_CONFIGPARAMETER:
            actId = Action::AID_CP_CONFIGPARAMETER;
            break;
        case CMD_WARNINGS:
            actId = Action::AID_CP_WARNINGS;
        // case CMD_STEERING_ANGLE:
        //     actId = Action::AID_CP_STEERING_ANGLE;
        default:
            LOG_WARN("###CentralPanelSerial::CMDResponse::exec() Unknown cmdId: %d\r\n", cmd);
            break;
    }

    Controller::getInstance()->response(cmd, actId, result);
    ((CentralPanelSerial*)proc)->m_sendId ++;
    ((CentralPanelSerial*)proc)->m_sendId |= 0x80;

    return 0;
}
