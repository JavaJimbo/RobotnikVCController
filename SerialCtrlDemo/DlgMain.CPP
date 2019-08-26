/*  Project: ROBOTNIK CONTROLLER
* 
*   Adapted from Roscid Modbus Logger Plus
*	8-19-19 - Use one COM port at 921600 baud for send/receive. Works great with USB Robotnik and four pots, sending 63 bytes x 32 per second using COM 3.
*	8-23-19: Send/receiving 32 frames/second with Brain Board running USB ROBOTNIK firmware.
*	8-26-19: Reading & writing to files. Pot assignments and fast output works great. Sending 100 servo positions at roughly 32 writes/second.
*/
// #include <windows.h>
#include "stdafx.h"
#include "SerialCtrlDemo.h"
#include "SerialCtrlDemoDlg.h"
#include "SerialCtrl.h"
#include "Definitions.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <iostream>
#include <sstream>
#include <windows.h>
#include <fstream>
#include <stdio.h>
#include <conio.h>

#define NUMBER_OF_POTS 4

CString IOserialPortName = "COM6";

using std::ios;
std::ifstream inFile;
std::ofstream outFile;

unsigned long bytesSent = 0;
CString strMusicFile = "C:\\Temp\\MusicFile.txt";
CString strScratchpad = "C:\\Temp\\Scratchpad.txt";
CString strNewPotData;
std::ofstream outTextFile;

extern UINT16 crc_modbus(unsigned char *bufData, int num_bytes);

//CString TxPortName, RxPortName;
//PortName.Format("\\\\.\\%s", COMname);

static int frames = 0;
static int seconds = 0;
static int minutes = 0;

int frameNumber = 0;

int arrPotData[MAX_POTS];
int arrPotAssignments[MAX_POTS] = {5,6,7,8,0,0,0,0};
int arrServoPosition[MAX_SERVOS];

byte arrOutputBytes[MAX_PACKET_SIZE];
byte arrPacket[MAX_PACKET_SIZE]; // [MAX_SERVOS * 16];
unsigned int packetLength;

bool runFlag = false;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	IDT_TIMER_0	WM_USER + 200
#define TIMER_INTERVAL 20

// Main process loop timer interval in milliseconds
// Actually, it's more like milliseconds x 2
// So an interval value of 100 corresponds to about 200 milliseconds,
// or two tenths of a second.


class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

														// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);		
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

CSerialCtrlDemoDlg::CSerialCtrlDemoDlg(CWnd* pParent /*=NULL*/) : CDialog(CSerialCtrlDemoDlg::IDD, pParent), bPortOpened(FALSE)
{
}


CSerialCtrlDemoDlg::~CSerialCtrlDemoDlg()
{
	outTextFile.close();
	inFile.close();
	outFile.close();	

	//m_RxPort.ClosePort();
	m_TxPort.ClosePort();
}
	
void CSerialCtrlDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_EDIT_TIMER, m_EditTimer);
	DDX_Control(pDX, IDC_EDIT_RECEIVE, m_EditReceive);
	DDX_Control(pDX, IDC_LABEL_INFO, m_staticInfo);


}


BEGIN_MESSAGE_MAP(CSerialCtrlDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP	
	
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CSerialCtrlDemoDlg::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CSerialCtrlDemoDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CSerialCtrlDemoDlg::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_REHEARSE, &CSerialCtrlDemoDlg::OnBnClickedButtonRehearse)
	ON_BN_CLICKED(IDC_BUTTON_RECORD, &CSerialCtrlDemoDlg::OnBnClickedButtonRecord)
END_MESSAGE_MAP()



void CSerialCtrlDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}


// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSerialCtrlDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


int CSerialCtrlDemoDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add your specialized creation code here
	SetWindowLong(this->m_hWnd,
		GWL_STYLE,
		GetWindowLong(this->m_hWnd, GWL_STYLE) | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

	return 0;
}



// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CSerialCtrlDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}



