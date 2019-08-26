#ifndef _SERIAL_CTRL_H
#define _SERIAL_CTRL_H

#pragma once
#include "StdAfx.h"

UINT16 crc_modbus(unsigned char *bufData, int num_bytes);

class SerialCtrl
{
private:
	BOOL m_portStatus;                    
	HANDLE m_portHandle;                  
	DCB m_portConfig;
public:
	SerialCtrl(void);
	~SerialCtrl(void);             
	void SetPortStatus(BOOL bOnOff);
	BOOL GetPortStatus(void);
	HANDLE GetPortHandle(void);
	
	BOOL ClosePort(void);
	void msDelay(int milliseconds);	

	BOOL SerialCtrl::WriteSerialPort(BYTE *ptrOutputBytes, int sizeBuffer, int &bytesSent);
	BOOL SerialCtrl::WriteSerialPort(char *ptrOutputChar, int sizeBuffer, int &bytesSent);

	int  ReadSerialPort(BYTE *ptrPacket);	
	BOOL OpenPort(const char * portName, UINT16 baudrate);
	int ResetFileBuffers();
};

/*
typedef union
{
	BYTE b[2];
	UINT16 integer;
} MConvertType;
*/
#endif