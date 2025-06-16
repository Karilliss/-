#pragma once
#include "GameDoc.h"
#include "GameLogic.h"  // Add this include

// Command IDs - these should match your resource.h
#define ID_GAME_NEW_EASY          32772
#define ID_GAME_NEW_MEDIUM        32773
#define ID_GAME_NEW_HARD          32774
#define ID_GAME_RESET             32775
#define ID_GAME_HINT              32776
#define ID_GAME_AUTOSOLVE         32777
#define ID_ORIENTATION_HORIZONTAL 32778
#define ID_ORIENTATION_VERTICAL   32779

class GameUI : public CView
{
    DECLARE_DYNCREATE(GameUI)

private:
    // Constants
    static const int GRID_MARGIN = 20;
    static const int CELL_SIZE = 40;
    static const int DOMINO_PANEL_WIDTH = 120;
    static const int DOMINO_SIZE = 60;

    // Game logic instance (not reference)
    GameLogic m_gameLogic;

public:
    GameUI();
    virtual ~GameUI();
    CGameDoc* GetDocument() const;

    // Drawing functions
    void DrawGrid(CDC* pDC);
    void DrawCell(CDC* pDC, int row, int col, const CRect& rect);
    void DrawDomino(CDC* pDC, const Domino& domino, const CRect& rect, bool selected = false);
    void DrawDominoPanel(CDC* pDC);
    void DrawStats(CDC* pDC);

    // Utility functions
    CRect GetCellRect(int row, int col);
    CRect GetDominoRect(int index);
    void GetCellFromPoint(CPoint point, int& row, int& col);
    int GetDominoFromPoint(CPoint point);
    CString FormatTime(DWORD elapsed);
    void ShowMessage(const CString& message);

    // Game control functions
    void NewGame(Difficulty difficulty);
    void ResetGame();
    void GetHint();
    void AutoSolve();

protected:
    virtual void OnDraw(CDC* pDC);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

    // Message handlers
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    // Command handlers
    afx_msg void OnGameNewEasy();
    afx_msg void OnGameNewMedium();
    afx_msg void OnGameNewHard();
    afx_msg void OnGameReset();
    afx_msg void OnGameHint();
    afx_msg void OnGameAutoSolve();
    afx_msg void OnGameOrientationHorizontal();
    afx_msg void OnGameOrientationVertical();
    afx_msg void OnUpdateOrientationHorizontal(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOrientationVertical(CCmdUI* pCmdUI);

    DECLARE_MESSAGE_MAP()

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
};