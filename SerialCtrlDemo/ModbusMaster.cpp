/* RoscidModbus.cpp
 *
 *
 *
 */

#pragma once
#include "stdafx.h"
#include "ModbusMaster.h"

BYTE HighDataByte, LowDataByte;
UINT16 DataInt;
int TestLength;
UINT16 arrReadByteTest[255];



ModbusMaster::ModbusMaster()
{
	;
}

ModbusMaster::~ModbusMaster()
{
	;
}


int ModbusMaster::SendReceiveModbus(BYTE *OutModbusPacket, UINT16 OutLength, BYTE *InModbusPacket, UINT16 *InLength, bool ExpectResponse, CString *strErrors)
{
	int errors = 0;
	unsigned long bytesWritten;

	msDelay(100);
	ResetFileBuffers();  // $$$$


	MConvertType convert;

	convert.integer = crc_modbus(OutModbusPacket, OutLength);
	OutModbusPacket[OutLength++] = convert.b[0];
	OutModbusPacket[OutLength++] = convert.b[1];

	// TODO: Add your control notification handler code here
	if (GetPortStatus())
	{		
		errors = WriteSerialPort(OutModbusPacket, OutLength, bytesWritten);
		if (errors)
		{
			*strErrors = "Serial WRITE error";
			return errors;
		}
		else if (bytesWritten == 0)
		{
			*strErrors = "Serial WRITE error - no bytes written";
			return 3;
		}
		if (!ExpectResponse) 
			return errors;

		int TestLength = *InLength = ReadSerialPort(InModbusPacket);

		int outID = OutModbusPacket[0];
		int inID = InModbusPacket[0];

		if (InModbusPacket[0] != OutModbusPacket[0]) return SENSOR_ERROR;

		TestLength = *InLength;
		LowDataByte = convert.b[0] = InModbusPacket[TestLength - 2];
		HighDataByte = convert.b[1] = InModbusPacket[TestLength - 1];

		int intCRC = convert.integer;

		if (*InLength == 0)	
		{
			*strErrors = "No bytes received.";
			return 4;
		}
		TestLength = TestLength - 2;
		*InLength = *InLength - 2; // Now that we read the two CRC bytes at the end of the packet, we ignore them.

		if (crc_modbus(InModbusPacket, TestLength) != intCRC)
		{
			*strErrors = "CRC error";
			return 5;
		}
		// else (*strErrors).Format("RX: %02X, %02X, %02X, %02X, %02X, %02X", InModbusPacket[0], InModbusPacket[1], InModbusPacket[2], InModbusPacket[3], InModbusPacket[4], InModbusPacket[5]);
	}
	else
	{
		(*strErrors).Format("Port not open");
		return 6;
	}

	return 0;
}

// Format for request packet sent by Modbus master to slave:
// 0,        1,             2-3,                4-5
// Slave ID, Function Code, Read start address, Num Registers to Read
#define READ_INPUT_REGISTER_FUNC_CODE 4
int ModbusMaster::ReadModbusInputRegister(BYTE slaveID, UINT16 StartReadAddress, UINT16 NumberOfRegistersToRead, UINT16 *ReadData, CString *strErrors)
{
	BYTE OutPacket[MAX_MODBUS_PACKET_LENGTH], InPacket[MAX_MODBUS_PACKET_LENGTH];
	ConvertType convert;
	UINT16 i, RxIndex, OutPacketLength, InPacketLength, ComErrors, NumberOfIncomingIntegers;
	

	if (NumberOfRegistersToRead > MAX_MODBUS_DATA_LENGTH) return SYSTEM_ERROR;

	// Construct outgoing Modbus packet. Bytes #0 and #1 are always the slave ID and function code:
	OutPacket[0] = slaveID;
	OutPacket[1] = READ_INPUT_REGISTER_FUNC_CODE;

	if (ReadData != NULL)
	{
		// Define start address and number of registers to read:
		convert.integer = StartReadAddress;
		OutPacket[2] = convert.b[1];
		OutPacket[3] = convert.b[0];
		convert.integer = NumberOfRegistersToRead;
		OutPacket[4] = convert.b[1];
		OutPacket[5] = convert.b[0];

		int SlaveID = OutPacket[0];

		OutPacketLength = 6;

		ComErrors = SendReceiveModbus(OutPacket, OutPacketLength, InPacket, &InPacketLength, true, strErrors);
		if (ComErrors == NO_ERRORS)
		{
			// Make sure number of integers matches number expected:
			/*
			NumberOfIncomingIntegers = (InPacketLength - 3) / 2;
			if (NumberOfIncomingIntegers != NumberOfRegistersToRead) return SYSTEM_ERROR;
			*/
			if ((InPacket[1] & 0x80) != 0)
			{
				return NO_RESPONSE;
			}
			NumberOfIncomingIntegers = InPacket[2] / 2;
			if (NumberOfIncomingIntegers != NumberOfRegistersToRead) return SYSTEM_ERROR;
			

			RxIndex = 3; // Incoming data begins with third byte
						 // Get data bytes read and convert to two byte integers:
			for (i = 0; i < NumberOfIncomingIntegers; i++)
			{
				HighDataByte = convert.b[1] = InPacket[RxIndex++];
				LowDataByte = convert.b[0] = InPacket[RxIndex++];
 				DataInt = ReadData[i] = convert.integer;
			}
		}
		else return NO_RESPONSE;
		return SENSOR_RESPONSE_OK;
	}
	else return SYSTEM_ERROR;
}