void CSerialCtrlDemoDlg::OnTimer(UINT_PTR TimerVal)
{
	if (!KillTimer(TimerVal))
	{
		;
	}
	if (TestHandler())
		StartTimer(TIMER_INTERVAL);
}


UINT CSerialCtrlDemoDlg::StartTimer(UINT TimerDuration)
{
	UINT TimerVal;
	TimerVal = (UINT)SetTimer(IDT_TIMER_0, TimerDuration, NULL);
	if (TimerVal == 0)
		MessageBox("Unable to obtain timer", "SYSTEM ERROR", MB_OK | MB_SYSTEMMODAL);
	return TimerVal;
}// end StartTimer

bool CSerialCtrlDemoDlg::StopTimer(UINT TimerVal)
{
	if (!KillTimer(TimerVal)) 
		return false;
	else return true;
} // end StopTimer





BOOL CSerialCtrlDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//if (!m_RxPort.OpenPort("COM9", 19200))
	//	return false;

	m_staticInfo.SetWindowTextA("This is the info label");
	mode = STANDBY;

	if (!m_TxPort.OpenPort(IOserialPortName, 921600))  // was 3
		return false;
	else
		return TRUE;  // return TRUE  unless you set the focus to a control

}

int CSerialCtrlDemoDlg::GetServoPosition(int &frameNumber, byte &command, byte &subcommand, byte &numServos, int *ptrServoPosition, int &lineCount, int &InFileErrors)
{
	std::string strLine;
	const char delimiters[] = " \r\n";  // Each line of ASCII read from file will be scanned for one of three delimiters: a space, carriage return , or line break	
	char *token = NULL;
	char *context = NULL;
	char charLine[MAXLINE];
	int testint = 0;

	InFileErrors = NO_ERRORS;


	if (m_playbackState != PLAYBACK_RUN)
		return NO_ERRORS;
	else
	{
		if (std::getline(inFile, strLine))
		{
			// Read next line from file and convert to CString.
			// Then convert CString to char[] ASCII array.
			CString CLine(strLine.c_str());
			if (CLine.GetLength() == 0 || CLine.GetLength() > MAXLINE)
			{
				InFileErrors = FILE_READ_ERROR;
				return 0;
			}

			strcpy_s(charLine, MAXLINE, CLine);

			// Now check ASCII array for first occurence of a delimiter, hopefully a space ' '
			token = strtok_s(charLine, delimiters, &context);
			if (NULL == token)
				return 0;

			if (NULL == strchr(token, '>'))
				return 0;

			int i = 0;
			// Now step through ASCII text, extract each position string, and convert to integer:
			do {
				token = strtok_s(NULL, delimiters, &context);
				if (NULL == token) break;
				testint = ptrServoPosition[i++] = atoi(token);
			} while (i < MAX_SERVOS);
			numServos = i;
			lineCount++;
		}
		else
		{
			inFile.close();
			m_playbackState = PLAYBACK_EOF;
			return 0;
		}
	}

	return 1;
}

int  CSerialCtrlDemoDlg::GetPotData(CString strInData, int *ptrInData, int numPots)
{
	CString strHexValue;
	CString strTest;
	char *token = NULL;
	char *context = NULL;
	const char delimiters[] = ">, \r\n";
	char arrData[MAXLINE];
	char *ptrStartData;
	int intPotValue;
	int i = 0;

	int errors = strcpy_s(arrData, MAXLINE, strInData);

	ptrStartData = strchr(arrData, '>');
	if (ptrStartData == NULL) return 0;

	if (!errors)
	{
		token = strtok_s(ptrStartData, delimiters, &context);
		if (NULL == token) return 0;
		// if (NULL == strchr(token, '>')) return 0;
		do {
			token = strtok_s(NULL, delimiters, &context);
			if (NULL == token) break;
			intPotValue = atoi(token);
			ptrInData[i] = intPotValue;
			i++;
		} while (i < MAX_SERVOS);
	}
	numPots = i;
	return i;
}

