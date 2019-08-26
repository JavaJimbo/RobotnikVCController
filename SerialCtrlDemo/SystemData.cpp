// SystemData.cpp: implementation of the SystemData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SystemData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

SystemData::SystemData()
{
	m_strSysConfigFileNameAndPath = ".\\Setup.INI";
	InitializeConfigFile();
	ReadConfigFile();
}

SystemData::~SystemData()
{
	;
}
	 
int SystemData::ReadConfigFile()
{
	char s[255];
	GetPrivateProfileString("LoggerSetup", "LoggingHours", "0", s, 255, m_strSysConfigFileNameAndPath);
	m_LoggingHours = atoi(s);
	GetPrivateProfileString("LoggerSetup", "LoggingMinutes", "1", s, 255, m_strSysConfigFileNameAndPath);
	m_LoggingMinutes = atoi(s);
	GetPrivateProfileString("LoggerSetup", "LoggingSeconds", "0", s, 255, m_strSysConfigFileNameAndPath);
	m_LoggingSeconds = atoi(s);

	GetPrivateProfileString("CalibrationSetup", "TotalSetpoints", "6", s, 255, m_strSysConfigFileNameAndPath);
	m_TotalSetpoints = atoi(s);
	GetPrivateProfileString("CalibrationSetup", "SetpointNumber", "0", s, 255, m_strSysConfigFileNameAndPath);
	m_SetpointIndex = atoi(s);
	GetPrivateProfileString("CalibrationSetup", "SetpointRH", "30.0", s, 255, m_strSysConfigFileNameAndPath);
	m_SetpointRH = atof(s);

	return 0;										// return no error
}

int SystemData::WriteConfigFile()
{
	CString sTemp = "";

	sTemp.Format("%i", m_LoggingHours);
	WritePrivateProfileString("LoggerSetup", "LoggingHours", sTemp, m_strSysConfigFileNameAndPath);
	sTemp.Format("%i", m_LoggingMinutes);
	WritePrivateProfileString("LoggerSetup", "LoggingMinutes", sTemp, m_strSysConfigFileNameAndPath);
	sTemp.Format("%i", m_LoggingSeconds);
	WritePrivateProfileString("LoggerSetup", "LoggingSeconds", sTemp, m_strSysConfigFileNameAndPath);

	sTemp.Format("%i", m_TotalSetpoints);
	WritePrivateProfileString("CalibrationSetup", "TotalSetpoints", sTemp, m_strSysConfigFileNameAndPath);
	sTemp.Format("%i", m_SetpointIndex);
	WritePrivateProfileString("CalibrationSetup", "SetpointNumber", sTemp, m_strSysConfigFileNameAndPath);
	sTemp.Format("%0.2f", m_SetpointRH);
	WritePrivateProfileString("CalibrationSetup", "SetpointRH", sTemp, m_strSysConfigFileNameAndPath);
	
	return 0;										// return no error
}

void SystemData::LoadConfigDefaults()
{
	m_LoggingHours = 0;
	m_LoggingMinutes = 1;
	m_LoggingSeconds = 0;

	m_TotalSetpoints = 6;
	m_SetpointIndex = 0;
	m_SetpointRH = 30.0;
}

bool SystemData::InitializeConfigFile()
{
	bool bExists = false;

	CFileStatus status;
	if (CFile::GetStatus(m_strSysConfigFileNameAndPath, status))		// check to see if the file exists
		bExists = true;
	else											// if file can not be opened, make new INI file	
	{
		LoadConfigDefaults();
		WriteConfigFile();
	}

	return bExists;
}