// READ MULTIPLE REGISTERS COMMAND:
// Format for request packet sent by Modbus master to slave:
// 0,        1,             2-3,                4-5,                   6-7,                 8-9,                    10,               11...
// Slave ID, Function Code, Read start address, Num Registers to Read, Write start address, Num Registers to write, Write byte count, Write Data
//
// RESPONSE:
//      0             1                2             
//   slave ID    func code      Write Byte Count     
#define READ_WRITE_FUNC_CODE 23
int ModbusMaster::ReadWriteModbusDevice(BYTE slaveID, UINT16 StartWriteAddress, UINT16 NumberIntegersToWrite, UINT16 *WriteData, UINT16 StartReadAddress, UINT16 NumberOfIntegersToRead, UINT16 *ReadData, CString *strErrors)
{
	BYTE OutPacket[MAX_MODBUS_PACKET_LENGTH], InPacket[MAX_MODBUS_PACKET_LENGTH];
	ConvertType convert;
	UINT16 i, TxIndex, RxIndex, OutPacketLength, InPacketLength, ComErrors;

	if (NumberIntegersToWrite > MAX_MODBUS_DATA_LENGTH) return SYSTEM_ERROR;
	if (NumberOfIntegersToRead > MAX_MODBUS_DATA_LENGTH) return SYSTEM_ERROR;
	if (ReadData == NULL && NumberOfIntegersToRead != 0) return SYSTEM_ERROR;
	if (WriteData == NULL && NumberIntegersToWrite != 0) return SYSTEM_ERROR;

	// Construct outgoing Modbus packet. Bytes #0 and #1 are always the slave ID and function code:
	OutPacket[0] = slaveID;
	OutPacket[1] = READ_WRITE_FUNC_CODE;

	// if (WriteData != NULL && ReadData != NULL)

	// Define start address and number of registers to read:
	convert.integer = StartReadAddress;
	OutPacket[2] = convert.b[1];
	OutPacket[3] = convert.b[0];
	convert.integer = NumberOfIntegersToRead;
	OutPacket[4] = convert.b[1];
	OutPacket[5] = convert.b[0];

	// Next define start address and number of registers to write:
	convert.integer = StartWriteAddress;
	OutPacket[6] = convert.b[1];
	OutPacket[7] = convert.b[0];
	convert.integer = NumberIntegersToWrite;
	OutPacket[8] = convert.b[1];
	OutPacket[9] = convert.b[0];

	// This byte is the nuumber of bytes to write - not sure why this is necesssary:
	OutPacket[10] = (BYTE)(NumberIntegersToWrite * 2);

	// Add data to write beginning at packet byte #11:
	TxIndex = 11;
	if (WriteData != NULL)
	{
		for (i = 0; i < NumberIntegersToWrite; i++)
		{
			convert.integer = WriteData[i];
			OutPacket[TxIndex++] = convert.b[1];
			OutPacket[TxIndex++] = convert.b[0];
		}
	}
	OutPacketLength = TxIndex;

	ComErrors = SendReceiveModbus(OutPacket, OutPacketLength, InPacket, &InPacketLength, true, strErrors);

	for (i = 0; i < InPacketLength; i++)
		arrReadByteTest[i] = InPacket[i];

	// if (InPacket[0] != OutPacket[0]) return SENSOR_ERROR;  // $$$$

	ReadData[0] = (UINT16) InPacket[0];

	if (ComErrors == NO_ERRORS)
	{
		// Make sure number of integers matches number expected:
		int NumberOfIncomingIntegers = (InPacketLength - 3) / 2;
		if (NumberOfIncomingIntegers != NumberOfIntegersToRead) 
			return SYSTEM_ERROR;

		NumberOfIncomingIntegers = InPacket[2] / 2;
		if (NumberOfIncomingIntegers != NumberOfIntegersToRead) 
			return SYSTEM_ERROR;

		RxIndex = 3; // Incoming data begins with third byte
						 // Get data bytes read and convert to two byte integers:
		if (ReadData != NULL)
		{
			for (i = 0; i < NumberOfIntegersToRead; i++)
			{
				convert.b[1] = InPacket[RxIndex++];
				convert.b[0] = InPacket[RxIndex++];
				ReadData[i] = convert.integer;
			}
		}
		else return SENSOR_ERROR;
	}
	else return SYSTEM_ERROR;
	return SENSOR_RESPONSE_OK;
}