int CSerialCtrlDemoDlg::SetServoPosition(int *arrServoPosition, int *arrPotData, int *arrPotAssignments)
{
	int potNumber, servoNumber, potData;
	for (potNumber = 0; potNumber < NUMBER_OF_POTS; potNumber++)
	{
		servoNumber = arrPotAssignments[potNumber];
		potData = arrPotData[potNumber];
		arrServoPosition[servoNumber] = potData;
	}
	return 0;
}



int CSerialCtrlDemoDlg::MakeTimeString(int frameCount, CString &strTime)
{
	int hours, minutes, seconds, frames;
	frames = frameCount;

	hours = frames / FRAMES_PER_HOUR;
	frames = frames - (hours * FRAMES_PER_HOUR);
	minutes = frames / FRAMES_PER_MINUTE;
	frames = frames - (minutes * FRAMES_PER_MINUTE);
	seconds = frames / FRAMES_PER_SECOND;
	frames = frames - (seconds * FRAMES_PER_SECOND);
	strTime.Format("[%02d:%02d:%02d:%02d]", hours, minutes, seconds, frames);
	return (0);
}


int CSerialCtrlDemoDlg::StoreServoPosition(int &frameNumber, byte &command, byte &subcommand, byte &numServos, int *ptrServoPosition)
{
	CString strTime, strTest, strDataValue, strOutLine, strCommand, strOutData;
	byte Command = 0, SubCommand = 0;

	MakeTimeString(frameNumber, strTime);
	m_staticInfo.SetWindowText(strTime);
	strCommand.Format("%d,%d,%d: ", Command, SubCommand, MAX_SERVOS);
	strOutData = "";
	for (int i = 0; i < MAX_SERVOS; i++)
	{
		strDataValue.Format("%03d ", ptrServoPosition[i]);
		strOutData = strOutData + strDataValue;
	}
	strOutLine.Format(">%s%s%s\r\n", strTime, strCommand, strOutData);
	outFile << strOutLine;

	return 0;
}

int CSerialCtrlDemoDlg::BuildPacket(byte command, byte subcommand, byte numServos, int *ptrServoPosition, byte *ptrPacket, unsigned int &packetLength)
{
	int i, j;
	UINT16 packetIndex = 0;
	MConvertType convert;

	BYTE B0, B1;
	
	j = 0;
	// Header first
	arrOutputBytes[j++] = command;
	arrOutputBytes[j++] = subcommand;
	arrOutputBytes[j++] = numServos;

	// Convert integer data to bytes
	for (i = 0; i < numServos; i++)
	{
		convert.integer = ptrServoPosition[i];
		arrOutputBytes[j++] = convert.b[0];
		arrOutputBytes[j++] = convert.b[1];
	}

	convert.integer = crc_modbus(arrOutputBytes, j);
	B0 = arrOutputBytes[j++] = convert.b[0];
	B1 = arrOutputBytes[j++] = convert.b[1];
	int numBytes = j;
			
	if (numBytes <= (MAX_DATA_BYTES + 16))
	{
		ptrPacket[packetIndex++] = STX;
		for (i = 0; i < numBytes; i++)
		{
			byte dataByte = arrOutputBytes[i];			
			if (dataByte == STX || dataByte == DLE || dataByte == ETX)
				ptrPacket[packetIndex++] = DLE;
			if (packetIndex >= MAX_PACKET_SIZE) return 0;
			if (dataByte == ETX) dataByte = ETX - 1;
			ptrPacket[packetIndex++] = dataByte;
		}		
		ptrPacket[packetIndex++] = ETX;
		packetLength = packetIndex;
		return (packetIndex);
	}
	else return (0);
}

BOOL CSerialCtrlDemoDlg::SendPacket(byte *ptrPacket, int packetLength, int &bytesSent)
{
	BOOL result = m_TxPort.WriteSerialPort(ptrPacket, packetLength, bytesSent);
	return result;
}


