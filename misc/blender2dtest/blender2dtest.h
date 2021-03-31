
// blender2dtest.h : main header file for the blender2dtest application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CBlender2dTestApp:
// See blender2dtest.cpp for the implementation of this class
//

class CBlender2dTestApp : public CWinApp
{
public:
	CBlender2dTestApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CBlender2dTestApp theApp;
