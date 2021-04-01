
// spdlog_test.h : main header file for the spdlog_test application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CspdlogtestApp:
// See spdlog_test.cpp for the implementation of this class
//

class CspdlogtestApp : public CWinApp
{
public:
	CspdlogtestApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CspdlogtestApp theApp;
