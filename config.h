#ifndef __CONFIG_H__
#define __CONFIG_H__

// define serial addr for debug
#define DEBUG_SERIAL_TX	PC_12 //USBTX
#define DEBUG_SERIAL_RX PD_2  //USBRX

// define serial addr for Central Panel
#define CENTRALPANEL_SERIAL_TX PA_9
#define CENTRALPANEL_SERIAL_RX PA_10

// define serial addr for Water Heater
#define WATERHEATER_SERIAL_TX PD_5
#define WATERHEATER_SERIAL_RX PD_6

// define serial addr for IC Card Reader
#define ICCARDREADER_SERIAL_TX PB_10
#define ICCARDREADER_SERIAL_RX PB_11

// define serial addr for QR Code Scan
#define QRCODE_SCAN_SERIAL_TX PA_0
#define QRCODE_SCAN_SERIAL_RX PA_1

// define serial addr for reserved port
#define RESERVED_SERIAL_TX PC_6
#define RESERVED_SERIAL_RX PC_7

// define power control of QR Code Scan
#define QRCODE_SCAN_POWER_CONTROL PA_2

// define Pin for beep
#define BEEP_PIN_ADDR PB_5

// define power control of central panel
#define CENTRLPANEL_POWER_CONTROL PA_8

// define Pin for infrared sensor
#define INFRARED_SENSOR_PIN_ADDR_0 PA_11
#define INFRARED_SENSOR_PIN_ADDR_1 PA_12

// define Pin for Hot/Cold water switch
#define HOT_WATER_SWITCH PD_8
#define COLD_WATER_SWITCH PD_9
#define ICE_WATER_SWITCH PB_0

// define Pin for Hot/Cold water valve
#define HOT_WATER_VALVE PE_8
#define COLD_WATER_VALVE PE_9
#define ICE_WATER_VALVE PE_10

// define Pin for water feed valve(B3)
#define FEED_WATER_GREEN_OUT PB_12
#define FEED_WATER_RED_OUT PB_13

// define Pin for 631 valve
#define FILTER_RINSE_MOTOR PD_7
#define FILTER_RINSE_POS_A PD_11
#define FILTER_RINSE_POS_B PD_10

// define Pin for flow meter
#define HOT_WATER_FLOW_METER PE_11
#define COLD_WATER_FLOW_METER PE_13
#define ICE_WATER_FLOW_METER PE_14
#define FEED_WATER_FLOW_METER PC_8

// define Pin for LED on switch
#define HOT_WATER_SWITCH_LED PC_9
#define COLD_WATER_SWITCH_LED PC_10
#define ICE_WATER_SWITCH_LED PC_11

// define Pin for LED
#define LED_LIGHT_RED PB_14
#define LED_LIGHT_GREEN PB_15

// define Pin for kid safety button
#define KID_SAFETY_BUTTON PB_1
#define ENABLE_KID_SAFETY_FUNCTION 1
#define KID_SAFETY_DELAY_SECOND 10
#define HOLD_TO_UNLOCK_DURATION 3 // in unit of second

// define Pin for invasion detect
#define INVASION_DETECT_0 PD_0
#define INVASION_DETECT_1 PD_1

// define Pin for water leak detection
#define DEVICE_WATER_LEAK_0 PC_2
#define DEVICE_WATER_LEAK_1 PC_3

// define Pin for UV fault detection
#define UV_DEVICE_FAULT PD_3

// define Pin for UV power control
#define UV_POWER_CONTROL PE_4

// define Pin for Dust Helmet step motor
#define STEP_MOTOR_BLUE PD_12
#define STEP_MOTOR_PINK PD_13
#define STEP_MOTOR_YELLOW PD_14
#define STEP_MOTOR_ORANGE PD_15

// define Pin for Dust Helmet Position switch
#define DUST_HELMET_OPENED PE_2
#define DUST_HELMET_CLOSED PE_3

// define Pin for TDS and PH
#define TDS_IN PC_1
#define PH_IN PC_0

// define Serial Command Ring Buffer size
#define COMMANDBUFFER_SIZE 256 //bytes

