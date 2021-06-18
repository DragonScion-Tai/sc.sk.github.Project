#ifndef __VERSION_H__
#define __VERSION_H__

#define VER_MAJOR		2
#define VER_MINOR		0
#define VER_BUILD	 	73

const char* getVersionString();
#define VER_STRING		getVersionString()

/** VERSION UPDATED HISTORY ***
 
v2.0.73
	1.

v2.0.72
	1.Add TDS adjustment function;

v2.0.70
    1, set WD timeout to 20sec;

v2.0.69
    1, add some debug log.

v2.0.67
    1, fix invasion bug; don't report warning while authorized;
    2, add hold to unlock function for protecting hot water like Kid safety button;

v2.0.66
    1, add Open/Close dust helmet function for Debug serial port;
    2, add property for Kidsafety function;

v2.0.65
    1, add KidSafety Button function.

v2.0.64
    1, add CentralPanelSerial command for write IC Card Region, set device type, set hot
       water restrict flag.

v2.0.63
    1, for hot water restrict device, do not close dust helmet if account is activated;
    2, load device type;

v2.0.62
    1, add CentralPanelSerial command for set/get config parameter
    2, refine device type.

v2.0.61
    1, change for customerized card type 4; Using IC card validate user

v2.0.60
    1, change delay time for app pay mode

v2.0.59
    1. release helmet function build

v2.0.56
    1, reimplement helmet open rule for FREE mode;

v2.0.55
    1, enable helmet;

v2.0.54
    1, change wait time to 60 sec for Wechat\Ali pay;

v2.0.53
    1, add function for control the UV device power;

v2.0.52
    1, do not change B3 status when boot;

v2.0.51
    1, change delay time to 2sec of VALVE error check;

v2.0.50
    1, change to SK-I device;

v2.0.49
    1, save filter fast rinse duration;

v2.0.48
    1, fix check VIP expired time failed;

v2.0.47
    1, enable button scan feature for test;

v2.0.46
    1, fix APP eject issue for using two valves;

v2.0.45
    1, fix somtime issue of no water after press button;

v2.0.44
    1, submit sell finished event for free mode;

v2.0.43
    1, enable device type(SK I, SK II and SK III);

v2.0.42
    1, legacy code.

v2.0.41
	1, set and upload five additional pulse.
	2, save five additional pulse.

v2.0.40
	1, cancel the warning about waterflow.
	2, Solve the problem that there is no flow sometimes when press down the button in free case.
	3, Solve the problem that uploading information about finish flowing when remove IC card.

v2.0.39
	1, Solve the problem that heater keeps working out of working time.
	2, enter sold state when press down the button in free case.

v2.0.38
	1, fix the code that keep uploading waterflow information  when pressing finish in free case;
	2, Solve the problem that QR Scaning unsuccessfully for the first time;

v2.0.37
	1, fix the code that uploading waterflow information when pressing finish in free case;

v2.0.36
	1, upload the waterflow information when pressing finish in free case;

v2.0.35
	1, change the formula calculation of TDS;
	2, upload the consumption information when price is free;
	3, when endtime is zero,set Heater works allday;

v2.0.34
	1, close both membrane rinse and water release when fatal error happen;
	2, try to close water release when warning happen;
	3, save IC card balance to be ZERO;
	4, return water heater waning type 4(high temperature);
	5, action command repeat 5 times;

v2.0.33
	1, fix command id for set temperature restrict;

v2.0.32
	1, change highest boiling water temperature restrict from 93 to 98;
	2, do not save balance if central panel set balance is ZERO;

v2.0.31
	1, upload configuration when settings changed by Repair App;
	2, disable temperature compensation;

v2.0.30
	1, fix response process of TDS upload;

v2.0.29
	1, add interface for upload TDS value;
	2, TDS detect every half an hour;
	3, deactive account if valve error detected;
	4, prohibit cold water at filter expired day;
	5, upload product test start event when reboot during product test;
	6, reboot MBED when reboot Central Panel;
	7, change open angle of dust helmit;
	8, fix 631 valve position A duplicated interrupt;

v2.0.28
	1, add serial class for TDS;
	2, add function for multi times filter rinse;

v2.0.27
	1, calculaete cost only when button pressed;
	2, remove temperature compensation;

v2.0.26
	1, implement temperature compensation;
	2, change disinfect temperature to 93;

v2.0.25
	1, add fucntion to check alive of IC card reader;
	2, stop calc cost after button released even if the water continues releasing;
	3, change valve error detect to 0.5 sec delay;
	4, fix storage size of service/heater work time;
	5, add SetICCardBalance(142) function from Central Panel;

v2.0.24
	1, change valve error detect to 1 sec delay;
	2, enable the sencond infrared sensor;
	3, report invalid IC card for error data;
	4, beep 30sec when enter fatal error;

v2.0.23
	1, turn on/off QR scaner depend on infrared detection;
	2, restrict hot water selling for heater out of service;
	3, send IC card removed Event at booted
	4, remove dust helmet open/close position interrupt, because conflict with
	   UVFault interrupt;

v2.0.22
	1, enable minutes set for drain fix time;
	2, change warning for filter expired;
	3, invasion unauthorize automatically after door closed;
	4, add warning restrict for water release
	5, add report for illegal card and expired VIP card;
	6, VIP valid date extends to day.

v2.0.21
	1, add water forbidden function for each warning;
	2, add temperature compensation for water heater;
	3, fix VIP expired date when write IC card;
	4, fix spell error;
	5, fix timing issue: remove card between upload card info and set card state;
	6, change Hot Water switch LED Pin to ICE Water Switch LED Pin(PC_11);

v2.0.20
	1, fix remove IC card EVENT, regression from v2.0.19
	2, fix state machine state when disinfect time exhausted

v2.0.19
	1, send drinkable event at heat stop;
	2, save balance 0 to IC card;

v2.0.18
	1, fix the open/close feed water switch(B3) command from Central Panel
	2, change water leak value to 0x5000

v2.0.17
	1, fix can't start filter rinse after membrane rinse. Because it is always in S8.
	2, fix remove IC card change the state when doing filter rinse.
	3, fix 631 valve interrupt tiggered very quirkly at B position.
	4, add last day valid for VIP validity.

v2.0.16:
	Fix new board QR device power, need to new release version

v2.0.15
	for develop state;

v2.0.14:
	Tuning at JIADING on Dec. 10th 2017
	1, remove response for Water Heater warning;
	2, fix warning id for Water Heater;

v2.0.13:
	for develop state;

v2.0.12:
	Tuning at JIADING on Dec. 7th 2017
	1, change UVFault to interrupt event;
	2, use rise/fall pair to increase flow meter;

v2.0.11:
    Release to Ted for temp use.

v2.0.10:
	Tuning at JIADING on Nov. 23rd.
	1, fix disinfect process;
	2, test product test process;

v2.0.9:
	Tuning at JIADING on Nov. 16th.
	1, Using external RTC as sync time;
	2, write flash directly instead of using FAT file system when
		store configure data;

v2.0.8:
	Tuning at JIADING on Nov. 14th. Update version for releasing.
*/

#endif //__VERSION_H__
