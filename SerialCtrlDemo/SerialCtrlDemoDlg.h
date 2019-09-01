// SerialCtrlDemoDlg.h : header file
//

#pragma once
#include "SerialCtrl.h"
#include "afxwin.h"
#include "SystemData.h"
#include "DlgSetup.h"
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <direct.h>

#define MAXLINE 1024
#define MAX_SERVOS 64
#define MAX_POTS MAX_SERVOS
#define MAX_PACKET_SIZE MAX_SERVOS * 4
#define MAX_DATA_BYTES (MAX_SERVOS * 2)

#define FRAMES_PER_HOUR (60*60*32)
#define FRAMES_PER_MINUTE (60*32)
#define FRAMES_PER_SECOND (32)

enum {
	STANDBY = 0,
	RECORD,
	PLAY,
	REHEARSE
};

enum {
	NO_ERRORS = 0,
	FILE_OPEN_ERROR,
	FILE_READ_ERROR,
	END_OF_FILE
};

class CSerialCtrlDemoDlg : public CDialog
{
// Construction
public:
	CSerialCtrlDemoDlg(CWnd* pParent = NULL);	// standard constructor
	~CSerialCtrlDemoDlg(); // Destructor

	bool bPortOpened;
			

// Dialog Data
	enum { IDD = IDD_SERIALCTRLDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support



// Implementation
protected:
	HICON m_hIcon;
		

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_EditTimer, m_EditReceive;
	SystemData	m_SD;
	SerialCtrl m_TxPort; // , m_RxPort;	

	bool TestHandler();
	bool StopTimer(UINT TimerVal);
	UINT StartTimer(UINT TimerDuration);
	void OnTimer(UINT_PTR TimerVal);

	bool DisplayServoPosition(int *ptrPotData, int numberofServos);
	bool GetServoPosition(int &frameNumber, byte &command, byte &subcommand, byte &numServos, int *ptrServoPosition);
	int GetPotData(CString strInData, int *ptrInData, int numPots);
	int SetServoPosition(int *arrServoPosition, int *arrPotData, int *arrPotAssignments);
	int StoreServoPosition(int &frameNumber, byte &command, byte &subcommand, byte &numServos, int *ptrServoPosition);
	int MakeTimeString(int frameCount, CString &strTime);
	int BuildPacket(byte command, byte subcommand, byte numServos, int *ptrServoPosition, byte *ptrPacket, unsigned int &packetLength);
	BOOL SendPacket(byte *ptrPacket, int packetLength, int &bytesSent);
	void PresetServos(char *ptrTxBuffer, int &numBytesToSend);

	int mode;
	CStatic m_staticInfo;	
	DlgSetup m_DlgSetup;		

public:
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedButtonRehearse();
	afx_msg void OnBnClickedButtonRecord();
};
