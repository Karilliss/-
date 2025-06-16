#include "pch.h"
#include "framework.h"
#include "DominoPuzzleApp.h"
#include "MainFrm.h"
#include "GameDoc.h"
#include "GameUI.h"
#include "AboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDominoPuzzleApp theApp;

BEGIN_MESSAGE_MAP(CDominoPuzzleApp, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, &CDominoPuzzleApp::OnAppAbout)
END_MESSAGE_MAP()

CDominoPuzzleApp::CDominoPuzzleApp() noexcept
{
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
}

void CDominoPuzzleApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

BOOL CDominoPuzzleApp::InitInstance()
{
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    EnableTaskbarInteraction(FALSE);
    SetRegistryKey(_T("DominoPuzzleGame"));

    CSingleDocTemplate* pDocTemplate;
    pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CGameDoc),
        RUNTIME_CLASS(CMainFrame),
        RUNTIME_CLASS(GameUI));
    if (!pDocTemplate)
        return FALSE;
    AddDocTemplate(pDocTemplate);

    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

    m_pMainWnd->ShowWindow(SW_SHOW);
    m_pMainWnd->UpdateWindow();

    return TRUE;
}

int CDominoPuzzleApp::ExitInstance()
{
    return CWinApp::ExitInstance();
}