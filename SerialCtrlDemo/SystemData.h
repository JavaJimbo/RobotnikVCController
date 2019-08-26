// SystemData.h: interface for the SystemData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYSTEMDATA_H__AC1A5597_B918_42FA_AA7F_7274DEF73570__INCLUDED_)
#define AFX_SYSTEMDATA_H__AC1A5597_B918_42FA_AA7F_7274DEF73570__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class SystemData  
{
public:
	SystemData();
	virtual ~SystemData();

public:
	CString m_strSysConfigFileNameAndPath;

// file read/write/init functions
public:
	bool InitializeConfigFile();
	int WriteConfigFile();
	int ReadConfigFile();
	void LoadConfigDefaults();



	int m_LoggingHours, m_LoggingMinutes, m_LoggingSeconds, m_TotalSetpoints, m_SetpointIndex;
	double m_SetpointRH;
	CString m_strKeysightUSB_ID;

// process control variables
public:
};

#endif // !defined(AFX_SYSTEMDATA_H__AC1A5597_B918_42FA_AA7F_7274DEF73570__INCLUDED_)
