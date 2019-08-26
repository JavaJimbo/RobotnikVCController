/* ModbusMaster.h
 * 
 *
 *
 */

#pragma once
#include "stdafx.h"
#include "SerialCtrl.h"

#define SLAVE_ID_EEPROM_ADDRESS 0
#define NUMBER_OF_SETPOINTS_EEPROM_ADDRESS 2
#define FILTER_SIZE_EEPROM_ADDRESS 4
#define PULSE_PERIOD_EEPROM_ADDRESS 6
#define REFERENCE_OFFSET_TEMPERATURE_ADDRESS 8
#define REFERENCE_GAIN_TEMPERATURE_ADDRESS 10
#define MEASURED_OFFSET_TEMPERATURE_ADDRESS 482
#define MEASURED_GAIN_TEMPERATURE_ADDRESS 484

#define MAX_NUMBER_OF_MODBUS_PORTS 16
#define MAX_SENSORS 256

#define VERSION_NUMBER_ADDRESS 0
#define RH_INPUT_ADDRESS 4
#define TEMPERATURE_CELSIUS_INPUT_ADDRESS 8
#define TEMPERATURE_FAHRENHEIT_INPUT_ADDRESS 12
#define DEWPOINT_INPUT_ADDRESS 16

#define MAX_DATA_REGISTERS 128
#define MAX_COM_PORTS 256
#define HIGHEST_SLAVE_ID 64
#define MAX_PORTNUMBER 255
#define MAX_SETPOINTS 64
#define MAXIMUM_RH_VALUE 200.00

#define CALIBRATE_TEMP_OFFSET_FUNC_CODE 125
#define CALIBRATE_TEMP_GAIN_FUNC_CODE 126

#define GET_SLAVE_ID_FUNC_CODE 127
#define SET_SLAVE_ID_FUNC_CODE 128

#define NUMBER_OF_CONFIGURATION_REGISTERS 6
#define DEFAULT_NUMBER_OF_SETPOINTS 6

enum STATUS {
	START = 0,
	IN_PROGRESS,
	DONE,	
	SENSOR_RESPONSE_OK,
	SYSTEM_ERROR,	
	SENSOR_ERROR,
	NO_RESPONSE
};

enum STATE {
	STANDBY = 0,
	LOG,
	GET_CAL_TABLE,

};

class ModbusMaster : public SerialCtrl
{
public:	
	ModbusMaster();
	virtual ~ModbusMaster();
	
	// void SetPointerToSerialControl(SerialCtrl *pSCTL) { m_pSCTL = pSCTL; };	
	int ReadWriteModbusDevice(BYTE slaveID, UINT16 StartWriteAddress, UINT16 NumberIntegersToWrite, UINT16 *WriteData, UINT16 StartReadAddress, UINT16 NumberOfIntegersToRead, UINT16 *ReadData, CString *strErrors);	
	int ReadModbusInputRegister(BYTE slaveID, UINT16 StartReadAddress, UINT16 NumberOfRegistersToRead, UINT16 *ReadData, CString *strErrors);
	int CalibrateRH(BYTE SlaveID, UINT16 totalSetpoints, UINT16 setpointIndex, UINT16 RHinteger, UINT16 *ptrADcounts);
	int CalibrateTemperatureOffset(double fOffsetCelsius);
	int CalibrateTemperatureGain(double fGainCelsius);
	int SendReceiveModbus(BYTE *OutModbusPacket, UINT16 OutLength, BYTE *InModbusPacket, UINT16 *InLength, bool ExpectResponse, CString *strErrors);	
	int SetModbusID(byte ExistingSlaveID, byte NewSlaveID, byte &VerifyID);	
	int GetModbusID(byte ExistingSlaveID, byte &NewSlaveID);
private:
	// SerialCtrl *m_pSCTL;
};

class ModbusPort
{
public:
	CString ComPortName;
	CString strSensors;
	int NumberOfSensors;
	int NumberOfRHComFailures;
	int NumberOfTEMPComFailures;
	BYTE SensorID[MAX_SENSORS];
	double SensorRH[MAX_SENSORS];
	double SensorTemperatureCelsius[MAX_SENSORS];
	double SensorTemperatureFahrenheit[MAX_SENSORS];
	ModbusMaster *pMod;
};

#define MAX_MODBUS_LENGTH 256
UINT16 crc_modbus(unsigned char *bufData, int num_bytes);


typedef union 
{
	BYTE b[2];
	UINT16 integer;
} ConvertType;


#define NO_ERRORS 0
#define MAX_MODBUS_PACKET_LENGTH 256
#define MAX_MODBUS_DATA_LENGTH 125








