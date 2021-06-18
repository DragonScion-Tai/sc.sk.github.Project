#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

class StateMachine
{
public:
	StateMachine();
	virtual ~StateMachine();

	static StateMachine* getInstance();

	typedef enum _STATE
	{
		S0 = 0, /**< Invalid state*/
		S1,		/**< Power on*/
		S2,		/**< Standby*/
		S3,		/**< Initialize failed*/
		S4,		/**< Hibernate*/
		S5,		/**< Sold state*/
		S6,		/**< Water release*/
		S7,		/**< Filter rinse*/
		S8,		/**< Membrane rinse*/
		S9,		/**< Drainage*/
		//S10,	/**< Water leak*/
		//S11,	/**< Water heater overflow*/
		//S12,	/**< Water heater over heated*/
		//S13,	/**< Invasion*/
		//S14,	/**< Disinfect*/
		//S15,	/**< Extern process*/
		//S16,	/**< Electric leak*/
		S10,	/**< Disinfect*/
		S11,	/**< Fatal Error state*/
		S12		/**< Production Test*/
	}State;

	const char* toString(State s);
	const char* currentState();

	enum _TRANSFERRESULT
	{
		TR_OK = 0,
		TR_FAIL = -1,
		TR_INVALID = -2
	};

	virtual int transferTo(State s);
	virtual int transferToStandby();
	virtual int transferToSoldState();
	virtual int transferToWaterRelease();
	virtual int transferToFilterRinse();
	virtual int transferToMembraneRinse();
	virtual int transferToDrainage();
	virtual int transferToDisinfect();
	virtual int transferToFatalErrorState();
	virtual int transferToProductionTest();

	virtual int stopFilterRinseManually();
	virtual int stopMembraneRinseManually();
	virtual int NotifyMembraneRinseComplete();
	virtual int stopDrainageManually();
	virtual int stopDisinfectManually();

	virtual int uploadStatus();

	virtual bool inStandbyState() { return m_state == S2 || m_state == S8; }
	virtual bool inSoldState() { return m_state == S5; }
	virtual bool inFilterRinseState() { return m_state == S7; }
	virtual bool inMembraneRinseState() { return m_bMembraneRinseState; }
	virtual bool inDrainageState() { return m_state == S9; }
	virtual bool inDisinfectState() { return m_state == S10; }
	virtual bool inFatalErrorState() { return m_state == S11; }
	virtual bool inProductionTestState() { return m_state == S12; }

private:
	friend class PersistentStorage;
	State m_state;
	bool m_bMembraneRinseState;

private:
    bool isFreeState();
};

extern StateMachine g_StateMachine;
#endif //__STATE_MACHINE_H__
