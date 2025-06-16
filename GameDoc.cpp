#include "pch.h"
#include "framework.h"
#include "GameDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CGameDoc, CDocument)

BEGIN_MESSAGE_MAP(CGameDoc, CDocument)
END_MESSAGE_MAP()

CGameDoc::CGameDoc() noexcept
{
}

CGameDoc::~CGameDoc()
{
}

BOOL CGameDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    return TRUE;
}

void CGameDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // TODO: Add storing code here
    }
    else
    {
        // TODO: Add loading code here
    }
}

#ifdef _DEBUG
void CGameDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CGameDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif