#pragma once
#include "framework.h"

class CDominoPuzzleApp : public CWinApp
{
public:
    CDominoPuzzleApp() noexcept;

public:
    BOOL InitInstance() override;
    int ExitInstance() override;

    // Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnAppAbout();
};