/*
if (mode == RECORD)
{
	GetServoPosition(frameNumber, command, subcommand, numServos, arrServoPosition, lineCount, InFileErrors);
	GetPotData(strData, arrPotData, numPots);
	SetServoPosition(arrServoPosition, arrPotData, arrPotAssignments);
	BuildPacket(command, subcommand, numServos, arrServoPosition, arrPacket, packetLength);
	// SendPacket(arrPacket, packetLength);
	StoreServoPosition(frameNumber, command, subcommand, numServos, arrServoPosition);
}
*/

/*
bool CSerialCtrlDemoDlg::TestHandler()
{
	CString strTimer;
	int i;
	static CString strData = "";
	CString strDisplayData = "";
	int numBytesReceived = 0;
	BYTE OutData[8] = "\r";
	static char TxBuffer[256];
	static unsigned char testByte = 'A';
	unsigned long bytesSent;
	static int counter = 1;

	int numPots;
	byte command, subcommand, numServos;
	int InFileErrors = 0;
	static int lineCount = 0;

	int length = sprintf_s(TxBuffer, 256, ">#%d: ", counter++);

	for (i = length; i < 70; i++)
	{
		if (testByte > 'Z') testByte = 'A';
		TxBuffer[i] = testByte++;
	}
	TxBuffer[68] = '<';
	TxBuffer[69] = '\r';
	m_TxPort.WriteSerialPort(TxBuffer, 70, bytesSent);

#define MAXBUFFER 256
	BYTE InData[MAXBUFFER];


	if (mode == RECORD)
	{
		GetServoPosition(frameNumber, command, subcommand, numServos, arrServoPosition, lineCount, InFileErrors);
		GetPotData(strData, arrPotData, numPots);
		SetServoPosition(arrServoPosition, arrPotData, arrPotAssignments);
		BuildPacket(command, subcommand, numServos, arrServoPosition, arrPacket, packetLength);
		SendPacket(arrPacket, packetLength, bytesSent);
		StoreServoPosition(frameNumber, command, subcommand, numServos, arrServoPosition);
	}


	numBytesReceived = (m_TxPort.ReadSerialPort(InData));
	if (numBytesReceived)
	{
		CString strInData(InData);
		strData = strData + strInData;
		if (strData.Find("\r") != -1)
		{
			strDisplayData = strData;
			m_EditReceive.SetWindowTextA(strDisplayData);
			strData = "";
		}
		frames++;
		if (frames >= 32)
		{
			frames = 0;
			seconds++;
			if (seconds >= 60)
			{
				seconds = 0;
				minutes++;
			}
		}
		strTimer.Format("Time: %d:%d:%d", minutes, seconds, frames);
		m_EditTimer.SetWindowTextA(strTimer);
	}

	return runFlag;
}
*/



void CSerialCtrlDemoDlg::OnBnClickedButtonPlay()
{
	runFlag = true;
	minutes = seconds = frames = 0;
	StartTimer(TIMER_INTERVAL);
}


void CSerialCtrlDemoDlg::OnBnClickedButtonStop()
{
	runFlag = false;
	StopTimer(TIMER_INTERVAL);

	//StopTimer(TIMER_INTERVAL);
	outTextFile.close();
	inFile.close();
	outFile.close();
	mode = STANDBY;
}




void CSerialCtrlDemoDlg::OnBnClickedButtonRehearse()
{
	// TODO: Add your control notification handler code here
}




void CSerialCtrlDemoDlg::PresetServos(char *ptrTxBuffer, int &numBytesToSend)
{
	for (int i = 0; i < MAX_SERVOS; i++) arrServoPosition[i] = i;
}

void CSerialCtrlDemoDlg::OnBnClickedButtonRecord()
{
	int i;
	char PresetBuffer[MAX_DATA_BYTES];
	for (i = 0; i < MAX_DATA_BYTES; i++) PresetBuffer[i] = 0x00;
	strcpy_s(PresetBuffer, MAX_DATA_BYTES, "This is the test preset just for now.\r");
	
	int numBytesToSend, bytesSent;
	int testPosition = 111;

	numBytesToSend = strlen(PresetBuffer);

	StartTimer(TIMER_INTERVAL);
	if (mode == STANDBY)
	{
		mode = RECORD;
		// PresetServos(PresetBuffer, numBytesToSend);
		byte command = 33, subCommand = 77;
		for (i = 0; i < MAX_SERVOS; i++)
		{
			arrServoPosition[i] = 0;
		}
		//GetServoPosition(frameNumber, command, subCommand, numServos, arrServoPosition, lineCount, InFileErrors);
		//GetPotData(strData, arrPotData, numPots);
		//SetServoPosition(arrServoPosition, arrPotData, arrPotAssignments);
		BuildPacket(command, subCommand, MAX_SERVOS, arrServoPosition, arrPacket, packetLength);
		SendPacket(arrPacket, packetLength, bytesSent);
		// BOOL result = m_TxPort.WriteSerialPort(PresetBuffer, numBytesToSend, bytesSent);
		//outTextFile.open(TextFileName, ios::out | ios::trunc);
		frameNumber = 0;
		m_playbackState = PLAYBACK_ENABLED;
		//SetServoPosition(arrServoPosition, arrPotData, arrPotAssignments);
		runFlag = true;
	}

}

bool CSerialCtrlDemoDlg::TestHandler()
{
	CString strTimer;
	int i;
	static CString strData = "";
	CString strDisplayData = "";
	int numBytesReceived = 0;
	BYTE OutData[8] = "\r";		
	int bytesSent;
	static int counter = 1;		
	int InFileErrors = 0;
	static int lineCount = 0;
	BYTE InData[256];	


	static int previousMode = STANDBY;

	if (previousMode != mode)
	{
		if (previousMode == STANDBY || previousMode == REHEARSE)
		{
			outFile.open(strScratchpad, std::ofstream::trunc);
			if (m_playbackState == PLAYBACK_ENABLED)
			{
				inFile.open(strMusicFile, std::ifstream::in);
				if (inFile.is_open())
					m_playbackState = PLAYBACK_RUN;
				else
				{
					m_playbackState = PLAYBACK_STANDBY;
					InFileErrors = FILE_OPEN_ERROR;
				}
			}

		}
		else inFile.close();
	}
	previousMode = mode;

	numBytesReceived = (m_TxPort.ReadSerialPort(InData));
	if (numBytesReceived)
	{
		CString strInData(InData);
		strData = strData + strInData;
		if (strData.Find("\r") != -1)
		{
			strDisplayData = strData;
			m_EditReceive.SetWindowTextA(strDisplayData);			
		}
		frames++;
		if (frames >= 32)
		{
			frames = 0;
			seconds++;
			if (seconds >= 60)
			{
				seconds = 0;
				minutes++;
			}
		}
		strTimer.Format("Time: %d:%d:%d", minutes, seconds, frames);
		m_EditTimer.SetWindowTextA(strTimer);
	}


	if (mode == RECORD)
	{
		frameNumber = frames + (seconds * 32) + (minutes * 32 * 32);
		byte command = 33, subCommand = 77, numServos = MAX_SERVOS;
		GetServoPosition(frameNumber, command, subCommand, numServos, arrServoPosition, lineCount, InFileErrors);
		GetPotData(strData, arrPotData, NUMBER_OF_POTS);
		strData = "";
		SetServoPosition(arrServoPosition, arrPotData, arrPotAssignments);
		BuildPacket(command, subCommand, numServos, arrServoPosition, arrPacket, packetLength);
		SendPacket(arrPacket, packetLength, bytesSent);
		StoreServoPosition(frameNumber, command, subCommand, numServos, arrServoPosition);
	}


	return runFlag;
}

void CSerialCtrlDemoDlg::OnBnClickedButtonReset()
{
	CString strTimer;
	if (mode == STANDBY)
	{
		frameNumber = 0;
		minutes = seconds = frames = 0;
		strTimer.Format("Time: %d:%d:%d", minutes, seconds, frames);
		m_EditTimer.SetWindowTextA(strTimer);
	}
}
