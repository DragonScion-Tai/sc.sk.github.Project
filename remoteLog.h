#ifndef __REMOTE_LOG_H__
#define __REMOTE_LOG_H__

/**
 * 16 bytes remote log format defination
 * | 0~3 |  4   |  5  |6~7| 8~15 |
 * |time |module|flag |id |param |
 *
 * module: define module id; indicate which module
 *         0x00: Command
 *         0x01: Controller
 *         0x02: CentralPanelSerial
 *         0x03: WaterHeaterSerial
 *         0x04: ICCardReaderSerial
 *         0x05: QRCodeScanSerial
 *         0x06: DebugSerial
 *         0x07: DigitalSwitches
 *         0x08: Scheduler
 *         0x09: StateMachine
 *         0x0a: WarningManager
 *         0x0b: iDrink
 *
 * flag: severity of log
 *       0x00: fatal error
 *       0x01: error
 *       0x02: warning
 *       0x03: information
 *       0x04: dump packet
 *
 * id: the log number in current module, defined in each module respectively
 *
 * param: depends on the module and id
 */

#define LOG_INFO(...) PersistentStorage::logger(LOG_MODULE, 0x03, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) PersistentStorage::logger(LOG_MODULE, 0x02, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) PersistentStorage::logger(LOG_MODULE, 0x01, __LINE__, __VA_ARGS__)

#endif //__REMOTE_LOG_H__