// define time out for resending command
#define ACTION_TIMEOUT	5 //seconds
#define ACTION_RESEND_LIMIT 5

// define main loop log interval; seconds
#define MAIN_LOOP_LOG_INTERVAL 5

// define digital switch ON/OFF value
#define DIGITAL_SWITCH_ON  1
#define DIGITAL_SWITCH_OFF 0

// define status update interval of water heater; in unit of seconds
#define WH_STATUS_UPDATE_INTERVAL 10

// define beep duration whild power on; in unit of seconds
#define POWERON_BEEP_DURATION 2.0

// define app eject timeout; in unit of seconds
//#define APP_EJECT_NO_OPERATE 10
//#define APP_EJECT_OPERATED 5

// define flags for enable/disable save log in Persistent Storage
#define ENABLE_SAVE_PERSISTENT_LOG 1

// define dump command packet; 0: off; 1: on;
#define DUMP_COMMAND_PACKET 1

// define refresh duration of watch dog; in unit of ms
#define WATCH_DOG_ENABLED 1
#define WATCH_DOG_REFRESH_DUR 20000

// define sector size of download firmware
#define FIRMWARE_DL_SECTOR_SIZE 128

// define delay seconds of infrared/UVFault detect
#define INFRARED_DETECTED_DELAY 12
#define UVFAULT_DETECTED_DELAY 15

// define warning repeat interval; in unit of second
#define WARNING_REPEAT_INTERVAL 10

// define flag for revert all configs
#define PERSISTENT_STORAGE_REVERT_ALL_CONFIGS 0
#define PERSISTENT_STORAGE_VERIFY_CONFIGS 1

// define minimal interval between two times of filter rinse/membrane rinse/drain/disinfect
// in unit of seconds;
#define FUNCTIONS_MINIMAL_INTERVAL 7200 // 2 hours

//Maximum duration of emptying
#define DRAIN_DURATION_TIME 30*60 //30 min

// define flag for enable extern RTC
#define ENABLE_EXTERNAL_RTC 1
#define USING_EXTERNAL_RTC_AS_SYNC_TIME 1

// define connected peripheral devices
#define PERIPHERAL_DEVICE_WATER_HEATER 1
#define PERIPHERAL_DEVICE_CENTRAL_PANEL 1
#define PERIPHERAL_DEVICE_IC_CARD_READER 1
#define PERIPHERAL_DEVICE_QR_SCANNER 1

// define variables for product test
#define PRODUCT_TEST_FILTER_BACK_WASH_DUR 15 // min
#define PRODUCT_TEST_FILTER_FAST_RINSE_DUR 5 // min
#define PRODUCT_TEST_MEMBRANE_RINSE_DUR 20 // min
#define PRODUCT_TEST_WAIT_TIME_BEFORE_DRAIN 60 //sec
#define PRODUCT_TEST_WAIT_TIME_AFTER_DRAIN 600 //sec

// define flag to enable/disable CRC32 function
#define ENABLE_CRC32_CHECKSUM 1

// define flag to enable/disable the sencond infrared sensor
#define ENABLE_DOUBLE_INFRARED_SENSOR 1

// define flag for temperature compensation calculate
#define ENABLE_TEMPERATURE_COMPENSATION 0
#define WATER_HEATER_HIGHEST_TEMPERATURE 98

// define IC card check interval
#define CHECK_INTERVAL_IC_CARD_READER 600 //sec

// define flag for enable DTS serial implement
#define ENABLE_DTS_SERIAL_IMPL 0

// TDS control switch
#define TDS_CONTROL_SWITCH 1

// define TDS detect interval; in uint of seconds
#define TDS_DETECT_INTERVAL 2*60

// define flag for user button scan instead of interrupt
#define USING_BUTTON_SCANNER 1
#define ENABLE_ICE_WATER 1

// define flag for UV power control
#define ENABLE_UV_POWER_CONTROL 1

// define flag for enable heltmet machine
#define ENABLE_CHECK_HELTMET_OPENED 1

#endif //__CONFIG_H__
