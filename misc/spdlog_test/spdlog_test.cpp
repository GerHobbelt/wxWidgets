
// spdlog_test.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "spdlog_test.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "spdlog_testDoc.h"
#include "spdlog_testView.h"

#include "logger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CspdlogtestApp

BEGIN_MESSAGE_MAP(CspdlogtestApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CspdlogtestApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CspdlogtestApp construction

CspdlogtestApp::CspdlogtestApp() noexcept
{
	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("spdlogtest.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CspdlogtestApp object

CspdlogtestApp theApp;

//template <typename T>
//void log_var(log_sink& s, T arg)
//{
//   s << arg;
//}

//template <typename T, typename ...ARGS>
//void log_info(log_sink& s, T arg, ARGS ...args)
//{
//	s.info(arg, args...);
//}

// CspdlogtestApp initialization

BOOL CspdlogtestApp::InitInstance()
{
	CWinApp::InitInstance();


	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_spdlogtestTYPE,
		RUNTIME_CLASS(CspdlogtestDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CspdlogtestView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

   LOG("Support for strings");
	LOG("Support for floats {:03.2f}", 1.23456);

   auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");

	logger->info("Welcome to spdlog!");
	logger->error("Some error message with arg: {}", 1);

	logger->warn("Easy padding in numbers like {:08d}", 12);
	logger->critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
	logger->info("Support for floats {:03.2f}", 1.23456);
	logger->info("Positional args are {1} {0}..", "too", "supported");
	logger->info("{:<30}", "left aligned");

	logger->set_level(spdlog::level::debug); // Set global log level to debug
	logger->debug("This message should be displayed..");

   // change log pattern
	logger->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");

   // Compile time log levels
   // define SPDLOG_ACTIVE_LEVEL to desired level
   SPDLOG_TRACE("Some trace message with param {}", 42);
   SPDLOG_DEBUG("Some debug message");

	return TRUE;
}

int CspdlogtestApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	return CWinApp::ExitInstance();
}

// CspdlogtestApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CspdlogtestApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CspdlogtestApp message handlers



