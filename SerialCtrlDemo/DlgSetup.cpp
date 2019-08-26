// DlgSetup.cpp : implementation file
//

#include "stdafx.h"
#include "DlgSetup.h"
#include "afxdialogex.h"
#include "SerialCtrlDemo.h"
#include "SerialCtrlDemoDlg.h"
#include "SerialCtrl.h"
#include <stdlib.h>
#include <stdio.h>



#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	IDT_TIMER_0	WM_USER + 200
#define TIMER_INTERVAL 100  // Main process loop timer interval in milliseconds
// Actually, it's more like milliseconds x 2
// So an interval value of 100 corresponds to about 200 milliseconds,
// or two tenths of a second.


IMPLEMENT_DYNAMIC(DlgSetup, CDialogEx)

DlgSetup::DlgSetup(CWnd* pParent /*=NULL*/) : CDialogEx(DlgSetup::IDD, pParent)
{

}

DlgSetup::~DlgSetup()
{
}

void DlgSetup::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);	
}


BEGIN_MESSAGE_MAP(DlgSetup, CDialogEx)	
	ON_WM_TIMER()
	
END_MESSAGE_MAP()


void DlgSetup::OnTimer(UINT_PTR TimerVal)
{
	if (!KillTimer(TimerVal))
	{
		;
	}
	if (TestHandler())
		StartTimer(TIMER_INTERVAL);
}

UINT DlgSetup::StartTimer(UINT TimerDuration)
{
	UINT TimerVal;
	TimerVal = (UINT)SetTimer(IDT_TIMER_0, TimerDuration, NULL);
	if (TimerVal == 0)
		MessageBox("Unable to obtain timer", "SYSTEM ERROR", MB_OK | MB_SYSTEMMODAL);
	return TimerVal;
}// end StartTimer

bool DlgSetup::StopTimer(UINT TimerVal)
{
	if (!KillTimer(TimerVal)) return false;
	else return true;
} // end StopTimer

bool DlgSetup::TestHandler()
{
	return true;
}

BOOL DlgSetup::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

