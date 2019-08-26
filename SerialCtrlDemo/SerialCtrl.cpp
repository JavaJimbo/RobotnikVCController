#include "StdAfx.h"
#include "SerialCtrl.h"

#define DEFAULT_BAUDRATE 19200

HANDLE gDoneEvent;
HANDLE hTimer = NULL;
HANDLE hTimerQueue = NULL;

unsigned char arrCopyTest[32];
int copyTestInt;

// extern CSerialCtrlDemoDlg *ptrDialog;

SerialCtrl::SerialCtrl(void) :m_portStatus(FALSE), m_portHandle(NULL)
{
	m_portStatus = false;
}

SerialCtrl::~SerialCtrl(void)
{
	m_portHandle = NULL;
}

void SerialCtrl::SetPortStatus(BOOL bOnOff)
{
	m_portStatus = bOnOff;
}

BOOL SerialCtrl::GetPortStatus(void)
{
	return m_portStatus;
}

HANDLE SerialCtrl::GetPortHandle(void)
{
	return m_portHandle;
}



BOOL SerialCtrl::OpenPort(const char * portName, UINT16 baudrate)
{
	CString strPortName;
	strPortName.Format("\\\\.\\%s", portName);

	if (m_portStatus == FALSE)  // if port is opened already, not open port again.
	{
		m_portHandle = CreateFile(strPortName,  // Specify port device: default "COM1"
			GENERIC_READ | GENERIC_WRITE,       // Specify mode that open device.
			0,                                  // the devide isn't shared.
			NULL,                               // the object gets a default security.
			OPEN_EXISTING,                      // Specify which action to take on file. 
			0,                                  // default.
			NULL);                              // default.

		// Get current configuration of serial communication port.
		if (GetCommState(m_portHandle,&m_portConfig) == 0)
		{
			AfxMessageBox("Cannot open serial port. Error code 1");
			SetPortStatus(false);
			return FALSE;
		}
		
		// Assign user parameter.
		m_portConfig.BaudRate = (DWORD) baudrate; //  dcb.BaudRate;    // Specify baud rate
		m_portConfig.StopBits = ONESTOPBIT; //  dcb.StopBits;    // Specify number of stopbits
		m_portConfig.Parity = NOPARITY; //  dcb.Parity;        // Specify parity
		m_portConfig.ByteSize = 8; //  dcb.ByteSize;    // Specify number of data bits in each transaction

		// Set current configuration of serial communication port.
		if (SetCommState(m_portHandle,&m_portConfig) == 0)
		{
			AfxMessageBox("Cannot open serial port. Error code 2");
			SetPortStatus(false);
			return FALSE;
		}

		// instance an object of COMMTIMEOUTS.
		COMMTIMEOUTS comTimeOut;                   
		comTimeOut.ReadIntervalTimeout = 3;   // Was 3?
		comTimeOut.ReadTotalTimeoutMultiplier = 3;  // was 3
		comTimeOut.ReadTotalTimeoutConstant = 2;	 // was 2
		comTimeOut.WriteTotalTimeoutMultiplier = 3; // Was 3
		comTimeOut.WriteTotalTimeoutConstant = 2;   // was 2

		SetCommTimeouts(m_portHandle,&comTimeOut);		// set the time-out parameter into device control.
		SetPortStatus(true);
		// m_portStatus = TRUE; 
		return TRUE;      
	}
	return FALSE;
}


BOOL SerialCtrl::WriteSerialPort(BYTE *ptrOutputBytes, int sizeBuffer, int &bytesSent)
{	 	
	LPDWORD lngBytesSent = 0;
	if (WriteFile(m_portHandle, // handle to file to write to
		ptrOutputBytes,              // pointer to data to write to file
		sizeBuffer,              // number of bytes to write
		lngBytesSent, NULL) == 0)      // pointer to number of bytes written
	{
		// AfxMessageBox("Serial write error.");
		return false;
	}
	bytesSent = (int)lngBytesSent;
	return true;
}

 BOOL SerialCtrl::WriteSerialPort(char *ptrOutputChar, int sizeBuffer, int &bytesSent)
{
	LPDWORD lngBytesSent = 0;
	if (WriteFile(m_portHandle, // handle to file to write to
		ptrOutputChar,              // pointer to data to write to file
		sizeBuffer,              // number of bytes to write
		lngBytesSent, NULL) == 0)      // pointer to number of bytes written
	{
		// AfxMessageBox("Serial write error.");
		return false;
	}
	bytesSent = (int)lngBytesSent;
	return true;
}