// CALIBRATE COMMAND:
//      0               1                2             3             4               5           6-7
//   slave ID       func code     total setpoints  setpoint #     RH MSB           RH LSB        CRC

#define CALIBRATE_FUNC_CODE 123
#define MAX_SETPOINTS 64
int ModbusMaster::CalibrateRH(BYTE SlaveID, UINT16 totalSetpoints, UINT16 setpointIndex, UINT16 RHinteger, UINT16 *ptrADcounts)
{
	BYTE OutPacket[MAX_MODBUS_PACKET_LENGTH], InPacket[MAX_MODBUS_PACKET_LENGTH];
	ConvertType convert;
	UINT16 OutPacketLength, InPacketLength, ComErrors;
	CString strErrors;

	if (totalSetpoints > MAX_SETPOINTS) return SYSTEM_ERROR;

	// Construct outgoing Modbus packet. Bytes #0 and #1 are always the slave ID and function code:
	OutPacket[0] = SlaveID;
	OutPacket[1] = CALIBRATE_FUNC_CODE;

	// Define start address and number of registers to read:
	OutPacket[2] = (BYTE)totalSetpoints;
	OutPacket[3] = (BYTE)setpointIndex;

	// Send RH as an integer, MSB first:
	convert.integer = RHinteger;
	OutPacket[4] = convert.b[1];
	OutPacket[5] = convert.b[0];

	OutPacketLength = 6;

	if (SlaveID == 0)
	{
		ComErrors = SendReceiveModbus(OutPacket, OutPacketLength, NULL, &InPacketLength, false, &strErrors);
		if (ComErrors != NO_ERRORS) return SYSTEM_ERROR;
		// SEND AD COUNT BACK TO MASTER:
		convert.b[1] = InPacket[6]; // AD count MSB
		convert.b[0] = InPacket[7]; // AD count LSB	  
		if (ptrADcounts != NULL) *ptrADcounts = convert.integer;
	}
	else ComErrors = SendReceiveModbus(OutPacket, OutPacketLength, InPacket, &InPacketLength, true, &strErrors);
	if (ComErrors != NO_ERRORS) return SYSTEM_ERROR;
	else return DONE;
}


// SET SLAVE ID COMMAND:
//       0                1             2-3                4-5
//   slave ID = 0      func code	Offset * 100 	       CRC


int ModbusMaster::CalibrateTemperatureOffset(double fOffsetCelsius)
{
	BYTE OutPacket[MAX_MODBUS_PACKET_LENGTH], InPacket[MAX_MODBUS_PACKET_LENGTH];
	ConvertType convert;
	UINT16 OutPacketLength, InPacketLength, ComErrors;
	CString strErrors;
	UINT16 intOffsetCelsius = (int)(fOffsetCelsius * 10);

	// Construct outgoing Modbus packet. 
	OutPacket[0] = 0;
	OutPacket[1] = CALIBRATE_TEMP_OFFSET_FUNC_CODE;
	
	convert.integer = intOffsetCelsius;
	OutPacket[2] = convert.b[1];
	OutPacket[3] = convert.b[0];
	
	OutPacketLength = 4;

	ComErrors = SendReceiveModbus(OutPacket, OutPacketLength, InPacket, &InPacketLength, false, &strErrors);
	if (ComErrors != NO_ERRORS) 
		return SYSTEM_ERROR;
	// else if (InPacket[2] != OutPacket[2]) 
	//	return SENSOR_ERROR;
	// else if (InPacket[3] != OutPacket[3]) 
	//	return SENSOR_ERROR;
	else return DONE;
}


int ModbusMaster::CalibrateTemperatureGain(double fGainCelsius)
{
	BYTE OutPacket[MAX_MODBUS_PACKET_LENGTH], InPacket[MAX_MODBUS_PACKET_LENGTH];
	ConvertType convert;
	UINT16 OutPacketLength, InPacketLength, ComErrors;
	CString strErrors;
	UINT16 intOffsetCelsius = (int)(fGainCelsius * 10);

	// Construct outgoing Modbus packet. 
	OutPacket[0] = 0;
	OutPacket[1] = CALIBRATE_TEMP_GAIN_FUNC_CODE;

	convert.integer = intOffsetCelsius;
	OutPacket[2] = convert.b[1];
	OutPacket[3] = convert.b[0];

	OutPacketLength = 4;

	ComErrors = SendReceiveModbus(OutPacket, OutPacketLength, InPacket, &InPacketLength, false, &strErrors);
	if (ComErrors != NO_ERRORS) return SYSTEM_ERROR;
	//else if (InPacket[2] != OutPacket[2]) return SENSOR_ERROR;
	//else if (InPacket[3] != OutPacket[3]) return SENSOR_ERROR;
	else return DONE;
}


// SET SLAVE ID COMMAND:
// Format for request packet sent by Modbus master to slave:
// 0,			1,             2
// Slave ID, Function Code, New Slave ID
//
// RESPONSE:
//      0           1          2             
// Slave ID, Function Code, New Slave ID     
int ModbusMaster::SetModbusID(byte ExistingSlaveID, byte NewSlaveID, byte &VerifyID)
{
	BYTE OutPacket[MAX_MODBUS_PACKET_LENGTH], InPacket[MAX_MODBUS_PACKET_LENGTH];	
	UINT16 OutPacketLength, InPacketLength, ComErrors;
	CString strErrors;

	// Construct outgoing Modbus packet. Bytes #0 and #1 are always the slave ID and function code:
	OutPacket[0] = ExistingSlaveID;
	OutPacket[1] = SET_SLAVE_ID_FUNC_CODE;
	OutPacket[2] = NewSlaveID;
	OutPacketLength = 3;

	ComErrors = SendReceiveModbus(OutPacket, OutPacketLength, InPacket, &InPacketLength, true, &strErrors);

	VerifyID = InPacket[2];

	if (ComErrors == NO_ERRORS && InPacket[2] == OutPacket[2]) return SENSOR_RESPONSE_OK;
	else return SENSOR_ERROR;	
}

int ModbusMaster::GetModbusID(byte ExistingSlaveID, byte &NewSlaveID)
{
	BYTE OutPacket[MAX_MODBUS_PACKET_LENGTH], InPacket[MAX_MODBUS_PACKET_LENGTH];
	UINT16 OutPacketLength, InPacketLength, ComErrors;
	CString strErrors;

	// Construct outgoing Modbus packet. Bytes #0 and #1 are always the slave ID and function code:
	OutPacket[0] = ExistingSlaveID;
	OutPacket[1] = GET_SLAVE_ID_FUNC_CODE;
	OutPacketLength = 2;

	ComErrors = SendReceiveModbus(OutPacket, OutPacketLength, InPacket, &InPacketLength, true, &strErrors);

	NewSlaveID = InPacket[2];

	if (ComErrors == NO_ERRORS) return SENSOR_RESPONSE_OK;
	else return SENSOR_ERROR;
}
