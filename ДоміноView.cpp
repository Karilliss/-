
// ДоміноView.cpp : implementation of the CДоміноView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "Доміно.h"
#endif

#include "ДоміноDoc.h"
#include "ДоміноView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CДоміноView

IMPLEMENT_DYNCREATE(CДоміноView, CView)

BEGIN_MESSAGE_MAP(CДоміноView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CДоміноView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CДоміноView construction/destruction

CДоміноView::CДоміноView() noexcept
{
	// TODO: add construction code here

}

CДоміноView::~CДоміноView()
{
}

BOOL CДоміноView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CДоміноView drawing

void CДоміноView::OnDraw(CDC* /*pDC*/)
{
	CДоміноDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CДоміноView printing


void CДоміноView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CДоміноView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CДоміноView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CДоміноView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CДоміноView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CДоміноView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CДоміноView diagnostics

#ifdef _DEBUG
void CДоміноView::AssertValid() const
{
	CView::AssertValid();
}

void CДоміноView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CДоміноDoc* CДоміноView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CДоміноDoc)));
	return (CДоміноDoc*)m_pDocument;
}
#endif //_DEBUG


// CДоміноView message handlers
