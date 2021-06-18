#ifndef __PERSISTENT_STORAGE_H__
#define __PERSISTENT_STORAGE_H__

class PersistentStorage
{
public:
	PersistentStorage();
	virtual ~PersistentStorage();

	static PersistentStorage* getInstance();

    virtual void ClearAllConfig();
	virtual int init();    
	virtual int loadAllConfig();
	virtual int saveAllConfig();
	virtual int process(time_t seconds);

	virtual int saveDeviceId();
	virtual int saveHotWaterPrice();
	virtual int saveColdWaterPrice();
	virtual int saveIceWaterPrice();
	virtual int saveFilterWaterCur();
	virtual int saveFilterWaterMax();
	virtual int saveFilterProductionDate();
	virtual int saveFilterRinseDur();
	virtual int saveMembraneRinseDur();
	virtual int saveDrainageDur();
	virtual int saveServiceTime();
	virtual int saveHeaterWorkTime();
	virtual int saveFilterShelfLife();
	virtual int saveDisinfectDur();
	virtual int savePulsePerLitre();
	virtual int saveTemperature();
	virtual int saveWarningTempOffset();
	virtual int saveHeaterFBDur();
	virtual int saveFilterRinseFixtime();
	virtual int saveFilterRinsePlan();
	virtual int saveMembraneRinseFixtime();
	virtual int saveMembraneRinsePlan();
	virtual int saveDrainFixtime();
	virtual int saveDrainPlan();
	virtual int saveDisinfectFixtime();
	virtual int saveDisinfectPlan();
	virtual int saveStateMachineState();
	virtual int saveProductTestStep();
	virtual int saveFilterCheckMode();
	virtual int saveDisinfectDelayMax();
	virtual int saveInvasionDetectedFlag();
	virtual int saveTempCompensationParam();
    virtual int saveConfigParamSecret();
    virtual int saveConfigParam();
    virtual int saveDeviceType();
	virtual int saveFilterRinseFixtimes(int index);
	virtual int saveMembraneRinseFixtimes(int index);
	virtual int saveDisinfectFixtimes(int index);
	virtual int saveTDS_On_Off();
	virtual int saveTDSValue();
	virtual int savePulseWidth();

	// log
	virtual int readSavedLog(int lines=-1);
	virtual int readSavedLog(int startline, int lines);
	static  int logger(char module, char severity, short id, const char* fmt, ...);

	// for debug
	virtual int dumpFlash(unsigned int addr, unsigned int size);
	virtual int printLog(int line, int time, int info, int param1, int param2);
	virtual int getWriteLine() { return writeLine; }

private:
	static PersistentStorage* pThis;

	bool bInitialized;
	unsigned int writeLine;
	unsigned int writeLinePosition;
	bool bUploadLogs;
	unsigned int readLine;
	unsigned int totalReadLines;
	time_t lastSavedSecond;

	int save(int offset, void* data, int size);
	int saveOneLog(const char log[16]);
	int log_common(char module, char severity, short id, int param1, int param2);
	int saveLogWriteLine();
	int process_plan_time();

	int loadConfig_ver_0(char* temp);
};

#endif //__PERSISTENT_STORAGE_H__