// These two routines implement a millisecond timer used for short delays
VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	SetEvent(gDoneEvent);
}

void SerialCtrl::msDelay(int milliseconds) {
	gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hTimerQueue = CreateTimerQueue();
	int arg = 123;
	CreateTimerQueueTimer(&hTimer, hTimerQueue, (WAITORTIMERCALLBACK)TimerRoutine, &arg, milliseconds, 0, 0);
	WaitForSingleObject(gDoneEvent, INFINITE);
	CloseHandle(gDoneEvent);
}


BOOL SerialCtrl::ClosePort(void)
{
	if (m_portStatus == TRUE)               // Port need to be open before.
	{
		m_portStatus = FALSE;                 // Update status
		if(CloseHandle(m_portHandle) == 0)    // Call this function to close port.
		{
			AfxMessageBox("Couldn't close serial port.");
			return FALSE;
		}    
		return TRUE;
	}
	return FALSE;
}

// Reads character string from serial port. String must be terminated by
// carriage return '\r' or newline '\n' or both.
// Inputs: ptrPortHandle points to serial port to read from
// Outputs: the array referenced by ptrPacket gets filled with incoming null characters, terminated by '\0'
//	 
// Returns TRUE if successful, FALSE if unable no string received after several retries		
/*
#define BUFFERSIZE 256
#define MAXTRIES 5
int SerialCtrl::ReadSerialPort(BYTE *ptrPacket) 
{	
	int i, numBytesRead;
	BYTE inBytes[BUFFERSIZE], ch;
	int packetIndex = 0;
	int trial = 0;

	// Make sure ptrPacket references an actual array:
	if (ptrPacket == NULL) return (FALSE);	

	packetIndex = 0;
	do 
	{
		// msDelay(10);
		// if (ReadFile(m_portHandle, inBytes, 32, &numBytesRead, NULL))
		Sleep(1);
		if ( Read(inBytes, 64, numBytesRead))
		{
			if (numBytesRead > 0 && numBytesRead < BUFFERSIZE) 
			{
				for (i = 0; i < numBytesRead; i++)
				{
					ch = inBytes[i];
					if (packetIndex < BUFFERSIZE) ptrPacket[packetIndex++] = ch;
					if (ch == '\r') break;
				}
				trial = 0;
			}
			else
			{
				trial++;
				Sleep(1);
			}
		}
		else break;		
	} while (trial < 4 && ch != '\r');
	if (packetIndex >= BUFFERSIZE) packetIndex = BUFFERSIZE - 1;
	ptrPacket[packetIndex] = '\0';
	return packetIndex;
}
*/

int SerialCtrl::ResetFileBuffers()
{
	if (!FlushFileBuffers(m_portHandle)) return 1;
	else return 0;
}


// Reads character string from serial port. String must be terminated by
// carriage return '\r' or newline '\n' or both.
// Inputs: ptrPortHandle points to serial port to read from
// Outputs: the array referenced by ptrPacket gets filled with incoming null characters, terminated by '\0'
//	 
// Returns TRUE if successful, FALSE if unable no string received after several retries		
#define BUFFERSIZE 256
#define MAXTRIES 5
int SerialCtrl::ReadSerialPort(BYTE *ptrPacket)
{
	DWORD i, numBytesRead;
	unsigned char inBytes[BUFFERSIZE], ch;
	int packetIndex = 0;
	int trial = 0;

	// Make sure ptrPacket references an actual array:
	if (ptrPacket == NULL) return (FALSE);

	packetIndex = 0;
	do
	{
		// msDelay(10);
		if (ReadFile(m_portHandle, inBytes, 256, &numBytesRead, NULL))
		{
			if (numBytesRead > 0 && numBytesRead < BUFFERSIZE)
			{
				for (i = 0; i < numBytesRead; i++)
				{
					ch = inBytes[i];
					if (packetIndex < BUFFERSIZE) ptrPacket[packetIndex++] = ch;
					if (ch == '\r') break;
				}
				trial = 0;
			}
			else
			{
				trial++;
				Sleep(1);
			}
		}
		else break;
	} while (trial < 4 && ch != '\r');
	if (packetIndex >= BUFFERSIZE) packetIndex = BUFFERSIZE - 1;
	ptrPacket[packetIndex] = '\0';
	return packetIndex;
}


