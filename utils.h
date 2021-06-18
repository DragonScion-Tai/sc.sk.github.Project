#ifndef __UTILS_H__
#define __UTILS_H__
#include "mbed.h"

#include "config.h"
#include "version.h"
#include "command.h"
#include "centralPanelSerial.h"
#include "waterHeaterSerial.h"
#include "ICCardReaderSerial.h"
#include "QRCodeScanSerial.h"
#include "debugSerial.h"
#include "controller.h"
#include "scheduler.h"
#include "stateMachine.h"
#include "digitalSwitch.h"
#include "persistentStorage.h"
#include "warningManager.h"
#include "remoteLog.h"
#include "TDSSerial.h"
#include "waterMixerValve.h"
#include "md5.h"

#define FTRACE(...) debug_trace(__VA_ARGS__)

void debug_trace(const char* fmt, ...);
void debug_trace_va_list(const char* fmt, va_list arg);

#endif //__UTILS_H__
