#pragma once
#include "SerialCtrl.h"
#include "afxwin.h"
#include "resource.h"
#include <stdlib.h>
#include <stdio.h>
#include "SystemData.h"

// DlgSetup dialog

class DlgSetup : public CDialogEx
{
	DECLARE_DYNAMIC(DlgSetup)

public:
	DlgSetup(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgSetup();
		
// Dialog Data
//#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
//#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CEdit m_EditSetpointIndex, m_EditTotalSetpoints, m_EditSetRH;
	CEdit m_EditTemperatureOffset, m_EditTemperatureGain;
	CEdit m_EditStatus2;
	CEdit m_EditExistingSensorID, m_EditNewSensorID, m_EditSelectComPort;
	CButton m_btnGetTable, m_ButtonRight, m_ButtonLeft;
	CListBox m_ListLog2;	
	// int m_NumberOfSetpoints;
	SystemData*				m_pSD;

public:	
	int  GetTable(bool ResetFlag);
	void DisplayLog(CString newString);	
	bool TestHandler();
	void OnTimer(UINT_PTR TimerVal);
	UINT StartTimer(UINT TimerDuration);
	bool StopTimer(UINT TimerVal);
	bool GetSensorID(int comPortIndex);
	bool SetSensorID(int comPortIndex);
	int  GetVersionNumber(bool ResetFlag);

	void	SetPointerToSD(SystemData*	pSD) { m_pSD = pSD; };
	void	KillPointerToSD() { m_pSD = NULL; };
	

	DECLARE_MESSAGE_MAP()
public:	
};
